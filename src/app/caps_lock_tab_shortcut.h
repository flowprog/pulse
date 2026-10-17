#pragma once
#include <windows.h>

namespace pulse {

struct AppCore;
struct AppState;

bool CanUseCapsLockTabShortcut(const AppState& state);

class CapsLockTabShortcut {
public:
    explicit CapsLockTabShortcut(AppCore& core);
    ~CapsLockTabShortcut();
    CapsLockTabShortcut(const CapsLockTabShortcut&) = delete;
    CapsLockTabShortcut& operator=(const CapsLockTabShortcut&) = delete;

private:
    static LRESULT CALLBACK KeyboardHook(int code, WPARAM wparam, LPARAM lparam);

    AppCore& core_;
    HHOOK hook_ = nullptr;
    bool key_down_ = false;
    bool consumed_ = false;
};

} // namespace pulse
