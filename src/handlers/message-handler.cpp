#include "handlers/message-handler.hpp"

#include "bot/message-service.hpp"
#include "bot/user-session.hpp"
#include "models/body-metrics.hpp"
#include "types/user-states.hpp"
#include "utils/input_parser.hpp"
#include "validation/body-metrics-validator.hpp"

#include <optional>
#include <string>
#include <string_view>

#include <spdlog/spdlog.h>
#include <tgbot/tgbot.h>
#include <tgbot/types/Message.h>
// dev
void MessageHandler::handleFileMessage(const TgBot::Message::Ptr& message) {
    UserSession& session = user_session_manager_.getSession(message);

    message_service_.deleteMessage(message);
}

void MessageHandler::handleTextMessage(const TgBot::Message::Ptr& message) {
    spdlog::debug("handle text message");
    UserSession& session = user_session_manager_.getSession(message);
    message_service_.deleteMessage(message);

    switch (session.current_state) {
    case UserStates::InputUserName:
        onName(message, session);
        break;
    case UserStates::InputUserAge:
        onAge(message, session);
        break;

    case UserStates::InputDate:
        onDate(message, session);
        break;
    case UserStates::InputWeight:
        onWeight(message, session);
        break;
    case UserStates::InputHeight:
        onHeight(message, session);
        break;
    case UserStates::InputMuscleMass:
        onMuscleMass(message, session);
        break;
    case UserStates::InputFatMass:
        onFatMass(message, session);
        break;

    case UserStates::InputWaterMass:
        onWaterMass(message, session);
        break;
    case UserStates::InputBoneMass:
        onBoneMass(message, session);
        break;
    case UserStates::InputVisceralFat:
        onVisceralFat(message, session);
        break;
    case UserStates::InputProteinMass:
        onProteinMass(message, session);
        break;
    case UserStates::InputMuscleSegments:
        onMuscleMassSegments(message, session);
        break;
    case UserStates::InputFatSegments:
        onFatMassSegments(message, session);
        break;

    case UserStates::InputLeftArmMuscle:
        onLeftArmMuscle(message, session);
        break;
    case UserStates::InputRightArmMuscle:
        onRightArmMuscle(message, session);
        break;
    case UserStates::InputLeftLegMuscle:
        onLeftLegMuscle(message, session);
        break;
    case UserStates::InputRightLegMuscle:
        onRightLegMuscle(message, session);
        break;
    case UserStates::InputTrunkMuscle:
        onTrunkMuscle(message, session);
        break;

    case UserStates::InputLeftArmFat:
        onLeftArmFat(message, session);
        break;
    case UserStates::InputRightArmFat:
        onRightArmFat(message, session);
        break;
    case UserStates::InputLeftLegFat:
        onLeftLegFat(message, session);
        break;
    case UserStates::InputRightLegFat:
        onRightLegFat(message, session);
        break;
    case UserStates::InputTrunkFat:
        onTrunkFat(message, session);
        break;
    default:
        spdlog::debug("Default handler");
        break;
    }
}

void MessageHandler::handleMessage(const TgBot::Message::Ptr& message) {
    if (message->document) {
        handleFileMessage(message);
    } else if (!message->text.empty()) {
        if (message->text[0] != '/')
            handleTextMessage(message);
    } else {
        spdlog::debug("Unsupported message type. Delete");
        message_service_.deleteMessage(message);
    }
}

void MessageHandler::onName(const TgBot::Message::Ptr& message,
                            UserSession& session) {
    spdlog::debug("Name handle");
    if (set_name(session.user_data.value(), message->text)) {
        spdlog::debug("Set user name: {}", session.user_data->user_name);
        message_service_.requestAge(session);
    } else {
        spdlog::debug("Incorrect name");
        message_service_.editMessage(
            session, {"invalid_name", {}, {{message->from->firstName}}});
    }
}

void MessageHandler::onAge(const TgBot::Message::Ptr& message,
                           UserSession& session) {
    spdlog::debug("Age handle");
    if (set_age(session.user_data.value(), message->text)) {
        spdlog::debug("Set user age: {}", session.user_data->age);
        message_service_.requestSex(session);
    } else {
        spdlog::debug("Incorrect age");
        message_service_.editMessage(session, {"invalid_age"});
    }
}

void MessageHandler::onDate(const TgBot::Message::Ptr& message,
                            UserSession& session) {
    spdlog::debug("Date handle");
    if (setDate(session.body_metrics_draft, message->text)) {
        // если запись с такой датой уже существует
        if (database_.getBodyMetricsByUserIdAndDate(
                session.user_data->user_id, session.body_metrics_draft.date)) {
            message_service_.doublicateDate(session);
            return;
        }
        spdlog::debug("Set date: {}", session.body_metrics_draft.date);
        message_service_.requestWeight(session);
    } else {
        message_service_.invalidDate(session);
    }
}

