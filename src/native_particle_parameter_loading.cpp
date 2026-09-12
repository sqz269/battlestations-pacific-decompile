#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_pooled_text.hpp"
#include "bsp/native_weak_owner.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle parameter loading requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Bindings = NativeParticleParameterLoadingBindings;
using Builder = NativeParticleParameterBuilderStorage;
struct Key {
    float x, y, incoming[2], outgoing[2];
    std::uint32_t kind;
    std::byte coefficients[16];
};
static_assert(sizeof(Key)==0x2c && sizeof(Builder)==0x10 && sizeof(void*)==4);
static_assert(sizeof(Bindings)==68 && offsetof(Bindings,backward_limit_00ce3928)==12);
static_assert(offsetof(Bindings,tangent_scale_00cf1450)==36 && offsetof(Bindings,integral_quarter_00d7a348)==48);
template<class T> T read(const void* p,std::size_t n) noexcept {
    T v; std::memcpy(&v,static_cast<const std::byte*>(p)+n,sizeof v); return v;
}
template<class T> void write(void* p,std::size_t n,T v) noexcept {
    std::memcpy(static_cast<std::byte*>(p)+n,&v,sizeof v);
}
Key* key(Builder& p,std::int32_t i) noexcept {
    return reinterpret_cast<Key*>(static_cast<std::byte*>(p.records_00)+static_cast<std::uint32_t>(i)*0x2cu);
}
void copy_words(void* destination,const void* source,std::size_t count) noexcept {
    // REP MOVSD's forward copy, including overlap and opaque scratch bytes.
    for(std::size_t i=0;i<count;++i) write(destination,i*4,read<std::uint32_t>(source,i*4));
}
std::int32_t __fastcall insert_bridge(void*,Bindings*,const void*);

// Native 00AFB670..00AFB6A7; symbolic current-data loads only.
__declspec(naked) void* __fastcall key_construct(void*) {
    __asm {
        mov edx, dword ptr [esp - 4] // 00afb670
        xorps xmm0, xmm0 // 00afb674
        mov eax, ecx // 00afb677
        mov ecx, dword ptr [esp - 8] // 00afb679
        mov dword ptr [eax + 8], ecx // 00afb67d
        mov ecx, dword ptr [esp - 8] // 00afb680
        mov dword ptr [eax + 0ch], edx // 00afb684
        mov edx, dword ptr [esp - 4] // 00afb687
        sub esp, 8 // 00afb68b
        movss dword ptr [eax], xmm0 // 00afb68e
        movss dword ptr [eax + 4], xmm0 // 00afb692
        mov dword ptr [eax + 018h], 1 // 00afb697
        mov dword ptr [eax + 010h], ecx // 00afb69e
        mov dword ptr [eax + 014h], edx // 00afb6a1
        add esp, 8 // 00afb6a4
        ret // 00afb6a7
    }
}

