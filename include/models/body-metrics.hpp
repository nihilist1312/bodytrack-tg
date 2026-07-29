#pragma once

#include <cstdint>
#include <optional>
#include <string>

struct SegmentMass {
    double left_arm;
    double right_arm;
    double left_leg;
    double right_leg;
    double trunk;
};

// храним данные о метриках тела пользователя в абсолютных значениях, kg, cm
struct BodyMetrics {
    // id of the record in the database, not the user id
    std::optional<int64_t> id;
    int64_t user_id;
    std::string date; // format: YYYY-MM-DD, for example: 2026-07-23

    // necessary
    double weight;
    double height;
    uint8_t age;
    double muscle_mass;
    double fat_mass;

    // additional
    std::optional<double> water_mass;
    std::optional<double> bone_mass;
    // value from 1 to 10, where 1 is the lowest and 10 is the highest
    std::optional<uint8_t> visceral_fat;
    std::optional<double> protein_mass;

    // segment mass
    std::optional<SegmentMass> segment_muscle_mass;
    std::optional<SegmentMass> segment_fat_mass;
};