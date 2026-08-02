#pragma once

#include "models/body-metrics.hpp"

[[nodiscard]]
bool validateBodyMetrics(const BodyMetrics& metrics) noexcept;

[[nodiscard]]
bool validateSegmentMass(const SegmentMass& segments, double total) noexcept;