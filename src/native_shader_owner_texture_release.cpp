#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "bsp/native_shader_owner_texture_release.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shader-owner texture release requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

std::uint32_t word(const volatile void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
std::uint32_t count_bits(const void* owner) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, owner
        movzx eax, word ptr [eax + 38h]
        mov value, eax
    }
    return value;
}
std::uint32_t signed_count_bits(const void* owner) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, owner
        movsx eax, word ptr [eax + 38h]
        mov value, eax
    }
    return value;
}
void put_count(void* owner, std::uint32_t value) noexcept {
    __asm {
        mov eax, owner
        mov edx, value
        mov word ptr [eax + 38h], dx
    }
}
void zero_slot(void* owner, std::uint32_t byte_offset) noexcept {
    __asm {
        mov eax, owner
        mov edx, byte_offset
        mov dword ptr [eax + edx], 0
    }
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
volatile LONG* references(void* owner) noexcept {
    return reinterpret_cast<volatile LONG*>(reinterpret_cast<std::uintptr_t>(owner) + 4u);
}
const volatile std::uint32_t* current_texture_table(const void* owner,
    const NativeRendererTextureBindingProfiles& profiles) noexcept {
    const auto current_profile = word(owner);
    if (current_profile == 0x00d61948) return profiles.actual_texture_2d_00d61948;
    if (current_profile == 0x00d61870) return profiles.actual_cube_00d61870;
    if (current_profile == 0x00d618b0) return profiles.actual_volume_00d618b0;
    __assume(0);
}
void release_at_zero(void* captured, NativeRendererTextureBindingContext& context) {
    const auto invoker = word(current_texture_table(captured, context.actual_profiles));
    __assume(invoker == 0x00bd30e0);
    // Complete BD30E0 rereads the captured nonnull owner's CURRENT table.
    const auto terminal = word(current_texture_table(captured, context.actual_profiles), 4);
    if (terminal == 0x00b3f590) {
        delete_native_texture_2d_00b3f590(captured, 1, context.actual_texture_2d_owner);
        return;
    }
    if (terminal == 0x00b3f410) {
        delete_native_cube_texture_00b3f410(captured, 1, context.actual_cube_owner);
        return;
    }
    if (terminal == 0x00b3f430) {
        delete_native_volume_texture_00b3f430(captured, 1, context.actual_volume_owner);
        return;
    }
    __assume(0);
}
} // namespace

void unload_native_shader_owner_textures_00b188a0(
    void* owner, NativeRendererTextureBindingContext& context) {
    std::uint32_t index = 0;
    if (count_bits(owner) == 0) return;
    do {
        if (index >= signed_count_bits(owner)) put_count(owner, index + 1u);
        const std::uint32_t slot = 0xcu + index * 4u;
        void* const captured = pointer(word(owner, slot));
        if (captured) {
            if (InterlockedDecrement(references(captured)) == 0)
                release_at_zero(captured, context);
            zero_slot(owner, slot);
        }
        const auto current_count = signed_count_bits(owner);
        ++index;
        if (index >= current_count) return;
    } while (true);
}

void unload_native_renderer_shader_owner_textures_00b24e20(
    void* renderer, NativeRendererTextureBindingContext& context) {
    const auto initial_count = word(renderer, 0x1aa0);
    std::uint32_t cursor = word(renderer, 0x1a9c);
    if (cursor == cursor + initial_count * 0x2cu) return;
    do {
        void* const owner = pointer(word(pointer(cursor), 0x28));
        unload_native_shader_owner_textures_00b188a0(owner, context);
        const auto current_count = word(renderer, 0x1aa0);
        const auto current_base = word(renderer, 0x1a9c);
        const auto current_end = current_base + current_count * 0x2cu;
        cursor += 0x2cu;
        if (cursor == current_end) return;
    } while (true);
}

} // namespace bsp
