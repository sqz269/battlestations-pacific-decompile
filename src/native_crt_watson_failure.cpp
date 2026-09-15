#include "bsp/native_crt_watson_failure.hpp"
#include "bsp/native_crt_cookie_initialization.hpp"
#include "bsp/native_crt_memset.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT Watson failure requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeCrtWatsonBindings) == 32);
static_assert(sizeof(EXCEPTION_RECORD) == 0x50);
static_assert(sizeof(CONTEXT) == 0x2cc);
static_assert(sizeof(EXCEPTION_POINTERS) == 8);

// Complete native 00C185A4; qualified entry/capture domain is in the header.
__declspec(naked) void __cdecl report_native_crt_gsfailure_00c185a4(
    const NativeCrtWatsonBindings&) {
    __asm {
        // Native 00c185a4
        push ebp
        // Native 00c185a5
        mov ebp, esp
        // Native 00c185a7
        sub esp, 0x328
        // Native 00c185ad
        push ecx
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 0x14]
        mov dword ptr [ecx + 0xb0], eax
        // Native 00c185b2
        mov eax, dword ptr [esp]
        mov dword ptr [ecx + 0xac], eax
        // Native 00c185b8
        mov dword ptr [ecx + 0xa8], edx
        // Native 00c185be
        mov dword ptr [ecx + 0xa4], ebx
        // Native 00c185c4
        mov dword ptr [ecx + 0xa0], esi
        // Native 00c185ca
        mov dword ptr [ecx + 0x9c], edi
        // Native 00c185d0
        mov word ptr [ecx + 0xc8], ss
        // Native 00c185d7
        mov word ptr [ecx + 0xbc], cs
        // Native 00c185de
        mov word ptr [ecx + 0x98], ds
        // Native 00c185e5
        mov word ptr [ecx + 0x94], es
        // Native 00c185ec
        mov word ptr [ecx + 0x90], fs
        // Native 00c185f3
        mov word ptr [ecx + 0x8c], gs
        // Native 00c185fa
        pushfd
        // Native 00c185fb
        pop dword ptr [ecx + 0xc0]
        pop ecx
        // Native 00c18601
        mov eax, dword ptr [ebp]
        // Native 00c18604
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 0x14]
        mov dword ptr [ecx + 0xb4], eax
        // Native 00c18609
        mov eax, dword ptr [ebp + 4]
        // Native 00c1860c
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 0x14]
        mov dword ptr [ecx + 0xb8], eax
        // Native 00c18611
        lea eax, [ebp + 8]
        // Native 00c18614
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 0x14]
        mov dword ptr [ecx + 0xc4], eax
        // Native 00c18619
        mov eax, dword ptr [ebp - 0x320]
        // Native 00c1861f
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 0x14]
        mov dword ptr [ecx + 0x0], 0x10001
        // Native 00c18629
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 0x14]
        mov eax, dword ptr [ecx + 0xb8]
        // Native 00c1862e
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 0x10]
        mov dword ptr [ecx + 0xc], eax
        // Native 00c18633
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 0x10]
        mov dword ptr [ecx + 0x0], 0xc0000409
        // Native 00c1863d
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 0x10]
        mov dword ptr [ecx + 0x4], 1
        // Native 00c18647
        mov eax, dword ptr [ebp + 8]
        mov eax, dword ptr [eax + 0]
        mov eax, dword ptr [eax]
        // Native 00c1864c
        mov dword ptr [ebp - 0x328], eax
        // Native 00c18652
        mov eax, dword ptr [ebp + 8]
        mov eax, dword ptr [eax + 4]
        mov eax, dword ptr [eax]
        // Native 00c18657
        mov dword ptr [ebp - 0x324], eax
        // Native 00c1865d
        call IsDebuggerPresent
        // Native 00c18663
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 0x18]
        mov dword ptr [ecx], eax
        // Native 00c18668
        mov ecx, dword ptr [ebp + 8]
        push dword ptr [ecx + 8]
        push 1
        // Native 00c1866a
        call clear_native_crt_debugger_hook_00c04ef3
        // Native 00c1866f
        pop ecx
        lea esp, [esp + 4]
        // Native 00c18670
        push 0
        // Native 00c18672
        call SetUnhandledExceptionFilter
        // Native 00c18678
        mov ecx, dword ptr [ebp + 8]
        push dword ptr [ecx + 0x1c]
        // Native 00c1867d
        call UnhandledExceptionFilter
        // Native 00c18683
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 0x18]
        cmp dword ptr [ecx], 0
        // Native 00c1868a
        jne native_00c18694
        // Native 00c1868c
        mov ecx, dword ptr [ebp + 8]
        push dword ptr [ecx + 8]
        push 1
        // Native 00c1868e
        call clear_native_crt_debugger_hook_00c04ef3
        // Native 00c18693
        pop ecx
        lea esp, [esp + 4]
    native_00c18694:
        push 0xc0000409
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

