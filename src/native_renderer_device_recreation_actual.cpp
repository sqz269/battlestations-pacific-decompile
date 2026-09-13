#include "bsp/native_renderer_device_recreation_actual.hpp"
#include "bsp/native_renderer_resource_release.hpp"
#include "bsp/native_renderer_resource_restore.hpp"
#include "bsp/native_renderer_frame_targets.hpp"
#include "bsp/native_dynamic_buffer_device_release.hpp"
#include "bsp/native_hardware_layout_device_release.hpp"
#include "bsp/native_hardware_layout_create_if_missing.hpp"
#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_logical_buffer_device_save.hpp"
#include "bsp/native_logical_buffer_device_restore.hpp"
#include "bsp/native_shader_device_reset.hpp"
#include "bsp/native_renderer_cached_states.hpp"
#include "bsp/native_renderer_gamma.hpp"
#include "bsp/native_renderer_reset_readiness.hpp"
#include "bsp/native_texture_2d_retained_recreation.hpp"
#include "bsp/native_cube_volume_retained_recreation.hpp"
#include "bsp/native_xlive_device_adapter.hpp"

#include <d3d9.h>
#include <cstddef>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual renderer device recreation requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(D3DCAPS9) == 0x130);
static_assert(offsetof(D3DCAPS9, DevCaps) == 0x1c);
static_assert(offsetof(D3DCAPS9, VertexShaderVersion) == 0xc4);
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
Word byte(const void* base, Word byte_offset) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { movzx eax, byte ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
Word halfword(const void* base, Word byte_offset) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { movzx eax, word ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void put(void* base, Word byte_offset, Word value = 0) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
void put_byte(void* base, Word byte_offset, Word value = 0) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov byte ptr [eax + edx], cl }
}
void adjust_depth(void* section, Word delta) noexcept {
    __asm { mov eax, section }
    __asm { mov edx, delta }
    __asm { add dword ptr [eax + 18h], edx }
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* at(void* base, Word byte_offset) noexcept { return pointer(address(base) + byte_offset); }
Word renderer_slot(void* renderer, Word slot, const NativeRendererBindingResetContext& context) noexcept {
    const Word profile = word(renderer);
    __assume(profile == 0x00d5f0a8u);
    return word(context.actual_renderer_profile_00d5f0a8, slot);
}
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) noexcept {
    __try { destroy_native_renderer_optional_guard_00b21110(guard, globals); }
    __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct GuardCleanup {
    const NativeRendererOptionalGuardStorage& guard;
    NativeRendererSynchronizationGlobals& globals;
    bool armed = true;
    ~GuardCleanup() noexcept { if (armed) unwind_guard(guard, globals); }
};

template<bool Restore>
void walk_layouts(NativeRendererDeviceRecreationContext& context) {
    void* const tree = context.actual_hardware_layout_tree_0108d530;
    void* const first_head = pointer(word(tree, 4));
    void* node = pointer(word(first_head));
    void* owner = tree;
    NativeHardwareLayoutTreeIterator iterator{owner, node};
    for (;;) {
        // Load current sentinel before a possibly returning validation call.
        void* const current_head = pointer(word(tree, 4));
        if (!owner || owner != tree)
            context.tree_validation.invalid_parameter(context.tree_validation.context);
        if (node == current_head) return;
        if (!owner)
            context.tree_validation.invalid_parameter(context.tree_validation.context);
        if (node == pointer(word(owner, 4)))
            context.tree_validation.invalid_parameter(context.tree_validation.context);
        void* const value = pointer(word(node, 0x20));
        if constexpr (Restore)
            create_native_hardware_layout_if_missing_00b60a10(value, &context.hardware_layout);
        else
            release_native_hardware_layout_device_resource_00b600b0(value);
        increment_native_hardware_layout_iterator_00b20dc0(iterator, context.tree_validation);
        node = pointer(word(&iterator, 4));
        owner = pointer(word(&iterator));
    }
}
template<class Body>
void walk_owners(void* renderer, Word base_offset, Word count_offset, Body body) {
    for (Word index = 0; index < word(renderer, count_offset); ++index) {
        void* const base = pointer(word(renderer, base_offset));
        body(pointer(word(base, index * 4u)));
    }
}
const volatile Word* texture_profile(Word identity, const NativeRendererResourceRestoreProfiles& profiles) noexcept {
    if (identity == 0x00d61948u) return profiles.texture_2d_00d61948;
    if (identity == 0x00d61870u) return profiles.cube_00d61870;
    __assume(identity == 0x00d618b0u);
    return profiles.volume_00d618b0;
}
template<bool Restore>
void walk_texture_records(void* renderer, NativeRendererDeviceRecreationContext& context) {
    const Word count = word(renderer, 0x1a7c);
    Word cursor = word(renderer, 0x1a78);
    if (cursor == cursor + count * 0x2cu) return;
    for (;;) {
        void* const owner = pointer(word(pointer(cursor), 0x28));
        const Word identity = word(owner);
        const auto* const profile = texture_profile(identity, context.release.actual_resources);
        const Word target = word(profile, Restore ? 0x2c : 0x28);
        if constexpr (Restore) {
            if (target == 0x00b3e190u)
                recreate_native_texture_2d_retained_00b3e190(owner, context.texture_2d);
            else if (target == 0x00b3e1f0u)
                recreate_native_cube_texture_retained_00b3e1f0(owner, context.cube_volume);
            else {
                __assume(target == 0x00b3e230u);
                recreate_native_volume_texture_retained_00b3e230(owner, context.cube_volume);
            }
        } else {
            if (target == 0x00b3d7b0u)
                release_native_texture_2d_retained_00b3d7b0(owner, context.texture_2d);
            else if (target == 0x00b3d7c0u)
                release_native_cube_texture_retained_00b3d7c0(owner);
            else {
                __assume(target == 0x00b3d800u);
                release_native_volume_texture_retained_00b3d800(owner);
            }
        }
        const Word current_count = word(renderer, 0x1a7c);
        const Word current_base = word(renderer, 0x1a78);
        const Word current_end = current_base + current_count * 0x2cu;
        cursor += 0x2cu;
        if (cursor == current_end) return;
    }
}
const volatile Word* physical_profile(Word identity, const NativeRendererResetReadinessProfiles& profiles) noexcept {
    if (identity == 0x00d61e58u) return profiles.pooled_index_00d61e58;
    if (identity == 0x00d61e7cu) return profiles.pooled_vertex_00d61e7c;
    if (identity == 0x00d61e10u) return profiles.private_index_00d61e10;
    __assume(identity == 0x00d61e34u);
    return profiles.private_vertex_00d61e34;
}
void recreate_physical(void* owner, Word target, void* device) {
    if (target == 0x00b49180u) recreate_native_physical_index_buffer_00b49180(owner, nullptr, device);
    else if (target == 0x00b492b0u) recreate_native_physical_vertex_buffer_00b492b0(owner, nullptr, device);
    else if (target == 0x00b4b810u) recreate_native_private_index_buffer_00b4b810(owner, nullptr, device);
    else { __assume(target == 0x00b4b9c0u); recreate_native_private_vertex_buffer_00b4b9c0(owner, nullptr, device); }
}
struct GammaAccess {
    const volatile Word* profile;
    const NativeRendererGammaContext* gamma;
};
__declspec(naked) void __fastcall apply_current_gamma(void*, const GammaAccess*) {
    __asm {
        push ebx
        push esi
        mov ebx, edx
        mov esi, ecx
        fld dword ptr [esi + 196ch] // B2990F precedes current profile/slot.
        mov eax, [esi]
        cmp eax, 0d5f0a8h
        jne unsupported
        mov edx, [ebx]
        mov eax, [edx + 0f0h]
        cmp eax, 0b21960h
        jne unsupported
        push ecx
        fstp dword ptr [esp] // B29920: the actual outgoing float word.
        lea eax, [esp]
        push dword ptr [ebx + 4]
        push eax
        push esi
        call set_native_renderer_gamma_00b21960
        add esp, 10h
        pop esi
        pop ebx
        ret
    unsupported:
        _emit 0x0f
        _emit 0x0b
    }
}
} // namespace

