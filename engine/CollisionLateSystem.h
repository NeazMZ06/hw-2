#pragma once

#include "CollisionComponent.h"
#include "ECSEngine.h"
#include "LocationComponent.h"

namespace ECSEngine {

// Copies each collision box into previousCollisionBox before movement this frame.
template <typename... Components>
class CollisionLateSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("CollisionLateSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entities = engine->GetEntityManager();
        for (auto it = entities.begin(); it != entities.end(); ++it) {
            if (!it->IsValid()) {
                continue;
            }
            const EntityID id = it->GetID();
            if (!entities.template HasComponent<LocationComponent>(id) ||
                !entities.template HasComponent<CollisionComponent>(id)) {
                continue;
            }
            const auto& loc = entities.template GetComponent<LocationComponent>(id);
            auto& collision = entities.template GetComponent<CollisionComponent>(id);
            collision.previousCollisionBox = WorldRect(loc.loc, collision.collisionBox);
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
