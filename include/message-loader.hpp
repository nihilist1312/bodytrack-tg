#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <string>

class MessageLoader {
  private:
    nlohmann::json messages;

    void load_messages();

  public:
    nlohmann::json get_message(const std::string& key);

    MessageLoader();
    ~MessageLoader() = default;
};