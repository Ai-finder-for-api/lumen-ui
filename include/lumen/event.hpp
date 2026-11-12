#pragma once
#include <cstdint>
namespace lumen {
enum class KeyCode { Character, Backspace, Delete, Left, Right, Home, End, Enter, Escape, Tab, Space };
struct KeyEvent {
    KeyCode code{KeyCode::Character};
    char32_t text{U'\0'};
    bool shift{false};
    bool control{false};
};
}
