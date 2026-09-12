#include "bsp/native_particle_emission_spawn.hpp"
#include "bsp/native_particle_record_children.hpp"
#include "bsp/native_particle_record_update.hpp"
#include "bsp/native_particle_model_update.hpp"
#include "bsp/native_particle_emitter_lifetime.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/camera_look_at.hpp"
#include <cstddef>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle record update requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(sizeof(NativeParticleModelArraysStorage) == 0x18);
static_assert(offsetof(NativeParticleRecordUpdateAccess, crt) == 0);
static_assert(offsetof(NativeParticleRecordUpdateAccess, percent_limit_00ce3d08) == 4);
static_assert(offsetof(NativeParticleRecordUpdateAccess, percent_00d7a220) == 8);
static_assert(offsetof(NativeParticleRecordUpdateAccess, transverse_00d7a258) == 12);
static_assert(offsetof(NativeParticleRecordUpdateAccess, direction_00e13028) == 16);
static_assert(offsetof(NativeParticleRecordUpdateAccess, length_threshold_00d7a268) == 20);
static_assert(offsetof(NativeParticleRecordUpdateAccess, one_00d7a24c) == 24);
static_assert(offsetof(NativeParticleRecordUpdateAccess, emission) == 28);
static_assert(offsetof(NativeParticleRecordUpdateAccess, children) == 32);
void* __fastcall acquire_bridge(void* emitter) {
    return acquire_native_particle_emitter_container_00aff690(emitter);
}
CameraMatrix* __stdcall look_at_dispatch(CameraMatrix* destination,
    const CameraAxis* eye, const CameraAxis* target, const CameraAxis* up,
    const NativeParticleRecordUpdateAccess* access) {
    return &build_camera_look_at_00b63f10(*destination, *eye, *target, *up,
        *access->crt, *access->one_00d7a24c);
}
// EAX adds access; original ECX/EDX and target/up argument words stay intact.
// Borrow the original up triple without another floating-point conversion.
__declspec(naked) void look_at_bridge() {
    __asm {
        push ebp
        mov ebp,esp
        push eax
        lea eax,[ebp+0ch]
        push eax
        push dword ptr [ebp+08h]
        push edx
        push ecx
        call look_at_dispatch
        pop ebp
        ret 010h
    }
}
} // namespace
// Instructions retain original ordering. The update kernels add four stack
// bytes for access. Only incoming-argument offsets shift; record locals and
// every original float32 spill remain intact. Added binding loads are MOVs.

__declspec(naked) void* __fastcall copy_native_particle_record_00afcf50(void*, void*, const void*) {
    __asm {
        push ebx // 00afcf50
        push esi // 00afcf51
        mov esi,ecx // 00afcf52
        push edi // 00afcf54
        mov edi,dword ptr [esp + 010h] // 00afcf55
        fld dword ptr [edi] // 00afcf59
        lea edx,[edi + 060h] // 00afcf5b
        fstp dword ptr [esi] // 00afcf5e
        push edx // 00afcf60
        fld dword ptr [edi + 04h] // 00afcf61
        fstp dword ptr [esi + 04h] // 00afcf64
        fld dword ptr [edi + 08h] // 00afcf67
        fstp dword ptr [esi + 08h] // 00afcf6a
        fld dword ptr [edi + 0ch] // 00afcf6d
        fstp dword ptr [esi + 0ch] // 00afcf70
        fld dword ptr [edi + 010h] // 00afcf73
        fstp dword ptr [esi + 010h] // 00afcf76
        fld dword ptr [edi + 014h] // 00afcf79
        fstp dword ptr [esi + 014h] // 00afcf7c
        fld dword ptr [edi + 018h] // 00afcf7f
        fstp dword ptr [esi + 018h] // 00afcf82
        fld dword ptr [edi + 01ch] // 00afcf85
        fstp dword ptr [esi + 01ch] // 00afcf88
        fld dword ptr [edi + 020h] // 00afcf8b
        fstp dword ptr [esi + 020h] // 00afcf8e
        fld dword ptr [edi + 024h] // 00afcf91
        fstp dword ptr [esi + 024h] // 00afcf94
        fld dword ptr [edi + 028h] // 00afcf97
        fstp dword ptr [esi + 028h] // 00afcf9a
        fld dword ptr [edi + 02ch] // 00afcf9d
        fstp dword ptr [esi + 02ch] // 00afcfa0
        fld dword ptr [edi + 030h] // 00afcfa3
        fstp dword ptr [esi + 030h] // 00afcfa6
        fld dword ptr [edi + 034h] // 00afcfa9
        fstp dword ptr [esi + 034h] // 00afcfac
        fld dword ptr [edi + 038h] // 00afcfaf
        fstp dword ptr [esi + 038h] // 00afcfb2
        fld dword ptr [edi + 03ch] // 00afcfb5
        fstp dword ptr [esi + 03ch] // 00afcfb8
        mov eax,dword ptr [edi + 040h] // 00afcfbb
        mov dword ptr [esi + 040h],eax // 00afcfbe
        mov ecx,dword ptr [edi + 044h] // 00afcfc1
        mov dword ptr [esi + 044h],ecx // 00afcfc4
        fld dword ptr [edi + 048h] // 00afcfc7
        fstp dword ptr [esi + 048h] // 00afcfca
        lea ecx,[esi + 060h] // 00afcfcd
        fld dword ptr [edi + 04ch] // 00afcfd0
        fstp dword ptr [esi + 04ch] // 00afcfd3
        fld dword ptr [edi + 050h] // 00afcfd6
        fstp dword ptr [esi + 050h] // 00afcfd9
        fld dword ptr [edi + 054h] // 00afcfdc
        fstp dword ptr [esi + 054h] // 00afcfdf
        fld dword ptr [edi + 058h] // 00afcfe2
        fstp dword ptr [esi + 058h] // 00afcfe5
        fld dword ptr [edi + 05ch] // 00afcfe8
        fstp dword ptr [esi + 05ch] // 00afcfeb
        call copy_native_camera_matrix_004134f0 // 00afcfee
        mov eax,dword ptr [edi + 0a0h] // 00afcff3
        mov dword ptr [esi + 0a0h],eax // 00afcff9
        mov ecx,dword ptr [edi + 0a4h] // 00afcfff
        mov dword ptr [esi + 0a4h],ecx // 00afd005
        fld dword ptr [edi + 0a8h] // 00afd00b
        fstp dword ptr [esi + 0a8h] // 00afd011
        mov edx,edi // 00afd017
        fld dword ptr [edi + 0ach] // 00afd019
        lea ecx,[edi + 0c0h] // 00afd01f
        fstp dword ptr [esi + 0ach] // 00afd025
        lea eax,[esi + 0b8h] // 00afd02b
        fld dword ptr [edi + 0b0h] // 00afd031
        sub edx,esi // 00afd037
        fstp dword ptr [esi + 0b0h] // 00afd039
        mov ebx,02h // 00afd03f
        fld dword ptr [edi + 0b4h] // 00afd044
        fstp dword ptr [esi + 0b4h] // 00afd04a
    L_00afd050:
        fld dword ptr [edx + eax*01h] // 00afd050
        add eax,018h // 00afd053
        fstp dword ptr [eax + -018h] // 00afd056
        add ecx,018h // 00afd059
        sub ebx,01h // 00afd05c
        fld dword ptr [ecx + -01ch] // 00afd05f
        fstp dword ptr [eax + -014h] // 00afd062
        fld dword ptr [ecx + -018h] // 00afd065
        fstp dword ptr [eax + -010h] // 00afd068
        fld dword ptr [ecx + -014h] // 00afd06b
        fstp dword ptr [eax + -0ch] // 00afd06e
        fld dword ptr [ecx + -010h] // 00afd071
        fstp dword ptr [eax + -08h] // 00afd074
        fld dword ptr [ecx + -0ch] // 00afd077
        fstp dword ptr [eax + -04h] // 00afd07a
        jnz l_00afd050 // 00afd07d
        fld dword ptr [edi + 0e8h] // 00afd07f
        mov eax,esi // 00afd085
        fstp dword ptr [esi + 0e8h] // 00afd087
        fld dword ptr [edi + 0ech] // 00afd08d
        fstp dword ptr [esi + 0ech] // 00afd093
        fld dword ptr [edi + 0f0h] // 00afd099
        fstp dword ptr [esi + 0f0h] // 00afd09f
        fld dword ptr [edi + 0f4h] // 00afd0a5
        fstp dword ptr [esi + 0f4h] // 00afd0ab
        fld dword ptr [edi + 0f8h] // 00afd0b1
        fstp dword ptr [esi + 0f8h] // 00afd0b7
        fld dword ptr [edi + 0fch] // 00afd0bd
        fstp dword ptr [esi + 0fch] // 00afd0c3
        fld dword ptr [edi + 0100h] // 00afd0c9
        fstp dword ptr [esi + 0100h] // 00afd0cf
        fld dword ptr [edi + 0104h] // 00afd0d5
        pop edi // 00afd0db
        fstp dword ptr [esi + 0104h] // 00afd0dc
        pop esi // 00afd0e2
        pop ebx // 00afd0e3
        ret 04h // 00afd0e4
    }
}

