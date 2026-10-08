#include "bsp/native_command_target_initialize.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This literal native entry requires MSVC Win32.
#endif

namespace bsp {

// 004F1830..004F1870, full-body SHA-256:
// d63a15a125ffd391d4da60d40c27c914a8b8cd8c571f6fe0abdae1a582c723f0
// Saved-program listing and original PE agree. The three addresses lie in the
// writable .data virtual zero-fill tail; loader zeroes do not prove immutable
// runtime values or absence of writers. See NATIVE_COMMAND_TARGET_INITIALIZE_CC12.md.
__declspec(naked) void* __fastcall initialize_native_command_target_004f1830(
    void*, void*) noexcept {
    __asm {
        // Numeric MOVSS encodings avoid an extra DS prefix from MSVC's inline
        // assembler. These are reads of the actual native absolute addresses.
        // 004F1830: movss xmm0, dword ptr [00F87574h]
        _emit 0xF3
        _emit 0x0F
        _emit 0x10
        _emit 0x05
        _emit 0x74
        _emit 0x75
        _emit 0xF8
        _emit 0x00
        mov eax, ecx
        movss dword ptr [eax + 08h], xmm0
        // 004F183F: movss xmm0, dword ptr [00F87578h]
        _emit 0xF3
        _emit 0x0F
        _emit 0x10
        _emit 0x05
        _emit 0x78
        _emit 0x75
        _emit 0xF8
        _emit 0x00
        movss dword ptr [eax + 0Ch], xmm0
        // 004F184C: movss xmm0, dword ptr [00F8757Ch]
        _emit 0xF3
        _emit 0x0F
        _emit 0x10
        _emit 0x05
        _emit 0x7C
        _emit 0x75
        _emit 0xF8
        _emit 0x00
        movss dword ptr [eax + 10h], xmm0
        xorps xmm0, xmm0
        xor ecx, ecx
        mov word ptr [eax], cx
        mov dword ptr [eax + 04h], ecx
        mov byte ptr [eax], cl
        mov word ptr [eax + 02h], cx
        movss dword ptr [eax + 14h], xmm0
        ret
    }
}

}  // namespace bsp
