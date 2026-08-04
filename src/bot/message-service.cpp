#include "bot/message-service.hpp"

#include "bot/user-session.hpp"
#include "models/body-metrics.hpp"
#include "types/message-data.hpp"
#include "types/message-template.hpp"
#include "types/user-states.hpp"
#include "utils/date.hpp"
#include "utils/formatting.hpp"
#include "utils/utils.hpp"

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
    session.body_metrics_draft = {};
    session.segment_mass_draft = std::nullopt;
    session.history_offset = 0;
    session.current_state = UserStates::MainMenu;
}

// ---- Ввод даты -------------------------------------------------------------
void MessageService::requestDate(UserSession& session) {
    spdlog::debug("Request date");
    if (session.body_metrics_draft.date != "") {
        spdlog::debug("Edit version");
        editMessage(session,
                    {"request_date_edit", {session.body_metrics_draft.date}});
    } else {
        spdlog::debug("Default version");
        editMessage(session, {"request_date", {}, {{getCurrentDate()}}});
    }

    session.current_state = UserStates::InputDate;
}

void MessageService::invalidDate(UserSession& session) {
    spdlog::debug("Invalid date");
    if (session.body_metrics_draft.date != "") {
        spdlog::debug("Edit version");
        editMessage(session,
                    {"invalid_date_edit", {session.body_metrics_draft.date}});
    } else {
        spdlog::debug("Default version");
        editMessage(session, {"invalid_date", {}, {{getCurrentDate()}}});
    }

    session.current_state = UserStates::InputDate;
}

void MessageService::doublicateDate(UserSession& sessions) {
    spdlog::debug("Dublicate date");
    editMessage(sessions, {"dublicate_date",
                           {sessions.body_metrics_draft.date},
                           {},
                           {{sessions.body_metrics_draft.date}}});
    sessions.current_state = UserStates::DateDublicate;
}

void MessageService::doublicateDateEdit(UserSession& session) {
    spdlog::debug("Dublicate date. Edit");
    editMessage(session,
                {"duplicate_date_edit", {session.body_metrics_draft.date}});
    session.current_state = UserStates::InputDate;
}

// ---- request_*/invalid_* для ОСНОВНЫХ метрик -------------------------------
template <class T>
void MessageService::requestMetric(UserSession& session,
                                   std::string_view metric_label,
                                   std::string_view op_label,
                                   T BodyMetrics::* field,
                                   const std::string& base_key,
                                   UserStates state) {
    spdlog::debug("{} {}", op_label, metric_label);
    if (session.body_metrics_draft.*field > 0.) {
        spdlog::debug("Edit version");
        editMessage(session,
                    {base_key + "_edit", {session.body_metrics_draft.*field}});
    } else if (session.last_body_metrics) {
        spdlog::debug("Previous version");
        editMessage(session, {base_key + "_previous",
                              {session.last_body_metrics.value().*field},
                              {{session.last_body_metrics.value().*field}}});
    } else {
        spdlog::debug("Default version");
        editMessage(session, {base_key});
    }

    session.current_state = state;
}

void MessageService::requestWeight(UserSession& session) {
    requestMetric(session, "weight", "Request", &BodyMetrics::weight,
                  "request_weight", UserStates::InputWeight);
}

void MessageService::requestHeight(UserSession& session) {
    requestMetric(session, "height", "Request", &BodyMetrics::height,
                  "request_height", UserStates::InputHeight);
}

void MessageService::requestMuscleMass(UserSession& session) {
    requestMetric(session, "muscle_mass", "Request", &BodyMetrics::muscle_mass,
                  "request_muscle_mass", UserStates::InputMuscleMass);
}

void MessageService::requestFatMass(UserSession& session) {
    requestMetric(session, "fat_mass", "Request", &BodyMetrics::fat_mass,
                  "request_fat_mass", UserStates::InputFatMass);
}

void MessageService::invalidWeight(UserSession& session) {
    requestMetric(session, "weight", "Invalid", &BodyMetrics::weight,
                  "invalid_weight", UserStates::InputWeight);
}

void MessageService::invalidHeight(UserSession& session) {
    requestMetric(session, "height", "Invalid", &BodyMetrics::height,
                  "invalid_height", UserStates::InputHeight);
}

void MessageService::invalidMuscleMass(UserSession& session) {
    requestMetric(session, "muscle_mass", "Invalid", &BodyMetrics::muscle_mass,
                  "invalid_muscle_mass", UserStates::InputMuscleMass);
}

void MessageService::invalidFatMass(UserSession& session) {
    requestMetric(session, "fat_mass", "Invalid", &BodyMetrics::fat_mass,
                  "invalid_fat_mass", UserStates::InputFatMass);
}
// ----------------------------------------------------------------------------

