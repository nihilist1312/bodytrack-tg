#pragma once

enum class UserStates {
    // ---- Регистрация -------------------------
    RegistrationNeed,
    InputUserName,
    InputUserAge,
    InputUserSex,

    MainMenu,

    // ---- Добавление записи -------------------
    AddMetricSelectMode,
    AddMetricFromFile,

    // ---- Ввод основных значений --------------
    InputDate,
    DateDuplicate,
    InputWeight,
    InputHeight,
    InputMuscleMass,
    InputFatMass,

    MetricSummary,
    SelectMainMetrics,

    // ---- Ввод доп значений -------------------
    SelectAdditionalMetrics,
    InputWaterMass,
    InputBoneMass,
    InputVisceralFat,
    InputProteinMass,
    InputMuscleSegments,
    InputFatSegments,

    // ---- Ввод сегментов значений -------------
    InputLeftArmMuscle,
    InputRightArmMuscle,
    InputLeftLegMuscle,
    InputRightLegMuscle,
    InputTrunkMuscle,

    InputLeftArmFat,
    InputRightArmFat,
    InputLeftLegFat,
    InputRightLegFat,
    InputTrunkFat,

    // ---- История -----------------------------
    HistoryScreen,
    HistoryRecord,
};