#pragma once

#include <string>
#include <vector>

// replace "letter {} letter {} letter {}" by {"a", "b", "c"} to "letter a
// letter b letter c"
void replaceByVector(std::string& text,
                     const std::vector<std::string>& text_replace);
