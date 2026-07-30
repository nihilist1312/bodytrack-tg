#include "utils/input_parser.hpp"

#include <sstream>
#include <string>

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