#include "handlers/callback-handler.hpp"

#include "bot/message-service.hpp"
#include "bot/user-session.hpp"
#include "models/body-metrics.hpp"
#include "types/user-states.hpp"
#include "utils/date.hpp"
#include "validation/body-metrics-validator.hpp"

#include <array>
#include <optional>
#include <string>
#include <string_view>

#include <spdlog/spdlog.h>
#include <tgbot/tgbot.h>
#include <tgbot/types/CallbackQuery.h>

namespace {

    // ---------------------------------------------------------------------------
    // Базовые предикаты
    // ---------------------------------------------------------------------------

    // сверяет текущее состояние и переданое
    bool checkState(const UserSession& session, UserStates expected_state) {
        if (session.current_state == expected_state) {
            return true;
        }
        spdlog::debug("Incorrect callback");
        return false;
    }

    template <class T> bool checkExist(const std::optional<T>& field) {
        if (field) {
            return true;
        }
        spdlog::debug("Incorrect callback");
        return false;
    }

    bool checkStateAndExist(const UserSession& session, UserStates state,
                            bool is_exist) {
        if (session.current_state == state && is_exist)
            return true;
        spdlog::debug("Incorrect callback");
        return false;
    }

    template <class T>
    bool checkStateAndExist(const UserSession& session, UserStates state,
                            const std::optional<T>& field) {
        if (session.current_state == state && field.has_value())
            return true;
        spdlog::debug("Incorrect callback");
        return false;
    }

    // проверяет что состояние входит в диапазон [start, end]
    bool checkState(const UserSession& session, UserStates start,
                    UserStates end) noexcept {
        if (session.current_state >= start && session.current_state <= end) {
            return true;
        }
        spdlog::debug("Incorrect callback");
        return false;
    }

    // проверяет что состояние есть в весторе
    bool checkState(const UserSession& session,
                    const std::vector<UserStates>& states) noexcept {
        for (auto state : states) {
            if (session.current_state == state)
                return true;
        }
        spdlog::debug("Incorrect callback");
        return false;
    }

    // ---------------------------------------------------------------------------
    // Таблицы диспетчеризации для onAddMetric.
    // ---------------------------------------------------------------------------

    using RequestFn = void (MessageService::*)(UserSession&);
    using CoreField = double BodyMetrics::*;
    using OptionalField = std::optional<double> BodyMetrics::*;
    using SegmentField = std::optional<double> SegmentMass::*;
    using SegmentMassPtr = std::optional<double>*;

    // NB: visceral_fat в BodyMetrics — std::optional<int>, а не optional<double>,
    // поэтому его нельзя положить в те же таблицы (другой тип member-pointer'а).
    // Обрабатывается отдельными явными ветками, без обобщения ради одного поля.

    // ---- 1. Ввод конкретного сегмента (мышцы/жир x 5 частей тела) ------------

    struct SegmentEntry {
        std::string_view suffix;
        UserStates state;
        RequestFn request;
    };

    constexpr std::array<SegmentEntry, 10> kSegmentEntries = {{
        {"segment_muscle_mass:left_arm", UserStates::InputMuscleSegments,
         &MessageService::requestLeftArmMuscle},
        {"segment_muscle_mass:right_arm", UserStates::InputMuscleSegments,
         &MessageService::requestRightArmMuscle},
        {"segment_muscle_mass:left_leg", UserStates::InputMuscleSegments,
         &MessageService::requestLeftLegMuscle},
        {"segment_muscle_mass:right_leg", UserStates::InputMuscleSegments,
         &MessageService::requestRightLegMuscle},
        {"segment_muscle_mass:trunk", UserStates::InputMuscleSegments,
         &MessageService::requestTrunkMuscle},

        {"segment_fat_mass:left_arm", UserStates::InputFatSegments,
         &MessageService::requestLeftArmFat},
        {"segment_fat_mass:right_arm", UserStates::InputFatSegments,
         &MessageService::requestRightArmFat},
        {"segment_fat_mass:left_leg", UserStates::InputFatSegments,
         &MessageService::requestLeftLegFat},
        {"segment_fat_mass:right_leg", UserStates::InputFatSegments,
         &MessageService::requestRightLegFat},
        {"segment_fat_mass:trunk", UserStates::InputFatSegments,
         &MessageService::requestTrunkFat},
    }};

