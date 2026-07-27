#include "handlers/command-handler.hpp"

#include "bot/user-session.hpp"
#include "types/user-states.hpp"

#include <spdlog/spdlog.h>
#include <tgbot/types/Message.h>

void CommandHandler::onStart(const TgBot::Message::Ptr& message) {
    spdlog::debug("Start command handler");
    UserSession& session = session_manager_.getSession(message);
    message_service_.deleteMessage(message);
    // если регистрация не пройдена, перезапускаем ее
    if (session.current_state >= UserStates::RegistrationNeed &&
        UserStates::InputUserSex >= session.current_state) {
        session.current_state = UserStates::RegistrationNeed;
        message_service_.editMessage(session, {"registration"});
    }
    // dev. используем версию главного меню без значений
    else {
        session.current_state = UserStates::MainMenu;
        message_service_.editMessage(session, {"empty_menu"});
    }
}