// Native 00AFB3A0..00AFB543; symbolic current-data loads only.
__declspec(naked) void __fastcall coefficients(void*,const NativeParticleParameterLoadingBindings*,const void*) {
    __asm {
        sub esp, 034h // 00afb3a0
        cmp dword ptr [ecx + 018h], 2 // 00afb3a3
        movss xmm0, dword ptr [ecx] // 00afb3a7
        movss xmm1, dword ptr [ecx + 4] // 00afb3ab
        movss dword ptr [esp + 014h], xmm0 // 00afb3b0
        fld dword ptr [esp + 014h] // 00afb3b6
        xorps xmm0, xmm0 // 00afb3ba
        movss dword ptr [esp + 024h], xmm1 // 00afb3bd
        fld dword ptr [esp + 024h] // 00afb3c3
        push eax
        mov eax,[edx+36]
        fld qword ptr [eax] // 00afb3c7
        pop eax
        je L_00afb3e7 // 00afb3cd
        fld dword ptr [ecx + 010h] // 00afb3cf
        fmul st(0), st(1) // 00afb3d2
        fadd st(0), st(3) // 00afb3d4
        fstp dword ptr [esp + 018h] // 00afb3d6
        fld dword ptr [ecx + 014h] // 00afb3da
        fmul st(0), st(1) // 00afb3dd
        fadd st(0), st(2) // 00afb3df
        fstp dword ptr [esp + 028h] // 00afb3e1
        jmp L_00afb3f3 // 00afb3e5
    L_00afb3e7:
        movss dword ptr [esp + 018h], xmm0 // 00afb3e7
        movss dword ptr [esp + 028h], xmm0 // 00afb3ed
    L_00afb3f3:
        mov eax, dword ptr [esp + 038h] // 00afb3f3
        cmp dword ptr [eax + 018h], 0 // 00afb3f7
        movss xmm2, dword ptr [eax] // 00afb3fb
        movss dword ptr [esp + 020h], xmm2 // 00afb3ff
        movss xmm2, dword ptr [eax + 4] // 00afb405
        fld dword ptr [esp + 020h] // 00afb40a
        movss dword ptr [esp + 030h], xmm2 // 00afb40e
        fld dword ptr [esp + 030h] // 00afb414
        je L_00afb438 // 00afb418
        fld st(1) // 00afb41a
        fld dword ptr [eax + 8] // 00afb41c
        fmul st(0), st(4) // 00afb41f
        fsubp st(1), st(0) // 00afb421
        fstp dword ptr [esp + 01ch] // 00afb423
        fld st(0) // 00afb427
        fld dword ptr [eax + 0ch] // 00afb429
        fmulp st(4), st(0) // 00afb42c
        fsubrp st(3), st(0) // 00afb42e
        fxch st(2) // 00afb430
        fstp dword ptr [esp + 02ch] // 00afb432
        jmp L_00afb446 // 00afb436
    L_00afb438:
        fstp st(2) // 00afb438
        movss dword ptr [esp + 01ch], xmm0 // 00afb43a
        movss dword ptr [esp + 02ch], xmm0 // 00afb440
    L_00afb446:
        fld st(0) // 00afb446
        push eax
        mov eax,[edx+40]
        movss xmm0, dword ptr [eax] // 00afb448
        pop eax
        fsub st(0), st(4) // 00afb450
        movss dword ptr [esp + 4], xmm0 // 00afb452
        movss dword ptr [esp], xmm0 // 00afb458
        fstp dword ptr [esp + 8] // 00afb45d
        fld st(1) // 00afb461
        fsub st(0), st(3) // 00afb463
        fstp dword ptr [esp + 0ch] // 00afb465
        fld dword ptr [esp + 018h] // 00afb469
        fsubrp st(4), st(0) // 00afb46d
        fxch st(3) // 00afb46f
        fstp dword ptr [esp + 038h] // 00afb471
        fld dword ptr [esp + 038h] // 00afb475
        fld st(0) // 00afb479
        fldz // 00afb47b
        fld st(0) // 00afb47d
        fxch st(2) // 00afb47f
        fucomip st(0), st(2) // 00afb481
        fstp st(1) // 00afb483
        lahf // 00afb485
        test ah, 044h // 00afb486
        jnp L_00afb49a // 00afb489
        fld dword ptr [esp + 028h] // 00afb48b
        fsubrp st(4), st(0) // 00afb48f
        fxch st(3) // 00afb491
        fdivrp st(1), st(0) // 00afb493
        fstp dword ptr [esp] // 00afb495
        jmp L_00afb49e // 00afb498
    L_00afb49a:
        fstp st(1) // 00afb49a
        fstp st(2) // 00afb49c
    L_00afb49e:
        fld dword ptr [esp + 01ch] // 00afb49e
        fsubp st(3), st(0) // 00afb4a2
        fxch st(2) // 00afb4a4
        fstp dword ptr [esp + 038h] // 00afb4a6
        fld dword ptr [esp + 038h] // 00afb4aa
        fld st(0) // 00afb4ae
        fucomip st(0), st(2) // 00afb4b0
        fstp st(1) // 00afb4b2
        lahf // 00afb4b4
        test ah, 044h // 00afb4b5
        jnp L_00afb4c8 // 00afb4b8
        fld dword ptr [esp + 02ch] // 00afb4ba
        fsubp st(2), st(0) // 00afb4be
        fdivp st(1), st(0) // 00afb4c0
        fstp dword ptr [esp + 4] // 00afb4c2
        jmp L_00afb4cc // 00afb4c6
    L_00afb4c8:
        fstp st(0) // 00afb4c8
        fstp st(0) // 00afb4ca
    L_00afb4cc:
        fld dword ptr [esp + 8] // 00afb4cc
        movss xmm0, dword ptr [esp] // 00afb4d0
        fld st(0) // 00afb4d5
        movss dword ptr [ecx + 024h], xmm0 // 00afb4d7
        fmul st(0), st(0) // 00afb4dc
        movss dword ptr [ecx + 028h], xmm1 // 00afb4de
        fld1 // 00afb4e3
        fdivrp st(1), st(0) // 00afb4e5
        fstp dword ptr [esp + 010h] // 00afb4e7
        fld dword ptr [esp] // 00afb4eb
        fmul st(0), st(1) // 00afb4ee
        fstp dword ptr [esp + 8] // 00afb4f0
        fld dword ptr [esp + 4] // 00afb4f4
        fmul st(0), st(1) // 00afb4f8
        fstp dword ptr [esp + 038h] // 00afb4fa
        fld dword ptr [esp + 038h] // 00afb4fe
        fld st(0) // 00afb502
        fld dword ptr [esp + 8] // 00afb504
        fld st(0) // 00afb508
        faddp st(2), st(0) // 00afb50a
        fld dword ptr [esp + 0ch] // 00afb50c
        fld st(0) // 00afb510
        fsubp st(3), st(0) // 00afb512
        fld st(0) // 00afb514
        fsubp st(3), st(0) // 00afb516
        fld dword ptr [esp + 010h] // 00afb518
        fld st(0) // 00afb51c
        fmulp st(4), st(0) // 00afb51e
        fxch st(3) // 00afb520
        fdivrp st(5), st(0) // 00afb522
        fxch st(4) // 00afb524
        fstp dword ptr [ecx + 01ch] // 00afb526
        fld st(3) // 00afb529
        fadd st(0), st(0) // 00afb52b
        faddp st(4), st(0) // 00afb52d
        fld st(0) // 00afb52f
        fsubp st(4), st(0) // 00afb531
        fsubp st(3), st(0) // 00afb533
        fxch st(2) // 00afb535
        fsubrp st(1), st(0) // 00afb537
        fmulp st(1), st(0) // 00afb539
        fstp dword ptr [ecx + 020h] // 00afb53b
        add esp, 034h // 00afb53e
        ret 4 // 00afb541
    }
}

