#pragma once

#include "MathUtil.h"

namespace ECSEngine {

// Screen shake added on top of the camera for frameLimit frames.
struct CameraShake {
    int frameLimit = 0;
    Point2D offset;
};

}  // namespace ECSEngine