void recreate_native_renderer_device_00b29670(void* renderer,
    NativeRendererDeviceRecreationContext& context) {
    auto& bindings = context.release.actual_bindings;
    auto& globals = bindings.actual_vertex.actual_logical_owner.actual_synchronization_0108d6dc;
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    void* const entered_lifecycle = pointer(word(renderer, 0x199c));
    GuardCleanup cleanup{guard, globals}; // state0 precedes EnterCriticalSection.
    EnterCriticalSection(static_cast<CRITICAL_SECTION*>(entered_lifecycle));
    adjust_depth(entered_lifecycle, 1);
    const Word frame_target = renderer_slot(renderer, 0x98, bindings);
    put_byte(renderer, 0x1d8a);
    __assume(frame_target == 0x00b24e70u);
    bind_native_renderer_frame_targets_00b24e70(renderer, nullptr, context.frame_targets);
    reset_native_renderer_bindings_00b24bf0(renderer, bindings);
    release_native_dynamic_buffers_for_reset_00b237d0(renderer, globals);
    release_native_renderer_resources_00b262c0(renderer, context.release);
    walk_layouts<false>(context);
    walk_owners(renderer, 0x1aac, 0x1ab0, [&](void* owner) { save_native_logical_vertex_buffer_for_device_reset_00b49d00(owner, context.logical_save); });
    walk_owners(renderer, 0x1ab8, 0x1abc, [&](void* owner) { save_native_logical_index_buffer_for_device_reset_00b49f80(owner, context.logical_save); });
    walk_owners(renderer, 0x1ac4, 0x1ac8, [](void* owner) { save_release_native_vertex_shader_00b5e810(owner); });
    walk_owners(renderer, 0x1ad0, 0x1ad4, [](void* owner) { save_release_native_pixel_shader_00b5e750(owner); });
    walk_texture_records<false>(renderer, context);
    if (word(context.actual_online_publication_00f8abe8) != 0)
        (void)context.online_device->on_destroy_device_00c2f1c6();

    using ReferenceCall = ULONG (WINAPI*)(void*);
    void* const captured_device = pointer(word(renderer, 0x1a10));
    if (captured_device) {
        auto add_ref = reinterpret_cast<ReferenceCall>(word(pointer(word(captured_device)), 4));
        (void)add_ref(captured_device);
        auto release = reinterpret_cast<ReferenceCall>(word(pointer(word(captured_device)), 8));
        (void)release(captured_device);
    }
    void* const final_device = pointer(word(renderer, 0x1a10));
    auto final_release = reinterpret_cast<ReferenceCall>(word(pointer(word(final_device)), 8));
    (void)final_release(final_device);
    void* const caps_d3d = pointer(word(renderer, 0x1990));
    D3DCAPS9 caps; // Native does not initialize this output or branch on HRESULT.
    put(renderer, 0x1a10);
    using CapsCall = HRESULT (WINAPI*)(void*, UINT, D3DDEVTYPE, D3DCAPS9*);
    auto get_caps = reinterpret_cast<CapsCall>(word(pointer(word(caps_d3d)), 0x38));
    (void)get_caps(caps_d3d, 0, D3DDEVTYPE_HAL, &caps);
    Word behavior = 0x20;
    if ((word(&caps, 0x1c) & 0x10000u) != 0 && halfword(&caps, 0xc4) >= 0x101u)
        behavior = 0x40;
    void* const create_d3d = pointer(word(renderer, 0x1990));
    using CreateCall = HRESULT (WINAPI*)(void*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);
    auto create_device = reinterpret_cast<CreateCall>(word(pointer(word(create_d3d)), 0x40));
    auto* const parameters = static_cast<D3DPRESENT_PARAMETERS*>(at(renderer, 0x1a28));
    const HWND window = reinterpret_cast<HWND>(word(renderer, 0x1a44));
    (void)create_device(create_d3d, 0, D3DDEVTYPE_HAL, window, behavior | 4u, parameters,
        static_cast<IDirect3DDevice9**>(at(renderer, 0x1a10)));
    set_native_renderer_render_state_00b24460(renderer, 0xa1, word(renderer, 0x1a38) != 0, globals);
    initialize_native_renderer_default_states_00b26170(renderer, globals);
    const GammaAccess gamma{bindings.actual_renderer_profile_00d5f0a8, &context.gamma};
    apply_current_gamma(renderer, &gamma);
    restore_native_renderer_resources_00b23b10(renderer, context.restore);
    if (byte(renderer, 0x1d8c) == 0 && byte(renderer, 0x1d8a) == 0) {
        void* const first = pointer(word(renderer, 0x1974));
        void* const first_device = pointer(word(renderer, 0x1a10));
        put_byte(renderer, 0x1d8c, 1);
        const Word first_target = word(physical_profile(word(first), context.physical_profiles), 0x20);
        recreate_physical(first, first_target, first_device);
        void* const second = pointer(word(renderer, 0x1978));
        const auto* const second_profile = physical_profile(word(second), context.physical_profiles);
        void* const second_device = pointer(word(renderer, 0x1a10));
        const Word second_target = word(second_profile, 0x20);
        recreate_physical(second, second_target, second_device);
    }
    if (word(context.actual_online_publication_00f8abe8) != 0) {
        void* const current_device = pointer(word(renderer, 0x1a10));
        (void)context.online_device->on_create_device_00c2f1c0(current_device, parameters);
    }
    walk_layouts<true>(context);
    walk_owners(renderer, 0x1aac, 0x1ab0, [&](void* owner) { restore_native_logical_vertex_buffer_after_device_reset_00b49dc0(owner, context.logical_restore); });
    walk_owners(renderer, 0x1ab8, 0x1abc, [&](void* owner) { restore_native_logical_index_buffer_after_device_reset_00b4a040(owner, context.logical_restore); });
    const void* const renderer_publication =
        const_cast<const void**>(&context.logical_restore.actual_renderer_00f8d394);
    walk_owners(renderer, 0x1ac4, 0x1ac8, [&](void* owner) { restore_native_vertex_shader_00b5e8e0(owner, renderer_publication); });
    walk_owners(renderer, 0x1ad0, 0x1ad4, [&](void* owner) { restore_native_pixel_shader_00b5e890(owner, renderer_publication); });
    walk_texture_records<true>(renderer, context);
    void* const current_lifecycle = pointer(word(renderer, 0x199c));
    adjust_depth(current_lifecycle, 0xffffffffu);
    LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(current_lifecycle));
    const auto current_mode = globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0)
        leave_native_renderer_optional_guard_00b33b00(guard.renderer_04, word(&guard), globals);
}
} // namespace bsp
