#pragma once
#include <cstdint>

namespace bsp {
class NativeStringStorage;
struct NativeMaterialPoolStorage;

// Borrow the same actual string service as the containing effect and the
// fixed0108FEE4 state-list pool storage, without a projected allocator list.
// Each profile view is its actual immutable slot-zero DWORD. The numeric
// words are validated and never invoked as original-EXE code addresses.
struct NativeMaterialEffectDescriptorContext {
    NativeStringStorage& strings;
    NativeMaterialPoolStorage& actual_state_list_pool_0108fee4;
    const volatile std::uint32_t* actual_descriptor_profile_00d61a44;
    const volatile std::uint32_t* actual_sampler_profile_00d621f4;
};

// B41B10's captured nonnull descriptor dispatch: CURRENT profile/slot0,
// D61A44 -> B46930(flags1). No reference decrement or effect-field clear here.
void invoke_native_material_effect_descriptor_terminal(
    void* captured_descriptor, NativeMaterialEffectDescriptorContext&);
// B458A0's captured nonnull sampler dispatch: CURRENT profile/slot0,
// D621F4 -> B56FC0(flags1). The caller clears its captured slot after return.
void invoke_native_shader_sampler_terminal(
    void* captured_sampler, NativeMaterialEffectDescriptorContext&);
} // namespace bsp
