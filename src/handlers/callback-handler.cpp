#include "handlers/callback-handler.hpp"

#include "bot/user-session.hpp"
#include "types/user-states.hpp"
#include "utils/date.hpp"

#include <string>

#include <spdlog/spdlog.h>
#include <tgbot/tgbot.h>
#include <tgbot/types/CallbackQuery.h>

void CallbackHandler::handleCallback(const TgBot::CallbackQuery::Ptr& query) {
    if (query->data.starts_with("reg:")) {
        onRegistration(query);
    } else if (query->data.starts_with("main_menu")) {
        onMainMenu(query);
    } else if (query->data.starts_with("add_metric")) {
        onAddMetric(query);
    }
}

void CallbackHandler::onRegistration(const TgBot::CallbackQuery::Ptr& query) {
    UserSession& session = session_manager_.getSession(query);
    spdlog::debug("Registration Callback handle. User {}",
                  session.user_data->user_id);
    if (query->data.ends_with("start")) {
        if (session.current_state != UserStates::RegistrationNeed) {
            spdlog::debug("Incorrect collback");
            return;
        }
        spdlog::debug("Start registration");
        session.current_state = UserStates::InputUserName;
        message_service_.editMessage(
            session, {"request_name", {}, {{query->from->firstName}}});
    } else if (query->data.ends_with("default_name")) {
        if (session.current_state != UserStates::InputUserName) {
            spdlog::debug("Incorrect collback");
            return;
        }
        spdlog::debug("Select default name: {}", query->from->firstName);
        if (set_name(session.user_data.value(), query->from->firstName)) {
            spdlog::debug("Set default name: {}", session.user_data->user_name);
            session.current_state = UserStates::InputUserAge;
            message_service_.editMessage(session, {"request_age"});
        } else {
            spdlog::debug("Incorrect tg name");
            message_service_.editMessage(
                session,
                {"invalid_name", {}, {{"Имя по умолчанию недоступно"}}});
        }
    } else if (query->data.ends_with("male")) {
        if (session.current_state != UserStates::InputUserSex) {
            spdlog::debug("Incorrect collback");
            return;
        }
        spdlog::debug("Select gender: Male");
        session.user_data->sex = Sex::Male;
        database_.addUser(session.user_data->user_id,
                          session.user_data.value());
        session.current_state = UserStates::MainMenu;
        message_service_.editMessage(session, {"empty_menu"});
    } else if (query->data.ends_with("female")) {
        if (session.current_state != UserStates::InputUserSex) {
            spdlog::debug("Incorrect collback");
            return;
        }
        spdlog::debug("Select gender: Female");
        session.user_data->sex = Sex::Female;
        database_.addUser(session.user_data->user_id,
                          session.user_data.value());
        session.current_state = UserStates::MainMenu;
        message_service_.editMessage(session, {"empty_menu"});
    } else {
        spdlog::debug("Incorrect collback");
        return;
    }
}

void CallbackHandler::onMainMenu(const TgBot::CallbackQuery::Ptr& query) {
    UserSession& session = session_manager_.getSession(query);
    if (session.current_state != UserStates::MainMenu) {
        spdlog::debug("Incorrect collback");
        return;
    }
    spdlog::debug("Main menu callback handle. User {}",
                  session.user_data->user_id);
    if (query->data.ends_with("add_record")) {
        spdlog::debug("Add record");
        message_service_.editMessage(session, {"add_record"});
        session.current_state = UserStates::AddMetricSelectMode;
    } else {
        spdlog::debug("Incorrect collback");
        return;
    }
}

