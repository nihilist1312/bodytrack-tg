#include "bot/message-service.hpp"

#include "bot/user-session.hpp"
#include "models/body-metrics.hpp"
#include "types/message-template.hpp"
#include "types/user-states.hpp"
#include "utils/date.hpp"
#include "utils/formatting.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

#include <spdlog/spdlog.h>
#include <tgbot/TgException.h>
#include <tgbot/types/InlineKeyboardButton.h>
#include <tgbot/types/InlineKeyboardMarkup.h>

MessageData
MessageService::loadMessage(const MessageTemplate& message_template) {
    MessageData data;

    // get text from json
    auto message_json = message_loader_.getMessage(message_template.key);
    data.text = message_json["text"];
    if (!message_template.text_placeholders.empty())
        formatInplace(data.text, message_template.text_placeholders);

    // get buttons from json
    auto button_rows = message_json["buttons"];
    data.keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
    size_t idx_btn = 0;
    for (const auto& row : button_rows) {
        std::vector<TgBot::InlineKeyboardButton::Ptr> buttons_row;

        for (const auto& button_json : row) {
            auto button_ptr = std::make_shared<TgBot::InlineKeyboardButton>();
            std::string btn_text = button_json["text"];
            if (idx_btn < message_template.buttons_text_placeholders.size() &&
                !message_template.buttons_text_placeholders[idx_btn].empty()) {
                formatInplace(
                    btn_text,
                    message_template.buttons_text_placeholders[idx_btn]);
            }
            spdlog::debug("Button {} text: {}", idx_btn, btn_text);
            std::string btn_data = button_json["callback"];
            if (idx_btn < message_template.buttons_data_placeholders.size() &&
                !message_template.buttons_data_placeholders[idx_btn].empty()) {
                formatInplace(
                    btn_data,
                    message_template.buttons_data_placeholders[idx_btn]);
            }
            spdlog::debug("Button {} data: {}", idx_btn, btn_data);
            button_ptr->text = std::move(btn_text);
            button_ptr->callbackData = std::move(btn_data);
            buttons_row.push_back(std::move(button_ptr));
            ++idx_btn;
        }

        data.keyboard->inlineKeyboard.push_back(std::move(buttons_row));
    }

    return data;
}

void MessageService::sendMessage(UserSession& session,
                                 const MessageTemplate& message_template) {
    spdlog::debug("Send message. Key: {}", message_template.key);
    auto message_data = loadMessage(message_template);
    sendMessage(session, message_template, message_data);
}
void MessageService::sendMessage(UserSession& session,
                                 const MessageTemplate& message_template,
                                 const MessageData& message_data) {
    auto message_ptr = bot_.getApi().sendMessage(
        session.chat_id, message_data.text, nullptr, 0, message_data.keyboard);
    session.last_message_template = message_template;

    spdlog::debug("Message sended. Chat ID: {}. Message ID: {}",
                  session.chat_id, message_ptr->messageId);
    session.last_message_id = message_ptr->messageId;
    spdlog::debug("Set last message id for user {}: {}",
                  session.user_data->user_id, session.last_message_id);
}

void MessageService::editMessage(UserSession& session,
                                 const MessageTemplate& message_template) {
    spdlog::debug("Edit message: Chat ID {}, Message ID {}. Message key: {}",
                  session.chat_id, session.last_message_id,
                  message_template.key);
    if (session.last_message_template == message_template) {
        spdlog::debug("Message is not modifield");
        return;
    }
    session.last_message_template = message_template;
    auto message_data = loadMessage(message_template);
    if (session.last_message_id == 0) {
        spdlog::debug("Cnnot edit message with ID 0. Send new");
        sendMessage(session, message_template, message_data);
        return;
    }
    try {
        bot_.getApi().editMessageText(message_data.text, session.chat_id,
                                      session.last_message_id, "", "", nullptr,
                                      message_data.keyboard);
        spdlog::debug("Edited succesfully");
    } catch (const TgBot::TgException& e) {
        std::string_view error_message = e.what();
        if (error_message.find("message is not modified") !=
            std::string::npos) {
            spdlog::debug("Message is not modifield");
            return;
        }

        spdlog ::debug("Edited failed. What: {}. Try send", e.what());
        sendMessage(session, message_template, message_data);
    }
}

