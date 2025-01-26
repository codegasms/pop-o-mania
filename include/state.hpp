#pragma once

#include <globals.hpp>

#include "gui.hpp"
#include "parser.hpp"
#include "utils.hpp"

class State {
   public:
    State() = default;
    virtual ~State() = default;
    int initDone = 0;
    double initStartTime = 0;

    virtual void init() = 0;
    virtual void render() = 0;
    virtual void update() = 0;
    virtual void unload() = 0;
};

class PlayMenu : public State {
   private:
    SelectableList dir_list;

    Button select;
    Button back;
    Button close;

    TextBox bg;
    TextBox path;
    TextBox description;

    Switch skin;
    Switch sound;

    TextBox usedskin;
    TextBox usedsound;

    std::string temp;

   public:
    PlayMenu();
    std::string lastPos = "";

    void init() override;
    void render() override;
    void update() override;
    void unload() override;
};

class LoadMenu : public State {
   private:
    SelectableList dir_list;

    Button select;
    Button back;
    Button close;

    TextBox bg;
    TextBox path;
    TextBox description;

   public:
    LoadMenu();

    void init() override;
    void render() override;
    void update() override;
    void unload() override;
};

class CreditMenu : public State {
   private:
    Button back;
    Button close;

    TextBox bg;
    TextBox path;
    TextBox description;
    TextBox us;

   public:
    CreditMenu();

    void init() override;
    void render() override;
    void update() override;
    void unload() override;
};

class MainMenu : public State {
   private:
    Button play;
    Button wip;
    Button load;
    Button credits;

    TestSlider volume;

   public:
    MainMenu();

    void init() override;
    void render() override;
    void update() override;
    void unload() override;
};

class Game : public State {
   private:
    TestSlider volume;

   public:
    Game();

    void init() override;
    void render() override;
    void update() override;
    void unload() override;
};
