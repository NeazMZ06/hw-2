#include "WindowManager.h"

#include <algorithm>
#include <cstdint>

namespace ECSEngine {

namespace {

float ScaleOrOne(float worldUnitsPerPixel) {
    return worldUnitsPerPixel == 0.f ? 1.f : worldUnitsPerPixel;
}

}  // namespace

WindowManager::WindowManager(unsigned int width, unsigned int height, const std::string& title)
    : mWindow(std::make_shared<sf::RenderWindow>(sf::VideoMode({width, height}), title)),
      mWorldCenter(static_cast<float>(width) * 0.5f, static_cast<float>(height) * 0.5f),
      mWorldUnitsPerPixel(1.f) {}

std::shared_ptr<sf::RenderWindow> WindowManager::GetWindow() const { return mWindow; }

Rect WindowManager::GetWindowBounds() const {
    const sf::Vector2u size = mWindow->getSize();
    return MakeRect(0.f, 0.f, static_cast<float>(size.x), static_cast<float>(size.y));
}

void WindowManager::SetCamera(const Point2D& worldPt, const Point2D& screenPt) {
    const sf::Vector2u size = mWindow->getSize();
    const Point2D screenCenter(static_cast<float>(size.x) * 0.5f, static_cast<float>(size.y) * 0.5f);
    const float scale = ScaleOrOne(mWorldUnitsPerPixel);
    mWorldCenter.x = worldPt.x - (screenPt.x - screenCenter.x) * scale;
    mWorldCenter.y = worldPt.y - (screenPt.y - screenCenter.y) * scale;
}

void WindowManager::SetWorldCenter(const Point2D& worldCenter) { mWorldCenter = worldCenter; }

void WindowManager::SetWorldRect(const Rect& worldCameraRect) {
    mWorldCenter.x = worldCameraRect.topLeft.x + worldCameraRect.width * 0.5f;
    mWorldCenter.y = worldCameraRect.topLeft.y + worldCameraRect.height * 0.5f;
    const sf::Vector2u size = mWindow->getSize();
    if (size.x > 0) {
        mWorldUnitsPerPixel = worldCameraRect.width / static_cast<float>(size.x);
    }
}

void WindowManager::SetWorldScale(float worldUnitsPerPixel) {
    mWorldUnitsPerPixel = std::max(worldUnitsPerPixel, 0.0001f);
}

Point2D WindowManager::GetWorldCenter() const { return mWorldCenter; }

float WindowManager::GetWorldScale() const { return mWorldUnitsPerPixel; }

float WindowManager::WindowToWorld(float x) const {
    const float screenCenterX = static_cast<float>(mWindow->getSize().x) * 0.5f;
    return mWorldCenter.x + (x - screenCenterX) * ScaleOrOne(mWorldUnitsPerPixel);
}

float WindowManager::WorldToWindow(float x) const {
    const float screenCenterX = static_cast<float>(mWindow->getSize().x) * 0.5f;
    return screenCenterX + (x - mWorldCenter.x) / ScaleOrOne(mWorldUnitsPerPixel);
}

Point2D WindowManager::WindowToWorld(const Point2D& pt) const {
    const sf::Vector2u size = mWindow->getSize();
    const float scale = ScaleOrOne(mWorldUnitsPerPixel);
    const float cx = static_cast<float>(size.x) * 0.5f;
    const float cy = static_cast<float>(size.y) * 0.5f;
    return Point2D(mWorldCenter.x + (pt.x - cx) * scale, mWorldCenter.y + (pt.y - cy) * scale);
}

Point2D WindowManager::WorldToWindow(const Point2D& pt) const {
    const sf::Vector2u size = mWindow->getSize();
    const float scale = ScaleOrOne(mWorldUnitsPerPixel);
    const float cx = static_cast<float>(size.x) * 0.5f;
    const float cy = static_cast<float>(size.y) * 0.5f;
    return Point2D(cx + (pt.x - mWorldCenter.x) / scale, cy + (pt.y - mWorldCenter.y) / scale);
}

Rect WindowManager::WindowToWorld(const Rect& rect) const {
    const Point2D origin = WindowToWorld(rect.topLeft);
    const float scale = ScaleOrOne(mWorldUnitsPerPixel);
    return MakeRect(origin.x, origin.y, rect.width * scale, rect.height * scale);
}

Rect WindowManager::WorldToWindow(const Rect& rect) const {
    const Point2D origin = WorldToWindow(rect.topLeft);
    const float scale = ScaleOrOne(mWorldUnitsPerPixel);
    return MakeRect(origin.x, origin.y, rect.width / scale, rect.height / scale);
}

void WindowManager::DebugDrawRect(const Rect& r, int red, int green, int blue, int alpha) {
    const Rect screen = WorldToWindow(r);
    sf::RectangleShape shape(sf::Vector2f(screen.width, screen.height));
    shape.setPosition({screen.topLeft.x, screen.topLeft.y});
    shape.setFillColor(sf::Color(static_cast<std::uint8_t>(red), static_cast<std::uint8_t>(green),
                                 static_cast<std::uint8_t>(blue), static_cast<std::uint8_t>(alpha)));
    mWindow->draw(shape);
}

void WindowManager::DebugFrameRect(const Rect& r, int red, int green, int blue, int alpha) {
    const Rect screen = WorldToWindow(r);
    sf::RectangleShape shape(sf::Vector2f(screen.width, screen.height));
    shape.setPosition({screen.topLeft.x, screen.topLeft.y});
    shape.setFillColor(sf::Color(0, 0, 0, 0));
    shape.setOutlineThickness(1.f);
    shape.setOutlineColor(sf::Color(static_cast<std::uint8_t>(red), static_cast<std::uint8_t>(green),
                                    static_cast<std::uint8_t>(blue), static_cast<std::uint8_t>(alpha)));
    mWindow->draw(shape);
}

}  // namespace ECSEngine
