#include "state.hpp"

#include <iostream>

#include "fastrender.hpp"
#include "fs.hpp"
#include "gamemanager.hpp"
#include "raylib.h"
#include "rlgl.h"
#include "utils.hpp"
#include "zip.h"

PlayMenu::PlayMenu() {
    description = TextBox({320, 140}, {520, 40}, {240, 98, 161, 255}, "Select a Beatmap to play!",
                          WHITE, 15, 50);
    bg = TextBox({320, 240}, {530, 290}, {240, 98, 161, 255}, "", {240, 98, 161, 255}, 15, 10);
    back = Button({395, 360}, {120, 40}, {255, 135, 198, 255}, "Back", BLACK, 15);
    select = Button({520, 360}, {120, 40}, {255, 135, 198, 255}, "Select", BLACK, 15);
    close = Button({70, 110}, {20, 20}, {255, 135, 198, 255}, "x", BLACK, 15);
    skin = Switch({310, 350}, {40, 20}, RED, GREEN, {255, 135, 198, 255}, BLACK);
    sound = Switch({310, 370}, {40, 20}, RED, GREEN, {255, 135, 198, 255}, BLACK);
    usedskin =
        TextBox({200, 350}, {150, 20}, {240, 98, 161, 255}, "Use default skin", WHITE, 10, 50);
    usedsound =
        TextBox({200, 370}, {150, 20}, {240, 98, 161, 255}, "Use default sound", WHITE, 10, 50);
    skin.state = Global.settings.useDefaultSkin;
    sound.state = Global.settings.useDefaultSounds;
}

void PlayMenu::init() {
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    Global.LastFrameTime = getTimer();
    temp = Global.Path;
    Global.Path = Global.BeatmapLocation;
    auto dir = ls(".osu");
    dir_list = SelectableList({320, 260}, {520, 150}, {255, 135, 198, 255}, dir, BLACK, 10, 15, 65);
}
void PlayMenu::render() {
    // Global.mutex.lock();
    bg.render();
    description.render();
    back.render();
    select.render();
    dir_list.render();
    close.render();
    skin.render();
    sound.render();
    usedskin.render();
    usedsound.render();
    // Global.mutex.unlock();
}
void PlayMenu::update() {
    Global.enableMouse = true;
    dir_list.update();
    select.update();
    back.update();
    close.update();
    skin.update();
    sound.update();

    if (skin.state != Global.settings.useDefaultSkin) Global.settings.useDefaultSkin = skin.state;

    if (sound.state != Global.settings.useDefaultSounds)
        Global.settings.useDefaultSounds = sound.state;

    if (close.action) {
        Global.Path = temp;
        Global.CurrentState->unload();
        Global.CurrentState.reset(new MainMenu());
    }

    if (select.action or dir_list.action) {
        if (dir_list.objects.size() > 0 and
            dir_list.objects[dir_list.selectedindex].text.size() > 0) {
            if (dir_list.objects[dir_list.selectedindex]
                    .text[dir_list.objects[dir_list.selectedindex].text.size() - 1] == '/') {
                dir_list.objects[dir_list.selectedindex].text.pop_back();
                if (Global.Path.size() == 1) Global.Path.pop_back();
                Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
                lastPos = dir_list.objects[dir_list.selectedindex].text;
                auto dir = ls(".osu");
                dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir,
                                          dir_list.textcolor, dir_list.textsize,
                                          dir_list.objectsize, dir_list.maxlength);
                dir_list.init();
            } else {
                Global.selectedPath =
                    Global.Path + '/' + dir_list.objects[dir_list.selectedindex].text;
                Global.CurrentLocation = "beatmaps/" + lastPos + "/";
                Global.CurrentState->unload();
                Global.CurrentState.reset(new Game());
                Global.CurrentState->init();
            }
        }
    } else if (back.action) {
        Global.Path = Global.BeatmapLocation;
        auto dir = ls(".osu");
        dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir,
                                  dir_list.textcolor, dir_list.textsize, dir_list.objectsize,
                                  dir_list.maxlength);
        dir_list.init();
    }
}
void PlayMenu::unload() {}

