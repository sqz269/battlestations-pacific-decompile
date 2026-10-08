#include "bsp/native_string_duplicate.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#pragma function(memcpy)

namespace bsp {
namespace {

// Source-only CDECL size adapter; no native reconstruction credit. The real
// canonical service retains current malloc/new-handler behavior and ownership.
__declspec(noinline) void* __cdecl allocate_native_string_bytes(
    std::size_t actual_bytes) {
    return singleton_lifetime_allocate(
        {SingletonAllocationKind::object, actual_bytes, actual_bytes});
}

}  // namespace

// Complete native57 SHA-256:
// 28510a4c8a423997dfca7d8e257c1b250a7bb9e79cce177fc9c501e7b64735c4
// Only the two CALL operands at [34,38) and [44,48) are rebound. All other
// 49 bytes retain the native scan, widths, stack timing and return behavior.
__declspec(naked) char* __fastcall duplicate_native_string_00438e40(
    const char*) {
    __asm {
        push ebx
        mov ebx, ecx
        test ebx, ebx
        jnz L_nonnull
        xor eax, eax
        pop ebx
        ret
    L_nonnull:
        mov eax, ebx
        lea edx, [eax + 1]
    L_scan:
        mov cl, byte ptr [eax]
        add eax, 1
        test cl, cl
        jnz L_scan
        push esi
        sub eax, edx
        lea esi, [eax + 1]
        push edi
        push esi
        call allocate_native_string_bytes
        push esi
        mov edi, eax
        push ebx
        push edi
        call memcpy
        add esp, 10h
        mov eax, edi
        pop edi
        pop esi
        pop ebx
        ret
    }
}

}  // namespace bsp
