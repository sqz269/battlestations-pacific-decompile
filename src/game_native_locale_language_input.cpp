#include "bsp/game_native_locale_language_input.hpp"
#include "bsp/game_native_locale_manager.hpp"
#include "bsp/game_native_settings_application.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw locale language input requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4,
    "requires the Win32 raw address model");

std::uint32_t word(const void* base, std::uint32_t offset) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(base) + offset;
    std::uint32_t result;
    _ReadWriteBarrier();
    std::memcpy(&result, reinterpret_cast<const void*>(address), sizeof result);
    _ReadWriteBarrier();
    return result;
}
void* pointer(std::uint32_t bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
} // namespace

GameNativeLocaleLanguageInputContext::GameNativeLocaleLanguageInputContext(
    GameNativeSettingsApplication& settings, GameNativeLocaleManagerProcess& locale,
    GameNativeStringProcess& strings) noexcept
    : settings_(settings), locale_(locale), strings_(strings) {}

GameNativeLocaleLanguageInputFrame::GameNativeLocaleLanguageInputFrame(
    const GameNativeLocaleLanguageInputContext& context) noexcept : context_(context) {}

void GameNativeLocaleLanguageInputFrame::fail() noexcept {
    phase_ = Phase::failed;
    normal_result_ = false;
    progress_.failed = true;
}
[[noreturn]] void GameNativeLocaleLanguageInputFrame::reject(const char* message) {
    fail(); // sticky before diagnostic allocation/provider reentry
    throw std::logic_error(message);
}
void GameNativeLocaleLanguageInputFrame::require_active() {
    if (phase_ != Phase::active || progress_.failed)
        reject("locale language input cannot resume after failure/reentry");
}
void GameNativeLocaleLanguageInputFrame::unwind_selected() noexcept {
    if (state_ != 0x25) return;
    state_ = -1; // consume THIS invocation's selected edge before lower cleanup
    progress_.selected_edge_consumed = true;
    progress_.selected_destroy_entered = true;
    try {
        destroy_native_string_header_0041dd20(input_, *raw_); // CURRENT header
        progress_.selected_destroy_returned = true;
    } catch (...) {
        std::terminate(); // second exception on the modeled true unwind edge
    }
}

void initialize_native_locale_language_input_0073e06a(
    const GameNativeLocaleLanguageInputContext& c, GameNativeLocaleLanguageInputFrame& f) {
    // Rejected nested invocation is OUTSIDE its cleanup try. The actually
    // admitted outer invocation alone owns/consumes its Native state0x25.
    if (&c != &f.context_ || f.phase_ != GameNativeLocaleLanguageInputFrame::Phase::fresh)
        f.reject("locale language input requires its fresh exact Context/Frame");
    f.phase_ = GameNativeLocaleLanguageInputFrame::Phase::active;
    f.progress_.attempted = true;
    try {
        if (&c.strings_ != &game_native_string_process() ||
            &c.locale_ != &game_native_locale_manager_process())
            f.reject("locale language input requires its actual canonical processes");
        auto& raw = c.strings_.raw_context();
        if (&raw.actual_published_01090aa8 != &c.strings_.pool_01090aa8() ||
            &raw.actual_small_returns_disabled_01090aa4 != &c.strings_.returns_disabled_01090aa4() ||
            &raw.actual_manager_publication_01090aa0 != &c.strings_.manager_01090aa0())
            f.reject("locale language input requires its SAME permanent raw cells");
        f.raw_ = &raw;
        f.require_active();
        f.progress_.language_entered = true;
        f.language_ = c.settings_.raw_language_name_008d4870();
        f.progress_.language_returned = true;
        f.require_active();
        f.progress_.construction_entered = true;
        f.constructor_result_ = construct_native_string_header_0041e870(f.input_, raw, f.language_);
        f.progress_.construction_returned = true;
        f.state_ = 0x25; // actual ctor return arms BEFORE the healthy Source fence
        f.progress_.state_0x25_armed = true;
        f.require_active();
        if (f.constructor_result_ != f.input_)
            f.reject("locale language input constructor did not return its actual header");
        f.progress_.locale_entered = true;
        c.locale_.set_initial_language_before_tables(f.input_); // BORROW only
        f.progress_.locale_returned = true;
        f.require_active();

        f.captured_data_ = pointer(word(f.input_, 4)); // 73E094 DATA capture
        f.state_ = -1; // 73E09A disarms BEFORE the conditional CURRENT length
        f.progress_.normal_edge_consumed = true;
        if (f.captured_data_) {
            f.captured_bytes_ = word(f.input_, 0) + 1u; // DWORD wrap, 73E0A3
            f.progress_.pool_getter_entered = true;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                raw.actual_published_01090aa8, raw.actual_manager_publication_01090aa0);
            f.progress_.pool_getter_returned = true;
            f.require_active(); // consumed edge: reject, never retry cleanup
            f.progress_.normal_return_entered = true;
            return_native_string_pool_00bd1510(pool, f.captured_data_, f.captured_bytes_,
                raw.actual_small_returns_disabled_01090aa4);
            f.progress_.normal_return_returned = true;
            f.require_active(); // own metadata only; input backing may have ended
        } else {
            f.progress_.normal_null_skip = true; // NO length/getter/return read
        }
        f.require_active();
        f.normal_result_ = true;
        f.progress_.normal_completed = true;
        f.phase_ = GameNativeLocaleLanguageInputFrame::Phase::returned;
    } catch (...) {
        f.fail();
        f.unwind_selected();
        throw;
    }
}
} // namespace bsp::game
