#pragma once
#include "lumen/geometry.hpp"
#include <cstdint>
#include <string_view>
#include <vector>
namespace lumen {
// Deterministic, dependency-free RGBA software canvas. Coordinates are clipped to its bounds.
class Canvas {
public:
    explicit Canvas(Size size, Color clear = transparent);
    [[nodiscard]] Size size() const noexcept { return size_; }
    [[nodiscard]] const std::vector<std::uint8_t>& pixels() const noexcept { return pixels_; }
    [[nodiscard]] Color pixel(Point p) const noexcept;
    void clear(Color color);
    void fill_rect(Rect rect, Color color);
    void stroke_rect(Rect rect, Color color, int thickness = 1);
    void draw_text(Point origin, std::string_view text, Color color, int scale = 1);
    void push_clip(Rect rect);
    void pop_clip() noexcept;
    [[nodiscard]] Rect clip() const noexcept { return clips_.back(); }
private:
    void put_pixel(int x, int y, Color color);
    Size size_;
    std::vector<std::uint8_t> pixels_;
    std::vector<Rect> clips_;
};
}
