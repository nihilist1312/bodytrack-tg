#include "message-loader.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <string>

nlohmann::json MessageLoader::getMessage(const std::string& key) {
    if (messages.contains(key)) {
        return messages[key];
    } else {
        throw std::runtime_error("Message key not found: " + key);
    }
}

MessageLoader::MessageLoader(const std::filesystem::path& path) {
    messages = nlohmann::json::object();

    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("Path does not exist: " + path.string());
    }

    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(path)) {
        if (!entry.is_regular_file())
            continue;
        if (entry.path().extension() != ".json")
            continue;

        std::ifstream file(entry.path());
        if (!file.is_open()) {
            throw std::runtime_error("Could not open " + entry.path().string());
        }

        nlohmann::json fileData;
        try {
            file >> fileData;
        } catch (const nlohmann::json::parse_error& e) {
            throw std::runtime_error("Parse error in " + entry.path().string() +
                                     ": " + e.what());
        }

        // Объединяет все файлы на одном уровне
        for (auto& [key, value] : fileData.items()) {
            messages[key] = value;
        }
    }
}