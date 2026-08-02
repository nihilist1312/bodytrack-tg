#pragma once

#include "models/body-metrics.hpp"

#include <optional>
#include <string>

[[nodiscard]] std::string normalizeName(const std::string& raw_name);

[[nodiscard]]
std::optional<SegmentMass> getSegmentsMass(const std::string& text) noexcept;
