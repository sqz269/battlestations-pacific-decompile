#include "bsp/native_renderer_vertex_binding.hpp"
#include "bsp/native_renderer_binding_getters.hpp"
#include "bsp/native_renderer_surface_bindings.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d9.h>

#include <exception>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = std::uint32_t;
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
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
void* at(void* base, Word offset) noexcept {
    return pointer(reinterpret_cast<Word>(base) + offset);
}
const volatile Word* logical_table(const void* logical,
    NativeRendererVertexBindingContext& context) noexcept {
    const auto profile = word(logical);
    __assume(profile == 0x00d61d6c);
    return context.actual_logical_owner.actual_logical_profile_00d61d6c;
}
void* logical_buffer(const void* logical, NativeRendererVertexBindingContext& context) {
    const auto getter = word(logical_table(logical, context), 0x2c);
    __assume(getter == 0x00b48cf0);
    return get_native_logical_vertex_buffer_00b48cf0(
        logical, context.actual_logical_owner.actual_physical_profiles);
}
void* logical_declaration(const void* logical, NativeRendererVertexBindingContext& context) {
    const auto getter = word(logical_table(logical, context), 0x24);
    __assume(getter == 0x00b48ce0);
    return native_logical_vertex_stream_get_declaration_00b48ce0(logical);
}
Word logical_offset(const void* logical, NativeRendererVertexBindingContext& context) {
    const auto getter = word(logical_table(logical, context), 0x28);
    __assume(getter == 0x00b48d10);
    return native_logical_vertex_stream_get_offset_00b48d10(logical);
}
int terminate_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) noexcept {
    __try {
        destroy_native_renderer_optional_guard_00b21110(guard, globals);
    } __except (terminate_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct GuardCleanup {
    const NativeRendererOptionalGuardStorage& guard;
    NativeRendererSynchronizationGlobals& globals;
    bool armed = true;
    ~GuardCleanup() noexcept { if (armed) unwind_guard(guard, globals); }
};
void enter_guard(void* renderer, NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) {
    guard.renderer_04 = renderer;
    guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
}
void leave_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) {
    leave_native_renderer_optional_guard_00b33b00(guard.renderer_04, guard.entered_00, globals);
}
void finish_guard(GuardCleanup& cleanup) {
    const auto current_mode = cleanup.globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0) leave_guard(cleanup.guard, cleanup.globals);
}
Word renderer_slot(void* renderer, Word offset, NativeRendererBindingResetContext& context) {
    const auto profile = word(renderer);
    __assume(profile == 0x00d5f0a8);
    return word(context.actual_renderer_profile_00d5f0a8, offset);
}
using SetStream = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, UINT,
    IDirect3DVertexBuffer9*, UINT, UINT);
using SetPixel = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, IDirect3DPixelShader9*);
using SetVertex = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, IDirect3DVertexShader9*);
using SetDepth = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, IDirect3DSurface9*);
} // namespace

void* get_native_logical_vertex_buffer_00b48cf0(const void* logical,
    const NativeLogicalBufferDeviceRestoreProfiles& profiles) {
    const void* const physical = pointer(word(logical, 0x58));
    const auto profile = word(physical);
    const volatile Word* table;
    if (profile == 0x00d61e34) table = profiles.private_vertex_00d61e34;
    else if (profile == 0x00d61e7c) table = profiles.pooled_vertex_00d61e7c;
    else { __assume(0); }
    const auto getter = word(table, 0x1c);
    __assume(getter == 0x00b4b9f0);
    return native_physical_vertex_buffer_get_com_00b4b9f0(physical);
}

void* assign_native_renderer_vertex_owner_00b23710(void* destination,
    const void* source, NativeRenderActualOwners& owners) {
    void* const incoming = pointer(word(source));
    void* const old = pointer(word(destination));
    if (old != incoming) {
        put(destination, 0, reinterpret_cast<Word>(incoming));
        if (incoming) InterlockedIncrement(reinterpret_cast<volatile LONG*>(at(incoming, 4)));
        if (old) release_native_render_actual_owner(owners, old);
    }
    return destination;
}

