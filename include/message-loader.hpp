#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <filesystem>
#include <string>

class MessageLoader {
  private:
    nlohmann::json messages;

  public:
    nlohmann::json getMessage(const std::string& key);

    // загружает все *.json файлы из директории, включая вложеные папки
    MessageLoader(const std::filesystem::path& path = "resource/messages");
};