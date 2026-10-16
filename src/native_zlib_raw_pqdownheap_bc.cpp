#include "bsp/native_zlib_raw_pqdownheap_bc.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw pqdownheap register/frame entry requires MSVC Win32.
#endif

namespace {
// Complete 00BCDE90..00BCDF5B register body. EAX=backing, EDI=tree and
// [entry ESP+4]=actual mutable index word; stdcall spelling describes only
// the one stack word/RET4 shape. Ordinary C++ must not call this private body.
// Incoming CPU saves, current scratch and all address/index bits remain real.
__declspec(naked) void __stdcall pqdownheap_body(std::int32_t) noexcept {
    __asm {
        push ecx
        mov edx, dword ptr [eax + 01448h]
        push ebp
        push esi
        mov esi, dword ptr [esp + 010h]
        mov ebp, dword ptr [eax + esi * 4 + 0b54h]
        // Native 8D0C36: LEA ECX,[ESI+ESI*1], preserving all flags.
        _emit 08dh
        _emit 00ch
        _emit 036h
        cmp ecx, edx
        mov dword ptr [esp + 008h], ebp
        jg initial_no_child
        push ebx
    compare_children:
        jge chosen_child
        mov esi, dword ptr [eax + ecx * 4 + 0b58h]
        mov ebp, dword ptr [eax + ecx * 4 + 0b54h]
        movzx edx, word ptr [edi + esi * 4]
        movzx ebx, word ptr [edi + ebp * 4]
        cmp dx, bx
        jb choose_right
        jne reload_cached_root
        mov dl, byte ptr [esi + eax + 01450h]
        cmp dl, byte ptr [eax + ebp + 01450h]
        ja reload_cached_root
    choose_right:
        add ecx, 1
    reload_cached_root:
        mov ebp, dword ptr [esp + 00ch]
    chosen_child:
        mov esi, dword ptr [eax + ecx * 4 + 0b54h]
        movzx edx, word ptr [edi + ebp * 4]
        movzx ebx, word ptr [edi + esi * 4]
        cmp dx, bx
        jb finish_with_ecx
        jne promote_child
        mov dl, byte ptr [eax + ebp + 01450h]
        cmp dl, byte ptr [esi + eax + 01450h]
        jbe finish_with_edx
    promote_child:
        mov edx, dword ptr [esp + 014h]
        mov dword ptr [eax + edx * 4 + 0b54h], esi
        mov edx, dword ptr [eax + 01448h]
        mov dword ptr [esp + 014h], ecx
        add ecx, ecx
        cmp ecx, edx
        jle compare_children
    finish_with_ecx:
        mov ecx, dword ptr [esp + 014h]
        pop ebx
        pop esi
        mov dword ptr [eax + ecx * 4 + 0b54h], ebp
        pop ebp
        pop ecx
        ret 4
    finish_with_edx:
        mov edx, dword ptr [esp + 014h]
        pop ebx
        pop esi
        mov dword ptr [eax + edx * 4 + 0b54h], ebp
        pop ebp
        pop ecx
        ret 4
    initial_no_child:
        mov dword ptr [eax + esi * 4 + 0b54h], ebp
        pop esi
        pop ebp
        pop ecx
        ret 4
    }
}
} // namespace

// These three real Source arguments select the physical kernel inputs/frame.
// The Source caller's own return word is not used as an invented continuation.
// No ordinary returning cdecl ABI or Original frame/adoption is promised.
extern "C" __declspec(naked) __declspec(noreturn) void __cdecl
bsp_native_zlib_pqdownheap_frame_00bcde90(void*, const void*, void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov edi, dword ptr [esp + 8]
        mov esp, dword ptr [esp + 00ch]
        jmp pqdownheap_body
    }
}
