#pragma once

#include <cstdint>
#include <string>

enum class Sex { Male, Female };

struct UserData {
    int64_t user_id;
    std::string user_name;
    int age;
    Sex sex;
};

bool set_name(UserData& data, const std::string& new_name);
bool set_age(UserData& data, const std::string& age_text);