// Native 00AFBC90..00AFBD02; symbolic current-data loads only.
__declspec(naked) void __fastcall incoming(float*,const NativeParticleParameterLoadingBindings*,float,float) {
    __asm {
        push ebx
        mov ebx,edx
        push ecx // 00afbc90
        push eax
        mov eax,[ebx+12]
        fld qword ptr [eax] // 00afbc91
        pop eax
        push esi // 00afbc97
        fld dword ptr [esp + 010h] // 00afbc98
        mov esi, ecx // 00afbc9c
        fcomip st(0), st(1) // 00afbc9e
        fstp st(0) // 00afbca0
        jbe L_00afbcae // 00afbca2
        push eax
        mov eax,[ebx+16]
        movss xmm0, dword ptr [eax] // 00afbca4
        pop eax
        jmp L_00afbcb4 // 00afbcac
    L_00afbcae:
        movss xmm0, dword ptr [esp + 010h] // 00afbcae
    L_00afbcb4:
        fld dword ptr [esp + 014h] // 00afbcb4
        movss dword ptr [esi], xmm0 // 00afbcb8
        fld dword ptr [esi] // 00afbcbc
        fstp dword ptr [esp + 010h] // 00afbcbe
        fld dword ptr [esp + 010h] // 00afbcc2
        fld st(1) // 00afbcc6
        fmulp st(2), st(0) // 00afbcc8
        fmul st(0), st(0) // 00afbcca
        faddp st(1), st(0) // 00afbccc
        fstp dword ptr [esp + 04h] // 00afbcce
        fld dword ptr [esp + 04h] // 00afbcd2
        mov ecx,[ebx+4]
        call native_crt_sqrt_st0_00bf7030 // 00afbcd6
        fstp dword ptr [esp + 04h] // 00afbcdb
        fld dword ptr [esp + 04h] // 00afbcdf
        fstp dword ptr [esp + 04h] // 00afbce3
        fld dword ptr [esp + 010h] // 00afbce7
        fld dword ptr [esp + 04h] // 00afbceb
        fld st(0) // 00afbcef
        fdivp st(2), st(0) // 00afbcf1
        fxch st(1) // 00afbcf3
        fstp dword ptr [esi] // 00afbcf5
        fdivr dword ptr [esp + 014h] // 00afbcf7
        fstp dword ptr [esi + 4] // 00afbcfb
        pop esi // 00afbcfe
        pop ecx // 00afbcff
        pop ebx
        ret 8 // 00afbd00
    }
}

