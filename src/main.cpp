#define SDL_MAIN_HANDLED

#include <assert.h>
#include <math.h>

#include <algorithm>
#include <condition_variable>
#include <filesystem>
#include <gui.hpp>
#include <iostream>
#include <queue>
#include <utils.hpp>
#include <vector>

#include "fastrender.hpp"
#include "fs.hpp"
#include "globals.hpp"
#include "raylib.h"
#include "rlgl.h"
#include "state.hpp"
#include "time_util.hpp"
#include "zip.h"

std::condition_variable cv;

double avgFPS = Global.FPS;
double avgHZ = 1000;

double Mx = 0;
double My = 0;

int VSYNC = 0;

int avgFPSqueueNUM = 0;
int avgHZqueueNUM = 0;
double avgFPSqueueSUM = 0;
double avgHZqueueSUM = 0;
std::queue<double> avgFPSq;
std::queue<double> avgHZq;

bool dumbsleep = false;

Globals Global;

void RenderLoop() {
    double last = 0;
    SDL_GL_MakeCurrent((SDL_Window*)GetWindowSDL(), GetWindowGL());
    std::cout << "Render make gl current\n";
    BeginDrawing();
    ClearBackground(Global.Background);
    rlDrawRenderBatchActive();
    SDL_GL_SwapWindow((SDL_Window*)GetWindowSDL());

    std::cout << "Clear first bg\n";

    RenderTexture2D frameGraph = LoadRenderTexture(512, 512);

    std::cout << "loadframegraph\n";

    int loc = 0;
    int lastFPS = 0;
    while (!WindowShouldClose()) {
        auto t1 = std::chrono::steady_clock::now();
        last = getTimer();
        rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
        BeginDrawing();
        Global.mutex.lock();
        if (Global.NeedForBackgroundClear) ClearBackground(Global.Background);
        if (Global.GameTextures == -1)
            Global.gameManager->unloadGameTextures();
        else if (Global.GameTextures == 1)
            Global.gameManager->loadGameTextures();
        Global.CurrentState->render();

        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),
                      (Color){0, (unsigned char)(255 * (int)Global.Key1P),
                              (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20),
                      (Color){0, (unsigned char)(255 * (int)Global.Key2P),
                              (unsigned char)(255 * (int)Global.Key2D), 100});
        renderMouse();
        DrawTextEx(Global.DefaultFont, TextFormat("FPS: %.3f TPS: %.3f", avgFPS, avgHZ),
                   {ScaleCordX(5), ScaleCordY(5)}, Scale(10), Scale(1), GREEN);
        Global.mutex.unlock();
        rlDrawRenderBatchActive();
        SDL_GL_SwapWindow((SDL_Window*)GetWindowSDL());

        if (VSYNC == 0) {
            std::chrono::duration<double, std::milli> sleepTime{std::chrono::steady_clock::now() -
                                                                t1};
            unsigned int sleepTimeInt =
                (unsigned int)(std::max(0.0, (1000.0 / Global.FPS) - (sleepTime.count())) * 800.0);
            if (!dumbsleep) SleepInUs(sleepTimeInt);
            while (getTimer() - last < 1000.0 / Global.FPS and getTimer() - last >= 0) continue;
        }

        std::chrono::duration<double, std::milli> elapsed{std::chrono::steady_clock::now() - t1};
        double fps = (1000.0f / (elapsed.count()));
        lastFPS = fps;
        if (lastFPS > 511) lastFPS = 511;
        if (elapsed.count() > 16 and VSYNC == 0)
            std::cout << "dropped frame with " << elapsed.count() << "ms\n";
        avgFPSq.push(fps);
        avgFPSqueueSUM += fps;
        if (avgFPSq.size() > 300) {
            avgFPSqueueSUM -= avgFPSq.front();
            avgFPSq.pop();
        }
        avgFPS = avgFPSqueueSUM / (double)(avgFPSq.size());
    }
    UnloadRenderTexture(frameGraph);
}

