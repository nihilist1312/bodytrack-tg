#include "handlers/callback-handler.hpp"
#include "bot/user-session.hpp"
#include "bot/user-states.hpp"
#include "utils.hpp"

void CallbackHandler::handleCallback(const TgBot::CallbackQuery::Ptr& query) {
    if (query->data.starts_with("reg:")) {
        registrationCallbackHandler(query);
    }
}

void CallbackHandler::registrationCallbackHandler(const TgBot::CallbackQuery::Ptr& query) {
    UserSession& session = sessionManager_.get_session(query->message);
    if (query->data.ends_with("start")) {
        editTextAndKeyboard("request_name", query->message->chat->id, query->message->messageId);
        session.currentState = UserStates::InputUserName;
    } else if (query->data.ends_with("default_name")) {
        session.userData->user_name = query->from->firstName;
        editTextAndKeyboard("request_age", query->message->chat->id, query->message->messageId);
        session.currentState = UserStates::InputUserAge;
    } else if (query->data.ends_with("male")) {
        session.userData->sex = Sex::Male;
        editTextAndKeyboard("empty_menu", query->message->chat->id, query->message->messageId);
        database_.addUser(query->from->id, session.userData.value());
        session.currentState = UserStates::MainMenu;
    } else if (query->data.ends_with("female")) {
        session.userData->sex = Sex::Female;
        editTextAndKeyboard("empty_menu", query->message->chat->id, query->message->messageId);
        database_.addUser(query->from->id, session.userData.value());
        session.currentState = UserStates::MainMenu;
    }
}

void CallbackHandler::deleteMessage(const TgBot::Message::Ptr& message) {
    bot_.getApi().deleteMessage(message->chat->id, message->messageId);
}

void CallbackHandler::editTextAndKeyboard(
    const std::string& key, int64_t chat_id, int32_t id,
    const std::vector<std::string>& text_replace,
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

    bot_.getApi().editMessageText(message_text, chat_id, id, "", "", nullptr, keyboard);
}