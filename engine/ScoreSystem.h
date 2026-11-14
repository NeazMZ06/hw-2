#pragma once

#include <string>

#include "ECSEngine.h"
#include "ScoreComponent.h"
#include "SpriteComponent.h"

namespace ECSEngine {

template <typename... Components>
class ScoreSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("ScoreSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entities = engine->GetEntityManager();
        for (auto it = entities.begin(); it != entities.end(); ++it) {
            if (!it->IsValid()) {
                continue;
            }
            const EntityID id = it->GetID();
            if (!entities.template HasComponent<ScoreComponent>(id)) {
                continue;
            }
            const auto& score = entities.template GetComponent<ScoreComponent>(id);
            int value = score.score;
            if (value < 0) {
                value = 0;
            }
            const int digits[3] = {(value / 100) % 10, (value / 10) % 10, value % 10};
            for (int place = 0; place < 3; ++place) {
                const EntityID digit = score.digitID[place];
                if (!entities.IsValid(digit) || !entities.template HasComponent<SpriteComponent>(digit)) {
                    continue;
                }
                entities.template GetComponent<SpriteComponent>(digit).textureID =
                    score.digitTextureID[digits[place]];
            }
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
