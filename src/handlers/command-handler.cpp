#include "handlers/command-handler.hpp"

#include "bot/user-session.hpp"
#include "types/user-states.hpp"

#include <string>

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
    }
    // dev. используем версию главного меню без значений
    else {
        session.current_state = UserStates::MainMenu;
        session.last_body_metrics =
            database_.getLastBodyMetrics(session.user_data->user_id);
        if (session.last_body_metrics) {
            spdlog::debug("Open main menu");
            auto& current_metrics = session.last_body_metrics;
            message_service_.editMessage(
                session, {"main_menu",
                          {
                              session.user_data->user_name,
                              std::to_string(current_metrics->weight),
                              std::to_string(current_metrics->fatMass),
                              std::to_string(current_metrics->muscleMass),
                              current_metrics->date,
                          }});
        } else {
            spdlog::debug("Metrics not found. Open empty main menu");
            message_service_.editMessage(session, {"empty_menu"});
        }
    }
}