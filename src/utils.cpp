#include <ctype.h>

#include <algorithm>
#include <cmath>
#include <gamemanager.hpp>
#include <globals.hpp>
#include <iostream>
#include <utils.hpp>
#include <vector>

// ============== Global variables ============== //

// Update the variables needed for the scrolling
void updateUpDown() {
    // Get the current state of the mouse wheel
    Global.Wheel = GetMouseWheelMove();
    if (IsKeyPressed(SDL_SCANCODE_DOWN)) {
        // If the down key is pressed, start a timer to simulate a fast mouse wheel movement
        Global.Wheel = -1;
        Global.FrameTimeCounterWheel = -170;
    }
    if (IsKeyPressed(SDL_SCANCODE_UP)) {
        // If the up key is pressed, start a timer to simulate a fast mouse wheel movement
        Global.Wheel = 1;
        Global.FrameTimeCounterWheel = -170;
    }
    Global.FrameTimeCounterWheel += GetFrameTime() * 1000.0f;
    while (Global.FrameTimeCounterWheel > 50.0f) {
        // If the keys are still down, trigger a wheel movement every 50 milliseconds
        Global.FrameTimeCounterWheel -= 50.0f;
        if (IsKeyDown(SDL_SCANCODE_UP)) Global.Wheel = 1;
        if (IsKeyDown(SDL_SCANCODE_DOWN)) Global.Wheel = -1;
    }
};

// Get the scaling needed for the current window size + Offset for the zero point
void GetScale() {
    Global.Scale = std::min(GetScreenWidth() / 640.0f, GetScreenHeight() / 480.0f);
    Global.ZeroPoint = {GetScreenWidth() / 2.0f - (Global.Scale * 320.0f),
                        GetScreenHeight() / 2.0f - (Global.Scale * 240.0f)};
};

// Get the absolute Mouse position & Check if in bound
void GetMouse() {
    Global.MouseInFocus = CheckCollisionPointRec(
        GetMousePosition(), (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()});
    if (!Global.MouseInFocus) {
        Global.MousePosition = {0, 0};
    } else {
        if (Global.useAuto)
            Global.MousePosition = Global.AutoMousePosition;
        else
            Global.MousePosition = {(GetMouseX() - Global.ZeroPoint.x) / Global.Scale,
                                    (GetMouseY() - Global.ZeroPoint.y) / Global.Scale};
    }
};

// Get the states of the main keys : Z, X, Mouse Left, Mouse Right => Pressed, Down, Released
void GetKeys() {
    if (IsKeyPressed(SDL_SCANCODE_Z) or
        (Global.enableMouse and IsMouseButtonPressed(SDL_BUTTON_LEFT)))
        Global.Key1P = true;
    else
        Global.Key1P = false;
    if (IsKeyPressed(SDL_SCANCODE_X) or
        (Global.enableMouse and IsMouseButtonPressed(SDL_BUTTON_RIGHT)))
        Global.Key2P = true;
    else
        Global.Key2P = false;
    if (IsKeyDown(SDL_SCANCODE_Z) or (Global.enableMouse and IsMouseButtonDown(SDL_BUTTON_LEFT)))
        Global.Key1D = true;
    else
        Global.Key1D = false;
    if (IsKeyDown(SDL_SCANCODE_X) or (Global.enableMouse and IsMouseButtonDown(SDL_BUTTON_RIGHT)))
        Global.Key2D = true;
    else
        Global.Key2D = false;
    if (IsKeyReleased(SDL_SCANCODE_Z) or
        (Global.enableMouse and IsMouseButtonReleased(SDL_BUTTON_LEFT)))
        Global.Key1R = true;
    else
        Global.Key1R = false;
    if (IsKeyReleased(SDL_SCANCODE_X) or
        (Global.enableMouse and IsMouseButtonReleased(SDL_BUTTON_RIGHT)))
        Global.Key2R = true;
    else
        Global.Key2R = false;
};

