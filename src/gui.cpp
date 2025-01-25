#include "gui.hpp"

#include <cstring>
#include <iostream>

#include "globals.hpp"
#include "utils.hpp"

GuiElement::GuiElement(Vector2 position, Vector2 size) : position(position), size(size) {}

void GuiElement::update() {}

void GuiElement::render() {}

Rectangle GuiElement::getRect() {
    return GetRaylibOriginR({position.x, position.y, size.x, size.y});
}

Button::Button(Vector2 position, Vector2 size, Color color, char* text, Color textcolor,
               int textsize)
    : GuiElement(position, size),
      color(color),
      text(text),
      textcolor(textcolor),
      textsize(textsize) {}

void Button::render() {
    Vector2 TextBoxSize = MeasureTextEx(Global.DefaultFont, text, textsize, 1);
    Vector2 TextBoxLocation = GetRaylibOrigin(
        {GetCenter(this->getRect()).x, GetCenter(this->getRect()).y, TextBoxSize.x, TextBoxSize.y});
    if ((this->clicked or (Global.Key1D and this->focused)) and !this->focusbreak) {
        DrawRectangleRec(ScaleRect(this->getRect()), textcolor);
        DrawTextEx(Global.DefaultFont, text, ScaleCords(TextBoxLocation), Scale(textsize), Scale(1),
                   this->color);
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
    } else if (this->focused) {
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawRectangleLinesEx(ScaleRect(this->getRect()), Scale(2), WHITE);
        DrawTextEx(Global.DefaultFont, text, ScaleCords(TextBoxLocation), Scale(textsize), Scale(1),
                   textcolor);
    } else {
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawTextEx(Global.DefaultFont, text, ScaleCords(TextBoxLocation), Scale(textsize), Scale(1),
                   textcolor);
    }
}

void Button::update() {
    bool hover = CheckCollisionPointRec(Global.MousePosition, this->getRect());
    bool click = Global.MouseInFocus and Global.Key1P;

    if (hover and click) {
        this->focused = true;
        this->clicked = true;
        this->focusbreak = false;
    } else if (hover) {
        this->focused = true;
        this->clicked = false;
    } else {
        this->focused = false;
        this->clicked = false;
        this->focusbreak = true;
    }

    if (hover and !focusbreak and Global.Key1R)
        action = true;
    else
        action = false;
}

TextBox::TextBox(Vector2 position, Vector2 size, Color color, std::string text, Color textcolor,
                 int textsize, int maxlength)
    : GuiElement(position, size),
      color(color),
      text(text),
      textcolor(textcolor),
      textsize(textsize),
      maxlength(maxlength) {
    init();
}

void TextBox::render() {
    Vector2 TextBoxSize = MeasureTextEx(Global.DefaultFont, rendertext.c_str(), textsize, 1);
    Vector2 TextBoxLocation = GetRaylibOrigin(
        {GetCenter(this->getRect()).x, GetCenter(this->getRect()).y, TextBoxSize.x, TextBoxSize.y});
    DrawRectangleRec(ScaleRect(this->getRect()), BLUE);
    if (focused) {
        DrawRectangleRec(ScaleRect(this->getRect()), textcolor);
        if (renderpos > -1)
            DrawTextEx(Global.DefaultFont, subtext.c_str(), ScaleCords(TextBoxLocation),
                       Scale(textsize), Scale(1), this->color);
        else
            DrawTextEx(Global.DefaultFont, rendertext.c_str(), ScaleCords(TextBoxLocation),
                       Scale(textsize), Scale(1), this->color);
    } else if (hover) {
        Color tempColor = this->color;
        tempColor.r = (this->color.r * 2 + textcolor.r) / 3;
        tempColor.g = (this->color.g * 2 + textcolor.g) / 3;
        tempColor.b = (this->color.b * 2 + textcolor.b) / 3;
        tempColor.a = (this->color.a * 2 + textcolor.a) / 3;
        DrawRectangleRec(ScaleRect(this->getRect()), tempColor);
        DrawTextEx(Global.DefaultFont, rendertext.c_str(), ScaleCords(TextBoxLocation),
                   Scale(textsize), Scale(1), textcolor);
    } else {
        DrawRectangleRec(ScaleRect(this->getRect()), this->color);
        DrawTextEx(Global.DefaultFont, rendertext.c_str(), ScaleCords(TextBoxLocation),
                   Scale(textsize), Scale(1), textcolor);
    }
}
void TextBox::init() {
    rendertext = text;
    maxlength = rendertext.length();
    while (true) {
        if (maxlength <= 0) break;
        Vector2 TextBoxSize =
            MeasureTextEx(Global.DefaultFont, rendertext.substr(1, maxlength).c_str(), textsize, 1);
        if (TextBoxSize.x > (size.x * 0.95f))
            maxlength--;
        else
            break;
    }
    std::cout << maxlength << std::endl;
    if (text.length() > maxlength) {
        rendertext = text;
        while (rendertext.length() > 0 and rendertext.length() > maxlength - 3) {
            rendertext.pop_back();
        }
        rendertext.push_back('.');
        rendertext.push_back('.');
        rendertext.push_back('.');
    }
}

void TextBox::update() {
    if (text.size() > maxlength) {
        if (focused) {
            if (counter < 0.f) counter = 0.f;
            counter += (Global.FrameTime / 1000.0f) * 1000.0f;
            while (counter > 1000.0f) {
                renderpos++;
                if (renderpos > (int)text.size()) {
                    renderpos = 0;
                }
                if (renderpos + maxlength <= (int)text.size()) {
                    subtext = text.substr(renderpos, maxlength);
                } else {
                    subtext = text.substr(renderpos, text.size() - renderpos) + "|" +
                              text.substr(0, maxlength - (text.size() - renderpos) - 1);
                }
                counter -= 100.0f;
            }
        } else {
            counter = -100.0f;
            renderpos = -1;
        }
    }
}
