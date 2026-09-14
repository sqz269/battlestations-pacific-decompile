#include "bsp/native_crt_canonical_failure.hpp"
#include "bsp/game_native_mutable_crt_data.hpp"
#include "bsp/native_crt_memset.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Canonical CRT failure native entries require MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(EXCEPTION_RECORD) == 0x50);
static_assert(sizeof(CONTEXT) == 0x2cc);
static_assert(sizeof(EXCEPTION_POINTERS) == 8);
static_assert(offsetof(game::NativeCrtFailureBlock, context) == 0x58);
static_assert(offsetof(game::NativeCrtFailureBlock, debugger) == 0x50);
static_assert(offsetof(CONTEXT, Eax) == 0xb0);
static_assert(offsetof(CONTEXT, Ecx) == 0xac);
static_assert(offsetof(CONTEXT, Edx) == 0xa8);
static_assert(offsetof(CONTEXT, Ebx) == 0xa4);
static_assert(offsetof(CONTEXT, Esi) == 0xa0);
static_assert(offsetof(CONTEXT, Edi) == 0x9c);
static_assert(offsetof(CONTEXT, SegSs) == 0xc8);
static_assert(offsetof(CONTEXT, SegCs) == 0xbc);
static_assert(offsetof(CONTEXT, SegDs) == 0x98);
static_assert(offsetof(CONTEXT, SegEs) == 0x94);
static_assert(offsetof(CONTEXT, SegFs) == 0x90);
static_assert(offsetof(CONTEXT, SegGs) == 0x8c);
static_assert(offsetof(CONTEXT, EFlags) == 0xc0);
static_assert(offsetof(CONTEXT, Ebp) == 0xb4);
static_assert(offsetof(CONTEXT, Eip) == 0xb8);
static_assert(offsetof(CONTEXT, Esp) == 0xc4);
static_assert(offsetof(CONTEXT, ContextFlags) == 0x0);
static_assert(std::is_same_v<decltype(&IsDebuggerPresent), BOOL (WINAPI*)()>);
static_assert(std::is_same_v<decltype(&GetCurrentProcess), HANDLE (WINAPI*)()>);
static_assert(std::is_same_v<decltype(&TerminateProcess), BOOL (WINAPI*)(HANDLE, UINT)>);
static_assert(std::is_same_v<decltype(&UnhandledExceptionFilter), LONG (WINAPI*)(EXCEPTION_POINTERS*)>);
static_assert(std::is_same_v<decltype(&SetUnhandledExceptionFilter),
    LPTOP_LEVEL_EXCEPTION_FILTER (WINAPI*)(LPTOP_LEVEL_EXCEPTION_FILTER)>);

// Exact instructions and data operands; only real provider/import relocations
// and source code addresses change. No canonical-owner accessor is called.

