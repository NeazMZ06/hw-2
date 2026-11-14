#include "SoundManager.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace ECSEngine {

void SoundManager::RegisterSound(const std::string& soundPath, const std::string& soundName) {
    try {
        auto buffer = std::make_unique<sf::SoundBuffer>(std::filesystem::path(soundPath));
        auto sound = std::make_unique<sf::Sound>(*buffer);
        mBuffers[soundName] = std::move(buffer);
        mSounds[soundName] = std::move(sound);
    } catch (const std::exception& ex) {
        std::cerr << "Failed to load sound '" << soundPath << "': " << ex.what() << '\n';
        throw;
    }
}

sf::Sound& SoundManager::GetSound(const std::string& soundName) {
    const auto it = mSounds.find(soundName);
    if (it == mSounds.end()) {
        throw std::runtime_error("Sound not registered: " + soundName);
    }
    return *it->second;
}

}  // namespace ECSEngine
