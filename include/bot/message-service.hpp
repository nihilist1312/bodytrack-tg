#pragma once

#include "bot/user-session.hpp"
#include "database/database.hpp"
#include "message-loader.hpp"
#include "types/message-data.hpp"
#include "types/message-template.hpp"
#include "types/user-states.hpp"

#include <cstdint>
#include <string>
#include <string_view>

#include <tgbot/tgbot.h>
#include <tgbot/types/InlineKeyboardMarkup.h>

class MessageService {
  public:
    MessageService(TgBot::Bot& bot, MessageLoader& message_loader,
                   Database& database)
        : bot_(bot), message_loader_(message_loader), database_(database) {}

    void sendMessage(UserSession& session,
                     const MessageTemplate& message_template);
    void sendMessage(UserSession& session,
                     const MessageTemplate& message_template,
                     const MessageData& message_data);
    void editMessage(UserSession& session,
                     const MessageTemplate& message_template);
    void
    editMessage(UserSession& session, const std::string& text,
                const std::shared_ptr<TgBot::InlineKeyboardMarkup>& keyboard);
    void deleteMessage(const TgBot::Message::Ptr& message);
    void deleteMessage(int64_t chat_id, int32_t message_id);
    void deleteLast(UserSession& session);

    // message printer
    void registration(UserSession& session);
    void openMainMenu(UserSession& session);
    void addRecord(UserSession& session);

    // ---- Ввод пользовательских данных --------------------------------------
    void requestName(UserSession& session);
    void requestSex(UserSession& session);
    void requestAge(UserSession& session);

    void requestDate(UserSession& session);
    void invalidDate(UserSession& session);
    void doublicateDate(UserSession& session);
    void doublicateDateEdit(UserSession& session);

    void requestWeight(UserSession& session);
    void requestHeight(UserSession& session);
    void requestMuscleMass(UserSession& session);
    void requestFatMass(UserSession& session);

    void invalidWeight(UserSession& session);
    void invalidHeight(UserSession& session);
    void invalidMuscleMass(UserSession& session);
    void invalidFatMass(UserSession& session);

    // сводка основных метрик
    void printSummary(UserSession& session);

    // выбор ОСНОВНОЙ метрики для редактирования
    void selectMain(UserSession& session);

    // выбор доп метрики для редактирования
    void selectAdditional(UserSession& session);

    void requestWaterMass(UserSession& session);
    void requestBoneMass(UserSession& session);
    void requestVisceralFat(UserSession& session);
    void requestProteinMass(UserSession& session);
    void requestMuscleSegment(UserSession& session);
    void requestFatSegment(UserSession& session);

    void invalidBoneMass(UserSession& session);
    void invalidWaterMass(UserSession& session);
    void invalidVisceralFat(UserSession& session);
    void invalidProteinMass(UserSession& session);
    void invalidMuscleSegment(UserSession& session);
    void invalidFatSegment(UserSession& session);

    void requestLeftArmMuscle(UserSession& session);
    void requestRightArmMuscle(UserSession& session);
    void requestLeftLegMuscle(UserSession& session);
    void requestRightLegMuscle(UserSession& session);
    void requestTrunkMuscle(UserSession& session);

    void invalidLeftArmMuscle(UserSession& session);
    void invalidRightArmMuscle(UserSession& session);
    void invalidLeftLegMuscle(UserSession& session);
    void invalidRightLegMuscle(UserSession& session);
    void invalidTrunkMuscle(UserSession& session);

    void requestLeftArmFat(UserSession& session);
    void requestRightArmFat(UserSession& session);
    void requestLeftLegFat(UserSession& session);
    void requestRightLegFat(UserSession& session);
    void requestTrunkFat(UserSession& session);

    void invalidLeftArmFat(UserSession& session);
    void invalidRightArmFat(UserSession& session);
    void invalidLeftLegFat(UserSession& session);
    void invalidRightLegFat(UserSession& session);
    void invalidTrunkFat(UserSession& session);

    void cannotSaveMuscleSegments(UserSession& session);
    void cannotSaveFatSegments(UserSession& session);

    void historyScreen(UserSession& session);

  private:
    TgBot::Bot& bot_;
    MessageLoader& message_loader_;
    Database& database_;

    // загружает сообщение из шаблона, заполняя плейсхолдеры
    MessageData loadMessage(const MessageTemplate& message);

    // request_*/invalid_* для ОСНОВНЫХ метрик
    template <class T>
    void requestMetric(UserSession& session, std::string_view log_label,
                       std::string_view op_label, T BodyMetrics::* field,
                       const std::string& base_key, UserStates state);

    // Универсальный хелпер для request_*/invalid_* ДОП простых (несегментных)
    // метрик
    template <class T>
    void requestSimpleMetric(UserSession& session,
                             std::optional<T> BodyMetrics::* field,
                             UserStates state,
                             const std::string& base_message_key,
                             const std::string& log_label, bool is_invalid);

    void requestSegmentGroup(UserSession& session,
                             std::optional<SegmentMass> BodyMetrics::* field,
                             UserStates state,
                             const std::string& base_message_key,
                             const std::string& log_label);

    // Универсальный хелпер для request_* и invalid_* сообщений по сегментам
    // массы
    void requestSegment(UserSession& session, const std::string& segment_name,
                        std::optional<SegmentMass> BodyMetrics::* metrics_field,
                        std::optional<double> SegmentMass::* field,
                        UserStates state, const std::string& base_message_key);
};