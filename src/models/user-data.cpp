#include "models/user-data.hpp"

#include "utils/conversions.hpp"
#include "utils/input_parser.hpp"

#include <cstddef>
#include <optional>
#include <string>

#include <utf8/checked.h>
#include <utf8cpp/utf8.h>

bool set_name(UserData& data, const std::string& new_name) {
    static constexpr size_t MIN_NAME_LENGTH = 2;
    static constexpr size_t MAX_NAME_LENGTH = 64;
    std::string normal_name = normalizeName(new_name);
    auto length = utf8::distance(normal_name.begin(), normal_name.end());
    if (MIN_NAME_LENGTH > length || length > MAX_NAME_LENGTH) {
        return false;
    }
    data.user_name = std::move(normal_name);
    return true;
}

bool set_age(UserData& data, const std::string& age_text) {
    static constexpr int MIN_AGE = 7;
    static constexpr int MAX_AGE = 150;
    std::optional<int> age = strToInt(age_text);
    if (!age || MIN_AGE > age || age > MAX_AGE) {
        return false;
    }
    data.age = age.value();
    return true;
}