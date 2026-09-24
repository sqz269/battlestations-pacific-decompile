#include "bsp/native_camera_pose_storage.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include <cstddef>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera pose storage requires MSVC Win32 x87/SSE assembly.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
Word read(Word p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(p + offset);
}
void write(Word p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(p + offset) = value;
}
struct KernelBindings {
    const CameraAxesCrtAccess* crt;
    const volatile Word* negative_zero;
    const volatile std::uint64_t* parallel;
    const volatile std::uint64_t* perturbation;
    const volatile Word* one;
};
static_assert(offsetof(NativeCameraLookAtFrame, arguments) == 4);
static_assert(offsetof(KernelBindings, one) == 16);
void require_crt(const NativeCameraLookAtBindings& bindings) {
    if (!bindings.crt.dispatch_bypass_0109dd78 || !bindings.crt.except_00c27489)
        throw std::invalid_argument("Raw camera pose requires genuine live CRT bindings");
}
// Called while z is live in ST0. Every operation and the required resolver are
// integer-only, x87-neutral metadata access. Do not introduce FP work here.
__declspec(noinline) Word __cdecl capture_world_target(void* actual,
    NativeCameraPoseDispatch* calls) {
    const Word profile = read(bits(actual));
    return calls->resolve_profile(profile)[0x34 / 4];
}

