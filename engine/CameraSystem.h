#pragma once

#include <string>

#include "CameraComponent.h"
#include "CameraFollowerComponent.h"
#include "CameraShakeComponent.h"
#include "CollisionComponent.h"
#include "ECSEngine.h"
#include "LocationComponent.h"
#include "MovementComponent.h"

namespace ECSEngine {

template <typename... Components>
class CameraSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name = "CameraSystem";
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entities = engine->GetEntityManager();
        auto& window = engine->GetWindowManager();
        const Rect bounds = window.GetWindowBounds();

        for (auto it = entities.begin(); it != entities.end(); ++it) {
            if (!it->IsValid()) {
                continue;
            }
            const EntityID id = it->GetID();
            if (!entities.template HasComponent<CameraComponent>(id) ||
                !entities.template HasComponent<CameraFollower>(id)) {
                continue;
            }
            auto& camera = entities.template GetComponent<CameraComponent>(id);
            const auto& follower = entities.template GetComponent<CameraFollower>(id);
            if (!entities.IsValid(follower.entityID) ||
                !entities.template HasComponent<LocationComponent>(follower.entityID)) {
                continue;
            }

            camera.hardExclusion = MakeRect(bounds.width * 0.10f, 0.f, bounds.width * 0.80f, bounds.height);
            camera.softExclusion = MakeRect(bounds.width * 0.30f, 0.f, bounds.width * 0.40f, bounds.height);

            const auto& targetLoc = entities.template GetComponent<LocationComponent>(follower.entityID);
            float focusX = targetLoc.loc.x;
            if (entities.template HasComponent<CollisionComponent>(follower.entityID)) {
                const auto& box =
                    entities.template GetComponent<CollisionComponent>(follower.entityID).collisionBox;
                focusX = targetLoc.loc.x + box.topLeft.x + box.width * 0.5f;
            }
            const float screenX = window.WorldToWindow(Point2D(focusX, targetLoc.loc.y)).x;
            const float leftHard = camera.hardExclusion.topLeft.x;
            const float rightHard = leftHard + camera.hardExclusion.width;
            const float leftSoft = camera.softExclusion.topLeft.x;
            const float rightSoft = leftSoft + camera.softExclusion.width;
            const float scale = camera.worldUnitsPerPixel;

            Point2D center(window.GetWorldCenter().x, camera.position.y);
            if (screenX < leftHard) {
                center.x -= (leftHard - screenX) * scale;
            } else if (screenX > rightHard) {
                center.x += (screenX - rightHard) * scale;
            } else if (screenX < leftSoft) {
                center.x -= (leftSoft - screenX) * scale * camera.speed;
            } else if (screenX > rightSoft) {
                center.x += (screenX - rightSoft) * scale * camera.speed;
            }
            camera.position.x = center.x;

            if (entities.template HasComponent<CameraShake>(id)) {
                auto& shake = entities.template GetComponent<CameraShake>(id);
                if (shake.frameLimit > 0) {
                    center.x += shake.offset.x;
                    center.y += shake.offset.y;
                    shake.offset.x *= 0.65f;
                    shake.offset.y *= 0.65f;
                    --shake.frameLimit;
                }
            }

            window.SetWorldScale(scale);
            window.SetWorldCenter(center);
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
