#pragma once
#include <math.h>

#include <algorithm>
#include <globals.hpp>
#include <iostream>
#include <utils.hpp>
#include <vector>

static std::vector<Vector2> MouseTrail(Global.MouseTrailSize, {-10, -10});
static float FrameTimeCounterTrail = 0;
static int indexTrail = 0;

void initMouseTrail();

void updateMouseTrail();

void renderMouse();
