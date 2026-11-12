#pragma once
#include "lumen/canvas.hpp"
#include "lumen/geometry.hpp"
#include "lumen/event.hpp"
#include "lumen/layout.hpp"
#include "lumen/signal.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
namespace lumen {
class Widget {
public:
    Widget() = default;
    virtual ~Widget() = default;
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    template<class T, class... Args> T& emplace(Args&&... args) {
        static_assert(std::is_base_of_v<Widget,T>);
        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        child->parent_ = this;
        T& ref = *child;
        children_.push_back(std::move(child));
        if (layout_) layout_->apply(*this);
        return ref;
    }
    [[nodiscard]] Widget* parent() const noexcept { return parent_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Widget>>& children() const noexcept { return children_; }
    void set_geometry(Rect geometry);
    [[nodiscard]] Rect geometry() const noexcept { return geometry_; }
    void set_size(Size size) { set_geometry({geometry_.x,geometry_.y,size.width,size.height}); }
    [[nodiscard]] virtual Size size_hint() const noexcept { return {80,28}; }
    void set_visible(bool value) noexcept { visible_ = value; }
    void show() noexcept { set_visible(true); }
    void hide() noexcept { set_visible(false); }
    [[nodiscard]] bool is_visible() const noexcept { return visible_; }
    void set_enabled(bool value) noexcept { enabled_ = value; }
    [[nodiscard]] bool is_enabled() const noexcept { return enabled_; }
    void set_focusable(bool value) noexcept { focusable_=value; if(!value) focused_=false; }
    [[nodiscard]] bool is_focusable() const noexcept { return focusable_; }
    [[nodiscard]] bool has_focus() const noexcept { return focused_; }
    bool set_focus();
    bool dispatch_key(const KeyEvent& event);
    void set_background(Color color) noexcept { background_ = color; }
    [[nodiscard]] Color background() const noexcept { return background_; }
    void set_layout(std::unique_ptr<Layout> layout);
    void update_layout();
    void render(Canvas& canvas, Point offset = {});
    bool dispatch_mouse_press(Point position);
    bool dispatch_mouse_release(Point position);
    Signal<> geometry_changed;
protected:
    virtual void paint(Canvas& canvas, Point origin);
    virtual bool mouse_press(Point local) { (void)local; return false; }
    virtual bool mouse_release(Point local) { (void)local; return false; }
    virtual bool key_press(const KeyEvent& event) { (void)event; return false; }
private:
    friend class BoxLayout;
    Widget* parent_{nullptr};
    std::vector<std::unique_ptr<Widget>> children_;
    std::unique_ptr<Layout> layout_;
    Rect geometry_{};
    Color background_{transparent};
    bool visible_{true}, enabled_{true}, focusable_{false}, focused_{false};
    void clear_focus_recursive();
    Widget* focused_descendant() noexcept;
};
class Label : public Widget {
public:
    explicit Label(std::string text = {}) : text_(std::move(text)) {}
    void set_text(std::string text) { text_=std::move(text); }
    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    void set_color(Color color) noexcept { color_=color; }
    void set_alignment(Alignment value) noexcept { alignment_=value; }
    [[nodiscard]] Size size_hint() const noexcept override { return {static_cast<int>(text_.size())*6+4, 18}; }
protected:
    void paint(Canvas& canvas, Point origin) override;
private:
    std::string text_;
    Color color_{30,35,45,255};
    Alignment alignment_{Alignment::Start};
};
class Button : public Widget {
public:
    explicit Button(std::string text = {}) : text_(std::move(text)) { set_focusable(true); }
    Signal<> clicked;
    void set_text(std::string text) { text_=std::move(text); }
    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    [[nodiscard]] Size size_hint() const noexcept override { return {static_cast<int>(text_.size())*8+28, 34}; }
protected:
    void paint(Canvas& canvas, Point origin) override;
    bool mouse_press(Point local) override;
    bool mouse_release(Point local) override;
    bool key_press(const KeyEvent& event) override;
private:
    std::string text_;
    bool pressed_{false};
};
class CheckBox : public Widget {
public:
    explicit CheckBox(std::string text = {}) : text_(std::move(text)) { set_focusable(true); }
    Signal<bool> toggled;
    void set_checked(bool value);
    [[nodiscard]] bool is_checked() const noexcept { return checked_; }
    [[nodiscard]] Size size_hint() const noexcept override { return {static_cast<int>(text_.size())*7+28,24}; }
protected:
    void paint(Canvas& canvas, Point origin) override;
    bool mouse_press(Point local) override;
    bool key_press(const KeyEvent& event) override;
private:
    std::string text_;
    bool checked_{false};
};
class Slider : public Widget {
public:
    Slider() = default;
    Signal<int> value_changed;
    void set_range(int minimum, int maximum);
    void set_value(int value);
    [[nodiscard]] int value() const noexcept { return value_; }
    [[nodiscard]] Size size_hint() const noexcept override { return {140,24}; }
protected:
    void paint(Canvas& canvas, Point origin) override;
    bool mouse_press(Point local) override;
private:
    int minimum_{0}, maximum_{100}, value_{0};
    void set_from_x(int x);
};
class LineEdit : public Widget {
public:
    explicit LineEdit(std::string text = {});
    Signal<const std::string&> text_changed;
    Signal<> editing_finished;
    void set_text(std::string text);
    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    void set_placeholder(std::string text) { placeholder_=std::move(text); }
    [[nodiscard]] std::size_t cursor_position() const noexcept { return cursor_; }
    [[nodiscard]] Size size_hint() const noexcept override { return {180,32}; }
protected:
    void paint(Canvas& canvas,Point origin) override;
    bool mouse_press(Point local) override;
    bool key_press(const KeyEvent& event) override;
private:
    std::string text_, placeholder_;
    std::size_t cursor_{0};
    void set_cursor_from_x(int x);
};
class ProgressBar : public Widget {
public:
    ProgressBar() = default;
    void set_range(int minimum, int maximum);
    void set_value(int value) noexcept;
    [[nodiscard]] int value() const noexcept { return value_; }
    [[nodiscard]] Size size_hint() const noexcept override { return {160,18}; }
protected:
    void paint(Canvas& canvas, Point origin) override;
private:
    int minimum_{0}, maximum_{100}, value_{0};
};
}
