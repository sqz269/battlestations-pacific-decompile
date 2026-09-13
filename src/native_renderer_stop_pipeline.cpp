#include "bsp/native_renderer_stop_pipeline.hpp"
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
void clear_run(void* worker) noexcept {
    __asm {
        mov eax, worker
        mov byte ptr [eax + 4], 0
    }
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
Word renderer_slot(void* renderer, Word offset,
    const NativeRendererBindingResetContext& bindings) noexcept {
    const auto profile = word(renderer);
    __assume(profile == 0x00d5f0a8u);
    return word(bindings.actual_renderer_profile_00d5f0a8, offset);
}
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) noexcept {
    __try {
        destroy_native_renderer_optional_guard_00b21110(guard, globals);
    } __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct GuardCleanup {
    const NativeRendererOptionalGuardStorage& guard;
    NativeRendererSynchronizationGlobals& globals;
    bool armed = true;
    ~GuardCleanup() noexcept { if (armed) unwind_guard(guard, globals); }
};
using SetDepth = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, IDirect3DSurface9*);
} // namespace

std::uint32_t request_native_renderer_worker_stop_00b33bf0(void* worker,
    const volatile Word* event_profile) noexcept {
    clear_run(worker);
    auto* const ack = static_cast<NativeEventOwnerStorage*>(pointer(word(worker, 0x10)));
    const auto profile = word(ack);
    __assume(profile == native_event_concrete_table_00d6821c);
    const auto target = word(event_profile, 8);
    __assume(target == 0x00bd17c0u);
    return wait_native_event_owner_00bd17c0(ack);
}

void clear_native_renderer_stop_pipeline_00b26920(void* renderer,
    NativeRendererStopPipelineContext& context) {
    auto& bindings = context.actual_bindings;
    auto& globals = bindings.actual_vertex.actual_logical_owner.actual_synchronization_0108d6dc;
    for (Word sampler = 0; sampler < 20; ++sampler) {
        const auto target = renderer_slot(renderer, 0x130, bindings);
        __assume(target == 0x00b24710u);
        bind_native_renderer_texture_00b24710(renderer, sampler, nullptr, bindings.actual_texture);
    }
    for (Word stream = 0; stream < 4; ++stream) {
        const auto target = renderer_slot(renderer, 0x134, bindings);
        __assume(target == 0x00b24840u);
        bind_native_renderer_vertex_stream_00b24840(renderer, stream, nullptr, bindings.actual_vertex);
    }
    const auto index_target = renderer_slot(renderer, 0x138, bindings);
    __assume(index_target == 0x00b24b00u);
    bind_native_renderer_index_stream_00b24b00(renderer, nullptr, 0, bindings.actual_index);
    clear_native_renderer_binding_cache_00b241c0(
        pointer(reinterpret_cast<Word>(renderer) + 0x34u), context.actual_cache);
    const auto* const color = static_cast<NativeSurfaceOwnerStorage*>(pointer(word(renderer, 0x197c)));
    bind_native_renderer_color_surface_00b23d80(renderer, 0, color, globals);
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    auto* const device = static_cast<IDirect3DDevice9*>(pointer(word(renderer, 0x1a10)));
    const auto call = reinterpret_cast<SetDepth>(word(pointer(word(device)), 0x9c));
    GuardCleanup cleanup{guard, globals}; // Native state0 only after device/slot capture.
    (void)call(device, nullptr);
    const auto current_mode = globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0)
        leave_native_renderer_optional_guard_00b33b00(guard.renderer_04, word(&guard), globals);
}

void stop_native_renderer_worker_mode_00b28a90(void* renderer,
    NativeRendererStopPipelineContext& context) {
    if (auto* const worker = pointer(word(renderer, 0x1970)))
        (void)request_native_renderer_worker_stop_00b33bf0(worker, context.actual_event_profile_00d6821c);
    auto& globals = context.actual_bindings.actual_vertex.actual_logical_owner.actual_synchronization_0108d6dc;
    set_native_renderer_synchronization_00b33aa0(globals, 0);
    clear_native_renderer_stop_pipeline_00b26920(renderer, context);
    Sleep(100);
}
} // namespace bsp
