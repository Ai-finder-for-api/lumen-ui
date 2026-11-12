#pragma once
#include "lumen/geometry.hpp"
#include <vector>
namespace lumen {
class Widget;
class Layout {
public:
    virtual ~Layout() = default;
    virtual void apply(Widget& owner) = 0;
};
class BoxLayout : public Layout {
public:
    enum class Direction { Horizontal, Vertical };
    explicit BoxLayout(Direction direction, int spacing = 0, Insets margins = {}) : direction_(direction), spacing_(spacing), margins_(margins) {}
    void add(Widget& widget, int stretch = 0);
    void apply(Widget& owner) override;
    [[nodiscard]] Direction direction() const noexcept { return direction_; }
private:
    struct Item { Widget* widget; int stretch; };
    Direction direction_;
    int spacing_;
    Insets margins_;
    std::vector<Item> items_;
};
class VBoxLayout : public BoxLayout {
public:
    explicit VBoxLayout(int spacing = 0, Insets margins = {}) : BoxLayout(Direction::Vertical,spacing,margins) {}
};
class HBoxLayout : public BoxLayout {
public:
    explicit HBoxLayout(int spacing = 0, Insets margins = {}) : BoxLayout(Direction::Horizontal,spacing,margins) {}
};
}
