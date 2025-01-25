#pragma once
#include <string>
#include <vector>

struct TimingPoint {
    int time;
    int meter;
    int sampleSet;
    int sampleIndex;
    int volume;
    int effects;
    double beatLength;
    bool uninherited;
    bool renderTicks = true;
};
