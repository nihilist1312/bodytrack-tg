#include "logger.hpp"

#include <spdlog/common.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Logger {
    void init(spdlog::level::level_enum level) {
        auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        auto file = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/bodytrack.log", 1024 * 1024 * 10, 5);

        std::vector<spdlog::sink_ptr> sinks{console, file};

        auto logger = std::make_shared<spdlog::logger>(
            "bodytrack", sinks.begin(), sinks.end());

        spdlog::set_default_logger(logger);
        spdlog::set_level(level);

        spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");
    }
} // namespace Logger