// Native 00AFBD10..00AFBD82; symbolic current-data loads only.
__declspec(naked) void __fastcall outgoing(float*,const NativeParticleParameterLoadingBindings*,float,float) {
    __asm {
        push ebx
        mov ebx,edx
        push ecx // 00afbd10
        fld dword ptr [esp + 0ch] // 00afbd11
        push esi // 00afbd15
        push eax
        mov eax,[ebx+20]
        fld qword ptr [eax] // 00afbd16
        pop eax
        mov esi, ecx // 00afbd1c
        fcomip st(0), st(1) // 00afbd1e
        fstp st(0) // 00afbd20
        jbe L_00afbd2e // 00afbd22
        push eax
        mov eax,[ebx+24]
        movss xmm0, dword ptr [eax] // 00afbd24
        pop eax
        jmp L_00afbd34 // 00afbd2c
    L_00afbd2e:
        movss xmm0, dword ptr [esp + 010h] // 00afbd2e
    L_00afbd34:
        fld dword ptr [esp + 014h] // 00afbd34
        movss dword ptr [esi], xmm0 // 00afbd38
        fld dword ptr [esi] // 00afbd3c
        fstp dword ptr [esp + 010h] // 00afbd3e
        fld dword ptr [esp + 010h] // 00afbd42
        fld st(1) // 00afbd46
        fmulp st(2), st(0) // 00afbd48
        fmul st(0), st(0) // 00afbd4a
        faddp st(1), st(0) // 00afbd4c
        fstp dword ptr [esp + 04h] // 00afbd4e
        fld dword ptr [esp + 04h] // 00afbd52
        mov ecx,[ebx+4]
        call native_crt_sqrt_st0_00bf7030 // 00afbd56
        fstp dword ptr [esp + 04h] // 00afbd5b
        fld dword ptr [esp + 04h] // 00afbd5f
        fstp dword ptr [esp + 04h] // 00afbd63
        fld dword ptr [esp + 010h] // 00afbd67
        fld dword ptr [esp + 04h] // 00afbd6b
        fld st(0) // 00afbd6f
        fdivp st(2), st(0) // 00afbd71
        fxch st(1) // 00afbd73
        fstp dword ptr [esi] // 00afbd75
        fdivr dword ptr [esp + 014h] // 00afbd77
        fstp dword ptr [esi + 4] // 00afbd7b
        pop esi // 00afbd7e
        pop ecx // 00afbd7f
        pop ebx
        ret 8 // 00afbd80
    }
}

// Native 00AFFCB0..00AFFD19; symbolic current-data loads only.
__declspec(naked) float __fastcall integral_hermite(const void*,const NativeParticleParameterLoadingBindings*,float) {
    __asm {
        fld dword ptr [esp + 4] // 00affcb0
        mov ecx, dword ptr [ecx + 4] // 00affcb4
        fld st(0) // 00affcb7
        fld dword ptr [ecx + 4] // 00affcb9
        fxch st(1) // 00affcbc
        fcomi st(0), st(1) // 00affcbe
        fstp st(1) // 00affcc0
        jbe L_00affcd2 // 00affcc2
    L_00affcc4:
        fld dword ptr [ecx + 020h] // 00affcc4
        add ecx, 01ch // 00affcc7
        fxch st(1) // 00affcca
        fcomi st(0), st(1) // 00affccc
        fstp st(1) // 00affcce
        ja L_00affcc4 // 00affcd0
    L_00affcd2:
        fstp st(0) // 00affcd2
        fsub dword ptr [ecx] // 00affcd4
        fstp dword ptr [esp + 4] // 00affcd6
        fld dword ptr [ecx + 0ch] // 00affcda
        fld dword ptr [esp + 4] // 00affcdd
        fld st(0) // 00affce1
        fmulp st(2), st(0) // 00affce3
        fxch st(1) // 00affce5
        push eax
        mov eax,[edx+48]
        fmul qword ptr [eax] // 00affce7
        pop eax
        fld dword ptr [ecx + 010h] // 00affced
        push eax
        mov eax,[edx+36]
        fmul qword ptr [eax] // 00affcf0
        pop eax
        faddp st(1), st(0) // 00affcf6
        fmul st(0), st(1) // 00affcf8
        fld dword ptr [ecx + 014h] // 00affcfa
        push eax
        mov eax,[edx+44]
        fmul qword ptr [eax] // 00affcfd
        pop eax
        faddp st(1), st(0) // 00affd03
        fmul st(0), st(1) // 00affd05
        fadd dword ptr [ecx + 018h] // 00affd07
        fmulp st(1), st(0) // 00affd0a
        fadd dword ptr [ecx + 8] // 00affd0c
        fstp dword ptr [esp + 4] // 00affd0f
        fld dword ptr [esp + 4] // 00affd13
        ret 4 // 00affd17
    }
}

