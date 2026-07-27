#pragma once

#include <cstdint>
#include <string>

enum class Sex { Male, Female };

struct UserData {
    int64_t user_id;
    std::string user_name;
    uint8_t age;
    Sex sex;

    bool set_name(const std::string& new_name);
    bool set_age(const std::string& age_text);
};