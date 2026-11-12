#pragma once
#include <algorithm>
#include <cstdint>

namespace lumen {
struct Point { int x{0}, y{0}; friend constexpr bool operator==(Point, Point) = default; };
struct Size { int width{0}, height{0}; friend constexpr bool operator==(Size, Size) = default; };
struct Insets { int left{0}, top{0}, right{0}, bottom{0}; };
struct Rect {
    int x{0}, y{0}, width{0}, height{0};
    [[nodiscard]] constexpr bool contains(Point p) const noexcept {
        return p.x >= x && p.y >= y && p.x < x + width && p.y < y + height;
    }
    [[nodiscard]] constexpr Size size() const noexcept { return {width, height}; }
    [[nodiscard]] constexpr bool empty() const noexcept { return width <= 0 || height <= 0; }
};
struct Color {
    std::uint8_t r{0}, g{0}, b{0}, a{255};
    friend constexpr bool operator==(Color, Color) = default;
};
enum class Alignment { Start, Center, End };
inline constexpr Color transparent{0,0,0,0};
inline constexpr Color white{255,255,255,255};
inline constexpr Color black{0,0,0,255};
}
