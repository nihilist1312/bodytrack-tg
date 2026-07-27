#include "models/user-data.hpp"

#include "utils/input_parser.hpp"

#include <cstddef>
#include <string>

bool UserData::set_name(const std::string& new_name) {
    static constexpr size_t MIN_NAME_LENGTH = 2;
    static constexpr size_t MAX_NAME_LENGTH = 64;
    std::string normal_name = normalizeName(new_name);
    if (MIN_NAME_LENGTH > normal_name.size() ||
        normal_name.size() > MAX_NAME_LENGTH) {
        return false;
    }
    user_name = normal_name;
    return true;
}

bool UserData::set_age(const std::string& age_text) {
    static constexpr int MIN_AGE = 7;
    static constexpr int MAX_AGE = 150;
    int normal_age = normalizeAge(age_text);
    if (MIN_AGE > normal_age || normal_age > MAX_AGE) {
        return false;
    }
    age = normal_age;
    return true;
}