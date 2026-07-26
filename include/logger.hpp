#pragma once

#include <spdlog/common.h>

namespace Logger {
    void init(spdlog::level::level_enum level = spdlog::level::info);
}