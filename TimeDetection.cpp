#define _CRT_SECURE_NO_WARNINGS
#include "TimeDetection.h"
#include <time.h>
#include <chrono>
#include <iostream>
#include "Point.h"

Point timeDetection() {
    // Get the current time point
    auto now = std::chrono::system_clock::now();

    // Convert time point to time_t
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);

    // Convert time_t to tm struct
    tm* now_tm = std::localtime(&now_t);

    // Print the current military time
    std::wcout << L"Current time: "
        << (now_tm->tm_hour < 10 ? "0" : "") << now_tm->tm_hour << ':'
        << (now_tm->tm_min < 10 ? "0" : "") << now_tm->tm_min << ':'
        << (now_tm->tm_sec < 10 ? "0" : "") << now_tm->tm_sec << '\n';

    Point time = { now_tm->tm_hour, now_tm->tm_min };

    return time;
}