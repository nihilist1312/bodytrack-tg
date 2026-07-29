#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>

using PlaceholderValue = std::variant<int, double, std::string_view>;

// заменяет {}, на элемент из вектора. На месте
void format_inplace(std::string& text,
                    const std::vector<PlaceholderValue>& data);

// то же самое, но по значению и с возвратом
std::string format(std::string text, const std::vector<PlaceholderValue>& data);
