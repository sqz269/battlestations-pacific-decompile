#pragma once

#include "bsp/native_lua_dot_path.hpp"
#include "bsp/native_lua_value_string.hpp"
#include <array>
#include <cstdint>

namespace bsp {
struct NativeLuaPathValueStringInvalidSpan {
    const char* data;
    std::uintptr_t original_address;
    std::uint32_t extent_including_nul;
};
class NativeLuaPathValueStringFrame;

// Immutable real root14/output8/path8/provider tuple. Preparation validates
// metadata only. Actual literal/mutable cells, interpreter/tracking and all
// reached backing/code lifetimes remain caller proof; no VM or text is created.
class NativeLuaPathValueStringContext final {
public:
    NativeLuaPathValueStringContext(NativeLuaObjectStorage& actual_root14,
        void* actual_fresh_output8, const void* borrowed_path8,
        NativeStringRawPoolContext&,
        const volatile std::uint32_t& actual_conversion_0109eea4,
        const std::array<NativeLuaTypeNameSpan, 11>& actual_type_spans,
        const std::array<NativeLuaValueStringSpan, 4>& actual_value_spans,
        NativeLuaDotPathLiteralSpan actual_dot,
        NativeLuaDotPathLiteralSpan actual_zero,
        char* actual_mutable_empty_0108ff2c,
        NativeLuaPathValueStringInvalidSpan actual_invalid_cf8594);
    NativeLuaPathValueStringContext(const NativeLuaPathValueStringContext&) = delete;
    NativeLuaPathValueStringContext& operator=(const NativeLuaPathValueStringContext&) = delete;
    NativeLuaPathValueStringContext(NativeLuaPathValueStringContext&&) = delete;
    NativeLuaPathValueStringContext& operator=(NativeLuaPathValueStringContext&&) = delete;
private:
    friend class NativeLuaPathValueStringFrame;
    friend void* construct_native_lua_path_value_string_00b692c0(
        const NativeLuaPathValueStringContext&, NativeLuaPathValueStringFrame&);
    NativeLuaObjectStorage& root_;
    void* const output_;
    const void* const path_;
    NativeStringRawPoolContext& strings_;
    const NativeLuaDotPathLiteralSpan dot_, zero_;
    char* const empty_;
    const NativeLuaPathValueStringInvalidSpan invalid_;
    const NativeLuaValueStringContext values_;
};

// Copied diagnostics do not authorize construction, release or completion.
struct NativeLuaPathValueStringProgress {
    bool attempted{}, failed{}, completed{};
    bool lower_entered{}, lower_returned{}, lower_normal_receipt{};
    bool status_read{}, native_state_zero_reached{};
    std::uint8_t captured_status{};
    bool output_constructor_entered{}, output_constructor_returned{};
    bool output_transferred{};
    bool normal_release_entered{}, normal_release_returned{};
    bool temporary_normal_completed{};
    bool selected_temporary_entered{}, selected_temporary_returned{};
};

// Retain this stable one-shot attempt and its lower graph after failure.
// Native temp14/status start uninitialized; lower Context/Frame follow them.
// No destructor invokes Native cleanup, including caller-output cleanup.
class NativeLuaPathValueStringFrame final {
public:
    explicit NativeLuaPathValueStringFrame(const NativeLuaPathValueStringContext&);
    ~NativeLuaPathValueStringFrame() = default;
    NativeLuaPathValueStringFrame(const NativeLuaPathValueStringFrame&) = delete;
    NativeLuaPathValueStringFrame& operator=(const NativeLuaPathValueStringFrame&) = delete;
    NativeLuaPathValueStringFrame(NativeLuaPathValueStringFrame&&) = delete;
    NativeLuaPathValueStringFrame& operator=(NativeLuaPathValueStringFrame&&) = delete;
    NativeLuaPathValueStringProgress progress() const noexcept { return progress_; }
    bool has_normal_result() const noexcept {
        return phase_ == Phase::returned && !failed_ && normal_result_;
    }
    void* normal_result() const noexcept {
        return has_normal_result() ? context_.output_ : nullptr;
    }
    // Actual-constructor facts tied to this tuple, not a generic free permit.
    bool output_constructor_entered() const noexcept {
        return output_debt_ != OutputCtorDebt::not_entered;
    }
    bool output_constructor_returned() const noexcept {
        return output_debt_ == OutputCtorDebt::normal_returned ||
            output_debt_ == OutputCtorDebt::transferred;
    }
    bool requires_process_retention() const noexcept {
        return phase_ == Phase::active || phase_ == Phase::failed;
    }
private:
    friend void* construct_native_lua_path_value_string_00b692c0(
        const NativeLuaPathValueStringContext&, NativeLuaPathValueStringFrame&);
    enum class Phase { fresh, active, returned, failed };
    enum class OutputCtorDebt { not_entered, entered_unresolved, normal_returned, transferred };
    const NativeLuaPathValueStringContext& context_;
    NativeLuaObjectStorage temporary_;
    std::uint8_t status_;
    const NativeLuaDotPathContext lower_context_;
    NativeLuaDotPathFrame lower_frame_;
    Phase phase_{Phase::fresh};
    OutputCtorDebt output_debt_{OutputCtorDebt::not_entered};
    bool failed_{}, temporary_action_armed_{}, normal_result_{};
    NativeLuaObjectStorage* lower_result_{};
    void* output_result_{};
    NativeLuaPathValueStringProgress progress_{};
    void fail() noexcept;
    [[noreturn]] void reject(const char*);
    void require_active();
    void unwind_temporary() noexcept;
};

// Whole B692C0 ordinary Source boundary: Native ECX=root14, stack hidden
// result8/path8, EAX=SAME result, RET8. No current-E1/VM/caller activation or
// pcall/error repair. Lower escape issues no wrapper temp action; a normally
// returned temp is released inline WITHOUT kind-clear, while actual armed
// escape selects CURRENT B67700. Caller output is NEVER locally destroyed.
// Lua nonlocal/private-stack/FH3/SEH/fault and current-provider limits remain.
void* construct_native_lua_path_value_string_00b692c0(
    const NativeLuaPathValueStringContext&, NativeLuaPathValueStringFrame&);
} // namespace bsp
