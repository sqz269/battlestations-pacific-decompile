#include "bsp/native_crt_x87_result_status.hpp"
#include "bsp/native_crt_x87_error_dispatch.hpp"

namespace bsp {

// Complete CW-top tail C08479..C0851C. No ordinary CALL adapter is added.
// EBX binds the actual original literal region; each six-byte _emit block
// is ONE decoded x87 instruction with only its address operand rebound.
// Waiting FSTSW is written as explicit FWAIT followed by FNSTSW AX; these
// are the exact original 9B DF E0 bytes, including each wait boundary.
__declspec(naked) void __cdecl finish_native_crt_x87_result_status_tail_00c08479() {
    __asm {
        sub esp, 8 // 00c08479
        fst qword ptr [esp] // 00c0847c
        mov eax, dword ptr [esp + 4] // 00c0847f
        add esp, 8 // 00c08483
        and eax, 0x7ff00000 // 00c08486
        je L_00c084ca // 00c0848b
        cmp eax, 0x7ff00000 // 00c0848d
        je L_00c084f3 // 00c08492
        mov ax, word ptr [esp] // 00c08494
        cmp ax, 0x27f // 00c08498
        je L_00c084c8 // 00c0849c
        and ax, 0x20 // 00c0849e
        jne L_00c084c5 // 00c084a2
        fwait  // 00c084a4
        fnstsw ax // 00c084a5
        and ax, 0x20 // 00c084a7
        je L_00c084c5 // 00c084ab
        mov eax, 8 // 00c084ad
    L_00c084b2:
        cmp edx, 0x1d // 00c084b2
        je L_00c084be // 00c084b5
        call dispatch_native_crt_unary_error_00c08347 // 00c084b7
        pop edx // 00c084bc
        ret  // 00c084bd
    L_00c084be:
        call dispatch_native_crt_binary_error_00c08330 // 00c084be
        pop edx // 00c084c3
        ret  // 00c084c4
    L_00c084c5:
        fldcw word ptr [esp] // 00c084c5
    L_00c084c8:
        pop edx // 00c084c8
        ret  // 00c084c9
    L_00c084ca:
        // 00c084ca: FLD qword [00D6A69C] -> [EBX+disp32(18h)].
        _emit 0ddh
        _emit 083h
        _emit 018h
        _emit 000h
        _emit 000h
        _emit 000h
        fxch st(1) // 00c084d0
        fscale  // 00c084d2
        fstp st(1) // 00c084d4
        fld st(0) // 00c084d6
        fabs  // 00c084d8
        // 00c084da: FCOMP qword [00D6A68C] -> [EBX+disp32(8h)].
        _emit 0dch
        _emit 09bh
        _emit 008h
        _emit 000h
        _emit 000h
        _emit 000h
        fwait  // 00c084e0
        fnstsw ax // 00c084e1
        sahf  // 00c084e3
        mov eax, 4 // 00c084e4
        jae L_00c084b2 // 00c084e9
        // 00c084eb: FMUL qword [00D6A6AC] -> [EBX+disp32(28h)].
        _emit 0dch
        _emit 08bh
        _emit 028h
        _emit 000h
        _emit 000h
        _emit 000h
        jmp L_00c084b2 // 00c084f1
    L_00c084f3:
        // 00c084f3: FLD qword [00D6A694] -> [EBX+disp32(10h)].
        _emit 0ddh
        _emit 083h
        _emit 010h
        _emit 000h
        _emit 000h
        _emit 000h
        fxch st(1) // 00c084f9
        fscale  // 00c084fb
        fstp st(1) // 00c084fd
        fld st(0) // 00c084ff
        fabs  // 00c08501
        // 00c08503: FCOMP qword [00D6A684] -> [EBX+disp32(0h)].
        _emit 0dch
        _emit 09bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        fwait  // 00c08509
        fnstsw ax // 00c0850a
        sahf  // 00c0850c
        mov eax, 3 // 00c0850d
        jbe L_00c084b2 // 00c08512
        // 00c08514: FMUL qword [00D6A6A4] -> [EBX+disp32(20h)].
        _emit 0dch
        _emit 08bh
        _emit 020h
        _emit 000h
        _emit 000h
        _emit 000h
        jmp L_00c084b2 // 00c0851a
    }
}

} // namespace bsp