// ---- ввод ОСНОВНЫХ метрик --------------------------------------------------

void MessageHandler::onWeight(const TgBot::Message::Ptr& message,
                              UserSession& session) {
    spdlog::debug("Weight handle");
    if (setMass(session.body_metrics_draft.weight, message->text)) {
        spdlog::debug("Set weight: {}", session.body_metrics_draft.weight);
        message_service_.requestHeight(session);
    } else {
        message_service_.invalidWeight(session);
    }
}

void MessageHandler::onHeight(const TgBot::Message::Ptr& message,
                              UserSession& session) {
    spdlog::debug("Height handle");
    if (setHeight(session.body_metrics_draft, message->text)) {
        spdlog::debug("Set height: {}", session.body_metrics_draft.height);
        message_service_.requestMuscleMass(session);
    } else {
        message_service_.invalidHeight(session);
    }
}

void MessageHandler::onMuscleMass(const TgBot::Message::Ptr& message,
                                  UserSession& session) {
    spdlog::debug("Muscle Mass handle");
    if (setMass(session.body_metrics_draft.muscle_mass, message->text) &&
        session.body_metrics_draft.muscle_mass <
            session.body_metrics_draft.weight) {
        spdlog::debug("Set muscle mass: {}",
                      session.body_metrics_draft.muscle_mass);
        message_service_.requestFatMass(session);
    } else {
        message_service_.invalidMuscleMass(session);
    }
}

void MessageHandler::onFatMass(const TgBot::Message::Ptr& message,
                               UserSession& session) {
    spdlog::debug("Fat mass handle");
    if (setMass(session.body_metrics_draft.fat_mass, message->text) &&
        validateBodyMetrics(session.body_metrics_draft)) {
        spdlog::debug("Set fat mass: {}", session.body_metrics_draft.fat_mass);
        message_service_.printSummary(session);
    } else {
        message_service_.invalidFatMass(session);
    }
}

// ---- ввод ДОП. метрик ------------------------------------------------------
template <class T>
void MessageHandler::inputAdditional(std::string_view message,
                                     UserSession& session,
                                     std::string_view log_label,
                                     std::optional<T> BodyMetrics::* field,
                                     ServiceFunc invalid_func) {
    spdlog::debug("{} handle", log_label);
    session.body_metrics_draft.*field = setMass(message);
    if (session.body_metrics_draft.*field &&
        validateBodyMetrics(session.body_metrics_draft)) {
        spdlog::debug("Set {}: {}", log_label,
                      *(session.body_metrics_draft.*field));
        message_service_.selectAdditional(session);
    } else {
        (message_service_.*invalid_func)(session);
    }
}

void MessageHandler::onWaterMass(const TgBot::Message::Ptr& message,
                                 UserSession& session) {
    inputAdditional(message->text, session, "water mass",
                    &BodyMetrics::water_mass,
                    &MessageService::invalidWaterMass);
}

void MessageHandler::onBoneMass(const TgBot::Message::Ptr& message,
                                UserSession& session) {
    inputAdditional(message->text, session, "bone mass",
                    &BodyMetrics::bone_mass, &MessageService::invalidBoneMass);
}

void MessageHandler::onVisceralFat(const TgBot::Message::Ptr& message,
                                   UserSession& session) {
    inputAdditional(message->text, session, "visceral fat",
                    &BodyMetrics::visceral_fat,
                    &MessageService::invalidVisceralFat);
}

void MessageHandler::onProteinMass(const TgBot::Message::Ptr& message,
                                   UserSession& session) {
    inputAdditional(message->text, session, "protein mass",
                    &BodyMetrics::protein_mass,
                    &MessageService::invalidProteinMass);
}

// ---- ввод всех сегментов за раз --------------------------------------------
void MessageHandler::onMuscleMassSegments(const TgBot::Message::Ptr& message,
                                          UserSession& session) {
    spdlog::debug("Muscle segments handle");
    session.segment_mass_draft = getSegmentsMass(message->text);
    if (session.segment_mass_draft &&
        validateSegmentMass(*session.segment_mass_draft,
                            session.body_metrics_draft.weight)) {
        spdlog::debug("Set muscle segments: {}", message->text);
        session.body_metrics_draft.segment_muscle_mass =
            session.segment_mass_draft;
        session.segment_mass_draft = std::nullopt;
        message_service_.selectAdditional(session);
    } else {
        message_service_.invalidMuscleSegment(session);
    }
}

