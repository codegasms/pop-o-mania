#pragma once

#include <string.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "SDL2/SDL.h"
#include "gamemanager.hpp"
#include "raylib.h"

#define PLATFORM_DESKTOP

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else  // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

struct GameSettings {
    bool useDefaultSkin = false;
    bool useDefaultSounds = false;
};

struct InputHandler {
    int mx = -1;
    int my = -1;
    bool k1p = 0;
    bool k1d = 0;
    bool k1r = 0;
    bool k2p = 0;
    bool k2d = 0;
    bool k2r = 0;
    int scr = 0;
};

class State;
struct TextureSizes {
    int hitCircle = 128;
    int comboNumber = 140;
    int hitCircleOverlay = 128;
    int approachCircle = 128;
    bool render300 = false;
};

struct Globals {
    float Scale = 1.f;
    Vector2 ZeroPoint = {0.f, 0.f};
    Color Background = {15, 0, 30, 255};

    Vector2 CallbackMouse = {0, 0};

    int GameTextures = 0;

    // Color Background = {42,22,33,255};
    int skinNumberOverlap = 18;
    double FPS = 4.0f * 100.0f;
    int Width = 640;
    int Height = 480;
    float offset = 45.0f;

    struct timespec ts1 = timespec{0, 0}, ts2 = timespec{0, 0};
    bool paused = false;

    int maxSliderSize = 10000;

    long int startTime2;
    double curTime = 0;
    double startTime = -1;
    double curTime2 = 0;

    double extraJudgementTime = 16.0;

    double offsetTime = -50;

    unsigned long long int startsTime = 0;
    unsigned long long int pausedFor = 0;
    Texture2D cursor;
    Texture2D cursorTrail;

    bool MouseInFocus = false;
    int MouseBlur = 20;

    Vector2 MousePosition;
    Vector2 AutoMousePosition = {0, 0};
    Vector2 AutoMousePositionStart = {-1, -1};
    double AutoMouseStartTime;
    bool useAuto = false;

    Font DefaultFont;

    std::string Path = std::filesystem::current_path().string();
    std::string BeatmapLocation = std::filesystem::current_path().string() + "/beatmaps";
    std::string GamePath = std::filesystem::current_path().string();
    std::string selectedPath = "";
    std::string CurrentLocation = std::filesystem::current_path().string();
    int MouseTrailSize = 150;

    float FrameTimeCounterWheel = 0.f;

    Music menuMusic;

    Texture2D popOManiaLogo;

    Texture2D credit1;
    Texture2D credit2;
    Texture2D credit3;
    Texture2D credit4;
    Texture2D credit5;

    double volume = 0.4f;
    double hitVolume = 0.7f;
    bool volumeChanged = true;

    bool Key1P = false;
    bool Key1D = false;
    bool Key2P = false;
    bool Key2D = false;
    bool Key1R = false;
    bool Key2R = false;
    bool enableMouse = true;
    int Wheel = 0;

    double LastFrameTime;
    double FrameTime;

    Shader shdrOutline;
    Shader shdrTest;
    std::shared_ptr<State> CurrentState;

    GameManager* gameManager = GameManager::getInstance();

    Globals() = default;

    float sliderTexSize = 1.0f;
    int circleSector = 16;
    bool legacyRender = false;

    long long errorSum = 0;
    long long errorLast = 0;
    long long errorDiv = 0;

    std::chrono::time_point<std::chrono::steady_clock> start, end;

    bool NeedForBackgroundClear = true;

    double CurrentInterpolatedTime = 0;
    double LastOsuTime = 0;
    double currentOsuTime = 0;

    double avgSum = 0;
    double avgNum = 0;
    double avgTime = 0;

    int numberLines = -1;
    int parsedLines = -1;

    int loadingState = 0;

    SDL_Window* win;
    bool quit = false;

    TextureSizes textureSize;

    InputHandler Input;
    bool renderFrame;
    std::mutex mutex;

    GameSettings settings;
};

extern Globals Global;