void MessageService::deleteMessage(int64_t chat_id, int32_t message_id) try {
    spdlog::debug("Delete message: Chat ID {}, Message ID {}", chat_id,
                  message_id);
    if (message_id == 0 || chat_id == 0) {
        spdlog::debug("Cannot delete a message that has a chat or message ID "
                      "equal to 0.");
        return;
    }
    bot_.getApi().deleteMessage(chat_id, message_id);
} catch (const TgBot::TgException& e) {
    spdlog::debug("Delete failed. What: {}", e.what());
}

void MessageService::deleteMessage(const TgBot::Message::Ptr& message) {
    deleteMessage(message->chat->id, message->messageId);
}

void MessageService::deleteLast(UserSession& session) {
    spdlog::debug("Delete last message.");
    session.last_message_template = {};
    deleteMessage(session.chat_id, session.last_message_id);
    session.last_message_id = 0;
}

void MessageService::openMainMenu(UserSession& session) {
    session.last_body_metrics =
        database_.getLastBodyMetrics(session.user_data->user_id);
    spdlog::debug("Open main menu");
    if (session.last_body_metrics) {
        auto& current_metrics = session.last_body_metrics;
        editMessage(session, {"main_menu",
                              {
                                  session.user_data->user_name,
                                  current_metrics->weight,
                                  fatPercentage(current_metrics.value()),
                                  current_metrics->muscle_mass,
                                  current_metrics->date,
                              }});
    } else {
        spdlog::debug("Open without data");
        editMessage(session, {"empty_menu"});
    }
    session.current_state = UserStates::MainMenu;
}

void MessageService::requestWeight(UserSession& session) {
    // если измерение есть то предлагаем прошлый вес
    if (session.last_body_metrics.has_value()) {
        editMessage(
            session,
            {"request_weight", {}, {{session.last_body_metrics->weight}}});
    } else {
        editMessage(session, {"request_weight_empty"});
    }
    session.current_state = UserStates::InputWeight;
}

void MessageService::requestHeight(UserSession& session) {
    if (session.last_body_metrics.has_value()) {
        editMessage(
            session,
            {"request_height", {}, {{session.last_body_metrics->height}}});
    } else {
        editMessage(session, {"request_height_empty"});
    }
    session.current_state = UserStates::InputHeight;
}

void MessageService::requestMuscleMass(UserSession& session) {
    if (session.last_body_metrics.has_value()) {
        editMessage(session, {"request_muscle_mass",
                              {},
                              {{session.last_body_metrics->muscle_mass}}});
    } else {
        editMessage(session, {"request_muscle_mass_empty"});
    }
    session.current_state = UserStates::InputMuscleMass;
}

void MessageService::requestFatMass(UserSession& session) {
    if (session.last_body_metrics.has_value()) {
        editMessage(
            session,
            {"request_fat_mass", {}, {{session.last_body_metrics->fat_mass}}});
    } else {
        editMessage(session, {"request_fat_mass_empty"});
    }
    session.current_state = UserStates::InputFatMass;
}

void MessageService::printSummary(UserSession& session) {
    editMessage(session, {"save_metric",
                          {session.body_metrics_draft.date,
                           session.body_metrics_draft.weight,
                           session.body_metrics_draft.height,
                           session.body_metrics_draft.muscle_mass,
                           session.body_metrics_draft.fat_mass}});
    session.current_state = UserStates::MetricSummary;
}

