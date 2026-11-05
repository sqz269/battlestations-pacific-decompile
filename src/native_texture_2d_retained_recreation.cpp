#include "bsp/native_texture_2d_retained_recreation.hpp"
#include "bsp/native_memory_stream.hpp"
#include "bsp/native_shader_device_reset.hpp"
#include "bsp/native_tagged_leaf_b3_03.hpp"
#include "bsp/native_tagged_leaf_b3_05.hpp"
#include "bsp/wire_goal4_chain574_switches.hpp"
#include "bsp/wire_goal4_chain586_switches.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native retained texture recreation requires MSVC Win32.
#endif

namespace bsp {
namespace {

void* address(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
__forceinline std::uint32_t word(const volatile void* location) noexcept {
    std::uint32_t value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(value);
}

} // namespace

void release_native_texture_2d_retained_00b3d7b0(void* owner,
    const NativeTexture2DRetainedRecreationContext& context) {
    const auto current_profile = word(owner);
    __assume(current_profile == 0x00d61948u);
    const auto current_method = word(context.actual_texture_profile_00d61948 + 8);
    __assume(current_method == 0x00b3dd30u);
    release_native_texture_2d_for_reset_00b3dd30(owner, context.reset_surfaces);
}

void recreate_native_texture_2d_retained_00b3e190(void* owner,
    const NativeTexture2DRetainedRecreationContext& context) {
    const void* const renderer = context.actual_renderer_00f8d394;
    auto* const device = static_cast<IDirect3DDevice9*>(
        get_native_renderer_device_00b1fef0(renderer));
    // 00B3E190 consumes this SAME retained texture owner's format at +18.
    // 00B33E60 is the complete DWORD getter; keep the original 14h->15h map.
    const auto format = [&] {
        if constexpr (kWireGoal4Chain574Bound) {
            return native_tagged_b3_read_dword_18_00b33e60(owner) == 0x14u ? 0x15u : 0u;
        } else {
            return word(address(owner, 0x18)) == 0x14u ? 0x15u : 0u;
        }
    }();
    void* const length_stream = pointer(word(address(owner, 0x4c)));
    auto** const output = reinterpret_cast<IDirect3DTexture9**>(address(owner, 0x10));
    // B3E190's retained-image path reads this actual owner's +2C, +3C,
    // then +28. The B3D0B0/B3D0A0 raw DWORD leaves preserve those loads;
    // they assert no class identity (bsp.py show <address> --asm).
    const auto height = [&] {
        if constexpr (kWireGoal4Chain586Bound) {
            return native_tagged_b3_read_dword_2c_00b3d0b0(owner);
        } else {
            return word(address(owner, 0x2c));
        }
    }();
    const auto mip_levels = word(address(owner, 0x3c));
    const auto width = [&] {
        if constexpr (kWireGoal4Chain586Bound) {
            return native_tagged_b3_read_dword_28_00b3d0a0(owner);
        } else {
            return word(address(owner, 0x28));
        }
    }();
    const auto length = static_cast<UINT>(
        dispatch_native_memory_stream_length(length_stream, context.retained_memory));
    void* const data_stream = pointer(word(address(owner, 0x4c)));
    const auto* const data = native_memory_stream_data_00bef610(data_stream, nullptr);
    (void)context.actual_d3dx_create_texture_from_memory_ex(device, data, length,
        width, height, mip_levels, 0, static_cast<D3DFORMAT>(format),
        D3DPOOL_MANAGED, 0x00070004u, 0xffffffffu, 0, nullptr, nullptr, output);
}

} // namespace bsp
