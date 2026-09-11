#include "bsp/native_crt_pow_special.hpp"
#include "bsp/native_crt_double_classification.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT pow special handling requires MSVC Win32 raw assembly.
#endif

namespace bsp {

// Complete original bodies. EBX/EDI bind borrowed original literal regions;
// each six-byte _emit block below is ONE original x87 memory instruction
// with only its address operand rebound. Original lengths/branches remain.
// Direct children are complete raw providers; native eight-byte arguments
// are unchanged despite __fpclass having a wider public readable-tail API.

__declspec(naked) std::int32_t __cdecl native_crt_d_inttype_00c19dc0(std::uint32_t, std::uint32_t) {
    __asm {
        push ebp // 00c19dc0
        mov ebp, esp // 00c19dc1
        push ecx // 00c19dc3
        push ecx // 00c19dc4
        fld qword ptr [ebp + 8] // 00c19dc5
        push ecx // 00c19dc8
        push ecx // 00c19dc9
        fstp qword ptr [esp] // 00c19dca
        call native_crt_fpclass_00bfa52f // 00c19dcd
        test al, 0x90 // 00c19dd2
        pop ecx // 00c19dd4
        pop ecx // 00c19dd5
        jne L_00c19e20 // 00c19dd6
        fld qword ptr [ebp + 8] // 00c19dd8
        push ecx // 00c19ddb
        push ecx // 00c19ddc
        fstp qword ptr [esp] // 00c19ddd
        call native_crt_frnd_st0_00c28548 // 00c19de0
        fcomp qword ptr [ebp + 8] // 00c19de5
        pop ecx // 00c19de8
        pop ecx // 00c19de9
        fnstsw ax // 00c19dea
        test ah, 0x44 // 00c19dec
        jp L_00c19e20 // 00c19def
        fld qword ptr [ebp + 8] // 00c19df1
        push ecx // 00c19df4
        // 00c19df5: FMUL qword [00D7A280] -> [EBX+disp32(0h)].
        _emit 0dch
        _emit 08bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        push ecx // 00c19dfb
        fstp qword ptr [ebp - 8] // 00c19dfc
        fld qword ptr [ebp - 8] // 00c19dff
        fstp qword ptr [esp] // 00c19e02
        call native_crt_frnd_st0_00c28548 // 00c19e05
        fcomp qword ptr [ebp - 8] // 00c19e0a
        pop ecx // 00c19e0d
        pop ecx // 00c19e0e
        fnstsw ax // 00c19e0f
        test ah, 0x44 // 00c19e11
        jp L_00c19e1b // 00c19e14
        push 2 // 00c19e16
        pop eax // 00c19e18
        leave  // 00c19e19
        ret  // 00c19e1a
    L_00c19e1b:
        xor eax, eax // 00c19e1b
        inc eax // 00c19e1d
        leave  // 00c19e1e
        ret  // 00c19e1f
    L_00c19e20:
        xor eax, eax // 00c19e20
        leave  // 00c19e22
        ret  // 00c19e23
    }
}

__declspec(naked) std::int32_t __cdecl evaluate_native_crt_pow_special_00c19e24(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, void*) {
    __asm {
        push ebp // 00c19e24
        mov ebp, esp // 00c19e25
        fldz  // 00c19e27
        push esi // 00c19e29
        fcom qword ptr [ebp + 8] // 00c19e2a
        xor edx, edx // 00c19e2d
        xor esi, esi // 00c19e2f
        fnstsw ax // 00c19e31
        fld qword ptr [ebp + 8] // 00c19e33
        test ah, 0x41 // 00c19e36
        jne L_00c19e3d // 00c19e39
        fchs  // 00c19e3b
    L_00c19e3d:
        mov eax, 0x7ff00000 // 00c19e3d
        cmp dword ptr [ebp + 0x14], eax // 00c19e42
        mov ecx, 0xfff00000 // 00c19e45
        jne L_00c19e89 // 00c19e4a
        cmp dword ptr [ebp + 0x10], edx // 00c19e4c
        jne L_00c19ed0 // 00c19e4f
        fld1  // 00c19e51
        fcom st(1) // 00c19e53
        fnstsw ax // 00c19e55
        test ah, 5 // 00c19e57
        jp L_00c19e6d // 00c19e5a
        fstp st(2) // 00c19e5c
        fstp st(1) // 00c19e5e
    L_00c19e60:
        fstp st(0) // 00c19e60
        // 00c19e62: FLD qword [00E165A0] -> [EDI+disp32(0h)].
        _emit 0ddh
        _emit 087h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        jmp L_00c19f4e // 00c19e68
    L_00c19e6d:
        fcom st(1) // 00c19e6d
        fnstsw ax // 00c19e6f
        fstp st(1) // 00c19e71
        test ah, 0x41 // 00c19e73
        mov eax, dword ptr [ebp + 0x18] // 00c19e76
        jne L_00c19e82 // 00c19e79
        fstp st(0) // 00c19e7b
        jmp L_00c19f51 // 00c19e7d
    L_00c19e82:
        fstp st(1) // 00c19e82
        jmp L_00c19f51 // 00c19e84
    L_00c19e89:
        cmp dword ptr [ebp + 0x14], ecx // 00c19e89
        jne L_00c19ed0 // 00c19e8c
        cmp dword ptr [ebp + 0x10], edx // 00c19e8e
        jne L_00c19ed0 // 00c19e91
        fld1  // 00c19e93
        fcom st(1) // 00c19e95
        fnstsw ax // 00c19e97
        test ah, 5 // 00c19e99
        jp L_00c19ea7 // 00c19e9c
        fstp st(0) // 00c19e9e
        fstp st(0) // 00c19ea0
        jmp L_00c19f4e // 00c19ea2
    L_00c19ea7:
        fstp st(2) // 00c19ea7
        fcompp  // 00c19ea9
        fnstsw ax // 00c19eab
        test ah, 5 // 00c19ead
        mov eax, dword ptr [ebp + 0x18] // 00c19eb0
        jp L_00c19ec0 // 00c19eb3
        // 00c19eb5: FLD qword [00E165A0] -> [EDI+disp32(0h)].
        _emit 0ddh
        _emit 087h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        jmp L_00c19f51 // 00c19ebb
    L_00c19ec0:
        // 00c19ec0: FLD qword [00E165A8] -> [EDI+disp32(8h)].
        _emit 0ddh
        _emit 087h
        _emit 008h
        _emit 000h
        _emit 000h
        _emit 000h
        xor esi, esi // 00c19ec6
        fstp qword ptr [eax] // 00c19ec8
        inc esi // 00c19eca
        jmp L_00c19f5d // 00c19ecb
    L_00c19ed0:
        cmp dword ptr [ebp + 0xc], eax // 00c19ed0
        fstp st(0) // 00c19ed3
        jne L_00c19efd // 00c19ed5
        cmp dword ptr [ebp + 8], edx // 00c19ed7
        jne L_00c19f5b // 00c19eda
        fcom qword ptr [ebp + 0x10] // 00c19edc
        fnstsw ax // 00c19edf
        test ah, 5 // 00c19ee1
        jnp L_00c19e60 // 00c19ee4
        fcom qword ptr [ebp + 0x10] // 00c19eea
        fnstsw ax // 00c19eed
        test ah, 0x41 // 00c19eef
        mov eax, dword ptr [ebp + 0x18] // 00c19ef2
        je L_00c19f51 // 00c19ef5
        fstp st(0) // 00c19ef7
        fld1  // 00c19ef9
        jmp L_00c19f51 // 00c19efb
    L_00c19efd:
        cmp dword ptr [ebp + 0xc], ecx // 00c19efd
        fstp st(0) // 00c19f00
        jne L_00c19f5d // 00c19f02
        cmp dword ptr [ebp + 8], edx // 00c19f04
        jne L_00c19f5d // 00c19f07
        fld qword ptr [ebp + 0x10] // 00c19f09
        push ecx // 00c19f0c
        push ecx // 00c19f0d
        fstp qword ptr [esp] // 00c19f0e
        call native_crt_d_inttype_00c19dc0 // 00c19f11
        fldz  // 00c19f16
        pop ecx // 00c19f18
        fcom qword ptr [ebp + 0x10] // 00c19f19
        pop ecx // 00c19f1c
        mov ecx, eax // 00c19f1d
        fnstsw ax // 00c19f1f
        test ah, 5 // 00c19f21
        jp L_00c19f37 // 00c19f24
        cmp ecx, 1 // 00c19f26
        fstp st(0) // 00c19f29
        // 00c19f2b: FLD qword [00E165A0] -> [EDI+disp32(0h)].
        _emit 0ddh
        _emit 087h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        jne L_00c19f4e // 00c19f31
        fchs  // 00c19f33
        jmp L_00c19f4e // 00c19f35
    L_00c19f37:
        fcom qword ptr [ebp + 0x10] // 00c19f37
        fnstsw ax // 00c19f3a
        test ah, 0x41 // 00c19f3c
        jne L_00c19f55 // 00c19f3f
        cmp ecx, 1 // 00c19f41
        jne L_00c19f4e // 00c19f44
        fstp st(0) // 00c19f46
        // 00c19f48: FLD qword [00E165C0] -> [EDI+disp32(20h)].
        _emit 0ddh
        _emit 087h
        _emit 020h
        _emit 000h
        _emit 000h
        _emit 000h
    L_00c19f4e:
        mov eax, dword ptr [ebp + 0x18] // 00c19f4e
    L_00c19f51:
        fstp qword ptr [eax] // 00c19f51
        jmp L_00c19f5d // 00c19f53
    L_00c19f55:
        fstp st(0) // 00c19f55
        fld1  // 00c19f57
        jmp L_00c19f4e // 00c19f59
    L_00c19f5b:
        fstp st(0) // 00c19f5b
    L_00c19f5d:
        mov eax, esi // 00c19f5d
        pop esi // 00c19f5f
        pop ebp // 00c19f60
        ret  // 00c19f61
    }
}

} // namespace bsp
