#include "bsp/native_tick_subnode_unlink_caller.hpp"

#include "bsp/native_pending_registry_getter.hpp"
#include "bsp/native_tick_sublist_unlink.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

// Source-only adaptation: explicit getter reference arguments, late node-cell
// dereference, and RET 0Ch for three borrowed stack references. Keep the owned
// effect order and raw current-EDI store; no extra cleanup or EDX clear.
__declspec(naked) void __fastcall unlink_native_tick_subnode_00875960(
    void*, std::uint32_t, void* volatile&, void* volatile&, void* volatile&) {
    __asm {
        push ebx                              // 00875960
        push esi                              // 00875961
        push edi                              // 00875962
        mov ebx, ecx                          // 00875963
        push dword ptr [esp + 18h]            // Source: actual M cell address
        push dword ptr [esp + 18h]            // Source: actual F cell address
        call get_native_pending_registry_00875280 // 00875965: current Source
        add esp, 8                            // Source: cdecl reference arguments
        mov esi, dword ptr [eax + 4]           // 0087596A
        test esi, esi                         // 0087596D
        jz short node_input                   // 0087596F
        push esi                              // 00875971
        call dword ptr [EnterCriticalSection]  // 00875972: current Win32 import
        add dword ptr [esi + 18h], 1           // 00875978
    node_input:
        mov edi, dword ptr [esp + 10h]         // 0087597C: actual Source input cell
        mov edi, dword ptr [edi]               // Source: late current node read
        push edi                              // 00875980
        lea ecx, [ebx + 1ch]                   // 00875981
        call unlink_native_tick_sublist_node_00874e60 // 00875984: current Source
        test esi, esi                         // 00875989
        mov dword ptr [edi + 4], 0             // 0087598B
        jz short unlink_complete              // 00875992
        add dword ptr [esi + 18h], -1          // 00875994
        push esi                              // 00875998
        call dword ptr [LeaveCriticalSection]  // 00875999: current Win32 import
    unlink_complete:
        pop edi                               // 0087599F
        pop esi                               // 008759A0
        pop ebx                               // 008759A1
        ret 0ch                               // Source: three references, not RET4
    }
}

} // namespace bsp