LoadMenu::LoadMenu() {
    description = TextBox({320, 140}, {520, 40}, {240, 98, 161, 255},
                          "Select a .OSZ Beatmap file\n    to extract!", WHITE, 15, 50);
    bg = TextBox({320, 240}, {530, 290}, {240, 98, 161, 255}, "", {240, 98, 161, 255}, 15, 10);
    back = Button({395, 360}, {120, 40}, {255, 135, 198, 255}, "Back", BLACK, 15);
    select = Button({520, 360}, {120, 40}, {255, 135, 198, 255}, "Select", BLACK, 15);
    close = Button({70, 110}, {20, 20}, {255, 135, 198, 255}, "x", BLACK, 15);
    auto dir = ls(".osz");
    dir_list = SelectableList({320, 260}, {520, 150}, {255, 135, 198, 255}, dir, BLACK, 10, 15, 60);
    path = TextBox({195, 360}, {270, 40}, {240, 98, 161, 255}, Global.Path, WHITE, 8, 40);
}

void LoadMenu::init() {
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    Global.LastFrameTime = getTimer();
    Global.FrameTime = 0.5;
}

void LoadMenu::render() {
    // Global.mutex.lock();
    bg.render();
    description.render();
    back.render();
    path.render();
    select.render();
    dir_list.render();
    close.render();
    // Global.mutex.unlock();
}
void LoadMenu::update() {
    Global.enableMouse = true;
    dir_list.update();
    select.update();
    back.update();
    close.update();

    if (close.action) {
        Global.CurrentState->unload();
        Global.CurrentState.reset(new MainMenu());
    }

    if (select.action or dir_list.action) {
        if (dir_list.objects.size() > 0 and
            dir_list.objects[dir_list.selectedindex].text.size() > 0) {
            if (dir_list.objects[dir_list.selectedindex]
                    .text[dir_list.objects[dir_list.selectedindex].text.size() - 1] == '/') {
                dir_list.objects[dir_list.selectedindex].text.pop_back();
                if (Global.Path.size() == 1) Global.Path.pop_back();
                Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
                auto dir = ls(".osz");
                dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir,
                                          dir_list.textcolor, dir_list.textsize,
                                          dir_list.objectsize, dir_list.maxlength);
                dir_list.init();
                path = TextBox(path.position, path.size, path.color, Global.Path, path.textcolor,
                               path.textsize, path.maxlength);
            } else {
                Global.selectedPath =
                    Global.Path + '/' + dir_list.objects[dir_list.selectedindex].text;
                std::string base_file = get_without_ext(Global.selectedPath);
                std::string final_path = Global.GamePath + "/beatmaps/" + base_file;
                create_dir(final_path);
                int arg = 2;
                std::cout << Global.selectedPath.c_str() << std::endl;
                zip_extract(Global.selectedPath.c_str(), final_path.c_str(), on_extract_entry,
                            &arg);
            }
        }
    } else if (back.action) {
        while (Global.Path.size() > 0) {
            if (Global.Path[Global.Path.size() - 1] == '/' ||
                Global.Path[Global.Path.size() - 1] == ':') {
                if (Global.Path.size() > 1 && Global.Path.size() != 2) Global.Path.pop_back();
                break;
            }
            Global.Path.pop_back();
        }
        auto dir = ls(".osz");
        dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir,
                                  dir_list.textcolor, dir_list.textsize, dir_list.objectsize,
                                  dir_list.maxlength);
        dir_list.init();
        path = TextBox(path.position, path.size, path.color, Global.Path, path.textcolor,
                       path.textsize, path.maxlength);
    }
}
void LoadMenu::unload() {}