    const SegmentEntry* findSegmentEntry(const std::string& data) {
        for (const auto& entry : kSegmentEntries) {
            if (data.ends_with(entry.suffix)) {
                return &entry;
            }
        }
        return nullptr;
    }

    // ---- 2. Выбор доп. метрики для ввода (просто request*, без черновика) ----

    struct AdditionalSelectEntry {
        std::string_view suffix;
        RequestFn request;
    };

    constexpr std::array<AdditionalSelectEntry, 4> kAdditionalSelectEntries = {{
        {"water_mass", &MessageService::requestWaterMass},
        {"bone_mass", &MessageService::requestBoneMass},
        {"visceral_fat", &MessageService::requestVisceralFat},
        {"protein_mass", &MessageService::requestProteinMass},
    }};

    const AdditionalSelectEntry*
    findAdditionalSelectEntry(const std::string& data) {
        for (const auto& entry : kAdditionalSelectEntries) {
            if (data.ends_with(entry.suffix)) {
                return &entry;
            }
        }
        return nullptr;
    }

    // ---- 3. Автоподстановка предыдущего значения для ОСНОВНЫХ метрик ---------
    // (поля не optional, порядок ввода фиксирован -> у каждой свой next_step)

    struct CoreMetricPreviousEntry {
        std::string_view suffix;
        UserStates state;
        CoreField field;
        std::string_view log_label;
        RequestFn next_step;
    };

    constexpr std::array<CoreMetricPreviousEntry, 4>
        kCoreMetricPreviousEntries = {{
            {"weight_previous", UserStates::InputWeight, &BodyMetrics::weight,
             "weight", &MessageService::requestHeight},
            {"height_previous", UserStates::InputHeight, &BodyMetrics::height,
             "height", &MessageService::requestMuscleMass},
            {"muscle_mass_previous", UserStates::InputMuscleMass,
             &BodyMetrics::muscle_mass, "muscle mass",
             &MessageService::requestFatMass},
            {"fat_mass_previous", UserStates::InputFatMass,
             &BodyMetrics::fat_mass, "fat mass", &MessageService::printSummary},
        }};

    // ---- 4. Автоподстановка предыдущего значения для ДОП. метрик -------------
    // (поля optional<double>, после подстановки всегда возврат к selectAdditional)

    struct AdditionalPreviousEntry {
        std::string_view suffix;
        UserStates state;
        OptionalField field;
        std::string_view log_label;
    };

    constexpr std::array<AdditionalPreviousEntry, 3>
        kAdditionalPreviousEntries = {{
            {"water_mass_previous", UserStates::InputWaterMass,
             &BodyMetrics::water_mass, "water mass"},
            {"bone_mass_previous", UserStates::InputBoneMass,
             &BodyMetrics::bone_mass, "bone mass"},
            {"protein_mass_previous", UserStates::InputProteinMass,
             &BodyMetrics::protein_mass, "protein mass"},
        }};

    // ---- 5. Удаление ДОП. метрики ---------------------------------------------

    struct AdditionalDeleteEntry {
        std::string_view suffix;
        UserStates state;
        OptionalField field;
        std::string_view log_label;
    };

    constexpr std::array<AdditionalDeleteEntry, 3> kAdditionalDeleteEntries = {{
        {"water_mass_delete", UserStates::InputWaterMass,
         &BodyMetrics::water_mass, "water mass"},
        {"bone_mass_delete", UserStates::InputBoneMass, &BodyMetrics::bone_mass,
         "bone mass"},
        {"protein_mass_delete", UserStates::InputProteinMass,
         &BodyMetrics::protein_mass, "protein mass"},
    }};

    // ---- 6. Автоподстановка предыдущего значения для сегментов -----------------

