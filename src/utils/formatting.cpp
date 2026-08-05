#include "utils/formatting.hpp"

#include "models/body-metrics.hpp"
#include "utils/conversions.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

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
                       [](std::string_view v) { return std::string(v); },
                       [](size_t v) { return std::to_string(v); }},
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
additionalPlaceholders(const BodyMetrics& metric) noexcept {
    std::vector<PlaceholderValue> res(6);

    res[0] = (metric.water_mass) ? doubleToStr(metric.water_mass.value()) : "-";
    res[1] = (metric.bone_mass) ? doubleToStr(metric.bone_mass.value()) : "-";
    res[2] = (metric.visceral_fat)
                 ? PlaceholderValue(metric.visceral_fat.value())
                 : "-";
    res[3] =
        (metric.protein_mass) ? doubleToStr(metric.protein_mass.value()) : "-";
    res[4] = segmentMassPlaceholders(metric.segment_muscle_mass);
    res[5] = segmentMassPlaceholders(metric.segment_fat_mass);

    return res;
}

// возвращает плейсхолдер для сегментной массы в развернутом виде,
// или "-" если записи нет
[[nodiscard]] std::string segmentMassPlaceholders(
    const std::optional<SegmentMass>& segment_mass) noexcept {
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

// Возвращает строку со всеми имеющимеся ДОП метриками
// или "" если таковых нет
[[nodiscard]] std::string
additionalMetricsText(const BodyMetrics& metric) noexcept {
    // ------------------------ Шаблоны записей -------------------------------
    static constexpr auto header = "💧 Дополнительные показатели\n\n";

    static constexpr auto water_mass = "💧 Вода: {} кг\n";
    static constexpr auto bone_mass = "🦴 Костная масса: {} кг\n";
    static constexpr auto visceral_fat = "🫀 Висцеральный жир: {}\n";
    static constexpr auto protein_mass = "🧬 Белок: {} кг\n";

    static constexpr auto muscle_segments =
        "💪 Мышечная масса по сегментам\n\n{}\n\n";
    static constexpr auto fat_segments = "🔥 Жировая масса по сегментам\n\n{}";
    // ------------------------------------------------------------------------

    std::string res;
    if (!haveAdditional(metric)) {
        return "";
    }
    res += header;
    if (metric.water_mass)
        res += format(water_mass, {*metric.water_mass});
    if (metric.bone_mass)
        res += format(bone_mass, {*metric.bone_mass});
    if (metric.visceral_fat)
        res += format(visceral_fat, {*metric.visceral_fat});
    if (metric.protein_mass)
        res += format(protein_mass, {*metric.visceral_fat});

    if (metric.segment_muscle_mass) {
        res += format(muscle_segments,
                      {segmentMassPlaceholders(metric.segment_muscle_mass)});
    }
    if (metric.segment_fat_mass) {
        res += format(fat_segments,
                      {segmentMassPlaceholders(metric.segment_fat_mass)});
    }

    return res;
}