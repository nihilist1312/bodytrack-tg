#pragma once

#include <cstdint>
#include <optional>
#include <string>

struct SegmentMass {
    std::optional<double> left_arm;
    std::optional<double> right_arm;
    std::optional<double> left_leg;
    std::optional<double> right_leg;
    std::optional<double> trunk;
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
    int age;
    double muscle_mass;
    double fat_mass;

    // additional
    std::optional<double> water_mass;
    std::optional<double> bone_mass;
    // value from 1 to 10, where 1 is the lowest and 10 is the highest
    std::optional<int> visceral_fat;
    std::optional<double> protein_mass;

    // segment mass
    std::optional<SegmentMass> segment_muscle_mass;
    std::optional<SegmentMass> segment_fat_mass;
};

// get body fat percent
[[nodiscard]]
double fatPercentage(const BodyMetrics& metric) noexcept;

bool setDate(BodyMetrics& target, const std::string& text);
bool setMass(double& target, std::string_view text) noexcept;
bool setHeight(BodyMetrics& target, std::string_view text) noexcept;

// true, if have someone additional metric
[[nodiscard]]
bool haveAdditional(const BodyMetrics& metrics) noexcept;

[[nodiscard]]
bool allSegmetsFilled(const std::optional<SegmentMass>& segments) noexcept;