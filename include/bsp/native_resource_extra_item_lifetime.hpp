#pragma once

#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_ref_counted.hpp"

#include <cstdint>

namespace bsp {

struct NativeStringRawPoolContext;

// Borrow the application's current D62ED4 profile and its concrete slot04
// deletion provider. AnimationChannels owns raw group pointers and calls that
// slot directly with flags1; it does not decrement the group's reference count.
struct NativeResourceExtraItemLifetimeContext {
    NativeStringRawPoolContext& strings;
    NativeRefCountedDeleteCalls& group_deletes;
    const volatile std::uint32_t* group_profile_00d62ed4;
};

// B8A680[161]: native ECX actual14h AnimationChannels item, RET. Stamp D6328C;
// delete nonnull groups from the CURRENT last cell through current D62ED4
// slot04 B78D00 with flags1; clear the captured cell only after success; then
// decrement the CURRENT count. Resize the +8 vector to zero, free its CURRENT
// data, and destroy the resource-item base. EH state1 destroys the vector then
// the base; state0 destroys only the base. No group reference decrement/retry.
void destroy_native_animation_channels_item_00b8a680(
    void* actual_item, NativeResourceExtraItemLifetimeContext&);

// B8A760[30]: native ECX item, stacked flags, EAX original item, RET4. Finish
// the entire destructor before testing low-byte bit0; optionally free through
// the canonical BF65AC boundary. A throwing destructor skips the outer free.
void* delete_native_animation_channels_item_00b8a760(
    void* actual_item, std::uint32_t flags, NativeResourceExtraItemLifetimeContext&);

// B8A8A0[97]: native ECX actual2Ch Bone item, RET. If +0C is nonnull, resolve
// the actual 00419CC0 pool and return it with size(+08+1) through BD1510.
// Leave +08/+0C untouched. Normal and exceptional exits then destroy the item
// base; the native body does not first restore D632B8 itself.
void destroy_native_bone_item_00b8a8a0(
    void* actual_item, NativeStringRawPoolContext&);

// B8AF10[30]: same scalar-wrapper contract as B8A760, for a Bone item.
void* delete_native_bone_item_00b8af10(
    void* actual_item, std::uint32_t flags, NativeStringRawPoolContext&);

// Compose the terminal reached after the caller's real +4 decrement. The two
// supplied pointers are the SAME live D6328C/D632B8 table cells. Unknown
// source/profile domains forward; a reached known domain whose current slot04
// changed is an explicit error. Group deletion remains the required concrete
// service in NativeResourceExtraItemLifetimeContext.
class NativeResourceExtraItemReferences final : public NativeAdoptedSubstreamDispatch {
public:
    NativeResourceExtraItemReferences(NativeAdoptedSubstreamDispatch&,
        NativeResourceExtraItemLifetimeContext&,
        const volatile std::uint32_t* animation_profile_00d6328c,
        const volatile std::uint32_t* bone_profile_00d632b8) noexcept;
    std::uint8_t source_is_open(std::uintptr_t, void*) override;
    std::uint32_t source_seek(std::uintptr_t, void*, std::uint32_t,
        std::uint32_t, std::uint32_t) override;
    void source_read(std::uintptr_t, void*, void*, std::uint32_t,
        std::uint32_t*) override;
    void source_write(std::uintptr_t, void*, const void*, std::uint32_t,
        std::uint32_t*) override;
    void source_zero_reference(std::uintptr_t, void*, std::uintptr_t) override;

private:
    NativeAdoptedSubstreamDispatch& other_;
    NativeResourceExtraItemLifetimeContext& context_;
    const volatile std::uint32_t* animation_profile_;
    const volatile std::uint32_t* bone_profile_;
};

// These are source interfaces over genuine native storage. They do not supply
// original callable ABI, FH3/SEH/hardware-fault identity, the B78D00 group
// destructor, arbitrary profile mutation, application wiring or gameplay proof.
} // namespace bsp
