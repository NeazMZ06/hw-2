#pragma once

#include <vector>

#include "CameraComponent.h"
#include "CameraFollowerComponent.h"
#include "CameraShakeComponent.h"
#include "CollisionComponent.h"
#include "EntityManager.h"
#include "GravityComponent.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "MovementComponent.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "SoundManager.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include "WindowManager.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ECSEngine {

template <typename... Components>
class EngineView {
public:
    virtual ~EngineView() = default;
    virtual SoundManager& GetSoundManager() = 0;
    virtual SpriteManager& GetSpriteManager() = 0;
    virtual EntityManager<Components...>& GetEntityManager() = 0;
    virtual WindowManager& GetWindowManager() = 0;
    virtual bool DebugEnabled() const = 0;
};

template <typename... Components>
class ECSEngine : public EngineView<Components...> {
public:
    ECSEngine(unsigned int width, unsigned int height, const std::string& name);
    ~ECSEngine() override;

    void Run();

    SoundManager& GetSoundManager() override { return mSoundManager; }

    SpriteManager& GetSpriteManager() override { return mSpriteManager; }

    EntityManager<Components...>& GetEntityManager() override { return mEntityManager; }

    WindowManager& GetWindowManager() override { return mWindowManager; }

    SystemManager<Components...>& GetSystemManager() { return mSystemManager; }

    bool DebugEnabled() const override { return debug != 0; }

private:
    bool ProcessEvents();
    void DebugDraw();
    void SetKey(unsigned int scancode, bool down);
    void ClearKeys();
    bool KeyDown(unsigned int scancode) const;

    EntityManager<Components...> mEntityManager;
    SystemManager<Components...> mSystemManager;
    SpriteManager mSpriteManager;
    SoundManager mSoundManager;
    WindowManager mWindowManager;

    // For debugging - pause and step the engine
    bool running;
    bool step;
    bool paused;
    int debug;
};

template <typename... Components>
ECSEngine<Components...>::ECSEngine(unsigned int width, unsigned int height, const std::string& name)
    : mWindowManager(width, height, name), running(true), step(false), paused(false), debug(0) {}

template <typename... Components>
ECSEngine<Components...>::~ECSEngine() = default;

template <typename... Components>
void ECSEngine<Components...>::Run() {
    running = true;
    auto window = mWindowManager.GetWindow();
    window->setVerticalSyncEnabled(true);
    while (running && window->isOpen()) {
        if (!ProcessEvents()) {
            break;
        }
        if (!paused || step) {
            if (!mSystemManager.Run(this)) {
                break;
            }
            step = false;
            if (debug != 0) {
                DebugDraw();
            }
            window->display();
        }
    }
    window->close();
}

template <typename... Components>
bool ECSEngine<Components...>::ProcessEvents() {
    auto window = mWindowManager.GetWindow();
    while (const std::optional event = window->pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            return false;
        }
        if (event->is<sf::Event::FocusLost>()) {
            ClearKeys();
        }
        if (const auto* pressed = event->getIf<sf::Event::KeyPressed>()) {
            const auto code = static_cast<unsigned int>(pressed->scancode);
            if (pressed->scancode == sf::Keyboard::Scancode::Escape) {
                return false;
            }
            const bool firstPress = !KeyDown(code);
            if (firstPress && pressed->scancode == sf::Keyboard::Scancode::F1) {
                debug = debug == 0 ? 1 : 0;
            } else if (firstPress && pressed->scancode == sf::Keyboard::Scancode::F2) {
                paused = !paused;
            } else if (firstPress && pressed->scancode == sf::Keyboard::Scancode::F3) {
                step = true;
            }
            SetKey(code, true);
        }
        if (const auto* released = event->getIf<sf::Event::KeyReleased>()) {
            SetKey(static_cast<unsigned int>(released->scancode), false);
        }
    }
    return true;
}

template <typename... Components>
void ECSEngine<Components...>::SetKey(unsigned int scancode, bool down) {
    if (scancode >= sf::Keyboard::ScancodeCount) {
        return;
    }
    for (auto it = mEntityManager.begin(); it != mEntityManager.end(); ++it) {
        if (!it->IsValid()) {
            continue;
        }
        const EntityID id = it->GetID();
        if (!mEntityManager.template HasComponent<InputComponent>(id)) {
            continue;
        }
        auto& input = mEntityManager.template GetComponent<InputComponent>(id);
        input.keydown.set(scancode, down);
    }
}

template <typename... Components>
bool ECSEngine<Components...>::KeyDown(unsigned int scancode) const {
    if (scancode >= sf::Keyboard::ScancodeCount) {
        return false;
    }
    for (auto it = mEntityManager.begin(); it != mEntityManager.end(); ++it) {
        if (!it->IsValid() || !mEntityManager.template HasComponent<InputComponent>(it->GetID())) {
            continue;
        }
        if (mEntityManager.template GetComponent<InputComponent>(it->GetID()).keydown.test(scancode)) {
            return true;
        }
    }
    return false;
}

template <typename... Components>
void ECSEngine<Components...>::ClearKeys() {
    for (auto it = mEntityManager.begin(); it != mEntityManager.end(); ++it) {
        if (!it->IsValid() || !mEntityManager.template HasComponent<InputComponent>(it->GetID())) {
            continue;
        }
        mEntityManager.template GetComponent<InputComponent>(it->GetID()).keydown.reset();
    }
}

template <typename... Components>
void ECSEngine<Components...>::DebugDraw() {
    for (auto it = mEntityManager.begin(); it != mEntityManager.end(); ++it) {
        if (!it->IsValid()) {
            continue;
        }
        const EntityID id = it->GetID();
        if (!mEntityManager.template HasComponent<LocationComponent>(id) ||
            !mEntityManager.template HasComponent<CollisionComponent>(id)) {
            continue;
        }
        const auto& loc = mEntityManager.template GetComponent<LocationComponent>(id);
        const auto& collision = mEntityManager.template GetComponent<CollisionComponent>(id);
        const Rect world = WorldRect(loc.loc, collision.collisionBox);
        if (collision.doesDamage != 0) {
            mWindowManager.DebugFrameRect(world, 255, 40, 40);
        } else if (collision.staticEntity != 0) {
            mWindowManager.DebugFrameRect(world, 40, 200, 80);
        } else {
            mWindowManager.DebugFrameRect(world, 240, 200, 40);
        }
    }
}

}  // namespace ECSEngine
