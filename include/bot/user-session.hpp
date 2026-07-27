#pragma once

#include "database/database.hpp"
#include "models/body-metrics.hpp"
#include "models/user-data.hpp"
#include "types/message-template.hpp"
#include "types/user-states.hpp"

#include <cstdint>
#include <optional>
#include <unordered_map>

#include <tgbot/tgbot.h>
#include <tgbot/types/CallbackQuery.h>

struct UserSession {
    UserStates current_state = UserStates::MainMenu;
    std::optional<UserData> user_data;
    std::optional<BodyMetrics> last_body_metrics;

    int64_t chat_id = 0;
    int32_t last_message_id = 0;
    MessageTemplate last_message_template;
};

class UserSessionManager {
  public:
    explicit UserSessionManager(Database& db) : database_(db) {}

    UserSession& getSession(const TgBot::Message::Ptr& message);
    UserSession& getSession(const TgBot::CallbackQuery::Ptr& query);
    void removeSession(int64_t user_id);

  private:
    std::unordered_map<int64_t, UserSession> sessions_;
    Database& database_;

    UserSession& getSession(int64_t user_id);
};