// Native 00AFFD20..00AFFD6F; symbolic current-data loads only.
__declspec(naked) float __fastcall integral_linear(const void*,const NativeParticleParameterLoadingBindings*,float) {
    __asm {
        fld dword ptr [esp + 4] // 00affd20
        mov ecx, dword ptr [ecx + 4] // 00affd24
        fld st(0) // 00affd27
        fld dword ptr [ecx + 4] // 00affd29
        fxch st(1) // 00affd2c
        fcomi st(0), st(1) // 00affd2e
        fstp st(1) // 00affd30
        jbe L_00affd42 // 00affd32
    L_00affd34:
        fld dword ptr [ecx + 018h] // 00affd34
        add ecx, 014h // 00affd37
        fxch st(1) // 00affd3a
        fcomi st(0), st(1) // 00affd3c
        fstp st(1) // 00affd3e
        ja L_00affd34 // 00affd40
    L_00affd42:
        fstp st(0) // 00affd42
        fsub dword ptr [ecx] // 00affd44
        fstp dword ptr [esp + 4] // 00affd46
        fld dword ptr [ecx + 0ch] // 00affd4a
        fld dword ptr [esp + 4] // 00affd4d
        fld st(0) // 00affd51
        fmulp st(2), st(0) // 00affd53
        fxch st(1) // 00affd55
        push eax
        mov eax,[edx+44]
        fmul qword ptr [eax] // 00affd57
        pop eax
        fadd dword ptr [ecx + 8] // 00affd5d
        fmulp st(1), st(0) // 00affd60
        fadd dword ptr [ecx + 010h] // 00affd62
        fstp dword ptr [esp + 4] // 00affd65
        fld dword ptr [esp + 4] // 00affd69
        ret 4 // 00affd6d
    }
}