void MessageService::invalidWeight(UserSession& session) {
    spdlog::debug("Invalid weigth");
    if (session.last_body_metrics.has_value()) {
        editMessage(
            session,
            {"invalid_weight", {}, {{session.last_body_metrics->weight}}});
    } else {
        editMessage(session, {"invalid_weight_empty"});
    }
    session.current_state = UserStates::InputWeight;
}

void MessageService::invalidHeight(UserSession& session) {
    spdlog::debug("Invalid heigth");
    if (session.last_body_metrics.has_value()) {
        editMessage(
            session,
            {"invalid_height", {}, {{session.last_body_metrics->height}}});
    } else {
        editMessage(session, {"invalid_height_empty"});
    }
    session.current_state = UserStates::InputHeight;
}

void MessageService::invalidMuscleMass(UserSession& session) {
    spdlog::debug("Invalid muscle mass");
    if (session.last_body_metrics.has_value()) {
        editMessage(session, {"invalid_muscle_mass",
                              {},
                              {{session.last_body_metrics->muscle_mass}}});
    } else {
        editMessage(session, {"invalid_muscle_mass_empty"});
    }
    session.current_state = UserStates::InputMuscleMass;
}

void MessageService::invalidFatMass(UserSession& session) {
    spdlog::debug("Invalid fat mass");
    if (session.last_body_metrics.has_value()) {
        editMessage(
            session,
            {"invalid_fat_mass", {}, {{session.last_body_metrics->fat_mass}}});
    } else {
        editMessage(session, {"invalid_fat_mass_empty"});
    }
    session.current_state = UserStates::InputFatMass;
}

void MessageService::requestDate(UserSession& session) {
    editMessage(session, {"request_date", {}, {{getCurrentDate()}}});
    session.current_state = UserStates::InputDate;
}

void MessageService::invalidDate(UserSession& session) {
    spdlog::debug("Invalid date");
    editMessage(session, {"invalid_date", {}, {{getCurrentDate()}}});
    session.current_state = UserStates::InputDate;
}

void MessageService::doublicateDate(UserSession& session) {
    spdlog::debug("Date doublicate: {}", session.body_metrics_draft.date);
    editMessage(session, {"duplicate_date",
                          {session.body_metrics_draft.date},
                          {{session.body_metrics_draft.date}},
                          {{session.body_metrics_draft.date}}});
    session.current_state = UserStates::DateDublicate;
}

void MessageService::selectAdditional(UserSession& session) {
    spdlog::debug("Additional metrics list");
    if (haveAdditional(session.body_metrics_draft)) {
        spdlog::debug("Addition metrics exist");
        editMessage(session, {"additional_metrics",
                              {additionalFormat(session.body_metrics_draft)}});
    }
    // пишем пояснение если ни одна метрика не была введена
    else {
        spdlog::debug("Additional metrics does exist");
        editMessage(session, {"additional_metrics_empty"});
    }

    session.current_state = UserStates::SelectAdditionalMetrics;
}

// Универсальный хелпер для request_*/invalid_* простых (несегментных) метрик
template <class T>
void MessageService::requestSimpleMetric(UserSession& session,
                                         std::optional<T> BodyMetrics::* field,
                                         UserStates state,
                                         const std::string& base_message_key,
                                         const std::string& log_label,
                                         bool is_invalid) {

    spdlog::debug("Request {}", log_label);

    if (session.body_metrics_draft.*field) {
        spdlog::debug("{} edit", log_label);
        if (is_invalid) {
            editMessage(session, {base_message_key + "_edit"});
        } else {
            editMessage(session,
                        {base_message_key + "_edit",
                         {(session.body_metrics_draft.*field).value()}});
        }
    } else if (session.last_body_metrics &&
               session.last_body_metrics.value().*field) {
        spdlog::debug("Previous {}", log_label);
        editMessage(session,
                    {base_message_key + "_previous",
                     {},
                     {{(session.last_body_metrics.value().*field).value()}}});
    } else {
        spdlog::debug("Empty {}", log_label);
        editMessage(session, {base_message_key});
    }

    session.current_state = state;
}

