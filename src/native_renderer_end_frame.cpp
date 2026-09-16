#include "bsp/native_renderer_end_frame.hpp"
#include "bsp/native_render_state_leaves.hpp"
#include "bsp/native_renderer_cached_states.hpp"
#include "bsp/native_renderer_frame_statistics.hpp"
#include "bsp/native_occlusion_query_poll.hpp"
#include "bsp/native_renderer_debug_lines.hpp"
#include "bsp/xlive_library.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d9.h>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
void* at(void* base, Word offset) noexcept {
    return pointer(reinterpret_cast<Word>(base) + offset);
}
void put(void* base, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(base, offset)) = value;
}
std::uint8_t byte(void* base, Word offset) noexcept {
    return *static_cast<volatile std::uint8_t*>(at(base, offset));
}
void require_profile(bool valid) {
    if (!valid) throw std::invalid_argument("unsupported actual EndFrame profile");
}
void renderer_slot(void* renderer, Word offset, Word expected,
    const NativeRendererBindingResetContext& bindings) {
    require_profile(word(renderer) == 0x00d5f0a8u);
    require_profile(word(bindings.actual_renderer_profile_00d5f0a8, offset) == expected);
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
void rewind_physical(void* buffer, NativePhysicalBufferRewindContext& context, bool vertex) {
    auto& globals = context.actual_synchronization_0108d6dc;
    NativeRendererOptionalGuardStorage guard;
    // Native compares the mode before conditionally reading the publication.
    const auto entry_mode = globals.mode_00;
    if (entry_mode != 0) {
        auto* const renderer = context.actual_renderer_00f8d394;
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    const auto initial_count = static_cast<std::int32_t>(word(buffer, 0x0c));
    GuardCleanup cleanup{guard, globals};
    if (initial_count > 0) {
        Word index = 0;
        do {
            auto* const child = pointer(word(pointer(word(buffer, 8)), index * 4u));
            const auto profile = word(child);
            const auto* const table = vertex ? context.actual_logical_vertex_profile_00d61d6c
                                            : context.actual_logical_index_profile_00d61de0;
            require_profile(profile == (vertex ? 0x00d61d6cu : 0x00d61de0u));
            require_profile(word(table, 8) == (vertex ? 0x00b48d40u : 0x00b48dd0u));
            if (vertex) rewind_native_logical_vertex_00b48d40(child);
            else rewind_native_logical_index_00b48dd0(child);
            ++index;
        } while (static_cast<std::int32_t>(index) < static_cast<std::int32_t>(word(buffer, 0x0c)));
    }
    put(buffer, 0x1c, 0);
    put(buffer, 0x24, 0);
    const auto exit_mode = globals.mode_00;
    cleanup.armed = false;
    if (exit_mode != 0)
        leave_native_renderer_optional_guard_00b33b00(guard.renderer_04, word(&guard), globals);
}
using EndScene = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*);
using Present = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
} // namespace

NativeXLiveRenderImport::NativeXLiveRenderImport(const XLiveLibrary& library) : render_(nullptr) {
    const auto module = static_cast<HMODULE>(library.module_handle());
    if (!module) throw std::invalid_argument("XLiveRender requires the loaded XLive library");
    const auto symbol = GetProcAddress(module, MAKEINTRESOURCEA(5002));
    if (!symbol) throw std::runtime_error("loaded XLive library lacks ordinal 5002");
    static_assert(sizeof(symbol) == sizeof(render_));
    // Avoid MSVC's incompatible-function-pointer warning without a callable
    // original-address cast. This is the resolved host DLL import address.
    __asm {
        mov eax, symbol
        mov edx, this
        mov dword ptr [edx], eax
    }
}
std::uint32_t NativeXLiveRenderImport::render_00c2f1cc() const { return render_(); }

__declspec(naked) void __fastcall rewind_native_logical_vertex_00b48d40(void*) noexcept {
    __asm { mov dword ptr [ecx + 05ch], 0ffffffffh }
    __asm { ret }
}
__declspec(naked) void __fastcall rewind_native_logical_index_00b48dd0(void*) noexcept {
    __asm { mov dword ptr [ecx + 0ch], 0ffffffffh }
    __asm { ret }
}
void rewind_native_physical_vertex_00b232b0(void* buffer, NativePhysicalBufferRewindContext& context) {
    rewind_physical(buffer, context, true);
}
void rewind_native_physical_index_00b231c0(void* buffer, NativePhysicalBufferRewindContext& context) {
    rewind_physical(buffer, context, false);
}

