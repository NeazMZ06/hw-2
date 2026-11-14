#pragma once

#include "MathUtil.h"

namespace ECSEngine {

// Velocity is applied once per frame by MovementSystem.
struct MovementComponent {
    Point2D velocity;  // Pixels per frame
};

}  // namespace ECSEngine
