#include "bsp/native_crt_pow_fallback.hpp"
#include "bsp/native_crt_x87_control.hpp"
#include "bsp/native_crt_x87_double_load.hpp"
#include "bsp/native_crt_x87_power_leaves.hpp"
#include "bsp/native_crt_x87_result_status.hpp"
#include "bsp/native_crt_x87_error_dispatch.hpp"
#include "bsp/native_crt_pow_special.hpp"

namespace bsp {
namespace {

// New private11-byte address-binding thunk. LEA changes EDX only; JMP adds
// no return slot. Both parent continuations discard the previous EDX.
__declspec(naked) void __cdecl bind_native_pow_parity_half() {
    __asm {
        lea edx, [edi - 0d50h]
        jmp classify_native_crt_power_exponent_00bfed32
    }
}
} // namespace

// Full BFEB6D instruction stream. The two marked XCHGs add four bytes;
// every other original instruction retains its order and stack accesses.
// Each _emit block is ONE decoded original memory instruction with only
// its address operand rebound; branch displacements account for the XCHGs.
// MSVC C4414 diagnoses external Jcc even with explicit near ptr. These four
// original branches are already six-byte near forms; the full relocation
// proof verifies each encoding. Suppress only this diagnostic in this body.
#pragma warning(push)
#pragma warning(disable: 4414)
__declspec(naked) void __cdecl evaluate_native_crt_pow_fallback_00bfeb6d() {
    __asm {
        mov ecx, eax // 00bfeb6d
        push eax // 00bfeb6f
        fwait  // 00bfeb70
        fnstcw word ptr [esp] // 00bfeb71
        cmp word ptr [esp], 0x27f // 00bfeb74
        je L_00bfeb81 // 00bfeb7a
        call prepare_native_crt_x87_control_00c083a5 // 00bfeb7c
    L_00bfeb81:
        and ecx, 0x7ff00000 // 00bfeb81
        lea edx, [esp + 8] // 00bfeb87
        cmp ecx, 0x7ff00000 // 00bfeb8b
        je L_00bfec34 // 00bfeb91
        call load_native_crt_double_x87_00c083d5 // 00bfeb97
        je L_00bfec30 // 00bfeb9c
        test eax, 0x7ff00000 // 00bfeba2
        je L_00bfeca3 // 00bfeba7
    L_00bfebad:
        mov cl, byte ptr [esp + 0xf] // 00bfebad
        and cl, 0x80 // 00bfebb1
        jne L_00bfed0d // 00bfebb4
    L_00bfebba:
        fyl2x  // 00bfebba
        call exp2_native_crt_x87_00c08390 // 00bfebbc
        cmp cl, 1 // 00bfebc1
        jne L_00bfebc8 // 00bfebc4
        fchs  // 00bfebc6
    L_00bfebc8:
        // 00bfebc8: CMP actual 0109DD78 -> EBP+disp32(0).
        _emit 083h
        _emit 0bdh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        jne near ptr restore_native_crt_x87_control_tail_00c0842e // 00bfebcf
        // 00bfebd5: LEA actual 00E15858 -> EDI+disp32(-3400).
        _emit 08dh
        _emit 08fh
        _emit 0b8h
        _emit 0f2h
        _emit 0ffh
        _emit 0ffh
        mov edx, 0x1d // 00bfebdb
        jmp finish_native_crt_x87_result_status_tail_00c08479 // 00bfebe0
    L_00bfebe5:
        // 00bfebe5: CMP actual 0109DD78 -> EBP+disp32(0).
        _emit 083h
        _emit 0bdh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        jne near ptr restore_native_crt_x87_control_tail_00c0842e // 00bfebec
        // 00bfebf2: LEA actual 00E15858 -> EDI+disp32(-3400).
        _emit 08dh
        _emit 08fh
        _emit 0b8h
        _emit 0f2h
        _emit 0ffh
        _emit 0ffh
        mov edx, 0x1d // 00bfebf8
        call dispatch_native_crt_binary_error_00c08330 // 00bfebfd
        pop edx // 00bfec02
        ret  // 00bfec03
    L_00bfec04:
        lea edx, [esp + 8] // 00bfec04
        call load_native_crt_double_x87_00c083d5 // 00bfec08
        test byte ptr [esp + 0x16], 8 // 00bfec0d
        jne L_00bfec17 // 00bfec12
        inc ecx // 00bfec14
        jmp L_00bfec4a // 00bfec15
    L_00bfec17:
        faddp st(1), st(0) // 00bfec17
        mov eax, 1 // 00bfec19
        jmp L_00bfebe5 // 00bfec1e
    L_00bfec20:
        test byte ptr [esp + 0xe], 8 // 00bfec20
        jne L_00bfec17 // 00bfec25
    L_00bfec27:
        faddp st(1), st(0) // 00bfec27
        mov eax, 7 // 00bfec29
        jmp L_00bfebe5 // 00bfec2e
    L_00bfec30:
        xor ecx, ecx // 00bfec30
        jmp L_00bfec4a // 00bfec32
    L_00bfec34:
        xor ecx, ecx // 00bfec34
        and eax, 0xfffff // 00bfec36
        or eax, dword ptr [esp + 0x10] // 00bfec3b
        jne L_00bfec04 // 00bfec3f
        lea edx, [esp + 8] // 00bfec41
        call load_native_crt_double_x87_00c083d5 // 00bfec45
    L_00bfec4a:
        mov eax, dword ptr [esp + 0xc] // 00bfec4a
        mov edx, eax // 00bfec4e
        and eax, 0x7ff00000 // 00bfec50
        and edx, 0xfffff // 00bfec55
        cmp eax, 0x7ff00000 // 00bfec5b
        jne L_00bfec68 // 00bfec60
        or edx, dword ptr [esp + 8] // 00bfec62
        jne L_00bfec20 // 00bfec66
    L_00bfec68:
        test ecx, ecx // 00bfec68
        jne L_00bfec27 // 00bfec6a
        sub esp, 0x74 // 00bfec6c
        mov ecx, esp // 00bfec6f
        push ecx // 00bfec71
        sub esp, 0x10 // 00bfec72
        fstp qword ptr [esp] // 00bfec75
        fstp qword ptr [esp + 8] // 00bfec78
        fwait  // 00bfec7c
        fnsave dword ptr [ecx + 8] // 00bfec7d
        // New context binding before the original special CALL; no FP/flags/stack effect.
        xchg ebx, esi
        call evaluate_native_crt_pow_special_00c19e24 // 00bfec80
        // Restore both caller bindings only after normal special-provider return.
        xchg ebx, esi
        add esp, 0x10 // 00bfec85
        pop ecx // 00bfec88
        frstor dword ptr [ecx + 8] // 00bfec89
        fld qword ptr [ecx] // 00bfec8c
        add esp, 0x74 // 00bfec8e
        test eax, eax // 00bfec91
        je near ptr restore_native_crt_x87_control_tail_00c0842e // 00bfec93
        mov eax, 1 // 00bfec99
        jmp L_00bfebe5 // 00bfec9e
    L_00bfeca3:
        mov eax, dword ptr [esp + 0xc] // 00bfeca3
        and eax, 0xfffff // 00bfeca7
        or eax, dword ptr [esp + 8] // 00bfecac
        jne L_00bfebad // 00bfecb0
        fstp st(0) // 00bfecb6
        mov eax, dword ptr [esp + 0x14] // 00bfecb8
        and eax, 0x7fffffff // 00bfecbc
        or eax, dword ptr [esp + 0x10] // 00bfecc1
        je L_00bfed04 // 00bfecc5
        call bind_native_pow_parity_half // 00bfecc7
        mov ch, byte ptr [esp + 0xf] // 00bfeccc
        shr ch, 7 // 00bfecd0
        test dword ptr [esp + 0x17], 0x80 // 00bfecd3
        je L_00bfecf3 // 00bfecdb
        // 00bfecdd: FLD actual 00E166D0 -> EDI+disp32(304).
        _emit 0dbh
        _emit 0afh
        _emit 030h
        _emit 001h
        _emit 000h
        _emit 000h
        // 00bfece3: exact TEST CH,CL (84 CD); MSVC otherwise commutes the operands.
        _emit 084h
        _emit 0cdh
        je L_00bfece9 // 00bfece5
        fchs  // 00bfece7
    L_00bfece9:
        mov eax, 2 // 00bfece9
        jmp L_00bfebe5 // 00bfecee
    L_00bfecf3:
        fldz  // 00bfecf3
        // 00bfecf5: exact TEST CH,CL (84 CD); MSVC otherwise commutes the operands.
        _emit 084h
        _emit 0cdh
        je near ptr restore_native_crt_x87_control_tail_00c0842e // 00bfecf7
        fchs  // 00bfecfd
        jmp restore_native_crt_x87_control_tail_00c0842e // 00bfecff
    L_00bfed04:
        fstp st(0) // 00bfed04
        fld1  // 00bfed06
        jmp restore_native_crt_x87_control_tail_00c0842e // 00bfed08
    L_00bfed0d:
        fld st(1) // 00bfed0d
        call bind_native_pow_parity_half // 00bfed0f
        fchs  // 00bfed14
        test cl, cl // 00bfed16
        jne L_00bfebba // 00bfed18
        fstp st(0) // 00bfed1e
        fstp st(0) // 00bfed20
        // 00bfed22: FLD actual 00E15C70 -> EDI+disp32(-2352).
        _emit 0dbh
        _emit 0afh
        _emit 0d0h
        _emit 0f6h
        _emit 0ffh
        _emit 0ffh
        mov eax, 1 // 00bfed28
        jmp L_00bfebe5 // 00bfed2d
    }
}
#pragma warning(pop)

} // namespace bsp
