# Lumen UI

**A modular, Qt-inspired C++20 UI toolkit designed for clarity, portability, and small applications.**

Lumen is an independent project, not a Qt wrapper or a claim of API/feature parity. Its goal is to grow toward the breadth of a modern application UI framework while keeping the core understandable and dependency-light.

## Project status

Early development. The first two milestones establish a buildable retained-mode core: a thread-safe application task queue, parent-owned widgets, box layouts, focus and key dispatch, typed signals, basic controls (label, button, checkbox, line edit, slider, progress bar), and a hierarchically clipped, deterministic software canvas. The toolkit is backend-neutral; a window-system backend is a planned extension, not yet included.

## Build

```sh
cmake -S . -B build -DLUMEN_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Requires CMake 3.20+ and a C++20 compiler. No external dependencies. To install and consume the exported target, run `cmake --install build --prefix <prefix>`, then use `find_package(Lumen 0.1 CONFIG REQUIRED)` and link `Lumen::Lumen`.

## Minimal example

```cpp
#include <lumen/lumen.hpp>
int main() {
    lumen::Widget view;
    view.set_geometry({0, 0, 320, 120});
    auto& title = view.emplace<lumen::Label>("Hello, Lumen!");
    title.set_geometry({12, 12, 296, 22});
    auto& name = view.emplace<lumen::LineEdit>("Ada");
    name.set_geometry({12, 44, 296, 32});
    name.set_focus();
    lumen::Canvas frame({320, 120}, lumen::white);
    view.render(frame); // A native window/presenter is not part of this release.
}
```

Keyboard input can be routed to the focused control with `view.dispatch_key({lumen::KeyCode::Character, U'!'})`. See `examples/hello.cpp` and `tests/test_main.cpp` for layout, signal and input examples.

## Design principles

- **Useful defaults, explicit extension points:** small controls compose from a predictable widget tree.
- **Portable core:** event handling, layout, signals, and software drawing do not depend on a native window system.
- **Testable:** deterministic geometry and rendering primitives work without a display server.
- **Modern C++:** C++20, RAII ownership, typed signals, and no macro-based object system.

## Planned roadmap

1. [x] Repository, build system, core API and tests.
2. [x] Box layouts, basic keyboard focus/input, editable single-line text and software canvas controls.
3. [x] Basic CMake install/export package.
4. [ ] Grid/form/stacked layouts, styles/themes, accessibility metadata and Unicode shaping.
5. [ ] Native window backends, high-DPI input, clipboard and IME.
6. [ ] Rich text, models/views, menus, dialogs and resource tooling.
7. [ ] ABI policy, examples and broader platform CI.

See [docs/architecture.md](docs/architecture.md), [docs/api.md](docs/api.md), and [CONTRIBUTING.md](CONTRIBUTING.md). APIs are experimental until a stable release.
