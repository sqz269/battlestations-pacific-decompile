#pragma once

#include "bsp/native_lua_variant_retirement.hpp"
#include <cstdint>

namespace bsp {
struct NativeLuaVariantSlot0ProfileSpan {
    const void* data;
    std::uintptr_t original_address;
    std::uint32_t bytes;
};
enum class NativeLuaVariantSlot0Status : std::uint32_t {
    completed, unadmitted_profile, unadmitted_target, child_noncompletion
};
class NativeLuaVariantSlot0Frame;

// Bind a genuine captured receiver and its existing typed/nested/provider
// domain. The span must come from the SAME retained verified RO owner through
// data_at(D0E6F4,4). Constructor checks metadata only; numerical equality is
// not mapping, producer, C++ type-birth or current-profile authority.
class NativeLuaVariantSlot0Context final {
public:
    NativeLuaVariantSlot0Context(void* actual_receiver20,
        NativeStringRawPoolContext&, NativeLuaVariantRetirementOperation&,
        const NativeLuaVariantCppCleanupBacking&,
        NativeLuaVariantSlot0ProfileSpan actual_d0e6f4_slot0);
    NativeLuaVariantSlot0Context(const NativeLuaVariantSlot0Context&) = delete;
    NativeLuaVariantSlot0Context& operator=(const NativeLuaVariantSlot0Context&) = delete;
    NativeLuaVariantSlot0Context(NativeLuaVariantSlot0Context&&) = delete;
    NativeLuaVariantSlot0Context& operator=(NativeLuaVariantSlot0Context&&) = delete;
private:
    friend NativeLuaVariantSlot0Status dispatch_native_lua_variant_slot0_flags0_qualified(
        const NativeLuaVariantSlot0Context&, NativeLuaVariantSlot0Frame&);
    void* const receiver_;
    NativeStringRawPoolContext& strings_;
    NativeLuaVariantRetirementOperation& operation_;
    const NativeLuaVariantCppCleanupBacking& cleanup_;
    const NativeLuaVariantSlot0ProfileSpan profile_;
};

// Copied diagnostics grant no route, completion, cleanup or pointer liveness.
struct NativeLuaVariantSlot0Progress {
    bool attempted{}, failed{}, stopped{}, completed{};
    bool profile_read{}, profile_admitted{}, target_read{}, target_admitted{};
    bool flags_home_stored{}, child_entered{}, child_returned{}, child_completed{};
    std::uint32_t captured_profile{}, captured_target{};
};

// Stable one-shot call attempt. HOME and child aggregate are not value-seeded;
// only the Native PUSH0 epoch/actual child return supplies their values.
// No destructor/cleanup, vector state or type-birth operation is provided.
class NativeLuaVariantSlot0Frame final {
public:
    explicit NativeLuaVariantSlot0Frame(const NativeLuaVariantSlot0Context&) noexcept;
    ~NativeLuaVariantSlot0Frame() = default;
    NativeLuaVariantSlot0Frame(const NativeLuaVariantSlot0Frame&) = delete;
    NativeLuaVariantSlot0Frame& operator=(const NativeLuaVariantSlot0Frame&) = delete;
    NativeLuaVariantSlot0Frame(NativeLuaVariantSlot0Frame&&) = delete;
    NativeLuaVariantSlot0Frame& operator=(NativeLuaVariantSlot0Frame&&) = delete;
    NativeLuaVariantSlot0Progress progress() const noexcept { return progress_; }
    bool has_completed_call() const noexcept {
        return phase_ == Phase::completed && !failed_ && child_returned_ && child_completed_;
    }
    bool has_child_result() const noexcept { return child_returned_; }
    const NativeLuaVariantScalarDeleteResult* actual_child_result() const noexcept {
        return child_returned_ ? &child_result_ : nullptr;
    }
    bool requires_attempt_retention() const noexcept {
        return phase_ == Phase::active || phase_ == Phase::stopped || phase_ == Phase::failed;
    }
private:
    friend NativeLuaVariantSlot0Status dispatch_native_lua_variant_slot0_flags0_qualified(
        const NativeLuaVariantSlot0Context&, NativeLuaVariantSlot0Frame&);
    enum class Phase { fresh, active, completed, stopped, failed };
    const NativeLuaVariantSlot0Context& context_;
    volatile std::uint32_t flags_home_;
    NativeLuaVariantScalarDeleteResult child_result_;
    Phase phase_{Phase::fresh};
    bool failed_{}, child_returned_{}, child_completed_{};
    std::uint32_t profile_bits_{}, target_bits_{};
    NativeLuaVariantSlot0Progress progress_{};
    void fail() noexcept;
    [[noreturn]] void reject(const char*);
    void require_active();
    NativeLuaVariantSlot0Status stop(NativeLuaVariantSlot0Status);
    NativeLuaVariantSlot0Status complete();
};

// Qualified6B8960..6B8966 callsite: CURRENT profile -> CURRENT slot -> live
// HOME0 -> direct existing scalar API. Unknown routes stop explicitly, never
// invoke a default or numeric function pointer. Caller retains the ALREADY
// decremented vector/count/receiver debt; this API has no vector-header input.
// Actual returned child completion is captured before Source healthy fences;
// receiver bits can dangle and no post-child payload read is permitted.
// Ordinary Source HOME/aggregate/reentry policy, not Native stack/ABI/FH3/SEH,
// whole-vector, object-birth, VM, startup or runtime authority.
NativeLuaVariantSlot0Status dispatch_native_lua_variant_slot0_flags0_qualified(
    const NativeLuaVariantSlot0Context&, NativeLuaVariantSlot0Frame&);
} // namespace bsp
