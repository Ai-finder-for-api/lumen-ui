# Contributing

Lumen is experimental. Issues and focused pull requests are welcome. Before proposing public API changes, describe the use case and compatibility implications.

## Development

- Use C++20 and keep the core dependency-free.
- Build with `cmake -S . -B build -DLUMEN_BUILD_TESTS=ON && cmake --build build`.
- Run `ctest --test-dir build --output-on-failure`.
- Add tests for behavioral changes; keep platform-specific code behind a backend boundary.
- Prefer small value types, RAII, clear ownership and documented invariants.

## Pull requests

Explain motivation, behavior and limitations. Do not claim platform/API support unless it is implemented and tested. Formatting follows the existing style (4-space indentation, braces on the same line).