__declspec(naked) void __fastcall append_native_particle_record_00afd410(NativeParticleModelArraysStorage*, void*, const void*) {
    __asm {
        push esi // 00afd410
        mov esi,ecx // 00afd411
        mov eax,dword ptr [esi + 014h] // 00afd413
        cmp eax,dword ptr [esi + 08h] // 00afd416
        jge l_00afd439 // 00afd419
        mov ecx,dword ptr [esp + 08h] // 00afd41b
        mov edx,dword ptr [esi + 04h] // 00afd41f
        push ecx // 00afd422
        movzx ecx,byte ptr [edx + eax*01h] // 00afd423
        imul ecx,ecx,0108h // 00afd427
        add ecx,dword ptr [esi + 0ch] // 00afd42d
        call copy_native_particle_record_00afcf50 // 00afd430
        add dword ptr [esi + 014h],01h // 00afd435
    L_00afd439:
        pop esi // 00afd439
        ret 04h // 00afd43a
    }
}

__declspec(naked) void* __fastcall copy_native_particle_model_position_00afe030(NativeNodeStorage*, void*, void*) {
    __asm {
        push esi // 00afe030
        mov esi,ecx // 00afe031
        cmp byte ptr [esi + 01b0h],00h // 00afe033
        jz l_00afe04e // 00afe03a
        push edi // 00afe03c
        mov edi,dword ptr [esp + 0ch] // 00afe03d
        push edi // 00afe041
        call copy_native_node_local_position_00b6e0a0 // 00afe042
        mov eax,edi // 00afe047
        pop edi // 00afe049
        pop esi // 00afe04a
        ret 04h // 00afe04b
    L_00afe04e:
        test byte ptr [esi + 05ch],02h // 00afe04e
        jnz l_00afe059 // 00afe052
        call refresh_native_camera_world_00b6db70 // 00afe054
    L_00afe059:
        fld dword ptr [esi + 0120h] // 00afe059
        mov eax,dword ptr [esp + 08h] // 00afe05f
        fstp dword ptr [eax] // 00afe063
        fld dword ptr [esi + 0124h] // 00afe065
        fstp dword ptr [eax + 04h] // 00afe06b
        fld dword ptr [esi + 0128h] // 00afe06e
        pop esi // 00afe074
        fstp dword ptr [eax + 08h] // 00afe075
        ret 04h // 00afe078
    }
}

