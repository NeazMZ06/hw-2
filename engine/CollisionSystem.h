#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "CameraShakeComponent.h"
#include "CollisionComponent.h"
#include "ECSEngine.h"
#include "InputComponent.h"
#include "Keyboard.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "SpawnComponent.h"

namespace ECSEngine {

namespace CollisionDetail {

enum class Side { None, Ground, Ceiling, LeftWall, RightWall };

inline float Bottom(const Rect& r) { return r.topLeft.y + r.height; }
inline float Right(const Rect& r) { return r.topLeft.x + r.width; }

inline void Place(LocationComponent& loc, const Rect& relative, const Rect& world) {
    loc.loc.x = world.topLeft.x - relative.topLeft.x;
    loc.loc.y = world.topLeft.y - relative.topLeft.y;
}

inline Side Classify(const Rect& previous, const Rect& tile, const Rect& mover) {
    const float eps = 1.25f;
    if (Bottom(previous) <= tile.topLeft.y + eps) {
        return Side::Ground;
    }
    if (previous.topLeft.y >= Bottom(tile) - eps) {
        return Side::Ceiling;
    }
    if (Right(previous) <= tile.topLeft.x + eps) {
        return Side::RightWall;
    }
    if (previous.topLeft.x >= Right(tile) - eps) {
        return Side::LeftWall;
    }
    const float penX = std::min(Right(mover), Right(tile)) - std::max(mover.topLeft.x, tile.topLeft.x);
    const float penY = std::min(Bottom(mover), Bottom(tile)) - std::max(mover.topLeft.y, tile.topLeft.y);
    const float moverCx = mover.topLeft.x + mover.width * 0.5f;
    const float tileCx = tile.topLeft.x + tile.width * 0.5f;
    const float moverCy = mover.topLeft.y + mover.height * 0.5f;
    const float tileCy = tile.topLeft.y + tile.height * 0.5f;
    if (penX < penY) {
        return moverCx < tileCx ? Side::RightWall : Side::LeftWall;
    }
    return moverCy < tileCy ? Side::Ground : Side::Ceiling;
}

inline void Separate(Rect& mover, const Rect& tile, Side side) {
    if (side == Side::Ground) {
        mover.topLeft.y = tile.topLeft.y - mover.height;
    } else if (side == Side::Ceiling) {
        mover.topLeft.y = Bottom(tile);
    } else if (side == Side::RightWall) {
        mover.topLeft.x = tile.topLeft.x - mover.width;
    } else if (side == Side::LeftWall) {
        mover.topLeft.x = Right(tile);
    }
}

inline void ApplyVelocity(Point2D& velocity, Side side, bool bounce) {
    constexpr float kRestitution = 0.7f;
    if (side == Side::Ground) {
        velocity.y = bounce ? -std::fabs(velocity.y) * kRestitution : std::min(velocity.y, 0.f);
    } else if (side == Side::Ceiling) {
        velocity.y = bounce ? std::fabs(velocity.y) * kRestitution : std::max(velocity.y, 0.f);
    } else if (side == Side::RightWall) {
        velocity.x = bounce ? -std::fabs(velocity.x) * kRestitution : std::min(velocity.x, 0.f);
    } else if (side == Side::LeftWall) {
        velocity.x = bounce ? std::fabs(velocity.x) * kRestitution : std::max(velocity.x, 0.f);
    }
    if (bounce) {
        if (std::fabs(velocity.x) < 0.35f) {
            velocity.x = 0.f;
        }
        if (std::fabs(velocity.y) < 0.35f) {
            velocity.y = 0.f;
        }
    }
}

template <typename... Components>
bool IsPlayer(EntityManager<Components...>& entities, EntityID id) {
    return entities.template HasComponent<InputComponent>(id);
}

template <typename... Components>
bool IsStar(EntityManager<Components...>& entities, EntityID id) {
    if (!entities.template HasComponent<MovementComponent>(id) ||
        !entities.template HasComponent<CollisionComponent>(id)) {
        return false;
    }
    if (entities.template GetComponent<CollisionComponent>(id).staticEntity != 0) {
        return false;
    }
    return !entities.template HasComponent<InputComponent>(id) &&
           !entities.template HasComponent<SpawnComponent>(id);
}

template <typename... Components>
void AddScore(EntityManager<Components...>& entities) {
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (!it->IsValid() || !entities.template HasComponent<ScoreComponent>(it->GetID())) {
            continue;
        }
        entities.template GetComponent<ScoreComponent>(it->GetID()).score += 1;
        return;
    }
}

template <typename... Components>
void Shake(EntityManager<Components...>& entities, float x, float y) {
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (!it->IsValid() || !entities.template HasComponent<CameraShake>(it->GetID())) {
            continue;
        }
        auto& shake = entities.template GetComponent<CameraShake>(it->GetID());
        shake.offset = Point2D(x, y);
        shake.frameLimit = 6;
        return;
    }
}

