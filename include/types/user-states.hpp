#pragma once

enum class UserStates {
    RegistrationNeed,

    // registration
    InputUserName,
    InputUserAge,
    InputUserSex,

    MainMenu,

    // adding metrics
    AddMetricSelectMode,

    // adding from file
    AddMetricFromFile,

    // input neccessary metrics
    InputDate,
    DateDublicate,
    InputWeight,
    InputHeight,
    InputMuscleMass,
    InputFatMass,

    MetricSummary,

    // input additional metrics
    SelectAdditionalMetrics,
    InputWaterMass,
    InputBoneMass,
    InputVisceralFat,
    InputProteinMass,
    InputMuscleSegments,
    InputFatSegments,

    // input segment value
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
};