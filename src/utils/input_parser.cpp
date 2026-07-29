#include "utils/input_parser.hpp"

#include "utils/date.hpp"

#include <charconv>
#include <regex>
#include <sstream>
#include <string>

std::string normalizeName(const std::string& raw_name) {
    std::istringstream oss{raw_name};
    std::string res;
    std::string temp;
    while (oss >> temp) {
        res += temp + " ";
    }
    res.pop_back();
    return res;
}

int parseIntStrict(std::string_view text) noexcept {
    int res = 0;
    auto [ptr, ec] =
        std::from_chars(text.data(), text.data() + text.size(), res);
    if (ec != std::errc{} || ptr != text.data() + text.size())
        return -1;
    return res;
}

int normalizeAge(std::string_view text) noexcept {
    return parseIntStrict(text);
}

bool setDate(std::string& target, const std::string& text) {
    static std::regex pat{R"((20\d{2})-(\d{2})-(\d{2}))"};
    std::smatch match;
    if (std::regex_match(text, match, pat)) {
        int year = parseIntStrict(match[1].str());
        int month = parseIntStrict(match[2].str());
        int day = parseIntStrict(match[3].str());
        if (year >= 2000 && getCurrentYear() <= year &&
            isDateCorrect(year, month, day)) {
            target = std::to_string(year) + "-" + std::to_string(month) + "-" +
                     std::to_string(day);
            return true;
        }
    }
    return false;
}

bool setMass(double& target, std::string_view text) noexcept {
    double res = 0.;
    auto [ptr, ec] =
        std::from_chars(text.data(), text.data() + text.size(), res);
    // ошибка в тексте
    if (ec != std::errc{} || ptr != text.data() + text.size())
        return false;
    // ошибка в значении
    if (res <= 0. || res > 500.)
        return false;
    target = res;
    return true;
}

bool setHeight(double& target, std::string_view text) noexcept {
    double res = 0.;
    auto [ptr, ec] =
        std::from_chars(text.data(), text.data() + text.size(), res);
    // ошибка в тексте
    if (ec != std::errc{} || ptr != text.data() + text.size())
        return false;
    // ошибка в значении
    if (res <= 150. || res > 250.)
        return false;
    target = res;
    return true;
}