#include "bsp/game_native_locale_globals_input.hpp"
#include "bsp/game_native_locale_manager.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw globals input requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 && sizeof(Word) == 4);

Word word(const void* header, Word offset) noexcept {
    Word value;
    const auto location = reinterpret_cast<std::uintptr_t>(header) + offset;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(location), sizeof value);
    _ReadWriteBarrier();
    return value;
}
void put(void* header, Word offset, Word value) noexcept {
    const auto location = reinterpret_cast<std::uintptr_t>(header) + offset;
    _ReadWriteBarrier();
    std::memcpy(reinterpret_cast<void*>(location), &value, sizeof value);
    _ReadWriteBarrier();
}
void* pointer(Word bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
bool overlaps(const volatile void* a, std::size_t an,
    const volatile void* b, std::size_t bn) noexcept {
    const auto av = reinterpret_cast<std::uintptr_t>(a);
    const auto bv = reinterpret_cast<std::uintptr_t>(b);
    return av >= bv ? av - bv < bn : bv - av < an;
}
} // namespace

GameNativeLocaleGlobalsInputContext::GameNativeLocaleGlobalsInputContext(
    GameNativeLocaleManagerProcess& owner, GameNativeStringProcess& strings,
    const GameNativeReadOnlyData& data) noexcept
    : owner_(owner), strings_(strings), data_(data) {}

GameNativeLocaleGlobalsInputFrame::GameNativeLocaleGlobalsInputFrame(
    const GameNativeLocaleGlobalsInputContext& context) noexcept : context_(context) {}

void GameNativeLocaleGlobalsInputFrame::fail() noexcept {
    phase_ = Phase::failed;
    normal_result_ = false;
    progress_.failed = true;
}
[[noreturn]] void GameNativeLocaleGlobalsInputFrame::reject(const char* message) {
    fail(); // latch before diagnostic allocation can reenter
    throw std::logic_error(message);
}
void GameNativeLocaleGlobalsInputFrame::require_active() {
    if (phase_ != Phase::active || progress_.failed)
        reject("globals input cannot resume after failure/reentry");
}
void GameNativeLocaleGlobalsInputFrame::complete() {
    require_active();
    normal_result_ = true;
    progress_.normal_returned = true;
    phase_ = Phase::returned;
}

void register_native_locale_globals_input_0073e0ba(
    const GameNativeLocaleGlobalsInputContext& c, GameNativeLocaleGlobalsInputFrame& f) {
    // An inner rejected call MUST NOT enter the active outer invocation's
    // selected-cleanup scope, even if that outer frame is already armed.
    if (&c != &f.context_ || f.phase_ != GameNativeLocaleGlobalsInputFrame::Phase::fresh)
        f.reject("globals input requires its exact fresh Context/Frame");
    f.phase_ = GameNativeLocaleGlobalsInputFrame::Phase::active;
    f.progress_.attempted = true;
    auto& raw = c.strings_.raw_context();
    try {
        if (&c.owner_ != &game_native_locale_manager_process() ||
            &c.strings_ != &game_native_string_process() ||
            &raw.actual_published_01090aa8 != &c.strings_.pool_01090aa8() ||
            &raw.actual_small_returns_disabled_01090aa4 != &c.strings_.returns_disabled_01090aa4() ||
            &raw.actual_manager_publication_01090aa0 != &c.strings_.manager_01090aa0() ||
            overlaps(&f, sizeof f, &c, sizeof c) ||
            overlaps(&f, sizeof f, &c.owner_, sizeof c.owner_) ||
            overlaps(&f, sizeof f, &c.strings_, sizeof c.strings_) ||
            overlaps(&f, sizeof f, &c.data_, sizeof c.data_))
            f.reject("globals input requires the disjoint SAME canonical owner/string graph");
        f.require_active();
        f.progress_.admitted = true;
        // Actual verified RO mapping, never a replacement literal constructor.
        f.literal_ = c.data_.data_at(0x00cf7774u, 8u);
        f.progress_.literal_returned = true;
        f.require_active();
        if (overlaps(&f, sizeof f, f.literal_, 8u) ||
            overlaps(&c, sizeof c, f.literal_, 8u))
            f.reject("globals input metadata cannot alias the actual literal span");

        put(f.header_, 0u, 0u); //73E0C2
        f.progress_.length_zeroed = true;
        put(f.header_, 4u, 0u); //73E0C6
        f.progress_.data_zeroed = true;
        f.progress_.resize_entered = true;
        resize_native_string_header_0041dd40(f.header_, raw, 7u, true);
        f.progress_.resize_returned = true;
        f.require_active();
        const Word copy_data = word(f.header_, 4u); //73E0CF BEFORE length
        if (copy_data != 0u) {
            const Word copied = word(f.header_, 0u) + 1u;
            f.progress_.copy_entered = true;
            if (copied != 0u) std::memmove(pointer(copy_data), f.literal_, copied);
            f.progress_.copy_returned = true;
        } else {
            f.progress_.copy_skipped = true;
        }
        // Arm only after construction/copy NORMAL return, before a healthy
        // fence can observe a provider's swallowed recursive rejection.
        f.cleanup_ = GameNativeLocaleGlobalsInputFrame::Cleanup::armed_26;
        f.progress_.state26_armed = true;
        f.require_active();
        f.progress_.registration_entered = true;
        c.owner_.register_table_name(f.header_);
        f.progress_.registration_returned = true;
        f.require_active();

        const Word returned_data = word(f.header_, 4u); //73E108, EARLY
        f.progress_.normal_data_captured = true;
        f.cleanup_ = GameNativeLocaleGlobalsInputFrame::Cleanup::consumed; //73E10E
        f.progress_.normal_disarmed = true;
        if (returned_data != 0u) {
            const Word returned_size = word(f.header_, 0u) + 1u;
            f.progress_.getter_entered = true;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                raw.actual_published_01090aa8, raw.actual_manager_publication_01090aa0);
            f.progress_.getter_returned = true;
            f.require_active(); // returning getter may have swallowed reentry
            f.progress_.return_entered = true;
            return_native_string_pool_00bd1510(pool, pointer(returned_data), returned_size,
                raw.actual_small_returns_disabled_01090aa4);
            f.progress_.return_returned = true;
            f.require_active();
        }
        f.complete();
    } catch (...) {
        f.fail();
        if (f.cleanup_ == GameNativeLocaleGlobalsInputFrame::Cleanup::armed_26) {
            f.cleanup_ = GameNativeLocaleGlobalsInputFrame::Cleanup::consumed;
            f.progress_.selected_entered = true;
            try {
                // C86C2F: CURRENT header, distinct from the EARLY normal pair.
                destroy_native_string_header_0041dd20(f.header_, raw);
                f.progress_.selected_returned = true;
            } catch (...) {
                std::terminate(); // true secondary exception during selected unwind
            }
        }
        throw; // prearm/consumed footer has no action; retain every unresolved effect
    }
}
} // namespace bsp::game