// Native 00AFC360..00AFC46B; symbolic current-data loads only.
__declspec(naked) void __fastcall endpoints(void*,NativeParticleParameterLoadingBindings*,float,float) {
    __asm {
        push ebx
        mov ebx,edx
        sub esp, 034h // 00afc360
        xorps xmm0, xmm0 // 00afc363
        fld dword ptr [esp + 040h] // 00afc366
        fsub dword ptr [esp + 03ch] // 00afc36a
        mov eax, dword ptr [esp + 00h] // 00afc36e
        movss xmm1, dword ptr [esp + 03ch] // 00afc371
        movss dword ptr [esp + 00h], xmm0 // 00afc377
        mov edx, dword ptr [esp + 00h] // 00afc37c
        fstp dword ptr [esp + 00h] // 00afc37f
        fld dword ptr [esp + 00h] // 00afc382
        push esi // 00afc385
        mov esi, ecx // 00afc386
        mov ecx, dword ptr [esp + 08h] // 00afc388
        sub esp, 8 // 00afc38c
        fstp dword ptr [esp + 04h] // 00afc38f
        mov dword ptr [esp + 01ch], eax // 00afc393
        push eax
        mov eax,[ebx+28]
        fld dword ptr [eax] // 00afc397
        pop eax
        movss dword ptr [esp + 010h], xmm0 // 00afc39d
        mov eax, dword ptr [esp + 010h] // 00afc3a3
        fstp dword ptr [esp + 00h] // 00afc3a7
        mov dword ptr [esp + 020h], ecx // 00afc3aa
        lea ecx, [esp + 024h] // 00afc3ae
        movss dword ptr [esp + 014h], xmm0 // 00afc3b2
        movss dword ptr [esp + 018h], xmm1 // 00afc3b8
        mov dword ptr [esp + 02ch], 0 // 00afc3be
        mov dword ptr [esp + 024h], edx // 00afc3c6
        mov dword ptr [esp + 028h], eax // 00afc3ca
        mov edx,ebx
        call outgoing // 00afc3ce
        lea ecx, [esp + 0ch] // 00afc3d3
        push ecx // 00afc3d7
        mov ecx, esi // 00afc3d8
        mov edx,ebx
        call insert_bridge // 00afc3da
        fld dword ptr [esp + 040h] // 00afc3df
        fsub dword ptr [esp + 044h] // 00afc3e3
        push eax
        mov eax,[ebx+28]
        movss xmm0, dword ptr [eax] // 00afc3e7
        pop eax
        mov edx, dword ptr [esp + 04h] // 00afc3ef
        mov eax, dword ptr [esp + 08h] // 00afc3f3
        movss dword ptr [esp + 0ch], xmm0 // 00afc3f7
        fstp dword ptr [esp + 040h] // 00afc3fd
        movss xmm0, dword ptr [esp + 044h] // 00afc401
        fld dword ptr [esp + 040h] // 00afc407
        movss dword ptr [esp + 010h], xmm0 // 00afc40b
        xorps xmm0, xmm0 // 00afc411
        sub esp, 8 // 00afc414
        movss dword ptr [esp + 0ch], xmm0 // 00afc417
        fstp dword ptr [esp + 04h] // 00afc41d
        mov ecx, dword ptr [esp + 0ch] // 00afc421
        push eax
        mov eax,[ebx+32]
        fld dword ptr [eax] // 00afc425
        pop eax
        mov dword ptr [esp + 024h], edx // 00afc42b
        fstp dword ptr [esp + 00h] // 00afc42f
        movss dword ptr [esp + 010h], xmm0 // 00afc432
        mov edx, dword ptr [esp + 010h] // 00afc438
        mov dword ptr [esp + 01ch], ecx // 00afc43c
        lea ecx, [esp + 01ch] // 00afc440
        mov dword ptr [esp + 02ch], 2 // 00afc444
        mov dword ptr [esp + 028h], eax // 00afc44c
        mov dword ptr [esp + 020h], edx // 00afc450
        mov edx,ebx
        call incoming // 00afc454
        lea eax, [esp + 0ch] // 00afc459
        push eax // 00afc45d
        mov ecx, esi // 00afc45e
        mov edx,ebx
        call insert_bridge // 00afc460
        pop esi // 00afc465
        add esp, 034h // 00afc466
        pop ebx
        ret 8 // 00afc469
    }
}

