#pragma once

#include <string>
#include <vector>

// data for loading message from json and replace placeholders
struct MessageTemplate {
    std::string key;
    std::vector<std::string> text_placeholders;
    std::vector<std::vector<std::string>> buttons_placeholders;

    friend auto operator<=>(const MessageTemplate& a,
                            const MessageTemplate& b) = default;
};
