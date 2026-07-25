#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

class MessageLoader {
  private:
    nlohmann::json messages;

    void loadMessages();

  public:
    nlohmann::json getMessage(const std::string& key);

    MessageLoader();
    ~MessageLoader() = default;
};