// ============== Scaling functions ============== //

// Scale a number based on the current scaling factor
float Scale(float a) { return a * Global.Scale; };

// Scale a Rectangle based on the current scaling factor
Rectangle ScaleRect(Rectangle a) {
    return {a.x * Global.Scale + Global.ZeroPoint.x, a.y * Global.Scale + Global.ZeroPoint.y,
            a.width * Global.Scale, a.height * Global.Scale};
};

// Scale a Vector based on the current scaling factor with origin offset
Vector2 ScaleCords(Vector2 a) {
    return {a.x * Global.Scale + Global.ZeroPoint.x, a.y * Global.Scale + Global.ZeroPoint.y};
};

// Scale the X position and add the origin offset
float ScaleCordX(float a) { return a * Global.Scale + Global.ZeroPoint.x; };

// Scale the Y position and add the origin offset
float ScaleCordY(float a) { return a * Global.Scale + Global.ZeroPoint.y; };

// ============== Get Data functions ============== //

// Get the center point of a rectangle
Vector2 GetCenter(Rectangle a) { return {a.x + a.width / 2, a.y + a.height / 2}; };

// Get the "origin" of a rectangle for raylib drawing functions and return a vector
Vector2 GetRaylibOrigin(Rectangle a) { return {a.x - a.width / 2, a.y - a.height / 2}; };

// Get the "origin" of a rectangle for raylib drawing functions and return a rectangle
Rectangle GetRaylibOriginR(Rectangle a) {
    return {a.x - a.width / 2, a.y - a.height / 2, a.width, a.height};
};

// ============== Drawing functions ============== //

// Find the nth digit of a number
int nthDigit(int v, int n) {
    while (n > 0) {
        v /= 10;
        n--;
    }
    return "0123456789"[v % 10] - '0';
}

// Draw a centered and scaled Texture
void DrawTextureCenter(Texture2D tex, float x, float y, float s, Color color) {
    DrawTextureEx(tex, ScaleCords(GetRaylibOrigin({x, y, tex.width * s, tex.height * s})), 0,
                  Scale(s), color);
}

// Draw centered and scaled combo numbers
void DrawCNumbersCenter(int n, float x, float y, float s, Color color) {
    GameManager *gm = GameManager::getInstance();
    int digits = log10(n) + 1;
    int i = (digits - 1) * 18;
    for (int k = 0; k < digits; k++) {
        DrawTextureCenter(gm->numbers[nthDigit(n, digits - k - 1)],
                          x - (float)i * s + k * 18 * s * 2, y, s, color);
    }
}

// Draw scaled combo numbers from left to right
void DrawCNumbersLeft(int n, float x, float y, float s, Color color) {
    GameManager *gm = GameManager::getInstance();
    int digits = log10(n) + 1;
    int i = (digits - 1) * 18;
    for (int k = 0; k < digits; k++) {
        DrawTextureCenter(gm->numbers[nthDigit(n, digits - k - 1)], x + k * 18 * s * 2, y, s,
                          color);
    }
}

// Draw a centered/left and scaled Texture with a rotation
void DrawTextCenter(const char *text, float x, float y, float s, Color color) {
    Vector2 size = MeasureTextEx(Global.DefaultFont, text, s, 1);
    DrawTextPro(Global.DefaultFont, text, ScaleCords(Vector2{x - size.x / 2.0f, y - size.y / 2.0f}),
                Vector2{0, 0}, 0, Scale(s), Scale(1), color);
}
void DrawTextLeft(const char *text, float x, float y, float s, Color color) {
    // draw text LTR
    Vector2 size = MeasureTextEx(Global.DefaultFont, text, s, 1);
    DrawTextPro(Global.DefaultFont, text, ScaleCords(Vector2{x, y - size.y / 2.0f}), Vector2{0, 0},
                0, Scale(s), Scale(1), color);
}

