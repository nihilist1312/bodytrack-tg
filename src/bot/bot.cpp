#include "bot/bot.hpp"

#include "bot/user-session.hpp"
#include "config.hpp"
#include "database/database.hpp"
#include "handlers/callback-handler.hpp"
#include "handlers/command-handler.hpp"
#include "handlers/message-handler.hpp"
#include "message-loader.hpp"
#include <tgbot/net/TgLongPoll.h>
#include <tgbot/tgbot.h>
#include <tgbot/types/Message.h>

void bot_start() {
    // init
    static Config config = loadConfig();
    static TgBot::Bot bot{config.botToken};
    static Database database;
    static UserSessionManager session_manager{database};
    static MessageLoader message_loader;

    MessageHandler message_handler{bot, database, session_manager, message_loader};
    CommandHandler command_handler{bot, database, session_manager, message_loader};
    CallbackHandler callback_handler{bot, database, session_manager, message_loader};

    // bind handlers

    // commands
    bot.getEvents().onCommand("start", [&command_handler](const TgBot::Message::Ptr& message) {
        command_handler.handleStartCommand(message);
    });

    bot.getEvents().onAnyMessage([&message_handler](const TgBot::Message::Ptr& message) {
        message_handler.handleMessage(message);
    });

    bot.getEvents().onCallbackQuery([&callback_handler](const TgBot::CallbackQuery::Ptr& query) {
        callback_handler.handleCallback(query);
    });

    TgBot::TgLongPoll long_poll{bot};
    while (true)
        try {
            long_poll.start();
        } catch (...) { continue; }
}