void CallbackHandler::onAddMetric(const TgBot::CallbackQuery::Ptr& query) {
    UserSession& session = session_manager_.getSession(query);
    spdlog::debug("Add metric callback handle. User {}",
                  session.user_data->user_id);
    if (query->data.ends_with("file_mode")) {
        if (session.current_state != UserStates::AddMetricSelectMode) {
            spdlog::debug("Incorrect collback");
            return;
        }
        // dev
        spdlog::debug("File mode");
        session.current_state = UserStates::AddMetricFromFile;
    } else if (query->data.ends_with("manually")) {
        if (session.current_state != UserStates::AddMetricSelectMode) {
            spdlog::debug("Incorrect collback");
            return;
        }
        spdlog::debug("Manually mode");
        // загружаем последнее измерение для подсказок
        session.last_body_metrics =
            database_.getLastBodyMetrics(session.user_data->user_id);
        // инициализируем запись с возрастом и user_id
        session.new_body_metrics = {.user_id = session.user_data->user_id,
                                    .age = session.user_data->age};
        message_service_.requestDate(session);
    } else if (query->data.ends_with("date_today")) {
        if (session.current_state != UserStates::InputDate) {
            spdlog::debug("Incorrect collback");
            return;
        }
        session.new_body_metrics.date = getCurrentDate();
        // если запись за сегодня уже существует
        if (database_.getBodyMetricsByUserIdAndDate(
                session.user_data->user_id, session.new_body_metrics.date)) {
            message_service_.doublicateDate(session);
            return;
        }
        spdlog::debug("Current date select: {}", session.new_body_metrics.date);
        message_service_.requestWeight(session);
    } else if (query->data.ends_with("weight_previous")) {
        if (session.current_state != UserStates::InputWeight) {
            spdlog::debug("Incorrect collback");
            return;
        }
        if (!session.last_body_metrics) {
            spdlog::debug("Error using previus weight");
            return;
        }
        spdlog::debug("Use previous weight: {}",
                      session.last_body_metrics->weight);
        session.new_body_metrics.weight = session.last_body_metrics->weight;
        message_service_.requestHeight(session);
    } else if (query->data.ends_with("height_previous")) {
        if (session.current_state != UserStates::InputHeight) {
            spdlog::debug("Incorrect collback");
            return;
        }
        if (!session.last_body_metrics) {
            spdlog::debug("Error using previus height");
            return;
        }
        spdlog::debug("Use previuos height: {}",
                      session.last_body_metrics->height);
        session.new_body_metrics.height = session.last_body_metrics->height;
        message_service_.requestMuscleMass(session);
    } else if (query->data.ends_with("muscle_mass_previous")) {
        if (session.current_state != UserStates::InputMuscleMass) {
            spdlog::debug("Incorrect collback");
            return;
        }
        if (!session.last_body_metrics) {
            spdlog::debug("Error using previus muscle mass");
            return;
        }
        spdlog::debug("Use previous muscle mass: {}",
                      session.last_body_metrics->muscle_mass);
        session.new_body_metrics.muscle_mass =
            session.last_body_metrics->muscle_mass;
        message_service_.requestFatMass(session);
    } else if (query->data.ends_with("fat_mass_previous")) {
        if (session.current_state != UserStates::InputFatMass) {
            spdlog::debug("Incorrect collback");
            return;
        }
        if (!session.last_body_metrics) {
            spdlog::debug("Error using previus fat mass");
            return;
        }
        spdlog::debug("User previous fat mass: {}",
                      session.last_body_metrics->fat_mass);
        session.new_body_metrics.fat_mass = session.last_body_metrics->fat_mass;
        message_service_.printSummary(session);
    } else if (query->data.ends_with("save")) {
        if (session.current_state != UserStates::MetricSummary) {
            spdlog::debug("Incorrect collback");
            return;
        }
        database_.addBodyMetrics(session.new_body_metrics);
        message_service_.openMainMenu(session);
    } else if (query->data.ends_with("change_date")) {
        if (session.current_state != UserStates::DateDublicate) {
            spdlog::debug("Incorrect callback");
            return;
        }
        message_service_.requestDate(session);
        session.current_state = UserStates::InputDate;
    } else {
        spdlog::debug("Incorrect callback");
    }
}