#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "bot/user-states.hpp"
#include "database/database.hpp"
#include "models/body-metrics.hpp"
#include "models/user-data.hpp"

struct UserSession {
    UserStates currentState = UserStates::MainMenu;
    std::optional<UserData> userData;
    std::optional<BodyMetrics> lastBodyMetrics;

    int64_t chatId;
    int32_t lastMessageId;
};

class UserSessionManager {
  public:
    UserSessionManager(Database& db) : database(db) {}

    UserSession& getSession(int64_t userId);
    void removeSession(int64_t userId);

  private:
    std::unordered_map<int64_t, UserSession> sessions;
    Database& database;
};
