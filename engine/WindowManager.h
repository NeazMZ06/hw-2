#pragma once

#include <memory>
#include <string>

#include "MathUtil.h"
#include <SFML/Graphics.hpp>

namespace ECSEngine {

// Converts between world pixels and window pixels.
// mWorldCenter is the world point drawn at the center of the window.
// mWorldUnitsPerPixel is how many world pixels correspond to one screen pixel.
// Both spaces grow to the right and down.
// GetWindow() keeps the window alive for as long as any shared_ptr copy remains,
// including the one owned by this manager.
class WindowManager {
public:
    WindowManager(unsigned int width, unsigned int height, const std::string& title);
    ~WindowManager() = default;
    std::shared_ptr<sf::RenderWindow> GetWindow() const;
    Rect GetWindowBounds() const;

    // Place worldPt at screenPt.
    void SetCamera(const Point2D& worldPt, const Point2D& screenPt);
    void SetWorldCenter(const Point2D& worldCenter);
    // Fit the given world rectangle across the window and center on it.
    void SetWorldRect(const Rect& worldCameraRect);
    void SetWorldScale(float worldUnitsPerPixel);
    Point2D GetWorldCenter() const;
    float GetWorldScale() const;

    float WindowToWorld(float x) const;
    float WorldToWindow(float x) const;
    Rect WindowToWorld(const Rect& rect) const;
    Rect WorldToWindow(const Rect& rect) const;
    Point2D WorldToWindow(const Point2D& pt) const;
    Point2D WindowToWorld(const Point2D& pt) const;

    void DebugDrawRect(const Rect& r, int red, int green, int blue, int alpha = 255);
    void DebugFrameRect(const Rect& r, int red, int green, int blue, int alpha = 255);

private:
    std::shared_ptr<sf::RenderWindow> mWindow;
    Point2D mWorldCenter;          // Center of the world view, in world pixels
    float mWorldUnitsPerPixel = 1.f;  // How many world units per screen pixel
};

}  // namespace ECSEngine