// Full B63F10 instruction schedule with original physical scalar locations
// mapped to caller-owned arrays. EBP=locals13, EBX=mutable arguments4,
// ESI=actual output, EDI=captured eye. Private saved-register/return cells are
// not part of this new ABI. Library kernels and their FP schedule are reused.
__declspec(naked) void* __fastcall look_at_raw_kernel(void*, const void*,
    const NativeCameraLookAtFrame*, const KernelBindings*) {
    __asm {
        push ebx
        push ebp
        push esi
        push edi
        mov esi,ecx
        mov edi,edx
        mov eax,dword ptr [esp + 0x14]
        mov ebp,dword ptr [eax]
        mov ebx,dword ptr [eax + 4]
        lea ecx,[ebx + 0x4] // 00b63f17
        mov edx,dword ptr [esp + 0x18] // pure CRT binding metadata
        mov edx,dword ptr [edx]
        call camera_vector_length_00419440 // 00b63f1d
        fstp dword ptr [ebp] // 00b63f22
        fldz // 00b63f26
        fld dword ptr [ebp] // 00b63f28
        fcomi st(0),st(1) // 00b63f2c
        fstp st(1) // 00b63f2e
        jbe L_00b63f3c // 00b63f30
        fld1 // 00b63f32
        fdivrp st(1),st(0) // 00b63f34
        fstp dword ptr [ebp] // 00b63f36
        jmp L_00b63f47 // 00b63f3a
    L_00b63f3c:
        xorps xmm0,xmm0 // 00b63f3c
        fstp st(0) // 00b63f3f
        movss dword ptr [ebp],xmm0 // 00b63f41
    L_00b63f47:
        fld dword ptr [ebx + 0x4] // 00b63f47
        mov eax,dword ptr [ebx] // 00b63f4b
        fld dword ptr [ebp] // 00b63f4f
        lea ecx,[ebx + 0x4] // 00b63f53
        fld st(0) // 00b63f57
        fmulp st(2),st(0) // 00b63f59
        fxch // 00b63f5b
        fstp dword ptr [ebp + 0x10] // 00b63f5d
        fld dword ptr [ebx + 0x8] // 00b63f61
        fmul st(0),st(1) // 00b63f65
        fstp dword ptr [ebp + 0x14] // 00b63f67
        fmul dword ptr [ebx + 0xc] // 00b63f6b
        fstp dword ptr [ebp + 0x18] // 00b63f6f
        fld dword ptr [edi] // 00b63f73
        fstp dword ptr [ebp + 0x28] // 00b63f75
        fld dword ptr [edi + 0x4] // 00b63f79
        fstp dword ptr [ebp + 0x2c] // 00b63f7c
        fld dword ptr [edi + 0x8] // 00b63f80
        fstp dword ptr [ebp + 0x30] // 00b63f83
        fld dword ptr [eax] // 00b63f87
        fsub dword ptr [ebp + 0x28] // 00b63f89
        fstp dword ptr [ebx + 0x4] // 00b63f8d
        fld dword ptr [eax + 0x4] // 00b63f91
        fsub dword ptr [ebp + 0x2c] // 00b63f94
        fstp dword ptr [ebx + 0x8] // 00b63f98
        fld dword ptr [eax + 0x8] // 00b63f9c
        fsub dword ptr [ebp + 0x30] // 00b63f9f
        fstp dword ptr [ebx + 0xc] // 00b63fa3
        mov edx,dword ptr [esp + 0x18] // pure CRT binding metadata
        mov edx,dword ptr [edx]
        call camera_vector_length_00419440 // 00b63fa7
        fstp dword ptr [ebx] // 00b63fac
        fldz // 00b63fb0
        fld dword ptr [ebx] // 00b63fb2
        fcomi st(0),st(1) // 00b63fb6
        jbe L_00b63fc4 // 00b63fb8
        fld1 // 00b63fba
        fdivrp st(1),st(0) // 00b63fbc
        fstp dword ptr [ebx] // 00b63fbe
        jmp L_00b63fcf // 00b63fc2
    L_00b63fc4:
        xorps xmm0,xmm0 // 00b63fc4
        fstp st(0) // 00b63fc7
        movss dword ptr [ebx],xmm0 // 00b63fc9
    L_00b63fcf:
        fld dword ptr [ebx + 0x4] // 00b63fcf
        fld dword ptr [ebx] // 00b63fd3
        fld st(0) // 00b63fd7
        fmulp st(2),st(0) // 00b63fd9
        fxch // 00b63fdb
        fstp dword ptr [ebp + 0x4] // 00b63fdd
        fld dword ptr [ebx + 0x8] // 00b63fe1
        fmul st(0),st(1) // 00b63fe5
        fstp dword ptr [ebp + 0x8] // 00b63fe7
        fmul dword ptr [ebx + 0xc] // 00b63feb
        fstp dword ptr [ebp + 0xc] // 00b63fef
        fld dword ptr [ebp + 0x14] // 00b63ff3
        fld dword ptr [ebp + 0x8] // 00b63ff7
        fld st(0) // 00b63ffb
        fmulp st(2),st(0) // 00b63ffd
        fld dword ptr [ebp + 0x10] // 00b63fff
        fld dword ptr [ebp + 0x4] // 00b64003
        fld st(0) // 00b64007
        fmulp st(2),st(0) // 00b64009
        fxch st(3) // 00b6400b
        faddp st(1),st(0) // 00b6400d
        fld dword ptr [ebp + 0x18] // 00b6400f
        fld st(0) // 00b64013
        fld dword ptr [ebp + 0xc] // 00b64015
        fld st(0) // 00b64019
        fmulp st(2),st(0) // 00b6401b
        fxch st(3) // 00b6401d
        faddp st(1),st(0) // 00b6401f
        fstp dword ptr [ebx] // 00b64021
        fld dword ptr [ebx] // 00b64025
        fcomip st(0),st(5) // 00b64029
        fstp st(4) // 00b6402b
        jbe L_00b64037 // 00b6402d
        movss xmm0,dword ptr [ebx] // 00b6402f
        jmp L_00b64045 // 00b64035
    L_00b64037:
        mov eax,dword ptr [esp + 0x18] // pure kernel binding metadata
        mov eax,dword ptr [eax + 0x4]
        movss xmm0,dword ptr [eax] // 00b64037
        subss xmm0,dword ptr [ebx] // 00b6403f
    L_00b64045:
        mov eax,dword ptr [esp + 0x18] // pure kernel binding metadata
        mov eax,dword ptr [eax + 0x8]
        fld qword ptr [eax] // 00b64045
        movss dword ptr [ebx],xmm0 // 00b6404b
        fld dword ptr [ebx] // 00b64051
        fcomip st(0),st(1) // 00b64055
        fstp st(0) // 00b64057
        jbe L_00b64105 // 00b64059
        fld st(2) // 00b6405f
        fabs // 00b64061
        fstp dword ptr [ebx] // 00b64063
        fld st(1) // 00b64067
        fabs // 00b64069
        fstp dword ptr [ebx + 0x4] // 00b6406b
        fld dword ptr [ebx + 0x4] // 00b6406f
        fld dword ptr [ebx] // 00b64073
        fcomi st(0),st(1) // 00b64077
        jbe L_00b640a5 // 00b64079
        fld st(2) // 00b6407b
        fabs // 00b6407d
        fstp dword ptr [ebx] // 00b6407f
        fld dword ptr [ebx] // 00b64083
        fxch // 00b64087
        fcomi st(0),st(1) // 00b64089
        fstp st(1) // 00b6408b
        jbe L_00b640a5 // 00b6408d
        fstp st(2) // 00b6408f
        fstp st(1) // 00b64091
        fstp st(1) // 00b64093
        fstp st(0) // 00b64095
        mov eax,dword ptr [esp + 0x18] // pure kernel binding metadata
        mov eax,dword ptr [eax + 0xc]
        fmul qword ptr [eax] // 00b64097
        faddp st(1),st(0) // 00b6409d
        fstp dword ptr [ebp + 0x18] // 00b6409f
        jmp L_00b64113 // 00b640a3
    L_00b640a5:
        fstp st(4) // 00b640a5
        fstp st(4) // 00b640a7
        fxch st(3) // 00b640a9
        fcomi st(0),st(2) // 00b640ab
        jbe L_00b640d9 // 00b640ad
        fld st(3) // 00b640af
        fabs // 00b640b1
        fstp dword ptr [ebx] // 00b640b3
        fld dword ptr [ebx] // 00b640b7
        fxch // 00b640bb
        fcomi st(0),st(1) // 00b640bd
        fstp st(1) // 00b640bf
        jbe L_00b640d9 // 00b640c1
        fstp st(3) // 00b640c3
        fstp st(1) // 00b640c5
        fstp st(1) // 00b640c7
        mov eax,dword ptr [esp + 0x18] // pure kernel binding metadata
        mov eax,dword ptr [eax + 0xc]
        fmul qword ptr [eax] // 00b640c9
        fadd dword ptr [ebp + 0x10] // 00b640cf
        fstp dword ptr [ebp + 0x10] // 00b640d3
        jmp L_00b64113 // 00b640d7
    L_00b640d9:
        fstp st(1) // 00b640d9
        fld st(2) // 00b640db
        fabs // 00b640dd
        fstp dword ptr [ebx] // 00b640df
        fld dword ptr [ebx] // 00b640e3
        fcomi st(0),st(2) // 00b640e7
        fstp st(2) // 00b640e9
        jbe L_00b6410d // 00b640eb
        fxch // 00b640ed
        fcomip st(0),st(1) // 00b640ef
        fstp st(0) // 00b640f1
        jbe L_00b64111 // 00b640f3
        mov eax,dword ptr [esp + 0x18] // pure kernel binding metadata
        mov eax,dword ptr [eax + 0xc]
        fmul qword ptr [eax] // 00b640f5
        fadd dword ptr [ebp + 0x10] // 00b640fb
        fstp dword ptr [ebp + 0x10] // 00b640ff
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
        lea eax,[ebp + 0x4] // 00b64113
        push eax // 00b64117
        lea edx,[ebp + 0x10] // 00b64118
        lea ecx,[ebp + 0x1c] // 00b6411c
        call camera_vector_cross_004f9b30 // 00b64120
        lea ecx,[ebp + 0x1c] // 00b64125
        mov edx,dword ptr [esp + 0x18] // pure CRT binding metadata
        mov edx,dword ptr [edx]
        call camera_vector_length_00419440 // 00b64129
        fstp dword ptr [ebx] // 00b6412e
        fldz // 00b64132
        fld dword ptr [ebx] // 00b64134
        fcomi st(0),st(1) // 00b64138
        fstp st(1) // 00b6413a
        jbe L_00b64148 // 00b6413c
        fld1 // 00b6413e
        fdivrp st(1),st(0) // 00b64140
        fstp dword ptr [ebx] // 00b64142
        jmp L_00b64153 // 00b64146
    L_00b64148:
        xorps xmm0,xmm0 // 00b64148
        fstp st(0) // 00b6414b
        movss dword ptr [ebx],xmm0 // 00b6414d
    L_00b64153:
        fld dword ptr [ebx] // 00b64153
        lea ecx,[ebp + 0x1c] // 00b64157
        fld st(0) // 00b6415b
        push ecx // 00b6415d
        fmul dword ptr [ebp + 0x1c] // 00b6415e
        lea edx,[ebp + 0x4] // 00b64162
        lea ecx,[ebp + 0x10] // 00b64166
        fstp dword ptr [ebx + 0x4] // 00b6416a
        fld st(0) // 00b6416e
        fmul dword ptr [ebp + 0x20] // 00b64170
        fstp dword ptr [ebx + 0x8] // 00b64174
        fmul dword ptr [ebp + 0x24] // 00b64178
        fstp dword ptr [ebx + 0xc] // 00b6417c
        fld dword ptr [ebx + 0x4] // 00b64180
        fstp dword ptr [ebp + 0x1c] // 00b64184
        fld dword ptr [ebx + 0x8] // 00b64188
        fstp dword ptr [ebp + 0x20] // 00b6418c
        fld dword ptr [ebx + 0xc] // 00b64190
        fstp dword ptr [ebp + 0x24] // 00b64194
        call camera_vector_cross_004f9b30 // 00b64198
        lea ecx,[ebp + 0x10] // 00b6419d
        mov edx,dword ptr [esp + 0x18] // pure CRT binding metadata
        mov edx,dword ptr [edx]
        call camera_vector_length_00419440 // 00b641a1
        fstp dword ptr [ebx] // 00b641a6
        fldz // 00b641aa
        xorps xmm0,xmm0 // 00b641ac
        fld dword ptr [ebx] // 00b641af
        fcomi st(0),st(1) // 00b641b3
        fstp st(1) // 00b641b5
        jbe L_00b641c3 // 00b641b7
        fld1 // 00b641b9
        fdivrp st(1),st(0) // 00b641bb
        fstp dword ptr [ebx] // 00b641bd
        jmp L_00b641cb // 00b641c1
    L_00b641c3:
        fstp st(0) // 00b641c3
        movss dword ptr [ebx],xmm0 // 00b641c5
    L_00b641cb:
        fld dword ptr [ebx] // 00b641cb
        movss xmm1,dword ptr [ebx + 0x4] // 00b641cf
        fld st(0) // 00b641d5
        movss dword ptr [esi],xmm1 // 00b641d7
        fmul dword ptr [ebp + 0x10] // 00b641db
        movss xmm1,dword ptr [ebp + 0x4] // 00b641df
        movss dword ptr [esi + 0x8],xmm1 // 00b641e5
        movss xmm1,dword ptr [ebx + 0x8] // 00b641ea
        fstp dword ptr [ebp + 0x1c] // 00b641f0
        movss dword ptr [esi + 0x10],xmm1 // 00b641f4
        movss xmm1,dword ptr [ebp + 0x8] // 00b641f9
        fld st(0) // 00b641ff
        movss dword ptr [esi + 0x18],xmm1 // 00b64201
        fmul dword ptr [ebp + 0x14] // 00b64206
        movss xmm1,dword ptr [ebx + 0xc] // 00b6420a
        movss dword ptr [esi + 0xc],xmm0 // 00b64210
        movss dword ptr [esi + 0x1c],xmm0 // 00b64215
        fstp dword ptr [ebp + 0x20] // 00b6421a
        movss dword ptr [esi + 0x20],xmm1 // 00b6421e
        movss xmm1,dword ptr [ebp + 0xc] // 00b64223
        fmul dword ptr [ebp + 0x18] // 00b64229
        movss dword ptr [esi + 0x2c],xmm0 // 00b6422d
        mov eax,dword ptr [esp + 0x18] // pure kernel binding metadata
        mov eax,dword ptr [eax + 0x10]
        movss xmm0,dword ptr [eax] // 00b64232
        fstp dword ptr [ebp + 0x24] // 00b6423b
        movss dword ptr [esi + 0x28],xmm1 // 00b6423f
        fld dword ptr [ebp + 0x1c] // 00b64244
        movss dword ptr [esi + 0x3c],xmm0 // 00b64248
        fst dword ptr [esi + 0x4] // 00b6424d
        mov eax,esi // 00b64250
        fld dword ptr [ebp + 0x20] // 00b64252
        fst dword ptr [esi + 0x14] // 00b64256
        fld dword ptr [ebp + 0x24] // 00b64259
        fstp dword ptr [esi + 0x24] // 00b6425d
        fld dword ptr [ebx + 0x8] // 00b64260
        fld dword ptr [ebp + 0x2c] // 00b64264
        fld st(0) // 00b64268
        fmulp st(2),st(0) // 00b6426a
        fld dword ptr [ebx + 0x4] // 00b6426c
        fld dword ptr [ebp + 0x28] // 00b64270
        fld st(0) // 00b64274
        fmulp st(2),st(0) // 00b64276
        fxch st(3) // 00b64278
        faddp st(1),st(0) // 00b6427a
        fld dword ptr [ebx + 0xc] // 00b6427c
        fld dword ptr [ebp + 0x30] // 00b64280
        fld st(0) // 00b64284
        fmulp st(2),st(0) // 00b64286
        fxch st(2) // 00b64288
        faddp st(1),st(0) // 00b6428a
        fstp dword ptr [ebx] // 00b6428c
        fld dword ptr [ebx] // 00b64290
        fchs // 00b64294
        fstp dword ptr [esi + 0x30] // 00b64296
        fld st(1) // 00b64299
        fmulp st(4),st(0) // 00b6429b
        fld st(2) // 00b6429d
        fmulp st(5),st(0) // 00b6429f
        fxch st(3) // 00b642a1
        faddp st(4),st(0) // 00b642a3
        fld dword ptr [ebp + 0x24] // 00b642a5
        fmul st(0),st(3) // 00b642a9
        faddp st(4),st(0) // 00b642ab
        fxch st(3) // 00b642ad
        fstp dword ptr [ebx] // 00b642af
        fld dword ptr [ebx] // 00b642b3
        fchs // 00b642b7
        fstp dword ptr [esi + 0x34] // 00b642b9
        fld dword ptr [ebp + 0x8] // 00b642bc
        fmulp st(3),st(0) // 00b642c0
        fmul dword ptr [ebp + 0x4] // 00b642c2
        faddp st(2),st(0) // 00b642c6
        fmul dword ptr [ebp + 0xc] // 00b642c8
        faddp st(1),st(0) // 00b642cc
        fstp dword ptr [ebx] // 00b642ce
        fld dword ptr [ebx] // 00b642d2
        fchs // 00b642d6
        fstp dword ptr [esi + 0x38] // 00b642d8
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 8
    }
}
} // namespace