void end_native_renderer_frame_00b2d8e0(void* renderer, const void* save_header,
    NativeRendererEndFrameContext* context) {
    if (word(renderer, 0x1998) == 0) return;
    auto& c = *context;
    auto& globals = c.actual_rewind.actual_synchronization_0108d6dc;
    if (c.actual_in_end_frame_0108d4cc != 0) DebugBreak();
    const auto entry_mode = globals.mode_00;
    c.actual_in_end_frame_0108d4cc = 1;
    NativeRendererOptionalGuardStorage guard;
    if (entry_mode != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    GuardCleanup cleanup{guard, globals};
    auto* const queue = get_native_render_command_queue_004c11f0(c.actual_queue_getter);
    c.remaining.execute_queue_00b1ebe0(*queue);
    for (Word sampler = 0; sampler < 20; ++sampler) {
        renderer_slot(renderer, 0x130, 0x00b24710u, c.actual_bindings);
        bind_native_renderer_texture_00b24710(renderer, sampler, nullptr, c.actual_bindings.actual_texture);
    }
    for (Word stream = 0; stream < 4; ++stream) {
        renderer_slot(renderer, 0x134, 0x00b24840u, c.actual_bindings);
        bind_native_renderer_vertex_stream_00b24840(renderer, stream, nullptr, c.actual_bindings.actual_vertex);
    }
    renderer_slot(renderer, 0x138, 0x00b24b00u, c.actual_bindings);
    bind_native_renderer_index_stream_00b24b00(renderer, nullptr, 0, c.actual_bindings.actual_index);
    auto* const section = at(renderer, 0x19f4);
    EnterCriticalSection(static_cast<CRITICAL_SECTION*>(section));
    put(section, 0x18, word(section, 0x18) + 1u);
    rewind_native_physical_vertex_00b232b0(pointer(word(renderer, 0x1974)), c.actual_rewind);
    // No native EH cleanup is armed for this embedded section.
    put(section, 0x18, word(section, 0x18) - 1u);
    LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(section));
    rewind_native_physical_index_00b231c0(pointer(word(renderer, 0x1978)), c.actual_rewind);
    if (save_header) {
        renderer_slot(renderer, 0x110, 0x00b23c50u, c.actual_bindings);
        capture_native_renderer_surface_save_00b23c50(renderer, &c.actual_surface_save, save_header);
    }
    c.remaining.render_00b2bb90(renderer);
    renderer_slot(renderer, 0xc4, 0x00b28d00u, c.actual_bindings);
    draw_native_renderer_debug_lines_00b28d00(renderer, c.actual_debug_lines);
    c.remaining.render_00b2b580(renderer);
    static constexpr Word states[][2] = {
        {0x1a, 0}, {0x1a, 0}, {0x1c, 0}, {7, 0}, {0x0e, 0}, {0x17, 2},
        {0x34, 0}, {0x0f, 0}, {0x1b, 0}, {0x16, 3}, {0x1d, 0}, {8, 3},
        {0xa8, 0x0f}, {0x34, 0}, {0xae, 0}, {0xc2, 0}
    };
    for (const auto& state : states)
        set_native_renderer_render_state_00b24460(renderer, state[0], state[1], globals);
    (void)c.actual_xlive.render_00c2f1cc();
    if (word(renderer, 0x1d90) == 0 && byte(renderer, 0x1d8a) == 0) {
        auto* const device = static_cast<IDirect3DDevice9*>(pointer(word(renderer, 0x1a10)));
        const auto call = reinterpret_cast<EndScene>(word(pointer(word(device)), 0xa8));
        (void)call(device);
    }
    clear_native_renderer_binding_cache_00b241c0(at(renderer, 0x34), c.actual_cache);
    publish_native_renderer_frame_statistics_00b0ccb0(at(renderer, 0x1b78));
    const auto initial_count = word(renderer, 0x19a4);
    put(renderer, 0x1998, 0);
    if (initial_count > 0) {
        Word index = 0;
        do {
            auto* const callback = pointer(word(pointer(word(renderer, 0x19a0)), index * 4u));
            if (word(callback, 8) == 0) {
                require_profile(word(callback) == 0x00d62ad0u);
                require_profile(word(c.actual_query_profile_00d62ad0, 0x10) == 0x00b5fca0u);
                (void)poll_native_occlusion_query_00b5fca0(callback);
            }
            ++index;
        } while (index < word(renderer, 0x19a4));
    }
    if (c.actual_clear_request_00e1306c != 0) {
        const Word color = 0;
        renderer_slot(renderer, 8, 0x00b21430u, c.actual_bindings);
        clear_native_renderer_00b21430(renderer, &c.actual_clear, 0, nullptr, 1, &color, 1.0f, 0);
        c.actual_clear_request_00e1306c = 0;
    }
    if (word(renderer, 0x1d90) == 0 && byte(renderer, 0x1d8a) == 0) {
        renderer_slot(renderer, 0x2c, 0x00b1fe20u, c.actual_bindings);
        if (native_render_is_frame_active_00b1fe20(renderer) == 0) {
            auto* const device = static_cast<IDirect3DDevice9*>(pointer(word(renderer, 0x1a10)));
            const auto call = reinterpret_cast<Present>(word(pointer(word(device)), 0x44));
            if (static_cast<Word>(call(device, nullptr, nullptr, nullptr, nullptr)) == 0x88760868u)
                c.actual_present_failure_0108d4b9 = 1;
        }
    }
    put(c.actual_counter_0108fe88, 8, 0);
    put(renderer, 0x14, word(renderer, 0x14) + 1u);
    const auto exit_mode = globals.mode_00;
    c.actual_in_end_frame_0108d4cc = 0;
    cleanup.armed = false;
    if (exit_mode != 0)
        leave_native_renderer_optional_guard_00b33b00(guard.renderer_04, word(&guard), globals);
}
void end_native_renderer_frame_default_00b2f4a0(void* renderer, NativeRendererEndFrameContext* context) {
    end_native_renderer_frame_00b2d8e0(renderer, nullptr, context);
}
void end_native_renderer_frame_with_save_00b2f4b0(void* renderer, const void* header,
    NativeRendererEndFrameContext* context) {
    end_native_renderer_frame_00b2d8e0(renderer, header, context);
}
} // namespace bsp
