cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

project(bodytrack-tg)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(bodytrack-tg
    src/main.cpp
    src/bot/message-service.cpp
    src/bot/bot.cpp
    src/bot/user-session.cpp
    src/handlers/callback-handler.cpp
    src/handlers/command-handler.cpp
    src/handlers/message-handler.cpp
    src/models/user-data.cpp
    src/config.cpp
    src/database.cpp
    src/message-loader.cpp
    src/utils/placeholders.cpp
    src/utils/input_parser.cpp
    src/logger.cpp
    src/utils/date.cpp
    src/utils/conversions.cpp
    src/models/body-metrics.cpp
)

target_include_directories(bodytrack-tg PRIVATE include)

find_package(TgBot CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)

target_link_libraries(bodytrack-tg PRIVATE
    TgBot::TgBot
    SQLiteCpp
    SQLite3::SQLite3
    spdlog::spdlog
    utf8cpp::utf8cpp
    nlohmann_json::nlohmann_json
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BUILD_DEBUG)
else()
    add_compile_definitions(BUILD_RELEASE)
endif()