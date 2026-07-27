#pragma once

#include <string>

struct Config {
    std::string bot_token;
};

Config loadConfig();