// Complete native 00BF65BB[252].
__declspec(naked) void __cdecl invoke_native_crt_canonical_watson_00bf65bb(const wchar_t*, const wchar_t*, const wchar_t*, std::uint32_t, std::uintptr_t) {
    __asm {
        // Native 00bf65bb
        push ebp
        // Native 00bf65bc
        lea ebp, [esp - 02a8h]
        // Native 00bf65c3
        sub esp, 0328h
        // Native 00bf65c9
        // mov eax, dword ptr [0xe15590]; exact absolute encoding (no added DS prefix).
        _emit 0xa1
        _emit 0x90
        _emit 0x55
        _emit 0xe1
        _emit 0x00
        // Native 00bf65ce
        xor eax, ebp
        // Native 00bf65d0
        mov dword ptr [ebp + 02a4h], eax
        // Native 00bf65d6
        push esi
        // Native 00bf65d7
        mov dword ptr [ebp + 088h], eax
        // Native 00bf65dd
        mov dword ptr [ebp + 084h], ecx
        // Native 00bf65e3
        mov dword ptr [ebp + 080h], edx
        // Native 00bf65e9
        mov dword ptr [ebp + 07ch], ebx
        // Native 00bf65ec
        mov dword ptr [ebp + 078h], esi
        // Native 00bf65ef
        mov dword ptr [ebp + 074h], edi
        // Native 00bf65f2
        mov word ptr [ebp + 0a0h], ss
        // Native 00bf65f9
        mov word ptr [ebp + 094h], cs
        // Native 00bf6600
        mov word ptr [ebp + 070h], ds
        // Native 00bf6604
        mov word ptr [ebp + 06ch], es
        // Native 00bf6608
        mov word ptr [ebp + 068h], fs
        // Native 00bf660c
        mov word ptr [ebp + 064h], gs
        // Native 00bf6610
        pushfd
        // Native 00bf6611
        pop dword ptr [ebp + 098h]
        // Native 00bf6617
        mov esi, dword ptr [ebp + 02ach]
        // Native 00bf661d
        lea eax, [ebp + 02ach]
        // Native 00bf6623
        mov dword ptr [ebp + 09ch], eax
        // Native 00bf6629
        mov dword ptr [ebp - 028h], 010001h
        // Native 00bf6630
        mov dword ptr [ebp + 090h], esi
        // Native 00bf6636
        mov eax, dword ptr [eax - 4]
        // Native 00bf6639
        push 050h
        // Native 00bf663b
        mov dword ptr [ebp + 08ch], eax
        // Native 00bf6641
        lea eax, [ebp - 080h]
        // Native 00bf6644
        push 0
        // Native 00bf6646
        push eax
        // Native 00bf6647
        // Count50h < 100h: complete CL body never reads its fourth
        // binding word on this path. Preserve original three words/ADD ESP,Ch.
        call fill_native_crt_bytes_00bf79f0
        // Native 00bf664c
        lea eax, [ebp - 080h]
        // Native 00bf664f
        mov dword ptr [ebp - 030h], eax
        // Native 00bf6652
        lea eax, [ebp - 028h]
        // Native 00bf6655
        add esp, 0ch
        // Native 00bf6658
        mov dword ptr [ebp - 080h], 0c000000dh
        // Native 00bf665f
        mov dword ptr [ebp - 074h], esi
        // Native 00bf6662
        mov dword ptr [ebp - 02ch], eax
        // Native 00bf6665
        call IsDebuggerPresent
        // Native 00bf666b
        push 0
        // Native 00bf666d
        mov esi, eax
        // Native 00bf666f
        call SetUnhandledExceptionFilter
        // Native 00bf6675
        lea eax, [ebp - 030h]
        // Native 00bf6678
        push eax
        // Native 00bf6679
        call UnhandledExceptionFilter
        // Native 00bf667f
        test eax, eax
        // Native 00bf6681
        jne native_00bf668f
        // Native 00bf6683
        test esi, esi
        // Native 00bf6685
        jne native_00bf668f
        // Native 00bf6687
        push 2
        // Native 00bf6689
        call clear_native_crt_canonical_debugger_hook_00c04ef3
        // Native 00bf668e
        pop ecx
    native_00bf668f:
        // Native 00bf668f
        push 0c000000dh
        // Native 00bf6694
        call GetCurrentProcess
        // Native 00bf669a
        push eax
        // Native 00bf669b
        call TerminateProcess
        // Native 00bf66a1
        mov ecx, dword ptr [ebp + 02a4h]
        // Native 00bf66a7
        xor ecx, ebp
        // Native 00bf66a9
        pop esi
        // Native 00bf66aa
        call check_native_crt_canonical_cookie_00bfe120
        // Native 00bf66af
        add ebp, 02a8h
        // Native 00bf66b5
        leave
        // Native 00bf66b6
        ret
    }
}

// Complete native 00BFE120[15].
__declspec(naked) void __cdecl check_native_crt_canonical_cookie_00bfe120() {
    __asm {
        // Native 00bfe120
        // cmp ecx, dword ptr [0xe15590]; exact absolute encoding (no added DS prefix).
        _emit 0x3b
        _emit 0x0d
        _emit 0x90
        _emit 0x55
        _emit 0xe1
        _emit 0x00
        // Native 00bfe126
        jne native_00bfe12a
        // Native 00bfe128
        _emit 0xf3
        ret
    native_00bfe12a:
        // Native 00bfe12a
        jmp report_native_crt_canonical_gsfailure_00c185a4
    }
}

