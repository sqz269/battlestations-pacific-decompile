#include "bsp/native_traffic_dword_resize.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_traffic_dword_insert.hpp"
#include "bsp/native_traffic_pointer_erase.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);

__declspec(naked) std::uint32_t __fastcall resize_native_traffic_dwords_00495ae0(
    void*, std::uint32_t, std::uint32_t, std::uint32_t) {
    __asm {
        sub esp, 8 //00495AE0: actual uninitialized output-area preimage
        push ebx //00495AE3
        push ebp //00495AE4
        push esi //00495AE5
        mov esi, ecx //00495AE6
        mov ecx, dword ptr [esi + 4] //00495AE8
        test ecx, ecx //00495AEB
        push edi //00495AED: preserve TEST flags
        jne native_00495af4 //00495AEE
        xor eax, eax //00495AF0
        jmp native_00495afc //00495AF2
    native_00495af4:
        mov eax, dword ptr [esi + 8] //00495AF4
        sub eax, ecx //00495AF7
        sar eax, 2 //00495AF9
    native_00495afc:
        mov ebx, dword ptr [esp + 0x1c] //00495AFC: actual requested count
        cmp eax, ebx //00495B00
        jae native_00495b3b //00495B02
        test ecx, ecx //00495B04
        jne native_00495b0c //00495B06
        xor edi, edi //00495B08
        jmp native_00495b14 //00495B0A
    native_00495b0c:
        mov edi, dword ptr [esi + 8] //00495B0C
        sub edi, ecx //00495B0F
        sar edi, 2 //00495B11
    native_00495b14:
        mov ebp, dword ptr [esi + 8] //00495B14: captured end
        cmp ecx, ebp //00495B17
        jbe native_00495b20 //00495B19
        call invoke_native_invalid_parameter_00bf6713 //00495B1B: genuine returning SDK boundary
    native_00495b20:
        lea eax, [esp + 0x20] //00495B20: CURRENT actual value argument cell
        push eax //00495B24
        sub ebx, edi //00495B25
        push ebx //00495B27
        push ebp //00495B28
        push esi //00495B29: actual opaque first child word
        mov ecx, esi //00495B2A
        call insert_native_traffic_dwords_00493120 //00495B2C: child RET10
        pop edi //00495B31
        pop esi //00495B32
        pop ebp //00495B33
        pop ebx //00495B34
        add esp, 8 //00495B35
        ret 8 //00495B38
    native_00495b3b:
        test ecx, ecx //00495B3B
        je native_00495b89 //00495B3D
        mov ebp, dword ptr [esi + 8] //00495B3F: reread actual end
        mov eax, ebp //00495B42
        sub eax, ecx //00495B44
        sar eax, 2 //00495B46
        cmp ebx, eax //00495B49
        jae native_00495b89 //00495B4B
        cmp ecx, ebp //00495B4D
        jbe native_00495b56 //00495B4F
        call invoke_native_invalid_parameter_00bf6713 //00495B51: may return and mutate actual cells
    native_00495b56:
        mov edi, dword ptr [esi + 4] //00495B56: reload after diagnostic
        cmp edi, dword ptr [esi + 8] //00495B59
        jbe native_00495b63 //00495B5C
        call invoke_native_invalid_parameter_00bf6713 //00495B5E
    native_00495b63:
        mov dword ptr [esp + 0x14], edi //00495B63: only second output-area DWORD
        lea edi, [edi + ebx*4] //00495B67: captured count, DWORD wrap
        cmp edi, dword ptr [esi + 8] //00495B6A
        ja native_00495b74 //00495B6D
        cmp edi, dword ptr [esi + 4] //00495B6F
        jae native_00495b79 //00495B72
    native_00495b74:
        call invoke_native_invalid_parameter_00bf6713 //00495B74
    native_00495b79:
        push ebp //00495B79: captured old end
        push esi //00495B7A: last owner
        push edi //00495B7B: captured cut
        push esi //00495B7C: first owner
        lea ecx, [esp + 0x20] //00495B7D: actual eight-byte output area
        push ecx //00495B81
        mov ecx, esi //00495B82
        call erase_native_traffic_pointer_range_00488ff0 //00495B84: child RET14
    native_00495b89:
        pop edi //00495B89
        pop esi //00495B8A
        pop ebp //00495B8B
        pop ebx //00495B8C
        add esp, 8 //00495B8D
        ret 8 //00495B90
    }
}

} // namespace bsp