// dev
void MessageService::printSummary(UserSession& session) {
    editMessage(session, {"save_metric",
                          {session.body_metrics_draft.date,
                           session.body_metrics_draft.weight,
                           session.body_metrics_draft.height,
                           session.body_metrics_draft.muscle_mass,
                           session.body_metrics_draft.fat_mass}});
    session.current_state = UserStates::MetricSummary;
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

// Универсальный хелпер для request_*/invalid_* простых (несегментных) ДОП. метрик
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
        editMessage(session, {base_message_key + "_edit",
                              {segmentMassFormat(session.segment_mass_draft)}});
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

void MessageService::addRecord(UserSession& session) {
    spdlog::debug("Add record");
    editMessage(session, {"add_record"});
    session.current_state = UserStates::AddMetricSelectMode;
}

void MessageService::registration(UserSession& session) {
    spdlog::debug("Registration");
    editMessage(session, {"registration"});
    session.current_state = UserStates::RegistrationNeed;
}

void MessageService::requestName(UserSession& session) {
    spdlog::debug("Request name");
    session.current_state = UserStates::InputUserName;
    editMessage(session,
                {"request_name", {}, {{session.user_data->user_name}}});
}

void MessageService::requestAge(UserSession& session) {
    spdlog::debug("Request age");
    editMessage(session, {"request_age"});
    session.current_state = UserStates::InputUserAge;
}

void MessageService::requestSex(UserSession& session) {
    spdlog::debug("Request sex");
    editMessage(session, {"request_gender"});
    session.current_state = UserStates::InputUserSex;
}

void MessageService::historyScreen(UserSession& session) {
    static constexpr size_t kPageSize = 10;
    spdlog::debug("History print");
    session.current_state = UserStates::HistoryPage;

    // ---- Определение шаблонов кнопок ---------------------------------------
    static const auto text_message_temp =
        message_loader_.getMessage("history_screen")["text"].get<std::string>();

    static const auto metric_btn =
        message_loader_.getMessage("history_screen")["buttons"]["metric"];
    static const auto metric_text = metric_btn["text"].get<std::string>();
    static const auto metric_data = metric_btn["callback"].get<std::string>();

    static const auto main_menu_btn =
        message_loader_.getMessage("history_screen")["buttons"]["main_menu"];
    static const auto main_menu_text = main_menu_btn["text"].get<std::string>();
    static const auto main_menu_data =
        main_menu_btn["callback"].get<std::string>();

    static const auto previous_btn =
        message_loader_.getMessage("history_screen")["buttons"]["previous"];
    static const auto previous_text = previous_btn["text"].get<std::string>();
    static const auto previous_data =
        previous_btn["callback"].get<std::string>();

    static const auto next_btn =
        message_loader_.getMessage("history_screen")["buttons"]["next"];
    static const auto next_text = next_btn["text"].get<std::string>();
    static const auto next_data = next_btn["callback"].get<std::string>();

    static const auto count_btn =
        message_loader_.getMessage("history_screen")["buttons"]["count"];
    static const auto count_text = count_btn["text"].get<std::string>();
    static const auto count_data = count_btn["callback"].get<std::string>();
    // ------------------------------------------------------------------------

    size_t metrics_count =
        database_.getBodyMetricsCount(session.user_data->user_id);

    // Если записей нет
    if (metrics_count == 0) {
        spdlog::debug("Empty history");
        editMessage(session, {"history_empty"});
        return;
    }

    std::string text_message = format(text_message_temp, {metrics_count});
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

    auto metrics = database_.getBodyMetricsHistory(
        session.user_data->user_id, session.history_offset * kPageSize);
    for (const auto& metric : metrics) {
        auto button_ptr = std::make_shared<TgBot::InlineKeyboardButton>();
        button_ptr->text = format(metric_text, {metric.date});
        button_ptr->callbackData = format(metric_data, {metric.date});
        keyboard->inlineKeyboard.push_back({button_ptr});
    }

    size_t total_page = ceil_div<size_t>(metrics_count, kPageSize);
    if (total_page > 1) {
        // если открыта 1 страницы
        std::vector<TgBot::InlineKeyboardButton::Ptr> buttons_row;

        if (session.history_offset > 0) {
            auto btn = std::make_shared<TgBot::InlineKeyboardButton>();
            btn->text = previous_text;
            btn->callbackData = previous_data;
            buttons_row.push_back(btn);
        }

        auto count_btn_ptr = std::make_shared<TgBot::InlineKeyboardButton>();
        count_btn_ptr->text =
            format(count_text, {session.history_offset + 1, total_page});
        count_btn_ptr->callbackData = count_data;
        buttons_row.push_back(count_btn_ptr);

        if (session.history_offset + 1 < total_page) {
            auto btn = std::make_shared<TgBot::InlineKeyboardButton>();
            btn->text = next_text;
            btn->callbackData = next_data;
            buttons_row.push_back(btn);
        }

        keyboard->inlineKeyboard.push_back(buttons_row);
    }

    auto button_ptr = std::make_shared<TgBot::InlineKeyboardButton>();
    button_ptr->text = main_menu_text;
    button_ptr->callbackData = main_menu_data;
    keyboard->inlineKeyboard.push_back({button_ptr});

    editMessage(session, text_message, keyboard);
}

void MessageService::editMessage(
    UserSession& session, const std::string& text,
    const std::shared_ptr<TgBot::InlineKeyboardMarkup>& keyboard) {
    spdlog::debug("Edit message: Chat ID {}, Message ID {}.", session.chat_id,
                  session.last_message_id);
    session.last_message_template = {};
    if (session.last_message_id == 0) {
        spdlog::debug("Cannot edit message with ID 0. Send new");
        sendMessage(session, {}, MessageData{text, keyboard});
        return;
    }
    try {
        bot_.getApi().editMessageText(text, session.chat_id,
                                      session.last_message_id, "", "", nullptr,
                                      keyboard);
        spdlog::debug("Edited successfully");
    } catch (const TgBot::TgException& e) {
        std::string_view error_message = e.what();
        if (error_message.find("message is not modified") !=
            std::string::npos) {
            spdlog::debug("Message is not modified");
            return;
        }

        spdlog::debug("Edited failed. What: {}. Try send", e.what());
        sendMessage(session, {}, MessageData{text, keyboard});
    }
}