// Complete native 00C185A4[260].
__declspec(naked) void __cdecl report_native_crt_canonical_gsfailure_00c185a4() {
    __asm {
        // Native 00c185a4
        push ebp
        // Native 00c185a5
        mov ebp, esp
        // Native 00c185a7
        sub esp, 0328h
        // Native 00c185ad
        // mov dword ptr [0x109e670], eax; exact absolute encoding (no added DS prefix).
        _emit 0xa3
        _emit 0x70
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c185b2
        // mov dword ptr [0x109e66c], ecx; exact absolute encoding (no added DS prefix).
        _emit 0x89
        _emit 0x0d
        _emit 0x6c
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c185b8
        // mov dword ptr [0x109e668], edx; exact absolute encoding (no added DS prefix).
        _emit 0x89
        _emit 0x15
        _emit 0x68
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c185be
        // mov dword ptr [0x109e664], ebx; exact absolute encoding (no added DS prefix).
        _emit 0x89
        _emit 0x1d
        _emit 0x64
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c185c4
        // mov dword ptr [0x109e660], esi; exact absolute encoding (no added DS prefix).
        _emit 0x89
        _emit 0x35
        _emit 0x60
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c185ca
        // mov dword ptr [0x109e65c], edi; exact absolute encoding (no added DS prefix).
        _emit 0x89
        _emit 0x3d
        _emit 0x5c
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c185d0
        // mov word ptr [0x109e688], ss; exact absolute encoding (no added DS prefix).
        _emit 0x66
        _emit 0x8c
        _emit 0x15
        _emit 0x88
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c185d7
        // mov word ptr [0x109e67c], cs; exact absolute encoding (no added DS prefix).
        _emit 0x66
        _emit 0x8c
        _emit 0x0d
        _emit 0x7c
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c185de
        // mov word ptr [0x109e658], ds; exact absolute encoding (no added DS prefix).
        _emit 0x66
        _emit 0x8c
        _emit 0x1d
        _emit 0x58
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c185e5
        // mov word ptr [0x109e654], es; exact absolute encoding (no added DS prefix).
        _emit 0x66
        _emit 0x8c
        _emit 0x05
        _emit 0x54
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c185ec
        // mov word ptr [0x109e650], fs; exact absolute encoding (no added DS prefix).
        _emit 0x66
        _emit 0x8c
        _emit 0x25
        _emit 0x50
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c185f3
        // mov word ptr [0x109e64c], gs; exact absolute encoding (no added DS prefix).
        _emit 0x66
        _emit 0x8c
        _emit 0x2d
        _emit 0x4c
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c185fa
        pushfd
        // Native 00c185fb
        // pop dword ptr [0x109e680]; exact absolute encoding (no added DS prefix).
        _emit 0x8f
        _emit 0x05
        _emit 0x80
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c18601
        mov eax, dword ptr [ebp]
        // Native 00c18604
        // mov dword ptr [0x109e674], eax; exact absolute encoding (no added DS prefix).
        _emit 0xa3
        _emit 0x74
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c18609
        mov eax, dword ptr [ebp + 4]
        // Native 00c1860c
        // mov dword ptr [0x109e678], eax; exact absolute encoding (no added DS prefix).
        _emit 0xa3
        _emit 0x78
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c18611
        lea eax, [ebp + 8]
        // Native 00c18614
        // mov dword ptr [0x109e684], eax; exact absolute encoding (no added DS prefix).
        _emit 0xa3
        _emit 0x84
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c18619
        mov eax, dword ptr [ebp - 0320h]
        // Native 00c1861f
        // mov dword ptr [0x109e5c0], 0x10001; exact absolute encoding (no added DS prefix).
        _emit 0xc7
        _emit 0x05
        _emit 0xc0
        _emit 0xe5
        _emit 0x09
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x01
        _emit 0x00
        // Native 00c18629
        // mov eax, dword ptr [0x109e678]; exact absolute encoding (no added DS prefix).
        _emit 0xa1
        _emit 0x78
        _emit 0xe6
        _emit 0x09
        _emit 0x01
        // Native 00c1862e
        // mov dword ptr [0x109e574], eax; exact absolute encoding (no added DS prefix).
        _emit 0xa3
        _emit 0x74
        _emit 0xe5
        _emit 0x09
        _emit 0x01
        // Native 00c18633
        // mov dword ptr [0x109e568], 0xc0000409; exact absolute encoding (no added DS prefix).
        _emit 0xc7
        _emit 0x05
        _emit 0x68
        _emit 0xe5
        _emit 0x09
        _emit 0x01
        _emit 0x09
        _emit 0x04
        _emit 0x00
        _emit 0xc0
        // Native 00c1863d
        // mov dword ptr [0x109e56c], 1; exact absolute encoding (no added DS prefix).
        _emit 0xc7
        _emit 0x05
        _emit 0x6c
        _emit 0xe5
        _emit 0x09
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // Native 00c18647
        // mov eax, dword ptr [0xe15590]; exact absolute encoding (no added DS prefix).
        _emit 0xa1
        _emit 0x90
        _emit 0x55
        _emit 0xe1
        _emit 0x00
        // Native 00c1864c
        mov dword ptr [ebp - 0328h], eax
        // Native 00c18652
        // mov eax, dword ptr [0xe15594]; exact absolute encoding (no added DS prefix).
        _emit 0xa1
        _emit 0x94
        _emit 0x55
        _emit 0xe1
        _emit 0x00
        // Native 00c18657
        mov dword ptr [ebp - 0324h], eax
        // Native 00c1865d
        call IsDebuggerPresent
        // Native 00c18663
        // mov dword ptr [0x109e5b8], eax; exact absolute encoding (no added DS prefix).
        _emit 0xa3
        _emit 0xb8
        _emit 0xe5
        _emit 0x09
        _emit 0x01
        // Native 00c18668
        push 1
        // Native 00c1866a
        call clear_native_crt_canonical_debugger_hook_00c04ef3
        // Native 00c1866f
        pop ecx
        // Native 00c18670
        push 0
        // Native 00c18672
        call SetUnhandledExceptionFilter
        // Native 00c18678
        push 0d6e1cch
        // Native 00c1867d
        call UnhandledExceptionFilter
        // Native 00c18683
        // cmp dword ptr [0x109e5b8], 0; exact absolute encoding (no added DS prefix).
        _emit 0x83
        _emit 0x3d
        _emit 0xb8
        _emit 0xe5
        _emit 0x09
        _emit 0x01
        _emit 0x00
        // Native 00c1868a
        jne native_00c18694
        // Native 00c1868c
        push 1
        // Native 00c1868e
        call clear_native_crt_canonical_debugger_hook_00c04ef3
        // Native 00c18693
        pop ecx
    native_00c18694:
        // Native 00c18694
        push 0c0000409h
        // Native 00c18699
        call GetCurrentProcess
        // Native 00c1869f
        push eax
        // Native 00c186a0
        call TerminateProcess
        // Native 00c186a6
        leave
        // Native 00c186a7
        ret
    }
}

// Complete native 00C04EF3[8].
__declspec(naked) void __cdecl clear_native_crt_canonical_debugger_hook_00c04ef3(std::uint32_t) {
    __asm {
        // Native 00c04ef3
        // and dword ptr [0x109eea8], 0; exact absolute encoding (no added DS prefix).
        _emit 0x83
        _emit 0x25
        _emit 0xa8
        _emit 0xee
        _emit 0x09
        _emit 0x01
        _emit 0x00
        // Native 00c04efa
        ret
    }
}
} // namespace bsp
