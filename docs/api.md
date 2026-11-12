# Core API notes (0.1.x)

This document describes the current small core; it is not a feature-parity promise.

## Ownership and layout

Create child controls with `parent.emplace<Control>(args...)`. The parent owns each child through `std::unique_ptr`, so references returned by `emplace` stay valid until the parent is destroyed. Geometry for a child is relative to its parent. `VBoxLayout` and `HBoxLayout` arrange controls added with `layout.add(widget, stretch)`: zero stretch uses the control's `size_hint()`, positive stretch shares remaining main-axis space by weight. Margins and spacing are in logical integer pixels.

## Input and focus

`Button`, `CheckBox`, and `LineEdit` are focusable. Call `set_focus()` or pass pointer presses through `Widget::dispatch_mouse_press`; send key events from the root with `dispatch_key`. Enter/Space activate buttons and toggle checkboxes. `LineEdit` supports printable ASCII insertion, deletion, cursor movement, Home/End, and Enter (`editing_finished`). Unicode shaping, selections, clipboard, IME, Tab traversal, and mouse dragging are not implemented yet.

## Signals

Connect with `signal.connect(callable)`. The returned `Connection` is a weak handle; discarding it does not unregister the callback. Call `disconnect()` to remove it explicitly. Slots fire in connection order. Emission snapshots callbacks, so connection changes during an emission take effect on the next emission. Signals and widgets are UI-thread-only.

## Software canvas

`Canvas` stores row-major RGBA8 pixels, clips primitive drawing to the active clip rectangle, and supports solid fill/stroke and a compact built-in 5x7 ASCII bitmap font. Widgets automatically intersect drawing with their own bounds and ancestor clips. It is a deterministic core renderer target, not a native-window presenter or a production font rasterizer.

## Application queue

Only one `Application` instance may be active. `post()` is thread-safe; `exec()` dispatches queued callbacks on the calling thread until `quit()`. Use `process_events()` to drain immediately available work in embedded/test scenarios. Widget tree access and UI signals should remain on the UI thread.
