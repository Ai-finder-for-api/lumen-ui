#include <lumen/lumen.hpp>
#include <atomic>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>
#include <vector>
namespace {
int failures=0;
void check(bool ok,const char* expr,int line) { if(!ok) { std::cerr<<"FAIL line "<<line<<": "<<expr<<'\n'; ++failures; } }
#define CHECK(...) check(static_cast<bool>((__VA_ARGS__)),#__VA_ARGS__,__LINE__)
void signals() {
    lumen::Signal<int> signal; int sum=0;
    std::vector<int> order;
    auto connection=signal.connect([&](int n){sum+=n; order.push_back(1);});
    signal.connect([&](int){order.push_back(2);});
    signal.emit(3); CHECK(sum==3); CHECK(connection.connected()); CHECK(order==std::vector<int>({1,2}));
    connection.disconnect(); CHECK(!connection.connected()); signal.emit(7); CHECK(sum==3); CHECK(order==std::vector<int>({1,2,2}));
    int calls=0; signal.connect([&](int){++calls;}); signal.connect([&](int){++calls;});
    signal.emit(0); CHECK(calls==2);
}
void application_queue() {
    lumen::Application app; CHECK(lumen::Application::instance()==&app);
    int value=0; app.post([&]{value=1;}); app.post([&]{value+=2;});
    std::thread producer([&]{app.post([&]{value+=4;});}); producer.join();
    CHECK(app.process_events()==3); CHECK(value==7); CHECK(app.process_events()==0);
}
void widgets_and_layout() {
    lumen::Widget root; root.set_geometry({0,0,240,100});
    auto layout=std::make_unique<lumen::VBoxLayout>(4,lumen::Insets{8,6,8,6});
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
    CHECK(button.has_focus()); CHECK(root.dispatch_key({lumen::KeyCode::Space})); CHECK(clicks==2);
    root.dispatch_mouse_press({b.x+4,b.y+4}); root.dispatch_mouse_release({0,0}); CHECK(clicks==2);
    CHECK(root.dispatch_mouse_press({b.x+4,b.y+4})); button.set_enabled(false);
    CHECK(root.dispatch_mouse_release({b.x+4,b.y+4})); CHECK(clicks==2); button.set_enabled(true);
    auto& checkbox=root.emplace<lumen::CheckBox>("Enabled"); checkbox.set_geometry({4,70,120,24});
    bool toggled=false; checkbox.toggled.connect([&](bool v){toggled=v;});
    CHECK(root.dispatch_mouse_press({10,75})); CHECK(checkbox.is_checked()); CHECK(toggled);
    CHECK(root.dispatch_key({lumen::KeyCode::Enter})); CHECK(!checkbox.is_checked());
    slider.set_range(-10,10); slider.set_value(7); CHECK(slider.value()==7);
    slider.set_value(500); CHECK(slider.value()==10);
    int changes=0; slider.value_changed.connect([&](int){++changes;}); slider.set_value(9); CHECK(changes==1);
    lumen::ProgressBar progress; progress.set_range(0,10); progress.set_value(20); CHECK(progress.value()==10);
    slider.set_range(std::numeric_limits<int>::min(),std::numeric_limits<int>::max());
    slider.set_value(std::numeric_limits<int>::max());
    CHECK(slider.value()==std::numeric_limits<int>::max());
    CHECK(slider.dispatch_mouse_press({112,12}));
    CHECK(slider.value()>std::numeric_limits<int>::min() && slider.value()<std::numeric_limits<int>::max());
    progress.set_range(std::numeric_limits<int>::min(),std::numeric_limits<int>::max());
    progress.set_value(std::numeric_limits<int>::max());
}
void line_edit() {
    lumen::Widget root; root.set_geometry({0,0,220,48});
    auto& edit=root.emplace<lumen::LineEdit>("cat"); edit.set_geometry({8,8,190,32}); edit.set_placeholder("Type here");
    CHECK(edit.is_focusable()); CHECK(edit.set_focus()); CHECK(edit.has_focus());
    int changes=0, finished=0;
    edit.text_changed.connect([&](const std::string&){++changes;});
    edit.editing_finished.connect([&]{++finished;});
    CHECK(root.dispatch_key({lumen::KeyCode::Left}));
    CHECK(edit.cursor_position()==2);
    CHECK(root.dispatch_key({lumen::KeyCode::Character,U'X'})); CHECK(edit.text()=="caXt");
    CHECK(root.dispatch_key({lumen::KeyCode::Backspace})); CHECK(edit.text()=="cat");
    CHECK(root.dispatch_key({lumen::KeyCode::Home})); CHECK(root.dispatch_key({lumen::KeyCode::Delete}));
    CHECK(edit.text()=="at"); CHECK(changes==3);
    CHECK(root.dispatch_key({lumen::KeyCode::End})); CHECK(root.dispatch_key({lumen::KeyCode::Space}));
    CHECK(edit.text()=="at ");
    CHECK(root.dispatch_key({lumen::KeyCode::Enter})); CHECK(finished==1);
    auto& other=root.emplace<lumen::LineEdit>(); CHECK(other.set_focus()); CHECK(!edit.has_focus()); CHECK(other.has_focus());
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
    lumen::Widget clipped; clipped.set_geometry({0,0,10,10}); clipped.set_background(lumen::white);
    auto& overflow=clipped.emplace<lumen::Widget>(); overflow.set_geometry({8,8,10,10}); overflow.set_background({255,0,0,255});
    lumen::Canvas clipped_frame({20,20}); clipped.render(clipped_frame);
    CHECK(clipped_frame.pixel({9,9})==lumen::Color{255,0,0,255});
    CHECK(clipped_frame.pixel({10,9})==lumen::Color{0,0,0,0});
    bool ink=false; for(int x=10;x<60;++x) for(int y=10;y<25;++y) if(frame.pixel({x,y})==lumen::Color{35,45,60,255}) ink=true;
    CHECK(ink);
}
}
int main() { signals(); application_queue(); widgets_and_layout(); line_edit(); canvas(); if(failures) return 1; std::cout<<"All Lumen core tests passed.\n"; }
