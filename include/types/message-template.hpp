#pragma once

#include "utils/placeholders.hpp"

#include <string>
#include <vector>

// data for loading message from json and replace placeholders
struct MessageTemplate {
    std::string key;
    std::vector<PlaceholderValue> text_placeholders;
    std::vector<std::vector<PlaceholderValue>> buttons_placeholders;

    friend auto operator<=>(const MessageTemplate& a,
                            const MessageTemplate& b) = default;
};
