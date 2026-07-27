#include "handlers/message-handler.hpp"

#include "bot/user-session.hpp"
#include "bot/user-states.hpp"

#include <string>

#include <spdlog/spdlog.h>
#include <tgbot/tgbot.h>
#include <tgbot/types/Message.h>
// dev
void MessageHandler::handleFileMessage(const TgBot::Message::Ptr& message) {
    UserSession& session = userSessionManager_.getSession(message);

    message_service_.deleteMessage(message);
}

void MessageHandler::handleTextMessage(const TgBot::Message::Ptr& message) {
    spdlog::debug("handle text message");
    UserSession& session = userSessionManager_.getSession(message);

    switch (session.current_state) {
    case UserStates::InputUserName:
        onName(message);
        break;
    case UserStates::InputUserAge:
        onAge(message);
        break;
    default:
        message_service_.deleteMessage(message);
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

void MessageHandler::onName(const TgBot::Message::Ptr& message) {
    spdlog::debug("Name handle");
    UserSession& session = userSessionManager_.getSession(message);
    if (session.user_data->set_name(message->text)) {
        spdlog::debug("Set user name: {}", session.user_data->user_name);
        message_service_.editMessage(session, {"request_age"});
    } else {
        spdlog::debug("Incorrect name");
        message_service_.editMessage(session, {"invalid_name"});
    }
}

void MessageHandler::onAge(const TgBot::Message::Ptr& message) {
    spdlog::debug("Ade handle");
    UserSession& session = userSessionManager_.getSession(message);
    if (session.user_data->set_age(message->text)) {
        spdlog::debug("Set user age: {}", session.user_data->age);
        message_service_.editMessage(session, {"request_gender"});
    } else {
        spdlog::debug("Incorrect age");
        message_service_.editMessage(session, {"invalid_age"});
    }
}