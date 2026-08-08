#include "utils/date.hpp"

#include <chrono>

// Вовзращает текущую дату в виде ГГГГ-ММ-ДД
std::string getCurrentDateStr() {
    auto now = std::chrono::system_clock::now();
    return std::format("{:%Y-%m-%d}", now);
}

Date getCurrentDate() {
    return std::chrono::floor<std::chrono::days>(
        std::chrono::system_clock::now());
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