// Draw a centered, scaled and rotated Texture
void DrawTextureRotate(Texture2D tex, float x, float y, float s, float r, Color color) {
    DrawTexturePro(
        tex, Rectangle{0, 0, tex.width, tex.height},
        Rectangle{ScaleCordX(x), ScaleCordY(y), Scale(tex.width * s), Scale(tex.height * s)},
        Vector2{Scale(tex.width * s / 2.0f), Scale(tex.height * s / 2.0f)}, r, color);
}

// Draw a centered, scaled and rotated Texture with a rectangle
void DrawTextureOnCircle(Texture2D tex, float x, float y, float rad, float s, float r, float ang,
                         Color color) {
    Vector2 pos = getPointOnCircle(x, y, rad, ang);
    DrawTextureRotate(tex, pos.x, pos.y, s, r, color);
}

// Draw a slider texture (exclusively for a slider object)
void DrawTextureSlider(Texture2D tex, float x, float y, Color color, float s) {
    DrawTextureEx(tex,
                  ScaleCords({x - s / 2.0f - 4.0f * Global.sliderTexSize,
                              y - s / 2.0f - 4.0f * Global.sliderTexSize}),
                  0, Scale(1.0f / Global.sliderTexSize), color);
}

void DrawSpinnerBack(Texture2D tex, Color color) {
    float x = 0;
    float y = 0;
    float ratio = (float)tex.width / (float)tex.height;
    float defaultRatio = 640.0f / 480.0f;
    if (defaultRatio > ratio) {
        x = 640.0f - 480.0f * ratio;
    } else {
        y = 480.0f - 640.0f / ratio;
    }
    Rectangle source = {0, 0, tex.width, tex.height};
    DrawTexturePro(tex, source, ScaleRect(Rectangle{0 + x / 2.0f, y / 2.0f, 640 - x, (480.0f - y)}),
                   Vector2{0, 0}, 0, color);
}

void DrawSpinnerMeter(Texture2D tex, float per) {
    per = clip(per, 0.001f, 0.999f);
    float x = 0;
    float y = 0;
    float ratio = (float)tex.width / (float)tex.height;
    float defaultRatio = 640.0f / 480.0f;
    if (defaultRatio > ratio) {
        x = 640.0f - 480.0f * ratio;
    } else {
        y = 480.0f - 640.0f / ratio;
    }
    Rectangle source = {0, tex.height * (1.0f - per), tex.width, tex.height * per};
    DrawTexturePro(tex, Rectangle{0, 0, tex.width, tex.height},
                   ScaleRect(Rectangle{0 + x / 2.0f, 0 + y / 2.0f, 640 - x, 480 - y}),
                   Vector2{0, 0}, 0, BLACK);
    DrawTexturePro(tex, source,
                   ScaleRect(Rectangle{0 + x / 2.0f, (480.0f - y) * (1.0f - per) + y / 2.0f,
                                       640 - x, (480.0f - y) * per}),
                   Vector2{0, 0}, 0, WHITE);
}

// ============== Math functions ============== //

// Clip a number between two other numbers
float clip(float n, float lower, float upper) {
    n = (n > lower) * n + !(n > lower) * lower;
    return (n < upper) * n + !(n < upper) * upper;
}

// Interpolate more points in between two points
Vector2 lerp(Vector2 a, Vector2 b, float t) {
    return {.x = (1 - t) * a.x + t * b.x, .y = (1 - t) * a.y + t * b.y};
}

// Return a vector with both of the cordinates set to a float
Vector2 vectorize(float i) { return Vector2{i, i}; }

// Calculate the distance between two points
float distance(Vector2 &p0, Vector2 &p1) {
    return std::sqrt(std::pow(std::abs(p0.x - p1.x), 2) + std::pow(std::abs(p0.y - p1.y), 2));
}

