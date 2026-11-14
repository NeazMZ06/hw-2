#pragma once

#include "MathUtil.h"

namespace ECSEngine {

// World location restored when the entity dies.
struct RespawnComponent {
    Point2D loc;
};

}  // namespace ECSEngine
