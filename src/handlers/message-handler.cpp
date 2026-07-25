#include "handlers/message-handler.hpp"
#include "bot/user-session.hpp"
#include "bot/user-states.hpp"
#include "utils.hpp"
#include <cstddef>
#include <exception>
#include <tgbot/types/InlineKeyboardMarkup.h>

#include <memory>
#include <tgbot/types/Message.h>

// dev
void MessageHandler::handleFileMessage(const TgBot::Message::Ptr& message) {
    UserSession& session = userSessionManager_.get_session(message);

    deleteMessage(message);
}

void MessageHandler::handleTextMessage(const TgBot::Message::Ptr& message) {
    int64_t userId = message->from->id;
    UserSession& session = userSessionManager_.get_session(message);

    switch (session.currentState) {
    case UserStates::RegisrationNeed:
        registerHandler(message);
        break;
    case UserStates::InputUserName:
        userNameHandler(message);
        break;
    case UserStates::InputUserAge:
        userAgeHandler(message);
        break;
    case UserStates::InputUserSex:
        userSexHandler(message);
        break;
    // case UserStates::InputWeight:
    //     weightHandler(message);
    //     break;
    // case UserStates::InputHeight:
    //     heightHandler(message);
    //     break;
    // case UserStates::InputMuscleMass:
    //     muscleMassHandler(message);
    //     break;
    // case UserStates::InputBodyFatMass:
    //     bodyFatMassHandler(message);
    //     break;
    // case UserStates::InputWaterMass:
    //     waterMassHandler(message);
    //     break;
    // case UserStates::InputBoneMass:
    //     boneMassHandler(message);
    //     break;
    // case UserStates::InputVisceralFat:
    //     visceralFatHandler(message);
    //     break;
    // case UserStates::InputProteinMass:
    //     proteinMassHandler(message);
    //     break;
    // case UserStates::InputMuscleMassSegments:
    //     muscleMassSegmentsHandler(message);
    //     break;
    // case UserStates::InputFatMassSegments:
    //     fatMassSegmentsHandler(message);
    //     break;
    default:
        // bot_.getApi().deleteMessage(message->chat->id, message->messageId);
        break;
    }
}

void MessageHandler::handleMessage(const TgBot::Message::Ptr& message) {
    std::cout << "msgHdl " << message->text << "\n";
    UserSession& session = userSessionManager_.get_session(message);

    if (!session.userData.has_value()) {
        session.currentState = UserStates::RegisrationNeed;
        session.userData = UserData{};
    }

    if (message->document) {
        handleFileMessage(message);
    } else if (!message->text.empty() && message->text[0] != '/') {
        handleTextMessage(message);
    }
    // else {
    //     deleteMessage(message);
    // }
}

// игнорируем любой текст, ждем нажатия кнопки
void MessageHandler::registerHandler(const TgBot::Message::Ptr& message) {
    deleteMessage(message);
}

void MessageHandler::userNameHandler(const TgBot::Message::Ptr& message) {
    static constexpr size_t MIN_NAME_SIZE = 2;
    static constexpr size_t MAX_NAME_SIZE = 64;
    deleteMessage(message);
    auto& user_session = userSessionManager_.get_session(message);
    auto name = normalize_name(message->text);

    if (name.size() < MIN_NAME_SIZE || name.size() > MAX_NAME_SIZE) {
        editTextAndKeyboard("invalid_name", user_session.chatId, user_session.lastMessageId, {},
                            {{0, {message->from->firstName}}});
    } else {
        user_session.userData->user_name = name;
        editTextAndKeyboard("request_age", user_session.chatId, user_session.lastMessageId);
        user_session.currentState = UserStates::InputUserAge;
    }
}

void MessageHandler::editTextAndKeyboard(
    const std::string& key, int64_t chat_id, int32_t id,
    const std::vector<std::string>& text_replace,
    const std::unordered_map<size_t, std::vector<std::string>>& button_replace) {
    if (chat_id == 0 || id == 0) {
        std::cout << "edit failed\n";
    }
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

void MessageHandler::userAgeHandler(const TgBot::Message::Ptr& message) {
    static constexpr int MIN_AGE = 7;
    static constexpr int MAX_AGE = 130;
    deleteMessage(message);
    auto& user_session = userSessionManager_.get_session(message);
    int age = normalize_age(message->text);

    if (MIN_AGE > age || age > MAX_AGE) {
        editTextAndKeyboard("invalid_age", user_session.chatId, user_session.lastMessageId);
    } else {
        user_session.userData->age = age;
        editTextAndKeyboard("request_gender", user_session.chatId, user_session.lastMessageId);
        user_session.currentState = UserStates::InputUserSex;
    }
}

void MessageHandler::userSexHandler(const TgBot::Message::Ptr& message) {
    deleteMessage(message);
}

void MessageHandler::deleteMessage(const TgBot::Message::Ptr& message) try {
    bot_.getApi().deleteMessage(message->chat->id, message->messageId);
} catch (const std::exception&) { std::cout << "delete failed\n"; }
