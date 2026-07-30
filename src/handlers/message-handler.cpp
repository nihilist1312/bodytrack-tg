#include "handlers/message-handler.hpp"

#include "bot/user-session.hpp"
#include "types/user-states.hpp"

#include <string>

#include <spdlog/spdlog.h>
#include <tgbot/tgbot.h>
#include <tgbot/types/Message.h>
// dev
void MessageHandler::handleFileMessage(const TgBot::Message::Ptr& message) {
    UserSession& session = user_session_manager_.getSession(message);

    message_service_.deleteMessage(message);
}

void MessageHandler::handleTextMessage(const TgBot::Message::Ptr& message) {
    spdlog::debug("handle text message");
    UserSession& session = user_session_manager_.getSession(message);
    message_service_.deleteMessage(message);

    switch (session.current_state) {
    case UserStates::InputUserName:
        onName(message, session);
        break;
    case UserStates::InputUserAge:
        onAge(message, session);
        break;
    case UserStates::InputDate:
        onDate(message, session);
        break;
    case UserStates::InputWeight:
        onWeight(message, session);
        break;
    case UserStates::InputHeight:
        onHeight(message, session);
        break;
    case UserStates::InputMuscleMass:
        onMuscleMass(message, session);
        break;
    case UserStates::InputFatMass:
        onFatMass(message, session);
        break;
    default:
        spdlog::debug("Default handler");
        break;
    }
}

void MessageHandler::handleMessage(const TgBot::Message::Ptr& message) {
    if (message->document) {
        handleFileMessage(message);
    } else if (!message->text.empty()) {
        if (message->text[0] != '/')
            handleTextMessage(message);
    } else {
        spdlog::debug("Unsupported message type. Delete");
        message_service_.deleteMessage(message);
    }
}

void MessageHandler::onName(const TgBot::Message::Ptr& message,
                            UserSession& session) {
    spdlog::debug("Name handle");
    if (set_name(session.user_data.value(), message->text)) {
        spdlog::debug("Set user name: {}", session.user_data->user_name);
        message_service_.editMessage(session, {"request_age"});
        session.current_state = UserStates::InputUserAge;
    } else {
        spdlog::debug("Incorrect name");
        message_service_.editMessage(
            session, {"invalid_name", {}, {{message->from->firstName}}});
    }
}

void MessageHandler::onAge(const TgBot::Message::Ptr& message,
                           UserSession& session) {
    spdlog::debug("Age handle");
    if (set_age(session.user_data.value(), message->text)) {
        spdlog::debug("Set user age: {}", session.user_data->age);
        message_service_.editMessage(session, {"request_gender"});
        session.current_state = UserStates::InputUserSex;
    } else {
        spdlog::debug("Incorrect age");
        message_service_.editMessage(session, {"invalid_age"});
    }
}

void MessageHandler::onDate(const TgBot::Message::Ptr& message,
                            UserSession& session) {
    spdlog::debug("Date handle");
    if (setDate(session.new_body_metrics, message->text)) {
        // если запись с такой датой уже существует
        if (database_.getBodyMetricsByUserIdAndDate(
                session.user_data->user_id, session.new_body_metrics.date)) {
            message_service_.doublicateDate(session);
            return;
        }
        spdlog::debug("Set date: {}", session.new_body_metrics.date);
        message_service_.requestWeight(session);
    } else {
        message_service_.invalidDate(session);
    }
}

void MessageHandler::onWeight(const TgBot::Message::Ptr& message,
                              UserSession& session) {
    spdlog::debug("Weight handle");
    if (setMass(session.new_body_metrics.weight, message->text)) {
        spdlog::debug("Set weight: {}", session.new_body_metrics.weight);
        message_service_.requestHeight(session);
    } else {
        message_service_.invalidWeight(session);
    }
}

void MessageHandler::onHeight(const TgBot::Message::Ptr& message,
                              UserSession& session) {
    spdlog::debug("Height handle");
    if (setHeight(session.new_body_metrics, message->text)) {
        spdlog::debug("Set height: {}", session.new_body_metrics.height);
        message_service_.requestMuscleMass(session);
    } else {
        message_service_.invalidHeight(session);
    }
}

void MessageHandler::onMuscleMass(const TgBot::Message::Ptr& message,
                                  UserSession& session) {
    spdlog::debug("Muscle Mass handle");
    if (setMass(session.new_body_metrics.muscle_mass, message->text)) {
        spdlog::debug("Set muscle mass: {}",
                      session.new_body_metrics.muscle_mass);
        message_service_.requestFatMass(session);
    } else {
        message_service_.invalidMuscleMass(session);
    }
}

void MessageHandler::onFatMass(const TgBot::Message::Ptr& message,
                               UserSession& session) {
    spdlog::debug("Fat mass handle");
    if (setMass(session.new_body_metrics.fat_mass, message->text)) {
        spdlog::debug("Set fat mass: {}", session.new_body_metrics.fat_mass);
        message_service_.printSummary(session);
    } else {
        message_service_.invalidFatMass(session);
    }
}