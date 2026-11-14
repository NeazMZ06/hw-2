#pragma once

#include <algorithm>
#include <string>

#include "CollisionComponent.h"
#include "ECSEngine.h"
#include "GravityComponent.h"
#include "Keyboard.h"
#include "MovementComponent.h"

namespace ECSEngine {

const float kMaxRunVelocity = 15;
const float kDashVelocity = 50;
const float kJumpVelocity = 25;
const float kGroundRunAcceleration = 5;
const float kAirRunAcceleration = 4;
// Starter hint, in frames. The behavior spec asks for at most one dash per second.
[[maybe_unused]] const float kDashInterval = 25;
const int kDashCooldownFrames = 60;
const int kDashBurstFrames = 10;

template <typename... Components>
class InputSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("InputSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entities = engine->GetEntityManager();
        auto& sounds = engine->GetSoundManager();
        for (auto it = entities.begin(); it != entities.end(); ++it) {
            if (!it->IsValid()) {
                continue;
            }
            const EntityID id = it->GetID();
            if (!entities.template HasComponent<InputComponent>(id) ||
                !entities.template HasComponent<MovementComponent>(id) ||
                !entities.template HasComponent<CollisionComponent>(id)) {
                continue;
            }
            auto& input = entities.template GetComponent<InputComponent>(id);
            auto& movement = entities.template GetComponent<MovementComponent>(id);
            const auto& collision = entities.template GetComponent<CollisionComponent>(id);

            const bool left = input.keydown.test(Key::Left);
            const bool right = input.keydown.test(Key::Right);
            const bool jumpEdge = input.keydown.test(Key::Z) && !input.lastKeydown.test(Key::Z);
            const bool jumpRelease = !input.keydown.test(Key::Z) && input.lastKeydown.test(Key::Z);
            const bool dashEdge = input.keydown.test(Key::C) && !input.lastKeydown.test(Key::C);

            bool startedDash = false;
            if (dashEdge && input.lastDash >= kDashCooldownFrames) {
                float direction = 1.f;
                if (left && !right) {
                    direction = -1.f;
                } else if (!left && right) {
                    direction = 1.f;
                } else if (movement.velocity.x < 0.f) {
                    direction = -1.f;
                }
                movement.velocity.x = direction * kDashVelocity;
                input.lastDash = 0;
                startedDash = true;
                if (entities.template HasComponent<GravityComponent>(id)) {
                    entities.template GetComponent<GravityComponent>(id).disabledTimer = kDashBurstFrames;
                }
            }

            const bool inBurst = startedDash || input.lastDash < kDashBurstFrames;
            if (!inBurst) {
                float target = 0.f;
                if (left) {
                    target -= kMaxRunVelocity;
                }
                if (right) {
                    target += kMaxRunVelocity;
                }
                const float accel =
                    collision.onGround != 0 ? kGroundRunAcceleration : kAirRunAcceleration;
                if (movement.velocity.x < target) {
                    movement.velocity.x = std::min(target, movement.velocity.x + accel);
                } else if (movement.velocity.x > target) {
                    movement.velocity.x = std::max(target, movement.velocity.x - accel);
                }
            }

            if (jumpEdge && collision.onGround != 0) {
                movement.velocity.y = -kJumpVelocity;
                sounds.GetSound("jump").play();
            } else if (jumpEdge && movement.velocity.y > 0.f &&
                       ((collision.onLeftWall != 0 && left) || (collision.onRightWall != 0 && right))) {
                movement.velocity.y = -kJumpVelocity * 0.92f;
                movement.velocity.x = collision.onLeftWall != 0 ? kMaxRunVelocity : -kMaxRunVelocity;
                sounds.GetSound("jump").play();
            }

            if (jumpRelease && movement.velocity.y < 0.f) {
                movement.velocity.y *= 0.45f;
            }

            if (!startedDash) {
                ++input.lastDash;
            }
            input.lastKeydown = input.keydown;
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
