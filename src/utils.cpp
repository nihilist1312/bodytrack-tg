#include "utils.hpp"

#include <charconv>
#include <cstddef>
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

void replaceByVector(std::string& text,
                     const std::vector<std::string>& text_replace) {
    size_t pos = 0;
    for (const auto& str : text_replace) {
        pos = text.find("{}", pos);
        if (pos == std::string::npos) {
            break; // или throw, если такая ситуация — ошибка
        }
        text.replace(pos, 2, str);
        pos += str.size(); // сдвигаемся за вставленную строку
    }
}

int normalizeAge(const std::string& text) noexcept {
    int res = 0;
    auto [ptr, ec] =
        std::from_chars(text.data(), text.data() + text.size(), res);
    if (ec != std::errc{} || ptr != text.data() + text.size())
        return 0;
    return res;
}