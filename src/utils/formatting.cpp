#include "utils/formatting.hpp"

#include "utils/conversions.hpp"

#include <string>
#include <string_view>
#include <variant>
#include <vector>

using PlaceholderValue = std::variant<int, double, std::string>;

namespace {
    // overloaded - класс с множествнной перегрузкой оператора (),
    // компилятор сам определяет нужный
    template <class... Ts> struct overloaded : Ts... {
        using Ts::operator()...;
    };
    // подсказка компилятору по определению шаблона
    template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    auto toPlaceholder = [](const auto& opt) -> PlaceholderValue {
        return opt ? PlaceholderValue(*opt) : PlaceholderValue("-");
    };
} // namespace

void formatInplace(std::string& text,
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
    formatInplace(text, data);
    return text;
}

// возвращает плейсхолдеры для дополнительных метрик,
// или "-", если таковая отсутствует
[[nodiscard]] std::vector<PlaceholderValue>
additionalFormat(const BodyMetrics& metric) noexcept {
    std::vector<PlaceholderValue> res(6);

    res[0] = (metric.water_mass) ? doubleToStr(metric.water_mass.value()) : "-";
    res[1] = (metric.bone_mass) ? doubleToStr(metric.bone_mass.value()) : "-";
    res[2] = (metric.visceral_fat)
                 ? PlaceholderValue(metric.visceral_fat.value())
                 : "-";
    res[3] =
        (metric.protein_mass) ? doubleToStr(metric.protein_mass.value()) : "-";
    res[4] = segmentMassFormat(metric.segment_muscle_mass);
    res[5] = segmentMassFormat(metric.segment_fat_mass);

    return res;
}

// возвращает плейсхолдер для сегментной массы в развернутом виде,
// или "-" если записи нет
[[nodiscard]] std::string
segmentMassFormat(const std::optional<SegmentMass>& segment_mass) noexcept {
    constexpr static auto temp = "• Левая рука: {}\n• Правая рука: {}\n• Левая "
                                 "нога: {}\n• Правая нога: {}\n• Туловище: {}";
    if (segment_mass) {
        return format(temp, {toPlaceholder(segment_mass->left_arm),
                             toPlaceholder(segment_mass->right_arm),
                             toPlaceholder(segment_mass->left_leg),
                             toPlaceholder(segment_mass->right_leg),
                             toPlaceholder(segment_mass->trunk)});
    } else
        return "-";
}