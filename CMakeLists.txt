cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

project(bodytrack-tg)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(TgBot CONFIG REQUIRED)

add_executable(bodytrack-tg
    src/main.cpp
    src/bot/message-service.cpp
    src/bot/bot.cpp
    src/bot/user-session.cpp
    src/handlers/callback-handler.cpp
    src/handlers/command-handler.cpp
    src/handlers/message-handler.cpp
    src/config.cpp
    src/database.cpp
    src/message-loader.cpp
    src/utils.cpp
)

target_link_libraries(bodytrack-tg PRIVATE
    TgBot::TgBot
)

target_include_directories(bodytrack-tg PRIVATE include)

find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(bodytrack-tg PRIVATE nlohmann_json::nlohmann_json)

find_package(SQLite3 REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)

target_link_libraries(bodytrack-tg PRIVATE
    SQLiteCpp
    SQLite3::SQLite3
)