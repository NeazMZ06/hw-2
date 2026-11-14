#pragma once

#include "MathUtil.h"

namespace ECSEngine {

// Top-left of an entity in world pixels. Sprite bounds and collision boxes are
// stored relative to this point, so moving loc moves those shapes with it.
struct LocationComponent {
    Point2D loc;
};

}  // namespace ECSEngine
