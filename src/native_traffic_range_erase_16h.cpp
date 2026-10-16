#include "bsp/native_traffic_range_erase_16h.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_traffic_pod18_helpers.hpp"

#include <cstddef>
#include <cstring>

namespace bsp {
namespace {

// Genuine selected SDK API; its entire emitted bridge/SDK code is accounted
// separately from Native BF67A7. The four physical cdecl words are preserved.
int __cdecl move_traffic_erase_words(
    void* destination, std::size_t capacity,
    const void* source, std::size_t count) {
    return ::memmove_s(destination, capacity, source, count);
}

} // namespace

static_assert(sizeof(void*) == 4);

__declspec(naked) void* __fastcall erase_native_traffic_pod18_range_0048e400(
    void*, std::uint32_t, void*, std::uint32_t, void*, std::uint32_t, const void*) {
    __asm {
        push ebx                            // 0048E400
        mov ebx, dword ptr [esp + 0ch]       // capture actual first metadata
        test ebx, ebx
        push esi
        push edi
        mov edi, ecx
        jz pod18_diagnostic
        cmp ebx, dword ptr [esp + 1ch]
        jz pod18_positions
    pod18_diagnostic:
        call invoke_native_invalid_parameter_00bf6713 // 0048E413
    pod18_positions:
        mov esi, dword ptr [esp + 18h]
        mov eax, dword ptr [esp + 20h]
        cmp esi, eax
        je pod18_output
        mov ecx, dword ptr [edi + 8]
        push esi
        push ecx
        push eax
        call copy_native_traffic_pod18_dispatch_00486850 // 0048E42A
        add esp, 0ch
        mov dword ptr [edi + 8], eax         // actual child return publication
    pod18_output:
        mov eax, dword ptr [esp + 10h]       // CURRENT actual out, after publish
        pop edi
        mov dword ptr [eax + 4], esi
        pop esi
        mov dword ptr [eax], ebx
        pop ebx
        ret 14h
    }
}

__declspec(naked) void* __fastcall erase_native_traffic_dword_range_00489110(
    void*, std::uint32_t, void*, std::uint32_t, void*, std::uint32_t, const void*) {
    __asm {
        push ebp                            // 00489110
        mov ebp, dword ptr [esp + 0ch]       // capture actual first metadata
        test ebp, ebp
        push esi
        push edi
        mov edi, ecx
        jz dword_diagnostic
        cmp ebp, dword ptr [esp + 1ch]
        jz dword_positions
    dword_diagnostic:
        call invoke_native_invalid_parameter_00bf6713 // 00489123
    dword_positions:
        mov esi, dword ptr [esp + 18h]
        mov edx, dword ptr [esp + 20h]
        cmp esi, edx
        je dword_output
        mov eax, dword ptr [edi + 8]
        sub eax, edx
        sar eax, 2
        test eax, eax
        lea ecx, [eax * 4]
        push ebx
        lea ebx, [ecx + esi]                 // capture before any copy
        jle dword_publish
        push ecx
        push edx
        push ecx
        push esi
        call move_traffic_erase_words        // 0048914F: real current SDK policy
        add esp, 10h
    dword_publish:
        mov dword ptr [edi + 8], ebx         // also nonpositive/error return
        pop ebx
    dword_output:
        mov eax, dword ptr [esp + 10h]       // CURRENT actual out, after publish
        pop edi
        mov dword ptr [eax + 4], esi
        pop esi
        mov dword ptr [eax], ebp
        pop ebp
        ret 14h
    }
}

} // namespace bsp
