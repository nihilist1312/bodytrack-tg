#include "config.hpp"

#include <cstdlib>
#include <stdexcept>

Config loadConfig() {
    Config config;
    // Load the bot token from an environment variable
    const char* token = std::getenv("BOT_TOKEN");
    if (token) {
        config.botToken = token;
    } else {
        throw std::runtime_error("BOT_TOKEN environment variable is not set.");
    }
    return config;
}