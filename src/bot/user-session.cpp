#include "bot/user-session.hpp"

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