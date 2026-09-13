#include "bsp/native_renderer_cache_clear.hpp"
#include "bsp/native_renderer_texture_stage_state.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer cache clear requires MSVC Win32.
#endif

#undef InterlockedIncrement
#undef InterlockedDecrement
extern "C" __declspec(dllimport) LONG WINAPI InterlockedIncrement(volatile LONG*);
extern "C" __declspec(dllimport) LONG WINAPI InterlockedDecrement(volatile LONG*);

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void put(void* base, Word byte_offset, Word value = 0) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
void put_byte(void* base, Word byte_offset) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov byte ptr [eax + edx], 0 }
}
void put_short(void* base, Word byte_offset) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov word ptr [eax + edx], 0 }
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* at(void* base, Word offset) noexcept {
    return pointer(reinterpret_cast<Word>(base) + offset);
}
const volatile Word* material_profile(Word identity,
    const NativeRendererThirdStateBindingContext& context) noexcept {
    if (identity == 0x00d61a2c)
        return context.actual_material_states.actual_render_profile_00d61a2c;
    if (identity == 0x00d61a34)
        return context.actual_material_states.actual_sampler_profile_00d61a34;
    __assume(identity == 0x00d61a3c);
    return context.actual_third_profile_00d61a3c;
}
void delete_material(void* owner, Word terminal) {
    auto* const actual = static_cast<NativeMaterialStateOwnerStorage*>(owner);
    if (terminal == 0x00b422f0)
        (void)delete_native_material_render_states_00b422f0(actual, 1);
    else if (terminal == 0x00b42310)
        (void)delete_native_material_sampler_states_00b42310(actual, 1);
    else {
        __assume(terminal == 0x00b42330);
        (void)delete_native_material_third_states_00b42330(actual, 1);
    }
}
void __fastcall release_current_material(void* owner, Word captured_profile,
    const NativeRendererThirdStateBindingContext* context) {
    const auto invoker = word(material_profile(captured_profile, *context));
    __assume(invoker == 0x00bd30e0);
    if (owner == nullptr) return; // Full BD30E0's receiver test.
    const auto terminal = word(material_profile(word(owner), *context), 4);
    delete_material(owner, terminal);
}
const volatile Word* owner_profile(Word identity, NativeRendererCacheClearContext& context) noexcept {
    switch (identity) {
    case 0x00d61a2c: case 0x00d61a34: case 0x00d61a3c:
        return material_profile(identity, context.actual_material_states);
    case 0x00d62af4: return context.actual_layout.actual_layout_profile_00d62af4;
    case 0x00d61de0: return context.actual_index.actual_logical_profile_00d61de0;
    case 0x00d61d6c: return context.actual_vertex.actual_logical_owner.actual_logical_profile_00d61d6c;
    case 0x00d61948: return context.actual_texture.actual_profiles.actual_texture_2d_00d61948;
    case 0x00d61870: return context.actual_texture.actual_profiles.actual_cube_00d61870;
    case 0x00d618b0: return context.actual_texture.actual_profiles.actual_volume_00d618b0;
    case 0x00d5e600: return context.actual_frame_profile_00d5e600;
    default: __assume(0);
    }
}
void release_at_zero(void* owner, NativeRendererCacheClearContext& context) {
    const auto invoker = word(owner_profile(word(owner), context));
    __assume(invoker == 0x00bd30e0);
    if (owner == nullptr) return; // Full BD30E0, without a second decrement.
    const auto terminal = word(owner_profile(word(owner), context), 4);
    switch (terminal) {
    case 0x00b422f0: case 0x00b42310: case 0x00b42330:
        delete_material(owner, terminal); return;
    case 0x00b60770:
        (void)delete_native_hardware_layout_00b60770(owner, 1,
            context.actual_layout.actual_hardware_layout_owner); return;
    case 0x00b4c1f0:
        (void)delete_native_pooled_logical_index_stream_00b4c1f0(owner, 1,
            context.actual_index.actual_logical_owner); return;
    case 0x00b4bf10:
        (void)delete_native_logical_vertex_stream_00b4bf10(owner, 1,
            context.actual_vertex.actual_logical_owner); return;
    case 0x00b3f590:
        (void)delete_native_texture_2d_00b3f590(owner, 1,
            context.actual_texture.actual_texture_2d_owner); return;
    case 0x00b3f410:
        (void)delete_native_cube_texture_00b3f410(owner, 1,
            context.actual_texture.actual_cube_owner); return;
    case 0x00b3f430:
        (void)delete_native_volume_texture_00b3f430(owner, 1,
            context.actual_texture.actual_volume_owner); return;
    case 0x00b1fcf0:
        (void)delete_native_frame_target_owner_00b1fcf0(
            *static_cast<NativeFrameTargetOwnerStorage*>(owner), 1,
            context.actual_frame_target); return;
    default: __assume(0);
    }
}
void release_then_clear(void* cache, Word offset, NativeRendererCacheClearContext& context) {
    void* const captured = pointer(word(cache, offset));
    if (captured) {
        if (InterlockedDecrement(static_cast<volatile LONG*>(at(captured, 4))) == 0)
            release_at_zero(captured, context);
        put(cache, offset);
    }
}
void __fastcall set_third_row(void* renderer,
    const NativeRendererThirdStateBindingContext* context, Word stage, Word state, Word value) {
    set_native_renderer_texture_stage_state_00b24510(renderer, stage, state, value,
        *context->actual_material_states.actual_synchronization_0108d6dc);
}
} // namespace

