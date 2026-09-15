#include "bsp/native_renderer_existing_vertex_registration.hpp"
#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_render_buffer_unregistration.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native existing vertex registration requires MSVC Win32 assembly.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
namespace {
// These concrete source bridges consume one native stacked argument (RET4).
// The unused second parameter occupies EDX so the last parameter stays stacked.
// Forward the actual address, never a copied stream word or a host container.
__declspec(noinline) std::uint32_t __fastcall remove_existing_vertex_word(
    void* actual_array, void*, const void* actual_value_word) noexcept {
    return remove_native_renderer_vertex_pointer_00b25300(actual_array, actual_value_word);
}

__declspec(noinline) void __fastcall reserve_existing_vertex_words(
    void* actual_array, void*, std::uint32_t requested) {
    reserve_native_renderer_pointer_array_00b22d10(actual_array, requested);
}
} // namespace

__declspec(naked) void __fastcall register_native_renderer_existing_vertex_00b28a40(
    void*, void*, void*) {
    __asm {
        push esi // B28A40
        lea esi, [ecx + 1aach]
        lea eax, [esp + 8] // Actual S+4 public word, after ESI save.
        push eax
        mov ecx, esi
        call remove_existing_vertex_word // B28A4E -> full B25300 source.
        mov eax, dword ptr [esi + 8]
        cmp dword ptr [esi + 4], eax
        jne append_existing_vertex
        add eax, eax
        cmp eax, 1
        jg reserve_existing_capacity
        mov eax, 1
    reserve_existing_capacity:
        push eax
        mov ecx, esi
        call reserve_existing_vertex_words // B28A6A -> full B22D10 source.
    append_existing_vertex:
        mov ecx, dword ptr [esi + 4] // B28A6F: current after provider return.
        mov edx, dword ptr [esi]
        lea eax, [edx + ecx * 4]
        test eax, eax
        jz increment_existing_count
        mov ecx, dword ptr [esp + 8] // B28A7B: current public word.
        mov dword ptr [eax], ecx
    increment_existing_count:
        add dword ptr [esi + 4], 1 // B28A81: current after possible alias store.
        pop esi
        ret 4
    }
}
} // namespace bsp