__declspec(naked) std::uint8_t __fastcall update_native_particle_record_00afe290(void*, const NativeParticleRecordUpdateAccess*, float, float, std::uint8_t) {
    __asm {
        sub esp,080h // 00afe290
        mov dword ptr [esp+07ch],edx // borrowed access
        fld dword ptr [esp + 084h] // 00afe293
        push ebx // 00afe29a
        push ebp // 00afe29b
        push esi // 00afe29c
        mov esi,ecx // 00afe29d
        fsub dword ptr [esi + 030h] // 00afe29f
        movss xmm0,dword ptr [esi + 034h] // 00afe2a2
        movss dword ptr [esp + 014h],xmm0 // 00afe2a7
        xorps xmm0,xmm0 // 00afe2ad
        fstp dword ptr [esp + 01ch] // 00afe2b0
        push edi // 00afe2b4
        fld dword ptr [esp + 020h] // 00afe2b5
        fst dword ptr [esi + 034h] // 00afe2b9
        fld dword ptr [esi + 038h] // 00afe2bc
        fxch // 00afe2bf
        fcomip st(0),st(1) // 00afe2c1
        fstp st(0) // 00afe2c3
        jbe l_00afe2f8 // 00afe2c5
        mov eax,dword ptr [esi + 0a0h] // 00afe2c7
        cmp byte ptr [eax + 015h],00h // 00afe2cd
        jnz l_00afe2df // 00afe2d1
    L_00afe2d3:
        xor al,al // 00afe2d3
        pop edi // 00afe2d5
        pop esi // 00afe2d6
        pop ebp // 00afe2d7
        pop ebx // 00afe2d8
        add esp,080h // 00afe2d9
        ret 0ch // 00afe2dc
    L_00afe2df:
        movss xmm1,dword ptr [esp + 094h] // 00afe2df
        movss dword ptr [esi + 030h],xmm1 // 00afe2e8
        movss dword ptr [esi + 034h],xmm0 // 00afe2ed
        movss dword ptr [esp + 018h],xmm0 // 00afe2f2
    L_00afe2f8:
        fld dword ptr [esi + 034h] // 00afe2f8
        fsub dword ptr [esp + 018h] // 00afe2fb
        fstp dword ptr [esp + 018h] // 00afe2ff
        fld dword ptr [esi] // 00afe303
        fstp dword ptr [esi + 0ch] // 00afe305
        fld dword ptr [esi + 04h] // 00afe308
        fstp dword ptr [esi + 010h] // 00afe30b
        fld dword ptr [esi + 08h] // 00afe30e
        fstp dword ptr [esi + 014h] // 00afe311
        fld dword ptr [esi + 03ch] // 00afe314
        fmul dword ptr [esi + 034h] // 00afe317
        fstp dword ptr [esp + 014h] // 00afe31a
        fld dword ptr [esp + 014h] // 00afe31e
        push eax // added binding
        mov eax,dword ptr [esp+090h] // added binding
        mov eax,dword ptr [eax+08h] // added binding
        fld qword ptr [eax + 00h] // 00afe322
        pop eax
        fcomip st(0),st(1) // 00afe328
        fstp st(0) // 00afe32a
        jbe l_00afe336 // 00afe32c
        movss xmm0,dword ptr [esp + 014h] // 00afe32e
        jmp l_00afe33e // 00afe334
    L_00afe336:
        push eax // added binding
        mov eax,dword ptr [esp+090h] // added binding
        mov eax,dword ptr [eax+04h] // added binding
        movss xmm0,dword ptr [eax + 00h] // 00afe336
        pop eax
    L_00afe33e:
        mov eax,dword ptr [esi + 0a0h] // 00afe33e
        cmp dword ptr [eax + 070h],00h // 00afe344
        movss dword ptr [esp + 010h],xmm0 // 00afe348
        fld dword ptr [esp + 010h] // 00afe34e
        jz l_00afe5dd // 00afe352
        mov ecx,dword ptr [eax + 02ch] // 00afe358
        movzx eax,word ptr [ecx + 0ah] // 00afe35b
        test ax,ax // 00afe35f
        jnz l_00afe373 // 00afe362
        movss xmm0,dword ptr [ecx + 04h] // 00afe364
        fstp st(0) // 00afe369
        movss dword ptr [esp + 014h],xmm0 // 00afe36b
        jmp l_00afe38d // 00afe371
    L_00afe373:
        cmp ax,01h // 00afe373
        push ecx // 00afe377
        fstp dword ptr [esp] // 00afe378
        jnz l_00afe384 // 00afe37b
        call evaluate_native_particle_linear_curve_00affa70 // 00afe37d
        jmp l_00afe389 // 00afe382
    L_00afe384:
        call evaluate_native_particle_cubic_curve_00affae0 // 00afe384
    L_00afe389:
        fstp dword ptr [esp + 014h] // 00afe389
    L_00afe38d:
        fld dword ptr [esi + 048h] // 00afe38d
        fld dword ptr [esp + 014h] // 00afe390
        fld st(0) // 00afe394
        fmulp st(2),st(0) // 00afe396
        fxch // 00afe398
        fstp dword ptr [esp + 028h] // 00afe39a
        fld dword ptr [esi + 04ch] // 00afe39e
        fmul st(0),st(1) // 00afe3a1
        fstp dword ptr [esp + 02ch] // 00afe3a3
        fmul dword ptr [esi + 050h] // 00afe3a7
        fstp dword ptr [esp + 030h] // 00afe3aa
        fld dword ptr [esp + 028h] // 00afe3ae
        fstp dword ptr [esi + 024h] // 00afe3b2
        fld dword ptr [esp + 02ch] // 00afe3b5
        fstp dword ptr [esi + 028h] // 00afe3b9
        fld dword ptr [esp + 030h] // 00afe3bc
        fstp dword ptr [esi + 02ch] // 00afe3c0
        mov ecx,dword ptr [esi + 0a0h] // 00afe3c3
        mov ecx,dword ptr [ecx + 030h] // 00afe3c9
        movzx eax,word ptr [ecx + 0ah] // 00afe3cc
        test ax,ax // 00afe3d0
        jnz l_00afe3e2 // 00afe3d3
        movss xmm0,dword ptr [ecx + 04h] // 00afe3d5
        movss dword ptr [esp + 014h],xmm0 // 00afe3da
        jmp l_00afe400 // 00afe3e0
    L_00afe3e2:
        cmp ax,01h // 00afe3e2
        fld dword ptr [esp + 010h] // 00afe3e6
        push ecx // 00afe3ea
        fstp dword ptr [esp] // 00afe3eb
        jnz l_00afe3f7 // 00afe3ee
        call evaluate_native_particle_linear_curve_00affa70 // 00afe3f0
        jmp l_00afe3fc // 00afe3f5
    L_00afe3f7:
        call evaluate_native_particle_cubic_curve_00affae0 // 00afe3f7
    L_00afe3fc:
        fstp dword ptr [esp + 014h] // 00afe3fc
    L_00afe400:
        fld dword ptr [esp + 014h] // 00afe400
        push eax // added binding
        mov eax,dword ptr [esp+090h] // added binding
        mov eax,dword ptr [eax+0ch] // added binding
        fld qword ptr [eax + 00h] // 00afe404
        pop eax
        fmul st(0),st(1) // 00afe40a
        fstp dword ptr [esp + 020h] // 00afe40c
        fld dword ptr [esp + 020h] // 00afe410
        fst dword ptr [esp + 028h] // 00afe414
        fstp dword ptr [esp + 030h] // 00afe418
        fstp dword ptr [esp + 02ch] // 00afe41c
        fld dword ptr [esi + 0ach] // 00afe420
        fstp dword ptr [esp + 020h] // 00afe426
        fld dword ptr [esp + 028h] // 00afe42a
        fld dword ptr [esp + 020h] // 00afe42e
        fld st(0) // 00afe432
        fmulp st(2),st(0) // 00afe434
        fxch // 00afe436
        fstp dword ptr [esp + 034h] // 00afe438
        fld dword ptr [esp + 02ch] // 00afe43c
        fmul st(0),st(1) // 00afe440
        fstp dword ptr [esp + 038h] // 00afe442
        fmul dword ptr [esp + 030h] // 00afe446
        fstp dword ptr [esp + 03ch] // 00afe44a
        fld dword ptr [esi + 024h] // 00afe44e
        fadd dword ptr [esp + 034h] // 00afe451
        fstp dword ptr [esi + 024h] // 00afe455
        fld dword ptr [esp + 038h] // 00afe458
        fadd dword ptr [esi + 028h] // 00afe45c
        fstp dword ptr [esi + 028h] // 00afe45f
        fld dword ptr [esp + 03ch] // 00afe462
        fadd dword ptr [esi + 02ch] // 00afe466
        fstp dword ptr [esi + 02ch] // 00afe469
        mov edx,dword ptr [esi + 0a0h] // 00afe46c
        mov ecx,dword ptr [edx + 038h] // 00afe472
        movzx eax,word ptr [ecx + 0ah] // 00afe475
        test ax,ax // 00afe479
        jnz l_00afe48b // 00afe47c
        movss xmm0,dword ptr [ecx + 04h] // 00afe47e
        movss dword ptr [esp + 014h],xmm0 // 00afe483
        jmp l_00afe4a9 // 00afe489
    L_00afe48b:
        cmp ax,01h // 00afe48b
        fld dword ptr [esp + 010h] // 00afe48f
        push ecx // 00afe493
        fstp dword ptr [esp] // 00afe494
        jnz l_00afe4a0 // 00afe497
        call evaluate_native_particle_linear_curve_00affa70 // 00afe499
        jmp l_00afe4a5 // 00afe49e
    L_00afe4a0:
        call evaluate_native_particle_cubic_curve_00affae0 // 00afe4a0
    L_00afe4a5:
        fstp dword ptr [esp + 014h] // 00afe4a5
    L_00afe4a9:
        fld dword ptr [esp + 014h] // 00afe4a9
        fld st(0) // 00afe4ad
        push eax // added binding
        mov eax,dword ptr [esp+090h] // added binding
        mov eax,dword ptr [eax+010h] // added binding
        fmul dword ptr [eax + 00h] // 00afe4af
        pop eax
        fstp dword ptr [esp + 034h] // 00afe4b5
        fld st(0) // 00afe4b9
        push eax // added binding
        mov eax,dword ptr [esp+090h] // added binding
        mov eax,dword ptr [eax+010h] // added binding
        fmul dword ptr [eax + 04h] // 00afe4bb
        pop eax
        fstp dword ptr [esp + 038h] // 00afe4c1
        push eax // added binding
        mov eax,dword ptr [esp+090h] // added binding
        mov eax,dword ptr [eax+010h] // added binding
        fmul dword ptr [eax + 08h] // 00afe4c5
        pop eax
        fstp dword ptr [esp + 03ch] // 00afe4cb
        fld dword ptr [esi + 0b0h] // 00afe4cf
        fstp dword ptr [esp + 020h] // 00afe4d5
        fld dword ptr [esp + 034h] // 00afe4d9
        fld dword ptr [esp + 020h] // 00afe4dd
        fld st(0) // 00afe4e1
        fmulp st(2),st(0) // 00afe4e3
        fxch // 00afe4e5
        fstp dword ptr [esp + 028h] // 00afe4e7
        fld dword ptr [esp + 038h] // 00afe4eb
        fmul st(0),st(1) // 00afe4ef
        fstp dword ptr [esp + 02ch] // 00afe4f1
        fmul dword ptr [esp + 03ch] // 00afe4f5
        fstp dword ptr [esp + 030h] // 00afe4f9
        fld dword ptr [esi + 024h] // 00afe4fd
        fadd dword ptr [esp + 028h] // 00afe500
        fstp dword ptr [esi + 024h] // 00afe504
        fld dword ptr [esi + 028h] // 00afe507
        fadd dword ptr [esp + 02ch] // 00afe50a
        fstp dword ptr [esi + 028h] // 00afe50e
        fld dword ptr [esp + 030h] // 00afe511
        fadd dword ptr [esi + 02ch] // 00afe515
        fstp dword ptr [esi + 02ch] // 00afe518
        mov eax,dword ptr [esi + 0a0h] // 00afe51b
        mov ecx,dword ptr [eax + 034h] // 00afe521
        test ecx,ecx // 00afe524
        jz l_00afe59b // 00afe526
        movzx eax,word ptr [ecx + 0ah] // 00afe528
        test ax,ax // 00afe52c
        jnz l_00afe53e // 00afe52f
        movss xmm0,dword ptr [ecx + 04h] // 00afe531
        movss dword ptr [esp + 014h],xmm0 // 00afe536
        jmp l_00afe55c // 00afe53c
    L_00afe53e:
        cmp ax,01h // 00afe53e
        fld dword ptr [esp + 010h] // 00afe542
        push ecx // 00afe546
        fstp dword ptr [esp] // 00afe547
        jnz l_00afe553 // 00afe54a
        call evaluate_native_particle_linear_curve_00affa70 // 00afe54c
        jmp l_00afe558 // 00afe551
    L_00afe553:
        call evaluate_native_particle_cubic_curve_00affae0 // 00afe553
    L_00afe558:
        fstp dword ptr [esp + 014h] // 00afe558
    L_00afe55c:
        fld dword ptr [esi + 054h] // 00afe55c
        fld dword ptr [esp + 014h] // 00afe55f
        fld st(0) // 00afe563
        fmulp st(2),st(0) // 00afe565
        fxch // 00afe567
        fstp dword ptr [esp + 034h] // 00afe569
        fld dword ptr [esi + 058h] // 00afe56d
        fmul st(0),st(1) // 00afe570
        fstp dword ptr [esp + 038h] // 00afe572
        fmul dword ptr [esi + 05ch] // 00afe576
        fstp dword ptr [esp + 03ch] // 00afe579
        fld dword ptr [esi + 024h] // 00afe57d
        fadd dword ptr [esp + 034h] // 00afe580
        fstp dword ptr [esi + 024h] // 00afe584
        fld dword ptr [esp + 038h] // 00afe587
        fadd dword ptr [esi + 028h] // 00afe58b
        fstp dword ptr [esi + 028h] // 00afe58e
        fld dword ptr [esp + 03ch] // 00afe591
        fadd dword ptr [esi + 02ch] // 00afe595
        fstp dword ptr [esi + 02ch] // 00afe598
    L_00afe59b:
        fld dword ptr [esi + 024h] // 00afe59b
        fld dword ptr [esp + 098h] // 00afe59e
        fld st(0) // 00afe5a5
        fmulp st(2),st(0) // 00afe5a7
        fxch // 00afe5a9
        fstp dword ptr [esp + 034h] // 00afe5ab
        fld dword ptr [esi + 028h] // 00afe5af
        fmul st(0),st(1) // 00afe5b2
        fstp dword ptr [esp + 038h] // 00afe5b4
        fmul dword ptr [esi + 02ch] // 00afe5b8
        fstp dword ptr [esp + 03ch] // 00afe5bb
        fld dword ptr [esp + 034h] // 00afe5bf
        fadd dword ptr [esi] // 00afe5c3
        fstp dword ptr [esi] // 00afe5c5
        fld dword ptr [esp + 038h] // 00afe5c7
        fadd dword ptr [esi + 04h] // 00afe5cb
        fstp dword ptr [esi + 04h] // 00afe5ce
        fld dword ptr [esi + 08h] // 00afe5d1
        fadd dword ptr [esp + 03ch] // 00afe5d4
        fstp dword ptr [esi + 08h] // 00afe5d8
        jmp l_00afe644 // 00afe5db
    L_00afe5dd:
        mov ecx,dword ptr [esi + 0a4h] // 00afe5dd
        fstp st(0) // 00afe5e3
        cmp byte ptr [ecx + 01a4h],00h // 00afe5e5
        jnz l_00afe5f8 // 00afe5ec
        cmp byte ptr [eax + 01ch],00h // 00afe5ee
        jnz l_00afe2d3 // 00afe5f2
    L_00afe5f8:
        lea edx,[esp + 040h] // 00afe5f8
        push edx // 00afe5fc
        call copy_native_particle_model_position_00afe030 // 00afe5fd
        fld dword ptr [eax] // 00afe602
        fstp dword ptr [esi] // 00afe604
        fld dword ptr [eax + 04h] // 00afe606
        fstp dword ptr [esi + 04h] // 00afe609
        fld dword ptr [eax + 08h] // 00afe60c
        mov eax,dword ptr [esi + 0a4h] // 00afe60f
        fstp dword ptr [esi + 08h] // 00afe615
        movss xmm0,dword ptr [eax + 0200h] // 00afe618
        movss xmm1,dword ptr [eax + 0204h] // 00afe620
        movss xmm2,dword ptr [eax + 0208h] // 00afe628
        add eax,0200h // 00afe630
        movss dword ptr [esi + 024h],xmm0 // 00afe635
        movss dword ptr [esi + 028h],xmm1 // 00afe63a
        movss dword ptr [esi + 02ch],xmm2 // 00afe63f
    L_00afe644:
        mov eax,dword ptr [esi + 0a0h] // 00afe644
        cmp byte ptr [eax + 01dh],00h // 00afe64a
        jz l_00afe79f // 00afe64e
        fld dword ptr [esi] // 00afe654
        lea ecx,[esp + 028h] // 00afe656
        fsub dword ptr [esi + 0ch] // 00afe65a
        fstp dword ptr [esp + 028h] // 00afe65d
        fld dword ptr [esi + 04h] // 00afe661
        fsub dword ptr [esi + 010h] // 00afe664
        fstp dword ptr [esp + 02ch] // 00afe667
        fld dword ptr [esi + 08h] // 00afe66b
        fsub dword ptr [esi + 014h] // 00afe66e
        fstp dword ptr [esp + 030h] // 00afe671
        mov edx,dword ptr [esp+08ch] // added binding
        mov edx,dword ptr [edx] // added binding
        call camera_vector_length_00419440 // 00afe675
        push eax // added binding
        mov eax,dword ptr [esp+090h] // added binding
        mov eax,dword ptr [eax+014h] // added binding
        fld qword ptr [eax + 00h] // 00afe67a
        pop eax
        fxch // 00afe680
        fcomip st(0),st(1) // 00afe682
        fstp st(0) // 00afe684
        jbe l_00afe79f // 00afe686
        movss xmm0,dword ptr [esp + 030h] // 00afe68c
        movss xmm1,dword ptr [esp + 028h] // 00afe692
        movss xmm2,dword ptr [esp + 02ch] // 00afe698
        sub esp,0ch // 00afe69e
        mov eax,esp // 00afe6a1
        movss dword ptr [eax],xmm0 // 00afe6a3
        movss dword ptr [eax + 04h],xmm1 // 00afe6a7
        movss dword ptr [eax + 08h],xmm2 // 00afe6ac
        lea ecx,[esp + 040h] // 00afe6b1
        movss dword ptr [esp + 048h],xmm0 // 00afe6b5
        xorps xmm0,xmm0 // 00afe6bb
        push ecx // 00afe6be
        lea edx,[esp + 050h] // 00afe6bf
        lea ecx,[esp + 05ch] // 00afe6c3
        movss dword ptr [esp + 044h],xmm1 // 00afe6c7
        movss dword ptr [esp + 048h],xmm2 // 00afe6cd
        movss dword ptr [esp + 050h],xmm0 // 00afe6d3
        movss dword ptr [esp + 054h],xmm0 // 00afe6d9
        movss dword ptr [esp + 058h],xmm0 // 00afe6df
        mov eax,dword ptr [esp+09ch] // added binding
        call look_at_bridge // 00afe6e5
        lea edi,[esi + 060h] // 00afe6ea
        push eax // 00afe6ed
        mov ecx,edi // 00afe6ee
        call copy_native_camera_matrix_004134f0 // 00afe6f0
        fld dword ptr [edi + 010h] // 00afe6f5
        movss xmm0,dword ptr [edi + 04h] // 00afe6f8
        fstp dword ptr [edi + 04h] // 00afe6fd
        movss dword ptr [edi + 010h],xmm0 // 00afe700
        fld dword ptr [edi + 020h] // 00afe705
        movss xmm0,dword ptr [edi + 08h] // 00afe708
        fstp dword ptr [edi + 08h] // 00afe70d
        movss dword ptr [edi + 020h],xmm0 // 00afe710
        fld dword ptr [edi + 030h] // 00afe715
        movss xmm0,dword ptr [edi + 0ch] // 00afe718
        fstp dword ptr [edi + 0ch] // 00afe71d
        movss dword ptr [edi + 030h],xmm0 // 00afe720
        fld dword ptr [edi + 024h] // 00afe725
        movss xmm0,dword ptr [edi + 018h] // 00afe728
        fstp dword ptr [edi + 018h] // 00afe72d
        movss dword ptr [edi + 024h],xmm0 // 00afe730
        fld dword ptr [edi + 034h] // 00afe735
        movss xmm0,dword ptr [edi + 01ch] // 00afe738
        fstp dword ptr [edi + 01ch] // 00afe73d
        movss dword ptr [edi + 034h],xmm0 // 00afe740
        fld dword ptr [edi + 038h] // 00afe745
        movss xmm0,dword ptr [edi + 02ch] // 00afe748
        fstp dword ptr [edi + 02ch] // 00afe74d
        push edi // 00afe750
        mov ecx,edi // 00afe751
        movss dword ptr [edi + 038h],xmm0 // 00afe753
        call copy_native_camera_matrix_004134f0 // 00afe758
        fld dword ptr [esi + 080h] // 00afe75d
        movss xmm0,dword ptr [esi + 070h] // 00afe763
        movss xmm1,dword ptr [esi + 074h] // 00afe768
        movss xmm2,dword ptr [esi + 078h] // 00afe76d
        fstp dword ptr [esi + 070h] // 00afe772
        fld dword ptr [esi + 084h] // 00afe775
        fstp dword ptr [esi + 074h] // 00afe77b
        fld dword ptr [esi + 088h] // 00afe77e
        fstp dword ptr [esi + 078h] // 00afe784
        movss dword ptr [esi + 080h],xmm0 // 00afe787
        movss dword ptr [esi + 084h],xmm1 // 00afe78f
        movss dword ptr [esi + 088h],xmm2 // 00afe797
    L_00afe79f:
        cmp byte ptr [esp + 09ch],00h // 00afe79f
        jz l_00afe954 // 00afe7a7
        mov edx,dword ptr [esi + 0a0h] // 00afe7ad
        mov ecx,dword ptr [edx + 024h] // 00afe7b3
        movzx eax,word ptr [ecx + 0ah] // 00afe7b6
        test ax,ax // 00afe7ba
        jnz l_00afe7cc // 00afe7bd
        movss xmm0,dword ptr [ecx + 04h] // 00afe7bf
        movss dword ptr [esp + 014h],xmm0 // 00afe7c4
        jmp l_00afe7ea // 00afe7ca
    L_00afe7cc:
        cmp ax,01h // 00afe7cc
        fld dword ptr [esp + 010h] // 00afe7d0
        push ecx // 00afe7d4
        fstp dword ptr [esp] // 00afe7d5
        jnz l_00afe7e1 // 00afe7d8
        call evaluate_native_particle_linear_curve_00affa70 // 00afe7da
        jmp l_00afe7e6 // 00afe7df
    L_00afe7e1:
        call evaluate_native_particle_cubic_curve_00affae0 // 00afe7e1
    L_00afe7e6:
        fstp dword ptr [esp + 014h] // 00afe7e6
    L_00afe7ea:
        mov edi,dword ptr [esi + 0a0h] // 00afe7ea
        cmp dword ptr [edi + 074h],01h // 00afe7f0
        jnz l_00afe84a // 00afe7f4
        fld dword ptr [esi] // 00afe7f6
        fsub dword ptr [esi + 0ch] // 00afe7f8
        fstp dword ptr [esp + 034h] // 00afe7fb
        fld dword ptr [esi + 04h] // 00afe7ff
        fsub dword ptr [esi + 010h] // 00afe802
        fstp dword ptr [esp + 038h] // 00afe805
        fld dword ptr [esi + 08h] // 00afe809
        fsub dword ptr [esi + 014h] // 00afe80c
        fstp dword ptr [esp + 03ch] // 00afe80f
        fld dword ptr [esp + 038h] // 00afe813
        fld dword ptr [esp + 034h] // 00afe817
        fld dword ptr [esp + 03ch] // 00afe81b
        fld st(1) // 00afe81f
        fmulp st(2),st(0) // 00afe821
        fld st(2) // 00afe823
        fmulp st(3),st(0) // 00afe825
        fxch // 00afe827
        faddp st(2),st(0) // 00afe829
        fmul st(0),st(0) // 00afe82b
        faddp st(1),st(0) // 00afe82d
        fstp dword ptr [esp + 020h] // 00afe82f
        fld dword ptr [esp + 020h] // 00afe833
        mov ecx,dword ptr [esp+08ch] // added binding
        mov ecx,dword ptr [ecx] // added binding
        call native_crt_sqrt_st0_00bf7030 // 00afe837
        fstp dword ptr [esp + 020h] // 00afe83c
        fld dword ptr [esp + 020h] // 00afe840
        fmul dword ptr [esp + 014h] // 00afe844
        jmp l_00afe855 // 00afe848
    L_00afe84a:
        fld dword ptr [esp + 014h] // 00afe84a
        fmul dword ptr [esp + 098h] // 00afe84e
    L_00afe855:
        xorps xmm1,xmm1 // 00afe855
        fstp dword ptr [esp + 014h] // 00afe858
        movss xmm0,dword ptr [edi + 06ch] // 00afe85c
        ucomiss xmm0,xmm1 // 00afe861
        lahf // 00afe864
        test ah,044h // 00afe865
        movss dword ptr [esp + 01ch],xmm1 // 00afe868
        movss dword ptr [esp + 020h],xmm0 // 00afe86e
        jnp l_00afe882 // 00afe874
        fld dword ptr [esp + 014h] // 00afe876
        fdiv dword ptr [esp + 020h] // 00afe87a
        fstp dword ptr [esp + 01ch] // 00afe87e
    L_00afe882:
        xor ebx,ebx // 00afe882
        cmp dword ptr [edi + 068h],ebx // 00afe884
        jle l_00afe954 // 00afe887
        mov ecx,0ffffff9ch // 00afe88d
        sub ecx,esi // 00afe892
        lea ebp,[esi + 0b8h] // 00afe894
        mov dword ptr [esp + 024h],ecx // 00afe89a
        mov edi,edi // 00afe89e
    L_00afe8a0:
        mov edx,dword ptr [esi + 0a0h] // 00afe8a0
        lea eax,[ecx + ebp*01h] // 00afe8a6
        mov edi,dword ptr [eax + edx*01h] // 00afe8a9
        fld dword ptr [edi + 024h] // 00afe8ac
        fmul dword ptr [esp + 01ch] // 00afe8af
        fstp dword ptr [esp + 020h] // 00afe8b3
        fld dword ptr [esp + 020h] // 00afe8b7
        mov dword ptr [esp + 020h],00h // 00afe8bb
        fadd dword ptr [ebp] // 00afe8c3
        fstp dword ptr [esp + 014h] // 00afe8c6
        fld1 // 00afe8ca
        fld dword ptr [esp + 014h] // 00afe8cc
        fcomip st(0),st(1) // 00afe8d0
        fstp st(0) // 00afe8d2
        jc l_00afe91e // 00afe8d4
        mov ecx,dword ptr [esi + 0a4h] // 00afe8d6
        mov edx,dword ptr [ecx + 0194h] // 00afe8dc
        mov eax,dword ptr [edi + 074h] // 00afe8e2
        mov ecx,dword ptr [edx + eax*04h] // 00afe8e5
        lea eax,[edx + eax*04h] // 00afe8e8
        call acquire_bridge // 00afe8eb
        fld dword ptr [esp + 018h] // 00afe8f0
        sub esp,0ch // 00afe8f4
        fstp dword ptr [esp + 08h] // 00afe8f7
        mov ecx,eax // 00afe8fb
        fld dword ptr [esp + 0a0h] // 00afe8fd
        fstp dword ptr [esp + 04h] // 00afe904
        fld dword ptr [esp + 020h] // 00afe908
        fstp dword ptr [esp] // 00afe90c
        push esi // 00afe90f
        push edi // 00afe910
        mov edx,dword ptr [esp+0a0h] // added binding
        mov edx,dword ptr [edx+01ch] // actual emission access
        call spawn_native_particle_emission_00b04c80 // 00afe911
        mov ecx,dword ptr [esp + 024h] // 00afe916
        mov dword ptr [esp + 020h],eax // 00afe91a
    L_00afe91e:
        mov eax,dword ptr [esi + 0a0h] // 00afe91e
        cmp dword ptr [eax + 074h],01h // 00afe924
        jnz l_00afe934 // 00afe928
        xorps xmm0,xmm0 // 00afe92a
        movss dword ptr [ebp],xmm0 // 00afe92d
        jmp l_00afe93f // 00afe932
    L_00afe934:
        fld dword ptr [esp + 014h] // 00afe934
        fisub dword ptr [esp + 020h] // 00afe938
        fstp dword ptr [ebp] // 00afe93c
    L_00afe93f:
        mov edx,dword ptr [esi + 0a0h] // 00afe93f
        add ebx,01h // 00afe945
        add ebp,04h // 00afe948
        cmp ebx,dword ptr [edx + 068h] // 00afe94b
        jl l_00afe8a0 // 00afe94e
    L_00afe954:
        mov eax,dword ptr [esi + 0a0h] // 00afe954
        mov ecx,dword ptr [eax + 028h] // 00afe95a
        movzx eax,word ptr [ecx + 0ah] // 00afe95d
        test ax,ax // 00afe961
        jnz l_00afe973 // 00afe964
        movss xmm0,dword ptr [ecx + 04h] // 00afe966
        movss dword ptr [esp + 014h],xmm0 // 00afe96b
        jmp l_00afe991 // 00afe971
    L_00afe973:
        cmp ax,01h // 00afe973
        fld dword ptr [esp + 010h] // 00afe977
        push ecx // 00afe97b
        fstp dword ptr [esp] // 00afe97c
        jnz l_00afe988 // 00afe97f
        call evaluate_native_particle_linear_curve_00affa70 // 00afe981
        jmp l_00afe98d // 00afe986
    L_00afe988:
        call evaluate_native_particle_cubic_curve_00affae0 // 00afe988
    L_00afe98d:
        fstp dword ptr [esp + 014h] // 00afe98d
    L_00afe991:
        mov edi,dword ptr [esi + 0a0h] // 00afe991
        cmp dword ptr [edi + 078h],01h // 00afe997
        jnz l_00afe9f1 // 00afe99b
        fld dword ptr [esi] // 00afe99d
        fsub dword ptr [esi + 0ch] // 00afe99f
        fstp dword ptr [esp + 034h] // 00afe9a2
        fld dword ptr [esi + 04h] // 00afe9a6
        fsub dword ptr [esi + 010h] // 00afe9a9
        fstp dword ptr [esp + 038h] // 00afe9ac
        fld dword ptr [esi + 08h] // 00afe9b0
        fsub dword ptr [esi + 014h] // 00afe9b3
        fstp dword ptr [esp + 03ch] // 00afe9b6
        fld dword ptr [esp + 038h] // 00afe9ba
        fld dword ptr [esp + 034h] // 00afe9be
        fld dword ptr [esp + 03ch] // 00afe9c2
        fld st(1) // 00afe9c6
        fmulp st(2),st(0) // 00afe9c8
        fld st(2) // 00afe9ca
        fmulp st(3),st(0) // 00afe9cc
        fxch // 00afe9ce
        faddp st(2),st(0) // 00afe9d0
        fmul st(0),st(0) // 00afe9d2
        faddp st(1),st(0) // 00afe9d4
        fstp dword ptr [esp + 024h] // 00afe9d6
        fld dword ptr [esp + 024h] // 00afe9da
        mov ecx,dword ptr [esp+08ch] // added binding
        mov ecx,dword ptr [ecx] // added binding
        call native_crt_sqrt_st0_00bf7030 // 00afe9de
        fstp dword ptr [esp + 024h] // 00afe9e3
        fld dword ptr [esp + 024h] // 00afe9e7
        fmul dword ptr [esp + 014h] // 00afe9eb
        jmp l_00afe9fc // 00afe9ef
    L_00afe9f1:
        fld dword ptr [esp + 014h] // 00afe9f1
        fmul dword ptr [esp + 098h] // 00afe9f5
    L_00afe9fc:
        xorps xmm1,xmm1 // 00afe9fc
        fstp dword ptr [esp + 014h] // 00afe9ff
        movss xmm0,dword ptr [edi + 050h] // 00afea03
        ucomiss xmm0,xmm1 // 00afea08
        lahf // 00afea0b
        test ah,044h // 00afea0c
        movss dword ptr [esp + 020h],xmm1 // 00afea0f
        movss dword ptr [esp + 024h],xmm0 // 00afea15
        jnp l_00afea29 // 00afea1b
        fld dword ptr [esp + 014h] // 00afea1d
        fdiv dword ptr [esp + 024h] // 00afea21
        fstp dword ptr [esp + 020h] // 00afea25
    L_00afea29:
        xor ebx,ebx // 00afea29
        cmp dword ptr [edi + 04ch],ebx // 00afea2b
        jle l_00afeacb // 00afea2e
        mov ebp,0ffffff54h // 00afea34
        lea edi,[esi + 0e8h] // 00afea39
        sub ebp,esi // 00afea3f
    L_00afea41:
        mov edx,dword ptr [esi + 0a0h] // 00afea41
        lea ecx,[edi + ebp*01h] // 00afea47
        mov eax,dword ptr [ecx + edx*01h] // 00afea4a
        fld dword ptr [eax + 018h] // 00afea4d
        mov edx,dword ptr [esi + 0a4h] // 00afea50
        fmul dword ptr [esp + 020h] // 00afea56
        mov ecx,dword ptr [edx + 0190h] // 00afea5a
        sub esp,0ch // 00afea60
        fstp dword ptr [esp + 030h] // 00afea63
        fld dword ptr [esp + 030h] // 00afea67
        fadd dword ptr [edi] // 00afea6b
        fstp dword ptr [esp + 028h] // 00afea6d
        fld dword ptr [esp + 024h] // 00afea71
        fstp dword ptr [esp + 08h] // 00afea75
        fld dword ptr [esp + 0a0h] // 00afea79
        fstp dword ptr [esp + 04h] // 00afea80
        fld dword ptr [esp + 028h] // 00afea84
        fstp dword ptr [esp] // 00afea88
        push esi // 00afea8b
        push eax // 00afea8c
        push edx // 00afea8d
        mov edx,dword ptr [esp+0a4h] // added binding
        mov edx,dword ptr [edx+020h] // actual child-record access
        call append_native_particle_record_children_00afd440 // 00afea8e
        mov dword ptr [esp + 024h],eax // 00afea93
        mov eax,dword ptr [esi + 0a0h] // 00afea97
        cmp dword ptr [eax + 078h],01h // 00afea9d
        jnz l_00afeaac // 00afeaa1
        xorps xmm0,xmm0 // 00afeaa3
        movss dword ptr [edi],xmm0 // 00afeaa6
        jmp l_00afeab6 // 00afeaaa
    L_00afeaac:
        fld dword ptr [esp + 01ch] // 00afeaac
        fisub dword ptr [esp + 024h] // 00afeab0
        fstp dword ptr [edi] // 00afeab4
    L_00afeab6:
        mov ecx,dword ptr [esi + 0a0h] // 00afeab6
        add ebx,01h // 00afeabc
        add edi,04h // 00afeabf
        cmp ebx,dword ptr [ecx + 04ch] // 00afeac2
        jl l_00afea41 // 00afeac5
    L_00afeacb:
        pop edi // 00afeacb
        pop esi // 00afeacc
        pop ebp // 00afeacd
        mov al,01h // 00afeace
        pop ebx // 00afead0
        add esp,080h // 00afead1
        ret 0ch // 00afead4
    }
}