void* build_native_camera_look_at_00b63f10(void* output, const void* eye,
    const NativeCameraLookAtFrame& frame, const NativeCameraLookAtBindings& bindings) {
    require_crt(bindings); // pure source-interface admission before native work
    const KernelBindings kernel{&bindings.crt, &bindings.negative_zero_00d7a208,
        &bindings.parallel_threshold_00d62ba0, &bindings.up_perturbation_00d7a3a0,
        &bindings.one_00d7a24c};
    return look_at_raw_kernel(output, eye, &frame, &kernel);
}

void set_native_raw_world_position_00b6dae0(void* actual,
    volatile Word& position_argument, NativeCameraPoseDispatch& calls) {
    const Word source = position_argument; // B6DAE0
    volatile Word* argument = &position_argument;
    NativeCameraPoseDispatch* dispatch = &calls;
    Word target;
    __asm {
        mov esi,actual
        mov eax,source
        fld dword ptr [eax]
        fstp dword ptr [esi + 0x120]
        lea edx,[esi + 0xf0]
        mov ecx,argument
        mov dword ptr [ecx],edx // B6DAF2 before y/z reads
        fld dword ptr [eax + 4]
        fstp dword ptr [esi + 0x124]
        fld dword ptr [eax + 8]
        push dispatch
        push esi
        call capture_world_target // B6DB02/B6DB04, z remains in ST0
        add esp,8
        mov target,eax
        fstp dword ptr [esi + 0x128] // B6DB07 after captured target
    }
    calls.invoke_virtual34(target, actual, position_argument); // B6DB0D tail projection
}

