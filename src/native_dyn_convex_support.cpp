#include "bsp/native_dyn_convex_support.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native convex support requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
namespace {
// Exact bit patterns from D7A228/D7A230/D7A268,verified live against PE.
alignas(8) const std::uint64_t seed_positive=0x3fffd70a40000000ULL;
alignas(8) const std::uint64_t seed_negative=0xbfffd70a40000000ULL;
alignas(8) const std::uint64_t support_epsilon=0x3f1a36e2e0000000ULL;

// Original floating-point instruction schedule,including operand direction.
// Integer adapters supply the borrowed CRT address; no native private-stack ABI.
__declspec(naked) void __fastcall local_support_kernel(const volatile std::uint32_t*,const AvoidZoneDynHullData*,OceanVec3*,const OceanVec3*) noexcept {
    __asm {
        mov eax,edx
        sub esp,10h // Native0Ch locals plus one context DWORD.
        mov dword ptr [esp+0ch],ecx
        push ebx // 00c358a3
        push ebp // 00c358a4
        push esi // 00c358a5
        push edi // 00c358a6
        mov edi, eax // 00c358a7
        mov eax, dword ptr [esp + 028h] // 00c358a9
        fld dword ptr [eax] // 00c358ad
        fstp dword ptr [esp + 014h] // 00c358af
        fld dword ptr [eax + 4] // 00c358b3
        fstp dword ptr [esp + 010h] // 00c358b6
        fld dword ptr [eax + 8] // 00c358ba
        fstp dword ptr [esp + 018h] // 00c358bd
        fld dword ptr [esp + 014h] // 00c358c1
        fld st(0) // 00c358c5
        fld qword ptr seed_negative // 00c358c7
        fmul st(1), st(0) // 00c358cd
        fxch st(1) // 00c358cf
        mov ecx,dword ptr [esp+1ch]
        call native_crt_truncate_st0_00bf7420 // 00c358d1
        fld dword ptr [esp + 010h] // 00c358d6
        fld st(0) // 00c358da
        lea esi, [eax + eax*2] // 00c358dc
        fmulp st(2), st(0) // 00c358df
        fxch st(1) // 00c358e1
        mov ecx,dword ptr [esp+1ch]
        call native_crt_truncate_st0_00bf7420 // 00c358e3
        fld dword ptr [esp + 018h] // 00c358e8
        fld qword ptr seed_positive // 00c358ec
        add esi, eax // 00c358f2
        lea eax, [esi + esi*2] // 00c358f4
        fmul st(0), st(1) // 00c358f7
        mov esi, eax // 00c358f9
        mov ecx,dword ptr [esp+1ch]
        call native_crt_truncate_st0_00bf7420 // 00c358fb
        mov edx, dword ptr [edi] // 00c35900
        sub eax, esi // 00c35902
        movsx ebx, word ptr [edi + eax*2 + 04ah] // 00c35904
        mov edi, dword ptr [edi + 0ch] // 00c35909
        mov ecx, ebx // 00c3590c
        shl ecx, 4 // 00c3590e
        lea eax, [ecx + edx] // 00c35911
        movzx ecx, word ptr [eax + 0ch] // 00c35914
        fld dword ptr [eax + 4] // 00c35918
        movzx ebp, word ptr [edi + ecx*2] // 00c3591b
        fmulp st(2), st(0) // 00c3591f
        fld st(2) // 00c35921
        add ecx, 1 // 00c35923
        fmul dword ptr [eax] // 00c35926
        xor esi, esi // 00c35928
        test ebp, ebp // 00c3592a
        mov dword ptr [esp + 014h], edx // 00c3592c
        faddp st(2), st(0) // 00c35930
        fmul dword ptr [eax + 8] // 00c35932
        faddp st(1), st(0) // 00c35935
        fstp dword ptr [esp + 010h] // 00c35937
        jle l_00c359d0 // 00c3593b
        mov eax, dword ptr [esp + 028h] // 00c35941
        fld qword ptr support_epsilon // 00c35945
        movss xmm0, dword ptr [eax + 4] // 00c3594b
        movss dword ptr [esp + 028h], xmm0 // 00c35950
        movss xmm0, dword ptr [eax + 8] // 00c35956
        fld dword ptr [esp + 028h] // 00c3595b
        movss dword ptr [esp + 018h], xmm0 // 00c3595f
        fld dword ptr [esp + 018h] // 00c35965
    l_00c35969:
        lea eax, [esi + ecx] // 00c35969
        movzx eax, word ptr [edi + eax*2] // 00c3596c
        mov dword ptr [esp + 018h], eax // 00c35970
        shl eax, 4 // 00c35974
        add edx, eax // 00c35977
        fld dword ptr [edx + 4] // 00c35979
        fmul st(0), st(2) // 00c3597c
        fld dword ptr [edx] // 00c3597e
        fmul st(0), st(5) // 00c35980
        faddp st(1), st(0) // 00c35982
        fld dword ptr [edx + 8] // 00c35984
        fmul st(0), st(2) // 00c35987
        faddp st(1), st(0) // 00c35989
        fstp dword ptr [esp + 028h] // 00c3598b
        fld dword ptr [esp + 028h] // 00c3598f
        fld dword ptr [esp + 010h] // 00c35993
        fadd st(0), st(4) // 00c35997
        fxch st(1) // 00c35999
        fcomip st(0), st(1) // 00c3599b
        fstp st(0) // 00c3599d
        jbe l_00c359bf // 00c3599f
        movzx ecx, word ptr [edx + 0ch] // 00c359a1
        movzx ebp, word ptr [edi + ecx*2] // 00c359a5
        movss xmm0, dword ptr [esp + 028h] // 00c359a9
        mov ebx, dword ptr [esp + 018h] // 00c359af
        add ecx, 1 // 00c359b3
        movss dword ptr [esp + 010h], xmm0 // 00c359b6
        or esi, 0ffffffffh // 00c359bc
    l_00c359bf:
        mov edx, dword ptr [esp + 014h] // 00c359bf
        add esi, 1 // 00c359c3
        cmp esi, ebp // 00c359c6
        jl l_00c35969 // 00c359c8
        fstp st(2) // 00c359ca
        fstp st(0) // 00c359cc
        fstp st(1) // 00c359ce
    l_00c359d0:
        mov eax, dword ptr [esp + 024h] // 00c359d0
        fstp st(0) // 00c359d4
        shl ebx, 4 // 00c359d6
        mov ecx, dword ptr [ebx + edx] // 00c359d9
        add ebx, edx // 00c359dc
        mov dword ptr [eax], ecx // 00c359de
        mov edx, dword ptr [ebx + 4] // 00c359e0
        pop edi // 00c359e3
        pop esi // 00c359e4
        mov dword ptr [eax + 4], edx // 00c359e5
        mov ecx, dword ptr [ebx + 8] // 00c359e8
        pop ebp // 00c359eb
        mov dword ptr [eax + 8], ecx // 00c359ec
        pop ebx // 00c359ef
        add esp, 010h // 00c359f0
        ret 8 // 00c359f3
    }
}

// Original floating-point instruction schedule,including operand direction.
// Integer adapters supply the borrowed CRT address; no native private-stack ABI.
__declspec(naked) void __fastcall support32_kernel(const DynConvexShapeStorage*,const volatile std::uint32_t*,OceanVec3*,const OceanVec3*,const float*) noexcept {
    __asm {
        movd xmm1,edx // Keep explicit conversion context until the mesh call.
        sub esp, 010h // 00c386e0
        mov eax, dword ptr [esp + 018h] // 00c386e3
        fld dword ptr [eax + 4] // 00c386e7
        push esi // 00c386ea
        mov esi, dword ptr [esp + 020h] // 00c386eb
        fstp dword ptr [esp + 01ch] // 00c386ef
        fld dword ptr [eax] // 00c386f3
        push edi // 00c386f5
        fstp dword ptr [esp + 024h] // 00c386f6
        mov edi, dword ptr [esp + 01ch] // 00c386fa
        fld dword ptr [eax + 8] // 00c386fe
        lea eax, [esp + 0ch] // 00c38701
        fstp dword ptr [esp + 8] // 00c38705
        push eax // 00c38709
        fld dword ptr [esp + 028h] // 00c3870a
        mov eax, dword ptr [ecx + 0210h] // 00c3870e
        fld st(0) // 00c38714
        push edi // 00c38716
        fmul dword ptr [esi] // 00c38717
        fld dword ptr [esi + 4] // 00c38719
        fld dword ptr [esp + 028h] // 00c3871c
        fld st(0) // 00c38720
        fmulp st(2), st(0) // 00c38722
        fxch st(2) // 00c38724
        faddp st(1), st(0) // 00c38726
        fld dword ptr [esi + 8] // 00c38728
        fld dword ptr [esp + 010h] // 00c3872b
        fld st(0) // 00c3872f
        fmulp st(2), st(0) // 00c38731
        fxch st(2) // 00c38733
        faddp st(1), st(0) // 00c38735
        fstp dword ptr [esp + 014h] // 00c38737
        fld dword ptr [esi + 010h] // 00c3873b
        fmul st(0), st(2) // 00c3873e
        fld dword ptr [esi + 0ch] // 00c38740
        fmul st(0), st(4) // 00c38743
        faddp st(1), st(0) // 00c38745
        fld dword ptr [esi + 014h] // 00c38747
        fmul st(0), st(2) // 00c3874a
        faddp st(1), st(0) // 00c3874c
        fstp dword ptr [esp + 018h] // 00c3874e
        fld dword ptr [esi + 01ch] // 00c38752
        fmulp st(2), st(0) // 00c38755
        fld dword ptr [esi + 018h] // 00c38757
        fmulp st(3), st(0) // 00c3875a
        fxch st(1) // 00c3875c
        faddp st(2), st(0) // 00c3875e
        fmul dword ptr [esi + 020h] // 00c38760
        faddp st(1), st(0) // 00c38763
        fstp dword ptr [esp + 01ch] // 00c38765
        movd ecx,xmm1
        mov edx,eax
        call local_support_kernel // 00c38769
        fld dword ptr [edi + 4] // 00c3876e
        fstp dword ptr [esp + 020h] // 00c38771
        fld dword ptr [edi] // 00c38775
        fstp dword ptr [esp + 024h] // 00c38777
        fld dword ptr [edi + 8] // 00c3877b
        fstp dword ptr [esp + 01ch] // 00c3877e
        fld dword ptr [esp + 024h] // 00c38782
        fld st(0) // 00c38786
        fmul dword ptr [esi] // 00c38788
        fld dword ptr [esp + 020h] // 00c3878a
        fld st(0) // 00c3878e
        fmul dword ptr [esi + 0ch] // 00c38790
        faddp st(2), st(0) // 00c38793
        fld dword ptr [esp + 01ch] // 00c38795
        fld st(0) // 00c38799
        fmul dword ptr [esi + 018h] // 00c3879b
        faddp st(3), st(0) // 00c3879e
        fld dword ptr [esi + 024h] // 00c387a0
        faddp st(3), st(0) // 00c387a3
        fxch st(2) // 00c387a5
        fstp dword ptr [esp + 0ch] // 00c387a7
        fld dword ptr [esi + 010h] // 00c387ab
        fmul st(0), st(1) // 00c387ae
        fld dword ptr [esi + 4] // 00c387b0
        fmul st(0), st(4) // 00c387b3
        faddp st(1), st(0) // 00c387b5
        fld dword ptr [esi + 01ch] // 00c387b7
        fmul st(0), st(3) // 00c387ba
        faddp st(1), st(0) // 00c387bc
        fadd dword ptr [esi + 028h] // 00c387be
        fstp dword ptr [esp + 010h] // 00c387c1
        fmul dword ptr [esi + 014h] // 00c387c5
        mov ecx, dword ptr [esp + 0ch] // 00c387c8
        fld dword ptr [esi + 8] // 00c387cc
        mov edx, dword ptr [esp + 010h] // 00c387cf
        fmulp st(3), st(0) // 00c387d3
        faddp st(2), st(0) // 00c387d5
        fmul dword ptr [esi + 020h] // 00c387d7
        faddp st(1), st(0) // 00c387da
        fadd dword ptr [esi + 02ch] // 00c387dc
        mov dword ptr [edi], ecx // 00c387df
        mov dword ptr [edi + 4], edx // 00c387e1
        fstp dword ptr [esp + 014h] // 00c387e4
        mov eax, dword ptr [esp + 014h] // 00c387e8
        mov dword ptr [edi + 8], eax // 00c387ec
        pop edi // 00c387ef
        pop esi // 00c387f0
        add esp, 010h // 00c387f1
        ret 0ch // 00c387f4
    }
}

// Original floating-point instruction schedule,including operand direction.
// Integer adapters supply the borrowed CRT address; no native private-stack ABI.
__declspec(naked) void __fastcall support64_kernel(const DynConvexShapeStorage*,const volatile std::uint32_t*,double*,const double*,const float*) noexcept {
    __asm {
        movd xmm1,edx // Keep explicit conversion context until the mesh call.
        sub esp, 01ch // 00c385b0
        mov eax, dword ptr [esp + 024h] // 00c385b3
        fld qword ptr [eax] // 00c385b7
        push esi // 00c385b9
        fstp dword ptr [esp + 8] // 00c385ba
        mov esi, dword ptr [esp + 02ch] // 00c385be
        fld qword ptr [eax + 8] // 00c385c2
        lea edx, [esp + 014h] // 00c385c5
        fstp dword ptr [esp + 0ch] // 00c385c9
        fld qword ptr [eax + 010h] // 00c385cd
        lea eax, [esp + 8] // 00c385d0
        fstp dword ptr [esp + 010h] // 00c385d4
        push eax // 00c385d8
        fld dword ptr [esi + 4] // 00c385d9
        mov eax, dword ptr [ecx + 0210h] // 00c385dc
        fstp dword ptr [esp + 030h] // 00c385e2
        push edx // 00c385e6
        fld dword ptr [esi] // 00c385e7
        fstp dword ptr [esp + 030h] // 00c385e9
        fld dword ptr [esi + 8] // 00c385ed
        fstp dword ptr [esp + 0ch] // 00c385f0
        fld dword ptr [esp + 030h] // 00c385f4
        fld dword ptr [esp + 010h] // 00c385f8
        fld st(0) // 00c385fc
        fmulp st(2), st(0) // 00c385fe
        fld dword ptr [esp + 034h] // 00c38600
        fld dword ptr [esp + 014h] // 00c38604
        fld st(0) // 00c38608
        fmulp st(2), st(0) // 00c3860a
        fxch st(3) // 00c3860c
        faddp st(1), st(0) // 00c3860e
        fld dword ptr [esp + 0ch] // 00c38610
        fld dword ptr [esp + 018h] // 00c38614
        fld st(0) // 00c38618
        fmulp st(2), st(0) // 00c3861a
        fxch st(2) // 00c3861c
        faddp st(1), st(0) // 00c3861e
        fstp dword ptr [esp + 010h] // 00c38620
        fld dword ptr [esi + 010h] // 00c38624
        fmul st(0), st(3) // 00c38627
        fld dword ptr [esi + 0ch] // 00c38629
        fmul st(0), st(3) // 00c3862c
        faddp st(1), st(0) // 00c3862e
        fld dword ptr [esi + 014h] // 00c38630
        fmul st(0), st(2) // 00c38633
        faddp st(1), st(0) // 00c38635
        fstp dword ptr [esp + 014h] // 00c38637
        fld dword ptr [esi + 01ch] // 00c3863b
        fmulp st(3), st(0) // 00c3863e
        fld dword ptr [esi + 018h] // 00c38640
        fmulp st(2), st(0) // 00c38643
        fxch st(2) // 00c38645
        faddp st(1), st(0) // 00c38647
        fld dword ptr [esi + 020h] // 00c38649
        fmulp st(2), st(0) // 00c3864c
        faddp st(1), st(0) // 00c3864e
        fstp dword ptr [esp + 018h] // 00c38650
        movd ecx,xmm1
        mov edx,eax
        call local_support_kernel // 00c38654
        fld dword ptr [esp + 028h] // 00c38659
        fld dword ptr [esp + 014h] // 00c3865d
        fld st(0) // 00c38661
        fmulp st(2), st(0) // 00c38663
        fld dword ptr [esp + 018h] // 00c38665
        fld st(0) // 00c38669
        fmul dword ptr [esi + 0ch] // 00c3866b
        faddp st(3), st(0) // 00c3866e
        fld dword ptr [esi + 018h] // 00c38670
        fld dword ptr [esp + 01ch] // 00c38673
        fld st(0) // 00c38677
        fmulp st(2), st(0) // 00c38679
        fxch st(4) // 00c3867b
        faddp st(1), st(0) // 00c3867d
        fadd dword ptr [esi + 024h] // 00c3867f
        fstp dword ptr [esp + 8] // 00c38682
        fld dword ptr [esi + 010h] // 00c38686
        fmul st(0), st(1) // 00c38689
        fld dword ptr [esp + 02ch] // 00c3868b
        fmul st(0), st(3) // 00c3868f
        faddp st(1), st(0) // 00c38691
        fld dword ptr [esi + 01ch] // 00c38693
        fmul st(0), st(4) // 00c38696
        faddp st(1), st(0) // 00c38698
        fadd dword ptr [esi + 028h] // 00c3869a
        mov eax, dword ptr [esp + 024h] // 00c3869d
        fstp dword ptr [esp + 0ch] // 00c386a1
        fmul dword ptr [esi + 014h] // 00c386a5
        fld dword ptr [esp + 4] // 00c386a8
        fmulp st(2), st(0) // 00c386ac
        faddp st(1), st(0) // 00c386ae
        fld dword ptr [esi + 020h] // 00c386b0
        fmulp st(2), st(0) // 00c386b3
        faddp st(1), st(0) // 00c386b5
        fadd dword ptr [esi + 02ch] // 00c386b7
        pop esi // 00c386ba
        fstp dword ptr [esp + 0ch] // 00c386bb
        fld dword ptr [esp + 4] // 00c386bf
        fld dword ptr [esp + 8] // 00c386c3
        fld dword ptr [esp + 0ch] // 00c386c7
        fxch st(2) // 00c386cb
        fstp qword ptr [eax] // 00c386cd
        fstp qword ptr [eax + 8] // 00c386cf
        fstp qword ptr [eax + 010h] // 00c386d2
        add esp, 01ch // 00c386d5
        ret 0ch // 00c386d8
    }
}

} // namespace
void support_native_dyn_hull_00c358a0(const AvoidZoneDynHullData& mesh,OceanVec3& output,const OceanVec3& direction,const volatile std::uint32_t& mode) noexcept {local_support_kernel(&mode,&mesh,&output,&direction);}
void support_native_dyn_convex_shape_00c386e0(const DynConvexShapeStorage& shape,OceanVec3& output,const OceanVec3& direction,const float* matrix,const volatile std::uint32_t& mode) noexcept {support32_kernel(&shape,&mode,&output,&direction,matrix);}
void support_native_dyn_convex_shape_00c385b0(const DynConvexShapeStorage& shape,double* output,const double* direction,const float* matrix,const volatile std::uint32_t& mode) noexcept {support64_kernel(&shape,&mode,output,direction,matrix);}
static_assert(std::is_standard_layout_v<NativeDynConvexShapeRuntime>);
NativeDynConvexShapeRuntime::NativeDynConvexShapeRuntime(DynConvexShapePoolStorage& pool,const volatile std::uint32_t& mode) noexcept
    :methods_{reinterpret_cast<std::uintptr_t>(&bounds),reinterpret_cast<std::uintptr_t>(&scalar),reinterpret_cast<std::uintptr_t>(&support32),reinterpret_cast<std::uintptr_t>(&support64)},pool_(&pool),conversion_(&mode) {}
