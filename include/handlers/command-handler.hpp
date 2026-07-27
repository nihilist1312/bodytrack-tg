#pragma once

#include "bot/message-service.hpp"
#include "bot/user-session.hpp"
#include "database/database.hpp"

#include <tgbot/tgbot.h>

class CommandHandler {
  public:
    CommandHandler(Database& database, UserSessionManager& session_manager,
                   MessageService& message_service)
        : database_(database), session_manager_(session_manager),
          message_service_(message_service) {}

    void onStart(const TgBot::Message::Ptr& message);
    void onHelp(const TgBot::Message::Ptr& message);
    void onAdd(const TgBot::Message::Ptr& message);
    void onStats(const TgBot::Message::Ptr& message);
    void onHistory(const TgBot::Message::Ptr& message);
    void onGoal(const TgBot::Message::Ptr& message);
    void onSettings(const TgBot::Message::Ptr& message);

  private:
    Database& database_;
    UserSessionManager& session_manager_;
    MessageService& message_service_;
};
