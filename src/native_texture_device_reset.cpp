#include "bsp/native_texture_device_reset.hpp"
#include "bsp/native_surface_owner.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture device reset requires MSVC Win32.
#endif

namespace bsp {
namespace {

void* address(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
__forceinline std::uint32_t read_word(const void* location) noexcept {
    std::uint32_t value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
__forceinline void write_word(void* location, std::uint32_t value) noexcept {
    __asm { mov eax, location }
    __asm { mov edx, value }
    __asm { mov dword ptr [eax], edx }
}
void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(value);
}
const void* current_surface_table(const void* owner,
    const NativeTextureResetSurfaceProfile& profile) noexcept {
    const auto current_profile = read_word(owner);
    __assume(current_profile == 0x00d619a0u);
    return const_cast<const std::uint32_t*>(profile.actual_surface_profile_00d619a0);
}

using ComReference = std::uint32_t (__stdcall*)(void*);
using CreateTexture = std::int32_t (__stdcall*)(void*, std::uint32_t,
    std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t,
    std::uint32_t, void*, void*);
using GetSurfaceLevel = std::int32_t (__stdcall*)(void*, std::uint32_t, void*);

ComReference current_reference_call(void* object, std::uint32_t offset) noexcept {
    const auto* const table = pointer(read_word(object));
    return reinterpret_cast<ComReference>(read_word(address(table, offset)));
}

} // namespace

void release_native_texture_2d_for_reset_00b3dd30(
    void* owner, const NativeTextureResetSurfaceProfile& profile) {
    std::uint32_t index = 0;
    if (static_cast<std::int32_t>(read_word(address(owner, 0x44))) > 0) {
        do {
            const auto* const records = pointer(read_word(address(owner, 0x40)));
            auto* const surface_owner = pointer(read_word(address(records, index * 8u + 4u)));
            const auto* const table = current_surface_table(surface_owner, profile);
            const auto method = read_word(address(table, 0x3c));
            __assume(method == 0x00b3d510u);
            release_native_surface_for_reset_00b3d510(
                *static_cast<NativeSurfaceOwnerStorage*>(surface_owner));
            index += 1u;
        } while (static_cast<std::int32_t>(index) <
                 static_cast<std::int32_t>(read_word(address(owner, 0x44))));
    }
    void* const captured_texture = pointer(read_word(address(owner, 0x10)));
    if (captured_texture) {
        current_reference_call(captured_texture, 4)(captured_texture);
        current_reference_call(captured_texture, 8)(captured_texture);
    }
    void* const current_texture = pointer(read_word(address(owner, 0x10)));
    if (current_texture) {
        current_reference_call(current_texture, 8)(current_texture);
        write_word(address(owner, 0x10), 0);
    }
}

void restore_native_texture_2d_after_reset_00b3dd90(void* owner,
    void* device_argument_and_texture_output, void* reused_surface_output,
    const NativeTextureResetSurfaceProfile& profile) {
    // Native PUSH ECX seeds S-4 before its first owner read, on every path.
    write_word(reused_surface_output, reinterpret_cast<std::uintptr_t>(owner));
    const auto flags = read_word(address(owner, 0x1c));
    const auto pool_code = flags & 0x0fu;
    const auto pool = pool_code <= 3u ? pool_code : read_word(device_argument_and_texture_output);
    std::uint32_t usage = (flags & 0x10u) != 0 ? 1u : 0u;
    switch (flags & 0x0f00u) {
    case 0x0100u: usage |= 0x0002u; break;
    case 0x0200u: usage |= 0x4000u; break;
    case 0x0300u: usage |= 0x0040u; break;
    case 0x0400u: usage |= 0x0100u; break;
    case 0x0500u: usage |= 0x0080u; break;
    }
    if ((flags & 0x0f000u) == 0x01000u) usage |= 0x0200u;
    if ((flags & 0xff000000u) == 0x01000000u) usage |= 0x0400u;

    void* const captured_device = pointer(read_word(device_argument_and_texture_output));
    const auto* const device_table = pointer(read_word(captured_device));
    const auto create = reinterpret_cast<CreateTexture>(read_word(address(device_table, 0x5c)));
    const auto format = read_word(address(owner, 0x18));
    const auto levels = read_word(address(owner, 0x14));
    const auto height = read_word(address(owner, 0x2c));
    const auto width = read_word(address(owner, 0x28));
    (void)create(captured_device, width, height, levels, usage, format, pool,
        device_argument_and_texture_output, nullptr);

    void* const captured_old = pointer(read_word(address(owner, 0x10)));
    auto temporary = read_word(device_argument_and_texture_output);
    if (reinterpret_cast<std::uintptr_t>(captured_old) != temporary) {
        write_word(address(owner, 0x10), temporary);
        if (temporary != 0) {
            void* const captured_new = pointer(temporary);
            current_reference_call(captured_new, 4)(captured_new);
            temporary = read_word(device_argument_and_texture_output);
        }
        if (captured_old) {
            current_reference_call(captured_old, 8)(captured_old);
            temporary = read_word(device_argument_and_texture_output);
        }
    }
    if (temporary != 0) {
        void* const current_temporary = pointer(temporary);
        current_reference_call(current_temporary, 8)(current_temporary);
        write_word(device_argument_and_texture_output, 0);
    }

    std::uint32_t index = 0;
    if (static_cast<std::int32_t>(read_word(address(owner, 0x44))) > 0) {
        do {
            void* const texture = pointer(read_word(address(owner, 0x10)));
            const auto* const texture_table = pointer(read_word(texture));
            const auto* const records = pointer(read_word(address(owner, 0x40)));
            const auto level = read_word(address(records, index * 8u));
            const auto get_surface = reinterpret_cast<GetSurfaceLevel>(
                read_word(address(texture_table, 0x48)));
            (void)get_surface(texture, level, reused_surface_output);

            const auto* const current_records = pointer(read_word(address(owner, 0x40)));
            auto* const surface_owner = pointer(read_word(address(current_records, index * 8u + 4u)));
            const auto* const surface_table = current_surface_table(surface_owner, profile);
            auto* const surface = static_cast<IDirect3DSurface9*>(pointer(read_word(reused_surface_output)));
            const auto bind = read_word(address(surface_table, 0x14));
            __assume(bind == 0x00b3cc80u);
            bind_native_surface_00b3cc80(*static_cast<NativeSurfaceOwnerStorage*>(surface_owner), surface);

            void* const current_output = pointer(read_word(reused_surface_output));
            current_reference_call(current_output, 8)(current_output);
            index += 1u;
        } while (static_cast<std::int32_t>(index) <
                 static_cast<std::int32_t>(read_word(address(owner, 0x44))));
    }
}

void native_texture_reset_noop_00b33f10(void*) noexcept {}
void native_texture_restore_noop_00b33f20(void*, void*) noexcept {}

} // namespace bsp
