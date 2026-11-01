#include "bsp/game_native_gui_page_text_active_child34.hpp"
#include "bsp/game_native_gui_page_application.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <stdexcept>

namespace bsp::game {
GameNativeGuiPageTextActiveChild34::GameNativeGuiPageTextActiveChild34(GameNativeGuiPageApplication& page)
    : page_(page), owner_thread_(GetCurrentThreadId()) {}

void GameNativeGuiPageTextActiveChild34::call_visible_34_00aa8530(void* child,
    std::uint32_t profile, std::uint32_t target, std::uint32_t flags) {
    if (GetCurrentThreadId() != owner_thread_ || !denial_) {
        if (denial_) denial_->store(true, std::memory_order_release);
        throw std::logic_error("captured Text child34 requires its private Page caller thread");
    }
    page_.call_captured_text_child34(*this, child, profile, target, flags);
}
} // namespace bsp::game
