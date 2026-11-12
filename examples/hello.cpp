#include <lumen/lumen.hpp>
#include <iostream>

int main() {
    lumen::Application app;
    lumen::Widget window;
    window.set_geometry({0,0,360,170});
    window.set_background({250,251,253,255});
    auto layout=std::make_unique<lumen::BoxLayout>(lumen::BoxLayout::Direction::Vertical,8,lumen::Insets{16,14,16,14});
    auto* box=layout.get();
    window.set_layout(std::move(layout));
    auto& title=window.emplace<lumen::Label>("Hello, Lumen!");
    auto& enabled=window.emplace<lumen::CheckBox>("Enable previews");
    auto& button=window.emplace<lumen::Button>("Continue");
    box->add(title); box->add(enabled); box->add(button); window.update_layout();
    button.clicked.connect([&]{ std::cout<<"Action activated; no native backend is connected yet.\n"; });
    enabled.toggled.connect([](bool on){std::cout<<"Previews "<<(on?"enabled":"disabled")<<".\n";});
    // The current release provides a deterministic software core, not a native window.
    lumen::Canvas frame({360,170},{255,255,255,255});
    window.render(frame);
    std::cout<<"Rendered a "<<frame.size().width<<'x'<<frame.size().height
             <<" software frame. Native window backends are on the roadmap.\n";
    app.post([&]{ app.quit(0); });
    return app.exec();
}
