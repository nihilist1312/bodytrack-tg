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

    switch (session.current_state) {
    case UserStates::InputUserName:
        onName(message, session);
        break;
    case UserStates::InputUserAge:
        onAge(message, session);
        break;
    default:
        defaultHandler(message, session);
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
    message_service_.deleteMessage(message);
    if (session.user_data->set_name(message->text)) {
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
    spdlog::debug("Ade handle");
    message_service_.deleteMessage(message);
    if (session.user_data->set_age(message->text)) {
        spdlog::debug("Set user age: {}", session.user_data->age);
        message_service_.editMessage(session, {"request_gender"});
        session.current_state = UserStates::InputUserSex;
    } else {
        spdlog::debug("Incorrect age");
        message_service_.editMessage(session, {"invalid_age"});
    }
}

void MessageHandler::defaultHandler(const TgBot::Message::Ptr& message,
                                    UserSession& session) {
    message_service_.deleteMessage(message);
    // если регистрация не пройдена, перезапускаем ее
    // if (session.current_state >= UserStates::RegistrationNeed &&
    //     UserStates::InputUserSex >= session.current_state) {
    //     session.current_state = UserStates::RegistrationNeed;
    //     message_service_.editMessage(session, {"registration"});
    // }
    // // dev. используем версию главного меню без значений
    // else {
    //     session.current_state = UserStates::MainMenu;
    //     message_service_.editMessage(session, {"empty_menu"});
    // }
}