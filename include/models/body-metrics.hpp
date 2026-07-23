#pragma once

#include <cstdint>
#include <optional>

struct SegmentMass {
    double leftArm;
    double rightArm;
    double leftLeg;
    double rightLeg;
    double trunk;
};

// храним данные о метриках тела пользователя в абсолютных значениях, kg, cm
struct BodyMetrics {
    int64_t id;

    // main
    double weight;
    double height;
    uint8_t age;
    double muscleMass;
    double bodyFatMass;
    double waterMass;

    // additional
    std::optional<double> boneMass;
    std::optional<uint8_t> visceralFat;
    std::optional<double> proteinMass;

    // segment mass
    std::optional<SegmentMass> muscleMassSegments;
    std::optional<SegmentMass> bodyFatMassSegments;
};