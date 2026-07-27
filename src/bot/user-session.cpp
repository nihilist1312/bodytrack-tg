#include "bot/user-session.hpp"

#include <cstdint>

#include <spdlog/spdlog.h>
#include <tgbot/tgbot.h>

UserSession& UserSessionManager::getSession(int64_t user_id) {
    spdlog::debug("Get session for user ID {}", user_id);
    auto [it, inserted] = sessions_.try_emplace(user_id);
    if (inserted) {
        spdlog::debug("User session not found. Create");
        it->second.user_data = database_.getUserData(user_id);
        spdlog::info("Active sessions: {}", sessions_.size());
    }

    return it->second;
}

void UserSessionManager::removeSession(int64_t user_id) {
    spdlog::debug("Delete session user {}", user_id);
    sessions_.erase(user_id);
    spdlog::info("Active sessions: {}", sessions_.size());
}

UserSession&
UserSessionManager::getSession(const TgBot::Message::Ptr& message) {
    int64_t user_id = message->from->id;
    UserSession& session = getSession(user_id);

    if (!session.user_data) {
        spdlog::info("User {} is not exist. Registration", user_id);
        session.current_state = UserStates::RegistrationNeed;
        session.user_data.emplace();
        session.user_data->user_id = message->from->id;
    }

    session.chat_id = message->chat->id;

    return session;
}

UserSession&
UserSessionManager::getSession(const TgBot::CallbackQuery::Ptr& query) {
    int64_t user_id = query->from->id;
    UserSession& session = getSession(user_id);

    if (!session.user_data) {
        spdlog::info("User {} is not exist. Registration", user_id);
        session.current_state = UserStates::RegistrationNeed;
        session.user_data.emplace();
        session.user_data->user_id = query->from->id;
    }

    session.chat_id = query->message->chat->id;

    return session;
}