void MessageService::requestWaterMass(UserSession& session) {
    requestSimpleMetric(session, &BodyMetrics::water_mass,
                        UserStates::InputWaterMass, "request_water_mass",
                        "water mass", false);
}

void MessageService::requestBoneMass(UserSession& session) {
    requestSimpleMetric(session, &BodyMetrics::bone_mass,
                        UserStates::InputBoneMass, "request_bone_mass",
                        "bone mass", false);
}

void MessageService::requestVisceralFat(UserSession& session) {
    requestSimpleMetric(session, &BodyMetrics::visceral_fat,
                        UserStates::InputVisceralFat, "request_visceral_fat",
                        "visceral fat", false);
}

void MessageService::requestProteinMass(UserSession& session) {
    requestSimpleMetric(session, &BodyMetrics::protein_mass,
                        UserStates::InputProteinMass, "request_protein_mass",
                        "protein mass", false);
}

void MessageService::invalidWaterMass(UserSession& session) {
    requestSimpleMetric(session, &BodyMetrics::water_mass,
                        UserStates::InputWaterMass, "invalid_water_mass",
                        "water mass", true);
}

void MessageService::invalidBoneMass(UserSession& session) {
    requestSimpleMetric(session, &BodyMetrics::bone_mass,
                        UserStates::InputBoneMass, "invalid_bone_mass",
                        "bone mass", true);
}

void MessageService::invalidVisceralFat(UserSession& session) {
    requestSimpleMetric(session, &BodyMetrics::visceral_fat,
                        UserStates::InputVisceralFat, "invalid_visceral_fat",
                        "visceral fat", true);
}

void MessageService::invalidProteinMass(UserSession& session) {
    requestSimpleMetric(session, &BodyMetrics::protein_mass,
                        UserStates::InputProteinMass, "invalid_protein_mass",
                        "protein mass", true);
}

// Универсальный хелпер для request_*/invalid_* сегментных групп (muscle/fat)
void MessageService::requestSegmentGroup(
    UserSession& session, std::optional<SegmentMass> BodyMetrics::* field,
    UserStates state, const std::string& base_message_key,
    const std::string& log_label) {

    spdlog::debug("Request {}", log_label);

    if (session.segment_mass_draft) {
        spdlog::debug("{} edit", log_label);
        editMessage(session,
                    {base_message_key + "_edit",
                     {segmentMassFormat(session.body_metrics_draft.*field)}});
    } else if (session.last_body_metrics &&
               session.last_body_metrics.value().*field) {
        spdlog::debug("Previous {}", log_label);
        editMessage(
            session,
            {base_message_key + "_previous",
             {segmentMassFormat(session.last_body_metrics.value().*field)}});
    } else {
        spdlog::debug("Empty {}", log_label);
        editMessage(session, {base_message_key});
    }

    session.current_state = state;
}

void MessageService::requestMuscleSegment(UserSession& session) {
    requestSegmentGroup(session, &BodyMetrics::segment_muscle_mass,
                        UserStates::InputMuscleSegments,
                        "request_segment_muscle_mass", "muscle segment");
}

void MessageService::requestFatSegment(UserSession& session) {
    requestSegmentGroup(session, &BodyMetrics::segment_fat_mass,
                        UserStates::InputFatSegments,
                        "request_segment_fat_mass", "fat segment");
}

void MessageService::invalidMuscleSegment(UserSession& session) {
    requestSegmentGroup(session, &BodyMetrics::segment_muscle_mass,
                        UserStates::InputMuscleSegments,
                        "invalid_segment_muscle_mass",
                        "invalid muscle segment");
}

