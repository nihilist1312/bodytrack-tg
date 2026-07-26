#pragma once

#include <string>
#include <vector>

std::string normalizeName(const std::string& raw_name);

// replace "letter {} letter {} letter {}" by {"a", "b", "c"} to "letter a
// letter b letter c"
void replaceByVector(std::string& text,
                     const std::vector<std::string>& text_replace);

int normalizeAge(const std::string& text) noexcept;