// Complete native 00BFE120; qualified entry/capture domain is in the header.
__declspec(naked) void __cdecl check_native_crt_cookie_00bfe120(
    const NativeCrtWatsonBindings&) {
    __asm {
        // Native 00bfe120
        push eax
        mov eax, dword ptr [esp + 8]
        mov eax, dword ptr [eax]
        cmp ecx, dword ptr [eax]
        pop eax
        // Native 00bfe126
        jne native_00bfe12a
        // Native 00bfe128
        _emit 0xf3
        ret
    native_00bfe12a:
        jmp report_native_crt_gsfailure_00c185a4
    }
}

// Complete native 00BF65BB; qualified entry/capture domain is in the header.
__declspec(naked) void __cdecl invoke_native_crt_watson_00bf65bb(
    const wchar_t*, const wchar_t*, const wchar_t*, std::uint32_t, std::uintptr_t, const NativeCrtWatsonBindings&) {
    __asm {
        // Native 00bf65bb
        push ebp
        // Native 00bf65bc
        lea ebp, [esp - 0x2a8]
        // Native 00bf65c3
        sub esp, 0x328
        // Native 00bf65c9
        mov eax, dword ptr [ebp + 0x2c4]
        mov eax, dword ptr [eax]
        mov eax, dword ptr [eax]
        // Native 00bf65ce
        xor eax, ebp
        // Native 00bf65d0
        mov dword ptr [ebp + 0x2a4], eax
        // Native 00bf65d6
        push esi
        // Native 00bf65d7
        mov dword ptr [ebp + 0x88], eax
        // Native 00bf65dd
        mov dword ptr [ebp + 0x84], ecx
        // Native 00bf65e3
        mov dword ptr [ebp + 0x80], edx
        // Native 00bf65e9
        mov dword ptr [ebp + 0x7c], ebx
        // Native 00bf65ec
        mov dword ptr [ebp + 0x78], esi
        // Native 00bf65ef
        mov dword ptr [ebp + 0x74], edi
        // Native 00bf65f2
        mov word ptr [ebp + 0xa0], ss
        // Native 00bf65f9
        mov word ptr [ebp + 0x94], cs
        // Native 00bf6600
        mov word ptr [ebp + 0x70], ds
        // Native 00bf6604
        mov word ptr [ebp + 0x6c], es
        // Native 00bf6608
        mov word ptr [ebp + 0x68], fs
        // Native 00bf660c
        mov word ptr [ebp + 0x64], gs
        // Native 00bf6610
        pushfd
        // Native 00bf6611
        pop dword ptr [ebp + 0x98]
        // Native 00bf6617
        mov esi, dword ptr [ebp + 0x2ac]
        // Native 00bf661d
        lea eax, [ebp + 0x2ac]
        // Native 00bf6623
        mov dword ptr [ebp + 0x9c], eax
        // Native 00bf6629
        mov dword ptr [ebp - 0x28], 0x10001
        // Native 00bf6630
        mov dword ptr [ebp + 0x90], esi
        // Native 00bf6636
        mov eax, dword ptr [eax - 4]
        // Native 00bf6639
        mov edx, dword ptr [ebp + 0x2c4]
        push dword ptr [edx + 0x0c]
        push 0x50
        // Native 00bf663b
        mov dword ptr [ebp + 0x8c], eax
        // Native 00bf6641
        lea eax, [ebp - 0x80]
        // Native 00bf6644
        push 0
        // Native 00bf6646
        push eax
        // Native 00bf6647
        call fill_native_crt_bytes_00bf79f0
        // Native 00bf664c
        lea eax, [ebp - 0x80]
        // Native 00bf664f
        mov dword ptr [ebp - 0x30], eax
        // Native 00bf6652
        lea eax, [ebp - 0x28]
        // Native 00bf6655
        add esp, 0x10
        // Native 00bf6658
        mov dword ptr [ebp - 0x80], 0xc000000d
        // Native 00bf665f
        mov dword ptr [ebp - 0x74], esi
        // Native 00bf6662
        mov dword ptr [ebp - 0x2c], eax
        // Native 00bf6665
        call IsDebuggerPresent
        // Native 00bf666b
        push 0
        // Native 00bf666d
        mov esi, eax
        // Native 00bf666f
        call SetUnhandledExceptionFilter
        // Native 00bf6675
        lea eax, [ebp - 0x30]
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
        mov ecx, dword ptr [ebp + 0x2c4]
        push dword ptr [ecx + 8]
        push 2
        // Native 00bf6689
        call clear_native_crt_debugger_hook_00c04ef3
        // Native 00bf668e
        pop ecx
        lea esp, [esp + 4]
    native_00bf668f:
        push 0xc000000d
        // Native 00bf6694
        call GetCurrentProcess
        // Native 00bf669a
        push eax
        // Native 00bf669b
        call TerminateProcess
        // Native 00bf66a1
        mov ecx, dword ptr [ebp + 0x2a4]
        // Native 00bf66a7
        xor ecx, ebp
        // Native 00bf66a9
        pop esi
        // Native 00bf66aa
        push dword ptr [ebp + 0x2c4]
        call check_native_crt_cookie_00bfe120
        lea esp, [esp + 4]
        // Native 00bf66af
        add ebp, 0x2a8
        // Native 00bf66b5
        leave
        // Native 00bf66b6
        ret
    }
}

} // namespace bsp