void reserve_00afbdb0(Builder& p,std::int32_t requested,Bindings& a) {
    if(requested<4) requested=4;
    if(requested<=p.capacity_08) return;
    const auto product=static_cast<std::uint64_t>(static_cast<std::uint32_t>(requested))*0x2cu;
    const auto bytes=product>0xffffffffu?0xffffffffu:static_cast<std::uint32_t>(product);
    void* replacement=a.allocate_array_00bf55be(bytes);
    if(replacement) for(std::int32_t i=0;i<requested;++i)
        key_construct(static_cast<std::byte*>(replacement)+static_cast<std::uint32_t>(i)*0x2cu);
    for(std::int32_t i=0;i<p.count_04;++i)
        copy_words(static_cast<std::byte*>(replacement)+static_cast<std::uint32_t>(i)*0x2cu,key(p,i),11);
    a.owners.free_array_00bf6989(p.records_00);
    p.records_00=replacement;
    p.capacity_08=requested;
}
bool strictly_after(float x,float y) noexcept {
    unsigned char result;
    __asm {
        fld y
        fld x
        fcomip st(0),st(1)
        fstp st(0)
        seta result
    }
    return result!=0;
}
std::int32_t insert_00afc260(Builder& p,const Key* source,Bindings& a) {
    std::int32_t position=0;
    while(position<p.count_04 && strictly_after(source->x,key(p,position)->x)) ++position;
    if(p.count_04==p.capacity_08)
        reserve_00afbdb0(p,static_cast<std::int32_t>(static_cast<std::uint32_t>(p.capacity_08)*2u),a);
    copy_words(key(p,p.count_04),source,11);
    ++p.count_04;
    if(position<p.count_04-1) {
        for(auto i=p.count_04-1;i>position;--i) copy_words(key(p,i),key(p,i-1),11);
        copy_words(key(p,position),source,11);
    }
    return position;
}
std::int32_t __fastcall insert_bridge(void* p,Bindings* a,const void* source) {
    return insert_00afc260(*static_cast<Builder*>(p),static_cast<const Key*>(source),*a);
}
float linear_slope(const Key* first,const Key* second) noexcept {
    float result;
    __asm {
        mov eax,second
        mov ecx,first
        fld dword ptr[eax+4]
        fsub dword ptr[ecx+4]
        fld dword ptr[eax]
        fsub dword ptr[ecx]
        fdivp st(1),st(0)
        fstp result
    }
    return result;
}
void allocate_segments_00affd70(void* p,std::uint8_t count,Bindings& a) {
    write(p,9,count);
    const auto kind=read<std::uint16_t>(p,10);
    if(kind!=1 && kind!=2) return;
    if(void* old=read<void*>(p,4)) {
        a.owners.free_array_00bf6989(old);
        write<void*>(p,4,nullptr);
    }
    const auto bytes=read<std::uint8_t>(p,9)*(kind==1?0x14u:0x1cu);
    write(p,4,a.allocate_array_00bf55be(bytes));
}
void append_segment(void* p,const void* segment,bool hermite,Bindings& a) {
    const auto stride=hermite?0x1cu:0x14u;
    const auto count=read<std::uint8_t>(p,8);
    float accumulated=0.0f;
    if(count) {
        auto* previous=static_cast<std::byte*>(read<void*>(p,4))+(static_cast<std::uint32_t>(count)-1)*stride;
        const float end=read<float>(previous,4);
        accumulated=hermite?integral_hermite(p,&a,end):integral_linear(p,&a,end);
    }
    auto* destination=static_cast<std::byte*>(read<void*>(p,4))+static_cast<std::uint32_t>(read<std::uint8_t>(p,8))*stride;
    copy_words(destination,segment,stride/4);
    destination=static_cast<std::byte*>(read<void*>(p,4))+static_cast<std::uint32_t>(read<std::uint8_t>(p,8))*stride;
    write(destination,hermite?8:16,accumulated);
    write(p,8,static_cast<std::uint8_t>(read<std::uint8_t>(p,8)+1u));
}
struct Token {
    NativePooledTextStorage storage;
    NativeStringStorage& strings;
    Token(const void* line,std::int32_t index,NativeStringStorage& domain):strings(domain) {
        get_native_pooled_text_token_00aee3c0(line,&storage,index,strings);
    }
    ~Token() { destroy_native_pooled_text_00aee2a0(&storage,strings); }
    Token(const Token&)=delete;
    Token& operator=(const Token&)=delete;
};
float token_number(const void* line,std::int32_t index,NativeStringStorage& strings) {
    Token token(line,index,strings);
    return static_cast<float>(std::atof(token.storage.data));
}
void token_pair(const void* line,std::int32_t index,float* destination,NativeStringStorage& strings) {
    // Native evaluates the second token first, obtains the first token, then
    // converts second/first and destroys first/second. Do not reorder pooling.
    Token y(line,index+1,strings);
    Token x(line,index,strings);
    const float second=static_cast<float>(std::atof(y.storage.data));
    const float first=static_cast<float>(std::atof(x.storage.data));
    destination[0]=first;
    destination[1]=second;
}
} // namespace

