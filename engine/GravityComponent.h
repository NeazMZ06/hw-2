#pragma once

#include "MathUtil.h"

namespace ECSEngine {

// Acceleration added to velocity each frame while disabledTimer is zero.
struct GravityComponent {
    GravityComponent(Point2D acceleration, int disabledTimer = 0)
        : acceleration(acceleration), disabledTimer(disabledTimer) {}
    Point2D acceleration;  // Pixels per frame squared
    int disabledTimer;
};

}  // namespace ECSEngine
