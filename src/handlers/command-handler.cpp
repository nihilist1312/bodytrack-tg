#include "handlers/command-handler.hpp"

#include "bot/user-session.hpp"
#include "types/user-states.hpp"

#include <spdlog/spdlog.h>
#include <tgbot/types/Message.h>

void CommandHandler::onStart(const TgBot::Message::Ptr& message) {
    UserSession& session = session_manager_.getSession(message);
    spdlog::debug("/start command handler.");

    message_service_.deleteMessage(message);
    // если регистрация не пройдена, перезапускаем ее
    if (session.current_state >= UserStates::RegistrationNeed &&
        UserStates::InputUserSex >= session.current_state) {
        message_service_.registration(session);
    } else {
        message_service_.openMainMenu(session);
    }
}

void CommandHandler::onAdd(const TgBot::Message::Ptr& message) {
    UserSession& session = session_manager_.getSession(message);
    spdlog::debug("/add command handle.");

    message_service_.deleteMessage(message);

    message_service_.addRecord(session);
}

void CommandHandler::onHistory(const TgBot::Message::Ptr& message) {
    UserSession& session = session_manager_.getSession(message);
    spdlog::debug("/history command handle");

    message_service_.deleteMessage(message);
    message_service_.historyScreen(session);
}