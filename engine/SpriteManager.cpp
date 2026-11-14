#include "SpriteManager.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace ECSEngine {

SpriteID SpriteManager::RegisterTexture(const std::string& texturePath, const Rect& sourceRect) {
    auto found = mTextures.find(texturePath);
    if (found == mTextures.end()) {
        try {
            auto texture = std::make_unique<sf::Texture>(std::filesystem::path(texturePath));
            texture->setSmooth(false);
            found = mTextures.emplace(texturePath, std::move(texture)).first;
        } catch (const std::exception& ex) {
            std::cerr << "Failed to load texture '" << texturePath << "': " << ex.what() << '\n';
            throw;
        }
    }

    sf::Sprite sprite(*found->second);
    const auto x = static_cast<int>(sourceRect.topLeft.x);
    const auto y = static_cast<int>(sourceRect.topLeft.y);
    const auto w = static_cast<int>(sourceRect.width);
    const auto h = static_cast<int>(sourceRect.height);
    sprite.setTextureRect(sf::IntRect({x, y}, {w, h}));
    mSprites.push_back(std::move(sprite));
    return mSprites.size() - 1;
}

sf::Sprite& SpriteManager::GetSprite(SpriteID id) {
    assert(id < mSprites.size());
    return mSprites[id];
}

}  // namespace ECSEngine
