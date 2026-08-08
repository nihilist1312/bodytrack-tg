#pragma once

#include "bot/message-service.hpp"
#include "bot/user-session.hpp"
#include "database/database.hpp"

#include <tgbot/tgbot.h>
#include <tgbot/types/CallbackQuery.h>

class CallbackHandler {
  public:
    CallbackHandler(Database& database, UserSessionManager& session_manager,
                    MessageService& message_service)
        : database_(database), session_manager_(session_manager),
          message_service_(message_service) {}

    void handleCallback(const TgBot::CallbackQuery::Ptr& query);

  private:
    Database& database_;
    UserSessionManager& session_manager_;
    MessageService& message_service_;

    void onRegistration(const TgBot::CallbackQuery::Ptr& query);
    void onMainMenu(const TgBot::CallbackQuery::Ptr& query);
    void onAddMetric(const TgBot::CallbackQuery::Ptr& query);
    void onHistory(const TgBot::CallbackQuery::Ptr& query);
    // go to main menu from any state
    void onCancel(const TgBot::CallbackQuery::Ptr& query);
    void onBack(const TgBot::CallbackQuery::Ptr& query);
    void metricCallbackHandler(const TgBot::CallbackQuery::Ptr& query);
    void onOptionalMetric(const TgBot::CallbackQuery::Ptr& query);
    void onSummaryMetric(const TgBot::CallbackQuery::Ptr& query);
};