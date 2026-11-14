#pragma once

#include <cstddef>
#include <vector>

namespace ECSEngine {

// Dense pool of one component type. Slots are not entity ids.
// mFreeList holds recycled slot indices so AddComponent does not scan for a hole.
// A reference returned from the pool is invalidated when this vector reallocates
// (an AddComponent that grows mStorage) and when that slot is removed.
template <typename T>
class ComponentStorage {
public:
    std::vector<T> mStorage;
    std::vector<bool> mValid;       // Quickly track if a component is valid
    std::vector<size_t> mFreeList;  // Avoid scanning for free slots
};

}  // namespace ECSEngine
