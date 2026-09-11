#include "bsp/native_renderer_vertex_layout_binding.hpp"
#include "bsp/native_renderer_binding_getters.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d9.h>

#include <exception>

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
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
volatile LONG* references(void* owner) noexcept {
    return reinterpret_cast<volatile LONG*>(reinterpret_cast<std::uintptr_t>(owner) + 4u);
}
const volatile std::uint32_t* current_layout_table(void* owner,
    NativeRendererVertexLayoutBindingContext& context) noexcept {
    const auto current_profile = word(owner);
    __assume(current_profile == 0x00d62af4);
    return context.actual_layout_profile_00d62af4;
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
using SetDeclaration = HRESULT (STDMETHODCALLTYPE*)(
    IDirect3DDevice9*, IDirect3DVertexDeclaration9*);
} // namespace

void bind_native_renderer_vertex_layout_00b23f20(void* renderer, void* incoming,
    NativeRendererVertexLayoutBindingContext& context) {
    if (pointer(word(renderer, 0x17b4)) == incoming) return;

    auto& globals = context.actual_synchronization_0108d6dc;
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    auto* const captured_old = pointer(word(renderer, 0x17b4));
    const bool replace = captured_old != incoming;
    GuardCleanup cleanup{guard, globals}; // Native arms state0 after capture/compare.
    if (replace) {
        put(renderer, 0x17b4, reinterpret_cast<std::uint32_t>(incoming));
        if (incoming) InterlockedIncrement(references(incoming));
        if (captured_old && InterlockedDecrement(references(captured_old)) == 0) {
            const auto zero_terminal = word(current_layout_table(captured_old, context));
            __assume(zero_terminal == 0x00bd30e0);
            // The complete BD30E0 terminal rereads the owner's CURRENT table.
            const auto deleting_terminal = word(current_layout_table(captured_old, context), 4);
            __assume(deleting_terminal == 0x00b60770);
            delete_native_hardware_layout_00b60770(captured_old, 1,
                context.actual_hardware_layout_owner);
        }
    }
    if (incoming) {
        auto* const before_getter_device = pointer(word(renderer, 0x1a10));
        const auto* const layout_table = current_layout_table(incoming, context);
        const auto* const captured_device_table = pointer(word(before_getter_device));
        const auto getter_terminal = word(layout_table, 8);
        __assume(getter_terminal == 0x00b5ff00);
        auto* const declaration = static_cast<IDirect3DVertexDeclaration9*>(
            native_vertex_layout_get_com_00b5ff00(incoming));
        auto* const current_device = static_cast<IDirect3DDevice9*>(
            pointer(word(renderer, 0x1a10)));
        const auto call = reinterpret_cast<SetDeclaration>(word(captured_device_table, 0x15c));
        (void)call(current_device, declaration);
    }
    put(renderer, 0x1bac, word(renderer, 0x1bac) + 1u);
    const auto current_mode = globals.mode_00;
    cleanup.armed = false; // Disarm before normal leave, including its failures.
    if (current_mode != 0) {
        // Native loads four bytes; B33B00 ignores the word. Only its initialized
        // low byte is read here; the three untouched padding bytes have no use.
        const std::uint32_t ignored = guard.entered_00;
        const void* const saved_renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, ignored, globals);
    }
}
} // namespace bsp

