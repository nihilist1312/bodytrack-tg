#pragma once

#include "bot/user-session.hpp"
#include "database/database.hpp"
#include "message-loader.hpp"
#include <tgbot/tgbot.h>

class CallbackHandler {
  public:
    CallbackHandler(TgBot::Bot& bot, Database& database, UserSessionManager& sessionManager,
                    MessageLoader& messageLoader)
        : bot_(bot), database_(database), sessionManager_(sessionManager),
          messageLoader_(messageLoader) {}

    void handleCallback(const TgBot::CallbackQuery::Ptr& query);

  private:
    TgBot::Bot& bot_;
    Database& database_;
    UserSessionManager& sessionManager_;
    MessageLoader& messageLoader_;

    void registrationCallbackHandler(const TgBot::CallbackQuery::Ptr& query);
    void mainMenuCallbackHandler(const TgBot::CallbackQuery::Ptr& query);
    void selectModeCallbackHandler(const TgBot::CallbackQuery::Ptr& query);
    // go to main menu from any state
    void cancelCallbackHandler(const TgBot::CallbackQuery::Ptr& query);
    void backCallbackHandler(const TgBot::CallbackQuery::Ptr& query);
    void metricCallbackHandler(const TgBot::CallbackQuery::Ptr& query);
    void optionalMetricsCallbackHandler(const TgBot::CallbackQuery::Ptr& query);
    void metricsSummaryCallbackHandler(const TgBot::CallbackQuery::Ptr& query);

    // utils
    // редактирует сообщение вставляя в текст строки из text_replace, а в кнопки из button_replace,
    // где ключ это индекс кнопки
    void editTextAndKeyboard(
        const std::string& key, int64_t chat_id, int32_t id,
        const std::vector<std::string>& text_replace = {},
        const std::unordered_map<size_t, std::vector<std::string>>& button_replace = {});
    void deleteMessage(const TgBot::Message::Ptr& message);
};