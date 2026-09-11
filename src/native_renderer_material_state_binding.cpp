#include "bsp/native_renderer_material_state_binding.hpp"
#include "bsp/native_renderer_cached_states.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material-state binding requires MSVC Win32.
#endif

// Windows maps these names to compiler intrinsics; the native bodies call the
// actual stdcall imports, so retain those concrete import edges here.
#undef InterlockedIncrement
#undef InterlockedDecrement
extern "C" __declspec(dllimport) LONG WINAPI InterlockedIncrement(volatile LONG*);
extern "C" __declspec(dllimport) LONG WINAPI InterlockedDecrement(volatile LONG*);

namespace bsp {
namespace {
using Context = NativeRendererMaterialStateBindingContext;
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Context) == 12);

Word word(const volatile void* base, Word byte_offset) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
const volatile Word* profile(Word identity, const Context& context) noexcept {
    if (identity == 0x00d61a2c) return context.actual_render_profile_00d61a2c;
    __assume(identity == 0x00d61a34);
    return context.actual_sampler_profile_00d61a34;
}

// Original current slot0 call, followed by the full reached BD30E0 action.
// The object profile is loaded again AFTER the first slot read, not cached
// across that edge. Actual deleting providers retain their throwing contract.
void __fastcall release_current_state(void* actual_owner, Word captured_profile,
    const Context* context) {
    const Word zero_terminal = word(profile(captured_profile, *context), 0);
    __assume(zero_terminal == 0x00bd30e0);
    if (actual_owner == nullptr) return; // Native BD30E0 receiver test.
    const Word current_profile = word(actual_owner, 0);
    const Word deleting_terminal = word(profile(current_profile, *context), 4);
    auto* const owner = static_cast<NativeMaterialStateOwnerStorage*>(actual_owner);
    if (current_profile == 0x00d61a2c) {
        __assume(deleting_terminal == 0x00b422f0);
        (void)delete_native_material_render_states_00b422f0(owner, 1);
    } else {
        __assume(deleting_terminal == 0x00b42310);
        (void)delete_native_material_sampler_states_00b42310(owner, 1);
    }
}
void __fastcall set_render_row(void* renderer, const Context* context,
    Word state, Word value) {
    set_native_renderer_render_state_00b24460(renderer, state, value,
        *context->actual_synchronization_0108d6dc);
}
void __fastcall set_sampler_row(void* renderer, const Context* context,
    Word sampler, Word state, Word value) {
    set_native_renderer_sampler_state_00b24610(renderer, sampler, state, value,
        *context->actual_synchronization_0108d6dc);
}
} // namespace

__declspec(naked) void __fastcall bind_native_renderer_material_render_states_00b27a80(
    void*, const NativeRendererMaterialStateBindingContext*, NativeMaterialStateOwnerStorage*) {
    __asm {
        push edx // New fixed context cell; no context dereference.
        push ebx
        push esi
        mov esi, dword ptr [esp + 10h]
        mov ebx, ecx
        cmp dword ptr [ebx + 34h], esi
        je native_00b27af5
        push edi
        mov edi, dword ptr [ebx + 34h]
        cmp edi, esi
        je native_00b27ac0
        test esi, esi
        mov dword ptr [ebx + 34h], esi
        je native_00b27aa6
        lea eax, [esi + 4]
        push eax
        call dword ptr [InterlockedIncrement]
    native_00b27aa6:
        test edi, edi
        je native_00b27ac0
        lea ecx, [edi + 4]
        push ecx
        call dword ptr [InterlockedDecrement]
        test eax, eax
        jne native_00b27ac0
        mov edx, dword ptr [edi]
        push dword ptr [esp + 0ch] // Context for reached slot0 bridge.
        mov ecx, edi
        call release_current_state
    native_00b27ac0:
        test esi, esi
        je native_00b27aed
        xor edi, edi
        cmp dword ptr [esi + 0ch], edi
        jle native_00b27aed
        jmp native_00b27ad0
        _emit 0x8d
        _emit 0x49
        _emit 0x00
    native_00b27ad0:
        mov ecx, dword ptr [esi + 8]
        mov edx, dword ptr [ecx + edi*8 + 4]
        lea eax, [ecx + edi*8]
        mov eax, dword ptr [eax]
        push edx
        push eax
        mov ecx, ebx
        mov edx, dword ptr [esp + 14h]
        call set_render_row
        add edi, 1
        cmp edi, dword ptr [esi + 0ch]
        jl native_00b27ad0
    native_00b27aed:
        add dword ptr [ebx + 1b94h], 1
        pop edi
    native_00b27af5:
        pop esi
        pop ebx
        pop edx // Discard added context cell without changing flags.
        ret 4
    }
}

__declspec(naked) void __fastcall bind_native_renderer_material_sampler_states_00b27b90(
    void*, const NativeRendererMaterialStateBindingContext*, NativeMaterialStateOwnerStorage*) {
    __asm {
        push edx // New fixed context cell; no context dereference.
        push ebp
        push esi
        mov esi, dword ptr [esp + 10h]
        mov ebp, ecx
        cmp dword ptr [ebp + 3ch], esi
        je native_00b27c0d
        push edi
        mov edi, dword ptr [ebp + 3ch]
        cmp edi, esi
        je native_00b27bd0
        test esi, esi
        mov dword ptr [ebp + 3ch], esi
        je native_00b27bb6
        lea eax, [esi + 4]
        push eax
        call dword ptr [InterlockedIncrement]
    native_00b27bb6:
        test edi, edi
        je native_00b27bd0
        lea ecx, [edi + 4]
        push ecx
        call dword ptr [InterlockedDecrement]
        test eax, eax
        jne native_00b27bd0
        mov edx, dword ptr [edi]
        push dword ptr [esp + 0ch] // Context for reached slot0 bridge.
        mov ecx, edi
        call release_current_state
    native_00b27bd0:
        test esi, esi
        je native_00b27c05
        xor edi, edi
        cmp dword ptr [esi + 0ch], edi
        jle native_00b27c05
        push ebx
        xor ebx, ebx
        mov edi, edi
    native_00b27be0:
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
        call set_sampler_row
        add edi, 1
        add ebx, 0ch
        cmp edi, dword ptr [esi + 0ch]
        jl native_00b27be0
        pop ebx
    native_00b27c05:
        add dword ptr [ebp + 1b9ch], 1
        pop edi
    native_00b27c0d:
        pop esi
        pop ebp
        pop edx // Discard added context cell without changing flags.
        ret 4
    }
}

} // namespace bsp
