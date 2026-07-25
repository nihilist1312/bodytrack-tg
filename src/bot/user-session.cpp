#include "bot/user-session.hpp"
#include <tgbot/types/Message.h>

UserSession& UserSessionManager::getSession(int64_t userId) {
    auto it = sessions.find(userId);
    if (it != sessions.end()) {
        return it->second;
    } else {
        // Create a new session for the user if it doesn't exist
        UserSession newSession;
        newSession.userData = database.getUserData(userId);
        sessions[userId] = newSession;
        return sessions[userId];
    }
}

void UserSessionManager::removeSession(int64_t userId) {
    sessions.erase(userId);
}

UserSession& UserSessionManager::get_session(const TgBot::Message::Ptr& message) {
    int64_t user_id = message->from->id;
    UserSession& session = getSession(user_id);

    if (!session.userData.has_value()) {
        session.currentState = UserStates::RegisrationNeed;
        session.userData = UserData{};
    }

    if (session.chatId == 0)
        session.chatId = message->chat->id;

    return session;
}