void bind_native_renderer_vertex_stream_00b24840(void* renderer, Word index,
    void* incoming, NativeRendererVertexBindingContext& context) {
    auto& owner = context.actual_logical_owner;
    auto& globals = owner.actual_synchronization_0108d6dc;
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) enter_guard(renderer, guard, globals);
    void* const header = at(renderer, index * 16u + 0x1774u);
    void* const initial_old = pointer(word(header));
    const bool identical = initial_old == incoming;
    GuardCleanup cleanup{guard, globals};
    if (identical) { finish_guard(cleanup); return; }
    if (initial_old && incoming) {
        void* const old_buffer = logical_buffer(initial_old, context);
        void* const new_buffer = logical_buffer(incoming, context);
        const Word cached_stride = word(header, 4);
        const Word new_stride = word(logical_declaration(incoming, context), 0xcc);
        const Word cached_offset = word(header, 8);
        const Word new_offset = logical_offset(incoming, context);
        if (old_buffer == new_buffer && cached_stride == new_stride && cached_offset == new_offset) {
            assign_native_renderer_vertex_owner_00b23710(header, &incoming, owner.actual_owners);
            finish_guard(cleanup);
            return;
        }
    }
    // Native inlines the same fresh capture, publish, retain, release sequence.
    assign_native_renderer_vertex_owner_00b23710(header, &incoming, owner.actual_owners);
    void* buffer = nullptr;
    Word stride = 0;
    Word offset = 0;
    if (incoming) {
        buffer = logical_buffer(incoming, context);
        stride = word(logical_declaration(incoming, context), 0xcc);
        offset = logical_offset(incoming, context);
    }
    put(header, 8, offset);
    put(header, 4, stride);
    auto* const device = static_cast<IDirect3DDevice9*>(pointer(word(renderer, 0x1a10)));
    const auto call = reinterpret_cast<SetStream>(word(pointer(word(device)), 0x190));
    (void)call(device, index, static_cast<IDirect3DVertexBuffer9*>(buffer), offset, stride);
    put(renderer, 0x1bb4, word(renderer, 0x1bb4) + 1u);
    finish_guard(cleanup);
}

void reset_native_renderer_bindings_00b24bf0(void* renderer,
    NativeRendererBindingResetContext& context) {
    auto& globals = context.actual_vertex.actual_logical_owner.actual_synchronization_0108d6dc;
    for (Word sampler = 0; sampler < 20; ++sampler) {
        const auto target = renderer_slot(renderer, 0x130, context);
        __assume(target == 0x00b24710);
        bind_native_renderer_texture_00b24710(renderer, sampler, nullptr, context.actual_texture);
    }
    for (Word remaining = 4; remaining != 0; --remaining) {
        const auto target = renderer_slot(renderer, 0x134, context);
        __assume(target == 0x00b24840);
        bind_native_renderer_vertex_stream_00b24840(renderer, 0, nullptr, context.actual_vertex);
    }
    const auto index_target = renderer_slot(renderer, 0x138, context);
    __assume(index_target == 0x00b24b00);
    bind_native_renderer_index_stream_00b24b00(renderer, nullptr, 0, context.actual_index);
    NativeRendererOptionalGuardStorage guard; // Same native eight-byte local for all three states.
    if (globals.mode_00 != 0) enter_guard(renderer, guard, globals);
    {
        const auto cached_pixel = word(renderer, 0x176c);
        GuardCleanup cleanup{guard, globals}; // state0 after cache read, before clear.
        put(renderer, 0x176c, 0);
        if (cached_pixel) {
            auto* const device = static_cast<IDirect3DDevice9*>(pointer(word(renderer, 0x1a10)));
            const auto call = reinterpret_cast<SetPixel>(word(pointer(word(device)), 0x1ac));
            (void)call(device, nullptr);
            put(renderer, 0x1bbc, word(renderer, 0x1bbc) + 1u);
        }
        const auto current_mode = globals.mode_00;
        cleanup.armed = false;
        if (current_mode != 0) {
            leave_guard(guard, globals);
            // Native second entry exists ONLY under this first exit branch.
            if (globals.mode_00 != 0) enter_guard(renderer, guard, globals);
        }
    }
    {
        const auto cached_vertex = word(renderer, 0x1770);
        GuardCleanup cleanup{guard, globals}; // state1, reusing even skipped-entry storage.
        put(renderer, 0x1770, 0);
        if (cached_vertex) {
            auto* const device = static_cast<IDirect3DDevice9*>(pointer(word(renderer, 0x1a10)));
            const auto call = reinterpret_cast<SetVertex>(word(pointer(word(device)), 0x170));
            (void)call(device, nullptr);
            put(renderer, 0x1bc0, word(renderer, 0x1bc0) + 1u);
        }
        finish_guard(cleanup);
    }
    const auto layout_target = renderer_slot(renderer, 0xe0, context);
    __assume(layout_target == 0x00b23f20);
    bind_native_renderer_vertex_layout_00b23f20(renderer, nullptr, context.actual_layout);
    for (Word color = 0; color < 4; ++color)
        bind_native_renderer_color_surface_00b23d80(renderer, color, nullptr, globals);
    if (globals.mode_00 != 0) enter_guard(renderer, guard, globals);
    auto* const device = static_cast<IDirect3DDevice9*>(pointer(word(renderer, 0x1a10)));
    const auto call = reinterpret_cast<SetDepth>(word(pointer(word(device)), 0x9c));
    GuardCleanup cleanup{guard, globals}; // state2 after current device/table capture.
    (void)call(device, nullptr);
    finish_guard(cleanup);
}
} // namespace bsp
