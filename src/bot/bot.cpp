#include "bot/bot.hpp"

#include "bot/message-service.hpp"
#include "bot/user-session.hpp"
#include "config.hpp"
#include "database/database.hpp"
#include "handlers/callback-handler.hpp"
#include "handlers/command-handler.hpp"
#include "handlers/message-handler.hpp"
#include "message-loader.hpp"

#include <filesystem>

#include <spdlog/spdlog.h>
#include <tgbot/net/TgLongPoll.h>
#include <tgbot/tgbot.h>
#include <tgbot/types/Message.h>

void bot_start() {
    // init
    static Config config = loadConfig();
    // create database directories
    std::filesystem::create_directories(config.db_path.parent_path());
    spdlog::info("Auth token: {}", config.bot_token);
    static TgBot::Bot bot{config.bot_token};

    // create folder for
    static Database database;
    static UserSessionManager session_manager{database};
    static MessageLoader message_loader;
    static MessageService message_service{bot, message_loader, database};

    // пропускаем старые сообщения
    // bot.getApi().getUpdates(0, 0, 0, 0);

    MessageHandler message_handler{database, session_manager, message_service};
    CommandHandler command_handler{database, session_manager, message_service};
    CallbackHandler callback_handler{database, session_manager,
                                     message_service};

    // bind handlers

    // commands
    bot.getEvents().onCommand(
        "start", [&command_handler](const TgBot::Message::Ptr& message) {
            command_handler.onStart(message);
        });

    bot.getEvents().onAnyMessage(
        [&message_handler](const TgBot::Message::Ptr& message) {
            message_handler.handleMessage(message);
        });

    bot.getEvents().onCallbackQuery(
        [&callback_handler](const TgBot::CallbackQuery::Ptr& query) {
            callback_handler.handleCallback(query);
        });

    auto updates = bot.getApi().getUpdates(-1, 0, 100);

    TgBot::TgLongPoll long_poll(bot);

    while (true) {
        long_poll.start();
    }
}
