#pragma once

#include <cmath>
#include <random>
#include <string>
#include <vector>

#include "CollisionComponent.h"
#include "ECSEngine.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"

namespace ECSEngine {

template <typename... Components>
class SpawnSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("SpawnSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entities = engine->GetEntityManager();
        struct Job {
            EntityID spawner;
            Point2D at;
            SpriteID texture;
            int interval;
        };
        std::vector<Job> jobs;

        for (auto it = entities.begin(); it != entities.end(); ++it) {
            if (!it->IsValid()) {
                continue;
            }
            const EntityID id = it->GetID();
            if (!entities.template HasComponent<SpawnComponent>(id) ||
                !entities.template HasComponent<LocationComponent>(id)) {
                continue;
            }
            auto& spawn = entities.template GetComponent<SpawnComponent>(id);
            if (spawn.totalSpawns <= 0) {
                continue;
            }
            if (spawn.timeToNextSpawn > 0) {
                --spawn.timeToNextSpawn;
                continue;
            }
            const auto& loc = entities.template GetComponent<LocationComponent>(id);
            jobs.push_back(Job{id, Point2D(loc.loc.x, loc.loc.y - 72.f), spawn.spawnTextureID,
                               spawn.timeBetweenSpawns});
        }

        static std::mt19937 rng{std::random_device{}()};
        std::uniform_real_distribution<float> spread(-7.f, 7.f);
        for (const Job& job : jobs) {
            const EntityID star = entities.CreateEntity("star");
            LocationComponent loc;
            loc.loc = job.at;
            entities.template AddComponent<LocationComponent>(star, loc);

            SpriteComponent sprite;
            sprite.textureID = job.texture;
            sprite.bounds = MakeRect(0.f, 0.f, 48.f, 48.f);
            sprite.worldSpace = true;
            entities.template AddComponent<SpriteComponent>(star, sprite);

            float vx = spread(rng);
            float vy = spread(rng);
            if (std::fabs(vx) < 3.f) {
                vx = vx < 0.f ? -5.f : 5.f;
            }
            if (std::fabs(vy) < 2.f) {
                vy = -4.f;
            }
            entities.template AddComponent<MovementComponent>(star, MovementComponent{Point2D(vx, vy)});

            CollisionComponent collision{};
            collision.staticEntity = 0;
            collision.collisionBox = MakeRect(6.f, 6.f, 36.f, 36.f);
            collision.previousCollisionBox = WorldRect(loc.loc, collision.collisionBox);
            entities.template AddComponent<CollisionComponent>(star, collision);

            if (entities.IsValid(job.spawner) &&
                entities.template HasComponent<SpawnComponent>(job.spawner)) {
                auto& spawn = entities.template GetComponent<SpawnComponent>(job.spawner);
                spawn.timeToNextSpawn = job.interval;
                if (spawn.totalSpawns > 0) {
                    --spawn.totalSpawns;
                }
            }
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
