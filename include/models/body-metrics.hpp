#pragma once

#include <cstdint>
#include <optional>
#include <string>

struct SegmentMass {
    double leftArm;
    double rightArm;
    double leftLeg;
    double rightLeg;
    double trunk;
};

// храним данные о метриках тела пользователя в абсолютных значениях, kg, cm
struct BodyMetrics {
    // id of the record in the database, not the user id
    int64_t id;
    int64_t user_id;
    std::string date; // format: YYYY-MM-DD, for example: 2026-07-23

    // necessary
    double weight;
    double height;
    uint8_t age;
    double muscleMass;
    double bodyFatMass;

    // additional
    std::optional<double> waterMass;
    std::optional<double> boneMass;
    // value from 1 to 10, where 1 is the lowest and 10 is the highest
    std::optional<uint8_t> visceralFat;
    std::optional<double> proteinMass;

    // segment mass
    std::optional<SegmentMass> muscleMassSegments;
    std::optional<SegmentMass> bodyFatMassSegments;
};