int main() {
    SDL_SetMainReady();
    Global.CurrentState = std::make_shared<MainMenu>();
    for (int i = 0; i < Global.GamePath.size(); i++) {
        if (Global.GamePath[i] == '\\') Global.GamePath[i] = '/';
    }

    SetTraceLogLevel(LOG_WARNING);
    InitAudioDevice();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    // SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetAudioStreamBufferSizeDefault(128);
    InitWindow(640, 480, "Pop-O-Mania");

    SDL_Surface* pIcon = SDL_CreateRGBSurface(0, 64, 64, 32, 0, 0, 0, 0);
    ;
    SDL_SetWindowIcon((SDL_Window*)GetWindowSDL(), pIcon);
    SDL_FreeSurface(pIcon);

    Global.DefaultFont = LoadFont("resources/telegrama_render.otf");
    Global.popOManiaLogo = LoadTexture("resources/pop-o-mania.png");
    Global.credit1 = LoadTexture("resources/credit1.png");
    Global.credit2 = LoadTexture("resources/credit2.png");
    Global.credit3 = LoadTexture("resources/credit3.png");
    Global.credit4 = LoadTexture("resources/credit4.png");
    Global.credit5 = LoadTexture("resources/credit5.png");

    FILE* menuBg = fopen("resources/main-bg.mp3", "rb");
    fseek(menuBg, 0, SEEK_END);
    long menuBgSize = ftell(menuBg);
    fseek(menuBg, 0, SEEK_SET);
    unsigned char* menuBgData = (unsigned char*)malloc(menuBgSize + 1);
    fread(menuBgData, menuBgSize, 1, menuBg);
    fclose(menuBg);
    menuBgData[menuBgSize] = 0;
    Global.menuMusic =
        LoadMusicStreamFromMemory(".mp3", (const unsigned char*)menuBgData, menuBgSize);

    // PlayMusicStream(Global.menuMusic);
    // SetMusicVolume(Global.menuMusic, 1.0f);
    // UpdateMusicStream(Global.menuMusic);

    Global.shdrOutline = LoadShader(0, TextFormat("resources/shaders/glsl%i/outline.fs", 100));

    Global.shdrTest = LoadShader(TextFormat("resources/shaders/glsl%i/mcosu.vsh", 330),
                                 TextFormat("resources/shaders/glsl%i/mcosu.fsh", 330));

    std::string lastPath = Global.Path;
    Global.Path = "resources/default_skin/";
    std::vector<std::string> files = ls(".png");
    std::sort(files.begin(), files.end(), [](const std::string& first, const std::string& second) {
        return first.size() < second.size();
    });
    std::reverse(files.begin(), files.end());

    for (int i = 0; i < files.size(); i++) {
        if (IsFileExtension(files[i].c_str(), ".png")) {
            if (files[i].rfind("cursortrail.png", 0) == 0)
                Global.cursorTrail = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("cursor.png", 0) == 0)
                Global.cursor = LoadTexture((Global.Path + files[i]).c_str());
        }
    }
    files.clear();
    Global.Path = "resources/skin/";
    files = ls(".png");
    std::sort(files.begin(), files.end(), [](const std::string& first, const std::string& second) {
        return first.size() < second.size();
    });
    std::reverse(files.begin(), files.end());

    for (int i = 0; i < files.size(); i++) {
        if (IsFileExtension(files[i].c_str(), ".png")) {
            if (files[i].rfind("cursortrail.png", 0) == 0)
                Global.cursorTrail = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("cursor.png", 0) == 0)
                Global.cursor = LoadTexture((Global.Path + files[i]).c_str());
        }
    }
    files.clear();
    Global.Path = lastPath;
    SetTextureFilter(Global.DefaultFont.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(Global.cursor, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(Global.popOManiaLogo, TEXTURE_FILTER_BILINEAR);

    std::cout << "Loaded all files and filters\n";

    double avgFrameTime;
    HideCursor();
    initMouseTrail();

    std::cout << "Cursor init done\n";

    Global.LastFrameTime = getTimer();
    double lastFrame = getTimer();
    Global.GameTextures = 0;
    SDL_GL_SetSwapInterval(VSYNC);
    std::cout << "SetVsync\n";
    SDL_GL_MakeCurrent((SDL_Window*)GetWindowSDL(), NULL);
    std::cout << "Make sdl current gl\n";
    std::cout << "Start render loop\n";
    std::thread rend(RenderLoop);

    while (!WindowShouldClose()) {
        double timerXXX = getTimer();
        auto t1 = std::chrono::steady_clock::now();
        Global.mutex.lock();
        PollInputEvents();
        GetScale();
        GetMouse();
        GetKeys();
        if (IsKeyDown(SDL_SCANCODE_LALT) and IsKeyPressed(SDL_SCANCODE_RETURN)) {
            ToggleFullscreen();
        }
        updateUpDown();

        Global.FrameTime = getTimer() - Global.LastFrameTime;
        Global.LastFrameTime = getTimer();
        updateMouseTrail();
        Global.CurrentState->update();
        Global.mutex.unlock();

        std::chrono::duration<double, std::milli> sleepTime{std::chrono::steady_clock::now() - t1};
        unsigned int sleepTimeInt =
            (unsigned int)(std::max(0.0, (1000.0 / 1000.0) - sleepTime.count()) * 900.0);
        if (!dumbsleep) SleepInUs(sleepTimeInt);

        while (getTimer() - timerXXX < 1000.0 / 1000.0 and getTimer() - timerXXX >= 0) continue;

        std::chrono::duration<double, std::milli> elapsed{std::chrono::steady_clock::now() - t1};
        double hz = (1000.0f / (elapsed.count()));
        avgHZq.push(hz);
        avgHZqueueSUM += hz;
        if (avgHZq.size() > 300) {
            avgHZqueueSUM -= avgHZq.front();
            avgHZq.pop();
        }
        avgHZ = avgHZqueueSUM / (double)(avgHZq.size());
    }
    rend.join();

    UnloadTexture(Global.popOManiaLogo);
    UnloadFont(Global.DefaultFont);
    CloseWindow();
}
