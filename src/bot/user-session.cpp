#include "bot/user-session.hpp"
#include <cstdint>
#include <tgbot/types/Message.h>

UserSession& UserSessionManager::get_session(int64_t user_id) {
    auto [it, inserted] = sessions_.try_emplace(user_id);
    if (inserted) {
        it->second.user_data = database_.getUserData(user_id);
    }

    return it->second;
}

void UserSessionManager::removeSession(int64_t user_id) {
    sessions_.erase(user_id);
}

UserSession& UserSessionManager::get_session(const TgBot::Message::Ptr& message) {
    int64_t user_id = message->from->id;
    UserSession& session = get_session(user_id);

    if (!session.user_data) {
        session.current_state = UserStates::RegistrationNeed;
        session.user_data.emplace();
        session.user_data->user_id = message->from->id;
    }

    session.chat_id = message->chat->id;

    return session;
}