#include "caps_lock_tab_shortcut.h"
#include "app_messages.h"
#include "app_state.h"

namespace pulse {
namespace {
CapsLockTabShortcut* g_shortcut = nullptr;
}

bool CanUseCapsLockTabShortcut(const AppState& state) {
    // Text editors and dialogs own their focus; only the main surface is a shortcut target.
    return GetForegroundWindow() == state.hwnd && GetFocus() == state.hwnd &&
        IsWindowEnabled(state.hwnd) && state.renameIndex < 0 &&
        !state.filterEditing && state.tagRenameId.empty() &&
        !state.settings.global_search_hotkey_capturing();
}

CapsLockTabShortcut::CapsLockTabShortcut(AppCore& core) : core_(core) {
    key_down_ = (GetAsyncKeyState(VK_CAPITAL) & 0x8000) != 0;
    g_shortcut = this;
    // WM_KEYDOWN is too late to prevent the system from toggling Caps Lock.
    hook_ = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardHook, GetModuleHandleW(nullptr), 0);
}

CapsLockTabShortcut::~CapsLockTabShortcut() {
    if (hook_) UnhookWindowsHookEx(hook_);
    g_shortcut = nullptr;
}

LRESULT CALLBACK CapsLockTabShortcut::KeyboardHook(int code, WPARAM wparam, LPARAM lparam) {
    if (code != HC_ACTION || !g_shortcut)
        return CallNextHookEx(nullptr, code, wparam, lparam);
    const auto& key = *reinterpret_cast<const KBDLLHOOKSTRUCT*>(lparam);
    if (key.vkCode != VK_CAPITAL)
        return CallNextHookEx(nullptr, code, wparam, lparam);

    auto& shortcut = *g_shortcut;
    const bool key_up = wparam == WM_KEYUP || wparam == WM_SYSKEYUP;
    if (!key_up && !shortcut.key_down_) {
        shortcut.key_down_ = true;
        AppState* target = shortcut.core_.windows.Lookup(GetForegroundWindow());
        shortcut.consumed_ = target && CanUseCapsLockTabShortcut(*target) &&
            PostMessageW(target->hwnd, WM_CAPS_LOCK_NEW_TAB, 0, 0);
    }
    // Keep the initial decision through repeats and key-up, even if focus changes.
    const bool consumed = shortcut.consumed_;
    if (key_up) {
        shortcut.key_down_ = false;
        shortcut.consumed_ = false;
    }
    return consumed ? 1 : CallNextHookEx(nullptr, code, wparam, lparam);
}

} // namespace pulse