    SegmentMassPtr get_segment_field(SegmentMass& mass, UserStates state) {
        switch (state) {
        case UserStates::InputLeftArmMuscle:
        case UserStates::InputLeftArmFat:
            return &mass.left_arm;
        case UserStates::InputRightArmMuscle:
        case UserStates::InputRightArmFat:
            return &mass.right_arm;
        case UserStates::InputLeftLegMuscle:
        case UserStates::InputLeftLegFat:
            return &mass.left_leg;
        case UserStates::InputRightLegMuscle:
        case UserStates::InputRightLegFat:
            return &mass.right_leg;
        case UserStates::InputTrunkMuscle:
        case UserStates::InputTrunkFat:
            return &mass.trunk;
        default:
            return nullptr;
        }
    }

    void handle_segment_previous(UserSession& session,
                                 std::optional<SegmentMass>& source) {
        if (!checkExist(source))
            return;
        auto* dst = get_segment_field(*session.segment_mass_draft,
                                      session.current_state);
        auto* src = get_segment_field(*source, session.current_state);
        if (!dst || !src) {
            spdlog::debug("Incorrect callback");
            return;
        }
        *dst = *src;
    }

    // ---- 7. Удаление значения сегмента --------------------------------------
    void handle_segment_delete(UserSession& session) {
        auto* segment_ptr = get_segment_field(*session.segment_mass_draft,
                                              session.current_state);
        if (!segment_ptr) {
            spdlog::debug("Incorrect callback");
            return;
        }
        spdlog::debug("Delete segment");
        *segment_ptr = std::nullopt;
    }

    // ------------------------------------------------------------------------
    // |                            Кнопка back                               |
    // ------------------------------------------------------------------------
    using MessageAction = void (MessageService::*)(UserSession&);

    struct BackEntry {
        UserStates state;
        MessageAction action;
    };

    constexpr std::array<BackEntry, 9> kBackEntries{{
        {UserStates::InputUserName, &MessageService::registration},
        {UserStates::InputUserAge, &MessageService::requestName},
        {UserStates::InputUserSex, &MessageService::requestAge},
        {UserStates::InputDate, &MessageService::addRecord},
        {UserStates::DateDublicate, &MessageService::addRecord},
        {UserStates::InputWeight, &MessageService::requestDate},
        {UserStates::InputHeight, &MessageService::requestWeight},
        {UserStates::InputMuscleMass, &MessageService::requestHeight},
        {UserStates::InputFatMass, &MessageService::requestMuscleMass},
    }};

} // namespace

void CallbackHandler::handleCallback(const TgBot::CallbackQuery::Ptr& query) {
    if (query->data.starts_with("reg:")) {
        onRegistration(query);
    } else if (query->data.starts_with("main_menu")) {
        onMainMenu(query);
    } else if (query->data.starts_with("add_metric")) {
        onAddMetric(query);
    } else if (query->data.starts_with("back")) {
        onBack(query);
    } else if (query->data.starts_with("cancel")) {
        onCancel(query);
    }

    else {
        spdlog::debug("Incorrect callback");
    }
}

