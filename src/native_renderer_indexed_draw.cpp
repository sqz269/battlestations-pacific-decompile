#include "bsp/native_renderer_indexed_draw.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <exception>

namespace bsp {
__declspec(naked) void* __fastcall native_viewport_size_address_00b1f740(
    const void*) noexcept {
    __asm { lea eax, [ecx + 10h] }
    __asm { ret }
}
__declspec(naked) std::uint32_t __fastcall native_logical_vertex_count_00b48cd0(
    const void*) noexcept {
    __asm { mov eax, dword ptr [ecx + 64h] }
    __asm { ret }
}

namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const volatile void* base, Word byte_offset) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
std::uint8_t byte(const void* base, Word byte_offset) noexcept {
    std::uint8_t result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov al, byte ptr [eax + edx] }
    __asm { mov result, al }
    return result;
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
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
void finish_guard(GuardCleanup& cleanup, const void* saved_renderer,
    std::uint8_t selected_mode) {
    cleanup.armed = false;
    if (selected_mode != 0) {
        const Word ignored = word(&cleanup.guard, 0);
        leave_native_renderer_optional_guard_00b33b00(
            saved_renderer, ignored, cleanup.globals);
    }
}
Word current_vertex_count(const void* logical, NativeRendererIndexedDrawContext& context) {
    const Word current_profile = word(logical, 0);
    __assume(current_profile == 0x00d61d6c);
    const Word current_getter = word(context.actual_logical_vertex_profile_00d61d6c, 0x20);
    __assume(current_getter == 0x00b48cd0);
    return native_logical_vertex_count_00b48cd0(logical);
}
using DrawIndexedPrimitive = std::int32_t (__stdcall*)(void*, Word,
    std::int32_t, Word, Word, Word, Word);
} // namespace

void draw_native_renderer_indexed_00b24010(void* renderer,
    Word primitive_type, Word minimum_vertex, Word vertex_count,
    Word start_index, Word primitive_count, NativeRendererIndexedDrawContext& context) {
    if (word(renderer, 0x1d90) != 0) return;
    if (byte(renderer, 0x1d8a) != 0) return;
    (void)native_viewport_size_address_00b1f740(pointer(word(renderer, 0x1904)));

    auto& globals = context.actual_synchronization_0108d6dc;
    NativeRendererOptionalGuardStorage guard;
    std::uint8_t captured_mode = globals.mode_00;
    const void* saved_renderer;
    if (captured_mode != 0) {
        saved_renderer = renderer;
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
        captured_mode = globals.mode_00;
    } else {
        // Original B24072 loads its uninitialized saved renderer even when
        // subsequent mode-zero exits never use it. Preserve the raw read.
        saved_renderer = pointer(word(&guard, 4));
    }
    std::uint8_t have_vertices;
    __asm { mov eax, vertex_count }
    __asm { test eax, eax }
    __asm { setnz al }
    __asm { mov have_vertices, al }
    GuardCleanup cleanup{guard, globals}; // Native TEST before B2407D EH arm.
    if (have_vertices == 0 || primitive_count == 0) {
        finish_guard(cleanup, saved_renderer, captured_mode);
        return;
    }
    if (auto* const logical = pointer(word(renderer, 0x1774))) {
        const Word count = current_vertex_count(logical, context);
        if (vertex_count > count) {
            const auto* const current_logical = pointer(word(renderer, 0x1774));
            if (word(current_logical, 0x54) == 0x40000001u) {
                finish_guard(cleanup, saved_renderer, globals.mode_00);
                return;
            }
        }
    }
    auto* const device = pointer(word(renderer, 0x1a10));
    const auto* const table = pointer(word(device, 0));
    const auto base_vertex = static_cast<std::int32_t>(word(renderer, 0x17bc));
    const auto call = reinterpret_cast<DrawIndexedPrimitive>(word(table, 0x148));
    (void)call(device, primitive_type, base_vertex, minimum_vertex,
        vertex_count, start_index, primitive_count);
    finish_guard(cleanup, saved_renderer, globals.mode_00);
}

} // namespace bsp
