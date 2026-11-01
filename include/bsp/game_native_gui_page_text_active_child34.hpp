#pragma once
#include "bsp/native_gui_text_active_storage.hpp"
#include <atomic>

namespace bsp::game {
class GameNativeGuiPageApplication;

// One private Page-prepared Source129 invocation. No Text birth, public
// readiness/context/token or Native ownership is supplied by this callback.
class GameNativeGuiPageTextActiveChild34 final : public NativeGuiTextActiveChild34Bindings {
public:
    ~GameNativeGuiPageTextActiveChild34() override = default;
    GameNativeGuiPageTextActiveChild34(const GameNativeGuiPageTextActiveChild34&) = delete;
    GameNativeGuiPageTextActiveChild34& operator=(const GameNativeGuiPageTextActiveChild34&) = delete;
    GameNativeGuiPageTextActiveChild34(GameNativeGuiPageTextActiveChild34&&) = delete;
    GameNativeGuiPageTextActiveChild34& operator=(GameNativeGuiPageTextActiveChild34&&) = delete;
    void call_visible_34_00aa8530(void*, std::uint32_t captured_profile,
        std::uint32_t captured_target, std::uint32_t captured_flags_dword) override;
private:
    friend class GameNativeGuiPageApplication;
    explicit GameNativeGuiPageTextActiveChild34(GameNativeGuiPageApplication&);
    GameNativeGuiPageApplication& page_;
    const std::uint32_t owner_thread_;
    // Page-owned stable denial storage; never dereference Page on a foreign
    // thread. This pointer is filled cold before publishing the callback.
    std::atomic<bool>* denial_{};
};
} // namespace bsp::game
