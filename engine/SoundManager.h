#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

namespace ECSEngine {

// Sounds are stored behind unique_ptr so an unordered_map rehash cannot move
// the SoundBuffer object. sf::Sound holds a pointer to that buffer.
// GetSound's reference stays valid until that name is registered again or the
// SoundManager is destroyed.
class SoundManager {
public:
    SoundManager() = default;
    ~SoundManager() = default;

    // Loads the file and keeps it under soundName. A second register with the
    // same name replaces the previous sound.
    void RegisterSound(const std::string& soundPath, const std::string& soundName);

    // Reference into the manager. Valid until this name is replaced or the
    // manager is destroyed.
    sf::Sound& GetSound(const std::string& soundName);

private:
    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> mBuffers;
    std::unordered_map<std::string, std::unique_ptr<sf::Sound>> mSounds;
};

}  // namespace ECSEngine
