#pragma once

#include "bot/user-session.hpp"
#include "database/database.hpp"
#include "message-loader.hpp"
#include <cstddef>
#include <tgbot/tgbot.h>
#include <tgbot/types/Message.h>

#include <unordered_map>
#include <vector>

class MessageHandler {
  public:
    MessageHandler(TgBot::Bot& bot, Database& database, UserSessionManager& userSessionManager,
                   MessageLoader& messageLoader)
        : bot_(bot), database_(database), userSessionManager_(userSessionManager),
          messageLoader_(messageLoader) {}

    void handleMessage(const TgBot::Message::Ptr& message);

  private:
    TgBot::Bot& bot_;
    Database& database_;
    UserSessionManager& userSessionManager_;
    MessageLoader& messageLoader_;

    void handleTextMessage(const TgBot::Message::Ptr& message);
    void handleFileMessage(const TgBot::Message::Ptr& message);

    // registration
    void registerHandler(const TgBot::Message::Ptr& message);
    void userNameHandler(const TgBot::Message::Ptr& message);
    void userAgeHandler(const TgBot::Message::Ptr& message);
    void userSexHandler(const TgBot::Message::Ptr& message);

    // input metrics
    void weightHandler(const TgBot::Message::Ptr& message);
    void heightHandler(const TgBot::Message::Ptr& message);
    void muscleMassHandler(const TgBot::Message::Ptr& message);
    void bodyFatMassHandler(const TgBot::Message::Ptr& message);

    // input optional metrics
    void waterMassHandler(const TgBot::Message::Ptr& message);
    void boneMassHandler(const TgBot::Message::Ptr& message);
    void visceralFatHandler(const TgBot::Message::Ptr& message);
    void proteinMassHandler(const TgBot::Message::Ptr& message);

    // ввод в формате leftArm,rightArm,leftLeg,rightLeg,trunk (dev)
    void muscleMassSegmentsHandler(const TgBot::Message::Ptr& message);
    void fatMassSegmentsHandler(const TgBot::Message::Ptr& message);

    // utils
    // редактирует сообщение вставляя в текст строки из text_replace, а в кнопки из button_replace,
    // где ключ это индекс кнопки
    void editTextAndKeyboard(
        const std::string& key, int64_t chat_id, int32_t id,
        const std::vector<std::string>& text_replace = {},
        const std::unordered_map<size_t, std::vector<std::string>>& button_replace = {});
    void deleteMessage(const TgBot::Message::Ptr& message);
};