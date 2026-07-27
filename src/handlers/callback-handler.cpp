#include "handlers/callback-handler.hpp"

#include "bot/user-session.hpp"
#include "types/user-states.hpp"

#include <spdlog/spdlog.h>
#include <tgbot/tgbot.h>
#include <tgbot/types/CallbackQuery.h>

void CallbackHandler::handleCallback(const TgBot::CallbackQuery::Ptr& query) {
    if (query->data.starts_with("reg:")) {
        onRegistration(query);
    }
}

void CallbackHandler::onRegistration(const TgBot::CallbackQuery::Ptr& query) {
    UserSession& session = session_manager_.getSession(query);
    if (query->data.ends_with("start")) {
        spdlog::debug("Start registration");
        session.current_state = UserStates::InputUserName;
        message_service_.editMessage(
            session, {"request_name", {}, {{query->from->firstName}}});
    } else if (query->data.ends_with("default_name")) {
        spdlog::debug("Select default name: {}", query->from->firstName);
        if (session.user_data->set_name(query->from->firstName)) {
            spdlog::debug("Set default name: {}", session.user_data->user_name);
            session.current_state = UserStates::InputUserAge;
            message_service_.editMessage(session, {"request_age"});
        } else {
            spdlog::debug("Incorrect tg name");
            message_service_.editMessage(
                session,
                {"invalid_name", {}, {{"Имя по умолчанию недоступно"}}});
        }
    } else if (query->data.ends_with("male")) {
        spdlog::debug("Select gender: Male");
        session.user_data->sex = Sex::Male;
        database_.addUser(session.user_data->user_id,
                          session.user_data.value());
        session.current_state = UserStates::MainMenu;
        message_service_.editMessage(session, {"empty_menu"});
    } else if (query->data.ends_with("female")) {
        spdlog::debug("Select gender: Female");
        session.user_data->sex = Sex::Female;
        database_.addUser(session.user_data->user_id,
                          session.user_data.value());
        session.current_state = UserStates::MainMenu;
        message_service_.editMessage(session, {"empty_menu"});
    }
}