void set_native_raw_camera_position_00b71400(void* actual,
    const volatile Word& position_argument, const NativeCameraPositionFrame& frame,
    NativeCameraPoseDispatch& calls) {
    const Word source = position_argument; // before2F0 clear
    const Word camera = bits(actual);
    write(camera, 0x2f0, read(camera, 0x2f0) & 0xfffffe4bu);
    *frame.base_argument = source; // actual pushed cell
    set_native_raw_world_position_00b6dae0(actual, *frame.base_argument, calls); // B71412
    refresh_native_raw_camera_direction_00b70660(actual, frame.direction); // B71419
}

void set_native_camera_look_at_storage_00b700e0(void* actual,
    const volatile Word& eye_argument, const volatile Word& target_argument,
    const NativeCameraPoseFrame& frame, NativeCameraPoseDispatch& calls,
    const NativeCameraLookAtBindings& bindings) {
    require_crt(bindings); // pure source-interface admission
    const Word eye = eye_argument; // captured once at B700E7
    const Word camera = bits(actual);
    const Word initial_profile = read(camera);
    const Word position_target = calls.resolve_profile(initial_profile)[0x30 / 4];
    write(camera, 0x2f0, read(camera, 0x2f0) & 0xfffffe4bu);
    *frame.position_argument = eye;
    calls.invoke_virtual30(position_target, actual, *frame.position_argument); // B70102
    const Word current_target = target_argument; // B70104, after callback
    volatile Word* locals = frame.locals;
    volatile Word* arguments = frame.builder.arguments;
    const CameraAxesCrtAccess* crt = &bindings.crt;
    const volatile Word* one = &bindings.one_00d7a24c;
    __asm {
        mov esi,actual
        mov eax,current_target
        lea edi,[esi + 0x1a0]
        fld dword ptr [eax]
        fstp dword ptr [edi]
        fld dword ptr [eax + 4]
        fstp dword ptr [edi + 4]
        fld dword ptr [eax + 8]
        fstp dword ptr [edi + 8]
        mov ebx,locals
        mov edx,eye
        fld dword ptr [edi]
        fsub dword ptr [edx]
        fstp dword ptr [ebx + 4]
        fld dword ptr [edi + 4]
        fsub dword ptr [edx + 4]
        fstp dword ptr [ebx + 8]
        fld dword ptr [edi + 8]
        fsub dword ptr [edx + 8]
        fstp dword ptr [ebx + 0x0c]
        lea ecx,[ebx + 4]
        mov edx,crt
        call camera_vector_length_00419440 // B70141
        xorps xmm0,xmm0
        fstp dword ptr [ebx]
        fldz
        fld dword ptr [ebx]
        fcomi st(0),st(1)
        fstp st(1)
        jbe nonpositive
        fld1
        fdivrp st(1),st(0)
        fstp dword ptr [ebx]
        jmp normalized
nonpositive:
        fstp st(0)
        movss dword ptr [ebx],xmm0
normalized:
        fld dword ptr [ebx + 4]
        mov eax,one
        movss xmm1,dword ptr [eax] // B7016F current one with ST0 live
        fld dword ptr [ebx]
        fld st(0)
        mov eax,arguments
        fmulp st(2),st(0)
        mov dword ptr [eax],edi // actual PUSH target at B70184
        fxch
        fstp dword ptr [ebx + 0x10]
        fld dword ptr [ebx + 8]
        fmul st(0),st(1)
        fstp dword ptr [ebx + 0x14]
        fmul dword ptr [ebx + 0x0c]
        fstp dword ptr [ebx + 0x18]
        fld dword ptr [ebx + 0x10]
        fstp dword ptr [esi + 0x1ac]
        fld dword ptr [ebx + 0x14]
        fstp dword ptr [esi + 0x1b0]
        fld dword ptr [ebx + 0x18]
        fstp dword ptr [esi + 0x1b4]
        movss dword ptr [eax + 4],xmm0
        movss dword ptr [eax + 8],xmm1
        movss dword ptr [eax + 0x0c],xmm0
    }
    void* view = build_native_camera_look_at_00b63f10(
        const_cast<Word*>(locals + 7), reinterpret_cast<void*>(eye), frame.builder, bindings); // B701CF
    const Word final_profile = read(camera); // B701D4 before inverse
    const volatile Word* final_table = calls.resolve_profile(final_profile);
    void* world = invert_native_camera_scaled_affine_00b63b30(
        const_cast<Word*>(locals + 23), view); // B701DC
    *frame.world_argument = bits(world); // B701E1 before slot34 read
    const Word world_target = final_table[0x34 / 4]; // B701E2 captured table
    calls.invoke_virtual34(world_target, actual, *frame.world_argument); // B701E7
}
} // namespace bsp
