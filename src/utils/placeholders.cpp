#include <string>
#include <vector>

void replaceByVector(std::string& text,
                     const std::vector<std::string>& text_replace) {
    size_t pos = 0;
    for (const auto& str : text_replace) {
        pos = text.find("{}", pos);
        if (pos == std::string::npos) {
            break;
        }
        text.replace(pos, 2, str);
        pos += str.size(); // сдвигаемся за вставленную строку
    }
}