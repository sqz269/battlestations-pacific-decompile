#include "bsp/native_lua_variant_slot0.hpp"

#include <cstring>
#include <intrin.h>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4);
std::uint32_t current_word(const void* storage) noexcept {
    std::uint32_t value;
    _ReadWriteBarrier();
    std::memcpy(&value, storage, sizeof value);
    _ReadWriteBarrier();
    return value;
}
} // namespace

NativeLuaVariantSlot0Context::NativeLuaVariantSlot0Context(void* receiver,
    NativeStringRawPoolContext& strings, NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup, NativeLuaVariantSlot0ProfileSpan profile)
    : receiver_(receiver), strings_(strings), operation_(operation), cleanup_(cleanup),
      profile_(profile) {
    // Span identity/extent only: no receiver/table/tag/pool payload read or birth.
    if (!receiver || profile.original_address != 0x00d0e6f4u || profile.bytes != 4u ||
        reinterpret_cast<std::uintptr_t>(profile.data) != 0x00d0e6f4u)
        throw std::invalid_argument("slot0 requires actual receiver and D0E6F4/4 metadata");
}
NativeLuaVariantSlot0Frame::NativeLuaVariantSlot0Frame(
    const NativeLuaVariantSlot0Context& context) noexcept : context_(context) {}
void NativeLuaVariantSlot0Frame::fail() noexcept {
    failed_ = true;
    phase_ = Phase::failed;
    progress_.failed = true;
}
[[noreturn]] void NativeLuaVariantSlot0Frame::reject(const char* text) {
    fail();
    throw std::logic_error(text);
}
void NativeLuaVariantSlot0Frame::require_active() {
    if (phase_ != Phase::active || failed_)
        reject("slot0 invocation is not healthy and active");
}
NativeLuaVariantSlot0Status NativeLuaVariantSlot0Frame::stop(NativeLuaVariantSlot0Status status) {
    require_active();
    phase_ = Phase::stopped;
    progress_.stopped = true;
    return status;
}
NativeLuaVariantSlot0Status NativeLuaVariantSlot0Frame::complete() {
    require_active();
    phase_ = Phase::completed;
    progress_.completed = true;
    return NativeLuaVariantSlot0Status::completed;
}

NativeLuaVariantSlot0Status dispatch_native_lua_variant_slot0_flags0_qualified(
    const NativeLuaVariantSlot0Context& c, NativeLuaVariantSlot0Frame& f) {
    // An inner rejected invocation cannot act on the admitted outer call/debt.
    if (&f.context_ != &c || f.phase_ != NativeLuaVariantSlot0Frame::Phase::fresh)
        f.reject("slot0 Frame mismatch, reentry or replay");
    f.phase_ = NativeLuaVariantSlot0Frame::Phase::active;
    f.progress_.attempted = true;
    try {
        f.profile_bits_ = current_word(c.receiver_);
        f.progress_.profile_read = true;
        f.progress_.captured_profile = f.profile_bits_;
        if (f.profile_bits_ != c.profile_.original_address)
            return f.stop(NativeLuaVariantSlot0Status::unadmitted_profile);
        f.progress_.profile_admitted = true;

        // This is the CURRENT data-table address, never a Source vtable/code cast.
        const auto* const actual_table = reinterpret_cast<const void*>(
            static_cast<std::uintptr_t>(f.profile_bits_));
        f.target_bits_ = current_word(actual_table);
        f.progress_.target_read = true;
        f.progress_.captured_target = f.target_bits_;
        if (f.target_bits_ != 0x00884fa0u)
            return f.stop(NativeLuaVariantSlot0Status::unadmitted_target);
        f.progress_.target_admitted = true;

        // Native PUSH0 follows BOTH actual reads. Child reads this live HOME late.
        _ReadWriteBarrier();
        f.flags_home_ = 0u;
        _ReadWriteBarrier();
        f.progress_.flags_home_stored = true;
        f.progress_.child_entered = true;
        f.child_result_ = scalar_delete_native_lua_variant_00884fa0_cpp_cleanup(
            c.receiver_, f.flags_home_, c.strings_, c.operation_, c.cleanup_);
        f.child_returned_ = true;
        f.child_completed_ = f.child_result_.completion == NativeLuaVariantRetirementResult::completed;
        f.progress_.child_returned = true;
        f.progress_.child_completed = f.child_completed_;
        // Actual aggregate/disposition is retained BEFORE this Source fence.
        f.require_active();
        if (!f.child_completed_)
            return f.stop(NativeLuaVariantSlot0Status::child_noncompletion);
        return f.complete();
    } catch (...) {
        f.fail();
        throw; // no count repair, second retirement, cleanup or terminal policy
    }
}
} // namespace bsp
