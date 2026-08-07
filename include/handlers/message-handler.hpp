#pragma once

#include "bot/message-service.hpp"
#include "bot/user-session.hpp"
#include "database/database.hpp"
#include "models/body-metrics.hpp"

#include <string_view>

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

    using ServiceFunc = void (MessageService::*)(UserSession&);

    void handleTextMessage(const TgBot::Message::Ptr& message);
    void handleFileMessage(const TgBot::Message::Ptr& message);

    // registration
    void onName(const TgBot::Message::Ptr& message, UserSession& session);
    void onAge(const TgBot::Message::Ptr& message, UserSession& session);

    // ---- ввод основных метрик ----------------------------------------------
    void onDate(const TgBot::Message::Ptr& message, UserSession& session);
    void inputMain(std::string_view message, UserSession& session,
                   std::string_view log_label,
                   bool (*parser)(double&, std::string_view),
                   double BodyMetrics::* field, ServiceFunc success_creat,
                   ServiceFunc success_edit, ServiceFunc invalid_func);
    void onWeight(const TgBot::Message::Ptr& message, UserSession& session);
    void onHeight(const TgBot::Message::Ptr& message, UserSession& session);
    void onMuscleMass(const TgBot::Message::Ptr& message, UserSession& session);
    void onFatMass(const TgBot::Message::Ptr& message, UserSession& session);

    // ---- ввод ДОП. метрик --------------------------------------------------
    template <class T>
    void inputAdditional(std::string_view message, UserSession& session,
                         std::string_view log_label,
                         std::optional<T> BodyMetrics::* field,
                         ServiceFunc invalid_func);
    void onWaterMass(const TgBot::Message::Ptr& message, UserSession& session);
    void onBoneMass(const TgBot::Message::Ptr& message, UserSession& session);
    void onVisceralFat(const TgBot::Message::Ptr& message,
                       UserSession& session);
    void onProteinMass(const TgBot::Message::Ptr& message,
                       UserSession& session);

    // ---- ввод в формате leftArm rightArm leftLeg rightLeg trunk-------------
    void onMuscleMassSegments(const TgBot::Message::Ptr& message,
                              UserSession& session);
    void onFatMassSegments(const TgBot::Message::Ptr& message,
                           UserSession& session);

    // ---- ввод сегментов ----------------------------------------------------
    void inputSegment(std::string_view message, UserSession& session,
                      std::string_view segment_label,
                      std::string_view groupe_label,
                      std::optional<double> SegmentMass::* field,
                      double BodyMetrics::* total_field,
                      ServiceFunc success_func, ServiceFunc error_func);

    void onLeftArmMuscle(const TgBot::Message::Ptr& message,
                         UserSession& session);
    void onRightArmMuscle(const TgBot::Message::Ptr& message,
                          UserSession& session);
    void onLeftLegMuscle(const TgBot::Message::Ptr& message,
                         UserSession& session);
    void onRightLegMuscle(const TgBot::Message::Ptr& message,
                          UserSession& session);
    void onTrunkMuscle(const TgBot::Message::Ptr& message,
                       UserSession& session);

    void onLeftArmFat(const TgBot::Message::Ptr& message, UserSession& session);
    void onRightArmFat(const TgBot::Message::Ptr& message,
                       UserSession& session);
    void onLeftLegFat(const TgBot::Message::Ptr& message, UserSession& session);
    void onRightLegFat(const TgBot::Message::Ptr& message,
                       UserSession& session);
    void onTrunkFat(const TgBot::Message::Ptr& message, UserSession& session);

    void defaultHandler(const TgBot::Message::Ptr& message,
                        UserSession& session);
};