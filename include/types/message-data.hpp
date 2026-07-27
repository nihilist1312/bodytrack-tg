#pragma once

#include <memory>
#include <string>

#include <tgbot/types/InlineKeyboardMarkup.h>

// data message for sending
struct MessageData {
    std::string text;
    std::shared_ptr<TgBot::InlineKeyboardMarkup> keyboard;
};