void* construct_native_particle_parameter_builder_00afbed0(void* raw,Bindings& a) {
    auto& p=*static_cast<Builder*>(raw);
    p.records_00=nullptr; p.count_04=0; p.capacity_08=0;
    try { reserve_00afbdb0(p,32,a); }
    catch(...) { destroy_native_particle_parameter_builder_00af4110(raw,a); throw; }
    return raw;
}
void destroy_native_particle_parameter_builder_00af4110(void* raw,Bindings& a) noexcept {
    auto& p=*static_cast<Builder*>(raw);
    a.owners.free_array_00bf6989(p.records_00);
    p.records_00=nullptr; p.count_04=0; p.capacity_08=0;
}
void initialize_native_particle_parameter_endpoints_00afc360(void* raw,float first,float last,Bindings& a) {
    endpoints(raw,&a,first,last);
}
float first_native_particle_parameter_value_00afc1b0(const void* raw) {
    return read<float>(read<void*>(raw,0),4);
}
bool parse_native_particle_parameter_00afc470(void* raw,const void* line,Bindings& a) {
    auto& p=*static_cast<Builder*>(raw);
    Token type(line,0,a.owners.strings);
    const bool hermite=_stricmp(type.storage.data,"Hermite")==0;
    const bool linear=!hermite && _stricmp(type.storage.data,"Linear")==0;
    if(hermite || linear) {
        std::int32_t count;
        { Token token(line,1,a.owners.strings); count=std::atol(token.storage.data); }
        if(count<2) return false;
        p.kind_0c=hermite?2:1;
        destroy_native_particle_parameter_builder_00af4110(raw,a);
        for(std::int32_t i=0;i<count;++i) {
            const auto index=static_cast<std::int32_t>(static_cast<std::uint32_t>(i)*(hermite?6u:2u)+2u);
            Key record; // Only coefficients remain native indeterminate scratch.
            record.x=token_number(line,index,a.owners.strings);
            record.y=token_number(line,index+1,a.owners.strings);
            if(i==0) { if(record.x!=*a.first_time_00d7a218) return false; record.kind=0; }
            else if(i==count-1) { if(static_cast<double>(record.x)!=*a.last_time_00d7a220) return false; record.kind=2; }
            else record.kind=1;
            if(hermite) {
                token_pair(line,index+2,record.incoming,a.owners.strings);
                token_pair(line,index+4,record.outgoing,a.owners.strings);
            } else {
                record.incoming[0]=*a.linear_incoming_00d7a260; record.incoming[1]=0.0f;
                record.outgoing[0]=*a.linear_outgoing_00d7a24c; record.outgoing[1]=0.0f;
            }
            insert_00afc260(p,&record,a);
        }
        return true;
    }
    if(_stricmp(type.storage.data,"Const")!=0) return false;
    p.kind_0c=0;
    Token value(line,1,a.owners.strings);
    const float number=static_cast<float>(std::atof(value.storage.data));
    write(p.records_00,4,number);
    return true;
}
void* convert_native_particle_parameter_00afbf60(void* raw,Bindings& a) {
    auto& p=*static_cast<Builder*>(raw);
    void* result=a.owners.parameter_pool_00f8d344.allocate_raw_slot_009242f0(); // B004A0
    if(result) { // AFF9B0: scale+0 and physical slab index+C are untouched.
        write(result,4,0u); write<std::uint8_t>(result,8,0); write<std::uint8_t>(result,9,0); write<std::uint16_t>(result,10,0);
    }
    if(p.kind_0c==2) {
        write<std::uint16_t>(result,10,2);
        for(std::int32_t i=0;i<p.count_04-1;++i) coefficients(key(p,i),&a,key(p,i+1)); // AFBF20
        allocate_segments_00affd70(result,static_cast<std::uint8_t>(p.count_04-1),a);
        for(std::int32_t i=0;i<p.count_04-1;++i) {
            const Key* first=key(p,i);
            std::uint32_t segment[7];
            segment[0]=read<std::uint32_t>(first,0); segment[1]=read<std::uint32_t>(first,0x2c); segment[2]=0;
            copy_words(segment+3,first->coefficients,4);
            append_segment(result,segment,true,a); // B000A0
        }
        return result;
    }
    if(p.kind_0c==1) {
        write<std::uint16_t>(result,10,1);
        allocate_segments_00affd70(result,static_cast<std::uint8_t>(p.count_04-1),a);
        for(std::int32_t i=0;i<p.count_04-1;++i) {
            const Key* first=key(p,i);
            const Key* second=key(p,i+1);
            float segment[5]{first->x,second->x,first->y,linear_slope(first,second),0.0f};
            append_segment(result,segment,false,a); // B00120
        }
        return result;
    }
    if(p.kind_0c==0) {
        write<std::uint16_t>(result,10,0); write(result,4,first_native_particle_parameter_value_00afc1b0(raw));
        return result;
    }
    return nullptr; // Original unknown kind abandons its newly allocated slot.
}
} // namespace bsp
