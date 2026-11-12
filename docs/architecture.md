# Architecture

## Current core

The library is split into a public `lumen` API and a small implementation library. `Application` owns a thread-safe FIFO of callbacks and provides a single-threaded dispatch loop. `Widget` owns its children with `std::unique_ptr`; parent pointers are non-owning. Controls expose typed `Signal` objects. Box layouts distribute fixed hints and stretch items deterministically. Focus and key events are routed within a widget tree; the initial `LineEdit` handles ASCII editing/navigation only. `Canvas` is a simple RGBA software target with hierarchical widget clipping and clipped rectangles and a small built-in bitmap font for tests and lightweight rendering.

The current `show()` operation marks a widget visible; it does not create a native window. Native surfaces, event-loop integration, full Unicode text shaping, accessibility bridges and platform services are explicitly outside current milestones. `Application::post` is safe across threads; widget-tree mutation and signal operations must stay on the UI thread.

## Extension seams

Future platform backends should translate native input into framework events and present a canvas/frame through a backend interface. Layout and controls must remain independently testable. Widget mutations and painting are currently expected on the UI thread; only posting work to `Application` is thread-safe.
