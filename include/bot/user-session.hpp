#pragma once

#include <cstdint>
#include <optional>
#include <tgbot/tgbot.h>
#include <unordered_map>

#include "bot/user-states.hpp"
#include "database/database.hpp"
#include "models/body-metrics.hpp"
#include "models/user-data.hpp"

struct UserSession {
    UserStates currentState = UserStates::MainMenu;
    std::optional<UserData> userData;
    std::optional<BodyMetrics> lastBodyMetrics;

    int64_t chatId = 0;
    int32_t lastMessageId = 0;
};

class UserSessionManager {
  public:
    UserSessionManager(Database& db) : database(db) {}

    UserSession& getSession(int64_t userId);
    UserSession& get_session(const TgBot::Message::Ptr& message);
    void removeSession(int64_t userId);

  private:
    std::unordered_map<int64_t, UserSession> sessions;
    Database& database;
};
