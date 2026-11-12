# Architecture

## Current core

The library is split into a public `lumen` API and a small implementation library. `Application` owns a thread-safe FIFO of callbacks and provides a single-threaded dispatch loop. `Widget` owns its children with `std::unique_ptr`; parent pointers are non-owning. Controls expose typed `Signal` objects. Layout is deterministic and backend-independent. `Canvas` is a simple RGBA software target for tests and lightweight rendering.

The current `show()` operation marks a widget visible; it does not create a native window. Native surfaces, event-loop integration, font shaping, accessibility bridges and platform services are explicitly outside the first milestone.

## Extension seams

Future platform backends should translate native input into framework events and present a canvas/frame through a backend interface. Layout and controls must remain independently testable. Widget mutations and painting are currently expected on the UI thread; only posting work to `Application` is thread-safe.
