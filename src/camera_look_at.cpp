#include "bsp/camera_look_at.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This recovered x87 operation schedule requires MSVC Win32.
#endif

namespace bsp {
namespace {
const unsigned char negative_zero[] = {0,0,0,0x80}; // 00D7A208, float -0
const unsigned char parallel_threshold[] = {0x2b,0x87,0x16,0xd9,0xce,0xf7,0xef,0x3f}; // 00D62BA0, double 0.999
const unsigned char up_perturbation[] = {0,0,0,0xa0,0x99,0x99,0xb9,0x3f}; // 00D7A3A0, double(float(0.1))
const std::uint32_t homogeneous_one = 0x3f800000u; // installed 00D7A24C

// Original frame and argument words remain at their native offsets. The
// additional final argument holds the actual CRT access at ESP+50h before
// each length call. The following argument borrows actual D7A24C at ESP+54h.
// Only the binding loads and RET18h differ from the native ABI.
__declspec(naked) float* __fastcall look_at_kernel(float*, const float*,
    const float*, float, float, float, const CameraAxesCrtAccess*, const volatile std::uint32_t*) {
    __asm {
        sub esp,0x34 // 00b63f10
        push esi // 00b63f13
        mov esi,ecx // 00b63f14
        push edi // 00b63f16
        lea ecx,[esp + 0x44] // 00b63f17
        mov edi,edx // 00b63f1b
        mov edx, dword ptr [esp + 0x50] // added actual CRT access
        call camera_vector_length_00419440 // 00b63f1d
        fstp dword ptr [esp + 0x8] // 00b63f22
        fldz // 00b63f26
        fld dword ptr [esp + 0x8] // 00b63f28
        fcomi st(0),st(1) // 00b63f2c
        fstp st(1) // 00b63f2e
        jbe L_00b63f3c // 00b63f30
        fld1 // 00b63f32
        fdivrp st(1),st(0) // 00b63f34
        fstp dword ptr [esp + 0x8] // 00b63f36
        jmp L_00b63f47 // 00b63f3a
L_00b63f3c:
        xorps xmm0,xmm0 // 00b63f3c
        fstp st(0) // 00b63f3f
        movss dword ptr [esp + 0x8],xmm0 // 00b63f41
L_00b63f47:
        fld dword ptr [esp + 0x44] // 00b63f47
        mov eax,dword ptr [esp + 0x40] // 00b63f4b
        fld dword ptr [esp + 0x8] // 00b63f4f
        lea ecx,[esp + 0x44] // 00b63f53
        fld st(0) // 00b63f57
        fmulp st(2),st(0) // 00b63f59
        fxch // 00b63f5b
        fstp dword ptr [esp + 0x18] // 00b63f5d
        fld dword ptr [esp + 0x48] // 00b63f61
        fmul st(0),st(1) // 00b63f65
        fstp dword ptr [esp + 0x1c] // 00b63f67
        fmul dword ptr [esp + 0x4c] // 00b63f6b
        fstp dword ptr [esp + 0x20] // 00b63f6f
        fld dword ptr [edi] // 00b63f73
        fstp dword ptr [esp + 0x30] // 00b63f75
        fld dword ptr [edi + 0x4] // 00b63f79
        fstp dword ptr [esp + 0x34] // 00b63f7c
        fld dword ptr [edi + 0x8] // 00b63f80
        fstp dword ptr [esp + 0x38] // 00b63f83
        fld dword ptr [eax] // 00b63f87
        fsub dword ptr [esp + 0x30] // 00b63f89
        fstp dword ptr [esp + 0x44] // 00b63f8d
        fld dword ptr [eax + 0x4] // 00b63f91
        fsub dword ptr [esp + 0x34] // 00b63f94
        fstp dword ptr [esp + 0x48] // 00b63f98
        fld dword ptr [eax + 0x8] // 00b63f9c
        fsub dword ptr [esp + 0x38] // 00b63f9f
        fstp dword ptr [esp + 0x4c] // 00b63fa3
        mov edx, dword ptr [esp + 0x50] // added actual CRT access
        call camera_vector_length_00419440 // 00b63fa7
        fstp dword ptr [esp + 0x40] // 00b63fac
        fldz // 00b63fb0
        fld dword ptr [esp + 0x40] // 00b63fb2
        fcomi st(0),st(1) // 00b63fb6
        jbe L_00b63fc4 // 00b63fb8
        fld1 // 00b63fba
        fdivrp st(1),st(0) // 00b63fbc
        fstp dword ptr [esp + 0x40] // 00b63fbe
        jmp L_00b63fcf // 00b63fc2
L_00b63fc4:
        xorps xmm0,xmm0 // 00b63fc4
        fstp st(0) // 00b63fc7
        movss dword ptr [esp + 0x40],xmm0 // 00b63fc9
L_00b63fcf:
        fld dword ptr [esp + 0x44] // 00b63fcf
        fld dword ptr [esp + 0x40] // 00b63fd3
        fld st(0) // 00b63fd7
        fmulp st(2),st(0) // 00b63fd9
        fxch // 00b63fdb
        fstp dword ptr [esp + 0xc] // 00b63fdd
        fld dword ptr [esp + 0x48] // 00b63fe1
        fmul st(0),st(1) // 00b63fe5
        fstp dword ptr [esp + 0x10] // 00b63fe7
        fmul dword ptr [esp + 0x4c] // 00b63feb
        fstp dword ptr [esp + 0x14] // 00b63fef
        fld dword ptr [esp + 0x1c] // 00b63ff3
        fld dword ptr [esp + 0x10] // 00b63ff7
        fld st(0) // 00b63ffb
        fmulp st(2),st(0) // 00b63ffd
        fld dword ptr [esp + 0x18] // 00b63fff
        fld dword ptr [esp + 0xc] // 00b64003
        fld st(0) // 00b64007
        fmulp st(2),st(0) // 00b64009
        fxch st(3) // 00b6400b
        faddp st(1),st(0) // 00b6400d
        fld dword ptr [esp + 0x20] // 00b6400f
        fld st(0) // 00b64013
        fld dword ptr [esp + 0x14] // 00b64015
        fld st(0) // 00b64019
        fmulp st(2),st(0) // 00b6401b
        fxch st(3) // 00b6401d
        faddp st(1),st(0) // 00b6401f
        fstp dword ptr [esp + 0x40] // 00b64021
        fld dword ptr [esp + 0x40] // 00b64025
        fcomip st(0),st(5) // 00b64029
        fstp st(4) // 00b6402b
        jbe L_00b64037 // 00b6402d
        movss xmm0,dword ptr [esp + 0x40] // 00b6402f
        jmp L_00b64045 // 00b64035
L_00b64037:
        movss xmm0,dword ptr negative_zero // 00b64037
        subss xmm0,dword ptr [esp + 0x40] // 00b6403f
L_00b64045:
        fld qword ptr parallel_threshold // 00b64045
        movss dword ptr [esp + 0x40],xmm0 // 00b6404b
        fld dword ptr [esp + 0x40] // 00b64051
        fcomip st(0),st(1) // 00b64055
        fstp st(0) // 00b64057
        jbe L_00b64105 // 00b64059
        fld st(2) // 00b6405f
        fabs // 00b64061
        fstp dword ptr [esp + 0x40] // 00b64063
        fld st(1) // 00b64067
        fabs // 00b64069
        fstp dword ptr [esp + 0x44] // 00b6406b
        fld dword ptr [esp + 0x44] // 00b6406f
        fld dword ptr [esp + 0x40] // 00b64073
        fcomi st(0),st(1) // 00b64077
        jbe L_00b640a5 // 00b64079
        fld st(2) // 00b6407b
        fabs // 00b6407d
        fstp dword ptr [esp + 0x40] // 00b6407f
        fld dword ptr [esp + 0x40] // 00b64083
        fxch // 00b64087
        fcomi st(0),st(1) // 00b64089
        fstp st(1) // 00b6408b
        jbe L_00b640a5 // 00b6408d
        fstp st(2) // 00b6408f
        fstp st(1) // 00b64091
        fstp st(1) // 00b64093
        fstp st(0) // 00b64095
        fmul qword ptr up_perturbation // 00b64097
        faddp st(1),st(0) // 00b6409d
        fstp dword ptr [esp + 0x20] // 00b6409f
        jmp L_00b64113 // 00b640a3
L_00b640a5:
        fstp st(4) // 00b640a5
        fstp st(4) // 00b640a7
        fxch st(3) // 00b640a9
        fcomi st(0),st(2) // 00b640ab
        jbe L_00b640d9 // 00b640ad
        fld st(3) // 00b640af
        fabs // 00b640b1
        fstp dword ptr [esp + 0x40] // 00b640b3
        fld dword ptr [esp + 0x40] // 00b640b7
        fxch // 00b640bb
        fcomi st(0),st(1) // 00b640bd
        fstp st(1) // 00b640bf
        jbe L_00b640d9 // 00b640c1
        fstp st(3) // 00b640c3
        fstp st(1) // 00b640c5
        fstp st(1) // 00b640c7
        fmul qword ptr up_perturbation // 00b640c9
        fadd dword ptr [esp + 0x18] // 00b640cf
        fstp dword ptr [esp + 0x18] // 00b640d3
        jmp L_00b64113 // 00b640d7
L_00b640d9:
        fstp st(1) // 00b640d9
        fld st(2) // 00b640db
        fabs // 00b640dd
        fstp dword ptr [esp + 0x40] // 00b640df
        fld dword ptr [esp + 0x40] // 00b640e3
        fcomi st(0),st(2) // 00b640e7
        fstp st(2) // 00b640e9
        jbe L_00b6410d // 00b640eb
        fxch // 00b640ed
        fcomip st(0),st(1) // 00b640ef
        fstp st(0) // 00b640f1
        jbe L_00b64111 // 00b640f3
        fmul qword ptr up_perturbation // 00b640f5
        fadd dword ptr [esp + 0x18] // 00b640fb
        fstp dword ptr [esp + 0x18] // 00b640ff
        jmp L_00b64113 // 00b64103
L_00b64105:
        fstp st(2) // 00b64105
        fstp st(1) // 00b64107
        fstp st(0) // 00b64109
        jmp L_00b64111 // 00b6410b
L_00b6410d:
        fstp st(1) // 00b6410d
        fstp st(1) // 00b6410f
L_00b64111:
        fstp st(0) // 00b64111
L_00b64113:
        lea eax,[esp + 0xc] // 00b64113
        push eax // 00b64117
        lea edx,[esp + 0x1c] // 00b64118
        lea ecx,[esp + 0x28] // 00b6411c
        call camera_vector_cross_004f9b30 // 00b64120
        lea ecx,[esp + 0x24] // 00b64125
        mov edx, dword ptr [esp + 0x50] // added actual CRT access
        call camera_vector_length_00419440 // 00b64129
        fstp dword ptr [esp + 0x40] // 00b6412e
        fldz // 00b64132
        fld dword ptr [esp + 0x40] // 00b64134
        fcomi st(0),st(1) // 00b64138
        fstp st(1) // 00b6413a
        jbe L_00b64148 // 00b6413c
        fld1 // 00b6413e
        fdivrp st(1),st(0) // 00b64140
        fstp dword ptr [esp + 0x40] // 00b64142
        jmp L_00b64153 // 00b64146
L_00b64148:
        xorps xmm0,xmm0 // 00b64148
        fstp st(0) // 00b6414b
        movss dword ptr [esp + 0x40],xmm0 // 00b6414d
L_00b64153:
        fld dword ptr [esp + 0x40] // 00b64153
        lea ecx,[esp + 0x24] // 00b64157
        fld st(0) // 00b6415b
        push ecx // 00b6415d
        fmul dword ptr [esp + 0x28] // 00b6415e
        lea edx,[esp + 0x10] // 00b64162
        lea ecx,[esp + 0x1c] // 00b64166
        fstp dword ptr [esp + 0x48] // 00b6416a
        fld st(0) // 00b6416e
        fmul dword ptr [esp + 0x2c] // 00b64170
        fstp dword ptr [esp + 0x4c] // 00b64174
        fmul dword ptr [esp + 0x30] // 00b64178
        fstp dword ptr [esp + 0x50] // 00b6417c
        fld dword ptr [esp + 0x48] // 00b64180
        fstp dword ptr [esp + 0x28] // 00b64184
        fld dword ptr [esp + 0x4c] // 00b64188
        fstp dword ptr [esp + 0x2c] // 00b6418c
        fld dword ptr [esp + 0x50] // 00b64190
        fstp dword ptr [esp + 0x30] // 00b64194
        call camera_vector_cross_004f9b30 // 00b64198
        lea ecx,[esp + 0x18] // 00b6419d
        mov edx, dword ptr [esp + 0x50] // added actual CRT access
        call camera_vector_length_00419440 // 00b641a1
        fstp dword ptr [esp + 0x40] // 00b641a6
        fldz // 00b641aa
        xorps xmm0,xmm0 // 00b641ac
        fld dword ptr [esp + 0x40] // 00b641af
        fcomi st(0),st(1) // 00b641b3
        fstp st(1) // 00b641b5
        jbe L_00b641c3 // 00b641b7
        fld1 // 00b641b9
        fdivrp st(1),st(0) // 00b641bb
        fstp dword ptr [esp + 0x40] // 00b641bd
        jmp L_00b641cb // 00b641c1
L_00b641c3:
        fstp st(0) // 00b641c3
        movss dword ptr [esp + 0x40],xmm0 // 00b641c5
L_00b641cb:
        fld dword ptr [esp + 0x40] // 00b641cb
        movss xmm1,dword ptr [esp + 0x44] // 00b641cf
        fld st(0) // 00b641d5
        movss dword ptr [esi],xmm1 // 00b641d7
        fmul dword ptr [esp + 0x18] // 00b641db
        movss xmm1,dword ptr [esp + 0xc] // 00b641df
        movss dword ptr [esi + 0x8],xmm1 // 00b641e5
        movss xmm1,dword ptr [esp + 0x48] // 00b641ea
        fstp dword ptr [esp + 0x24] // 00b641f0
        movss dword ptr [esi + 0x10],xmm1 // 00b641f4
        movss xmm1,dword ptr [esp + 0x10] // 00b641f9
        fld st(0) // 00b641ff
        movss dword ptr [esi + 0x18],xmm1 // 00b64201
        fmul dword ptr [esp + 0x1c] // 00b64206
        movss xmm1,dword ptr [esp + 0x4c] // 00b6420a
        movss dword ptr [esi + 0xc],xmm0 // 00b64210
        movss dword ptr [esi + 0x1c],xmm0 // 00b64215
        fstp dword ptr [esp + 0x28] // 00b6421a
        movss dword ptr [esi + 0x20],xmm1 // 00b6421e
        movss xmm1,dword ptr [esp + 0x14] // 00b64223
        fmul dword ptr [esp + 0x20] // 00b64229
        movss dword ptr [esi + 0x2c],xmm0 // 00b6422d
        mov eax,dword ptr [esp + 0x54] // borrowed actual D7A24C address
        movss xmm0,dword ptr [eax] // 00b64232; after all builder callbacks
        pop edi // 00b6423a
        fstp dword ptr [esp + 0x28] // 00b6423b
        movss dword ptr [esi + 0x28],xmm1 // 00b6423f
        fld dword ptr [esp + 0x20] // 00b64244
        movss dword ptr [esi + 0x3c],xmm0 // 00b64248
        fst dword ptr [esi + 0x4] // 00b6424d
        mov eax,esi // 00b64250
        fld dword ptr [esp + 0x24] // 00b64252
        fst dword ptr [esi + 0x14] // 00b64256
        fld dword ptr [esp + 0x28] // 00b64259
        fstp dword ptr [esi + 0x24] // 00b6425d
        fld dword ptr [esp + 0x44] // 00b64260
        fld dword ptr [esp + 0x30] // 00b64264
        fld st(0) // 00b64268
        fmulp st(2),st(0) // 00b6426a
        fld dword ptr [esp + 0x40] // 00b6426c
        fld dword ptr [esp + 0x2c] // 00b64270
        fld st(0) // 00b64274
        fmulp st(2),st(0) // 00b64276
        fxch st(3) // 00b64278
        faddp st(1),st(0) // 00b6427a
        fld dword ptr [esp + 0x48] // 00b6427c
        fld dword ptr [esp + 0x34] // 00b64280
        fld st(0) // 00b64284
        fmulp st(2),st(0) // 00b64286
        fxch st(2) // 00b64288
        faddp st(1),st(0) // 00b6428a
        fstp dword ptr [esp + 0x3c] // 00b6428c
        fld dword ptr [esp + 0x3c] // 00b64290
        fchs // 00b64294
        fstp dword ptr [esi + 0x30] // 00b64296
        fld st(1) // 00b64299
        fmulp st(4),st(0) // 00b6429b
        fld st(2) // 00b6429d
        fmulp st(5),st(0) // 00b6429f
        fxch st(3) // 00b642a1
        faddp st(4),st(0) // 00b642a3
        fld dword ptr [esp + 0x28] // 00b642a5
        fmul st(0),st(3) // 00b642a9
        faddp st(4),st(0) // 00b642ab
        fxch st(3) // 00b642ad
        fstp dword ptr [esp + 0x3c] // 00b642af
        fld dword ptr [esp + 0x3c] // 00b642b3
        fchs // 00b642b7
        fstp dword ptr [esi + 0x34] // 00b642b9
        fld dword ptr [esp + 0xc] // 00b642bc
        fmulp st(3),st(0) // 00b642c0
        fmul dword ptr [esp + 0x8] // 00b642c2
        faddp st(2),st(0) // 00b642c6
        fmul dword ptr [esp + 0x10] // 00b642c8
        faddp st(1),st(0) // 00b642cc
        fstp dword ptr [esp + 0x3c] // 00b642ce
        fld dword ptr [esp + 0x3c] // 00b642d2
        fchs // 00b642d6
        fstp dword ptr [esi + 0x38] // 00b642d8
        pop esi // 00b642db
        add esp,0x34 // 00b642dc
        ret 0x18 // native RET10h plus CRT and constant pointers // 00b642df
    }
}
} // namespace

CameraMatrix& build_camera_look_at_00b63f10(CameraMatrix& destination,
    const CameraAxis& eye, const CameraAxis& target, const CameraAxis& world_up,
    const CameraAxesCrtAccess& crt) {
    return build_camera_look_at_00b63f10(destination, eye, target, world_up, crt, homogeneous_one);
}
CameraMatrix& build_camera_look_at_00b63f10(CameraMatrix& destination,
    const CameraAxis& eye, const CameraAxis& target, const CameraAxis& world_up,
    const CameraAxesCrtAccess& crt, const volatile std::uint32_t& one_00d7a24c) {
    if (!crt.dispatch_bypass_0109dd78 || !crt.except_00c27489)
        throw std::invalid_argument("Camera look-at requires actual CRT state and __87except binding");
    float* output = destination.data();
    const float* position = eye.data();
    const float* aim = target.data();
    const float* up = world_up.data();
    const CameraAxesCrtAccess* access = &crt;
    const volatile std::uint32_t* actual_one = &one_00d7a24c;
    __asm {
        push actual_one
        push access
        mov eax, up
        push dword ptr [eax + 8]
        push dword ptr [eax + 4]
        push dword ptr [eax]
        push aim
        mov edx, position
        mov ecx, output
        call look_at_kernel
    }
    return destination;
}
} // namespace bsp
