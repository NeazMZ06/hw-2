#pragma once

#include <algorithm>
#include <string>

#include "CollisionComponent.h"
#include "ECSEngine.h"
#include "GravityComponent.h"
#include "InputComponent.h"
#include "Keyboard.h"
#include "MovementComponent.h"

namespace ECSEngine {

template <typename... Components>
class GravitySystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("GravitySystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entities = engine->GetEntityManager();
        for (auto it = entities.begin(); it != entities.end(); ++it) {
            if (!it->IsValid()) {
                continue;
            }
            const EntityID id = it->GetID();
            if (!entities.template HasComponent<GravityComponent>(id) ||
                !entities.template HasComponent<MovementComponent>(id) ||
                !entities.template HasComponent<CollisionComponent>(id)) {
                continue;
            }
            auto& gravity = entities.template GetComponent<GravityComponent>(id);
            auto& movement = entities.template GetComponent<MovementComponent>(id);
            const auto& collision = entities.template GetComponent<CollisionComponent>(id);
            if (collision.staticEntity != 0) {
                continue;
            }
            if (gravity.disabledTimer > 0) {
                --gravity.disabledTimer;
                continue;
            }

            float accel = gravity.acceleration.y;
            if (entities.template HasComponent<InputComponent>(id)) {
                const auto& input = entities.template GetComponent<InputComponent>(id);
                if (input.keydown.test(Key::Down)) {
                    accel = 2.8f;
                }
                const bool intoWall =
                    (collision.onLeftWall != 0 && input.keydown.test(Key::Left)) ||
                    (collision.onRightWall != 0 && input.keydown.test(Key::Right));
                if (movement.velocity.y > 0.f && intoWall) {
                    accel = 0.22f;
                }
            }
            movement.velocity.y += accel;
            movement.velocity.y = std::min(movement.velocity.y, 18.f);
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
