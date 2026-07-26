#include "utils/input_parser.hpp"

#include <charconv>
#include <sstream>

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

int normalizeAge(const std::string& text) noexcept {
    int res = 0;
    auto [ptr, ec] =
        std::from_chars(text.data(), text.data() + text.size(), res);
    if (ec != std::errc{} || ptr != text.data() + text.size())
        return 0;
    return res;
}