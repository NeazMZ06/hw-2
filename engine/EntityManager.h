#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>

#include "Component.h"
#include "Entity.h"

namespace ECSEngine {

template <typename T, typename... Ts>
struct ComponentIndex;

template <typename T, typename... Rest>
struct ComponentIndex<T, T, Rest...> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename U, typename... Rest>
struct ComponentIndex<T, U, Rest...>
    : std::integral_constant<std::size_t, 1 + ComponentIndex<T, Rest...>::value> {};

// Entities are stored in a vector indexed by EntityID. RemoveEntity does not
// compact the vector, so iteration visits gaps. Skip any entity for which
// IsValid() is false. Freed ids are recycled.
//
// GetComponent returns a reference into that component's pool. The reference
// is invalidated by a later AddComponent of the same type that grows the pool,
// and by RemoveComponent / RemoveEntity for that slot. Do not hold it across
// those calls.
template <typename... Components>
class EntityManager {
public:
    static constexpr size_t kNullComponent = static_cast<size_t>(-1);
    static constexpr size_t kComponentCount = sizeof...(Components);

    [[nodiscard]] EntityID CreateEntity(const std::string& name) {
        std::array<size_t, kComponentCount> empty{};
        empty.fill(kNullComponent);

        EntityID id = 0;
        if (!mFreeIds.empty()) {
            id = mFreeIds.back();
            mFreeIds.pop_back();
            mEntities[id] = Entity(id, name);
            mComponentIndex[id] = empty;
            return id;
        }

        id = mEntities.size();
        mEntities.push_back(Entity(id, name));
        mComponentIndex.push_back(empty);
        return id;
    }

    void RemoveEntity(EntityID entity) {
        if (!IsValid(entity)) {
            return;
        }
        RemoveAllComponents(entity);
        mEntities[entity].mAlive = false;
        mFreeIds.push_back(entity);
    }

    [[nodiscard]] bool IsValid(EntityID entity) const {
        return entity < mEntities.size() && mEntities[entity].IsValid();
    }

    template <typename T>
    void AddComponent(EntityID entity, T component) {
        assert(IsValid(entity));
        auto& store = GetComponentStorage<T>();
        size_t& slot = mComponentIndex[entity][Index<T>];
        if (slot != kNullComponent) {
            store.mStorage[slot] = std::move(component);
            store.mValid[slot] = true;
            return;
        }
        if (!store.mFreeList.empty()) {
            slot = store.mFreeList.back();
            store.mFreeList.pop_back();
            store.mStorage[slot] = std::move(component);
            store.mValid[slot] = true;
            return;
        }
        slot = store.mStorage.size();
        store.mStorage.push_back(std::move(component));
        store.mValid.push_back(true);
    }

    template <typename T>
    void RemoveComponent(EntityID entity) {
        if (!IsValid(entity)) {
            return;
        }
        size_t& slot = mComponentIndex[entity][Index<T>];
        if (slot == kNullComponent) {
            return;
        }
        auto& store = GetComponentStorage<T>();
        store.mValid[slot] = false;
        store.mFreeList.push_back(slot);
        slot = kNullComponent;
    }

    template <typename T>
    bool HasComponent(EntityID entity) const {
        if (!IsValid(entity)) {
            return false;
        }
        return mComponentIndex[entity][Index<T>] != kNullComponent;
    }

    template <typename T>
    T& GetComponent(EntityID entity) {
        assert(HasComponent<T>(entity));
        const size_t slot = mComponentIndex[entity][Index<T>];
        return GetComponentStorage<T>().mStorage[slot];
    }

    template <typename T>
    const T& GetComponent(EntityID entity) const {
        assert(HasComponent<T>(entity));
        const size_t slot = mComponentIndex[entity][Index<T>];
        return GetComponentStorage<T>().mStorage[slot];
    }

    template <typename T>
    ComponentStorage<T>& GetComponentStorage() {
        return std::get<ComponentStorage<T>>(mPools);
    }

    template <typename T>
    const ComponentStorage<T>& GetComponentStorage() const {
        return std::get<ComponentStorage<T>>(mPools);
    }

    using tEntity = std::vector<Entity>;
    using const_iterator = typename tEntity::const_iterator;

    const_iterator cbegin() const { return mEntities.begin(); }
    const_iterator cend() const { return mEntities.end(); }
    const_iterator begin() const { return mEntities.begin(); }
    const_iterator end() const { return mEntities.end(); }
    const_iterator begin() { return mEntities.begin(); }
    const_iterator end() { return mEntities.end(); }

private:
    template <typename T>
    static constexpr size_t Index = ComponentIndex<T, Components...>::value;

    void RemoveAllComponents(EntityID entity) { (RemoveComponent<Components>(entity), ...); }

    tEntity mEntities;
    std::vector<size_t> mFreeIds;
    std::tuple<ComponentStorage<Components>...> mPools;
    std::vector<std::array<size_t, kComponentCount>> mComponentIndex;
};

}  // namespace ECSEngine