MainMenu::MainMenu() {
    play = Button({250, 420}, {120, 60}, {255, 135, 198, 255}, "Play", BLACK, 20);
    load = Button({390, 420}, {120, 60}, {255, 135, 198, 255}, "Load", BLACK, 20);
    volume = TestSlider({510, 460}, {240, 20}, BLACK, PURPLE, WHITE, WHITE);
}
void MainMenu::init() {
    Global.NeedForBackgroundClear = true;
    Global.LastFrameTime = getTimer();
    Global.FrameTime = 0.5;
    Global.useAuto = false;
    volume.location = Global.volume * 100.0f;
}
void MainMenu::update() {
    Global.enableMouse = true;
    play.update();
    load.update();
    // test.update();
    if (play.action) {
        Global.CurrentState->unload();
        Global.CurrentState.reset(new PlayMenu());
        Global.CurrentState->init();
        return;
    } else if (load.action) {
        Global.CurrentState->unload();
        Global.CurrentState.reset(new LoadMenu());
        Global.CurrentState->init();
        return;
    }

    if (IsKeyDown(SDL_SCANCODE_LALT)) volume.update();
    float lastVolume = Global.volume;
    Global.volume = volume.location / 100.0f;
    if (!AreSame(lastVolume, Global.volume)) {
        std::cout << "Volume: " << Global.volume << std::endl;
        Global.volumeChanged = true;
    }
}
void MainMenu::render() {
    // Global.mutex.lock();
    DrawTextureCenter(Global.popOManiaLogo, 320, 200, 1 / 3.f, WHITE);
    play.render();
    load.render();
    if (IsKeyDown(SDL_SCANCODE_LALT)) volume.render();
}

void MainMenu::unload() {}

