#pragma once

#include <string>

#include "ECSEngine.h"
#include "LocationComponent.h"
#include "SpriteComponent.h"

namespace ECSEngine {

template <typename... Components>
class SpriteSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("SpriteSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entities = engine->GetEntityManager();
        auto& sprites = engine->GetSpriteManager();
        auto& window = engine->GetWindowManager();
        auto renderWindow = window.GetWindow();
        renderWindow->clear(sf::Color(70, 150, 210));

        const float worldScale = window.GetWorldScale() == 0.f ? 1.f : window.GetWorldScale();
        for (auto it = entities.begin(); it != entities.end(); ++it) {
            if (!it->IsValid()) {
                continue;
            }
            const EntityID id = it->GetID();
            if (!entities.template HasComponent<SpriteComponent>(id) ||
                !entities.template HasComponent<LocationComponent>(id)) {
                continue;
            }
            const auto& sprite = entities.template GetComponent<SpriteComponent>(id);
            const auto& loc = entities.template GetComponent<LocationComponent>(id);
            sf::Sprite& drawable = sprites.GetSprite(sprite.textureID);
            const sf::IntRect tex = drawable.getTextureRect();
            const float texW = tex.size.x > 0 ? static_cast<float>(tex.size.x) : 1.f;
            const float texH = tex.size.y > 0 ? static_cast<float>(tex.size.y) : 1.f;
            const float drawX = loc.loc.x + sprite.bounds.topLeft.x;
            const float drawY = loc.loc.y + sprite.bounds.topLeft.y;
            if (sprite.worldSpace) {
                const Point2D screen = window.WorldToWindow(Point2D(drawX, drawY));
                drawable.setPosition({screen.x, screen.y});
                drawable.setScale({(sprite.bounds.width / texW) / worldScale,
                                   (sprite.bounds.height / texH) / worldScale});
            } else {
                drawable.setPosition({drawX, drawY});
                drawable.setScale({sprite.bounds.width / texW, sprite.bounds.height / texH});
            }
            renderWindow->draw(drawable);
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
