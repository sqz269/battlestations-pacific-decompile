#include "bsp/native_lua_path_value_string.hpp"

#include <cstring>
#include <exception>
#include <intrin.h>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeLuaObjectStorage) == 0x14);
std::uint32_t word(const void* bytes, std::size_t offset) noexcept {
    std::uint32_t value;
    _ReadWriteBarrier();
    std::memcpy(&value, static_cast<const unsigned char*>(bytes) + offset, 4);
    _ReadWriteBarrier();
    return value;
}
std::uint8_t status_byte(const std::uint8_t& status) noexcept {
    std::uint8_t value;
    _ReadWriteBarrier();
    std::memcpy(&value, &status, 1);
    _ReadWriteBarrier();
    return value;
}
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof result);
    return result;
}
} // namespace

NativeLuaPathValueStringContext::NativeLuaPathValueStringContext(
    NativeLuaObjectStorage& root, void* output, const void* path,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& conversion,
    const std::array<NativeLuaTypeNameSpan, 11>& type_spans,
    const std::array<NativeLuaValueStringSpan, 4>& value_spans,
    NativeLuaDotPathLiteralSpan dot, NativeLuaDotPathLiteralSpan zero,
    char* empty, NativeLuaPathValueStringInvalidSpan invalid)
    : root_(root), output_(output), path_(path), strings_(strings),
      dot_(dot), zero_(zero), empty_(empty), invalid_(invalid),
      values_(strings, conversion, type_spans, value_spans) {
    // Identity/extent metadata only; no literal/header/cell/interpreter read.
    if (!output || !path || invalid.original_address != 0x00cf8594u ||
        invalid.extent_including_nul != 8u ||
        reinterpret_cast<std::uintptr_t>(invalid.data) != 0x00cf8594u)
        throw std::invalid_argument("raw path value requires actual output/path/invalid span");
}

NativeLuaPathValueStringFrame::NativeLuaPathValueStringFrame(
    const NativeLuaPathValueStringContext& context)
    : context_(context),
      lower_context_(context.root_, &temporary_, context.path_, &status_,
          context.strings_, context.dot_, context.zero_, context.empty_),
      lower_frame_(lower_context_) {}

void NativeLuaPathValueStringFrame::fail() noexcept {
    failed_ = true;
    phase_ = Phase::failed;
    progress_.failed = true;
}
[[noreturn]] void NativeLuaPathValueStringFrame::reject(const char* text) {
    fail();
    throw std::logic_error(text);
}
void NativeLuaPathValueStringFrame::require_active() {
    if (phase_ != Phase::active || failed_)
        reject("raw path value invocation is not healthy and active");
}
void NativeLuaPathValueStringFrame::unwind_temporary() noexcept {
    if (!temporary_action_armed_) return;
    temporary_action_armed_ = false; // consume selected action before its call
    progress_.selected_temporary_entered = true;
    try {
        destroy_native_lua_object_00b67700(temporary_);
        progress_.selected_temporary_returned = true;
    } catch (...) {
        std::terminate();
    }
}

void* construct_native_lua_path_value_string_00b692c0(
    const NativeLuaPathValueStringContext& c, NativeLuaPathValueStringFrame& f) {
    // A rejected inner invocation must not select the outer invocation's action.
    if (&f.context_ != &c || f.phase_ != NativeLuaPathValueStringFrame::Phase::fresh)
        f.reject("raw path value Frame mismatch, reentry or replay");
    f.phase_ = NativeLuaPathValueStringFrame::Phase::active;
    f.progress_.attempted = true;
    try {
        f.progress_.lower_entered = true;
        f.lower_result_ = resolve_native_lua_dot_path_00b68d70(
            f.lower_context_, f.lower_frame_);
        f.progress_.lower_returned = true;
        if (!f.lower_frame_.has_normal_result() || f.lower_result_ != &f.temporary_)
            f.reject("raw dot path did not transfer this actual temporary");
        f.progress_.lower_normal_receipt = true;
        // Private actual-construction authority precedes the added Source fence.
        // Native status-read then physical state0-arm remain the next epochs.
        f.temporary_action_armed_ = true;
        f.require_active();
        const auto status = status_byte(f.status_);
        f.progress_.captured_status = status;
        f.progress_.status_read = true;
        f.progress_.native_state_zero_reached = true;

        f.output_debt_ = NativeLuaPathValueStringFrame::OutputCtorDebt::entered_unresolved;
        f.progress_.output_constructor_entered = true;
        if (status != 0) {
            f.output_result_ = construct_native_lua_value_string_00b69130(
                f.temporary_, c.output_, c.values_);
        } else {
            f.output_result_ = construct_native_string_header_0041e870(
                c.output_, c.strings_, c.invalid_.data);
        }
        f.output_debt_ = NativeLuaPathValueStringFrame::OutputCtorDebt::normal_returned;
        f.progress_.output_constructor_returned = true;
        if (f.output_result_ != c.output_)
            f.reject("raw value constructor returned a different output");
        f.require_active();

        if (word(&f.temporary_, 4) != 0) {
            const auto index = signed_word(word(&f.temporary_, 8));
            auto* const owner = reinterpret_cast<NativeLuaStateStorage*>(
                static_cast<std::uintptr_t>(word(&f.temporary_, 0)));
            f.progress_.normal_release_entered = true;
            release_native_lua_tracked_object_00b66de0(owner, f.temporary_, index, 1);
            f.progress_.normal_release_returned = true;
            // No current-kind clear; the actual call's escape is STILL armed.
        }
        // Actual normal release/kind0 disposition consumes private temp debt
        // BEFORE the final Source fence, preventing a swallowed-reentry retry.
        f.temporary_action_armed_ = false;
        f.progress_.temporary_normal_completed = true;
        f.require_active();
        f.output_debt_ = NativeLuaPathValueStringFrame::OutputCtorDebt::transferred;
        f.progress_.output_transferred = true;
        f.normal_result_ = true;
        f.progress_.completed = true;
        f.phase_ = NativeLuaPathValueStringFrame::Phase::returned;
        return c.output_;
    } catch (...) {
        f.fail();
        f.unwind_temporary();
        throw;
    }
}
} // namespace bsp
