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
    UserStates current_state = UserStates::MainMenu;
    std::optional<UserData> user_data;
    std::optional<BodyMetrics> last_body_metrics;

    int64_t chat_id = 0;
    int32_t last_message_id = 0;
};

class UserSessionManager {
  public:
    explicit UserSessionManager(Database& db) : database_(db) {}

    UserSession& get_session(int64_t user_id);
    UserSession& get_session(const TgBot::Message::Ptr& message);
    void removeSession(int64_t user_id);

  private:
    std::unordered_map<int64_t, UserSession> sessions_;
    Database& database_;
};
