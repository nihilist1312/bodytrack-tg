#include "utils/date.hpp"

#include <chrono>

std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    return std::format("{:%d.%m.%Y}", now);
}