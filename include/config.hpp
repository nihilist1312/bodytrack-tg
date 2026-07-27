#pragma once

#include <filesystem>
#include <string>

struct Config {
    std::string bot_token;
    std::filesystem::path db_path = "data/bodytrack.db";
};

Config loadConfig();