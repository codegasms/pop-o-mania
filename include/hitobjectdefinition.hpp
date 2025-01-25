#pragma once
#include <raylib.h>

#include <climits>
#include <iostream>
#include <string>
#include <vector>

struct catmullRomSegment {
    Vector2 a;
    Vector2 b;
    Vector2 c;
    Vector2 d;
};

struct timingSettings {
    int time;
    double beatLength;
    float meter;
    float sampleSet;
    float sampleIndex;
    float volume;
    bool uninherited;
    int effects;
    double sliderSpeedOverride = 1;
    bool renderTicks = true;
};

struct HitObjectData {
    int x;
    int y;
    int ex;
    int ey;
    double time;
    double lastTime;
    int type;
    int hitSound;
    int point = 0;
    char curveType;
    std::vector<std::pair<int, int> > curvePoints;
    int slides;
    double length;
    std::vector<int> edgeSounds;
    std::vector<std::pair<int, int> > edgeSets;
    int endTime;
    int normalSet;
    int additionSet;
    int hindex;
    int index;
    int volume;
    std::string filename = "";
    bool startingACombo;
    int skipComboColours = 0;
    int comboNumber;
    bool useDefaultHitSound;
    std::vector<int> colour;
    timingSettings timing;
    bool touch = false;
    bool PlayAddition = false;
    bool PlayCustom = false;
    std::string CustomSound = "";
    std::string AdditionSound = "";
    std::string NormalSound = "";
    std::vector<std::string> EdgeNormalSound;
    std::vector<std::string> EdgeAdditionSound;
    std::vector<float> lengths;
    bool textureReady = false;
    bool textureLoaded = false;
    double totalLength;
    Vector2 extraPos;
    bool expired = false;
};

class HitObject {
   public:
    HitObject() {};
    virtual ~HitObject() = default;
    virtual void init() {}
    virtual void render() {}
    virtual void update() {}
    virtual void dead_update() {}
    virtual void dead_render() {}
    HitObjectData data;
    float x, y;
    Music id;

   private:
};