// Binary search algorithm
int Search(std::vector<float> arr, float x, int l, int r) {
    if (r >= l) {
        int mid = (l + r) / 2;
        if (arr[mid] == x || l == r) return mid;
        if (arr[mid] > x)
            return Search(arr, x, l, mid - 1);
        else
            return Search(arr, x, mid + 1, r);
    } else
        return l;
}

Vector2 getPointOnCircle(float x, float y, float radius, float angle) {
    angle = (angle * M_PI) / 180;
    float xdiff = radius * cos(angle);
    float ydiff = radius * sin(angle);
    return Vector2{x + xdiff, y + ydiff};
}

std::string getSampleSetFromInt(int s) {
    if (s == 1)
        return "normal";
    else if (s == 2)
        return "soft";
    else if (s == 3)
        return "drum";
}

// ============== Extra Utility functions ============== //

float easeInOutCubic(float x) { return x; }

float easeOutQuad(float x) { return 1.0f - ((1.0f - x) * (1.0f - x)); }

bool AreSame(double a, double b) { return std::fabs(a - b) < 0.0001f; }

// ============== File functions ============== //

std::vector<std::string> ParseNameFolder(std::string folder) {
    std::vector<std::string> output;
    if (folder[folder.size() - 1] == '/') {
        folder.pop_back();
    }
    int end = folder.size() - 1;
    int begin = folder.size() - 1;
    while (begin >= 0 and folder[begin] != '-') begin--;
    begin = std::max(0, begin);
    if (folder[begin] == '-') {
        output.push_back("");
        for (int i = begin + 2; i <= end; i++) output[output.size() - 1].push_back(folder[i]);
        end = std::max(0, begin - 2);
        begin = std::max(0, begin - 2);
        while (begin >= 0 and !isdigit(folder[begin])) begin--;
        begin = std::max(0, begin);
        if (isdigit(folder[begin])) {
            output.push_back("");
            for (int i = begin + 2; i <= end; i++) output[output.size() - 1].push_back(folder[i]);
            end = std::max(0, begin);
            begin = std::max(0, begin);
            begin = 0;
            if (isdigit(folder[begin])) {
                output.push_back("");
                for (int i = begin; i <= end; i++) output[output.size() - 1].push_back(folder[i]);
                end = std::max(0, begin - 2);
                begin = std::max(0, begin - 2);
            }
        } else if (begin == 0) {
            output.push_back("");
            for (int i = begin; i <= end; i++) output[output.size() - 1].push_back(folder[i]);
            end = std::max(0, begin);
            begin = std::max(0, begin);
        }
    } else {
        output.push_back(folder);
    }

    if (output.size() == 0) {
        output.push_back(folder);
    }
    return output;
}

std::vector<std::string> ParseNameFile(std::string file) {
    std::vector<std::string> output;
    Parser parser = Parser();
    GameFile geym = parser.parseMetadata(file);
    output.push_back(geym.configMetadata["Title"]);
    output.push_back(geym.configMetadata["Artist"]);
    output.push_back(geym.configMetadata["Creator"]);
    output.push_back(geym.configMetadata["Version"]);
    output.push_back(geym.configMetadata["BeatmapSetID"]);
    return output;
}

// ============== Timer functions ============== //

void initTimer() { Global.start = std::chrono::steady_clock::now(); };
void pauseTimer();
void resumeTimer();
double getTimer() {
    Global.end = std::chrono::steady_clock::now();
    return (Global.end - Global.start).count() / 1000000.0;
};
void addOffsetTimer(unsigned long long int time) { Global.pausedFor += time; }
void updateTimer();

// ============== Raylib functions ============== //

bool IsTextureReady(Texture2D texture) {
    return ((texture.id > 0) &&                             // Validate OpenGL id
            (texture.width > 0) && (texture.height > 0) &&  // Validate texture size
            (texture.format > 0) &&                         // Validate texture pixel format
            (texture.mipmaps > 0));  // Validate texture mipmaps (at least 1 for basic mipmap level)
}

