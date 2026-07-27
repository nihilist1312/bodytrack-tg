#pragma once

#include "bot/user-session.hpp"
#include "message-loader.hpp"
#include "types/message-data.hpp"
#include "types/message-template.hpp"

#include <cstdint>

#include <tgbot/tgbot.h>
#include <tgbot/types/InlineKeyboardMarkup.h>

class MessageService {
  public:
    MessageService(TgBot::Bot& bot, MessageLoader& message_loader)
        : bot_(bot), message_loader_(message_loader) {}

    void sendMessage(UserSession& session,
                     const MessageTemplate& message_template);
    void sendMessage(UserSession& session,
                     const MessageTemplate& message_template,
                     const MessageData& message_data);
    void editMessage(UserSession& session,
                     const MessageTemplate& message_template);
    void deleteMessage(const TgBot::Message::Ptr& message);
    void deleteMessage(int64_t chat_id, int32_t message_id);
    void deleteLast(UserSession& session);

  private:
    TgBot::Bot& bot_;
    MessageLoader& message_loader_;

    MessageData loadMessage(const MessageTemplate& message);
};