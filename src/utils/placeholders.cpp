#include "utils/placeholders.hpp"

#include "utils/conversions.hpp"

#include <string>
#include <string_view>
#include <variant>
#include <vector>

using PlaceholderValue = std::variant<int, double, std::string_view>;

// overloaded - класс с множествнной перегрузкой оператора (),
// компилятор сам определяет нужный
template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
// подсказка компилятору по определению шаблона
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void format_inplace(std::string& text,
                    const std::vector<PlaceholderValue>& data) {
    size_t pos = 0;
    for (const auto& elem : data) {
        pos = text.find("{}", pos);
        if (pos == std::string::npos) {
            break;
        }
        // вызовет нужную лямюду в соответствии с реальным типом elem
        std::string str = std::visit(
            overloaded{[](int v) { return std::to_string(v); },
                       [](double v) { return doubleToStr(v); },
                       [](std::string_view v) { return std::string(v); }},
            elem);
        text.replace(pos, 2, str);
        pos += str.size(); // сдвигаемся за вставленную строку
    }
}

// Обёртка для удобства: по значению, с возвратом
std::string format(std::string text,
                   const std::vector<PlaceholderValue>& data) {
    format_inplace(text, data);
    return text;
}