const NativeDynConvexShapeRuntime& NativeDynConvexShapeRuntime::owner(const void* shape) noexcept {
    // Standard-layout first member is the table; recover its stable owner.
    return **reinterpret_cast<const NativeDynConvexShapeRuntime* const*>(shape);
}
void __fastcall NativeDynConvexShapeRuntime::bounds(void* p,void*) {dyn_convex_shape_refresh_bounds_00c57c40(*static_cast<DynConvexShapeStorage*>(p));}
void* __fastcall NativeDynConvexShapeRuntime::scalar(void* p,void*,std::uint32_t flags) {auto* pool=owner(p).pool_;return delete_native_dyn_convex_shape_004062c0(*static_cast<DynConvexShapeStorage*>(p),flags,*pool);}
void __fastcall NativeDynConvexShapeRuntime::support32(void* p,void*,OceanVec3* output,const OceanVec3* direction,const float* matrix) noexcept {support32_kernel(static_cast<const DynConvexShapeStorage*>(p),owner(p).conversion_,output,direction,matrix);}
void __fastcall NativeDynConvexShapeRuntime::support64(void* p,void*,double* output,const double* direction,const float* matrix) noexcept {support64_kernel(static_cast<const DynConvexShapeStorage*>(p),owner(p).conversion_,output,direction,matrix);}
} // namespace bsp