void MessageService::invalidFatSegment(UserSession& session) {
    requestSegmentGroup(session, &BodyMetrics::segment_fat_mass,
                        UserStates::InputFatSegments,
                        "invalid_segment_fat_mass", "invalid fat segment");
}

void MessageService::requestLeftArmMuscle(UserSession& session) {
    requestSegment(session, "Left arm", &BodyMetrics::segment_muscle_mass,
                   &SegmentMass::left_arm, UserStates::InputLeftArmMuscle,
                   "request_segment_muscle_mass_value");
}

void MessageService::requestRightArmMuscle(UserSession& session) {
    requestSegment(session, "Right arm", &BodyMetrics::segment_muscle_mass,
                   &SegmentMass::right_arm, UserStates::InputRightArmMuscle,
                   "request_segment_muscle_mass_value");
}

void MessageService::requestLeftLegMuscle(UserSession& session) {
    requestSegment(session, "Left leg", &BodyMetrics::segment_muscle_mass,
                   &SegmentMass::left_leg, UserStates::InputLeftLegMuscle,
                   "request_segment_muscle_mass_value");
}

void MessageService::requestRightLegMuscle(UserSession& session) {
    requestSegment(session, "Right leg", &BodyMetrics::segment_muscle_mass,
                   &SegmentMass::right_leg, UserStates::InputRightLegMuscle,
                   "request_segment_muscle_mass_value");
}

void MessageService::requestTrunkMuscle(UserSession& session) {
    requestSegment(session, "Trunk", &BodyMetrics::segment_muscle_mass,
                   &SegmentMass::trunk, UserStates::InputTrunkMuscle,
                   "request_segment_muscle_mass_value");
}

// Универсальный хелпер для request_* и invalid_* сообщений по сегментам массы
void MessageService::requestSegment(
    UserSession& session, const std::string& segment_name,
    std::optional<SegmentMass> BodyMetrics::* metrics_field,
    std::optional<double> SegmentMass::* field, UserStates state,
    const std::string& base_message_key) {

    spdlog::debug("Request {}", segment_name);

    std::optional<double>* new_value;

    // проверяем что масса сегментов есть, берем указатель на нужный
    // и проверяем определен ли он
    if (session.body_metrics_draft.*metrics_field &&
        (new_value =
             &((session.body_metrics_draft.*metrics_field).value().*field)) &&
        *new_value) {
        spdlog::debug("{} edit", segment_name);
        editMessage(session, {base_message_key + "_edit",
                              {segment_name, new_value->value()}});
    } else if (session.last_body_metrics &&
               session.last_body_metrics.value().*metrics_field) {
        auto& prev_value =
            (session.last_body_metrics.value().*metrics_field).value().*field;
        spdlog::debug("Previous value");
        editMessage(session, {base_message_key + "_previous",
                              {segment_name, prev_value.value()},
                              {{prev_value.value()}}});
    } else {
        spdlog::debug("Default version");
        editMessage(session, {base_message_key, {segment_name}});
    }

    session.current_state = state;
}

// Обёртки request_*
void MessageService::requestLeftArmFat(UserSession& session) {
    requestSegment(session, "Left arm", &BodyMetrics::segment_fat_mass,
                   &SegmentMass::left_arm, UserStates::InputLeftArmFat,
                   "request_segment_fat_mass_value");
}

void MessageService::requestRightArmFat(UserSession& session) {
    requestSegment(session, "Right arm", &BodyMetrics::segment_fat_mass,
                   &SegmentMass::right_arm, UserStates::InputRightArmFat,
                   "request_segment_fat_mass_value");
}

void MessageService::requestLeftLegFat(UserSession& session) {
    requestSegment(session, "Left leg", &BodyMetrics::segment_fat_mass,
                   &SegmentMass::left_leg, UserStates::InputLeftLegFat,
                   "request_segment_fat_mass_value");
}

