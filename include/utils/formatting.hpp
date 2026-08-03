#pragma once

#include "models/body-metrics.hpp"

#include <cstddef>
#include <string>
#include <variant>
#include <vector>

using PlaceholderValue = std::variant<int, double, std::string, size_t>;

// заменяет {}, на элемент из вектора. На месте
void formatInplace(std::string& text,
                   const std::vector<PlaceholderValue>& data);

// то же самое, но по значению и с возвратом
[[nodiscard]] std::string format(std::string text,
                                 const std::vector<PlaceholderValue>& data);

// возвращает плейсхолдеры для дополнительных метрик,
// или "-", если таковая отсутствует
[[nodiscard]] std::vector<PlaceholderValue>
additionalFormat(const BodyMetrics& metric) noexcept;

// возвращает плейсхолдер для сегментной массы в развернутом виде,
// или "-" если записи нет
[[nodiscard]] std::string
segmentMassFormat(const std::optional<SegmentMass>& segment_mass) noexcept;