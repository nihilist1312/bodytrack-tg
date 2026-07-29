#pragma once

#include "bot/user-session.hpp"
#include "database/database.hpp"
#include "message-loader.hpp"
#include "types/message-data.hpp"
#include "types/message-template.hpp"

#include <cstdint>

#include <tgbot/tgbot.h>
#include <tgbot/types/InlineKeyboardMarkup.h>

class MessageService {
  public:
    MessageService(TgBot::Bot& bot, MessageLoader& message_loader,
                   Database& database)
        : bot_(bot), message_loader_(message_loader), database_(database) {}

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

    // message printer
    void openMainMenu(UserSession& session);

    void requestDate(UserSession& session);
    void requestWeight(UserSession& session);
    void requestHeight(UserSession& session);
    void requestMuscleMass(UserSession& session);
    void requestFatMass(UserSession& session);

    void invalidDate(UserSession& session);
    void invalidWeight(UserSession& session);
    void invalidHeight(UserSession& session);
    void invalidMuscleMass(UserSession& session);
    void invalidFatMass(UserSession& session);

    void printSummary(UserSession& session);

  private:
    TgBot::Bot& bot_;
    MessageLoader& message_loader_;
    Database& database_;

    MessageData loadMessage(const MessageTemplate& message);
};