bool IsRenderTextureReady(RenderTexture2D target) {
    return ((target.id > 0) &&                // Validate OpenGL id
            IsTextureReady(target.depth) &&   // Validate FBO depth texture/renderbuffer
            IsTextureReady(target.texture));  // Validate FBO texture
}

float getAngle(Vector2 p1, Vector2 p2) { return atan2(p1.y - p2.y, p1.x - p2.x); }

std::vector<std::string> getAudioFilenames(int, int, int, int, int, int, int, std::string);
std::vector<std::string> getAudioFilenames(int timingSet, int timingSampleIndex,
                                           int defaultSampleSet, int normalSet, int additionSet,
                                           int hitSound, int index, std::string filename) {
    int defaultSampleSetForObject = 0;
    int defaultSampleIndexForObject = timingSampleIndex;

    if (timingSet == 1)
        defaultSampleSetForObject = 0;
    else if (timingSet == 2)
        defaultSampleSetForObject = 1;
    else if (timingSet == 3)
        defaultSampleSetForObject = 2;
    else
        defaultSampleSetForObject = defaultSampleSet;

    int NormalSetForObject = 0;

    int HitSoundForObject = 0;
    int SampleIndexForObject = 0;

    if (normalSet == 1)
        NormalSetForObject = 0;
    else if (normalSet == 2)
        NormalSetForObject = 1;
    else if (normalSet == 3)
        NormalSetForObject = 2;
    else
        NormalSetForObject = defaultSampleSetForObject;

    if (hitSound == 2)
        HitSoundForObject = 1;
    else if (hitSound == 4)
        HitSoundForObject = 2;
    else if (hitSound == 8)
        HitSoundForObject = 3;
    else
        HitSoundForObject = 0;

    if (index != 0)
        SampleIndexForObject = index;
    else
        SampleIndexForObject = defaultSampleIndexForObject;

    std::string HitSoundIndex = "";

    if (!(SampleIndexForObject == 0)) {
        HitSoundIndex = std::to_string(SampleIndexForObject);
    }

    std::string NormalFileName;

    if (NormalSetForObject == 0)
        NormalFileName = "normal-hitnormal";
    else if (NormalSetForObject == 1)
        NormalFileName = "soft-hitnormal";
    else
        NormalFileName = "drum-hitnormal";

    std::vector<std::string> out;

    out.push_back(NormalFileName + HitSoundIndex);
    out.push_back(NormalFileName);

    std::string AdditionFilename;

    int AdditionSetForObject = 0;

    if (additionSet == 1)
        AdditionSetForObject = 0;
    else if (additionSet == 2)
        AdditionSetForObject = 1;
    else if (additionSet == 3)
        AdditionSetForObject = 2;
    else
        AdditionSetForObject = NormalSetForObject;

    if (AdditionSetForObject == 0)
        AdditionFilename = "normal-hit";
    else if (AdditionSetForObject == 1)
        AdditionFilename = "soft-hit";
    else
        AdditionFilename = "drum-hit";

    if (filename.size() > 4) {
        if (filename[filename.size() - 5] == '.') {
            for (int i = 0; i < 4; i++) filename.pop_back();
        }
        out.push_back(filename);
        out.push_back(AdditionFilename);
    } else {
        if ((hitSound & (1 << 1)) != 0) {
            out.push_back(AdditionFilename + "whistle" + HitSoundIndex);
            out.push_back(AdditionFilename + "whistle");
        }
        if ((hitSound & (1 << 2)) != 0) {
            out.push_back(AdditionFilename + "finish" + HitSoundIndex);
            out.push_back(AdditionFilename + "finish");
        }
        if ((hitSound & (1 << 3)) != 0) {
            out.push_back(AdditionFilename + "clap" + HitSoundIndex);
            out.push_back(AdditionFilename + "clap");
        }
    }

    return out;
}
