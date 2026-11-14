#pragma once

#include <cstddef>
#include <string>

// An entity can have any component it is templated with,
// but does not have these installed by default.
// This is a base class for all entities in the ECS system.

namespace ECSEngine {

using EntityID = size_t;

// EntityID is an index into EntityManager's entity vector. Removed entities leave
// a gap in that vector; IsValid() is false for those gaps and for ids that were
// never issued. A freed id may later be reused by CreateEntity.
class Entity {
public:
    EntityID GetID() const { return mID; }

    // False after RemoveEntity, until this id is reused.
    bool IsValid() const { return mAlive; }

private:
    Entity() = default;
    Entity(EntityID eid, const std::string& name) : mID(eid), mType(name), mAlive(true) {}
    EntityID mID = 0;
    std::string mType;
    bool mAlive = false;

    template <typename... T>
    friend class EntityManager;
};

}  // namespace ECSEngine
