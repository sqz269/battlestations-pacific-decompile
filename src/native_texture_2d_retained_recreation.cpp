#include "bsp/native_texture_2d_retained_recreation.hpp"
#include "bsp/native_memory_stream.hpp"
#include "bsp/native_shader_device_reset.hpp"

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
    const auto format = word(address(owner, 0x18)) == 0x14u ? 0x15u : 0u;
    void* const length_stream = pointer(word(address(owner, 0x4c)));
    auto** const output = reinterpret_cast<IDirect3DTexture9**>(address(owner, 0x10));
    const auto height = word(address(owner, 0x2c));
    const auto mip_levels = word(address(owner, 0x3c));
    const auto width = word(address(owner, 0x28));
    const auto length = static_cast<UINT>(
        dispatch_native_memory_stream_length(length_stream, context.retained_memory));
    void* const data_stream = pointer(word(address(owner, 0x4c)));
    const auto* const data = native_memory_stream_data_00bef610(data_stream, nullptr);
    (void)context.actual_d3dx_create_texture_from_memory_ex(device, data, length,
        width, height, mip_levels, 0, static_cast<D3DFORMAT>(format),
        D3DPOOL_MANAGED, 0x00070004u, 0xffffffffu, 0, nullptr, nullptr, output);
}

} // namespace bsp