void MessageHandler::onFatMassSegments(const TgBot::Message::Ptr& message,
                                       UserSession& session) {
    spdlog::debug("Fat segments handle");
    session.segment_mass_draft = getSegmentsMass(message->text);
    if (session.segment_mass_draft &&
        validateSegmentMass(*session.segment_mass_draft,
                            session.body_metrics_draft.fat_mass)) {
        spdlog::debug("Set fat segments: {}", message->text);
        session.body_metrics_draft.segment_fat_mass =
            session.segment_mass_draft;
        session.segment_mass_draft = std::nullopt;
        message_service_.selectAdditional(session);
    } else {
        message_service_.invalidFatSegment(session);
    }
}

// ---- ввод сегмента ---------------------------------------------------------
void MessageHandler::inputSegment(std::string_view message,
                                  UserSession& session,
                                  std::string_view segment_label,
                                  std::string_view groupe_label,
                                  std::optional<double> SegmentMass::* field,
                                  double BodyMetrics::* total_field,
                                  ServiceFunc success_func,
                                  ServiceFunc error_func) {
    spdlog::debug("{} {} handle", segment_label, groupe_label);
    session.segment_mass_draft.value().*field = setMass(message);
    if (session.segment_mass_draft.value().*field &&
        validateSegmentMass(*session.segment_mass_draft,
                            session.body_metrics_draft.*total_field)) {
        spdlog::debug("Set {} {}: {}", segment_label, groupe_label,
                      *(session.segment_mass_draft.value().*field));
        (message_service_.*success_func)(session);
    } else {
        (message_service_.*error_func)(session);
    }
}

void MessageHandler::onLeftArmMuscle(const TgBot::Message::Ptr& message,
                                     UserSession& session) {
    inputSegment(message->text, session, "left arm", "muscle",
                 &SegmentMass::left_arm, &BodyMetrics::weight,
                 &MessageService::requestMuscleSegment,
                 &MessageService::invalidMuscleSegment);
}

void MessageHandler::onRightArmMuscle(const TgBot::Message::Ptr& message,
                                      UserSession& session) {
    inputSegment(message->text, session, "right arm", "muscle",
                 &SegmentMass::right_arm, &BodyMetrics::weight,
                 &MessageService::requestMuscleSegment,
                 &MessageService::invalidMuscleSegment);
}

void MessageHandler::onLeftLegMuscle(const TgBot::Message::Ptr& message,
                                     UserSession& session) {
    inputSegment(message->text, session, "left leg", "muscle",
                 &SegmentMass::left_leg, &BodyMetrics::weight,
                 &MessageService::requestMuscleSegment,
                 &MessageService::invalidMuscleSegment);
}

void MessageHandler::onRightLegMuscle(const TgBot::Message::Ptr& message,
                                      UserSession& session) {
    inputSegment(message->text, session, "right leg", "muscle",
                 &SegmentMass::right_leg, &BodyMetrics::weight,
                 &MessageService::requestMuscleSegment,
                 &MessageService::invalidMuscleSegment);
}

void MessageHandler::onTrunkMuscle(const TgBot::Message::Ptr& message,
                                   UserSession& session) {
    inputSegment(message->text, session, "trunk", "muscle", &SegmentMass::trunk,
                 &BodyMetrics::weight, &MessageService::requestMuscleSegment,
                 &MessageService::invalidMuscleSegment);
}

void MessageHandler::onLeftArmFat(const TgBot::Message::Ptr& message,
                                  UserSession& session) {
    inputSegment(message->text, session, "left arm", "fat",
                 &SegmentMass::left_arm, &BodyMetrics::fat_mass,
                 &MessageService::requestFatSegment,
                 &MessageService::invalidFatSegment);
}

void MessageHandler::onRightArmFat(const TgBot::Message::Ptr& message,
                                   UserSession& session) {
    inputSegment(message->text, session, "right arm", "fat",
                 &SegmentMass::right_arm, &BodyMetrics::fat_mass,
                 &MessageService::requestFatSegment,
                 &MessageService::invalidFatSegment);
}

void MessageHandler::onLeftLegFat(const TgBot::Message::Ptr& message,
                                  UserSession& session) {
    inputSegment(message->text, session, "left leg", "fat",
                 &SegmentMass::left_leg, &BodyMetrics::fat_mass,
                 &MessageService::requestFatSegment,
                 &MessageService::invalidFatSegment);
}

void MessageHandler::onRightLegFat(const TgBot::Message::Ptr& message,
                                   UserSession& session) {
    inputSegment(message->text, session, "right leg", "fat",
                 &SegmentMass::right_leg, &BodyMetrics::fat_mass,
                 &MessageService::requestFatSegment,
                 &MessageService::invalidFatSegment);
}

void MessageHandler::onTrunkFat(const TgBot::Message::Ptr& message,
                                UserSession& session) {
    inputSegment(message->text, session, "trunk", "fat", &SegmentMass::trunk,
                 &BodyMetrics::fat_mass, &MessageService::requestFatSegment,
                 &MessageService::invalidFatSegment);
}