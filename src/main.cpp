#include "bot/bot.hpp"
#include "logger.hpp"

#include <spdlog/spdlog.h>

int main() {
#ifdef BUILD_DEBUG
    Logger::init(spdlog::level::debug);
    spdlog::info("Debug mode");
#else
    spdlog::info("Release mode");
    Logger::init();
#endif
    bot_start();
}