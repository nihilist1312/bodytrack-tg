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
    InputWeight,
    InputHeight,
    InputMuscleMass,
    InputBodyFatMass,

    // input optional metrics
    SelectOptionalMetrics,
    InputWaterMass,
    InputBoneMass,
    InputVisceralFat,
    InputProteinMass,
    InputMuscleMassSegments,
    InputFatMassSegments,

    MetricSummary
};