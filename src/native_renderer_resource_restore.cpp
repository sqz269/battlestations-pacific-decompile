#include "bsp/native_renderer_resource_restore.hpp"
#include "bsp/native_occlusion_query_device_reset.hpp"
#include "bsp/native_surface_owner.hpp"
#include "bsp/native_texture_device_reset.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer resource restoration requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = std::uint32_t;
void* at(const volatile void* base, Word byte_offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + byte_offset);
}
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
std::uint8_t byte(const void* base, Word byte_offset) noexcept {
    std::uint8_t value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov al, byte ptr [eax + edx]
        mov value, al
    }
    return value;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void ready(void* renderer) noexcept {
    __asm {
        mov eax, renderer
        mov byte ptr [eax + 0x1d8b], 1
    }
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
std::int32_t signed_bits(Word bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, 4);
    return result;
}
const volatile Word* texture_table(void* owner,
    const NativeRendererResourceRestoreProfiles& profiles) noexcept {
    const auto token = word(owner);
    if (token == 0x00d61948u) return profiles.texture_2d_00d61948;
    if (token == 0x00d61870u) return profiles.cube_00d61870;
    if (token == 0x00d618b0u) return profiles.volume_00d618b0;
    __assume(0); // Other profiles require an established native binding.
}
const volatile Word* surface_table(void* owner,
    const NativeRendererResourceRestoreProfiles& profiles) noexcept {
    const auto token = word(owner);
    __assume(token == 0x00d619a0u);
    return profiles.surface_00d619a0;
}
void bind_default(void* renderer, Word owner_offset, const void* output,
    const NativeRendererResourceRestoreProfiles& profiles) {
    auto* const owner = pointer(word(renderer, owner_offset));
    const auto* const table = surface_table(owner, profiles);
    auto* const surface = static_cast<IDirect3DSurface9*>(pointer(word(output)));
    const auto bind = word(table, 0x14);
    __assume(bind == 0x00b3cc80u);
    bind_native_surface_00b3cc80(*static_cast<NativeSurfaceOwnerStorage*>(owner), surface);
}
using ComReference = Word (__stdcall*)(void*);
using GetRenderTarget = std::int32_t (__stdcall*)(void*, Word, void*);
using GetDepthSurface = std::int32_t (__stdcall*)(void*, void*);
void release_output(const void* output) {
    auto* const com = pointer(word(output));
    const auto* const table = pointer(word(com));
    const auto release = reinterpret_cast<ComReference>(word(table, 8));
    (void)release(com); // Native unconditionally dereferences this fresh output.
}
} // namespace

void restore_native_renderer_resources_00b23b10(void* renderer,
    NativeRendererResourceRestoreContext& context) {
    Word default_output;
    put(&default_output, 0, reinterpret_cast<Word>(renderer)); // Entry PUSH ECX.
    if (byte(renderer, 0x1d8b) != 0) return;
    if (byte(renderer, 0x1d8a) != 0) return;

    auto* const first_device = pointer(word(renderer, 0x1a10));
    ready(renderer);
    put(&default_output, 0, 0);
    const auto* const first_table = pointer(word(first_device));
    const auto get_color = reinterpret_cast<GetRenderTarget>(word(first_table, 0x98));
    (void)get_color(first_device, 0, &default_output);
    bind_default(renderer, 0x197c, &default_output, context.actual_profiles);
    release_output(&default_output);

    auto* const depth_device = pointer(word(renderer, 0x1a10));
    put(&default_output, 0, 0);
    const auto* const depth_table = pointer(word(depth_device));
    const auto get_depth = reinterpret_cast<GetDepthSurface>(word(depth_table, 0xa0));
    (void)get_depth(depth_device, &default_output);
    bind_default(renderer, 0x198c, &default_output, context.actual_profiles);
    release_output(&default_output);

    const NativeTextureResetSurfaceProfile nested_surfaces{
        context.actual_profiles.surface_00d619a0};
    Word texture_scratch[3]; // Two genuine callback cells separated by eight bytes.
    Word cursor = word(renderer, 0x1b00);
    const auto initial_texture_count = word(renderer, 0x1b04);
    if (cursor != cursor + initial_texture_count * 4u) {
        do {
            auto* const owner = pointer(word(pointer(cursor)));
            const auto* const table = texture_table(owner, context.actual_profiles);
            auto* const device = pointer(word(renderer, 0x1a10));
            const auto restore = word(table, 0x24);
            put(&texture_scratch[2], 0, reinterpret_cast<Word>(device));
            if (restore == 0x00b3dd90u) {
                restore_native_texture_2d_after_reset_00b3dd90(owner,
                    &texture_scratch[2], &texture_scratch[0], nested_surfaces);
            } else {
                __assume(restore == 0x00b33f20u);
                native_texture_restore_noop_00b33f20(owner, device);
            }
            const auto count = word(renderer, 0x1b04);
            const auto base = word(renderer, 0x1b00);
            cursor += 4u;
            if (cursor == base + count * 4u) break;
        } while (true);
    }

    cursor = word(renderer, 0x1b0c);
    const auto initial_surface_count = word(renderer, 0x1b10);
    if (cursor != cursor + initial_surface_count * 4u) {
        do {
            auto* const owner = pointer(word(pointer(cursor)));
            const auto* const table = surface_table(owner, context.actual_profiles);
            auto* const device = static_cast<IDirect3DDevice9*>(pointer(word(renderer, 0x1a10)));
            const auto restore = word(table, 0x40);
            __assume(restore == 0x00b3d550u);
            (void)recreate_native_surface_00b3d550(
                *static_cast<NativeSurfaceOwnerStorage*>(owner), *device);
            const auto count = word(renderer, 0x1b10);
            const auto base = word(renderer, 0x1b0c);
            cursor += 4u;
            if (cursor == base + count * 4u) break;
        } while (true);
    }

    Word index = 0;
    if (signed_bits(word(renderer, 0x19a4)) > 0) {
        do {
            const auto base = word(renderer, 0x19a0);
            auto* const owner = pointer(word(at(pointer(base), index * 4u)));
            const auto profile = word(owner);
            __assume(profile == 0x00d62ad0u);
            const auto restore = word(context.actual_profiles.query_00d62ad0, 0x1c);
            __assume(restore == 0x00b5fe60u);
            restore_native_occlusion_query_after_reset_00b5fe60(
                owner, context.actual_renderer_publication_00f8d394);
            ++index;
        } while (signed_bits(index) < signed_bits(word(renderer, 0x19a4)));
    }
}
} // namespace bsp
