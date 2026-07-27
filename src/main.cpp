#include "bot/bot.hpp"
#include "logger.hpp"

#include <spdlog/common.h>

int main() {
#ifdef BUILD_DEBUG
    Logger::init(spdlog::level::debug);
#else
    Logger::init();
#endif
    bot_start();
}