void clear_native_renderer_binding_cache_00b241c0(void* cache,
    NativeRendererCacheClearContext& context) {
    release_then_clear(cache, 0, context);
    release_then_clear(cache, 4, context);
    release_then_clear(cache, 8, context);
    std::memset(at(cache, 0xc), 0, 0xd2);
    put(cache, 0x1738);
    put(cache, 0x173c);
    release_then_clear(cache, 0x1780, context);
    release_then_clear(cache, 0x1784, context);
    put(cache, 0x1788, 0xffffffffu);
    for (Word offset = 0x1740; offset != 0x1780; offset += 0x10) {
        release_then_clear(cache, offset, context);
        put(cache, offset + 4);
        put(cache, offset + 8);
        put(cache, offset + 0xc);
    }
    for (Word bank = 0; bank != 16; ++bank) {
        const Word owner_offset = 0x4d0 + bank * 0xac;
        for (Word offset = 0; offset != 0x20; offset += 4)
            put(cache, owner_offset - 0xa8 + offset);
        put_byte(cache, owner_offset - 0x88);
        release_then_clear(cache, owner_offset, context);
    }
    for (Word bank = 0; bank != 20; ++bank) {
        const Word offset = 0x1198 + bank * 0x48;
        put(cache, offset);
        put(cache, offset + 4);
        put(cache, offset + 8);
        put_short(cache, offset + 0xc);
    }
    release_then_clear(cache, 0x18d4, context);
    put(cache, 0x18d8);
    put(cache, 0x18e8);
    put(cache, 0x18dc);
    put(cache, 0x18e0);
    put(cache, 0x18e4);
    for (Word offset = 0x18ec; offset != 0x1938; offset += 4) put(cache, offset);
}

__declspec(naked) void __fastcall bind_native_renderer_material_third_states_00b27b00(
    void*, const NativeRendererThirdStateBindingContext*, NativeMaterialStateOwnerStorage*) {
    __asm {
        push edx // Added fixed context; unchanged original incoming identity.
        push ebp
        push esi
        mov esi, dword ptr [esp + 10h]
        mov ebp, ecx
        cmp dword ptr [ebp + 38h], esi
        je native_00b27b7d
        push edi
        mov edi, dword ptr [ebp + 38h]
        cmp edi, esi
        je native_00b27b40
        test esi, esi
        mov dword ptr [ebp + 38h], esi
        je native_00b27b26
        lea eax, [esi + 4]
        push eax
        call dword ptr [InterlockedIncrement]
    native_00b27b26:
        test edi, edi
        je native_00b27b40
        lea ecx, [edi + 4]
        push ecx
        call dword ptr [InterlockedDecrement]
        test eax, eax
        jne native_00b27b40
        mov edx, dword ptr [edi]
        push dword ptr [esp + 0ch]
        mov ecx, edi
        call release_current_material
    native_00b27b40:
        test esi, esi
        je native_00b27b75
        xor edi, edi
        cmp dword ptr [esi + 0ch], edi
        jle native_00b27b75
        push ebx
        xor ebx, ebx
        mov edi, edi
    native_00b27b50:
        mov ecx, dword ptr [esi + 8]
        mov edx, dword ptr [ecx + ebx + 8]
        lea eax, [ecx + ebx]
        mov ecx, dword ptr [eax + 4]
        push edx
        mov edx, dword ptr [eax]
        push ecx
        push edx
        mov ecx, ebp
        mov edx, dword ptr [esp + 1ch]
        call set_third_row
        add edi, 1
        add ebx, 0ch
        cmp edi, dword ptr [esi + 0ch]
        jl native_00b27b50
        pop ebx
    native_00b27b75:
        add dword ptr [ebp + 1b98h], 1
        pop edi
    native_00b27b7d:
        pop esi
        pop ebp
        pop edx
        ret 4
    }
}
} // namespace bsp
