#include "utils/date.hpp"

#include <chrono>

std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    return std::format("{:%Y-%m-%d}", now);
}

int getCurrentYear() {
    auto now = std::chrono::system_clock::now();
    auto today = std::chrono::floor<std::chrono::days>(now);
    std::chrono::year_month_day ymd{today};

    int year = static_cast<int>(ymd.year());
    return year;
}

bool isDateCorrect(int y, int m, int d) {
    using namespace std::chrono;
    year_month_day date{year(y), month(m), day(d)};
    return date.ok();
}