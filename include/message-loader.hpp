#pragma once

#include <string>
#include <nlohmann/json.hpp>

class MessageLoader {
    private:
        nlohmann::json messages;

        void loadMessages();
    public:
        std::string getMessage(const std::string& key);


        MessageLoader();
        ~MessageLoader() = default;
};