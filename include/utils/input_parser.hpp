#pragma once

#include <string>
#include <string_view>

std::string normalizeName(const std::string& raw_name);

int normalizeAge(std::string_view text) noexcept;

bool setDate(std::string& target, const std::string& text);
bool setMass(double& target, std::string_view text) noexcept;
bool setHeight(double& target, std::string_view text) noexcept;
