#include "bot/message-service.hpp"

#include "bot/user-session.hpp"
#include "types/message-template.hpp"
#include "types/user-states.hpp"
#include "utils/date.hpp"
#include "utils/placeholders.hpp"

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
        format_inplace(data.text, message_template.text_placeholders);

    // get buttons from json
    auto button_rows = message_json["buttons"];
    data.keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
    size_t idx_btn = 0;
    for (const auto& row : button_rows) {
        std::vector<TgBot::InlineKeyboardButton::Ptr> buttons_row;

        for (const auto& button_json : row) {
            auto button_ptr = std::make_shared<TgBot::InlineKeyboardButton>();
            std::string btn_text = button_json["text"];
            if (idx_btn < message_template.buttons_placeholders.size() &&
                !message_template.buttons_placeholders[idx_btn].empty()) {
                format_inplace(btn_text,
                               message_template.buttons_placeholders[idx_btn]);
            }
            button_ptr->text = std::move(btn_text);
            button_ptr->callbackData = button_json["callback"];
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
                                  current_metrics->fat_mass,
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
    editMessage(
        session,
        {"save_metric",
         {session.new_body_metrics.date, session.new_body_metrics.weight,
          session.new_body_metrics.height, session.new_body_metrics.muscle_mass,
          session.new_body_metrics.fat_mass}});
    session.current_state = UserStates::MetricSummary;
}

void MessageService::invalidWeight(UserSession& session) {
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
    editMessage(session, {"invalid_date", {}, {{getCurrentDate()}}});
    session.current_state = UserStates::InputDate;
}