void MessageService::requestRightLegFat(UserSession& session) {
    requestSegment(session, "Right leg", &BodyMetrics::segment_fat_mass,
                   &SegmentMass::right_leg, UserStates::InputRightLegFat,
                   "request_segment_fat_mass_value");
}

void MessageService::requestTrunkFat(UserSession& session) {
    requestSegment(session, "Trunk", &BodyMetrics::segment_fat_mass,
                   &SegmentMass::trunk, UserStates::InputTrunkFat,
                   "request_segment_fat_mass_value");
}

void MessageService::invalidLeftArmMuscle(UserSession& session) {
    requestSegment(session, "Left arm", &BodyMetrics::segment_muscle_mass,
                   &SegmentMass::left_arm, UserStates::InputLeftArmMuscle,
                   "invalid_segment_muscle_mass_value");
}

void MessageService::invalidRightArmMuscle(UserSession& session) {
    requestSegment(session, "Right arm", &BodyMetrics::segment_muscle_mass,
                   &SegmentMass::right_arm, UserStates::InputRightArmMuscle,
                   "invalid_segment_muscle_mass_value");
}

void MessageService::invalidLeftLegMuscle(UserSession& session) {
    requestSegment(session, "Left leg", &BodyMetrics::segment_muscle_mass,
                   &SegmentMass::left_leg, UserStates::InputLeftLegMuscle,
                   "invalid_segment_muscle_mass_value");
}

void MessageService::invalidRightLegMuscle(UserSession& session) {
    requestSegment(session, "Right leg", &BodyMetrics::segment_muscle_mass,
                   &SegmentMass::right_leg, UserStates::InputRightLegMuscle,
                   "invalid_segment_muscle_mass_value");
}

void MessageService::invalidTrunkMuscle(UserSession& session) {
    requestSegment(session, "Trunk", &BodyMetrics::segment_muscle_mass,
                   &SegmentMass::trunk, UserStates::InputTrunkMuscle,
                   "invalid_segment_muscle_mass_value");
}

void MessageService::invalidLeftArmFat(UserSession& session) {
    requestSegment(session, "Left arm", &BodyMetrics::segment_fat_mass,
                   &SegmentMass::left_arm, UserStates::InputLeftArmFat,
                   "invalid_segment_fat_mass_value");
}

void MessageService::invalidRightArmFat(UserSession& session) {
    requestSegment(session, "Right arm", &BodyMetrics::segment_fat_mass,
                   &SegmentMass::right_arm, UserStates::InputRightArmFat,
                   "invalid_segment_fat_mass_value");
}

void MessageService::invalidLeftLegFat(UserSession& session) {
    requestSegment(session, "Left leg", &BodyMetrics::segment_fat_mass,
                   &SegmentMass::left_leg, UserStates::InputLeftLegFat,
                   "invalid_segment_fat_mass_value");
}

void MessageService::invalidRightLegFat(UserSession& session) {
    requestSegment(session, "Right leg", &BodyMetrics::segment_fat_mass,
                   &SegmentMass::right_leg, UserStates::InputRightLegFat,
                   "invalid_segment_fat_mass_value");
}

void MessageService::invalidTrunkFat(UserSession& session) {
    requestSegment(session, "Trunk", &BodyMetrics::segment_fat_mass,
                   &SegmentMass::trunk, UserStates::InputTrunkFat,
                   "invalid_segment_fat_mass_value");
}

void MessageService::cannotSaveMuscleSegments(UserSession& session) {
    spdlog::debug("Error save muscle segments");
    editMessage(session, {"invalid_save_muscle_segments",
                          {segmentMassFormat(session.segment_mass_draft)}});
    session.current_state = UserStates::InputMuscleSegments;
}

void MessageService::cannotSaveFatSegments(UserSession& session) {
    spdlog::debug("Error save fat segments");
    editMessage(session, {"invalid_save_fat_segments",
                          {segmentMassFormat(session.segment_mass_draft)}});
    session.current_state = UserStates::InputFatSegments;
}