__declspec(naked) std::int32_t __fastcall update_native_particle_records_00afd7a0(NativeParticleModelArraysStorage*, const NativeParticleRecordUpdateAccess*, float, std::uint8_t) {
    __asm {
        push edx // per-call access above the original frame
        push esi // 00afd7a0
        push edi // 00afd7a1
        mov esi,ecx // 00afd7a2
        xor edi,edi // 00afd7a4
        cmp dword ptr [esi + 014h],edi // 00afd7a6
        jle l_00afd816 // 00afd7a9
        push ebx // 00afd7ab
        push ebp // 00afd7ac
        mov ebp,dword ptr [esp + 01ch] // 00afd7ad
    L_00afd7b1:
        mov ecx,dword ptr [esi + 04h] // 00afd7b1
        movzx ecx,byte ptr [ecx + edi*01h] // 00afd7b4
        mov eax,dword ptr [esi] // 00afd7b8
        imul ecx,ecx,0108h // 00afd7ba
        fld dword ptr [eax + 01a0h] // 00afd7c0
        fstp dword ptr [esp + 01ch] // 00afd7c6
        fld dword ptr [esp + 01ch] // 00afd7ca
        add ecx,dword ptr [esi + 0ch] // 00afd7ce
        push ebp // 00afd7d1
        sub esp,08h // 00afd7d2
        fstp dword ptr [esp + 04h] // 00afd7d5
        fld dword ptr [esp + 024h] // 00afd7d9
        fstp dword ptr [esp] // 00afd7dd
        mov edx,dword ptr [esp+01ch] // added binding
        call update_native_particle_record_00afe290 // 00afd7e0
        test al,al // 00afd7e5
        jnz l_00afd80c // 00afd7e7
        mov eax,dword ptr [esi + 014h] // 00afd7e9
        lea ecx,[eax + -01h] // 00afd7ec
        cmp edi,ecx // 00afd7ef
        jz l_00afd808 // 00afd7f1
        mov eax,dword ptr [esi + 04h] // 00afd7f3
        mov bl,byte ptr [eax + edi*01h] // 00afd7f6
        mov dl,byte ptr [eax + ecx*01h] // 00afd7f9
        mov byte ptr [eax + ecx*01h],bl // 00afd7fc
        mov eax,dword ptr [esi + 04h] // 00afd7ff
        mov byte ptr [edi + eax*01h],dl // 00afd802
        sub edi,01h // 00afd805
    L_00afd808:
        add dword ptr [esi + 014h],-01h // 00afd808
    L_00afd80c:
        add edi,01h // 00afd80c
        cmp edi,dword ptr [esi + 014h] // 00afd80f
        jl l_00afd7b1 // 00afd812
        pop ebp // 00afd814
        pop ebx // 00afd815
    L_00afd816:
        mov eax,dword ptr [esi + 014h] // 00afd816
        pop edi // 00afd819
        pop esi // 00afd81a
        add esp,04h // discard borrowed access // added binding
        ret 08h // 00afd81b
    }
}
} // namespace bsp
