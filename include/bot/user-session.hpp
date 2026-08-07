#pragma once

#include "database/database.hpp"
#include "models/body-metrics.hpp"
#include "models/user-data.hpp"
#include "types/input-mode.hpp"
#include "types/message-template.hpp"
#include "types/user-states.hpp"

#include <cstdint>
#include <optional>
#include <unordered_map>

#include <tgbot/tgbot.h>
#include <tgbot/types/CallbackQuery.h>

struct UserSession {
    UserStates current_state = UserStates::MainMenu;

    // пользовательские данные, инциализация при создании
    std::optional<UserData> user_data;

    // Последнее измерение, для подсказок и меню
    // Инциализация:
    // 1. callback_handler: при выборе ручного режима (из database)
    // 2. message_service: при открытии главного меню (из databse)
    std::optional<BodyMetrics> last_body_metrics;

    // Промежуточный результат записи измерения.
    // Инциализация:
    // 1. message_service: при открытии главного меню {}
    // 2. callback_handler: при выборе ручного режима создания записи {user_id, age}
    BodyMetrics body_metrics_draft;

    // Промежуточный результат изменения сегментной массы
    // Инициализация:
    // 1. message_service: при открытии главного меню и меню выбора ДОП метрики {}
    // 2. message_handler: при вводе данных {data} || {null}
    // 3. callback_handler: при выборе сегментных мышц/жира {body_metrics_draft.segment_*}
    std::optional<SegmentMass> segment_mass_draft;

    InputMode input_mode = InputMode::Creating;

    int64_t chat_id = 0;
    int32_t last_message_id = 0;
    MessageTemplate last_message_template;
    int history_offset = 0;
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
