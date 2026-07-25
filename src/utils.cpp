#include "utils.hpp"
#include <charconv>
#include <cstddef>
#include <sstream>

std::string normalize_name(const std::string& raw_name) {
    std::istringstream oss{raw_name};
    std::string res;
    while (oss >> res)
        res.push_back(' ');
    res.pop_back();
    return res;
}

void replace_by_vector(std::string& text, const std::vector<std::string>& text_replace) {
    size_t pos = 0;
    for (const auto& str : text_replace) {
        pos = text.find("{}", pos);
        text.replace(pos, 2, str);
    }
}

int normalize_age(const std::string& text) noexcept {
    int res = 0;
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), res);
    if (ec != std::errc{} || ptr != text.data() + text.size())
        return 0;
    return res;
}