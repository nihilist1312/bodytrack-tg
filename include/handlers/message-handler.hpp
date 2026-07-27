#pragma once

#include "bot/message-service.hpp"
#include "bot/user-session.hpp"
#include "database/database.hpp"

#include <tgbot/tgbot.h>
#include <tgbot/types/Message.h>

class MessageHandler {
  public:
    MessageHandler(Database& database, UserSessionManager& user_session_manager,
                   MessageService& message_service)
        : database_(database), user_session_manager_(user_session_manager),
          message_service_(message_service) {}

    void handleMessage(const TgBot::Message::Ptr& message);

  private:
    // TgBot::Bot& bot_;
    Database& database_;
    UserSessionManager& user_session_manager_;
    MessageService& message_service_;

    void handleTextMessage(const TgBot::Message::Ptr& message);
    void handleFileMessage(const TgBot::Message::Ptr& message);

    // registration
    void onName(const TgBot::Message::Ptr& message, UserSession& session);
    void onAge(const TgBot::Message::Ptr& message, UserSession& session);

    // input metrics
    void onWeight(const TgBot::Message::Ptr& message);
    void onHeight(const TgBot::Message::Ptr& message);
    void onMusculeMass(const TgBot::Message::Ptr& message);
    void onFatMass(const TgBot::Message::Ptr& message);

    // input optional metrics
    void onWaterMass(const TgBot::Message::Ptr& message);
    void onBoneMass(const TgBot::Message::Ptr& message);
    void onVisceralFat(const TgBot::Message::Ptr& message);
    void onPriteinMass(const TgBot::Message::Ptr& message);

    // ввод в формате leftArm,rightArm,leftLeg,rightLeg,trunk (dev)
    void onMuscleMassSegments(const TgBot::Message::Ptr& message);
    void onFatMassSegments(const TgBot::Message::Ptr& message);

    void defaultHandler(const TgBot::Message::Ptr& message,
                        UserSession& session);
};