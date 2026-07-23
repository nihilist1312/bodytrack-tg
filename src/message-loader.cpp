#include "message-loader.hpp"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>

class MessageLoader {
    private:
        nlohmann::json messages;

        void loadMessages();
    public:
        std::string getMessage(const std::string& key);


        MessageLoader() { loadMessages(); }
        ~MessageLoader() = default;
};

void MessageLoader::loadMessages() {
    // Load messages from a JSON file
    std::ifstream file("resource/messages.json");
    if (!file.is_open()) {
        throw std::runtime_error("Could not open messages.json");
    }

    file >> messages;    
}

std::string MessageLoader::getMessage(const std::string& key) {
    if (messages.contains(key)) {
        return messages[key].get<std::string>();
    } else {
        throw std::runtime_error("Message key not found: " + key);
    }
    
}