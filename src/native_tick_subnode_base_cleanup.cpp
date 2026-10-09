#include "bsp/native_tick_subnode_base_cleanup.hpp"

#include "bsp/native_pending_registry_getter.hpp"
#include "bsp/native_tick_sublist_unlink.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

// Two explicit Source publication references and cdecl getter arguments adapt
// the owned schedule. Preserve the pre-profile comparison, captured parent,
// raw current-EDI use after unlink, and TEST flags across the node+4 clear.
__declspec(naked) void __fastcall cleanup_native_tick_subnode_base_00875b30(
    void*, std::uint32_t, void* volatile&, void* volatile&) {
    __asm {
        push esi                              // 00875B30
        mov esi, ecx                          // 00875B31
        cmp dword ptr [esi + 8], 0             // 00875B33
        mov dword ptr [esi], 0cfd99ch          // 00875B37: opaque profile DWORD
        jnz short cleanup_slow                // 00875B3D: consumes the CMP flags
        cmp dword ptr [esi + 0ch], 0           // 00875B3F: fresh selected link read
        jz short cleanup_complete             // 00875B43
    cleanup_slow:
        push ebx                              // 00875B45
        mov ebx, dword ptr [esi + 4]           // 00875B46: capture parent now
        push edi                              // 00875B49
        push dword ptr [esp + 14h]            // Source: actual M cell address
        push dword ptr [esp + 14h]            // Source: actual F cell address
        call get_native_pending_registry_00875280 // 00875B4A: admitted Source
        add esp, 8                            // Source: cdecl reference arguments
        mov edi, dword ptr [eax + 4]           // 00875B4F: capture section once
        test edi, edi                         // 00875B52
        jz short unlink_node                  // 00875B54
        push edi                              // 00875B56
        call dword ptr [EnterCriticalSection]  // 00875B57: current Win32 import
        add dword ptr [edi + 18h], 1           // 00875B5D
    unlink_node:
        push esi                              // 00875B61: captured node argument
        lea ecx, [ebx + 1ch]                   // 00875B62: captured parent list
        call unlink_native_tick_sublist_node_00874e60 // 00875B65: admitted RET4 leaf
        test edi, edi                         // 00875B6A: actual current section
        mov dword ptr [esi + 4], 0             // 00875B6C: preserves TEST flags
        jz short slow_complete                // 00875B73
        add dword ptr [edi + 18h], -1          // 00875B75
        push edi                              // 00875B79
        call dword ptr [LeaveCriticalSection]  // 00875B7A: current Win32 import
    slow_complete:
        pop edi                               // 00875B80
        pop ebx                               // 00875B81
    cleanup_complete:
        pop esi                               // 00875B82
        ret 8                                 // 00875B83: Source two references
    }
}

} // namespace bsp
