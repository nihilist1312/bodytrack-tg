#include "models/body-metrics.hpp"

#include "utils/conversions.hpp"
#include "utils/date.hpp"
#include "utils/formatting.hpp"
#include "utils/input_parser.hpp"

#include <chrono>
#include <optional>

namespace {
    constexpr double MIN_MASS = 0.;
    constexpr double MAX_MASS = 300.;
    constexpr double MIN_HEIGHT = 150.;
    constexpr double MAX_HEIGHT = 250.;
    constexpr int MIN_YEAR = 2000;
} // namespace

[[nodiscard]] double fatPercentage(const BodyMetrics& metric) noexcept {
    return metric.fat_mass / metric.weight * 100;
}

bool setDate(BodyMetrics& target, const std::string& text) {
    static constexpr std::chrono::year min_year{2000};
    auto date = getDate(text);
    if (!date)
        return false;
    if (date->year() < min_year || date > getCurrentDate())
        return false;
    target.date = dateToString(*date);
    return true;
}

// чтобы не писать под каждое поле использую общее Mass(double&)
// позже добавлю унификацию, с проверкой реалистичности значений
bool setMass(double& target, std::string_view text) noexcept {
    std::optional<double> res = strToDouble(text);
    if (!res)
        return false;
    // ошибка в значении
    if (res <= MIN_MASS || res > MAX_MASS)
        return false;
    target = res.value();
    return true;
}

// версия с возвратом
[[nodiscard]]
std::optional<double> setMass(std::string_view text) noexcept {
    std::optional<double> res = strToDouble(text);
    if (!res)
        return std::nullopt;
    // ошибка в значении
    if (res <= MIN_MASS || res > MAX_MASS)
        return std::nullopt;

    return res;
}

bool setHeight(double& target, std::string_view text) noexcept {
    std::optional<double> res = strToDouble(text);
    if (!res)
        return false;
    // ошибка в значении
    if (res <= MIN_HEIGHT || res > MAX_HEIGHT)
        return false;
    target = res.value();
    return true;
}

[[nodiscard]]
bool haveAdditional(const BodyMetrics& metric) noexcept {
    return (metric.water_mass || metric.bone_mass || metric.visceral_fat ||
            metric.protein_mass || metric.segment_muscle_mass ||
            metric.segment_fat_mass);
}

[[nodiscard]] bool
allSegmetsFilled(const std::optional<SegmentMass>& segments) noexcept {
    return (segments && segments->left_arm && segments->right_arm &&
            segments->left_leg && segments->right_leg && segments->trunk);
}