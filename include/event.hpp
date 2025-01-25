#pragma once
#include <string>
#include <vector>

struct Event {
    int eventType;
    int startTime;

    std::string filename;
    int xOffset;
    int yOffset;

    int endTime;
};
