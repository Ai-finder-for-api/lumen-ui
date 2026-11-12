#include "lumen/widget.hpp"
#include <algorithm>
#include <numeric>
namespace lumen {
void Widget::set_geometry(Rect r) {
    r.width=std::max(0,r.width); r.height=std::max(0,r.height);
    if(r.x==geometry_.x && r.y==geometry_.y && r.width==geometry_.width && r.height==geometry_.height) return;
    const bool resized=r.width!=geometry_.width || r.height!=geometry_.height;
    geometry_=r;
    if(resized) update_layout();
    geometry_changed.emit();
}
void Widget::set_layout(std::unique_ptr<Layout> layout) { layout_=std::move(layout); update_layout(); }
void Widget::update_layout() { if(layout_) layout_->apply(*this); }
void Widget::clear_focus_recursive() { focused_=false; for(auto& child:children_) child->clear_focus_recursive(); }
Widget* Widget::focused_descendant() noexcept {
    if(focused_) return this;
    for(auto& child:children_) if(auto* found=child->focused_descendant()) return found;
    return nullptr;
}
bool Widget::set_focus() {
    if(!focusable_ || !enabled_ || !visible_) return false;
    Widget* root=this; while(root->parent_) root=root->parent_;
    root->clear_focus_recursive(); focused_=true; return true;
}
bool Widget::dispatch_key(const KeyEvent& event) {
    Widget* root=this; while(root->parent_) root=root->parent_;
    Widget* target=root->focused_descendant();
    return target && target->enabled_ && target->visible_ && target->key_press(event);
}
void Widget::render(Canvas& canvas,Point parent_origin) {
    if(!visible_) return;
    const Point origin{parent_origin.x+geometry_.x,parent_origin.y+geometry_.y};
    canvas.push_clip({origin.x,origin.y,geometry_.width,geometry_.height});
    if(background_.a) canvas.fill_rect({origin.x,origin.y,geometry_.width,geometry_.height},background_);
    paint(canvas,origin);
    for(auto& child:children_) child->render(canvas,origin);
    canvas.pop_clip();
}
void Widget::paint(Canvas&,Point) {}
bool Widget::dispatch_mouse_press(Point p) {
    if(!visible_ || !enabled_ || p.x<0 || p.y<0 || p.x>=geometry_.width || p.y>=geometry_.height) return false;
    for(auto it=children_.rbegin();it!=children_.rend();++it) {
        auto& c=**it;
        if(c.geometry_.contains(p) && c.dispatch_mouse_press({p.x-c.geometry_.x,p.y-c.geometry_.y})) return true;
    }
    return mouse_press(p);
}
bool Widget::dispatch_mouse_release(Point p) {
    // Always deliver releases so controls can clear a pressed state even if hidden or disabled mid-gesture.
    for(auto it=children_.rbegin();it!=children_.rend();++it) {
        auto& c=**it;
        if(c.dispatch_mouse_release({p.x-c.geometry_.x,p.y-c.geometry_.y})) return true;
    }
    return mouse_release(p);
}
void Label::paint(Canvas& c,Point o) {
    int x=o.x+4;
    if(alignment_==Alignment::Center) x=o.x+(geometry().width-static_cast<int>(text_.size())*6)/2;
    else if(alignment_==Alignment::End) x=o.x+geometry().width-static_cast<int>(text_.size())*6-4;
    c.draw_text({x,o.y+(geometry().height-7)/2},text_,color_);
}
void Button::paint(Canvas& c,Point o) {
    const auto r=Rect{o.x,o.y,geometry().width,geometry().height};
    const Color fill=pressed_?Color{205,219,242,255}:Color{235,240,248,255};
    c.fill_rect(r,fill); c.stroke_rect(r,has_focus()?Color{47,111,224,255}:(is_enabled()?Color{93,112,142,255}:Color{170,174,182,255}),has_focus()?2:1);
    const int x=o.x+(geometry().width-static_cast<int>(text_.size())*6)/2;
    c.draw_text({x,o.y+(geometry().height-7)/2},text_,is_enabled()?Color{35,45,60,255}:Color{125,128,132,255});
}
bool Button::mouse_press(Point p) { (void)p; set_focus(); pressed_=true; return true; }
bool Button::mouse_release(Point p) {
    if(!pressed_) return false;
    pressed_=false;
    if(is_enabled() && is_visible() && p.x>=0 && p.y>=0 && p.x<geometry().width && p.y<geometry().height) clicked.emit();
    return true;
}
bool Button::key_press(const KeyEvent& e) { if(e.code==KeyCode::Enter || e.code==KeyCode::Space) { clicked.emit(); return true; } return false; }
void CheckBox::set_checked(bool v) { if(checked_==v) return; checked_=v; toggled.emit(checked_); }
void CheckBox::paint(Canvas& c,Point o) {
    c.fill_rect({o.x,o.y+3,16,16},white); c.stroke_rect({o.x,o.y+3,16,16},has_focus()?Color{47,111,224,255}:Color{90,102,122,255},has_focus()?2:1);
    if(checked_) { c.fill_rect({o.x+4,o.y+7,8,8},Color{47,111,224,255}); c.draw_text({o.x+4,o.y+7},"+",white); }
    c.draw_text({o.x+23,o.y+7},text_,Color{35,42,53,255});
}
bool CheckBox::mouse_press(Point p) { (void)p; set_focus(); set_checked(!checked_); return true; }
bool CheckBox::key_press(const KeyEvent& e) { if(e.code==KeyCode::Enter || e.code==KeyCode::Space) { set_checked(!checked_); return true; } return false; }
void Slider::set_range(int a,int b) { if(a>b) std::swap(a,b); minimum_=a; maximum_=b; set_value(value_); }
void Slider::set_value(int v) { v=std::clamp(v,minimum_,maximum_); if(v==value_) return; value_=v; value_changed.emit(value_); }
void Slider::set_from_x(int x) {
    const int span=std::max(1,geometry().width-12);
    const int pos=std::clamp(x-6,0,span);
    if(maximum_==minimum_) set_value(minimum_);
    else { const long long range=static_cast<long long>(maximum_)-minimum_;
        set_value(static_cast<int>(static_cast<long long>(minimum_)+(static_cast<long long>(pos)*range+span/2)/span)); }
}
void Slider::paint(Canvas& c,Point o) {
    const int cy=o.y+geometry().height/2, span=std::max(1,geometry().width-12);
    c.fill_rect({o.x+6,cy-2,span,4},Color{191,201,216,255});
    int pos=maximum_==minimum_?0:static_cast<int>(((static_cast<long long>(value_)-minimum_)*span)/(static_cast<long long>(maximum_)-minimum_));
    c.fill_rect({o.x+pos+2,cy-6,12,12},is_enabled()?Color{47,111,224,255}:Color{150,158,172,255});
}
bool Slider::mouse_press(Point p) { set_from_x(p.x); return true; }
LineEdit::LineEdit(std::string text) : text_(std::move(text)),cursor_(text_.size()) { set_focusable(true); }
void LineEdit::set_text(std::string text) {
    if(text_==text) return;
    text_=std::move(text); cursor_=std::min(cursor_,text_.size()); text_changed.emit(text_);
}
void LineEdit::set_cursor_from_x(int x) {
    const int column=std::max(0,(x-7)/6);
    cursor_=std::min(static_cast<std::size_t>(column),text_.size());
}
bool LineEdit::mouse_press(Point p) { set_cursor_from_x(p.x); return set_focus(); }
void LineEdit::paint(Canvas& c,Point o) {
    const Rect r{o.x,o.y,geometry().width,geometry().height};
    c.fill_rect(r,white); c.stroke_rect(r,has_focus()?Color{47,111,224,255}:Color{151,163,181,255},has_focus()?2:1);
    const std::string& shown=text_.empty()?placeholder_:text_;
    const Color color=text_.empty()?Color{145,151,162,255}:Color{35,42,53,255};
    const std::size_t visible_chars=static_cast<std::size_t>(std::max(0,(geometry().width-12)/6));
    std::size_t start=0;
    if(text_.size()>visible_chars && cursor_>visible_chars) start=cursor_-visible_chars;
    const auto begin=std::min(start,shown.size()); const auto count=std::min(visible_chars,shown.size()-begin);
    c.draw_text({o.x+6,o.y+(geometry().height-7)/2},std::string_view(shown).substr(begin,count),color);
    if(has_focus()) { const int cx=o.x+6+static_cast<int>(cursor_-start)*6; c.fill_rect({cx,o.y+7,1,std::max(1,geometry().height-14)},Color{47,111,224,255}); }
}
bool LineEdit::key_press(const KeyEvent& e) {
    if(e.code==KeyCode::Enter) { editing_finished.emit(); return true; }
    if(e.code==KeyCode::Left) { if(cursor_) --cursor_; return true; }
    if(e.code==KeyCode::Right) { if(cursor_<text_.size()) ++cursor_; return true; }
    if(e.code==KeyCode::Home) { cursor_=0; return true; }
    if(e.code==KeyCode::End) { cursor_=text_.size(); return true; }
    std::string updated=text_;
    if(e.code==KeyCode::Backspace && cursor_>0) { updated.erase(cursor_-1,1); --cursor_; }
    else if(e.code==KeyCode::Delete && cursor_<text_.size()) updated.erase(cursor_,1);
    else if(e.code==KeyCode::Character && e.text>=32 && e.text<=126 && !e.control) { updated.insert(updated.begin()+static_cast<std::ptrdiff_t>(cursor_),static_cast<char>(e.text)); ++cursor_; }
    else if(e.code==KeyCode::Space) { updated.insert(updated.begin()+static_cast<std::ptrdiff_t>(cursor_),' '); ++cursor_; }
    else return false;
    set_text(std::move(updated)); return true;
}
void ProgressBar::set_range(int a,int b) { if(a>b) std::swap(a,b); minimum_=a;maximum_=b;set_value(value_); }
void ProgressBar::set_value(int v) noexcept { value_=std::clamp(v,minimum_,maximum_); }
void ProgressBar::paint(Canvas& c,Point o) {
    const int w=geometry().width,h=geometry().height;
    c.fill_rect({o.x,o.y,w,h},Color{232,236,243,255}); c.stroke_rect({o.x,o.y,w,h},Color{160,171,188,255});
    if(maximum_>minimum_ && w>2) { const int f=static_cast<int>(((static_cast<long long>(value_)-minimum_)*(w-2))/(static_cast<long long>(maximum_)-minimum_)); c.fill_rect({o.x+1,o.y+1,f,std::max(0,h-2)},Color{47,111,224,255}); }
}
void BoxLayout::add(Widget& w,int stretch) { items_.push_back({&w,std::max(0,stretch)}); }
void BoxLayout::apply(Widget& owner) {
    if(items_.empty()) return;
    const bool horizontal=direction_==Direction::Horizontal;
    const int extent=horizontal ? owner.geometry_.width-margins_.left-margins_.right
                                : owner.geometry_.height-margins_.top-margins_.bottom;
    const int available=std::max(0,extent-spacing_*static_cast<int>(items_.size()-1));
    int fixed=0,total_weight=0;
    for(const auto& item:items_) {
        if(item.stretch) total_weight+=item.stretch;
        else { const auto hint=item.widget->size_hint(); fixed+=horizontal?hint.width:hint.height; }
    }
    const int flex=std::max(0,available-fixed);
    int cursor=horizontal?margins_.left:margins_.top;
    int weight_used=0, flex_used=0;
    for(const auto& item:items_) {
        int main;
        if(item.stretch && total_weight) {
            const int next_weight=weight_used+item.stretch;
            const int next_flex=static_cast<int>((static_cast<long long>(flex)*next_weight)/total_weight);
            main=next_flex-flex_used; weight_used=next_weight; flex_used=next_flex;
        } else main=horizontal?item.widget->size_hint().width:item.widget->size_hint().height;
        if(horizontal) item.widget->set_geometry({cursor,margins_.top,std::max(0,main),std::max(0,owner.geometry_.height-margins_.top-margins_.bottom)});
        else item.widget->set_geometry({margins_.left,cursor,std::max(0,owner.geometry_.width-margins_.left-margins_.right),std::max(0,main)});
        cursor+=main+spacing_;
    }
}
}
