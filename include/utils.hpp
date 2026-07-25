#pragma once

#include <string>
#include <vector>

std::string normalize_name(const std::string& raw_name);

// replace "letter {} letter {} letter {}" by {"a", "b", "c"} to "letter a letter b letter c"
void replace_by_vector(std::string& text, const std::vector<std::string>& text_replace);

int normalize_age(const std::string& text) noexcept;
