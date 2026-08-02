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
#include <tgbot/EventHandler.h>
#include <tgbot/net/TgLongPoll.h>
#include <tgbot/tgbot.h>
#include <tgbot/types/Message.h>

void bot_start() {
    // ---- Инициализация инструментов ----------------------------------------
    static Config config = loadConfig();
    // Создание директории для БД
    std::filesystem::create_directories(config.db_path.parent_path());
    spdlog::info("Auth token: {}", config.bot_token);
    static TgBot::Bot bot{config.bot_token};

    static Database database;
    static UserSessionManager session_manager{database};
    static MessageLoader message_loader;
    static MessageService message_service{bot, message_loader, database};

    MessageHandler message_handler{database, session_manager, message_service};
    CommandHandler command_handler{database, session_manager, message_service};
    CallbackHandler callback_handler{database, session_manager,
                                     message_service};

    // ---- Подключение обработчиков ------------------------------------------

    // ---- 1. Команды --------------------------------------------------------
    bot.getEvents().onCommand(
        "start", [&command_handler](const TgBot::Message::Ptr& message) {
            command_handler.onStart(message);
        });
    bot.getEvents().onCommand(
        "add", [&command_handler](const TgBot::Message::Ptr& message) {
            command_handler.onAdd(message);
        });

    // ---- 2. Текстовые сообщения и файлы ------------------------------------
    bot.getEvents().onAnyMessage(
        [&message_handler](const TgBot::Message::Ptr& message) {
            message_handler.handleMessage(message);
        });

    // ---- 3. Встроенная клавиатура ------------------------------------------
    bot.getEvents().onCallbackQuery(
        [&callback_handler](const TgBot::CallbackQuery::Ptr& query) {
            callback_handler.handleCallback(query);
        });

    // ---- Запуск бота -------------------------------------------------------
    // Пропуск старых сообщений
    auto updates = bot.getApi().getUpdates(-1, 0, 100);

    TgBot::TgLongPoll long_poll(bot);

    while (true) {
        long_poll.start();
    }
}