Game::Game() { volume = TestSlider({510, 460}, {240, 20}, BLACK, PURPLE, WHITE, WHITE); }
void Game::init() {
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    initDone = 0;
    Global.LastFrameTime = getTimer();
    std::cout << Global.selectedPath << std::endl;
    Global.numberLines = 0;
    Global.parsedLines = 0;
    Global.loadingState = 0;
    initDone = -2;
    initStartTime = getTimer();
    Global.mutex.unlock();
    Global.gameManager->loadGame(Global.selectedPath);
    Global.gameManager->timingSettingsForHitObject.clear();
    Global.mutex.lock();
    Global.startTime = -5000.0f;
    Global.errorSum = 0;
    Global.errorLast = 0;
    Global.errorDiv = 0;

    volume.location = Global.volume * 100.0f;
    std::cout << volume.location << std::endl;
    std::cout << "done init" << std::endl;
}
void Game::update() {
    if (initDone == 1) {
        Global.enableMouse = false;
        Global.gameManager->run();
        if (IsKeyPressed(SDL_SCANCODE_BACKSPACE)) {
            Global.gameManager->unloadGame();
            Global.CurrentState->unload();
            Global.CurrentState.reset(new PlayMenu());
            Global.CurrentState->init();
            return;
        }
    } else {
        if (initDone == 0 or Global.GameTextures == 0) {
            initDone = -1;
        }
        if (initDone == -1 and getTimer() - initStartTime > 0.0f) {
            std::cout << "init done in " << getTimer() - initStartTime << " secs\n";
            initDone = true;
        }
    }
    if (IsKeyDown(SDL_SCANCODE_LALT)) volume.update();
    float lastVolume = Global.volume;
    Global.volume = volume.location / 100.0f;
    if (!AreSame(lastVolume, Global.volume)) {
        std::cout << "Volume: " << Global.volume << std::endl;
        Global.volumeChanged = true;
    }
}
void Game::render() {
    if (initDone == 1) {
        Global.enableMouse = false;
        Global.gameManager->render();
        // Global.mutex.lock();
        if (IsMusicStreamPlaying(Global.gameManager->backgroundMusic)) {
            DrawTextEx(Global.DefaultFont,
                       TextFormat("Playing: %.3f/%.3f", (Global.currentOsuTime / 1000.0),
                                  GetMusicTimeLength(Global.gameManager->backgroundMusic)),
                       {ScaleCordX(5), ScaleCordY(15)}, Scale(10), Scale(1), WHITE);

        } else {
            DrawTextEx(
                Global.DefaultFont,
                TextFormat("Paused: %.3f/%.3f",
                           GetMusicTimePlayed(Global.gameManager->backgroundMusic) * 1000000.0f,
                           GetMusicTimeLength(Global.gameManager->backgroundMusic)),
                {ScaleCordX(5), ScaleCordY(15)}, Scale(10), Scale(1), WHITE);
            if (Global.errorDiv != 0)
                DrawTextEx(
                    Global.DefaultFont,
                    TextFormat("Error Avg: %ld ms", (Global.errorSum / Global.errorDiv) / 1000),
                    {ScaleCordX(5), ScaleCordY(40)}, Scale(15), Scale(1), WHITE);
        }
        if (GetMusicTimeLength(Global.gameManager->backgroundMusic) != 0) {
            DrawLineEx(
                {0, GetScreenHeight() - Scale(2)},
                {GetScreenWidth() * ((Global.currentOsuTime / 1000.0) /
                                     GetMusicTimeLength(Global.gameManager->backgroundMusic)),
                 GetScreenHeight() - Scale(2)},
                Scale(3), Fade(WHITE, 0.8));
        }
        // Global.mutex.unlock();
    } else if (initDone == -1) {
        std::string message;
        if (getTimer() - initStartTime < 2000.0f)
            message = "Loaded Game!";
        else if (getTimer() - initStartTime < 2500.0f)
            message = "3...";
        else if (getTimer() - initStartTime < 3000.0f)
            message = "2...";
        else if (getTimer() - initStartTime < 3500.0f)
            message = "1...";
        else if (getTimer() - initStartTime < 4000.0f)
            message = "GO!";
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),
                      (Color){0, (unsigned char)(255 * (int)Global.Key1P),
                              (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20),
                      (Color){0, (unsigned char)(255 * (int)Global.Key2P),
                              (unsigned char)(255 * (int)Global.Key2D), 100});
        // Global.mutex.lock();
        DrawTextEx(Global.DefaultFont, message.c_str(),
                   {ScaleCordX(320 - message.size() * 7.5f), ScaleCordY(220)}, Scale(20), Scale(1),
                   WHITE);
        // Global.mutex.unlock();
    } else if (initDone == -2) {
        // Global.mutex.lock();
        std::string message;
        message = "Loading Game...";

        if (Global.loadingState == 1) {
            // std::cout << "Precalculating HitObjects" << std::endl;
            message = "Precalculating HitObjects";
        } else if (Global.loadingState == 2) {
            // std::cout << "Loading Background Music" << std::endl;
            message = "Loading Background Music";
        } else if (Global.loadingState == 3) {
            // std::cout << "Loading ComboBreak Sound" << std::endl;
            message = "Loading ComboBreak Sound";
        } else if (Global.loadingState == 4) {
            // std::cout << "Loading Hit Sounds" << std::endl;
            message = "Loading Hitsound " + std::to_string(Global.parsedLines) + " of " +
                      std::to_string(Global.numberLines);
        } else if (Global.loadingState == 5) {
            message = "Parsing line " + std::to_string(Global.parsedLines) + " of " +
                      std::to_string(Global.numberLines);
        } else if (Global.loadingState == 6) {
            message = "Parsing Timing Points";
        } else if (Global.loadingState == 7) {
            message = "Loading Textures";
        }
        DrawTextEx(Global.DefaultFont, message.c_str(),
                   {ScaleCordX(320 - message.size() * 7.5f), ScaleCordY(220)}, Scale(20), Scale(1),
                   WHITE);
        // Global.mutex.unlock();
    }
    if (IsKeyDown(SDL_SCANCODE_LALT)) volume.render();
}

void Game::unload() { Global.NeedForBackgroundClear = true; }
