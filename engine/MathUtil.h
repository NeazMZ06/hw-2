#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <iostream>

namespace ECSEngine {

#include <cstdio>

struct Point2D {
    float x, y;
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
    Point2D(int ix, int iy) : x(static_cast<float>(ix)), y(static_cast<float>(iy)) {}
};

[[maybe_unused]] static std::ostream& operator<<(std::ostream& os, const Point2D& p) {
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}

[[maybe_unused]] static Point2D operator*(float number, const Point2D& rhs) {
    return Point2D(number * rhs.x, number * rhs.y);
}

struct Line {
    Point2D p1, p2;

    Line(Point2D p1 = {0, 0}, Point2D p2 = {0, 0}) : p1(p1), p2(p2) {}
    Line(float x1, float y1, float x2, float y2) : p1(x1, y1), p2(x2, y2) {}
};

[[maybe_unused]] static std::ostream& operator<<(std::ostream& os, const Line& l) {
    os << "L{" << l.p1 << ", " << l.p2 << "}";
    return os;
}

struct Circle {
    Point2D center;
    float radius;

    Circle(Point2D c = {0, 0}, float r = 0) : center(c), radius(r) {}
    Circle(float x, float y, float r) : center(x, y), radius(r) {}
};

struct Rect {
    Point2D topLeft;
    float width, height;
};

[[maybe_unused]] static std::ostream& operator<<(std::ostream& os, const Rect& r) {
    os << "R{" << r.topLeft << ", " << r.width << "x" << r.height << "}";
    return os;
}

inline Rect MakeRect(float x, float y, float w, float h) {
    Rect r;
    r.topLeft = Point2D(x, y);
    r.width = w;
    r.height = h;
    return r;
}

// relative is stored as an offset from an entity's LocationComponent.
inline Rect WorldRect(const Point2D& origin, const Rect& relative) {
    return MakeRect(origin.x + relative.topLeft.x, origin.y + relative.topLeft.y, relative.width,
                    relative.height);
}

inline bool RectsOverlap(const Rect& a, const Rect& b) {
    return a.topLeft.x < b.topLeft.x + b.width && a.topLeft.x + a.width > b.topLeft.x &&
           a.topLeft.y < b.topLeft.y + b.height && a.topLeft.y + a.height > b.topLeft.y;
}

}  // namespace ECSEngine

#endif  // MATHUTIL_H
