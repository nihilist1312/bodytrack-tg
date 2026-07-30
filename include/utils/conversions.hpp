#pragma once

#include <optional>
#include <string>
#include <string_view>

std::optional<int> strToInt(std::string_view text) noexcept;

// double -> string, p - точность
std::string doubleToStr(double number, int p = 1) noexcept;

std::optional<double> strToDouble(std::string_view text) noexcept;