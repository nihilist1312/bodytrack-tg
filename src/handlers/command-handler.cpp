#include "handlers/command-handler.hpp"
#include "bot/user-session.hpp"
#include "bot/user-states.hpp"
#include "utils.hpp"

void CommandHandler::handleStartCommand(const TgBot::Message::Ptr& message) {
    std::cout << "cmdHdl" << message->text << "\n";
    UserSession& session = userSessionManager_.get_session(message);
    if (session.lastMessageId != 0) {
        bot_.getApi().deleteMessage(message->chat->id, session.lastMessageId);
    }
    deleteMessage(message);

    if (session.currentState == UserStates::RegisrationNeed) {
        sendTextAndKeyboard("registration", session);
    } else if (session.currentState >= UserStates::InputUserName &&
               UserStates::InputUserSex >= session.currentState) {
        deleteMessage(message);
    } else {
        if (!session.lastBodyMetrics.has_value()) {
            session.lastBodyMetrics = database_.getLastBodyMetrics(message->from->id);
        }
        auto key = (session.lastBodyMetrics.has_value()) ? "main_menu" : "empty_menu";
        sendTextAndKeyboard(key, session);
    }
}

TgBot::Message::Ptr CommandHandler::sendTextAndKeyboard(
    const std::string& key, UserSession& session, const std::vector<std::string>& text_replace,
    const std::unordered_map<size_t, std::vector<std::string>>& button_replace) {
    auto message_json = messageLoader_.getMessage(key);
    std::string message_text = message_json["text"];
    if (!text_replace.empty()) {
        replace_by_vector(message_text, text_replace);
    }
    auto button_rows = message_json["buttons"];

    size_t i = 0;
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
    for (const auto& row : button_rows) {
        std::vector<TgBot::InlineKeyboardButton::Ptr> buttons;

        for (const auto& button : row) {
            auto btn = std::make_shared<TgBot::InlineKeyboardButton>();

            std::string text_button = button["text"];
            auto it = button_replace.find(i);
            if (it != button_replace.end()) {
                replace_by_vector(text_button, it->second);
            }
            btn->text = text_button;
            btn->callbackData = button["callback"];

            buttons.push_back(btn);
            ++i;
        }

        keyboard->inlineKeyboard.push_back(buttons);
    }

    auto ptr = bot_.getApi().sendMessage(session.chatId, message_text, nullptr, 0, keyboard);
    session.lastMessageId = ptr->messageId;
    return ptr;
}

void CommandHandler::deleteMessage(const TgBot::Message::Ptr& message) try {
    bot_.getApi().deleteMessage(message->chat->id, message->messageId);
} catch (const std::exception&) { std::cout << "delete failed\n"; }
