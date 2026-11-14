#include <iostream>

#include "EntityManager.h"

namespace {

struct Position {
    float x = 0.f;
    float y = 0.f;
};

struct Health {
    int hp = 0;
};

bool Fail(const char* message) {
    std::cerr << "EntityManager test failed: " << message << '\n';
    return false;
}

}  // namespace

int main() {
    using Manager = ECSEngine::EntityManager<Position, Health>;
    Manager manager;

    const ECSEngine::EntityID first = manager.CreateEntity("first");
    const ECSEngine::EntityID second = manager.CreateEntity("second");
    if (first != 0 || second != 1) {
        return Fail("ids are not dense on first create") ? 0 : 1;
    }
    if (!manager.IsValid(first) || !manager.IsValid(second)) {
        return 1;
    }

    manager.AddComponent<Position>(first, Position{3.f, 4.f});
    manager.AddComponent<Health>(first, Health{9});
    if (!manager.HasComponent<Position>(first) || manager.HasComponent<Position>(second)) {
        return Fail("HasComponent") ? 0 : 1;
    }
    if (manager.GetComponent<Position>(first).x != 3.f || manager.GetComponent<Health>(first).hp != 9) {
        return Fail("GetComponent") ? 0 : 1;
    }

    manager.AddComponent<Position>(first, Position{8.f, 1.f});
    if (manager.GetComponent<Position>(first).x != 8.f) {
        return Fail("replacing a component") ? 0 : 1;
    }

    manager.RemoveComponent<Position>(first);
    if (manager.HasComponent<Position>(first) || !manager.HasComponent<Health>(first)) {
        return Fail("RemoveComponent") ? 0 : 1;
    }
    manager.AddComponent<Position>(first, Position{1.f, 1.f});
    if (!manager.HasComponent<Position>(first)) {
        return Fail("free list reuse") ? 0 : 1;
    }

    manager.RemoveEntity(first);
    if (manager.IsValid(first) || manager.HasComponent<Health>(first)) {
        return Fail("RemoveEntity") ? 0 : 1;
    }

    int alive = 0;
    bool sawGap = false;
    for (auto it = manager.cbegin(); it != manager.cend(); ++it) {
        if (!it->IsValid()) {
            sawGap = true;
            continue;
        }
        ++alive;
    }
    if (!sawGap || alive != 1) {
        return Fail("iteration did not expose the gap") ? 0 : 1;
    }

    const ECSEngine::EntityID reused = manager.CreateEntity("reused");
    if (reused != first || !manager.IsValid(reused) || manager.HasComponent<Position>(reused)) {
        return Fail("id reuse") ? 0 : 1;
    }

    std::cout << "EntityManager tests passed\n";
    return 0;
}
