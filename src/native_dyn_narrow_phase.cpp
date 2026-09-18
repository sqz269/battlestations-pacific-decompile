#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "bsp/native_dyn_narrow_phase.hpp"
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native narrow phase requires MSVC Win32 x87 assembly.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
struct Context {const AvoidZoneDynHullMemory& memory;NativeDynNarrowPhaseCalls& calls;};
void* __cdecl allocate_bridge(U site,Context* c,U size){return site==0xc3f539?c->calls.malloc_00bf9f1a(site,size,c->memory):c->calls.allocate_00bf55be(site,size,c->memory);}
void __cdecl free_bridge(U site,Context* c,void* p){c->calls.free_00bf6989(site,p,c->memory);}
void __cdecl enter_bridge(U site,Context* c,void* p){c->calls.enter_critical_section(site,p);}
void __cdecl leave_bridge(U site,Context* c,void* p){c->calls.leave_critical_section(site,p);}
void append_kernel();
void manifold_kernel();
alignas(8) const std::uint32_t constant_00d7a208=0x80000000U;
alignas(8) const std::uint64_t constant_00d7a270=0x3fa99999a0000000ULL;
alignas(8) const std::uint64_t constant_00d7a280=0x3fe0000000000000ULL;
alignas(8) const std::uint32_t constant_00d7a2d8=0x3b23d70bU;
// Explicit context adapter; retain original argument offsets.
__declspec(naked) void manifold_shim(){
    __asm {
        push dword ptr [esp+4]
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        call manifold_kernel
        ret 0ch
    }
}
// Explicit context adapter; retain original argument offsets.
__declspec(naked) void append_shim(){
    __asm {
        push dword ptr [esp+4]
        push dword ptr [esp+0ch]
        call append_kernel
        ret 8
    }
}
// Explicit context adapter; retain original argument offsets.
__declspec(naked) void bridge_00c44272(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c44272h
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
// Explicit context adapter; retain original argument offsets.
__declspec(naked) void bridge_00c442cd(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c442cdh
        call free_bridge
        add esp,0ch
        ret 4
    }
}
// Explicit context adapter; retain original argument offsets.
__declspec(naked) void bridge_00c3f4e2(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c3f4e2h
        call enter_bridge
        add esp,0ch
        ret 8
    }
}
// Explicit context adapter; retain original argument offsets.
__declspec(naked) void bridge_00c3f539(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c3f539h
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
// Explicit context adapter; retain original argument offsets.
__declspec(naked) void bridge_00c3f57c(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c3f57ch
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
// Explicit context adapter; retain original argument offsets.
__declspec(naked) void bridge_00c3f5ad(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c3f5adh
        call free_bridge
        add esp,0ch
        ret 4
    }
}
// Explicit context adapter; retain original argument offsets.
__declspec(naked) void bridge_00c3f638(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c3f638h
        call leave_bridge
        add esp,0ch
        ret 8
    }
}
// Explicit context adapter; retain original argument offsets.
__declspec(naked) void bridge_00c35275(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c35275h
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
// Explicit context adapter; retain original argument offsets.
__declspec(naked) void bridge_00c352a7(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c352a7h
        call free_bridge
        add esp,0ch
        ret 4
    }
}
// Recovered instruction order, including x87 spills and reloads after calls.
// Address comments refer to the verified original PE instruction starts.
__declspec(naked) void abs_kernel(){
    __asm {
        push ebp // 00401170
        mov ebp, esp // 00401171
        and esp, 0fffffff8h // 00401173
        sub esp, 8 // 00401176
        fld dword ptr [ebp + 8] // 00401179
        fabs  // 0040117c
        fstp dword ptr [esp + 4] // 0040117e
        fld dword ptr [esp + 4] // 00401182
        mov esp, ebp // 00401186
        pop ebp // 00401188
        ret 4 // 00401189
    }
}
// Recovered instruction order, including x87 spills and reloads after calls.
// Address comments refer to the verified original PE instruction starts.
__declspec(naked) void match_kernel(){
    __asm {
        sub esp, 024h // 00c3f650
        push esi // 00c3f653
        mov esi, dword ptr [eax + 0c8h] // 00c3f654
        xor edx, edx // 00c3f65a
        test esi, esi // 00c3f65c
        jle l_00c3f73f // 00c3f65e
        movss xmm0, dword ptr [edi] // 00c3f664
        fld dword ptr constant_00d7a2d8 // 00c3f668
        movss dword ptr [esp + 4], xmm0 // 00c3f66e
        movss xmm0, dword ptr [edi + 4] // 00c3f674
        fld dword ptr [esp + 4] // 00c3f679
        movss dword ptr [esp + 8], xmm0 // 00c3f67d
        movss xmm0, dword ptr [edi + 8] // 00c3f683
        fld dword ptr [esp + 8] // 00c3f688
        movss dword ptr [esp + 0ch], xmm0 // 00c3f68c
        fld dword ptr [esp + 0ch] // 00c3f692
        lea ecx, [eax + 01ch] // 00c3f696
    l_00c3f699:
        fld dword ptr [ecx - 8] // 00c3f699
        fsub st(0), st(3) // 00c3f69c
        fstp dword ptr [esp + 010h] // 00c3f69e
        fld dword ptr [ecx - 4] // 00c3f6a2
        fsub st(0), st(2) // 00c3f6a5
        fstp dword ptr [esp + 014h] // 00c3f6a7
        fld dword ptr [ecx] // 00c3f6ab
        fsub st(0), st(1) // 00c3f6ad
        fstp dword ptr [esp + 018h] // 00c3f6af
        fld dword ptr [esp + 014h] // 00c3f6b3
        fld dword ptr [esp + 010h] // 00c3f6b7
        fld dword ptr [esp + 018h] // 00c3f6bb
        fld st(1) // 00c3f6bf
        fmulp st(2), st(0) // 00c3f6c1
        fld st(2) // 00c3f6c3
        fmulp st(3), st(0) // 00c3f6c5
        fxch st(1) // 00c3f6c7
        faddp st(2), st(0) // 00c3f6c9
        fmul st(0), st(0) // 00c3f6cb
        faddp st(1), st(0) // 00c3f6cd
        fstp dword ptr [esp + 0ch] // 00c3f6cf
        fld dword ptr [esp + 0ch] // 00c3f6d3
        fxch st(4) // 00c3f6d7
        fcomi st(0), st(4) // 00c3f6d9
        fstp st(4) // 00c3f6db
        ja l_00c3f746 // 00c3f6dd
        fld dword ptr [ecx + 4] // 00c3f6df
        fsub dword ptr [edi + 0ch] // 00c3f6e2
        fstp dword ptr [esp + 01ch] // 00c3f6e5
        fld dword ptr [ecx + 8] // 00c3f6e9
        fsub dword ptr [edi + 010h] // 00c3f6ec
        fstp dword ptr [esp + 020h] // 00c3f6ef
        fld dword ptr [ecx + 0ch] // 00c3f6f3
        fsub dword ptr [edi + 014h] // 00c3f6f6
        fstp dword ptr [esp + 024h] // 00c3f6f9
        fld dword ptr [esp + 020h] // 00c3f6fd
        fld dword ptr [esp + 01ch] // 00c3f701
        fld dword ptr [esp + 024h] // 00c3f705
        fld st(1) // 00c3f709
        fmulp st(2), st(0) // 00c3f70b
        fld st(2) // 00c3f70d
        fmulp st(3), st(0) // 00c3f70f
        fxch st(1) // 00c3f711
        faddp st(2), st(0) // 00c3f713
        fmul st(0), st(0) // 00c3f715
        faddp st(1), st(0) // 00c3f717
        fstp dword ptr [esp + 0ch] // 00c3f719
        fld dword ptr [esp + 0ch] // 00c3f71d
        fxch st(4) // 00c3f721
        fcomi st(0), st(4) // 00c3f723
        fstp st(4) // 00c3f725
        ja l_00c3f746 // 00c3f727
        add edx, 1 // 00c3f729
        add ecx, 030h // 00c3f72c
        cmp edx, esi // 00c3f72f
        jl l_00c3f699 // 00c3f731
        fstp st(3) // 00c3f737
        fstp st(1) // 00c3f739
        fstp st(0) // 00c3f73b
        fstp st(0) // 00c3f73d
    l_00c3f73f:
        mov eax, esi // 00c3f73f
        pop esi // 00c3f741
        add esp, 024h // 00c3f742
        ret  // 00c3f745
    l_00c3f746:
        fstp st(3) // 00c3f746
        mov eax, edx // 00c3f748
        fstp st(1) // 00c3f74a
        pop esi // 00c3f74c
        fstp st(0) // 00c3f74d
        fstp st(0) // 00c3f74f
        add esp, 024h // 00c3f751
        ret  // 00c3f754
    }
}
// Recovered instruction order, including x87 spills and reloads after calls.
// Address comments refer to the verified original PE instruction starts.
__declspec(naked) void insert_kernel(){
    __asm {
        sub esp, 040h // 00c3f760
        push esi // 00c3f763
        push edi // 00c3f764
        mov edi, eax // 00c3f765
        fld dword ptr [edi + 01ch] // 00c3f767
        push ecx // 00c3f76a
        fld dword ptr [edi + 018h] // 00c3f76b
        mov esi, ecx // 00c3f76e
        fld dword ptr [edi + 020h] // 00c3f770
        fld st(1) // 00c3f773
        fmulp st(2), st(0) // 00c3f775
        fld st(2) // 00c3f777
        fmulp st(3), st(0) // 00c3f779
        fxch st(1) // 00c3f77b
        faddp st(2), st(0) // 00c3f77d
        fmul st(0), st(0) // 00c3f77f
        faddp st(1), st(0) // 00c3f781
        fstp dword ptr [esp + 0ch] // 00c3f783
        fld dword ptr [esp + 0ch] // 00c3f787
        fld1  // 00c3f78b
        fsubrp st(1), st(0) // 00c3f78d
        fstp dword ptr [esp + 0ch] // 00c3f78f
        fld dword ptr [esp + 0ch] // 00c3f793
        fstp dword ptr [esp] // 00c3f797
        call abs_kernel // 00c3f79a
        fld qword ptr constant_00d7a270 // 00c3f79f
        fxch st(1) // 00c3f7a5
        fcomip st(0), st(1) // 00c3f7a7
        fstp st(0) // 00c3f7a9
        ja l_00c3ffd0 // 00c3f7ab
        mov eax, esi // 00c3f7b1
        call match_kernel // 00c3f7b3
        mov ecx, dword ptr [esi + 0c8h] // 00c3f7b8
        cmp eax, ecx // 00c3f7be
        jge l_00c3f943 // 00c3f7c0
        mov ecx, dword ptr [edi] // 00c3f7c6
        lea eax, [eax + eax*2] // 00c3f7c8
        shl eax, 4 // 00c3f7cb
        mov dword ptr [eax + esi + 014h], ecx // 00c3f7ce
        mov edx, dword ptr [edi + 4] // 00c3f7d2
        mov dword ptr [eax + esi + 018h], edx // 00c3f7d5
        mov ecx, dword ptr [edi + 8] // 00c3f7d9
        mov dword ptr [eax + esi + 01ch], ecx // 00c3f7dc
        mov edx, dword ptr [edi + 0ch] // 00c3f7e0
        mov dword ptr [eax + esi + 020h], edx // 00c3f7e3
        mov ecx, dword ptr [edi + 010h] // 00c3f7e7
        add eax, esi // 00c3f7ea
        mov dword ptr [eax + 024h], ecx // 00c3f7ec
        mov edx, dword ptr [edi + 014h] // 00c3f7ef
        mov dword ptr [eax + 028h], edx // 00c3f7f2
        mov ecx, dword ptr [edi + 018h] // 00c3f7f5
        mov dword ptr [eax + 8], ecx // 00c3f7f8
        mov edx, dword ptr [edi + 01ch] // 00c3f7fb
        mov dword ptr [eax + 0ch], edx // 00c3f7fe
        mov ecx, dword ptr [edi + 020h] // 00c3f801
        mov dword ptr [eax + 010h], ecx // 00c3f804
        fld dword ptr [eax + 024h] // 00c3f807
        fstp dword ptr [esp + 8] // 00c3f80a
        mov ecx, dword ptr [esi + 0d0h] // 00c3f80e
        fld dword ptr [eax + 020h] // 00c3f814
        add ecx, 8 // 00c3f817
        fstp dword ptr [esp + 0ch] // 00c3f81a
        mov esi, dword ptr [esi + 0cch] // 00c3f81e
        fld dword ptr [eax + 028h] // 00c3f824
        add esi, 8 // 00c3f827
        fstp dword ptr [esp + 010h] // 00c3f82a
        fld dword ptr [ecx + 0ch] // 00c3f82e
        fld dword ptr [esp + 8] // 00c3f831
        fld st(0) // 00c3f835
        fmulp st(2), st(0) // 00c3f837
        fld dword ptr [esp + 0ch] // 00c3f839
        fld st(0) // 00c3f83d
        fmul dword ptr [ecx] // 00c3f83f
        faddp st(3), st(0) // 00c3f841
        fld dword ptr [ecx + 018h] // 00c3f843
        fld dword ptr [esp + 010h] // 00c3f846
        fld st(0) // 00c3f84a
        fmulp st(2), st(0) // 00c3f84c
        fxch st(4) // 00c3f84e
        faddp st(1), st(0) // 00c3f850
        fadd dword ptr [ecx + 024h] // 00c3f852
        fstp dword ptr [esp + 014h] // 00c3f855
        fld dword ptr [ecx + 4] // 00c3f859
        fmul st(0), st(1) // 00c3f85c
        fld dword ptr [ecx + 010h] // 00c3f85e
        fmul st(0), st(3) // 00c3f861
        faddp st(1), st(0) // 00c3f863
        fld dword ptr [ecx + 01ch] // 00c3f865
        fmul st(0), st(4) // 00c3f868
        faddp st(1), st(0) // 00c3f86a
        fadd dword ptr [ecx + 028h] // 00c3f86c
        fstp dword ptr [esp + 018h] // 00c3f86f
        fmul dword ptr [ecx + 8] // 00c3f873
        fld dword ptr [ecx + 014h] // 00c3f876
        fmulp st(2), st(0) // 00c3f879
        faddp st(1), st(0) // 00c3f87b
        fld dword ptr [ecx + 020h] // 00c3f87d
        fmulp st(2), st(0) // 00c3f880
        faddp st(1), st(0) // 00c3f882
        fadd dword ptr [ecx + 02ch] // 00c3f884
        fstp dword ptr [esp + 01ch] // 00c3f887
        fld dword ptr [eax + 018h] // 00c3f88b
        fstp dword ptr [esp + 010h] // 00c3f88e
        fld dword ptr [eax + 014h] // 00c3f892
        fstp dword ptr [esp + 0ch] // 00c3f895
        fld dword ptr [eax + 01ch] // 00c3f899
        fstp dword ptr [esp + 8] // 00c3f89c
        fld dword ptr [esi + 0ch] // 00c3f8a0
        fld dword ptr [esp + 010h] // 00c3f8a3
        fld st(0) // 00c3f8a7
        fmulp st(2), st(0) // 00c3f8a9
        fld dword ptr [esp + 0ch] // 00c3f8ab
        fld st(0) // 00c3f8af
        fmul dword ptr [esi] // 00c3f8b1
        faddp st(3), st(0) // 00c3f8b3
        fld dword ptr [esi + 018h] // 00c3f8b5
        fld dword ptr [esp + 8] // 00c3f8b8
        pop edi // 00c3f8bc
        fld st(0) // 00c3f8bd
        fmulp st(2), st(0) // 00c3f8bf
        fxch st(4) // 00c3f8c1
        faddp st(1), st(0) // 00c3f8c3
        fadd dword ptr [esi + 024h] // 00c3f8c5
        fstp dword ptr [esp + 01ch] // 00c3f8c8
        fld dword ptr [esi + 4] // 00c3f8cc
        fmul st(0), st(1) // 00c3f8cf
        fld dword ptr [esi + 010h] // 00c3f8d1
        fmul st(0), st(3) // 00c3f8d4
        faddp st(1), st(0) // 00c3f8d6
        fld dword ptr [esi + 01ch] // 00c3f8d8
        fmul st(0), st(4) // 00c3f8db
        faddp st(1), st(0) // 00c3f8dd
        fadd dword ptr [esi + 028h] // 00c3f8df
        fstp dword ptr [esp + 020h] // 00c3f8e2
        fmul dword ptr [esi + 8] // 00c3f8e6
        fld dword ptr [esi + 014h] // 00c3f8e9
        fmulp st(2), st(0) // 00c3f8ec
        faddp st(1), st(0) // 00c3f8ee
        fld dword ptr [esi + 020h] // 00c3f8f0
        fmulp st(2), st(0) // 00c3f8f3
        faddp st(1), st(0) // 00c3f8f5
        fadd dword ptr [esi + 02ch] // 00c3f8f7
        pop esi // 00c3f8fa
        fstp dword ptr [esp + 020h] // 00c3f8fb
        fld dword ptr [esp + 018h] // 00c3f8ff
        fsub dword ptr [esp + 0ch] // 00c3f903
        fstp dword ptr [esp + 024h] // 00c3f907
        fld dword ptr [esp + 01ch] // 00c3f90b
        fsub dword ptr [esp + 010h] // 00c3f90f
        fstp dword ptr [esp + 028h] // 00c3f913
        fld dword ptr [esp + 020h] // 00c3f917
        fsub dword ptr [esp + 014h] // 00c3f91b
        fstp dword ptr [esp + 02ch] // 00c3f91f
        fld dword ptr [eax + 0ch] // 00c3f923
        fmul dword ptr [esp + 028h] // 00c3f926
        fld dword ptr [eax + 8] // 00c3f92a
        fmul dword ptr [esp + 024h] // 00c3f92d
        faddp st(1), st(0) // 00c3f931
        fld dword ptr [eax + 010h] // 00c3f933
        fmul dword ptr [esp + 02ch] // 00c3f936
        faddp st(1), st(0) // 00c3f93a
        fstp dword ptr [eax + 034h] // 00c3f93c
        add esp, 040h // 00c3f93f
        ret  // 00c3f942
    l_00c3f943:
        cmp ecx, 4 // 00c3f943
        jge l_00c3fb29 // 00c3f946
        lea edx, [ecx + ecx*2] // 00c3f94c
        mov ecx, dword ptr [edi] // 00c3f94f
        shl edx, 4 // 00c3f951
        mov dword ptr [edx + esi + 014h], ecx // 00c3f954
        lea eax, [edx + esi + 014h] // 00c3f958
        mov edx, dword ptr [edi + 4] // 00c3f95c
        mov dword ptr [eax + 4], edx // 00c3f95f
        mov ecx, dword ptr [edi + 8] // 00c3f962
        mov dword ptr [eax + 8], ecx // 00c3f965
        mov eax, dword ptr [esi + 0c8h] // 00c3f968
        mov ecx, dword ptr [edi + 0ch] // 00c3f96e
        lea edx, [eax + eax*2] // 00c3f971
        shl edx, 4 // 00c3f974
        mov dword ptr [edx + esi + 020h], ecx // 00c3f977
        lea eax, [edx + esi + 020h] // 00c3f97b
        mov edx, dword ptr [edi + 010h] // 00c3f97f
        mov dword ptr [eax + 4], edx // 00c3f982
        mov ecx, dword ptr [edi + 014h] // 00c3f985
        mov dword ptr [eax + 8], ecx // 00c3f988
        mov eax, dword ptr [esi + 0c8h] // 00c3f98b
        mov ecx, dword ptr [edi + 018h] // 00c3f991
        xorps xmm0, xmm0 // 00c3f994
        lea edx, [eax + eax*2] // 00c3f997
        shl edx, 4 // 00c3f99a
        mov dword ptr [edx + esi + 8], ecx // 00c3f99d
        lea eax, [edx + esi + 8] // 00c3f9a1
        mov edx, dword ptr [edi + 01ch] // 00c3f9a5
        mov dword ptr [eax + 4], edx // 00c3f9a8
        mov ecx, dword ptr [edi + 020h] // 00c3f9ab
        mov dword ptr [eax + 8], ecx // 00c3f9ae
        mov eax, dword ptr [esi + 0c8h] // 00c3f9b1
        lea edx, [eax + eax*2] // 00c3f9b7
        shl edx, 4 // 00c3f9ba
        movss dword ptr [edx + esi + 02ch], xmm0 // 00c3f9bd
        mov eax, dword ptr [esi + 0c8h] // 00c3f9c3
        add eax, 1 // 00c3f9c9
        lea eax, [eax + eax*2] // 00c3f9cc
        shl eax, 4 // 00c3f9cf
        movss dword ptr [eax + esi], xmm0 // 00c3f9d2
        mov eax, dword ptr [esi + 0c8h] // 00c3f9d7
        lea ecx, [eax + eax*2] // 00c3f9dd
        mov eax, dword ptr [esi + 0d0h] // 00c3f9e0
        shl ecx, 4 // 00c3f9e6
        fld dword ptr [ecx + esi + 024h] // 00c3f9e9
        add ecx, esi // 00c3f9ed
        fstp dword ptr [esp + 010h] // 00c3f9ef
        add eax, 8 // 00c3f9f3
        fld dword ptr [ecx + 020h] // 00c3f9f6
        fstp dword ptr [esp + 0ch] // 00c3f9f9
        fld dword ptr [ecx + 028h] // 00c3f9fd
        fstp dword ptr [esp + 8] // 00c3fa00
        fld dword ptr [eax + 0ch] // 00c3fa04
        fld dword ptr [esp + 010h] // 00c3fa07
        fld st(0) // 00c3fa0b
        fmulp st(2), st(0) // 00c3fa0d
        fld dword ptr [esp + 0ch] // 00c3fa0f
        fld st(0) // 00c3fa13
        fmul dword ptr [eax] // 00c3fa15
        faddp st(3), st(0) // 00c3fa17
        fld dword ptr [eax + 018h] // 00c3fa19
        fld dword ptr [esp + 8] // 00c3fa1c
        fld st(0) // 00c3fa20
        fmulp st(2), st(0) // 00c3fa22
        fxch st(4) // 00c3fa24
        faddp st(1), st(0) // 00c3fa26
        fadd dword ptr [eax + 024h] // 00c3fa28
        fstp dword ptr [esp + 020h] // 00c3fa2b
        fld dword ptr [eax + 4] // 00c3fa2f
        fmul st(0), st(1) // 00c3fa32
        fld dword ptr [eax + 010h] // 00c3fa34
        fmul st(0), st(3) // 00c3fa37
        faddp st(1), st(0) // 00c3fa39
        fld dword ptr [eax + 01ch] // 00c3fa3b
        fmul st(0), st(4) // 00c3fa3e
        faddp st(1), st(0) // 00c3fa40
        fadd dword ptr [eax + 028h] // 00c3fa42
        fstp dword ptr [esp + 024h] // 00c3fa45
        fmul dword ptr [eax + 8] // 00c3fa49
        fld dword ptr [eax + 014h] // 00c3fa4c
        fmulp st(2), st(0) // 00c3fa4f
        faddp st(1), st(0) // 00c3fa51
        fld dword ptr [eax + 020h] // 00c3fa53
        pop edi // 00c3fa56
        fmulp st(2), st(0) // 00c3fa57
        faddp st(1), st(0) // 00c3fa59
        fadd dword ptr [eax + 02ch] // 00c3fa5b
        mov eax, dword ptr [esi + 0cch] // 00c3fa5e
        add eax, 8 // 00c3fa64
        fstp dword ptr [esp + 024h] // 00c3fa67
        fld dword ptr [ecx + 018h] // 00c3fa6b
        fstp dword ptr [esp + 0ch] // 00c3fa6e
        fld dword ptr [ecx + 014h] // 00c3fa72
        fstp dword ptr [esp + 8] // 00c3fa75
        fld dword ptr [ecx + 01ch] // 00c3fa79
        fstp dword ptr [esp + 4] // 00c3fa7c
        fld dword ptr [eax + 0ch] // 00c3fa80
        fld dword ptr [esp + 0ch] // 00c3fa83
        fld st(0) // 00c3fa87
        fmulp st(2), st(0) // 00c3fa89
        fld dword ptr [esp + 8] // 00c3fa8b
        fld st(0) // 00c3fa8f
        fmul dword ptr [eax] // 00c3fa91
        faddp st(3), st(0) // 00c3fa93
        fld dword ptr [eax + 018h] // 00c3fa95
        fld dword ptr [esp + 4] // 00c3fa98
        fld st(0) // 00c3fa9c
        fmulp st(2), st(0) // 00c3fa9e
        fxch st(4) // 00c3faa0
        faddp st(1), st(0) // 00c3faa2
        fadd dword ptr [eax + 024h] // 00c3faa4
        fstp dword ptr [esp + 028h] // 00c3faa7
        fld dword ptr [eax + 4] // 00c3faab
        fmul st(0), st(1) // 00c3faae
        fld dword ptr [eax + 010h] // 00c3fab0
        fmul st(0), st(3) // 00c3fab3
        faddp st(1), st(0) // 00c3fab5
        fld dword ptr [eax + 01ch] // 00c3fab7
        fmul st(0), st(4) // 00c3faba
        faddp st(1), st(0) // 00c3fabc
        fadd dword ptr [eax + 028h] // 00c3fabe
        fstp dword ptr [esp + 02ch] // 00c3fac1
        fmul dword ptr [eax + 8] // 00c3fac5
        fld dword ptr [eax + 014h] // 00c3fac8
        fmulp st(2), st(0) // 00c3facb
        faddp st(1), st(0) // 00c3facd
        fld dword ptr [eax + 020h] // 00c3facf
        fmulp st(2), st(0) // 00c3fad2
        faddp st(1), st(0) // 00c3fad4
        fadd dword ptr [eax + 02ch] // 00c3fad6
        fstp dword ptr [esp + 030h] // 00c3fad9
        fld dword ptr [esp + 028h] // 00c3fadd
        fsub dword ptr [esp + 01ch] // 00c3fae1
        fstp dword ptr [esp + 010h] // 00c3fae5
        fld dword ptr [esp + 02ch] // 00c3fae9
        fsub dword ptr [esp + 020h] // 00c3faed
        fstp dword ptr [esp + 014h] // 00c3faf1
        fld dword ptr [esp + 030h] // 00c3faf5
        fsub dword ptr [esp + 024h] // 00c3faf9
        fstp dword ptr [esp + 018h] // 00c3fafd
        fld dword ptr [ecx + 0ch] // 00c3fb01
        fmul dword ptr [esp + 014h] // 00c3fb04
        fld dword ptr [ecx + 8] // 00c3fb08
        fmul dword ptr [esp + 010h] // 00c3fb0b
        faddp st(1), st(0) // 00c3fb0f
        fld dword ptr [ecx + 010h] // 00c3fb11
        fmul dword ptr [esp + 018h] // 00c3fb14
        faddp st(1), st(0) // 00c3fb18
        fstp dword ptr [ecx + 034h] // 00c3fb1a
        add dword ptr [esi + 0c8h], 1 // 00c3fb1d
        pop esi // 00c3fb24
        add esp, 040h // 00c3fb25
        ret  // 00c3fb28
    l_00c3fb29:
        fld dword ptr [edi + 010h] // 00c3fb29
        mov eax, dword ptr [esi + 0d0h] // 00c3fb2c
        fstp dword ptr [esp + 010h] // 00c3fb32
        add eax, 8 // 00c3fb36
        fld dword ptr [edi + 0ch] // 00c3fb39
        fstp dword ptr [esp + 0ch] // 00c3fb3c
        fld dword ptr [edi + 014h] // 00c3fb40
        fstp dword ptr [esp + 8] // 00c3fb43
        fld dword ptr [eax + 0ch] // 00c3fb47
        fld dword ptr [esp + 010h] // 00c3fb4a
        fld st(0) // 00c3fb4e
        fmulp st(2), st(0) // 00c3fb50
        fld dword ptr [eax] // 00c3fb52
        fld dword ptr [esp + 0ch] // 00c3fb54
        fld st(0) // 00c3fb58
        fmulp st(2), st(0) // 00c3fb5a
        fxch st(3) // 00c3fb5c
        faddp st(1), st(0) // 00c3fb5e
        fld dword ptr [eax + 018h] // 00c3fb60
        fld dword ptr [esp + 8] // 00c3fb63
        fld st(0) // 00c3fb67
        fmulp st(2), st(0) // 00c3fb69
        fxch st(2) // 00c3fb6b
        faddp st(1), st(0) // 00c3fb6d
        fadd dword ptr [eax + 024h] // 00c3fb6f
        fstp dword ptr [esp + 020h] // 00c3fb72
        fld dword ptr [eax + 4] // 00c3fb76
        fmul st(0), st(3) // 00c3fb79
        fld dword ptr [eax + 010h] // 00c3fb7b
        fmul st(0), st(3) // 00c3fb7e
        faddp st(1), st(0) // 00c3fb80
        fld dword ptr [eax + 01ch] // 00c3fb82
        fmul st(0), st(2) // 00c3fb85
        faddp st(1), st(0) // 00c3fb87
        fadd dword ptr [eax + 028h] // 00c3fb89
        fstp dword ptr [esp + 024h] // 00c3fb8c
        fld dword ptr [eax + 8] // 00c3fb90
        fmulp st(3), st(0) // 00c3fb93
        fld dword ptr [eax + 014h] // 00c3fb95
        fmulp st(2), st(0) // 00c3fb98
        fxch st(2) // 00c3fb9a
        faddp st(1), st(0) // 00c3fb9c
        fld dword ptr [eax + 020h] // 00c3fb9e
        fmulp st(2), st(0) // 00c3fba1
        faddp st(1), st(0) // 00c3fba3
        fadd dword ptr [eax + 02ch] // 00c3fba5
        mov eax, dword ptr [esi + 0cch] // 00c3fba8
        add eax, 8 // 00c3fbae
        fstp dword ptr [esp + 028h] // 00c3fbb1
        fld dword ptr [edi + 4] // 00c3fbb5
        fstp dword ptr [esp + 010h] // 00c3fbb8
        fld dword ptr [edi] // 00c3fbbc
        fstp dword ptr [esp + 0ch] // 00c3fbbe
        fld dword ptr [edi + 8] // 00c3fbc2
        fstp dword ptr [esp + 8] // 00c3fbc5
        fld dword ptr [eax + 0ch] // 00c3fbc9
        fld dword ptr [esp + 010h] // 00c3fbcc
        fld st(0) // 00c3fbd0
        fmulp st(2), st(0) // 00c3fbd2
        fld dword ptr [eax] // 00c3fbd4
        fld dword ptr [esp + 0ch] // 00c3fbd6
        fld st(0) // 00c3fbda
        fmulp st(2), st(0) // 00c3fbdc
        fxch st(3) // 00c3fbde
        faddp st(1), st(0) // 00c3fbe0
        fld dword ptr [eax + 018h] // 00c3fbe2
        fld dword ptr [esp + 8] // 00c3fbe5
        fld st(0) // 00c3fbe9
        fmulp st(2), st(0) // 00c3fbeb
        fxch st(2) // 00c3fbed
        faddp st(1), st(0) // 00c3fbef
        fadd dword ptr [eax + 024h] // 00c3fbf1
        fstp dword ptr [esp + 02ch] // 00c3fbf4
        fld dword ptr [eax + 4] // 00c3fbf8
        fmul st(0), st(3) // 00c3fbfb
        fld dword ptr [eax + 010h] // 00c3fbfd
        fmul st(0), st(3) // 00c3fc00
        faddp st(1), st(0) // 00c3fc02
        fld dword ptr [eax + 01ch] // 00c3fc04
        fmul st(0), st(2) // 00c3fc07
        faddp st(1), st(0) // 00c3fc09
        fadd dword ptr [eax + 028h] // 00c3fc0b
        fstp dword ptr [esp + 030h] // 00c3fc0e
        fld dword ptr [eax + 8] // 00c3fc12
        fmul st(0), st(3) // 00c3fc15
        fld dword ptr [eax + 014h] // 00c3fc17
        fmul st(0), st(3) // 00c3fc1a
        faddp st(1), st(0) // 00c3fc1c
        fld dword ptr [eax + 020h] // 00c3fc1e
        fmul st(0), st(2) // 00c3fc21
        faddp st(1), st(0) // 00c3fc23
        fadd dword ptr [eax + 02ch] // 00c3fc25
        fstp dword ptr [esp + 034h] // 00c3fc28
        fld dword ptr [esp + 02ch] // 00c3fc2c
        fsub dword ptr [esp + 020h] // 00c3fc30
        fstp dword ptr [esp + 014h] // 00c3fc34
        fld dword ptr [esp + 030h] // 00c3fc38
        fsub dword ptr [esp + 024h] // 00c3fc3c
        fstp dword ptr [esp + 018h] // 00c3fc40
        fld dword ptr [esp + 034h] // 00c3fc44
        fsub dword ptr [esp + 028h] // 00c3fc48
        fstp dword ptr [esp + 01ch] // 00c3fc4c
        fld dword ptr [edi + 01ch] // 00c3fc50
        fmul dword ptr [esp + 018h] // 00c3fc53
        fld dword ptr [edi + 018h] // 00c3fc57
        fmul dword ptr [esp + 014h] // 00c3fc5a
        faddp st(1), st(0) // 00c3fc5e
        fld dword ptr [edi + 020h] // 00c3fc60
        fmul dword ptr [esp + 01ch] // 00c3fc63
        faddp st(1), st(0) // 00c3fc67
        fstp dword ptr [esp + 0ch] // 00c3fc69
        fld st(2) // 00c3fc6d
        fsub dword ptr [esi + 044h] // 00c3fc6f
        fstp dword ptr [esp + 020h] // 00c3fc72
        fld dword ptr [esi + 048h] // 00c3fc76
        fsubp st(2), st(0) // 00c3fc79
        fxch st(1) // 00c3fc7b
        fstp dword ptr [esp + 024h] // 00c3fc7d
        fsub dword ptr [esi + 04ch] // 00c3fc81
        fstp dword ptr [esp + 028h] // 00c3fc84
        fld dword ptr [esi + 0a4h] // 00c3fc88
        fsub dword ptr [esi + 074h] // 00c3fc8e
        fstp dword ptr [esp + 02ch] // 00c3fc91
        fld dword ptr [esi + 0a8h] // 00c3fc95
        fsub dword ptr [esi + 078h] // 00c3fc9b
        fstp dword ptr [esp + 030h] // 00c3fc9e
        fld dword ptr [esi + 0ach] // 00c3fca2
        fsub dword ptr [esi + 07ch] // 00c3fca8
        fstp dword ptr [esp + 034h] // 00c3fcab
        fld dword ptr [esp + 034h] // 00c3fcaf
        fld st(0) // 00c3fcb3
        fld dword ptr [esp + 024h] // 00c3fcb5
        fld st(0) // 00c3fcb9
        fmulp st(2), st(0) // 00c3fcbb
        fld dword ptr [esp + 030h] // 00c3fcbd
        fld st(0) // 00c3fcc1
        fld dword ptr [esp + 028h] // 00c3fcc3
        fld st(0) // 00c3fcc7
        fmulp st(2), st(0) // 00c3fcc9
        fxch st(4) // 00c3fccb
        fsubrp st(1), st(0) // 00c3fccd
        fstp dword ptr [esp + 014h] // 00c3fccf
        fld dword ptr [esp + 02ch] // 00c3fcd3
        fld st(0) // 00c3fcd7
        fmulp st(4), st(0) // 00c3fcd9
        fld dword ptr [esp + 020h] // 00c3fcdb
        fld st(0) // 00c3fcdf
        fmulp st(6), st(0) // 00c3fce1
        fxch st(4) // 00c3fce3
        fsubrp st(5), st(0) // 00c3fce5
        fxch st(4) // 00c3fce7
        fstp dword ptr [esp + 018h] // 00c3fce9
        fmulp st(2), st(0) // 00c3fced
        fmulp st(2), st(0) // 00c3fcef
        fsubrp st(1), st(0) // 00c3fcf1
        fstp dword ptr [esp + 01ch] // 00c3fcf3
        fld dword ptr [esp + 018h] // 00c3fcf7
        fld dword ptr [esp + 014h] // 00c3fcfb
        fld dword ptr [esp + 01ch] // 00c3fcff
        fld st(1) // 00c3fd03
        fmulp st(2), st(0) // 00c3fd05
        fld st(2) // 00c3fd07
        fmulp st(3), st(0) // 00c3fd09
        fxch st(1) // 00c3fd0b
        faddp st(2), st(0) // 00c3fd0d
        fmul st(0), st(0) // 00c3fd0f
        faddp st(1), st(0) // 00c3fd11
        fstp dword ptr [esp + 038h] // 00c3fd13
        fld st(0) // 00c3fd17
        fsub dword ptr [esi + 014h] // 00c3fd19
        fstp dword ptr [esp + 014h] // 00c3fd1c
        fld dword ptr [edi + 4] // 00c3fd20
        fstp dword ptr [esp + 010h] // 00c3fd23
        fld dword ptr [esp + 010h] // 00c3fd27
        fld st(0) // 00c3fd2b
        fsub dword ptr [esi + 018h] // 00c3fd2d
        fstp dword ptr [esp + 018h] // 00c3fd30
        fld dword ptr [edi + 8] // 00c3fd34
        fstp dword ptr [esp + 010h] // 00c3fd37
        fld dword ptr [esp + 010h] // 00c3fd3b
        fld st(0) // 00c3fd3f
        fsub dword ptr [esi + 01ch] // 00c3fd41
        fstp dword ptr [esp + 01ch] // 00c3fd44
        fld dword ptr [esi + 0a4h] // 00c3fd48
        fsub dword ptr [esi + 074h] // 00c3fd4e
        fstp dword ptr [esp + 020h] // 00c3fd51
        fld dword ptr [esi + 0a8h] // 00c3fd55
        fsub dword ptr [esi + 078h] // 00c3fd5b
        fstp dword ptr [esp + 024h] // 00c3fd5e
        fld dword ptr [esi + 0ach] // 00c3fd62
        fsub dword ptr [esi + 07ch] // 00c3fd68
        fstp dword ptr [esp + 028h] // 00c3fd6b
        fld dword ptr [esp + 028h] // 00c3fd6f
        fld st(0) // 00c3fd73
        fld dword ptr [esp + 018h] // 00c3fd75
        fld st(0) // 00c3fd79
        fmulp st(2), st(0) // 00c3fd7b
        fld dword ptr [esp + 024h] // 00c3fd7d
        fmul dword ptr [esp + 01ch] // 00c3fd81
        fsubp st(2), st(0) // 00c3fd85
        fxch st(1) // 00c3fd87
        fstp dword ptr [esp + 02ch] // 00c3fd89
        fld dword ptr [esp + 01ch] // 00c3fd8d
        fld dword ptr [esp + 020h] // 00c3fd91
        fld st(0) // 00c3fd95
        fmulp st(2), st(0) // 00c3fd97
        fld dword ptr [esp + 014h] // 00c3fd99
        fmulp st(4), st(0) // 00c3fd9d
        fxch st(1) // 00c3fd9f
        fsubrp st(3), st(0) // 00c3fda1
        fxch st(2) // 00c3fda3
        fstp dword ptr [esp + 030h] // 00c3fda5
        fld dword ptr [esp + 024h] // 00c3fda9
        fmul dword ptr [esp + 014h] // 00c3fdad
        fxch st(1) // 00c3fdb1
        fmulp st(2), st(0) // 00c3fdb3
        fsubrp st(1), st(0) // 00c3fdb5
        fstp dword ptr [esp + 034h] // 00c3fdb7
        fld dword ptr [esp + 030h] // 00c3fdbb
        fld dword ptr [esp + 02ch] // 00c3fdbf
        fld dword ptr [esp + 034h] // 00c3fdc3
        fld st(1) // 00c3fdc7
        fmulp st(2), st(0) // 00c3fdc9
        fld st(2) // 00c3fdcb
        fmulp st(3), st(0) // 00c3fdcd
        fxch st(1) // 00c3fdcf
        faddp st(2), st(0) // 00c3fdd1
        fmul st(0), st(0) // 00c3fdd3
        faddp st(1), st(0) // 00c3fdd5
        fstp dword ptr [esp + 03ch] // 00c3fdd7
        fld st(2) // 00c3fddb
        fsub dword ptr [esi + 014h] // 00c3fddd
        fstp dword ptr [esp + 014h] // 00c3fde0
        fld st(1) // 00c3fde4
        fsub dword ptr [esi + 018h] // 00c3fde6
        fstp dword ptr [esp + 018h] // 00c3fde9
        fld st(0) // 00c3fded
        fsub dword ptr [esi + 01ch] // 00c3fdef
        fstp dword ptr [esp + 01ch] // 00c3fdf2
        fld dword ptr [esi + 0a4h] // 00c3fdf6
        fsub dword ptr [esi + 044h] // 00c3fdfc
        fstp dword ptr [esp + 020h] // 00c3fdff
        fld dword ptr [esi + 0a8h] // 00c3fe03
        fsub dword ptr [esi + 048h] // 00c3fe09
        fstp dword ptr [esp + 024h] // 00c3fe0c
        fld dword ptr [esi + 0ach] // 00c3fe10
        fsub dword ptr [esi + 04ch] // 00c3fe16
        fstp dword ptr [esp + 028h] // 00c3fe19
        fld dword ptr [esp + 028h] // 00c3fe1d
        fld st(0) // 00c3fe21
        fld dword ptr [esp + 018h] // 00c3fe23
        fld st(0) // 00c3fe27
        fmulp st(2), st(0) // 00c3fe29
        fld dword ptr [esp + 024h] // 00c3fe2b
        fmul dword ptr [esp + 01ch] // 00c3fe2f
        fsubp st(2), st(0) // 00c3fe33
        fxch st(1) // 00c3fe35
        fstp dword ptr [esp + 02ch] // 00c3fe37
        fld dword ptr [esp + 01ch] // 00c3fe3b
        fld dword ptr [esp + 020h] // 00c3fe3f
        fld st(0) // 00c3fe43
        fmulp st(2), st(0) // 00c3fe45
        fld dword ptr [esp + 014h] // 00c3fe47
        fmulp st(4), st(0) // 00c3fe4b
        fxch st(1) // 00c3fe4d
        fsubrp st(3), st(0) // 00c3fe4f
        fxch st(2) // 00c3fe51
        fstp dword ptr [esp + 030h] // 00c3fe53
        fld dword ptr [esp + 024h] // 00c3fe57
        fmul dword ptr [esp + 014h] // 00c3fe5b
        fxch st(1) // 00c3fe5f
        fmulp st(2), st(0) // 00c3fe61
        fsubrp st(1), st(0) // 00c3fe63
        fstp dword ptr [esp + 034h] // 00c3fe65
        fld dword ptr [esp + 030h] // 00c3fe69
        fld dword ptr [esp + 02ch] // 00c3fe6d
        fld dword ptr [esp + 034h] // 00c3fe71
        fld st(1) // 00c3fe75
        fmulp st(2), st(0) // 00c3fe77
        fld st(2) // 00c3fe79
        fmulp st(3), st(0) // 00c3fe7b
        fxch st(1) // 00c3fe7d
        faddp st(2), st(0) // 00c3fe7f
        fmul st(0), st(0) // 00c3fe81
        faddp st(1), st(0) // 00c3fe83
        fstp dword ptr [esp + 040h] // 00c3fe85
        fld dword ptr [esi + 014h] // 00c3fe89
        fsubp st(3), st(0) // 00c3fe8c
        fxch st(2) // 00c3fe8e
        fstp dword ptr [esp + 020h] // 00c3fe90
        fsub dword ptr [esi + 018h] // 00c3fe94
        fstp dword ptr [esp + 024h] // 00c3fe97
        fsub dword ptr [esi + 01ch] // 00c3fe9b
        fstp dword ptr [esp + 028h] // 00c3fe9e
        fld dword ptr [esi + 074h] // 00c3fea2
        fsub dword ptr [esi + 044h] // 00c3fea5
        fstp dword ptr [esp + 02ch] // 00c3fea8
        fld dword ptr [esi + 078h] // 00c3feac
        fsub dword ptr [esi + 048h] // 00c3feaf
        fstp dword ptr [esp + 030h] // 00c3feb2
        fld dword ptr [esi + 07ch] // 00c3feb6
        fsub dword ptr [esi + 04ch] // 00c3feb9
        fstp dword ptr [esp + 034h] // 00c3febc
        fld dword ptr [esp + 034h] // 00c3fec0
        fld st(0) // 00c3fec4
        fld dword ptr [esp + 024h] // 00c3fec6
        fld st(0) // 00c3feca
        fmulp st(2), st(0) // 00c3fecc
        fld dword ptr [esp + 030h] // 00c3fece
        fld st(0) // 00c3fed2
        fld dword ptr [esp + 028h] // 00c3fed4
        fld st(0) // 00c3fed8
        fmulp st(2), st(0) // 00c3feda
        fxch st(4) // 00c3fedc
        fsubrp st(1), st(0) // 00c3fede
        fstp dword ptr [esp + 014h] // 00c3fee0
        fld dword ptr [esp + 02ch] // 00c3fee4
        fld st(0) // 00c3fee8
        fmulp st(4), st(0) // 00c3feea
        fld dword ptr [esp + 020h] // 00c3feec
        fld st(0) // 00c3fef0
        fmulp st(6), st(0) // 00c3fef2
        fxch st(4) // 00c3fef4
        fsubrp st(5), st(0) // 00c3fef6
        fxch st(4) // 00c3fef8
        movss xmm0, dword ptr [esp + 038h] // 00c3fefa
        fstp dword ptr [esp + 018h] // 00c3ff00
        xor ecx, ecx // 00c3ff04
        movss dword ptr [esp + 8], xmm0 // 00c3ff06
        fmulp st(2), st(0) // 00c3ff0c
        fmulp st(2), st(0) // 00c3ff0e
        fsubrp st(1), st(0) // 00c3ff10
        fstp dword ptr [esp + 01ch] // 00c3ff12
        fld dword ptr [esp + 018h] // 00c3ff16
        fld dword ptr [esp + 014h] // 00c3ff1a
        fld dword ptr [esp + 01ch] // 00c3ff1e
        fld st(1) // 00c3ff22
        fmulp st(2), st(0) // 00c3ff24
        fld st(2) // 00c3ff26
        fmulp st(3), st(0) // 00c3ff28
        fxch st(1) // 00c3ff2a
        faddp st(2), st(0) // 00c3ff2c
        fmul st(0), st(0) // 00c3ff2e
        faddp st(1), st(0) // 00c3ff30
        fstp dword ptr [esp + 044h] // 00c3ff32
        fld dword ptr [esp + 038h] // 00c3ff36
        fld dword ptr [esp + 03ch] // 00c3ff3a
        fcomip st(0), st(1) // 00c3ff3e
        fstp st(0) // 00c3ff40
        jbe l_00c3ff55 // 00c3ff42
        movss xmm0, dword ptr [esp + 03ch] // 00c3ff44
        mov ecx, 1 // 00c3ff4a
        movss dword ptr [esp + 8], xmm0 // 00c3ff4f
    l_00c3ff55:
        fld dword ptr [esp + 8] // 00c3ff55
        fld dword ptr [esp + 040h] // 00c3ff59
        fcomip st(0), st(1) // 00c3ff5d
        fstp st(0) // 00c3ff5f
        jbe l_00c3ff74 // 00c3ff61
        movss xmm0, dword ptr [esp + 040h] // 00c3ff63
        mov ecx, 2 // 00c3ff69
        movss dword ptr [esp + 8], xmm0 // 00c3ff6e
    l_00c3ff74:
        fld dword ptr [esp + 8] // 00c3ff74
        fld dword ptr [esp + 044h] // 00c3ff78
        fcomip st(0), st(1) // 00c3ff7c
        fstp st(0) // 00c3ff7e
        jbe l_00c3ff87 // 00c3ff80
        mov ecx, 3 // 00c3ff82
    l_00c3ff87:
        lea edx, [ecx + ecx*2] // 00c3ff87
        mov ecx, dword ptr [edi] // 00c3ff8a
        movss xmm0, dword ptr [esp + 0ch] // 00c3ff8c
        shl edx, 4 // 00c3ff92
        lea eax, [edx + esi] // 00c3ff95
        mov dword ptr [eax + 014h], ecx // 00c3ff98
        mov edx, dword ptr [edi + 4] // 00c3ff9b
        mov dword ptr [eax + 018h], edx // 00c3ff9e
        mov ecx, dword ptr [edi + 8] // 00c3ffa1
        mov dword ptr [eax + 01ch], ecx // 00c3ffa4
        mov edx, dword ptr [edi + 0ch] // 00c3ffa7
        mov dword ptr [eax + 020h], edx // 00c3ffaa
        mov ecx, dword ptr [edi + 010h] // 00c3ffad
        mov dword ptr [eax + 024h], ecx // 00c3ffb0
        mov edx, dword ptr [edi + 014h] // 00c3ffb3
        mov dword ptr [eax + 028h], edx // 00c3ffb6
        mov ecx, dword ptr [edi + 018h] // 00c3ffb9
        mov dword ptr [eax + 8], ecx // 00c3ffbc
        mov edx, dword ptr [edi + 01ch] // 00c3ffbf
        mov dword ptr [eax + 0ch], edx // 00c3ffc2
        mov ecx, dword ptr [edi + 020h] // 00c3ffc5
        mov dword ptr [eax + 010h], ecx // 00c3ffc8
        movss dword ptr [eax + 034h], xmm0 // 00c3ffcb
    l_00c3ffd0:
        pop edi // 00c3ffd0
        pop esi // 00c3ffd1
        add esp, 040h // 00c3ffd2
        ret  // 00c3ffd5
    }
}
// Recovered instruction order, including x87 spills and reloads after calls.
// Address comments refer to the verified original PE instruction starts.
__declspec(naked) void append_kernel(){
    __asm {
        mov eax, dword ptr [esi + 07ch] // 00c35260
        cmp dword ptr [esi + 078h], eax // 00c35263
        jne l_00c352b3 // 00c35266
        lea eax, [eax + eax + 2] // 00c35268
        mov dword ptr [esi + 07ch], eax // 00c3526c
        add eax, eax // 00c3526f
        add eax, eax // 00c35271
        push edi // 00c35273
        push eax // 00c35274
        push dword ptr [esp+16] // Borrowed invocation context.
        call bridge_00c35275 // 00c35275
        mov edi, eax // 00c3527a
        xor eax, eax // 00c3527c
        add esp, 4 // 00c3527e
        cmp dword ptr [esi + 078h], eax // 00c35281
        jbe l_00c3529f // 00c35284
        mov ecx, edi // 00c35286
    l_00c35288:
        test ecx, ecx // 00c35288
        je l_00c35294 // 00c3528a
        mov edx, dword ptr [esi + 074h] // 00c3528c
        mov edx, dword ptr [edx + eax*4] // 00c3528f
        mov dword ptr [ecx], edx // 00c35292
    l_00c35294:
        add eax, 1 // 00c35294
        add ecx, 4 // 00c35297
        cmp eax, dword ptr [esi + 078h] // 00c3529a
        jb l_00c35288 // 00c3529d
    l_00c3529f:
        mov eax, dword ptr [esi + 074h] // 00c3529f
        test eax, eax // 00c352a2
        je l_00c352af // 00c352a4
        push eax // 00c352a6
        push dword ptr [esp+16] // Borrowed invocation context.
        call bridge_00c352a7 // 00c352a7
        add esp, 4 // 00c352ac
    l_00c352af:
        mov dword ptr [esi + 074h], edi // 00c352af
        pop edi // 00c352b2
    l_00c352b3:
        mov eax, dword ptr [esi + 078h] // 00c352b3
        mov ecx, dword ptr [esi + 074h] // 00c352b6
        lea eax, [ecx + eax*4] // 00c352b9
        test eax, eax // 00c352bc
        je l_00c352c6 // 00c352be
        mov edx, dword ptr [esp + 4] // 00c352c0
        mov dword ptr [eax], edx // 00c352c4
    l_00c352c6:
        add dword ptr [esi + 078h], 1 // 00c352c6
        ret 8 // 00c352ca
    }
}
// Recovered instruction order, including x87 spills and reloads after calls.
// Address comments refer to the verified original PE instruction starts.
__declspec(naked) void manifold_kernel(){
    __asm {
        push ecx // 00c3f4d0
        push ebx // 00c3f4d1
        push esi // 00c3f4d2
        mov esi, dword ptr [esp + 010h] // 00c3f4d3
        lea eax, [edi + 01d8h] // 00c3f4d7
        push eax // 00c3f4dd
        mov dword ptr [esp + 0ch], eax // 00c3f4de
        push dword ptr [esp+28] // Borrowed invocation context.
        call bridge_00c3f4e2 // 00c3f4e2
        mov ecx, dword ptr [esi + 078h] // 00c3f4e8
        xor eax, eax // 00c3f4eb
        test ecx, ecx // 00c3f4ed
        jle l_00c3f529 // 00c3f4ef
        mov ebx, dword ptr [esi + 074h] // 00c3f4f1
        mov edx, ebx // 00c3f4f4
        jmp l_00c3f500 // 00c3f4f6
        lea esp, [esp] // 00c3f4f8
        nop  // 00c3f4ff
    l_00c3f500:
        mov esi, dword ptr [edx] // 00c3f500
        mov esi, dword ptr [esi + 0d0h] // 00c3f502
        cmp esi, dword ptr [esp + 014h] // 00c3f508
        je l_00c3f51a // 00c3f50c
        add eax, 1 // 00c3f50e
        add edx, 4 // 00c3f511
        cmp eax, ecx // 00c3f514
        jl l_00c3f500 // 00c3f516
        jmp l_00c3f525 // 00c3f518
    l_00c3f51a:
        mov ebx, dword ptr [ebx + eax*4] // 00c3f51a
        test ebx, ebx // 00c3f51d
        jne l_00c3f633 // 00c3f51f
    l_00c3f525:
        mov esi, dword ptr [esp + 010h] // 00c3f525
    l_00c3f529:
        cmp dword ptr [edi + 0ch], 0 // 00c3f529
        jne l_00c3f5ca // 00c3f52d
        push ebp // 00c3f533
        push 036b00h // 00c3f534
        push dword ptr [esp+32] // Borrowed invocation context.
        call bridge_00c3f539 // 00c3f539
        mov ebp, eax // 00c3f53e
        add esp, 4 // 00c3f540
        lea eax, [ebp + 0dch] // 00c3f543
        mov ecx, 03e7h // 00c3f549
        mov edi, edi // 00c3f54e
    l_00c3f550:
        lea edx, [eax + 4] // 00c3f550
        mov dword ptr [eax], edx // 00c3f553
        add eax, 0e0h // 00c3f555
        sub ecx, 1 // 00c3f55a
        jne l_00c3f550 // 00c3f55d
        mov dword ptr [ebp + 036afch], ecx // 00c3f55f
        mov eax, dword ptr [edi + 8] // 00c3f565
        cmp dword ptr [edi + 4], eax // 00c3f568
        mov dword ptr [edi + 0ch], ebp // 00c3f56b
        jne l_00c3f5b7 // 00c3f56e
        lea eax, [eax + eax + 2] // 00c3f570
        mov dword ptr [edi + 8], eax // 00c3f574
        add eax, eax // 00c3f577
        add eax, eax // 00c3f579
        push eax // 00c3f57b
        push dword ptr [esp+32] // Borrowed invocation context.
        call bridge_00c3f57c // 00c3f57c
        mov ebx, eax // 00c3f581
        xor eax, eax // 00c3f583
        add esp, 4 // 00c3f585
        cmp dword ptr [edi + 4], eax // 00c3f588
        jbe l_00c3f5a6 // 00c3f58b
        mov ecx, ebx // 00c3f58d
        nop  // 00c3f58f
    l_00c3f590:
        test ecx, ecx // 00c3f590
        je l_00c3f59b // 00c3f592
        mov edx, dword ptr [edi] // 00c3f594
        mov edx, dword ptr [edx + eax*4] // 00c3f596
        mov dword ptr [ecx], edx // 00c3f599
    l_00c3f59b:
        add eax, 1 // 00c3f59b
        add ecx, 4 // 00c3f59e
        cmp eax, dword ptr [edi + 4] // 00c3f5a1
        jb l_00c3f590 // 00c3f5a4
    l_00c3f5a6:
        mov eax, dword ptr [edi] // 00c3f5a6
        test eax, eax // 00c3f5a8
        je l_00c3f5b5 // 00c3f5aa
        push eax // 00c3f5ac
        push dword ptr [esp+32] // Borrowed invocation context.
        call bridge_00c3f5ad // 00c3f5ad
        add esp, 4 // 00c3f5b2
    l_00c3f5b5:
        mov dword ptr [edi], ebx // 00c3f5b5
    l_00c3f5b7:
        mov eax, dword ptr [edi + 4] // 00c3f5b7
        mov ecx, dword ptr [edi] // 00c3f5ba
        lea eax, [ecx + eax*4] // 00c3f5bc
        test eax, eax // 00c3f5bf
        je l_00c3f5c5 // 00c3f5c1
        mov dword ptr [eax], ebp // 00c3f5c3
    l_00c3f5c5:
        add dword ptr [edi + 4], 1 // 00c3f5c5
        pop ebp // 00c3f5c9
    l_00c3f5ca:
        mov ebx, dword ptr [edi + 0ch] // 00c3f5ca
        mov edx, dword ptr [ebx + 0dch] // 00c3f5cd
        add dword ptr [edi + 01d0h], 1 // 00c3f5d3
        mov dword ptr [edi + 0ch], edx // 00c3f5da
        lea eax, [edi + 0f0h] // 00c3f5dd
        mov dword ptr [ebx + 0dch], eax // 00c3f5e3
        mov ecx, dword ptr [edi + 01c8h] // 00c3f5e9
        mov eax, dword ptr [esp + 014h] // 00c3f5ef
        mov dword ptr [ebx + 0d8h], ecx // 00c3f5f3
        mov edx, dword ptr [edi + 01c8h] // 00c3f5f9
        mov dword ptr [edx + 0dch], ebx // 00c3f5ff
        mov dword ptr [edi + 01c8h], ebx // 00c3f605
        push ebx // 00c3f60b
        mov dword ptr [ebx + 0c8h], 0 // 00c3f60c
        mov dword ptr [ebx + 0cch], esi // 00c3f616
        mov dword ptr [ebx + 0d0h], eax // 00c3f61c
        push dword ptr [esp+28] // Borrowed invocation context.
        call append_shim // 00c3f622
        mov esi, dword ptr [ebx + 0d0h] // 00c3f627
        push ebx // 00c3f62d
        push dword ptr [esp+28] // Borrowed invocation context.
        call append_shim // 00c3f62e
    l_00c3f633:
        mov ecx, dword ptr [esp + 8] // 00c3f633
        push ecx // 00c3f637
        push dword ptr [esp+28] // Borrowed invocation context.
        call bridge_00c3f638 // 00c3f638
        pop esi // 00c3f63e
        mov eax, ebx // 00c3f63f
        pop ebx // 00c3f641
        pop ecx // 00c3f642
        ret 12 // 00c3f643
    }
}
// Recovered instruction order, including x87 spills and reloads after calls.
// Address comments refer to the verified original PE instruction starts.
__declspec(naked) void range_kernel(){
    __asm {
        sub esp, 0148h // 00c44090
        cmp eax, dword ptr [esp + 014ch] // 00c44096
        mov edx, eax // 00c4409d
        mov dword ptr [esp + 8], eax // 00c4409f
        jg l_00c44355 // 00c440a3
        push ebx // 00c440a9
        push ebp // 00c440aa
        push edi // 00c440ab
        lea esp, [esp] // 00c440ac
    l_00c440b0:
        mov eax, dword ptr [esi + 0b4h] // 00c440b0
        mov eax, dword ptr [eax + edx*4] // 00c440b6
        mov ecx, dword ptr [eax] // 00c440b9
        mov ecx, dword ptr [ecx] // 00c440bb
        mov eax, dword ptr [eax + 4] // 00c440bd
        mov edi, dword ptr [ecx + 070h] // 00c440c0
        test edi, edi // 00c440c3
        mov eax, dword ptr [eax] // 00c440c5
        mov dword ptr [esp + 024h], ecx // 00c440c7
        mov dword ptr [esp + 01ch], eax // 00c440cb
        mov dword ptr [esp + 0ch], edi // 00c440cf
        je l_00c4433e // 00c440d3
        jmp l_00c440e4 // 00c440d9
        jmp l_00c440e0 // 00c440db
        lea ecx, [ecx] // 00c440dd
    l_00c440e0:
        mov edi, dword ptr [esp + 0ch] // 00c440e0
    l_00c440e4:
        mov ecx, dword ptr [esp + 01ch] // 00c440e4
        mov ebx, dword ptr [ecx + 070h] // 00c440e8
        test ebx, ebx // 00c440eb
        mov dword ptr [esp + 010h], ebx // 00c440ed
        je l_00c44328 // 00c440f1
        jmp l_00c44104 // 00c440f7
        lea esp, [esp] // 00c440f9
    l_00c44100:
        mov ebx, dword ptr [esp + 010h] // 00c44100
    l_00c44104:
        mov edx, dword ptr [ebx + 030h] // 00c44104
        mov eax, dword ptr [edi + 030h] // 00c44107
        and edx, dword ptr [edi + 02ch] // 00c4410a
        and eax, dword ptr [ebx + 02ch] // 00c4410d
        or edx, eax // 00c44110
        je l_00c44316 // 00c44112
        mov eax, dword ptr [edi + 8] // 00c44118
        mov edx, dword ptr [ebx + 8] // 00c4411b
        lea ecx, [eax + eax*2] // 00c4411e
        lea eax, [edx + ecx*2] // 00c44121
        mov ecx, dword ptr [esi + eax*4] // 00c44124
        test ecx, ecx // 00c44127
        je l_00c44316 // 00c44129
        mov eax, dword ptr [esp + 01ch] // 00c4412f
        mov edx, dword ptr [ecx] // 00c44133
        mov edx, dword ptr [edx] // 00c44135
        add eax, 8 // 00c44137
        push eax // 00c4413a
        mov eax, dword ptr [esp + 028h] // 00c4413b
        push ebx // 00c4413f
        add eax, 8 // 00c44140
        push eax // 00c44143
        push edi // 00c44144
        lea eax, [esp + 040h] // 00c44145
        push eax // 00c44149
        call edx // 00c4414a
        test al, al // 00c4414c
        je l_00c44316 // 00c4414e
        movss xmm0, dword ptr [edi + 028h] // 00c44154
        xorps xmm1, xmm1 // 00c44159
        comiss xmm1, xmm0 // 00c4415c
        movss dword ptr [esp + 02ch], xmm0 // 00c4415f
        ja l_00c44177 // 00c44165
        movss xmm0, dword ptr [ebx + 028h] // 00c44167
        comiss xmm1, xmm0 // 00c4416c
        movss dword ptr [esp + 028h], xmm0 // 00c4416f
        jbe l_00c4418b // 00c44175
    l_00c44177:
        movss xmm1, dword ptr constant_00d7a208 // 00c44177
        subss xmm1, xmm0 // 00c4417f
        movss dword ptr [esp + 018h], xmm1 // 00c44183
        jmp l_00c44197 // 00c44189
    l_00c4418b:
        fld dword ptr [esp + 028h] // 00c4418b
        fmul dword ptr [esp + 02ch] // 00c4418f
        fstp dword ptr [esp + 018h] // 00c44193
    l_00c44197:
        mov eax, dword ptr [esi + 0b4h] // 00c44197
        fld dword ptr [ebx + 024h] // 00c4419d
        mov ecx, dword ptr [esp + 014h] // 00c441a0
        fadd dword ptr [edi + 024h] // 00c441a4
        mov eax, dword ptr [eax + ecx*4] // 00c441a7
        mov edx, dword ptr [eax + 4] // 00c441aa
        mov ecx, dword ptr [edx] // 00c441ad
        fmul qword ptr constant_00d7a280 // 00c441af
        mov eax, dword ptr [eax] // 00c441b5
        mov eax, dword ptr [eax] // 00c441b7
        mov edi, dword ptr [esi + 0b0h] // 00c441b9
        fstp dword ptr [esp + 020h] // 00c441bf
        push ecx // 00c441c3
        push eax // 00c441c4
        push dword ptr [esp+356] // Borrowed invocation context.
        call manifold_shim // 00c441c5
        fld dword ptr [esp + 020h] // 00c441ca
        movss xmm0, dword ptr [esp + 018h] // 00c441ce
        mov ebp, eax // 00c441d4
        xor edi, edi // 00c441d6
        fstp dword ptr [ebp + 4] // 00c441d8
        movss dword ptr [ebp], xmm0 // 00c441db
        cmp dword ptr [esp + 030h], edi // 00c441e0
        jle l_00c44209 // 00c441e4
        lea ebx, [esp + 034h] // 00c441e6
        lea ebx, [ebx] // 00c441ea
    l_00c441f0:
        mov eax, ebx // 00c441f0
        mov ecx, ebp // 00c441f2
        call insert_kernel // 00c441f4
        add edi, 1 // 00c441f9
        add ebx, 024h // 00c441fc
        cmp edi, dword ptr [esp + 030h] // 00c441ff
        jl l_00c441f0 // 00c44203
        mov ebx, dword ptr [esp + 010h] // 00c44205
    l_00c44209:
        mov ecx, dword ptr [esp + 024h] // 00c44209
        mov eax, dword ptr [ecx + 068h] // 00c4420d
        test eax, eax // 00c44210
        je l_00c4421c // 00c44212
        mov edx, dword ptr [eax + 4] // 00c44214
        test dword ptr [ebx + 02ch], edx // 00c44217
        jne l_00c4423b // 00c4421a
    l_00c4421c:
        mov eax, dword ptr [esp + 01ch] // 00c4421c
        mov eax, dword ptr [eax + 068h] // 00c44220
        test eax, eax // 00c44223
        je l_00c44312 // 00c44225
        mov ecx, dword ptr [eax + 4] // 00c4422b
        mov edx, dword ptr [esp + 0ch] // 00c4422e
        test dword ptr [edx + 02ch], ecx // 00c44232
        je l_00c44312 // 00c44235
    l_00c4423b:
        lea ecx, [esi + 0e4h] // 00c4423b
    l_00c44241:
        mov edx, 1 // 00c44241
        mov edi, ecx // 00c44246
        xor eax, eax // 00c44248
        lock cmpxchg dword ptr [edi], edx // 00c4424a
        test eax, eax // 00c4424e
        jne l_00c44241 // 00c44250
        mov eax, dword ptr [esi + 0e0h] // 00c44252
        cmp dword ptr [esi + 0dch], eax // 00c44258
        jne l_00c442df // 00c4425e
        lea eax, [eax + eax + 2] // 00c44260
        mov dword ptr [esi + 0e0h], eax // 00c44264
        lea eax, [eax + eax*2] // 00c4426a
        add eax, eax // 00c4426d
        add eax, eax // 00c4426f
        push eax // 00c44271
        push dword ptr [esp+352] // Borrowed invocation context.
        call bridge_00c44272 // 00c44272
        xor edx, edx // 00c44277
        add esp, 4 // 00c44279
        cmp dword ptr [esi + 0dch], edx // 00c4427c
        mov dword ptr [esp + 020h], eax // 00c44282
        jbe l_00c442c2 // 00c44286
        xor edi, edi // 00c44288
        mov ecx, eax // 00c4428a
        lea esp, [esp] // 00c4428c
    l_00c44290:
        test ecx, ecx // 00c44290
        je l_00c442b1 // 00c44292
        mov eax, dword ptr [esi + 0d8h] // 00c44294
        mov ebx, dword ptr [eax + edi] // 00c4429a
        add eax, edi // 00c4429d
        mov dword ptr [ecx], ebx // 00c4429f
        mov ebx, dword ptr [eax + 4] // 00c442a1
        mov dword ptr [ecx + 4], ebx // 00c442a4
        mov eax, dword ptr [eax + 8] // 00c442a7
        mov ebx, dword ptr [esp + 010h] // 00c442aa
        mov dword ptr [ecx + 8], eax // 00c442ae
    l_00c442b1:
        add edx, 1 // 00c442b1
        add edi, 0ch // 00c442b4
        add ecx, 0ch // 00c442b7
        cmp edx, dword ptr [esi + 0dch] // 00c442ba
        jb l_00c44290 // 00c442c0
    l_00c442c2:
        mov eax, dword ptr [esi + 0d8h] // 00c442c2
        test eax, eax // 00c442c8
        je l_00c442d5 // 00c442ca
        push eax // 00c442cc
        push dword ptr [esp+352] // Borrowed invocation context.
        call bridge_00c442cd // 00c442cd
        add esp, 4 // 00c442d2
    l_00c442d5:
        mov ecx, dword ptr [esp + 020h] // 00c442d5
        mov dword ptr [esi + 0d8h], ecx // 00c442d9
    l_00c442df:
        mov eax, dword ptr [esi + 0dch] // 00c442df
        lea edx, [eax + eax*2] // 00c442e5
        mov eax, dword ptr [esi + 0d8h] // 00c442e8
        lea eax, [eax + edx*4] // 00c442ee
        test eax, eax // 00c442f1
        je l_00c44301 // 00c442f3
        mov ecx, dword ptr [esp + 0ch] // 00c442f5
        mov dword ptr [eax], ebp // 00c442f9
        mov dword ptr [eax + 4], ecx // 00c442fb
        mov dword ptr [eax + 8], ebx // 00c442fe
    l_00c44301:
        add dword ptr [esi + 0dch], 1 // 00c44301
        xor edx, edx // 00c44308
        lea eax, [esi + 0e4h] // 00c4430a
        xchg dword ptr [eax], edx // 00c44310
    l_00c44312:
        mov edi, dword ptr [esp + 0ch] // 00c44312
    l_00c44316:
        mov ebx, dword ptr [ebx + 0208h] // 00c44316
        test ebx, ebx // 00c4431c
        mov dword ptr [esp + 010h], ebx // 00c4431e
        jne l_00c44100 // 00c44322
    l_00c44328:
        mov edi, dword ptr [edi + 0208h] // 00c44328
        test edi, edi // 00c4432e
        mov dword ptr [esp + 0ch], edi // 00c44330
        jne l_00c440e0 // 00c44334
        mov edx, dword ptr [esp + 014h] // 00c4433a
    l_00c4433e:
        add edx, 1 // 00c4433e
        cmp edx, dword ptr [esp + 0158h] // 00c44341
        mov dword ptr [esp + 014h], edx // 00c44348
        jle l_00c440b0 // 00c4434c
        pop edi // 00c44352
        pop ebp // 00c44353
        pop ebx // 00c44354
    l_00c44355:
        add esp, 0148h // 00c44355
        ret 8 // 00c4435b
    }
}
} // namespace
void* NativeDynNarrowPhaseCalls::allocate_00bf55be(U,U size,const AvoidZoneDynHullMemory& m){return m.allocate(m.context,size);}
void* NativeDynNarrowPhaseCalls::malloc_00bf9f1a(U,U size,const AvoidZoneDynHullMemory& m){return m.allocate(m.context,size);}
void NativeDynNarrowPhaseCalls::free_00bf6989(U,void* p,const AvoidZoneDynHullMemory& m){m.release(m.context,p);}
void NativeDynNarrowPhaseCalls::enter_critical_section(U,void* p){EnterCriticalSection(static_cast<CRITICAL_SECTION*>(p));}
void NativeDynNarrowPhaseCalls::leave_critical_section(U,void* p){LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(p));}
void append_native_dyn_manifold_00c35260(void* body,void* manifold,const AvoidZoneDynHullMemory& memory,NativeDynNarrowPhaseCalls& calls){
    Context context{memory,calls};auto* c=&context;
    __asm {
        push esi
        mov esi,body
        push c
        push manifold
        call append_kernel
        pop esi
    }
}
void* find_or_create_native_dyn_manifold_00c3f4d0(void* pool,void* first_body,void* second_body,const AvoidZoneDynHullMemory& memory,NativeDynNarrowPhaseCalls& calls){
    Context context{memory,calls};auto* c=&context;void* result;
    __asm {
        push edi
        mov edi,pool
        push c
        push second_body
        push first_body
        call manifold_kernel
        pop edi
        mov result,eax
    }
    return result;
}
std::int32_t match_native_dyn_contact_00c3f650(void* manifold,const float* candidate) noexcept {
    std::int32_t result;
    __asm {
        push edi
        mov eax,manifold
        mov edi,candidate
        call match_kernel
        pop edi
        mov result,eax
    }
    return result;
}
void insert_native_dyn_contact_00c3f760(void* manifold,const float* candidate) noexcept {
    __asm {
        mov ecx,manifold
        mov eax,candidate
        call insert_kernel
    }
}
void intersect_native_dyn_pairs_00c44090(void* scene,std::int32_t first,std::int32_t last,const AvoidZoneDynHullMemory& memory,NativeDynNarrowPhaseCalls& calls){
    Context context{memory,calls};auto* c=&context;
    __asm {
        push esi
        mov esi,scene
        mov eax,first
        push c
        push last
        call range_kernel
        pop esi
    }
}
static_assert(std::is_standard_layout_v<NativeDynIntersectTaskRuntime>);
NativeDynIntersectTaskRuntime::NativeDynIntersectTaskRuntime(const AvoidZoneDynHullMemory& memory,NativeDynNarrowPhaseCalls& calls) noexcept
    :methods_{reinterpret_cast<std::uintptr_t>(&run)},memory_(memory),calls_(&calls) {}
void __fastcall NativeDynIntersectTaskRuntime::run(void* task,void*){
    auto* words=static_cast<volatile U*>(task);
    const auto last=static_cast<std::int32_t>(words[4]);
    auto* scene=reinterpret_cast<void*>(words[2]);
    const auto first=static_cast<std::int32_t>(words[3]);
    const auto* owner=reinterpret_cast<const NativeDynIntersectTaskRuntime*>(words[0]);
    intersect_native_dyn_pairs_00c44090(scene,first,last,owner->memory_,*owner->calls_);
}
} // namespace bsp
