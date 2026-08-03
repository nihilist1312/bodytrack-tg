#include "validation/body-metrics-validator.hpp"

#include "models/body-metrics.hpp"

#include <boost/system/detail/error_category.hpp>

#include <spdlog/spdlog.h>

[[nodiscard]]
bool validateBodyMetrics(const BodyMetrics& metrics) noexcept {
    // суммируем мышцы с жиром и сравнивает с общей массой
    if (metrics.muscle_mass + metrics.fat_mass > metrics.weight) {
        return false;
    }

    // проверка что ДОП. массы укладываюся в общий вес
    double total = 0.;
    total += (metrics.water_mass) ? metrics.water_mass.value() : 0.;
    total += (metrics.protein_mass) ? metrics.protein_mass.value() : 0.;
    total += (metrics.bone_mass) ? metrics.bone_mass.value() : 0.;
    total += metrics.fat_mass;
    if (total - metrics.weight > .1) {
        return false;
    }

    // проверка что сумма масс сегментов равна общей массе группы
    if (metrics.segment_muscle_mass &&
        !validateSegmentMass(metrics.segment_muscle_mass.value(),
                             metrics.weight)) {
        return false;
    }
    if (metrics.segment_fat_mass &&
        !validateSegmentMass(metrics.segment_fat_mass.value(),
                             metrics.fat_mass)) {
        return false;
    }

    return true;
}

[[nodiscard]]
bool validateSegmentMass(const SegmentMass& segments, double total) noexcept {
    double total_sum = 0.;
    total_sum += (segments.left_arm) ? segments.left_arm.value() : 0.;
    total_sum += (segments.right_arm) ? segments.right_arm.value() : 0.;
    total_sum += (segments.left_leg) ? segments.left_leg.value() : 0.;
    total_sum += (segments.right_leg) ? segments.right_leg.value() : 0.;
    total_sum += (segments.trunk) ? segments.trunk.value() : 0.;

    if (total_sum - total > .1) {
        spdlog::debug("Invalid Segment mass");
        return false;
    } else {
        return true;
    }
}