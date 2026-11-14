#pragma once

#include "Entity.h"

namespace ECSEngine {

// The camera tracks this entity on the x axis.
struct CameraFollower {
    EntityID entityID = 0;
};

}  // namespace ECSEngine
