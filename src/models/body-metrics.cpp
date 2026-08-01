#include "models/body-metrics.hpp"

#include "utils/conversions.hpp"
#include "utils/date.hpp"

#include <regex>

[[nodiscard]] double fatPercentage(const BodyMetrics& metric) noexcept {
    return metric.fat_mass / metric.weight * 100;
}

bool setDate(BodyMetrics& target, const std::string& text) {
    static std::regex pat{R"((20\d{2})-(\d{2})-(\d{2}))"};
    std::smatch match;
    if (std::regex_match(text, match, pat)) {
        int year = strToInt(match[1].str()).value();
        int month = strToInt(match[2].str()).value();
        int day = strToInt(match[3].str()).value();
        if (year >= 2000 && getCurrentYear() >= year &&
            isDateCorrect(year, month, day)) {
            target.date = text;
            return true;
        }
    }
    return false;
}

// чтобы не писать под каждое поле использую общее Mass(double&)
// позже добавлю унификацию, с проверкой реалистичности значений
bool setMass(double& target, std::string_view text) noexcept {
    std::optional<double> res = strToDouble(text);
    if (!res)
        return false;
    // ошибка в значении
    if (res <= 0. || res > 500.)
        return false;
    target = res.value();
    return true;
}

bool setHeight(BodyMetrics& target, std::string_view text) noexcept {
    std::optional<double> res = strToDouble(text);
    if (!res)
        return false;
    // ошибка в значении
    if (res <= 150. || res > 250.)
        return false;
    target.height = res.value();
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