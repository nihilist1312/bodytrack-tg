#include "message-loader.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <string>

void MessageLoader::loadMessages() {
    // Load messages from a JSON file
    std::ifstream file("resource/messages.json");
    if (!file.is_open()) {
        throw std::runtime_error("Could not open messages.json");
    }

    file >> messages;
}

nlohmann::json MessageLoader::getMessage(const std::string& key) {
    if (messages.contains(key)) {
        return messages[key];
    } else {
        throw std::runtime_error("Message key not found: " + key);
    }
}

MessageLoader::MessageLoader() {
    loadMessages();
}