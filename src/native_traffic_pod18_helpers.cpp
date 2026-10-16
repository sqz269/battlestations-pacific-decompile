#include "bsp/native_traffic_pod18_helpers.hpp"

namespace bsp {

// Complete literal Win32 bodies. All branch displacements and private/actual
// stack-cell accesses remain Native; only two CALL operands bind real Source
// children below. No current CRT, mutable globals, C++ EH frame or callback.

// 0047C4D0..0047C4EE, 31 bytes.
__declspec(naked) std::uint32_t __fastcall size_native_traffic_pod18_0047c4d0(const void*) {
    __asm {
        // 0047C4D0: mov eax, dword ptr [ecx + 4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 0047C4D3: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 0047C4D5: jne 0x47c4d8
        _emit 0x75
        _emit 0x01
        // 0047C4D7: ret
        _emit 0xc3
        // 0047C4D8: mov ecx, dword ptr [ecx + 8]
        _emit 0x8b
        _emit 0x49
        _emit 0x08
        // 0047C4DB: sub ecx, eax
        _emit 0x2b
        _emit 0xc8
        // 0047C4DD: mov eax, 0x2aaaaaab
        _emit 0xb8
        _emit 0xab
        _emit 0xaa
        _emit 0xaa
        _emit 0x2a
        // 0047C4E2: imul ecx
        _emit 0xf7
        _emit 0xe9
        // 0047C4E4: sar edx, 2
        _emit 0xc1
        _emit 0xfa
        _emit 0x02
        // 0047C4E7: mov eax, edx
        _emit 0x8b
        _emit 0xc2
        // 0047C4E9: shr eax, 0x1f
        _emit 0xc1
        _emit 0xe8
        _emit 0x1f
        // 0047C4EC: add eax, edx
        _emit 0x03
        _emit 0xc2
        // 0047C4EE: ret
        _emit 0xc3
    }
}

// 0047FC30..0047FC7C, 77 bytes.
__declspec(naked) void* __cdecl copy_native_traffic_pod18_0047fc30(const void*, const void*, void*, std::uint32_t, std::uint32_t, std::uint32_t) {
    __asm {
        // 0047FC30: mov eax, dword ptr [esp + 0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0047FC34: push esi
        _emit 0x56
        // 0047FC35: mov esi, dword ptr [esp + 8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0047FC39: push edi
        _emit 0x57
        // 0047FC3A: mov edi, dword ptr [esp + 0x10]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0047FC3E: cmp esi, edi
        _emit 0x3b
        _emit 0xf7
        // 0047FC40: je 0x47fc7a
        _emit 0x74
        _emit 0x38
        // 0047FC42: lea ecx, [eax + 0x14]
        _emit 0x8d
        _emit 0x48
        _emit 0x14
        // 0047FC45: lea edx, [esi + 0x14]
        _emit 0x8d
        _emit 0x56
        _emit 0x14
        // 0047FC48: fld dword ptr [esi]
        _emit 0xd9
        _emit 0x06
        // 0047FC4A: add esi, 0x18
        _emit 0x83
        _emit 0xc6
        _emit 0x18
        // 0047FC4D: fstp dword ptr [eax]
        _emit 0xd9
        _emit 0x18
        // 0047FC4F: add eax, 0x18
        _emit 0x83
        _emit 0xc0
        _emit 0x18
        // 0047FC52: fld dword ptr [edx - 0x10]
        _emit 0xd9
        _emit 0x42
        _emit 0xf0
        // 0047FC55: add ecx, 0x18
        _emit 0x83
        _emit 0xc1
        _emit 0x18
        // 0047FC58: fstp dword ptr [ecx - 0x28]
        _emit 0xd9
        _emit 0x59
        _emit 0xd8
        // 0047FC5B: add edx, 0x18
        _emit 0x83
        _emit 0xc2
        _emit 0x18
        // 0047FC5E: cmp esi, edi
        _emit 0x3b
        _emit 0xf7
        // 0047FC60: fld dword ptr [edx - 0x24]
        _emit 0xd9
        _emit 0x42
        _emit 0xdc
        // 0047FC63: fstp dword ptr [ecx - 0x24]
        _emit 0xd9
        _emit 0x59
        _emit 0xdc
        // 0047FC66: fld dword ptr [edx - 0x20]
        _emit 0xd9
        _emit 0x42
        _emit 0xe0
        // 0047FC69: fstp dword ptr [ecx - 0x20]
        _emit 0xd9
        _emit 0x59
        _emit 0xe0
        // 0047FC6C: fld dword ptr [edx - 0x1c]
        _emit 0xd9
        _emit 0x42
        _emit 0xe4
        // 0047FC6F: fstp dword ptr [ecx - 0x1c]
        _emit 0xd9
        _emit 0x59
        _emit 0xe4
        // 0047FC72: fld dword ptr [edx - 0x18]
        _emit 0xd9
        _emit 0x42
        _emit 0xe8
        // 0047FC75: fstp dword ptr [ecx - 0x18]
        _emit 0xd9
        _emit 0x59
        _emit 0xe8
        // 0047FC78: jne 0x47fc48
        _emit 0x75
        _emit 0xce
        // 0047FC7A: pop edi
        _emit 0x5f
        // 0047FC7B: pop esi
        _emit 0x5e
        // 0047FC7C: ret
        _emit 0xc3
    }
}

// 004841A0..004841E8, 73 bytes.
__declspec(naked) void* __fastcall copy_uninitialized_native_traffic_pod18_004841a0(const void*, const void*, void*, std::uint32_t, std::uint32_t, std::uint32_t) {
    __asm {
        // 004841A0: cmp ecx, edx
        _emit 0x3b
        _emit 0xca
        // 004841A2: mov eax, dword ptr [esp + 4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 004841A6: je 0x4841e6
        _emit 0x74
        _emit 0x3e
        // 004841A8: push esi
        _emit 0x56
        // 004841A9: push edi
        _emit 0x57
        // 004841AA: lea esi, [eax + 0x14]
        _emit 0x8d
        _emit 0x70
        _emit 0x14
        // 004841AD: lea edi, [ecx + 0x14]
        _emit 0x8d
        _emit 0x79
        _emit 0x14
        // 004841B0: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 004841B2: je 0x4841d4
        _emit 0x74
        _emit 0x20
        // 004841B4: fld dword ptr [ecx]
        _emit 0xd9
        _emit 0x01
        // 004841B6: fstp dword ptr [eax]
        _emit 0xd9
        _emit 0x18
        // 004841B8: fld dword ptr [edi - 0x10]
        _emit 0xd9
        _emit 0x47
        _emit 0xf0
        // 004841BB: fstp dword ptr [esi - 0x10]
        _emit 0xd9
        _emit 0x5e
        _emit 0xf0
        // 004841BE: fld dword ptr [edi - 0xc]
        _emit 0xd9
        _emit 0x47
        _emit 0xf4
        // 004841C1: fstp dword ptr [esi - 0xc]
        _emit 0xd9
        _emit 0x5e
        _emit 0xf4
        // 004841C4: fld dword ptr [edi - 8]
        _emit 0xd9
        _emit 0x47
        _emit 0xf8
        // 004841C7: fstp dword ptr [esi - 8]
        _emit 0xd9
        _emit 0x5e
        _emit 0xf8
        // 004841CA: fld dword ptr [edi - 4]
        _emit 0xd9
        _emit 0x47
        _emit 0xfc
        // 004841CD: fstp dword ptr [esi - 4]
        _emit 0xd9
        _emit 0x5e
        _emit 0xfc
        // 004841D0: fld dword ptr [edi]
        _emit 0xd9
        _emit 0x07
        // 004841D2: fstp dword ptr [esi]
        _emit 0xd9
        _emit 0x1e
        // 004841D4: add ecx, 0x18
        _emit 0x83
        _emit 0xc1
        _emit 0x18
        // 004841D7: add eax, 0x18
        _emit 0x83
        _emit 0xc0
        _emit 0x18
        // 004841DA: add esi, 0x18
        _emit 0x83
        _emit 0xc6
        _emit 0x18
        // 004841DD: add edi, 0x18
        _emit 0x83
        _emit 0xc7
        _emit 0x18
        // 004841E0: cmp ecx, edx
        _emit 0x3b
        _emit 0xca
        // 004841E2: jne 0x4841b0
        _emit 0x75
        _emit 0xcc
        // 004841E4: pop edi
        _emit 0x5f
        // 004841E5: pop esi
        _emit 0x5e
        // 004841E6: ret 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}

// 00486BE0..00486C25, 70 bytes.
__declspec(naked) void __fastcall fill_n_uninitialized_native_traffic_pod18_00486be0(void*, std::uint32_t, const void*, std::uint32_t, std::uint32_t, std::uint32_t) {
    __asm {
        // 00486BE0: test edx, edx
        _emit 0x85
        _emit 0xd2
        // 00486BE2: jbe 0x486c23
        _emit 0x76
        _emit 0x3f
        // 00486BE4: push esi
        _emit 0x56
        // 00486BE5: mov esi, dword ptr [esp + 8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00486BE9: lea eax, [ecx + 0x14]
        _emit 0x8d
        _emit 0x41
        _emit 0x14
        // 00486BEC: lea esp, [esp]
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // 00486BF0: test ecx, ecx
        _emit 0x85
        _emit 0xc9
        // 00486BF2: je 0x486c15
        _emit 0x74
        _emit 0x21
        // 00486BF4: fld dword ptr [esi]
        _emit 0xd9
        _emit 0x06
        // 00486BF6: fstp dword ptr [ecx]
        _emit 0xd9
        _emit 0x19
        // 00486BF8: fld dword ptr [esi + 4]
        _emit 0xd9
        _emit 0x46
        _emit 0x04
        // 00486BFB: fstp dword ptr [eax - 0x10]
        _emit 0xd9
        _emit 0x58
        _emit 0xf0
        // 00486BFE: fld dword ptr [esi + 8]
        _emit 0xd9
        _emit 0x46
        _emit 0x08
        // 00486C01: fstp dword ptr [eax - 0xc]
        _emit 0xd9
        _emit 0x58
        _emit 0xf4
        // 00486C04: fld dword ptr [esi + 0xc]
        _emit 0xd9
        _emit 0x46
        _emit 0x0c
        // 00486C07: fstp dword ptr [eax - 8]
        _emit 0xd9
        _emit 0x58
        _emit 0xf8
        // 00486C0A: fld dword ptr [esi + 0x10]
        _emit 0xd9
        _emit 0x46
        _emit 0x10
        // 00486C0D: fstp dword ptr [eax - 4]
        _emit 0xd9
        _emit 0x58
        _emit 0xfc
        // 00486C10: fld dword ptr [esi + 0x14]
        _emit 0xd9
        _emit 0x46
        _emit 0x14
        // 00486C13: fstp dword ptr [eax]
        _emit 0xd9
        _emit 0x18
        // 00486C15: sub edx, 1
        _emit 0x83
        _emit 0xea
        _emit 0x01
        // 00486C18: add ecx, 0x18
        _emit 0x83
        _emit 0xc1
        _emit 0x18
        // 00486C1B: add eax, 0x18
        _emit 0x83
        _emit 0xc0
        _emit 0x18
        // 00486C1E: test edx, edx
        _emit 0x85
        _emit 0xd2
        // 00486C20: ja 0x486bf0
        _emit 0x77
        _emit 0xce
        // 00486C22: pop esi
        _emit 0x5e
        // 00486C23: ret 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}

// 0048D2F0..0048D324, 53 bytes.
__declspec(naked) void* __fastcall fill_n_native_traffic_pod18_dispatch_0048d2f0(std::uint32_t, std::uint32_t, void*, std::uint32_t, const void*) {
    __asm {
        // 0048D2F0: push ecx
        _emit 0x51
        // 0048D2F1: mov edx, dword ptr [esp + 0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0048D2F5: push esi
        _emit 0x56
        // 0048D2F6: mov esi, dword ptr [esp + 0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0048D2FA: push edi
        _emit 0x57
        // 0048D2FB: mov edi, dword ptr [esp + 0x10]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0048D2FF: mov byte ptr [esp + 8], 0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        // 0048D304: mov eax, dword ptr [esp + 8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0048D308: push eax
        _emit 0x50
        // 0048D309: mov eax, dword ptr [esp + 0x1c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0048D30D: push edx
        _emit 0x52
        // 0048D30E: push ecx
        _emit 0x51
        // 0048D30F: push eax
        _emit 0x50
        // 0048D310: mov edx, esi
        _emit 0x8b
        _emit 0xd6
        // 0048D312: mov ecx, edi
        _emit 0x8b
        _emit 0xcf
        // 0048D314: call 0x486be0
        call fill_n_uninitialized_native_traffic_pod18_00486be0
        // 0048D319: lea ecx, [esi + esi*2]
        _emit 0x8d
        _emit 0x0c
        _emit 0x76
        // 0048D31C: lea eax, [edi + ecx*8]
        _emit 0x8d
        _emit 0x04
        _emit 0xcf
        // 0048D31F: pop edi
        _emit 0x5f
        // 0048D320: pop esi
        _emit 0x5e
        // 0048D321: pop ecx
        _emit 0x59
        // 0048D322: ret 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}

// 00486850..004868A5, 86 bytes.
__declspec(naked) void* __cdecl copy_native_traffic_pod18_dispatch_00486850(const void*, const void*, void*) {
    __asm {
        // 00486850: sub esp, 8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00486853: mov edx, dword ptr [esp + 0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00486857: push ebx
        _emit 0x53
        // 00486858: mov ebx, dword ptr [esp + 0x10]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // 0048685C: push esi
        _emit 0x56
        // 0048685D: mov esi, dword ptr [esp + 0x18]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x18
        // 00486861: push edi
        _emit 0x57
        // 00486862: mov edi, dword ptr [esp + 0x20]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        // 00486866: xor al, al
        _emit 0x32
        _emit 0xc0
        // 00486868: mov byte ptr [esp + 0x10], al
        _emit 0x88
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0048686C: mov ecx, dword ptr [esp + 0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00486870: mov byte ptr [esp + 0xc], al
        _emit 0x88
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00486874: mov eax, dword ptr [esp + 0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00486878: push eax
        _emit 0x50
        // 00486879: push ecx
        _emit 0x51
        // 0048687A: push edx
        _emit 0x52
        // 0048687B: push edi
        _emit 0x57
        // 0048687C: push esi
        _emit 0x56
        // 0048687D: push ebx
        _emit 0x53
        // 0048687E: call 0x47fc30
        call copy_native_traffic_pod18_0047fc30
        // 00486883: sub esi, ebx
        _emit 0x2b
        _emit 0xf3
        // 00486885: mov eax, 0x2aaaaaab
        _emit 0xb8
        _emit 0xab
        _emit 0xaa
        _emit 0xaa
        _emit 0x2a
        // 0048688A: imul esi
        _emit 0xf7
        _emit 0xee
        // 0048688C: sar edx, 2
        _emit 0xc1
        _emit 0xfa
        _emit 0x02
        // 0048688F: add esp, 0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 00486892: mov eax, edx
        _emit 0x8b
        _emit 0xc2
        // 00486894: shr eax, 0x1f
        _emit 0xc1
        _emit 0xe8
        _emit 0x1f
        // 00486897: add eax, edx
        _emit 0x03
        _emit 0xc2
        // 00486899: lea eax, [eax + eax*2]
        _emit 0x8d
        _emit 0x04
        _emit 0x40
        // 0048689C: lea eax, [edi + eax*8]
        _emit 0x8d
        _emit 0x04
        _emit 0xc7
        // 0048689F: pop edi
        _emit 0x5f
        // 004868A0: pop esi
        _emit 0x5e
        // 004868A1: pop ebx
        _emit 0x5b
        // 004868A2: add esp, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 004868A5: ret
        _emit 0xc3
    }
}

} // namespace bsp
