#pragma once

#include <string>

struct Config {
    std::string botToken;
};

Config loadConfig();