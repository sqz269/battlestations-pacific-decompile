#include "bsp/cc16_dyn_radix_update.hpp"

#include "bsp/cc16_dyn_sap_update.hpp"
#include "bsp/cc16_dyn_storage.hpp"
#include "bsp/native_crt_memset.hpp"

#include <cstddef>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This recovered instruction schedule requires MSVC Win32.
#endif

// Reuse the actual target compiler runtime, as native_dyn_sap_processing.cpp
// does. These are stack-changing compiler support routines, not heap helpers
// or reconstructed/vendor-body credit.
extern "C" void __cdecl _alloca_probe();
extern "C" void __cdecl _alloca_probe_16();

namespace bsp::cc16_dyn_radix_update {
namespace {
using U32 = std::uint32_t;
struct Context {
    const cc15_dyn_live::DynLiveCrt* crt;
    const volatile U32* feature_word;
    const volatile double* half;
};
static_assert(sizeof(void*) == 4);
static_assert(std::is_standard_layout_v<Context> && sizeof(Context) == 12);
static_assert(offsetof(Context, crt) == 0 && offsetof(Context, feature_word) == 4
              && offsetof(Context, half) == 8);

void* __cdecl memset_bridge(const Context* context, void* destination,
                            int fill, U32 bytes) {
    return fill_native_crt_bytes_00bf79f0(destination, fill, bytes, *context->feature_word);
}
U32 __cdecl append_bridge(void* manager, const Context* context,
                          void* endpoint, U32 axis) {
    return cc16_dyn_sap_update::append_axis_endpoints_00c40900(
        manager, endpoint, axis, *context->crt);
}
void** __cdecl add_bridge(void* manager, const Context* context,
                          void* first, void* second) {
    return cc16_dyn_storage::add_endpoint_pair_00c40a30(
        manager, first, second, *context->crt);
}

// Inserted context is the first stack word after the return address. The
// shim removes only that word; the original caller still removes its 12B.
__declspec(naked) void memset_shim() {
    __asm {
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        call memset_bridge
        add esp,10h
        ret 4
    }
}
// ECX manager; stack(context,endpoint,axis). Restore the original RET8
// effect plus the inserted context word. The leaf's observed EAX survives.
__declspec(naked) void append_shim() {
    __asm {
        push dword ptr [esp+0ch]
        push dword ptr [esp+0ch]
        push dword ptr [esp+0ch]
        push ecx
        call append_bridge
        add esp,10h
        ret 0ch
    }
}
// EDI manager; stack(context,first,second). Pass the actual endpoints and
// manager directly to the recovered add body with this same CRT.
__declspec(naked) void add_shim() {
    __asm {
        push dword ptr [esp+0ch]
        push dword ptr [esp+0ch]
        push dword ptr [esp+0ch]
        push edi
        call add_bridge
        add esp,10h
        ret 0ch
    }
}

// Every original instruction is address-marked below. Only dependency calls,
// the borrowed half-constant load, and final cleanup of the extra Context
// argument are adapted. All native frame slots, probes, branches, repeated
// memory reads, SSE/x87 spills, comparisons and radix store order remain.
// ECX manager, ignored EDX, stack Context*. No original game ABI is claimed.
__declspec(naked) U32 __fastcall kernel(void*, void*, const Context*) {
    __asm {
        push ebp // 00c54da0
        mov ebp,esp // 00c54da1
        mov eax,01040h // 00c54da3
        call _alloca_probe // 00c54da8
        push ebx // 00c54dad
        push esi // 00c54dae
        push edi // 00c54daf
        mov edi,ecx // 00c54db0
        mov dword ptr [ebp + -024h],edi // 00c54db2
        rdtsc // 00c54db5
        mov eax,dword ptr [edi + 0f4h] // 00c54db7
        mov ecx,dword ptr [edi + 0190h] // 00c54dbd
        lea ebx,[ecx + eax*01h] // 00c54dc3
        cvtsi2ss xmm0,ebx // 00c54dc6
        movss dword ptr [ebp + -010h],xmm0 // 00c54dca
        xorps xmm0,xmm0 // 00c54dcf
        mov dword ptr [ebp + -03ch],edx // 00c54dd2
        lea eax,[ebx*08h + 00h] // 00c54dd5
        mov dword ptr [ebp + -0ch],ebx // 00c54ddc
        movss dword ptr [ebp + -030h],xmm0 // 00c54ddf
        movss dword ptr [ebp + -02ch],xmm0 // 00c54de4
        movss dword ptr [ebp + -028h],xmm0 // 00c54de9
        movss dword ptr [ebp + -03ch],xmm0 // 00c54dee
        movss dword ptr [ebp + -038h],xmm0 // 00c54df3
        movss dword ptr [ebp + -034h],xmm0 // 00c54df8
        call _alloca_probe_16 // 00c54dfd
        fld dword ptr [ebp + -010h] // 00c54e02
        mov eax,dword ptr [edi + 0ach] // 00c54e05
        mov edx,dword ptr [ebp+8] // Explicit borrowed Context.
        mov edx,dword ptr [edx+8] // Actual native 00D7A280 cell.
        fld qword ptr [edx] // 00c54e0b
        lea edx,[edi + 0b0h] // 00c54e11
        xor ecx,ecx // 00c54e17
        cmp eax,edx // 00c54e19
        mov esi,esp // 00c54e1b
        jz l_00c54ed5 // 00c54e1d
        lea ebx,[esi + 04h] // 00c54e23
    l_00c54e26:
        mov dword ptr [eax + 034h],00h // 00c54e26
        mov dword ptr [ebx],eax // 00c54e2d
        fld dword ptr [eax + 04h] // 00c54e2f
        fadd dword ptr [eax + 010h] // 00c54e32
        add ecx,01h // 00c54e35
        add ebx,08h // 00c54e38
        fstp dword ptr [ebp + -010h] // 00c54e3b
        fld dword ptr [eax + 014h] // 00c54e3e
        fadd dword ptr [eax + 08h] // 00c54e41
        fstp dword ptr [ebp + -04h] // 00c54e44
        fld dword ptr [eax + 018h] // 00c54e47
        fadd dword ptr [eax + 0ch] // 00c54e4a
        mov eax,dword ptr [eax + 040h] // 00c54e4d
        cmp eax,edx // 00c54e50
        fstp dword ptr [ebp + -08h] // 00c54e52
        fld dword ptr [ebp + -010h] // 00c54e55
        fmul st(0),st(1) // 00c54e58
        fstp dword ptr [ebp + -010h] // 00c54e5a
        fld dword ptr [ebp + -04h] // 00c54e5d
        fmul st(0),st(1) // 00c54e60
        fstp dword ptr [ebp + -04h] // 00c54e62
        fld dword ptr [ebp + -08h] // 00c54e65
        fmul st(0),st(1) // 00c54e68
        fstp dword ptr [ebp + -08h] // 00c54e6a
        fld dword ptr [ebp + -010h] // 00c54e6d
        fld st(0) // 00c54e70
        fdiv st(0),st(3) // 00c54e72
        fstp dword ptr [ebp + -010h] // 00c54e74
        fld dword ptr [ebp + -04h] // 00c54e77
        fld st(0) // 00c54e7a
        fdiv st(0),st(4) // 00c54e7c
        fstp dword ptr [ebp + -04h] // 00c54e7e
        fld dword ptr [ebp + -08h] // 00c54e81
        fld st(0) // 00c54e84
        fdiv st(0),st(5) // 00c54e86
        fstp dword ptr [ebp + -08h] // 00c54e88
        fld dword ptr [ebp + -010h] // 00c54e8b
        fadd dword ptr [ebp + -030h] // 00c54e8e
        fstp dword ptr [ebp + -030h] // 00c54e91
        fld dword ptr [ebp + -04h] // 00c54e94
        fadd dword ptr [ebp + -02ch] // 00c54e97
        fstp dword ptr [ebp + -02ch] // 00c54e9a
        fld dword ptr [ebp + -08h] // 00c54e9d
        fadd dword ptr [ebp + -028h] // 00c54ea0
        fstp dword ptr [ebp + -028h] // 00c54ea3
        fld st(2) // 00c54ea6
        fmulp st(3),st(0) // 00c54ea8
        fld st(4) // 00c54eaa
        fdivp st(3),st(0) // 00c54eac
        fld dword ptr [ebp + -03ch] // 00c54eae
        faddp st(3),st(0) // 00c54eb1
        fxch st(2) // 00c54eb3
        fstp dword ptr [ebp + -03ch] // 00c54eb5
        fmul st(0),st(0) // 00c54eb8
        fdiv st(0),st(3) // 00c54eba
        fadd dword ptr [ebp + -038h] // 00c54ebc
        fstp dword ptr [ebp + -038h] // 00c54ebf
        fmul st(0),st(0) // 00c54ec2
        fdiv st(0),st(2) // 00c54ec4
        fadd dword ptr [ebp + -034h] // 00c54ec6
        fstp dword ptr [ebp + -034h] // 00c54ec9
        jnz l_00c54e26 // 00c54ecc
        mov ebx,dword ptr [ebp + -0ch] // 00c54ed2
    l_00c54ed5:
        mov eax,dword ptr [edi + 0148h] // 00c54ed5
        add edi,014ch // 00c54edb
        cmp eax,edi // 00c54ee1
        jz l_00c54f99 // 00c54ee3
        lea edx,[esi + ecx*08h + 04h] // 00c54ee9
    l_00c54eed:
        mov dword ptr [eax + 034h],00h // 00c54eed
        mov dword ptr [edx],eax // 00c54ef4
        fld dword ptr [eax + 04h] // 00c54ef6
        fadd dword ptr [eax + 010h] // 00c54ef9
        add ecx,01h // 00c54efc
        add edx,08h // 00c54eff
        fstp dword ptr [ebp + -010h] // 00c54f02
        fld dword ptr [eax + 014h] // 00c54f05
        fadd dword ptr [eax + 08h] // 00c54f08
        fstp dword ptr [ebp + -04h] // 00c54f0b
        fld dword ptr [eax + 018h] // 00c54f0e
        fadd dword ptr [eax + 0ch] // 00c54f11
        mov eax,dword ptr [eax + 040h] // 00c54f14
        cmp eax,edi // 00c54f17
        fstp dword ptr [ebp + -08h] // 00c54f19
        fld dword ptr [ebp + -010h] // 00c54f1c
        fmul st(0),st(1) // 00c54f1f
        fstp dword ptr [ebp + -010h] // 00c54f21
        fld dword ptr [ebp + -04h] // 00c54f24
        fmul st(0),st(1) // 00c54f27
        fstp dword ptr [ebp + -04h] // 00c54f29
        fld dword ptr [ebp + -08h] // 00c54f2c
        fmul st(0),st(1) // 00c54f2f
        fstp dword ptr [ebp + -08h] // 00c54f31
        fld dword ptr [ebp + -010h] // 00c54f34
        fld st(0) // 00c54f37
        fdiv st(0),st(3) // 00c54f39
        fstp dword ptr [ebp + -010h] // 00c54f3b
        fld dword ptr [ebp + -04h] // 00c54f3e
        fld st(0) // 00c54f41
        fdiv st(0),st(4) // 00c54f43
        fstp dword ptr [ebp + -04h] // 00c54f45
        fld dword ptr [ebp + -08h] // 00c54f48
        fld st(0) // 00c54f4b
        fdiv st(0),st(5) // 00c54f4d
        fstp dword ptr [ebp + -08h] // 00c54f4f
        fld dword ptr [ebp + -010h] // 00c54f52
        fadd dword ptr [ebp + -030h] // 00c54f55
        fstp dword ptr [ebp + -030h] // 00c54f58
        fld dword ptr [ebp + -04h] // 00c54f5b
        fadd dword ptr [ebp + -02ch] // 00c54f5e
        fstp dword ptr [ebp + -02ch] // 00c54f61
        fld dword ptr [ebp + -08h] // 00c54f64
        fadd dword ptr [ebp + -028h] // 00c54f67
        fstp dword ptr [ebp + -028h] // 00c54f6a
        fld st(2) // 00c54f6d
        fmulp st(3),st(0) // 00c54f6f
        fld st(4) // 00c54f71
        fdivp st(3),st(0) // 00c54f73
        fld dword ptr [ebp + -03ch] // 00c54f75
        faddp st(3),st(0) // 00c54f78
        fxch st(2) // 00c54f7a
        fstp dword ptr [ebp + -03ch] // 00c54f7c
        fmul st(0),st(0) // 00c54f7f
        fdiv st(0),st(3) // 00c54f81
        fadd dword ptr [ebp + -038h] // 00c54f83
        fstp dword ptr [ebp + -038h] // 00c54f86
        fmul st(0),st(0) // 00c54f89
        fdiv st(0),st(2) // 00c54f8b
        fadd dword ptr [ebp + -034h] // 00c54f8d
        fstp dword ptr [ebp + -034h] // 00c54f90
        jnz l_00c54eed // 00c54f93
    l_00c54f99:
        fstp st(1) // 00c54f99
        xor edx,edx // 00c54f9b
        fstp st(0) // 00c54f9d
        mov dword ptr [ebp + -01ch],edx // 00c54f9f
        fld dword ptr [ebp + -030h] // 00c54fa2
        fld st(0) // 00c54fa5
        fmulp st(1),st(0) // 00c54fa7
        fsubr dword ptr [ebp + -03ch] // 00c54fa9
        fstp dword ptr [ebp + -018h] // 00c54fac
        fld dword ptr [ebp + -02ch] // 00c54faf
        fld st(0) // 00c54fb2
        fmulp st(1),st(0) // 00c54fb4
        fsubr dword ptr [ebp + -038h] // 00c54fb6
        fstp dword ptr [ebp + -014h] // 00c54fb9
        fld dword ptr [ebp + -028h] // 00c54fbc
        fld st(0) // 00c54fbf
        fmulp st(1),st(0) // 00c54fc1
        fsubr dword ptr [ebp + -034h] // 00c54fc3
        fstp dword ptr [ebp + -010h] // 00c54fc6
        fld dword ptr [ebp + -018h] // 00c54fc9
        fld dword ptr [ebp + -014h] // 00c54fcc
        fcomip st(0),st(1) // 00c54fcf
        fstp st(0) // 00c54fd1
        jbe l_00c54fdd // 00c54fd3
        mov edx,01h // 00c54fd5
        mov dword ptr [ebp + -01ch],edx // 00c54fda
    l_00c54fdd:
        fld dword ptr [ebp + edx*04h + -018h] // 00c54fdd
        fld dword ptr [ebp + -010h] // 00c54fe1
        fcomip st(0),st(1) // 00c54fe4
        fstp st(0) // 00c54fe6
        jbe l_00c54ff2 // 00c54fe8
        mov edx,02h // 00c54fea
        mov dword ptr [ebp + -01ch],edx // 00c54fef
    l_00c54ff2:
        xor eax,eax // 00c54ff2
        test ecx,ecx // 00c54ff4
        jle l_00c55030 // 00c54ff6
        jmp l_00c55003 // 00c54ff8
    l_00c55000:
        mov edx,dword ptr [ebp + -01ch] // 00c55000
    l_00c55003:
        mov edi,dword ptr [esi + eax*08h + 04h] // 00c55003
        fld dword ptr [edi + edx*04h + 04h] // 00c55007
        add eax,01h // 00c5500b
        fstp dword ptr [esi + eax*08h + -08h] // 00c5500e
        mov edx,dword ptr [esi + eax*08h + -08h] // 00c55012
        mov edi,edx // 00c55016
        shr edi,01fh // 00c55018
        not edi // 00c5501b
        add edi,01h // 00c5501d
        or edi,080000000h // 00c55020
        xor edi,edx // 00c55026
        cmp eax,ecx // 00c55028
        mov dword ptr [esi + eax*08h + -08h],edi // 00c5502a
        jl l_00c55000 // 00c5502e
    l_00c55030:
        lea eax,[ebx*08h + 00h] // 00c55030
        call _alloca_probe_16 // 00c55037
        mov dword ptr [ebp + -04h],esp // 00c5503c
        push 01000h // 00c5503f
        lea eax,[ebp + 0ffffefc0h] // 00c55044
        push 00h // 00c5504a
        push eax // 00c5504c
        push dword ptr [ebp+8] // Borrow the same explicit Context.
        call memset_shim // 00c5504d
        add esp,0ch // 00c55052
        xor ecx,ecx // 00c55055
        test ebx,ebx // 00c55057
        jle l_00c550cb // 00c55059
        jmp l_00c55060 // 00c5505b
    l_00c55060:
        mov eax,dword ptr [esi + ecx*08h] // 00c55060
        mov edx,eax // 00c55063
        and edx,0ffh // 00c55065
        shl edx,04h // 00c5506b
        add dword ptr [ebp + edx*01h + 0ffffefc0h],01h // 00c5506e
        lea edx,[ebp + edx*01h + 0ffffefc0h] // 00c55076
        movzx edx,ah // 00c5507d
        shl edx,04h // 00c55080
        add dword ptr [ebp + edx*01h + 0ffffefc4h],01h // 00c55083
        lea edx,[ebp + edx*01h + 0ffffefc4h] // 00c5508b
        mov edx,eax // 00c55092
        shr edx,010h // 00c55094
        and edx,0ffh // 00c55097
        shr eax,018h // 00c5509d
        shl edx,04h // 00c550a0
        add dword ptr [ebp + edx*01h + 0ffffefc8h],01h // 00c550a3
        shl eax,04h // 00c550ab
        add dword ptr [ebp + eax*01h + 0ffffefcch],01h // 00c550ae
        lea edx,[ebp + edx*01h + 0ffffefc8h] // 00c550b6
        lea eax,[ebp + eax*01h + 0ffffefcch] // 00c550bd
        add ecx,01h // 00c550c4
        cmp ecx,ebx // 00c550c7
        jl l_00c55060 // 00c550c9
    l_00c550cb:
        xor ecx,ecx // 00c550cb
        xor edx,edx // 00c550cd
        xor edi,edi // 00c550cf
        mov dword ptr [ebp + -08h],edi // 00c550d1
        lea eax,[ebp + 0ffffefc4h] // 00c550d4
        mov dword ptr [ebp + -020h],0100h // 00c550da
        jmp l_00c550e6 // 00c550e1
    l_00c550e3:
        mov edi,dword ptr [ebp + -010h] // 00c550e3
    l_00c550e6:
        mov ebx,dword ptr [eax + -04h] // 00c550e6
        mov dword ptr [eax + -04h],ecx // 00c550e9
        add ecx,ebx // 00c550ec
        mov ebx,dword ptr [eax] // 00c550ee
        mov dword ptr [eax],edx // 00c550f0
        add edx,ebx // 00c550f2
        mov ebx,dword ptr [eax + 04h] // 00c550f4
        mov dword ptr [eax + 04h],edi // 00c550f7
        add edi,ebx // 00c550fa
        mov ebx,dword ptr [ebp + -08h] // 00c550fc
        mov dword ptr [ebp + -010h],edi // 00c550ff
        mov edi,dword ptr [eax + 08h] // 00c55102
        mov dword ptr [eax + 08h],ebx // 00c55105
        add ebx,edi // 00c55108
        add eax,010h // 00c5510a
        sub dword ptr [ebp + -020h],01h // 00c5510d
        mov dword ptr [ebp + -08h],ebx // 00c55111
        jnz l_00c550e3 // 00c55114
        mov ecx,dword ptr [ebp + -0ch] // 00c55116
        test ecx,ecx // 00c55119
        mov edi,dword ptr [ebp + -04h] // 00c5511b
        jle l_00c5519a // 00c5511e
        mov eax,esi // 00c55120
        mov dword ptr [ebp + -08h],ecx // 00c55122
        jmp l_00c55130 // 00c55125
    l_00c55130:
        movzx ecx,byte ptr [eax] // 00c55130
        shl ecx,04h // 00c55133
        lea edx,[ebp + ecx*01h + 0ffffefc0h] // 00c55136
        mov ecx,dword ptr [edx] // 00c5513d
        lea ebx,[ecx + 01h] // 00c5513f
        mov dword ptr [edx],ebx // 00c55142
        mov edx,dword ptr [eax] // 00c55144
        mov dword ptr [edi + ecx*08h],edx // 00c55146
        mov edx,dword ptr [eax + 04h] // 00c55149
        add eax,08h // 00c5514c
        sub dword ptr [ebp + -08h],01h // 00c5514f
        mov dword ptr [edi + ecx*08h + 04h],edx // 00c55153
        jnz l_00c55130 // 00c55157
        mov ecx,dword ptr [ebp + -0ch] // 00c55159
        test ecx,ecx // 00c5515c
        jle l_00c5519a // 00c5515e
        mov eax,edi // 00c55160
        mov dword ptr [ebp + -04h],ecx // 00c55162
        jmp l_00c55170 // 00c55165
    l_00c55170:
        movzx ecx,byte ptr [eax + 01h] // 00c55170
        shl ecx,04h // 00c55174
        lea edx,[ebp + ecx*01h + 0ffffefc4h] // 00c55177
        mov ecx,dword ptr [edx] // 00c5517e
        lea ebx,[ecx + 01h] // 00c55180
        mov dword ptr [edx],ebx // 00c55183
        mov edx,dword ptr [eax] // 00c55185
        mov dword ptr [esi + ecx*08h],edx // 00c55187
        mov edx,dword ptr [eax + 04h] // 00c5518a
        add eax,08h // 00c5518d
        sub dword ptr [ebp + -04h],01h // 00c55190
        mov dword ptr [esi + ecx*08h + 04h],edx // 00c55194
        jnz l_00c55170 // 00c55198
    l_00c5519a:
        cmp dword ptr [ebp + -0ch],00h // 00c5519a
        jle l_00c5521a // 00c5519e
        mov eax,dword ptr [ebp + -0ch] // 00c551a0
        mov ecx,esi // 00c551a3
        mov dword ptr [ebp + -04h],eax // 00c551a5
        jmp l_00c551b0 // 00c551a8
    l_00c551b0:
        mov eax,dword ptr [ecx] // 00c551b0
        shr eax,010h // 00c551b2
        and eax,0ffh // 00c551b5
        shl eax,04h // 00c551ba
        lea edx,[ebp + eax*01h + 0ffffefc8h] // 00c551bd
        mov eax,dword ptr [edx] // 00c551c4
        lea ebx,[eax + 01h] // 00c551c6
        mov dword ptr [edx],ebx // 00c551c9
        mov edx,dword ptr [ecx] // 00c551cb
        mov dword ptr [edi + eax*08h],edx // 00c551cd
        mov edx,dword ptr [ecx + 04h] // 00c551d0
        add ecx,08h // 00c551d3
        sub dword ptr [ebp + -04h],01h // 00c551d6
        mov dword ptr [edi + eax*08h + 04h],edx // 00c551da
        jnz l_00c551b0 // 00c551de
        cmp dword ptr [ebp + -0ch],00h // 00c551e0
        jle l_00c5521a // 00c551e4
        mov eax,edi // 00c551e6
        mov edi,dword ptr [ebp + -0ch] // 00c551e8
        jmp l_00c551f0 // 00c551eb
    l_00c551f0:
        mov ecx,dword ptr [eax] // 00c551f0
        shr ecx,018h // 00c551f2
        shl ecx,04h // 00c551f5
        lea edx,[ebp + ecx*01h + 0ffffefcch] // 00c551f8
        mov ecx,dword ptr [edx] // 00c551ff
        lea ebx,[ecx + 01h] // 00c55201
        mov dword ptr [edx],ebx // 00c55204
        mov edx,dword ptr [eax] // 00c55206
        mov dword ptr [esi + ecx*08h],edx // 00c55208
        mov edx,dword ptr [eax + 04h] // 00c5520b
        add eax,08h // 00c5520e
        sub edi,01h // 00c55211
        mov dword ptr [esi + ecx*08h + 04h],edx // 00c55214
        jnz l_00c551f0 // 00c55218
    l_00c5521a:
        mov edi,dword ptr [ebp + -024h] // 00c5521a
        cmp dword ptr [edi + 058h],00h // 00c5521d
        jz l_00c55254 // 00c55221
        mov eax,dword ptr [edi + 044h] // 00c55223
        lea ecx,[edi + 048h] // 00c55226
        cmp dword ptr [eax + 0ch],ecx // 00c55229
        jz l_00c55238 // 00c5522c
        mov edi,edi // 00c5522e
    l_00c55230:
        mov eax,dword ptr [eax + 0ch] // 00c55230
        cmp dword ptr [eax + 0ch],ecx // 00c55233
        jnz l_00c55230 // 00c55236
    l_00c55238:
        mov edx,dword ptr [edi + 034h] // 00c55238
        mov dword ptr [eax + 0ch],edx // 00c5523b
        mov eax,dword ptr [edi + 044h] // 00c5523e
        mov dword ptr [edi + 044h],ecx // 00c55241
        lea ecx,[edi + 038h] // 00c55244
        mov dword ptr [edi + 034h],eax // 00c55247
        mov dword ptr [edi + 050h],ecx // 00c5524a
        mov dword ptr [edi + 058h],00h // 00c5524d
    l_00c55254:
        cmp dword ptr [ebp + -0ch],00h // 00c55254
        jle l_00c5532a // 00c55258
        mov edx,dword ptr [ebp + -0ch] // 00c5525e
        add esi,04h // 00c55261
        mov dword ptr [ebp + -08h],01h // 00c55264
        mov dword ptr [ebp + -020h],esi // 00c5526b
        mov dword ptr [ebp + -010h],edx // 00c5526e
    l_00c55271:
        mov eax,dword ptr [ebp + -020h] // 00c55271
        mov esi,dword ptr [eax] // 00c55274
        mov eax,dword ptr [ebp + -08h] // 00c55276
        add esi,04h // 00c55279
        cmp eax,dword ptr [ebp + -0ch] // 00c5527c
        mov dword ptr [ebp + -04h],eax // 00c5527f
        jge l_00c55315 // 00c55282
        mov ebx,dword ptr [ebp + -020h] // 00c55288
        add ebx,08h // 00c5528b
    l_00c5528e:
        mov eax,dword ptr [ebp + -01ch] // 00c5528e
        mov ecx,dword ptr [ebx] // 00c55291
        fld dword ptr [esi + eax*04h + 0ch] // 00c55293
        fld dword ptr [ecx + eax*04h + 04h] // 00c55297
        fcomip st(0),st(1) // 00c5529b
        fstp st(0) // 00c5529d
        ja l_00c55315 // 00c5529f
        mov edx,dword ptr [ebp + -020h] // 00c552a1
        mov edx,dword ptr [edx] // 00c552a4
        cmp byte ptr [edx + 01ch],00h // 00c552a6
        jz l_00c552b2 // 00c552aa
        cmp byte ptr [ecx + 01ch],00h // 00c552ac
        jnz l_00c55300 // 00c552b0
    l_00c552b2:
        fld dword ptr [esi + 0ch] // 00c552b2
        fld dword ptr [ecx + 04h] // 00c552b5
        fcomip st(0),st(1) // 00c552b8
        fstp st(0) // 00c552ba
        ja l_00c55300 // 00c552bc
        fld dword ptr [esi + 010h] // 00c552be
        fld dword ptr [ecx + 08h] // 00c552c1
        fcomip st(0),st(1) // 00c552c4
        fstp st(0) // 00c552c6
        ja l_00c55300 // 00c552c8
        fld dword ptr [esi + 014h] // 00c552ca
        fld dword ptr [ecx + 0ch] // 00c552cd
        fcomip st(0),st(1) // 00c552d0
        fstp st(0) // 00c552d2
        ja l_00c55300 // 00c552d4
        fld dword ptr [ecx + 010h] // 00c552d6
        fld dword ptr [esi] // 00c552d9
        fcomip st(0),st(1) // 00c552db
        fstp st(0) // 00c552dd
        ja l_00c55300 // 00c552df
        fld dword ptr [ecx + 014h] // 00c552e1
        fld dword ptr [esi + 04h] // 00c552e4
        fcomip st(0),st(1) // 00c552e7
        fstp st(0) // 00c552e9
        ja l_00c55300 // 00c552eb
        fld dword ptr [ecx + 018h] // 00c552ed
        fld dword ptr [esi + 08h] // 00c552f0
        fcomip st(0),st(1) // 00c552f3
        fstp st(0) // 00c552f5
        ja l_00c55300 // 00c552f7
        push ecx // 00c552f9
        push edx // 00c552fa
        push dword ptr [ebp+8] // Borrow the same explicit Context.
        call add_shim // 00c552fb
    l_00c55300:
        mov eax,dword ptr [ebp + -04h] // 00c55300
        add eax,01h // 00c55303
        add ebx,08h // 00c55306
        cmp eax,dword ptr [ebp + -0ch] // 00c55309
        mov dword ptr [ebp + -04h],eax // 00c5530c
        jl l_00c5528e // 00c5530f
    l_00c55315:
        add dword ptr [ebp + -020h],08h // 00c55315
        mov eax,01h // 00c55319
        add dword ptr [ebp + -08h],eax // 00c5531e
        sub dword ptr [ebp + -010h],eax // 00c55321
        jnz l_00c55271 // 00c55324
    l_00c5532a:
        xor ebx,ebx // 00c5532a
        cmp dword ptr [edi + 0198h],ebx // 00c5532c
        jbe l_00c5536a // 00c55332
    l_00c55334:
        mov eax,dword ptr [edi + 0194h] // 00c55334
        mov esi,dword ptr [eax + ebx*04h] // 00c5533a
        push 00h // 00c5533d
        push esi // 00c5533f
        mov ecx,edi // 00c55340
        push dword ptr [ebp+8] // Borrow the same explicit Context.
        call append_shim // 00c55342
        push 01h // 00c55347
        push esi // 00c55349
        mov ecx,edi // 00c5534a
        push dword ptr [ebp+8] // Borrow the same explicit Context.
        call append_shim // 00c5534c
        push 02h // 00c55351
        push esi // 00c55353
        mov ecx,edi // 00c55354
        push dword ptr [ebp+8] // Borrow the same explicit Context.
        call append_shim // 00c55356
        add ebx,01h // 00c5535b
        mov byte ptr [esi + 02ch],01h // 00c5535e
        cmp ebx,dword ptr [edi + 0198h] // 00c55362
        jc l_00c55334 // 00c55368
    l_00c5536a:
        xor esi,esi // 00c5536a
        mov edx,edi // 00c5536c
        mov dword ptr [edx + 0198h],esi // 00c5536e
        mov ebx,dword ptr [edx + 08h] // 00c55374
        cmp ebx,esi // 00c55377
        mov dword ptr [ebp + -0ch],ebx // 00c55379
        jle l_00c5543b // 00c5537c
    l_00c55382:
        mov eax,dword ptr [edx + 04h] // 00c55382
        mov ecx,dword ptr [eax + esi*08h + 04h] // 00c55385
        and ecx,0fffffffeh // 00c55389
        test byte ptr [eax + esi*08h + 04h],01h // 00c5538c
        jz l_00c5539a // 00c55391
        movss xmm0,dword ptr [ecx + 04h] // 00c55393
        jmp l_00c5539f // 00c55398
    l_00c5539a:
        movss xmm0,dword ptr [ecx + 010h] // 00c5539a
    l_00c5539f:
        movss dword ptr [eax + esi*08h],xmm0 // 00c5539f
        mov ecx,dword ptr [eax + esi*08h] // 00c553a4
        mov edi,ecx // 00c553a7
        shr edi,01fh // 00c553a9
        not edi // 00c553ac
        add edi,01h // 00c553ae
        or edi,080000000h // 00c553b1
        xor edi,ecx // 00c553b7
        mov dword ptr [eax + esi*08h],edi // 00c553b9
        mov eax,dword ptr [edx + 010h] // 00c553bc
        mov ecx,dword ptr [eax + esi*08h + 04h] // 00c553bf
        and ecx,0fffffffeh // 00c553c3
        test byte ptr [eax + esi*08h + 04h],01h // 00c553c6
        jz l_00c553d4 // 00c553cb
        movss xmm0,dword ptr [ecx + 08h] // 00c553cd
        jmp l_00c553d9 // 00c553d2
    l_00c553d4:
        movss xmm0,dword ptr [ecx + 014h] // 00c553d4
    l_00c553d9:
        movss dword ptr [eax + esi*08h],xmm0 // 00c553d9
        mov ecx,dword ptr [eax + esi*08h] // 00c553de
        mov edi,ecx // 00c553e1
        shr edi,01fh // 00c553e3
        not edi // 00c553e6
        add edi,01h // 00c553e8
        or edi,080000000h // 00c553eb
        xor edi,ecx // 00c553f1
        mov dword ptr [eax + esi*08h],edi // 00c553f3
        mov eax,dword ptr [edx + 01ch] // 00c553f6
        mov ecx,dword ptr [eax + esi*08h + 04h] // 00c553f9
        and ecx,0fffffffeh // 00c553fd
        test byte ptr [eax + esi*08h + 04h],01h // 00c55400
        jz l_00c5540e // 00c55405
        movss xmm0,dword ptr [ecx + 0ch] // 00c55407
        jmp l_00c55413 // 00c5540c
    l_00c5540e:
        movss xmm0,dword ptr [ecx + 018h] // 00c5540e
    l_00c55413:
        movss dword ptr [eax + esi*08h],xmm0 // 00c55413
        mov ecx,dword ptr [eax + esi*08h] // 00c55418
        mov edi,ecx // 00c5541b
        shr edi,01fh // 00c5541d
        not edi // 00c55420
        add edi,01h // 00c55422
        or edi,080000000h // 00c55425
        xor edi,ecx // 00c5542b
        mov dword ptr [eax + esi*08h],edi // 00c5542d
        add esi,01h // 00c55430
        cmp esi,ebx // 00c55433
        jl l_00c55382 // 00c55435
    l_00c5543b:
        lea eax,[ebx*08h + 00h] // 00c5543b
        call _alloca_probe_16 // 00c55442
        mov edx,dword ptr [ebp + -024h] // 00c55447
        mov edi,dword ptr [edx + 04h] // 00c5544a
        mov esi,esp // 00c5544d
        push 01000h // 00c5544f
        lea eax,[ebp + 0ffffefc0h] // 00c55454
        push 00h // 00c5545a
        push eax // 00c5545c
        push dword ptr [ebp+8] // Borrow the same explicit Context.
        call memset_shim // 00c5545d
        add esp,0ch // 00c55462
        xor ecx,ecx // 00c55465
        test ebx,ebx // 00c55467
        jle l_00c554db // 00c55469
        jmp l_00c55470 // 00c5546b
    l_00c55470:
        mov eax,dword ptr [edi + ecx*08h] // 00c55470
        mov edx,eax // 00c55473
        and edx,0ffh // 00c55475
        shl edx,04h // 00c5547b
        add dword ptr [ebp + edx*01h + 0ffffefc0h],01h // 00c5547e
        lea edx,[ebp + edx*01h + 0ffffefc0h] // 00c55486
        movzx edx,ah // 00c5548d
        shl edx,04h // 00c55490
        add dword ptr [ebp + edx*01h + 0ffffefc4h],01h // 00c55493
        lea edx,[ebp + edx*01h + 0ffffefc4h] // 00c5549b
        mov edx,eax // 00c554a2
        shr edx,010h // 00c554a4
        and edx,0ffh // 00c554a7
        shr eax,018h // 00c554ad
        shl edx,04h // 00c554b0
        add dword ptr [ebp + edx*01h + 0ffffefc8h],01h // 00c554b3
        shl eax,04h // 00c554bb
        add dword ptr [ebp + eax*01h + 0ffffefcch],01h // 00c554be
        lea edx,[ebp + edx*01h + 0ffffefc8h] // 00c554c6
        lea eax,[ebp + eax*01h + 0ffffefcch] // 00c554cd
        add ecx,01h // 00c554d4
        cmp ecx,ebx // 00c554d7
        jl l_00c55470 // 00c554d9
    l_00c554db:
        xor edx,edx // 00c554db
        mov dword ptr [ebp + -010h],edx // 00c554dd
        mov dword ptr [ebp + -04h],edx // 00c554e0
        mov dword ptr [ebp + -08h],edx // 00c554e3
        lea eax,[ebp + 0ffffefc4h] // 00c554e6
        mov dword ptr [ebp + -01ch],0100h // 00c554ec
        jmp l_00c554f8 // 00c554f3
    l_00c554f5:
        mov edx,dword ptr [ebp + -020h] // 00c554f5
    l_00c554f8:
        mov ecx,dword ptr [eax + -04h] // 00c554f8
        mov dword ptr [eax + -04h],edx // 00c554fb
        add edx,ecx // 00c554fe
        mov ecx,dword ptr [eax] // 00c55500
        mov dword ptr [ebp + -020h],edx // 00c55502
        mov edx,dword ptr [ebp + -010h] // 00c55505
        mov dword ptr [eax],edx // 00c55508
        add edx,ecx // 00c5550a
        mov ecx,dword ptr [eax + 04h] // 00c5550c
        mov dword ptr [ebp + -010h],edx // 00c5550f
        mov edx,dword ptr [ebp + -04h] // 00c55512
        mov dword ptr [eax + 04h],edx // 00c55515
        add edx,ecx // 00c55518
        mov ecx,dword ptr [eax + 08h] // 00c5551a
        mov dword ptr [ebp + -04h],edx // 00c5551d
        mov edx,dword ptr [ebp + -08h] // 00c55520
        mov dword ptr [eax + 08h],edx // 00c55523
        add edx,ecx // 00c55526
        add eax,010h // 00c55528
        sub dword ptr [ebp + -01ch],01h // 00c5552b
        mov dword ptr [ebp + -08h],edx // 00c5552f
        jnz l_00c554f5 // 00c55532
        test ebx,ebx // 00c55534
        jle l_00c55620 // 00c55536
        mov eax,edi // 00c5553c
        mov dword ptr [ebp + -010h],ebx // 00c5553e
    l_00c55541:
        movzx ecx,byte ptr [eax] // 00c55541
        shl ecx,04h // 00c55544
        lea edx,[ebp + ecx*01h + 0ffffefc0h] // 00c55547
        mov ecx,dword ptr [edx] // 00c5554e
        lea ebx,[ecx + 01h] // 00c55550
        mov dword ptr [edx],ebx // 00c55553
        mov edx,dword ptr [eax] // 00c55555
        mov dword ptr [esi + ecx*08h],edx // 00c55557
        mov edx,dword ptr [eax + 04h] // 00c5555a
        add eax,08h // 00c5555d
        sub dword ptr [ebp + -010h],01h // 00c55560
        mov dword ptr [esi + ecx*08h + 04h],edx // 00c55564
        jnz l_00c55541 // 00c55568
        mov ebx,dword ptr [ebp + -0ch] // 00c5556a
        test ebx,ebx // 00c5556d
        jle l_00c55620 // 00c5556f
        mov eax,esi // 00c55575
        mov dword ptr [ebp + -010h],ebx // 00c55577
        lea ebx,[ebx] // 00c5557a
    l_00c55580:
        movzx ecx,byte ptr [eax + 01h] // 00c55580
        shl ecx,04h // 00c55584
        lea edx,[ebp + ecx*01h + 0ffffefc4h] // 00c55587
        mov ecx,dword ptr [edx] // 00c5558e
        lea ebx,[ecx + 01h] // 00c55590
        mov dword ptr [edx],ebx // 00c55593
        mov edx,dword ptr [eax] // 00c55595
        mov dword ptr [edi + ecx*08h],edx // 00c55597
        mov edx,dword ptr [eax + 04h] // 00c5559a
        add eax,08h // 00c5559d
        sub dword ptr [ebp + -010h],01h // 00c555a0
        mov dword ptr [edi + ecx*08h + 04h],edx // 00c555a4
        jnz l_00c55580 // 00c555a8
        mov ebx,dword ptr [ebp + -0ch] // 00c555aa
        test ebx,ebx // 00c555ad
        jle l_00c55620 // 00c555af
        mov ecx,edi // 00c555b1
        mov dword ptr [ebp + -010h],ebx // 00c555b3
    l_00c555b6:
        mov eax,dword ptr [ecx] // 00c555b6
        shr eax,010h // 00c555b8
        and eax,0ffh // 00c555bb
        shl eax,04h // 00c555c0
        lea edx,[ebp + eax*01h + 0ffffefc8h] // 00c555c3
        mov eax,dword ptr [edx] // 00c555ca
        lea ebx,[eax + 01h] // 00c555cc
        mov dword ptr [edx],ebx // 00c555cf
        mov edx,dword ptr [ecx] // 00c555d1
        mov dword ptr [esi + eax*08h],edx // 00c555d3
        mov edx,dword ptr [ecx + 04h] // 00c555d6
        add ecx,08h // 00c555d9
        sub dword ptr [ebp + -010h],01h // 00c555dc
        mov dword ptr [esi + eax*08h + 04h],edx // 00c555e0
        jnz l_00c555b6 // 00c555e4
        mov ebx,dword ptr [ebp + -0ch] // 00c555e6
        test ebx,ebx // 00c555e9
        jle l_00c55620 // 00c555eb
        mov eax,esi // 00c555ed
        mov dword ptr [ebp + -010h],ebx // 00c555ef
    l_00c555f2:
        mov ecx,dword ptr [eax] // 00c555f2
        shr ecx,018h // 00c555f4
        shl ecx,04h // 00c555f7
        lea edx,[ebp + ecx*01h + 0ffffefcch] // 00c555fa
        mov ecx,dword ptr [edx] // 00c55601
        lea ebx,[ecx + 01h] // 00c55603
        mov dword ptr [edx],ebx // 00c55606
        mov edx,dword ptr [eax] // 00c55608
        mov dword ptr [edi + ecx*08h],edx // 00c5560a
        mov edx,dword ptr [eax + 04h] // 00c5560d
        add eax,08h // 00c55610
        sub dword ptr [ebp + -010h],01h // 00c55613
        mov dword ptr [edi + ecx*08h + 04h],edx // 00c55617
        jnz l_00c555f2 // 00c5561b
        mov ebx,dword ptr [ebp + -0ch] // 00c5561d
    l_00c55620:
        mov eax,dword ptr [ebp + -024h] // 00c55620
        mov edi,dword ptr [eax + 010h] // 00c55623
        push 01000h // 00c55626
        lea ecx,[ebp + 0ffffefc0h] // 00c5562b
        push 00h // 00c55631
        push ecx // 00c55633
        push dword ptr [ebp+8] // Borrow the same explicit Context.
        call memset_shim // 00c55634
        add esp,0ch // 00c55639
        xor ecx,ecx // 00c5563c
        test ebx,ebx // 00c5563e
        jle l_00c556ad // 00c55640
    l_00c55642:
        mov eax,dword ptr [edi + ecx*08h] // 00c55642
        mov edx,eax // 00c55645
        and edx,0ffh // 00c55647
        shl edx,04h // 00c5564d
        add dword ptr [ebp + edx*01h + 0ffffefc0h],01h // 00c55650
        lea edx,[ebp + edx*01h + 0ffffefc0h] // 00c55658
        movzx edx,ah // 00c5565f
        shl edx,04h // 00c55662
        add dword ptr [ebp + edx*01h + 0ffffefc4h],01h // 00c55665
        lea edx,[ebp + edx*01h + 0ffffefc4h] // 00c5566d
        mov edx,eax // 00c55674
        shr edx,010h // 00c55676
        and edx,0ffh // 00c55679
        shr eax,018h // 00c5567f
        shl edx,04h // 00c55682
        add dword ptr [ebp + edx*01h + 0ffffefc8h],01h // 00c55685
        shl eax,04h // 00c5568d
        add dword ptr [ebp + eax*01h + 0ffffefcch],01h // 00c55690
        lea edx,[ebp + edx*01h + 0ffffefc8h] // 00c55698
        lea eax,[ebp + eax*01h + 0ffffefcch] // 00c5569f
        add ecx,01h // 00c556a6
        cmp ecx,ebx // 00c556a9
        jl l_00c55642 // 00c556ab
    l_00c556ad:
        xor edx,edx // 00c556ad
        mov dword ptr [ebp + -010h],edx // 00c556af
        mov dword ptr [ebp + -04h],edx // 00c556b2
        mov dword ptr [ebp + -08h],edx // 00c556b5
        lea eax,[ebp + 0ffffefc4h] // 00c556b8
        mov dword ptr [ebp + -01ch],0100h // 00c556be
        jmp l_00c556ca // 00c556c5
    l_00c556c7:
        mov edx,dword ptr [ebp + -020h] // 00c556c7
    l_00c556ca:
        mov ecx,dword ptr [eax + -04h] // 00c556ca
        mov dword ptr [eax + -04h],edx // 00c556cd
        add edx,ecx // 00c556d0
        mov ecx,dword ptr [eax] // 00c556d2
        mov dword ptr [ebp + -020h],edx // 00c556d4
        mov edx,dword ptr [ebp + -010h] // 00c556d7
        mov dword ptr [eax],edx // 00c556da
        add edx,ecx // 00c556dc
        mov ecx,dword ptr [eax + 04h] // 00c556de
        mov dword ptr [ebp + -010h],edx // 00c556e1
        mov edx,dword ptr [ebp + -04h] // 00c556e4
        mov dword ptr [eax + 04h],edx // 00c556e7
        add edx,ecx // 00c556ea
        mov ecx,dword ptr [eax + 08h] // 00c556ec
        mov dword ptr [ebp + -04h],edx // 00c556ef
        mov edx,dword ptr [ebp + -08h] // 00c556f2
        mov dword ptr [eax + 08h],edx // 00c556f5
        add edx,ecx // 00c556f8
        add eax,010h // 00c556fa
        sub dword ptr [ebp + -01ch],01h // 00c556fd
        mov dword ptr [ebp + -08h],edx // 00c55701
        jnz l_00c556c7 // 00c55704
        test ebx,ebx // 00c55706
        jle l_00c557f4 // 00c55708
        mov eax,edi // 00c5570e
        mov dword ptr [ebp + -010h],ebx // 00c55710
    l_00c55713:
        movzx ecx,byte ptr [eax] // 00c55713
        shl ecx,04h // 00c55716
        lea edx,[ebp + ecx*01h + 0ffffefc0h] // 00c55719
        mov ecx,dword ptr [edx] // 00c55720
        lea ebx,[ecx + 01h] // 00c55722
        mov dword ptr [edx],ebx // 00c55725
        mov edx,dword ptr [eax] // 00c55727
        mov dword ptr [esi + ecx*08h],edx // 00c55729
        mov edx,dword ptr [eax + 04h] // 00c5572c
        add eax,08h // 00c5572f
        sub dword ptr [ebp + -010h],01h // 00c55732
        mov dword ptr [esi + ecx*08h + 04h],edx // 00c55736
        jnz l_00c55713 // 00c5573a
        mov ebx,dword ptr [ebp + -0ch] // 00c5573c
        test ebx,ebx // 00c5573f
        jle l_00c557f4 // 00c55741
        mov eax,esi // 00c55747
        mov dword ptr [ebp + -010h],ebx // 00c55749
        lea esp,[esp] // 00c5574c
    l_00c55750:
        movzx ecx,byte ptr [eax + 01h] // 00c55750
        shl ecx,04h // 00c55754
        lea edx,[ebp + ecx*01h + 0ffffefc4h] // 00c55757
        mov ecx,dword ptr [edx] // 00c5575e
        lea ebx,[ecx + 01h] // 00c55760
        mov dword ptr [edx],ebx // 00c55763
        mov edx,dword ptr [eax] // 00c55765
        mov dword ptr [edi + ecx*08h],edx // 00c55767
        mov edx,dword ptr [eax + 04h] // 00c5576a
        add eax,08h // 00c5576d
        sub dword ptr [ebp + -010h],01h // 00c55770
        mov dword ptr [edi + ecx*08h + 04h],edx // 00c55774
        jnz l_00c55750 // 00c55778
        mov ebx,dword ptr [ebp + -0ch] // 00c5577a
        test ebx,ebx // 00c5577d
        jle l_00c557f4 // 00c5577f
        mov ecx,edi // 00c55785
        mov dword ptr [ebp + -010h],ebx // 00c55787
    l_00c5578a:
        mov eax,dword ptr [ecx] // 00c5578a
        shr eax,010h // 00c5578c
        and eax,0ffh // 00c5578f
        shl eax,04h // 00c55794
        lea edx,[ebp + eax*01h + 0ffffefc8h] // 00c55797
        mov eax,dword ptr [edx] // 00c5579e
        lea ebx,[eax + 01h] // 00c557a0
        mov dword ptr [edx],ebx // 00c557a3
        mov edx,dword ptr [ecx] // 00c557a5
        mov dword ptr [esi + eax*08h],edx // 00c557a7
        mov edx,dword ptr [ecx + 04h] // 00c557aa
        add ecx,08h // 00c557ad
        sub dword ptr [ebp + -010h],01h // 00c557b0
        mov dword ptr [esi + eax*08h + 04h],edx // 00c557b4
        jnz l_00c5578a // 00c557b8
        mov ebx,dword ptr [ebp + -0ch] // 00c557ba
        test ebx,ebx // 00c557bd
        jle l_00c557f4 // 00c557bf
        mov eax,esi // 00c557c1
        mov dword ptr [ebp + -010h],ebx // 00c557c3
    l_00c557c6:
        mov ecx,dword ptr [eax] // 00c557c6
        shr ecx,018h // 00c557c8
        shl ecx,04h // 00c557cb
        lea edx,[ebp + ecx*01h + 0ffffefcch] // 00c557ce
        mov ecx,dword ptr [edx] // 00c557d5
        lea ebx,[ecx + 01h] // 00c557d7
        mov dword ptr [edx],ebx // 00c557da
        mov edx,dword ptr [eax] // 00c557dc
        mov dword ptr [edi + ecx*08h],edx // 00c557de
        mov edx,dword ptr [eax + 04h] // 00c557e1
        add eax,08h // 00c557e4
        sub dword ptr [ebp + -010h],01h // 00c557e7
        mov dword ptr [edi + ecx*08h + 04h],edx // 00c557eb
        jnz l_00c557c6 // 00c557ef
        mov ebx,dword ptr [ebp + -0ch] // 00c557f1
    l_00c557f4:
        mov eax,dword ptr [ebp + -024h] // 00c557f4
        mov edi,dword ptr [eax + 01ch] // 00c557f7
        push 01000h // 00c557fa
        lea ecx,[ebp + 0ffffefc0h] // 00c557ff
        push 00h // 00c55805
        push ecx // 00c55807
        push dword ptr [ebp+8] // Borrow the same explicit Context.
        call memset_shim // 00c55808
        add esp,0ch // 00c5580d
        xor ecx,ecx // 00c55810
        test ebx,ebx // 00c55812
        jle l_00c5588b // 00c55814
        lea ebx,[ebx] // 00c5581a
    l_00c55820:
        mov eax,dword ptr [edi + ecx*08h] // 00c55820
        mov edx,eax // 00c55823
        and edx,0ffh // 00c55825
        shl edx,04h // 00c5582b
        add dword ptr [ebp + edx*01h + 0ffffefc0h],01h // 00c5582e
        lea edx,[ebp + edx*01h + 0ffffefc0h] // 00c55836
        movzx edx,ah // 00c5583d
        shl edx,04h // 00c55840
        add dword ptr [ebp + edx*01h + 0ffffefc4h],01h // 00c55843
        lea edx,[ebp + edx*01h + 0ffffefc4h] // 00c5584b
        mov edx,eax // 00c55852
        shr edx,010h // 00c55854
        and edx,0ffh // 00c55857
        shr eax,018h // 00c5585d
        shl edx,04h // 00c55860
        add dword ptr [ebp + edx*01h + 0ffffefc8h],01h // 00c55863
        shl eax,04h // 00c5586b
        add dword ptr [ebp + eax*01h + 0ffffefcch],01h // 00c5586e
        lea edx,[ebp + edx*01h + 0ffffefc8h] // 00c55876
        lea eax,[ebp + eax*01h + 0ffffefcch] // 00c5587d
        add ecx,01h // 00c55884
        cmp ecx,ebx // 00c55887
        jl l_00c55820 // 00c55889
    l_00c5588b:
        xor ecx,ecx // 00c5588b
        xor edx,edx // 00c5588d
        mov dword ptr [ebp + -010h],ecx // 00c5588f
        mov dword ptr [ebp + -04h],ecx // 00c55892
        lea eax,[ebp + 0ffffefc4h] // 00c55895
        mov dword ptr [ebp + -08h],0100h // 00c5589b
        jmp l_00c558a7 // 00c558a2
    l_00c558a4:
        mov edx,dword ptr [ebp + -01ch] // 00c558a4
    l_00c558a7:
        mov ebx,dword ptr [eax + -04h] // 00c558a7
        mov dword ptr [eax + -04h],ecx // 00c558aa
        add ecx,ebx // 00c558ad
        mov ebx,dword ptr [eax] // 00c558af
        mov dword ptr [eax],edx // 00c558b1
        add edx,ebx // 00c558b3
        mov ebx,dword ptr [ebp + -010h] // 00c558b5
        mov dword ptr [ebp + -01ch],edx // 00c558b8
        mov edx,dword ptr [eax + 04h] // 00c558bb
        mov dword ptr [eax + 04h],ebx // 00c558be
        add ebx,edx // 00c558c1
        mov edx,dword ptr [eax + 08h] // 00c558c3
        mov dword ptr [ebp + -010h],ebx // 00c558c6
        mov ebx,dword ptr [ebp + -04h] // 00c558c9
        mov dword ptr [eax + 08h],ebx // 00c558cc
        add ebx,edx // 00c558cf
        add eax,010h // 00c558d1
        sub dword ptr [ebp + -08h],01h // 00c558d4
        mov dword ptr [ebp + -04h],ebx // 00c558d8
        jnz l_00c558a4 // 00c558db
        mov ebx,dword ptr [ebp + -0ch] // 00c558dd
        test ebx,ebx // 00c558e0
        jle l_00c559d0 // 00c558e2
        mov eax,edi // 00c558e8
        mov dword ptr [ebp + -010h],ebx // 00c558ea
        lea ecx,[ecx] // 00c558ed
    l_00c558f0:
        movzx ecx,byte ptr [eax] // 00c558f0
        shl ecx,04h // 00c558f3
        lea edx,[ebp + ecx*01h + 0ffffefc0h] // 00c558f6
        mov ecx,dword ptr [edx] // 00c558fd
        lea ebx,[ecx + 01h] // 00c558ff
        mov dword ptr [edx],ebx // 00c55902
        mov edx,dword ptr [eax] // 00c55904
        mov dword ptr [esi + ecx*08h],edx // 00c55906
        mov edx,dword ptr [eax + 04h] // 00c55909
        add eax,08h // 00c5590c
        sub dword ptr [ebp + -010h],01h // 00c5590f
        mov dword ptr [esi + ecx*08h + 04h],edx // 00c55913
        jnz l_00c558f0 // 00c55917
        mov ebx,dword ptr [ebp + -0ch] // 00c55919
        test ebx,ebx // 00c5591c
        jle l_00c559d0 // 00c5591e
        mov eax,esi // 00c55924
        mov dword ptr [ebp + -010h],ebx // 00c55926
        lea esp,[esp] // 00c55929
    l_00c55930:
        movzx ecx,byte ptr [eax + 01h] // 00c55930
        shl ecx,04h // 00c55934
        lea edx,[ebp + ecx*01h + 0ffffefc4h] // 00c55937
        mov ecx,dword ptr [edx] // 00c5593e
        lea ebx,[ecx + 01h] // 00c55940
        mov dword ptr [edx],ebx // 00c55943
        mov edx,dword ptr [eax] // 00c55945
        mov dword ptr [edi + ecx*08h],edx // 00c55947
        mov edx,dword ptr [eax + 04h] // 00c5594a
        add eax,08h // 00c5594d
        sub dword ptr [ebp + -010h],01h // 00c55950
        mov dword ptr [edi + ecx*08h + 04h],edx // 00c55954
        jnz l_00c55930 // 00c55958
        mov ebx,dword ptr [ebp + -0ch] // 00c5595a
        test ebx,ebx // 00c5595d
        jle l_00c559d0 // 00c5595f
        mov ecx,edi // 00c55965
        mov dword ptr [ebp + -010h],ebx // 00c55967
    l_00c5596a:
        mov eax,dword ptr [ecx] // 00c5596a
        shr eax,010h // 00c5596c
        and eax,0ffh // 00c5596f
        shl eax,04h // 00c55974
        lea edx,[ebp + eax*01h + 0ffffefc8h] // 00c55977
        mov eax,dword ptr [edx] // 00c5597e
        lea ebx,[eax + 01h] // 00c55980
        mov dword ptr [edx],ebx // 00c55983
        mov edx,dword ptr [ecx] // 00c55985
        mov dword ptr [esi + eax*08h],edx // 00c55987
        mov edx,dword ptr [ecx + 04h] // 00c5598a
        add ecx,08h // 00c5598d
        sub dword ptr [ebp + -010h],01h // 00c55990
        mov dword ptr [esi + eax*08h + 04h],edx // 00c55994
        jnz l_00c5596a // 00c55998
        mov ebx,dword ptr [ebp + -0ch] // 00c5599a
        test ebx,ebx // 00c5599d
        jle l_00c559d0 // 00c5599f
        mov edx,ebx // 00c559a1
    l_00c559a3:
        mov eax,dword ptr [esi] // 00c559a3
        shr eax,018h // 00c559a5
        shl eax,04h // 00c559a8
        lea ecx,[ebp + eax*01h + 0ffffefcch] // 00c559ab
        mov eax,dword ptr [ecx] // 00c559b2
        lea ebx,[eax + 01h] // 00c559b4
        mov dword ptr [ecx],ebx // 00c559b7
        mov ecx,dword ptr [esi] // 00c559b9
        mov dword ptr [edi + eax*08h],ecx // 00c559bb
        mov ecx,dword ptr [esi + 04h] // 00c559be
        add esi,08h // 00c559c1
        sub edx,01h // 00c559c4
        mov dword ptr [edi + eax*08h + 04h],ecx // 00c559c7
        jnz l_00c559a3 // 00c559cb
        mov ebx,dword ptr [ebp + -0ch] // 00c559cd
    l_00c559d0:
        mov esi,dword ptr [ebp + -024h] // 00c559d0
        xor eax,eax // 00c559d3
        test ebx,ebx // 00c559d5
        jle l_00c55a44 // 00c559d7
        lea ecx,[ecx] // 00c559dd
    l_00c559e0:
        mov edx,dword ptr [esi + 04h] // 00c559e0
        test byte ptr [edx + eax*08h + 04h],01h // 00c559e3
        mov edx,dword ptr [edx + eax*08h + 04h] // 00c559e8
        movzx ecx,ax // 00c559ec
        jz l_00c559fa // 00c559ef
        and edx,0fffffffeh // 00c559f1
        mov word ptr [edx + 020h],cx // 00c559f4
        jmp l_00c55a01 // 00c559f8
    l_00c559fa:
        and edx,0fffffffeh // 00c559fa
        mov word ptr [edx + 026h],cx // 00c559fd
    l_00c55a01:
        mov edx,dword ptr [esi + 010h] // 00c55a01
        test byte ptr [edx + eax*08h + 04h],01h // 00c55a04
        mov edx,dword ptr [edx + eax*08h + 04h] // 00c55a09
        jz l_00c55a18 // 00c55a0d
        and edx,0fffffffeh // 00c55a0f
        mov word ptr [edx + 022h],cx // 00c55a12
        jmp l_00c55a1f // 00c55a16
    l_00c55a18:
        and edx,0fffffffeh // 00c55a18
        mov word ptr [edx + 028h],cx // 00c55a1b
    l_00c55a1f:
        mov edx,dword ptr [esi + 01ch] // 00c55a1f
        test byte ptr [edx + eax*08h + 04h],01h // 00c55a22
        mov edx,dword ptr [edx + eax*08h + 04h] // 00c55a27
        jz l_00c55a36 // 00c55a2b
        and edx,0fffffffeh // 00c55a2d
        mov word ptr [edx + 024h],cx // 00c55a30
        jmp l_00c55a3d // 00c55a34
    l_00c55a36:
        and edx,0fffffffeh // 00c55a36
        mov word ptr [edx + 02ah],cx // 00c55a39
    l_00c55a3d:
        add eax,01h // 00c55a3d
        cmp eax,ebx // 00c55a40
        jl l_00c559e0 // 00c55a42
    l_00c55a44:
        rdtsc // 00c55a44
        mov dword ptr [ebp + -010h],edx // 00c55a46
        lea esp,[ebp + 0ffffefb4h] // 00c55a49
        pop edi // 00c55a4f
        pop esi // 00c55a50
        pop ebx // 00c55a51
        mov esp,ebp // 00c55a52
        pop ebp // 00c55a54
        ret 4 // 00c55a55
    }
}
}  // namespace

U32 rebuild_radix_pairs_00c54da0(
    void* manager, const cc15_dyn_live::DynLiveCrt& crt,
    const volatile U32& actual_feature_word_0109eea4,
    const volatile double& actual_half_00d7a280) {
    const Context context{&crt, &actual_feature_word_0109eea4, &actual_half_00d7a280};
    return kernel(manager, nullptr, &context);
}

}  // namespace bsp::cc16_dyn_radix_update
