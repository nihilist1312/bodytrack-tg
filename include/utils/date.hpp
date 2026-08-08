#pragma once

#include <chrono>
#include <string>

using Date = std::chrono::year_month_day;

std::string getCurrentDateStr();
Date getCurrentDate();

int getCurrentYear();

bool isDateCorrect(int year, int month, int day);