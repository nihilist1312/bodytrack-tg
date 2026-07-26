#pragma once

#include "bot/user-session.hpp"
#include "message-loader.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <tgbot/tgbot.h>
#include <tgbot/types/InlineKeyboardMarkup.h>

// data for loading message from json and replace placeholders
struct MessageTemplateData {
    std::string key;
    std::vector<std::string> text_placeholders;
    std::vector<std::vector<std::string>> buttons_placeholders;
};

// data message for sending
struct MessageData {
    std::string text;
    std::shared_ptr<TgBot::InlineKeyboardMarkup> keyboard;
};

class MessageService {
  public:
    MessageService(TgBot::Bot& bot, MessageLoader& message_loader)
        : bot_(bot), message_loader_(message_loader) {}

    void send_message(UserSession& session,
                      const MessageTemplateData& message_template);
    void send_message(UserSession& session, const MessageData& message_data);
    void edit_message(UserSession& session,
                      const MessageTemplateData& message_template);
    void delete_message(const TgBot::Message::Ptr& message);
    void delete_message(int64_t chat_id, int32_t message_id);
    void delete_last(UserSession& session);

  private:
    TgBot::Bot& bot_;
    MessageLoader& message_loader_;

    MessageData load_message(const MessageTemplateData& message);
};