#include "utils/input_parser.hpp"

#include "models/body-metrics.hpp"

#include <optional>
#include <sstream>
#include <string>

#include <spdlog/spdlog.h>

[[nodiscard]]
std::string normalizeName(const std::string& raw_name) {
    std::istringstream oss{raw_name};
    std::string res;
    std::string temp;
    while (oss >> temp) {
        res += temp + " ";
    }
    res.pop_back();
    return res;
}

[[nodiscard]] std::optional<SegmentMass>
getSegmentsMass(const std::string& text) noexcept {
    std::istringstream iss(text);
    std::optional<SegmentMass> segments = SegmentMass{};

    double num;
    if (!(iss >> num)) {
        spdlog::debug("Can't get 1 number");
        return std::nullopt;
    }
    segments->left_arm = num;
    if (!(iss >> num)) {
        spdlog::debug("Can't get 2 number");
        return std::nullopt;
    }
    segments->right_arm = num;
    if (!(iss >> num)) {
        spdlog::debug("Can't get 3 number");
        return std::nullopt;
    }
    segments->left_leg = num;
    if (!(iss >> num)) {
        spdlog::debug("Can't get 4 number");
        return std::nullopt;
    }
    segments->right_leg = num;
    if (!(iss >> num)) {
        spdlog::debug("Can't get 5 number");
        return std::nullopt;
    }
    segments->trunk = num;

    // проверка что в строке больше ничего нет
    if (!iss.eof()) {
        spdlog::debug("Another symbols in massege");
        return std::nullopt;
    }

    return segments;
}