void CallbackHandler::onRegistration(const TgBot::CallbackQuery::Ptr& query) {
    UserSession& session = session_manager_.getSession(query);
    spdlog::debug("Registration Callback handle. User {}",
                  session.user_data->user_id);
    if (query->data.ends_with("start")) {
        if (!checkState(session, UserStates::RegistrationNeed)) {
            return;
        }
        spdlog::debug("Start registration");
        message_service_.requestName(session);
    } else if (query->data.ends_with("default_name")) {
        if (!checkState(session, UserStates::InputUserName)) {
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
        if (!checkState(session, UserStates::InputUserSex)) {
            return;
        }
        spdlog::debug("Select gender: Male");
        session.user_data->sex = Sex::Male;
        database_.addUser(session.user_data->user_id,
                          session.user_data.value());
        session.current_state = UserStates::MainMenu;
        message_service_.editMessage(session, {"empty_menu"});
    } else if (query->data.ends_with("female")) {
        if (!checkState(session, UserStates::InputUserSex)) {
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
    if (!checkState(session, UserStates::MainMenu)) {
        return;
    }
    spdlog::debug("Main menu callback handle. User {}",
                  session.user_data->user_id);
    if (query->data.ends_with("add_record")) {
        message_service_.addRecord(session);
    } else if (query->data.ends_with("history")) {
        message_service_.historyScreen(session);
    }

    else {
        spdlog::debug("Incorrect collback");
        return;
    }
}

void CallbackHandler::onAddMetric(const TgBot::CallbackQuery::Ptr& query) {
    UserSession& session = session_manager_.getSession(query);
    spdlog::debug("Add metric callback handle. User {}",
                  session.user_data->user_id);

    const std::string& data = query->data;

    // ---- выбор режима ------------------------------------------------
    if (data.ends_with("file_mode")) {
        if (!checkState(session, UserStates::AddMetricSelectMode)) {
            return;
        }
        // dev
        spdlog::debug("File mode");
        session.current_state = UserStates::AddMetricFromFile;
        return;
    }
    if (data.ends_with("manually")) {
        if (!checkState(session, UserStates::AddMetricSelectMode)) {
            return;
        }
        spdlog::debug("Manually mode");
        // загружаем последнее измерение для подсказок
        session.last_body_metrics =
            database_.getLastBodyMetrics(session.user_data->user_id);
        // инициализируем запись с возрастом и user_id
        session.body_metrics_draft = {.user_id = session.user_data->user_id,
                                      .age = session.user_data->age};
        message_service_.requestDate(session);
        return;
    }

    // ---- дата -----------------------------------------------------------
    if (data.ends_with("date_today")) {
        if (!checkState(session, UserStates::InputDate)) {
            return;
        }
        session.body_metrics_draft.date = getCurrentDate();
        // если запись за сегодня уже существует
        if (database_.getBodyMetricsByUserIdAndDate(
                session.user_data->user_id, session.body_metrics_draft.date)) {
            message_service_.doublicateDate(session);
            return;
        }
        spdlog::debug("Current date select: {}",
                      session.body_metrics_draft.date);
        message_service_.requestWeight(session);
        return;
    }
    if (data.ends_with("change_date")) {
        if (!checkState(session, UserStates::DateDublicate)) {
            return;
        }
        message_service_.requestDate(session);
        session.current_state = UserStates::InputDate;
        return;
    }

    // ---- автоподстановка предыдущих значений (основные метрики) --------
    for (const auto& entry : kCoreMetricPreviousEntries) {
        if (!data.ends_with(entry.suffix)) {
            continue;
        }
        if (!checkStateAndExist(session, entry.state,
                                session.last_body_metrics)) {
            return;
        }
        double value = session.last_body_metrics.value().*entry.field;
        session.body_metrics_draft.*entry.field = value;
        spdlog::debug("Use previous {}: {}", entry.log_label, value);
        (message_service_.*entry.next_step)(session);
        return;
    }

    // ---- сводки и прочие меню --------------------------------------------
    if (data.ends_with("save")) {
        if (session.current_state != UserStates::MetricSummary &&
            session.current_state != UserStates::SelectAdditionalMetrics) {
            spdlog::debug("Incorrect collback");
            return;
        }
        database_.addBodyMetrics(session.body_metrics_draft);
        message_service_.openMainMenu(session);
        return;
    }
    if (data.ends_with("additional")) {
        // вызывать можно только из сводки и ввода доп метрик
        if (session.current_state < UserStates::MetricSummary ||
            session.current_state > UserStates::InputFatSegments) {
            spdlog::debug("Incorrect callback");
            return;
        }
        message_service_.selectAdditional(session);
        session.segment_mass_draft = std::nullopt;
        return;
    }
    if (data.ends_with("save_segments")) {
        if (session.current_state == UserStates::InputMuscleSegments) {
            if (allSegmetsFilled(session.segment_mass_draft) &&
                validateSegmentMass(*session.segment_mass_draft,
                                    session.body_metrics_draft.weight)) {
                spdlog::debug("Muscle segments save");
                session.body_metrics_draft.segment_muscle_mass =
                    session.segment_mass_draft;
            } else {
                message_service_.cannotSaveMuscleSegments(session);
                return;
            }
        } else if (session.current_state == UserStates::InputFatSegments) {
            if (allSegmetsFilled(session.segment_mass_draft) &&
                validateSegmentMass(*session.segment_mass_draft,
                                    session.body_metrics_draft.fat_mass)) {
                spdlog::debug("Fat segments save");
                session.body_metrics_draft.segment_fat_mass =
                    session.segment_mass_draft;
            } else {
                message_service_.cannotSaveFatSegments(session);
                return;
            }
        } else {
            spdlog::debug("Incorrect callback");
            return;
        }

        session.segment_mass_draft = std::nullopt;
        message_service_.selectAdditional(session);
        return;
    }

    // ---- выбор доп метрик для редактирования ------------------------------
    if (const auto* entry = findAdditionalSelectEntry(data)) {
        if (!checkState(session, UserStates::SelectAdditionalMetrics)) {
            return;
        }
        (message_service_.*entry->request)(session);
        return;
    }
    if (data.ends_with("segment_muscle_mass")) {
        if (session.current_state != UserStates::SelectAdditionalMetrics &&
            !checkState(session, UserStates::InputLeftArmMuscle,
                        UserStates::InputTrunkMuscle)) {
            return;
        }
        // инициализируем черновик
        if (!session.segment_mass_draft)
            session.segment_mass_draft =
                session.body_metrics_draft.segment_muscle_mass;
        // if (!session.segment_mass_draft)
        //     session.segment_mass_draft.emplace();
        message_service_.requestMuscleSegment(session);
        return;
    }
    if (data.ends_with("segment_fat_mass")) {
        if (session.current_state != UserStates::SelectAdditionalMetrics &&
            !checkState(session, UserStates::InputLeftArmFat,
                        UserStates::InputTrunkFat)) {
            return;
        }
        // инициализируем черновик
        if (!session.segment_mass_draft)
            session.segment_mass_draft =
                session.body_metrics_draft.segment_fat_mass;
        // if (!session.segment_mass_draft)
        //     session.segment_mass_draft.emplace();
        message_service_.requestFatSegment(session);
        return;
    }

    // ---- автоподстановка доп метрик ---------------------------------------
    for (const auto& entry : kAdditionalPreviousEntries) {
        if (!data.ends_with(entry.suffix)) {
            continue;
        }
        const bool exists =
            session.last_body_metrics &&
            (session.last_body_metrics.value().*entry.field).has_value();
        if (!checkStateAndExist(session, entry.state, exists)) {
            return;
        }
        double value = (session.last_body_metrics.value().*entry.field).value();
        session.body_metrics_draft.*entry.field = value;
        spdlog::debug("Using previous {}: {}", entry.log_label, value);
        message_service_.selectAdditional(session);
        return;
    }
    if (data.ends_with("visceral_fat_previous")) {
        // visceral_fat — optional<int>, отдельная ветка, см. комментарий у OptionalField
        const bool exists = session.last_body_metrics &&
                            session.last_body_metrics->visceral_fat.has_value();
        if (!checkStateAndExist(session, UserStates::InputVisceralFat,
                                exists)) {
            return;
        }
        int value = session.last_body_metrics->visceral_fat.value();
        session.body_metrics_draft.visceral_fat = value;
        spdlog::debug("Using previous visceral fat: {}", value);
        message_service_.selectAdditional(session);
        return;
    }
    if (data.ends_with("muscle_segments_previous")) {
        const bool exists = session.last_body_metrics &&
                            session.last_body_metrics->segment_muscle_mass;
        if (!checkStateAndExist(session, UserStates::InputMuscleSegments,
                                exists)) {
            return;
        }
        session.body_metrics_draft.segment_muscle_mass =
            session.last_body_metrics->segment_muscle_mass;
        session.segment_mass_draft = std::nullopt;
        spdlog::debug("Using previous muscle segment");
        message_service_.selectAdditional(session);
        return;
    }
    if (data.ends_with("fat_segments_previous")) {
        const bool exists = session.last_body_metrics &&
                            session.last_body_metrics->segment_fat_mass;
        if (!checkStateAndExist(session, UserStates::InputFatSegments,
                                exists)) {
            return;
        }
        session.body_metrics_draft.segment_fat_mass =
            session.last_body_metrics->segment_fat_mass;
        session.segment_mass_draft = std::nullopt;
        spdlog::debug("Using previous fat segments");
        message_service_.selectAdditional(session);
        return;
    }

    // ---- удаление доп метрик -----------------------------------------------
    for (const auto& entry : kAdditionalDeleteEntries) {
        if (!data.ends_with(entry.suffix)) {
            continue;
        }
        if (!checkStateAndExist(session, entry.state,
                                session.body_metrics_draft.*entry.field)) {
            return;
        }
        session.body_metrics_draft.*entry.field = std::nullopt;
        spdlog::debug("Delete {}", entry.log_label);
        message_service_.selectAdditional(session);
        return;
    }
    if (data.ends_with("visceral_fat_delete")) {
        if (!checkStateAndExist(session, UserStates::InputVisceralFat,
                                session.body_metrics_draft.visceral_fat)) {
            return;
        }
        session.body_metrics_draft.visceral_fat = std::nullopt;
        spdlog::debug("Delete visceral fat");
        message_service_.selectAdditional(session);
        return;
    }
    if (data.ends_with("segment_muscle_mass_delete")) {
        if (!checkStateAndExist(session, UserStates::InputMuscleSegments,
                                session.segment_mass_draft)) {
            return;
        }
        session.segment_mass_draft =
            session.body_metrics_draft.segment_muscle_mass = std::nullopt;
        spdlog::debug("Delete segment muscle mass");
        message_service_.selectAdditional(session);
        return;
    }
    if (data.ends_with("segment_fat_mass_delete")) {
        if (!checkStateAndExist(session, UserStates::InputFatSegments,
                                session.segment_mass_draft)) {
            return;
        }
        session.segment_mass_draft =
            session.body_metrics_draft.segment_fat_mass = std::nullopt;
        spdlog::debug("Delete segment fat mass");
        message_service_.selectAdditional(session);
        return;
    }

    // ---- ввод сегментов -----------------------------------------------------
    if (const auto* entry = findSegmentEntry(data)) {
        if (!checkState(session, entry->state)) {
            return;
        }
        if (!session.segment_mass_draft) {
            session.segment_mass_draft.emplace();
        }
        (message_service_.*entry->request)(session);
        return;
    }

    // ---- автоподстановка сегментов ------------------------------------------
    if (data.ends_with("segment_muscle_mass_value_previous")) {
        handle_segment_previous(session,
                                session.last_body_metrics->segment_muscle_mass);
        return;
    }
    if (data.ends_with("segment_fat_mass_value_previous")) {
        handle_segment_previous(session,
                                session.last_body_metrics->segment_fat_mass);
        return;
    }

    // ---- удаление сегмента --------------------------------------------------
    if (data.ends_with("segment_muscle_mass_value_delete")) {
        handle_segment_delete(session);
        message_service_.requestMuscleSegment(session);
        return;
    }
    if (data.ends_with("segment_fat_mass_value_delete")) {
        handle_segment_delete(session);
        message_service_.requestFatSegment(session);
        return;
    }

    spdlog::debug("Incorrect callback");
}

void CallbackHandler::onBack(const TgBot::CallbackQuery::Ptr& query) {
    UserSession& session = session_manager_.getSession(query);

    for (const auto& entry : kBackEntries) {
        if (session.current_state != entry.state)
            continue;

        (message_service_.*entry.action)(session);
        return;
    }
}

void CallbackHandler::onCancel(const TgBot::CallbackQuery::Ptr& query) {
    UserSession& session = session_manager_.getSession(query);

    if (!checkState(session, {UserStates::AddMetricSelectMode,
                              UserStates::HistoryPage})) {
        return;
    }

    message_service_.openMainMenu(session);
}