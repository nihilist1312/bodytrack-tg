#pragma once

#include "bot/user-session.hpp"
#include "database/database.hpp"
#include "message-loader.hpp"
#include <tgbot/tgbot.h>

class CommandHandler {
  public:
    CommandHandler(TgBot::Bot& bot, Database& database, UserSessionManager& userSessionManager,
                   MessageLoader& messageLoader)
        : bot_(bot), database_(database), userSessionManager_(userSessionManager),
          messageLoader_(messageLoader) {}

    // void handleCommand(const TgBot::Message::Ptr& message);
    void handleStartCommand(const TgBot::Message::Ptr& message);
    void handleHelpCommand(const TgBot::Message::Ptr& message);
    void handleAddCommand(const TgBot::Message::Ptr& message);
    void handleStatsCommand(const TgBot::Message::Ptr& message);
    void handleHistoryCommand(const TgBot::Message::Ptr& message);
    void handleGoalCommand(const TgBot::Message::Ptr& message);
    void handleSettingsCommand(const TgBot::Message::Ptr& message);

  private:
    TgBot::Bot& bot_;
    Database& database_;
    UserSessionManager& userSessionManager_;
    MessageLoader& messageLoader_;

    // UserSession& get_session(const TgBot::Message::Ptr& message);
    TgBot::Message::Ptr sendTextAndKeyboard(
        const std::string& key, UserSession& session,
        const std::vector<std::string>& text_replace = {},
        const std::unordered_map<size_t, std::vector<std::string>>& button_replace = {});
    void deleteMessage(const TgBot::Message::Ptr& message);
};
