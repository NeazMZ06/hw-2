#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "MathUtil.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

using SpriteID = size_t;

// Textures live behind unique_ptr so a map rehash cannot move them.
// Sprites are stored in a vector; SpriteID is the index.
// GetSprite's reference is invalidated by a later RegisterTexture that grows
// the vector. Register every sprite before the game loop, then the references
// stay valid for the rest of the run.
class SpriteManager {
public:
    SpriteManager() = default;
    ~SpriteManager() = default;

    // Loads texturePath once and appends a sprite using sourceRect on that texture.
    // sourceRect is x, y, width, height in pixels on the sheet.
    [[nodiscard]] SpriteID RegisterTexture(const std::string& texturePath, const Rect& sourceRect);

    // Reference into the sprite vector. Invalid after a reallocating RegisterTexture.
    sf::Sprite& GetSprite(SpriteID id);

private:
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> mTextures;
    std::vector<sf::Sprite> mSprites;
};

}  // namespace ECSEngine