template <typename... Components>
void Respawn(EntityManager<Components...>& entities, EntityID id, SoundManager& sounds) {
    if (entities.template HasComponent<RespawnComponent>(id) &&
        entities.template HasComponent<LocationComponent>(id)) {
        entities.template GetComponent<LocationComponent>(id).loc =
            entities.template GetComponent<RespawnComponent>(id).loc;
    }
    if (entities.template HasComponent<MovementComponent>(id)) {
        entities.template GetComponent<MovementComponent>(id).velocity = Point2D(0.f, 0.f);
    }
    if (entities.template HasComponent<CollisionComponent>(id)) {
        auto& collision = entities.template GetComponent<CollisionComponent>(id);
        collision.onGround = 0;
        collision.onCeiling = 0;
        collision.onLeftWall = 0;
        collision.onRightWall = 0;
        if (entities.template HasComponent<LocationComponent>(id)) {
            collision.previousCollisionBox = WorldRect(
                entities.template GetComponent<LocationComponent>(id).loc, collision.collisionBox);
        }
    }
    sounds.GetSound("hurt").play();
}

}  // namespace CollisionDetail

template <typename... Components>
class CollisionSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("CollisionSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        using namespace CollisionDetail;
        auto& entities = engine->GetEntityManager();
        auto& sounds = engine->GetSoundManager();

        for (auto it = entities.begin(); it != entities.end(); ++it) {
            if (!it->IsValid()) {
                continue;
            }
            const EntityID id = it->GetID();
            if (!entities.template HasComponent<CollisionComponent>(id) ||
                !entities.template HasComponent<LocationComponent>(id) ||
                !entities.template HasComponent<MovementComponent>(id)) {
                continue;
            }
            auto& collision = entities.template GetComponent<CollisionComponent>(id);
            if (collision.staticEntity != 0) {
                continue;
            }

            const bool player = IsPlayer(entities, id);
            const bool star = IsStar(entities, id);
            const bool wasGround = collision.onGround != 0;
            const bool wasLeft = collision.onLeftWall != 0;
            const bool wasRight = collision.onRightWall != 0;
            collision.onGround = 0;
            collision.onCeiling = 0;
            collision.onLeftWall = 0;
            collision.onRightWall = 0;

            bool died = false;
            float impact = 0.f;
            Side strongest = Side::None;
            auto& movement = entities.template GetComponent<MovementComponent>(id);

            for (int pass = 0; pass < 3 && !died; ++pass) {
                for (auto other = entities.begin(); other != entities.end(); ++other) {
                    if (!other->IsValid() || other->GetID() == id) {
                        continue;
                    }
                    const EntityID otherId = other->GetID();
                    if (!entities.template HasComponent<CollisionComponent>(otherId) ||
                        !entities.template HasComponent<LocationComponent>(otherId)) {
                        continue;
                    }
                    auto& otherCollision = entities.template GetComponent<CollisionComponent>(otherId);
                    auto& selfLoc = entities.template GetComponent<LocationComponent>(id);
                    const auto& otherLoc = entities.template GetComponent<LocationComponent>(otherId);
                    Rect selfWorld = WorldRect(selfLoc.loc, collision.collisionBox);
                    const Rect otherWorld = WorldRect(otherLoc.loc, otherCollision.collisionBox);
                    if (!RectsOverlap(selfWorld, otherWorld)) {
                        continue;
                    }

                    if (otherCollision.staticEntity != 0) {
                        if (player && otherCollision.doesDamage != 0) {
                            Respawn(entities, id, sounds);
                            died = true;
                            break;
                        }
                        const Point2D incoming = movement.velocity;
                        const Side side =
                            Classify(collision.previousCollisionBox, otherWorld, selfWorld);
                        Separate(selfWorld, otherWorld, side);
                        ApplyVelocity(movement.velocity, side, star);
                        Place(selfLoc, collision.collisionBox, selfWorld);
                        if (side == Side::Ground) {
                            collision.onGround = 1;
                        } else if (side == Side::Ceiling) {
                            collision.onCeiling = 1;
                        } else if (side == Side::LeftWall) {
                            collision.onLeftWall = 1;
                        } else if (side == Side::RightWall) {
                            collision.onRightWall = 1;
                        }
                        const float hitSpeed = std::fabs(incoming.x) + std::fabs(incoming.y);
                        if (hitSpeed > impact) {
                            impact = hitSpeed;
                            strongest = side;
                        }
                    } else if (player && IsStar(entities, otherId)) {
                        AddScore(entities);
                        entities.RemoveEntity(otherId);
                        sounds.GetSound("coin").play();
                    }
                }
            }
            if (died) {
                continue;
            }

            if (player && entities.template HasComponent<LocationComponent>(id) &&
                entities.template GetComponent<LocationComponent>(id).loc.y > 1200.f) {
                Respawn(entities, id, sounds);
                continue;
            }

            if (movement.velocity.y >= 0.f) {
                auto& selfLoc = entities.template GetComponent<LocationComponent>(id);
                Rect body = WorldRect(selfLoc.loc, collision.collisionBox);
                for (auto other = entities.begin(); other != entities.end(); ++other) {
                    if (!other->IsValid() || other->GetID() == id) {
                        continue;
                    }
                    const EntityID otherId = other->GetID();
                    if (!entities.template HasComponent<CollisionComponent>(otherId) ||
                        !entities.template HasComponent<LocationComponent>(otherId)) {
                        continue;
                    }
                    const auto& otherCollision = entities.template GetComponent<CollisionComponent>(otherId);
                    if (otherCollision.staticEntity == 0 || otherCollision.doesDamage != 0) {
                        continue;
                    }
                    const Rect tile =
                        WorldRect(entities.template GetComponent<LocationComponent>(otherId).loc,
                                  otherCollision.collisionBox);
                    const bool xOverlap = body.topLeft.x < Right(tile) && Right(body) > tile.topLeft.x;
                    const float gap = tile.topLeft.y - Bottom(body);
                    if (xOverlap && gap >= -1.f && gap <= 4.f) {
                        body.topLeft.y = tile.topLeft.y - body.height;
                        Place(selfLoc, collision.collisionBox, body);
                        collision.onGround = 1;
                        movement.velocity.y = std::min(movement.velocity.y, 0.f);
                        break;
                    }
                }
            }

            if (!player) {
                continue;
            }
            if (!wasGround && collision.onGround != 0) {
                sounds.GetSound("land").play();
            }
            if (entities.template HasComponent<InputComponent>(id)) {
                const auto& input = entities.template GetComponent<InputComponent>(id);
                const bool slideLeft = collision.onLeftWall != 0 && input.keydown.test(Key::Left);
                const bool slideRight = collision.onRightWall != 0 && input.keydown.test(Key::Right);
                const bool wasSlide = (wasLeft && input.keydown.test(Key::Left)) ||
                                      (wasRight && input.keydown.test(Key::Right));
                if (movement.velocity.y > 0.f && (slideLeft || slideRight) && !wasSlide) {
                    sounds.GetSound("wall").play();
                }
            }
            if (impact > 4.f && strongest != Side::None) {
                float sx = 0.f;
                float sy = 0.f;
                const float mag = std::min(10.f, impact * 0.18f);
                if (strongest == Side::Ground) {
                    sy = mag;
                } else if (strongest == Side::Ceiling) {
                    sy = -mag;
                } else if (strongest == Side::LeftWall) {
                    sx = -mag;
                } else if (strongest == Side::RightWall) {
                    sx = mag;
                }
                Shake(entities, sx, sy);
            }
        }

        std::vector<EntityID> stars;
        for (auto it = entities.begin(); it != entities.end(); ++it) {
            if (it->IsValid() && IsStar(entities, it->GetID())) {
                stars.push_back(it->GetID());
            }
        }
        for (size_t i = 0; i < stars.size(); ++i) {
            if (!entities.IsValid(stars[i])) {
                continue;
            }
            for (size_t j = i + 1; j < stars.size(); ++j) {
                if (!entities.IsValid(stars[j])) {
                    continue;
                }
                auto& aLoc = entities.template GetComponent<LocationComponent>(stars[i]);
                auto& bLoc = entities.template GetComponent<LocationComponent>(stars[j]);
                auto& aCol = entities.template GetComponent<CollisionComponent>(stars[i]);
                auto& bCol = entities.template GetComponent<CollisionComponent>(stars[j]);
                Rect aWorld = WorldRect(aLoc.loc, aCol.collisionBox);
                Rect bWorld = WorldRect(bLoc.loc, bCol.collisionBox);
                if (!RectsOverlap(aWorld, bWorld)) {
                    continue;
                }
                auto& aMove = entities.template GetComponent<MovementComponent>(stars[i]);
                auto& bMove = entities.template GetComponent<MovementComponent>(stars[j]);
                const float penX =
                    std::min(Right(aWorld), Right(bWorld)) - std::max(aWorld.topLeft.x, bWorld.topLeft.x);
                const float penY =
                    std::min(Bottom(aWorld), Bottom(bWorld)) - std::max(aWorld.topLeft.y, bWorld.topLeft.y);
                if (penX < penY) {
                    const float push = penX * 0.5f;
                    if (aWorld.topLeft.x < bWorld.topLeft.x) {
                        aWorld.topLeft.x -= push;
                        bWorld.topLeft.x += push;
                    } else {
                        aWorld.topLeft.x += push;
                        bWorld.topLeft.x -= push;
                    }
                    std::swap(aMove.velocity.x, bMove.velocity.x);
                    aMove.velocity.x *= 0.85f;
                    bMove.velocity.x *= 0.85f;
                } else {
                    const float push = penY * 0.5f;
                    if (aWorld.topLeft.y < bWorld.topLeft.y) {
                        aWorld.topLeft.y -= push;
                        bWorld.topLeft.y += push;
                    } else {
                        aWorld.topLeft.y += push;
                        bWorld.topLeft.y -= push;
                    }
                    std::swap(aMove.velocity.y, bMove.velocity.y);
                    aMove.velocity.y *= 0.85f;
                    bMove.velocity.y *= 0.85f;
                }
                Place(aLoc, aCol.collisionBox, aWorld);
                Place(bLoc, bCol.collisionBox, bWorld);
            }
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
