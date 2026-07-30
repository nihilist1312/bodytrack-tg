#pragma once

#include <string>
#include <string_view>

std::string normalizeName(const std::string& raw_name);

int normalizeAge(std::string_view text) noexcept;
