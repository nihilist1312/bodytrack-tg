#include "models/body-metrics.hpp"

[[nodiscard]]
double fatPercentage(const BodyMetrics& metric) noexcept {
    return metric.fat_mass / metric.weight * 100;
}