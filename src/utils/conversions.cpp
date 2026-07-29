#include "utils/conversions.hpp"

#include <array>
#include <charconv>

int strToInt(std::string_view text) noexcept {
    int res = 0;
    auto [ptr, ec] =
        std::from_chars(text.data(), text.data() + text.size(), res);
    if (ec != std::errc{} || ptr != text.data() + text.size())
        return -1;
    return res;
}

std::string doubleToStr(double v, int p) noexcept {
    std::array<char, 32> buf;
    auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), v,
                                   std::chars_format::fixed, p);
    return std::string(buf.data(), ptr);
}