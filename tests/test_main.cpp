#include <lumen/lumen.hpp>
#include <atomic>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>
namespace {
int failures=0;
void check(bool ok,const char* expr,int line) { if(!ok) { std::cerr<<"FAIL line "<<line<<": "<<expr<<'\n'; ++failures; } }
#define CHECK(...) check(static_cast<bool>((__VA_ARGS__)),#__VA_ARGS__,__LINE__)
void signals() {
    lumen::Signal<int> signal; int sum=0;
    auto connection=signal.connect([&](int n){sum+=n;});
    signal.emit(3); CHECK(sum==3); CHECK(connection.connected());
    connection.disconnect(); CHECK(!connection.connected()); signal.emit(7); CHECK(sum==3);
    int calls=0; signal.connect([&](int){++calls;}); signal.connect([&](int){++calls;});
    signal.emit(0); CHECK(calls==2);
}
void application_queue() {
    lumen::Application app; CHECK(lumen::Application::instance()==&app);
    int value=0; app.post([&]{value=1;}); app.post([&]{value+=2;});
    CHECK(app.process_events()==2); CHECK(value==3); CHECK(app.process_events()==0);
}
void widgets_and_layout() {
    lumen::Widget root; root.set_geometry({0,0,240,100});
    auto layout=std::make_unique<lumen::BoxLayout>(lumen::BoxLayout::Direction::Vertical,4,lumen::Insets{8,6,8,6});
    auto* raw=layout.get(); root.set_layout(std::move(layout));
    auto& title=root.emplace<lumen::Label>("Lumen");
    auto& button=root.emplace<lumen::Button>("Run");
    auto& slider=root.emplace<lumen::Slider>();
    raw->add(title); raw->add(button); raw->add(slider,1); root.update_layout();
    CHECK(title.geometry().x==8); CHECK(title.geometry().width==224); CHECK(title.geometry().y==6);
    CHECK(button.geometry().y==28); CHECK(slider.geometry().height==28);
    int clicks=0; button.clicked.connect([&]{++clicks;});
    const auto b=button.geometry();
    CHECK(root.dispatch_mouse_press({b.x+4,b.y+4}));
    CHECK(root.dispatch_mouse_release({b.x+4,b.y+4})); CHECK(clicks==1);
    root.dispatch_mouse_press({b.x+4,b.y+4}); root.dispatch_mouse_release({0,0}); CHECK(clicks==1);
    auto& checkbox=root.emplace<lumen::CheckBox>("Enabled"); checkbox.set_geometry({4,70,120,24});
    bool toggled=false; checkbox.toggled.connect([&](bool v){toggled=v;});
    CHECK(root.dispatch_mouse_press({10,75})); CHECK(checkbox.is_checked()); CHECK(toggled);
    slider.set_range(-10,10); slider.set_value(7); CHECK(slider.value()==7);
    slider.set_value(500); CHECK(slider.value()==10);
    int changes=0; slider.value_changed.connect([&](int){++changes;}); slider.set_value(9); CHECK(changes==1);
    lumen::ProgressBar progress; progress.set_range(0,10); progress.set_value(20); CHECK(progress.value()==10);
}
void canvas() {
    lumen::Canvas canvas({20,12},{255,255,255,255});
    canvas.fill_rect({-2,-2,6,6},{255,0,0,255});
    CHECK(canvas.pixel({0,0})==lumen::Color{255,0,0,255});
    CHECK(canvas.pixel({5,5})==lumen::Color{255,255,255,255});
    canvas.fill_rect({2,2,1,1},{0,0,255,128});
    auto blend=canvas.pixel({2,2}); CHECK(blend.b==128 && blend.r==127 && blend.a==255);
    canvas.stroke_rect({8,2,8,8},{0,0,0,255},2);
    CHECK(canvas.pixel({8,5})==lumen::Color{0,0,0,255}); CHECK(canvas.pixel({11,5})==lumen::Color{255,255,255,255});
    lumen::Widget root; root.set_geometry({0,0,100,40}); root.set_background({240,240,240,255});
    auto& button=root.emplace<lumen::Button>("OK"); button.set_geometry({5,5,60,28});
    lumen::Canvas frame({100,40}); root.render(frame);
    CHECK(frame.pixel({0,0})==lumen::Color{240,240,240,255});
    CHECK(frame.pixel({6,6})==lumen::Color{235,240,248,255});
    bool ink=false; for(int x=10;x<60;++x) for(int y=10;y<25;++y) if(frame.pixel({x,y})==lumen::Color{35,45,60,255}) ink=true;
    CHECK(ink);
}
}
int main() { signals(); application_queue(); widgets_and_layout(); canvas(); if(failures) return 1; std::cout<<"All Lumen core tests passed.\n"; }
