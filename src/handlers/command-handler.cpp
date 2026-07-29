#include "handlers/command-handler.hpp"

#include "bot/user-session.hpp"
#include "types/user-states.hpp"

#include <spdlog/spdlog.h>
#include <tgbot/types/Message.h>

void CommandHandler::onStart(const TgBot::Message::Ptr& message) {
    UserSession& session = session_manager_.getSession(message);
    spdlog::debug("Start command handler. User {}", session.user_data->user_id);

    message_service_.deleteMessage(message);
    // если регистрация не пройдена, перезапускаем ее
    if (session.current_state >= UserStates::RegistrationNeed &&
        UserStates::InputUserSex >= session.current_state) {
        session.current_state = UserStates::RegistrationNeed;
        message_service_.editMessage(session, {"registration"});
    } else {
        message_service_.openMainMenu(session);
    }
}