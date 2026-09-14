#include "bsp/native_crt_seh4_nested_handler.hpp"
#include "bsp/native_crt_canonical_failure.hpp"
#include "bsp/native_crt_local_cleanup_call.hpp"
#include "bsp/native_crt_nlg_notify.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native SEH4 loop and nested handler require MSVC Win32.
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstddef>

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(offsetof(EXCEPTION_RECORD, ExceptionFlags) == 4);
static_assert(EXCEPTION_UNWINDING == 2 && EXCEPTION_EXIT_UNWIND == 4);
static_assert(ExceptionContinueSearch == 1 && ExceptionCollidedUnwind == 3);

// The exact FS publication/restoration is paired with the explicit external-PROC
// .SAFESEH metadata object. Its actual handler-table membership is a link gate;
// MSVC cannot infer that separate object while diagnosing this inline assembly.
#pragma warning(push)
#pragma warning(disable : 4733)
__declspec(naked) void __cdecl unwind_native_crt_local_scopes_00c0dbc4(
    const volatile std::uint32_t*, void*, std::uint32_t) {
    __asm {
        push ebx                                              // 00c0dbc4
        push esi                                              // 00c0dbc5
        push edi                                              // 00c0dbc6
        mov edx, dword ptr [esp + 10h]                        // 00c0dbc7
        mov eax, dword ptr [esp + 14h]                        // 00c0dbcb
        mov ecx, dword ptr [esp + 18h]                        // 00c0dbcf
        push ebp                                              // 00c0dbd3
        push edx                                              // 00c0dbd4
        push eax                                              // 00c0dbd5
        push ecx                                              // 00c0dbd6
        push ecx                                              // 00c0dbd7
        push offset handle_native_crt_seh4_nested_unwind_00c0dc54 // 00c0dbd8
        push dword ptr fs:[0]                                // 00c0dbdd
        // Exact A1 absolute load. MSVC treats the bare constant bracket form
        // as an immediate; adding DS would insert a non-native prefix.
        _emit 0xa1                                           // 00c0dbe4
        _emit 0x90
        _emit 0x55
        _emit 0xe1
        _emit 0x00
        xor eax, esp                                         // 00c0dbe9
        mov dword ptr [esp + 8], eax                          // 00c0dbeb
        mov dword ptr fs:[0], esp                             // 00c0dbef
    loop_reload:
        mov eax, dword ptr [esp + 30h]                        // 00c0dbf6
        mov ebx, dword ptr [eax + 8]                          // 00c0dbfa
        mov ecx, dword ptr [esp + 2ch]                        // 00c0dbfd
        xor ebx, dword ptr [ecx]                              // 00c0dc01
        mov esi, dword ptr [eax + 0ch]                        // 00c0dc03
        cmp esi, -2                                          // 00c0dc06
        je loop_done                                         // 00c0dc09
        mov edx, dword ptr [esp + 34h]                        // 00c0dc0b
        cmp edx, -2                                          // 00c0dc0f
        je loop_record                                       // 00c0dc12
        cmp esi, edx                                         // 00c0dc14
        jbe loop_done                                        // 00c0dc16
    loop_record:
        lea esi, [esi + esi * 2]                              // 00c0dc18
        lea ebx, [ebx + esi * 4 + 10h]                        // 00c0dc1b
        mov ecx, dword ptr [ebx]                              // 00c0dc1f
        mov dword ptr [eax + 0ch], ecx                        // 00c0dc21
        cmp dword ptr [ebx + 4], 0                            // 00c0dc24
        jne loop_reload                                      // 00c0dc28
        push 101h                                            // 00c0dc2a
        mov eax, dword ptr [ebx + 8]                          // 00c0dc2f
        call notify_native_crt_nlg_00c16879                    // 00c0dc32
        mov ecx, 1                                           // 00c0dc37
        mov eax, dword ptr [ebx + 8]                          // 00c0dc3c
        call call_native_crt_cleanup_00c16898                  // 00c0dc3f
        jmp loop_reload                                      // 00c0dc44
    loop_done:
        pop dword ptr fs:[0]                                 // 00c0dc46
        add esp, 18h                                         // 00c0dc4d
        pop edi                                              // 00c0dc50
        pop esi                                              // 00c0dc51
        pop ebx                                              // 00c0dc52
        ret                                                  // 00c0dc53
    }
}

#pragma warning(pop)

__declspec(naked) EXCEPTION_DISPOSITION __cdecl handle_native_crt_seh4_nested_unwind_00c0dc54(
    _EXCEPTION_RECORD*, void*, _CONTEXT*, void*) {
    __asm {
        mov ecx, dword ptr [esp + 4]                          // 00c0dc54
        test dword ptr [ecx + 4], 6                           // 00c0dc58
        mov eax, 1                                           // 00c0dc5f
        je handler_return                                    // 00c0dc64
        mov eax, dword ptr [esp + 8]                          // 00c0dc66
        mov ecx, dword ptr [eax + 8]                          // 00c0dc6a
        xor ecx, eax                                         // 00c0dc6d
        call check_native_crt_canonical_cookie_00bfe120        // 00c0dc6f
        push ebp                                             // 00c0dc74
        mov ebp, dword ptr [eax + 18h]                        // 00c0dc75
        push dword ptr [eax + 0ch]                            // 00c0dc78
        push dword ptr [eax + 10h]                            // 00c0dc7b
        push dword ptr [eax + 14h]                            // 00c0dc7e
        call unwind_native_crt_local_scopes_00c0dbc4           // 00c0dc81
        add esp, 0ch                                         // 00c0dc86
        pop ebp                                              // 00c0dc89
        mov eax, dword ptr [esp + 8]                          // 00c0dc8a
        mov edx, dword ptr [esp + 10h]                        // 00c0dc8e
        mov dword ptr [edx], eax                              // 00c0dc92
        mov eax, 3                                           // 00c0dc94
    handler_return:
        ret                                                  // 00c0dc99
    }
}
} // namespace bsp
