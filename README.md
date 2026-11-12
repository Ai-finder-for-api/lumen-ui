# Lumen UI

**A modular, Qt-inspired C++20 UI toolkit designed for clarity, portability, and small applications.**

Lumen is an independent project, not a Qt wrapper or a claim of API/feature parity. Its goal is to grow toward the breadth of a modern application UI framework while keeping the core understandable and dependency-light.

## Project status

Early development. The first milestone establishes a buildable retained-mode core: an application event queue, parent-owned widgets, geometry and layout, signals, basic controls, and a deterministic software canvas. The toolkit is backend-neutral; a window-system backend is a planned extension, not yet included.

## Build

```sh
cmake -S . -B build -DLUMEN_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Requires CMake 3.20+ and a C++20 compiler. No external dependencies.

## Minimal example

```cpp
#include <lumen/lumen.hpp>

int main() {
    lumen::Application app;
    lumen::Widget window;
    window.set_size({320, 160});
    auto& title = window.emplace<lumen::Label>("Hello, Lumen!");
    title.set_alignment(lumen::Alignment::Center);
    auto& button = window.emplace<lumen::Button>("Continue");
    button.clicked.connect([] { /* handle action */ });
    app.post([&] { window.show(); });
    return app.exec();
}
```

## Design principles

- **Useful defaults, explicit extension points:** small controls compose from a predictable widget tree.
- **Portable core:** event handling, layout, signals, and software drawing do not depend on a native window system.
- **Testable:** deterministic geometry and rendering primitives work without a display server.
- **Modern C++:** C++20, RAII ownership, typed signals, and no macro-based object system.

## Planned roadmap

1. [x] Repository, build system, core API and tests.
2. [ ] Layout containers, styles/themes, accessibility metadata, text shaping.
3. [ ] Native window backends, high-DPI input, clipboard and IME.
4. [ ] Rich text, models/views, menus, dialogs and resource tooling.
5. [ ] Packaging, ABI policy, examples and broader platform CI.

See [docs/architecture.md](docs/architecture.md) and [CONTRIBUTING.md](CONTRIBUTING.md). APIs are experimental until a stable release.
