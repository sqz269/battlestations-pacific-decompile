#include "bsp/native_renderer_query_terminal.hpp"
#include "bsp/native_render_buffer_unregistration.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word load(const volatile void* base, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(reinterpret_cast<Word>(base) + offset);
}
void store(void* base, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(reinterpret_cast<Word>(base) + offset) = value;
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
Word saved_guard_word(const void* record, Word byte_offset) noexcept {
    Word result;
    __asm {
        mov eax, record
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
struct BaseCleanup {
    void* owner;
    bool armed{true};
    ~BaseCleanup() noexcept { if (armed) destroy_native_renderer_query_base_00b5fd40(owner); }
};
using ComRelease = Word (__stdcall*)(void*);
} // namespace

std::uint32_t remove_native_renderer_query_pointer_00b25290(
    void* actual_array, const void* actual_value_word) noexcept {
    // Entire103-byte native body equals B25300, without operand masking.
    return remove_native_renderer_vertex_pointer_00b25300(actual_array, actual_value_word);
}

void unregister_native_renderer_query_00b27cf0(void* renderer, void* query,
    NativeRendererSynchronizationGlobals& globals) {
    NativeRendererOptionalGuardStorage guard;
    const bool entry_enabled = globals.mode_00 != 0;
    if (entry_enabled) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    const Word raw_query = reinterpret_cast<Word>(query);
    (void)remove_native_renderer_query_pointer_00b25290(
        pointer(reinterpret_cast<Word>(renderer) + 0x19a0), &raw_query);
    if (globals.mode_00 != 0) {
        __assume(entry_enabled); // Documented native valid-input domain.
        // Copy the native ignored DWORD, including unwritten padding, with an
        // isolated MOV rather than an indeterminate C++ scalar evaluation.
        const std::uint32_t ignored = saved_guard_word(&guard, 0);
        const void* const saved_renderer = pointer(saved_guard_word(&guard, 4));
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, ignored, globals);
    }
}

__declspec(naked) void __fastcall destroy_native_renderer_query_base_00b5fd40(void*) noexcept {
    __asm {
        mov dword ptr [ecx], 00d62ab0h
        jmp destroy_native_ref_counted_base_00bd30f0
    }
}

void destroy_native_renderer_query_00b5fda0(void* query,
    NativeRendererQueryTerminalContext& context) {
    store(query, 0, 0x00d62ad0);
    void* const captured_com = pointer(load(query, 0x10));
    BaseCleanup cleanup{query};
    if (captured_com) {
        void* const table = pointer(load(captured_com));
        const auto release = reinterpret_cast<ComRelease>(load(table, 8));
        (void)release(captured_com);
        store(query, 0x10, 0);
    }
    void* const renderer = context.actual_renderer_00f8d394;
    const Word current_renderer_profile = load(renderer);
    __assume(current_renderer_profile == 0x00d5f0a8);
    const Word terminal = load(context.actual_renderer_profile_00d5f0a8, 0x28);
    __assume(terminal == 0x00b27cf0);
    unregister_native_renderer_query_00b27cf0(renderer, query,
        context.actual_synchronization_0108d6dc);
    cleanup.armed = false;
    destroy_native_renderer_query_base_00b5fd40(query);
}

void* delete_native_renderer_query_00b5fe40(void* query, std::uint32_t flags,
    NativeRendererQueryTerminalContext& context) {
    destroy_native_renderer_query_00b5fda0(query, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(query);
    return query;
}

void invoke_native_renderer_query_delete_00bd30e0(void* query,
    NativeRendererQueryTerminalContext& context) {
    if (query == nullptr) return;
    const Word current_query_profile = load(query);
    __assume(current_query_profile == 0x00d62ad0);
    const Word terminal = load(context.actual_query_profile_00d62ad0, 4);
    __assume(terminal == 0x00b5fe40);
    (void)delete_native_renderer_query_00b5fe40(query, 1, context);
}
} // namespace bsp
