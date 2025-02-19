#pragma once
#include <string.h>

#include <map>
#include <vector>

#include "event.hpp"
#include "followpoint.hpp"
#include "gamefile.hpp"
#include "hitobject.hpp"
#include "timingpoint.hpp"

struct GameFile {
    std::map<std::string, std::string> configGeneral;
    std::map<std::string, std::string> configEditor;
    std::map<std::string, std::string> configMetadata;
    std::map<std::string, std::string> configDifficulty;
    std::map<std::string, std::string> configColours;
    std::vector<std::vector<int>> comboColours;
    std::vector<HitObjectData> hitObjects;
    std::vector<TimingPoint> timingPoints;
    std::vector<FollowPoint> followPoints;
    std::vector<Event> events;
    float p300 = 80.0f;
    float p300Change = 6.0f;
    float p300Final;
    float p100 = 140.0f;
    float p100Change = 8.0f;
    float p100Final;
    float p50 = 200.0f;
    float p50Change = 10.0f;
    float p50Final;
    int preempt;
    int fade_in;
};
