#pragma once

#include <concepts>

template <std::unsigned_integral T>
constexpr T ceil_div(T numerator, T denominator) {
    return numerator / denominator + (numerator % denominator != 0);
}