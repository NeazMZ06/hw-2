#pragma once

#include <string>

#include "ECSEngine.h"
#include "LocationComponent.h"
#include "MovementComponent.h"

namespace ECSEngine {

template <typename... Components>
class MovementSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("MovementSystem");
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
                !entities.template HasComponent<MovementComponent>(id)) {
                continue;
            }
            auto& loc = entities.template GetComponent<LocationComponent>(id);
            const auto& movement = entities.template GetComponent<MovementComponent>(id);
            loc.loc.x += movement.velocity.x;
            loc.loc.y += movement.velocity.y;
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
