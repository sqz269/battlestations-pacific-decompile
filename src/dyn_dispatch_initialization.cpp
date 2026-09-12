#include "bsp/dyn_dispatch_initialization.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This recovered x87 operation schedule requires MSVC Win32.
#endif

namespace bsp {
namespace {
const double negative_one = -1.0; // Exact 00D7A250 bytes 000000000000F0BF.
DynDispatchGlobalsStorage* bound_globals = nullptr;

DynDispatchGlobalsStorage& actual_globals() {
    if (!bound_globals) throw std::logic_error("Dyn dispatch globals are not bound");
    return *bound_globals;
}

// The additional borrowed CRT input is preserved in EBX. All original x87
// instructions and double stores remain in order; ECX binds the shared CRT
// forwarder without spilling ST0. Native global addresses become EDI offsets.
__declspec(naked) DynGeneralConvexIntersectStorage* __fastcall construct_kernel(
    DynGeneralConvexIntersectStorage*, const void*, const CameraAxesCrtAccess*) {
    __asm {
        push ebp
        mov ebp,esp
        and esp,0fffffff8h
        push ebx
        push esi
        push edi
        sub esp,4
        mov edi,ecx
        mov ebx,[ebp + 8]
        push 2710h // 00c48fd8
        lea eax,[edi + 278h]
        push eax // 00c48fdd
        mov [edi],edx // 00c48fe2: publish before OS call
        call InitializeCriticalSectionAndSpinCount // 00c48fec, BOOL ignored
        fldz // 00c48ff2
        fst qword ptr [edi + 08h] // 00c48ff4
        fld1 // 00c48ffa
        fst qword ptr [edi + 010h] // 00c48ffc
        fxch // 00c49002
        fst qword ptr [edi + 018h] // 00c49004
        fst qword ptr [edi + 020h] // 00c4900a
        fld qword ptr negative_one // 00c49010
        fst qword ptr [edi + 028h] // 00c49016
        fst qword ptr [edi + 070h] // 00c4901c
        fst qword ptr [edi + 088h] // 00c49022
        fst qword ptr [edi + 090h] // 00c49028
        fst qword ptr [edi + 0A8h] // 00c4902e
        fst qword ptr [edi + 0C0h] // 00c49034
        fst qword ptr [edi + 0130h] // 00c4903a
        fst qword ptr [edi + 0148h] // 00c49040
        fst qword ptr [edi + 0160h] // 00c49046
        fst qword ptr [edi + 0168h] // 00c4904c
        fst qword ptr [edi + 0180h] // 00c49052
        fst qword ptr [edi + 0198h] // 00c49058
        fst qword ptr [edi + 01A0h] // 00c4905e
        fst qword ptr [edi + 01B8h] // 00c49064
        fst qword ptr [edi + 01D0h] // 00c4906a
        fst qword ptr [edi + 01E8h] // 00c49070
        fst qword ptr [edi + 0200h] // 00c49076
        fst qword ptr [edi + 0208h] // 00c4907c
        fst qword ptr [edi + 0218h] // 00c49082
        fst qword ptr [edi + 0220h] // 00c49088
        fst qword ptr [edi + 0230h] // 00c4908e
        fst qword ptr [edi + 0238h] // 00c49094
        fst qword ptr [edi + 0240h] // 00c4909a
        fst qword ptr [edi + 0248h] // 00c490a0
        fst qword ptr [edi + 0258h] // 00c490a6
        fst qword ptr [edi + 0260h] // 00c490ac
        fstp qword ptr [edi + 0270h] // 00c490b2
        fst qword ptr [edi + 030h] // 00c490b8
        fst qword ptr [edi + 038h] // 00c490be
        fst qword ptr [edi + 050h] // 00c490c4
        fst qword ptr [edi + 058h] // 00c490ca
        fst qword ptr [edi + 068h] // 00c490d0
        fst qword ptr [edi + 080h] // 00c490d6
        fst qword ptr [edi + 098h] // 00c490dc
        fst qword ptr [edi + 0A0h] // 00c490e2
        fst qword ptr [edi + 0B0h] // 00c490e8
        fst qword ptr [edi + 0D0h] // 00c490ee
        fst qword ptr [edi + 0D8h] // 00c490f4
        fst qword ptr [edi + 0F0h] // 00c490fa
        fst qword ptr [edi + 0118h] // 00c49100
        fst qword ptr [edi + 0150h] // 00c49106
        fst qword ptr [edi + 0178h] // 00c4910c
        fst qword ptr [edi + 01A8h] // 00c49112
        fst qword ptr [edi + 01B0h] // 00c49118
        fst qword ptr [edi + 01C8h] // 00c4911e
        fst qword ptr [edi + 01F0h] // 00c49124
        fst qword ptr [edi + 0228h] // 00c4912a
        fstp qword ptr [edi + 0250h] // 00c49130
        fst qword ptr [edi + 040h] // 00c49136
        fst qword ptr [edi + 048h] // 00c4913c
        fst qword ptr [edi + 060h] // 00c49142
        fst qword ptr [edi + 078h] // 00c49148
        fst qword ptr [edi + 0B8h] // 00c4914e
        fst qword ptr [edi + 0C8h] // 00c49154
        fst qword ptr [edi + 0E0h] // 00c4915a
        fst qword ptr [edi + 0E8h] // 00c49160
        fst qword ptr [edi + 0F8h] // 00c49166
        fst qword ptr [edi + 0100h] // 00c4916c
        fst qword ptr [edi + 0108h] // 00c49172
        fst qword ptr [edi + 0110h] // 00c49178
        fst qword ptr [edi + 0120h] // 00c4917e
        fst qword ptr [edi + 0128h] // 00c49184
        fst qword ptr [edi + 0138h] // 00c4918a
        fst qword ptr [edi + 0140h] // 00c49190
        fst qword ptr [edi + 0158h] // 00c49196
        fst qword ptr [edi + 0170h] // 00c4919c
        fst qword ptr [edi + 0188h] // 00c491a2
        fst qword ptr [edi + 0190h] // 00c491a8
        fst qword ptr [edi + 01C0h] // 00c491ae
        lea esi,[edi + 18h] // 00c491b4
        fst qword ptr [edi + 01D8h] // 00c491b9
        fst qword ptr [edi + 01E0h] // 00c491bf
        fst qword ptr [edi + 01F8h] // 00c491c5
        fst qword ptr [edi + 0210h] // 00c491cb
        fstp qword ptr [edi + 0268h] // 00c491d1

L_normalize:
        fld qword ptr [esi - 8] // 00c491d7: y
        fld qword ptr [esi - 10h] // 00c491da: x
        fld qword ptr [esi] // 00c491dd: z
        fld st(1)
        fmulp st(2),st(0)
        fld st(2)
        fmulp st(3),st(0)
        fxch st(1)
        faddp st(2),st(0)
        fmul st(0),st(0)
        faddp st(1),st(0) // 00c491ed: (y*y + x*x) + z*z
        mov ecx,ebx
        call native_crt_sqrt_st0_00bf7030 // 00c491ef: actual shared CRT service
        fld1
        fdivrp st(1),st(0)
        add esi,18h
        lea eax,[edi + 288h]
        cmp esi,eax // 00c491fb, flags survive x87 stores
        fld st(0)
        fmul qword ptr [esi - 28h]
        fstp qword ptr [esi - 28h] // x first
        fld qword ptr [esi - 20h]
        fmul st(0),st(1)
        fstp qword ptr [esi - 20h] // then y
        fmul qword ptr [esi - 18h]
        fstp qword ptr [esi - 18h] // then z, pop reciprocal
        jl L_normalize
        mov eax,edi
        add esp,4
        pop edi
        pop esi
        pop ebx
        mov esp,ebp
        pop ebp
        ret 4 // New explicit CRT argument, not original ABI.
    }
}
} // namespace

void bind_dyn_dispatch_static_objects(DynDispatchGlobalsStorage& state,
    const DynDispatchVtables& tables) {
    if (!tables.box_box_00d7a184 || !tables.terrain_convex_mesh_00d7a18c ||
        !tables.sphere_sphere_00d7a1cc || !tables.box_sphere_00d7a1d4 ||
        !tables.general_convex_00d7a1a8 || !tables.convex_ray_00d7a1f4 ||
        !tables.box_ray_00d7a1fc || !tables.sphere_ray_00d7a204)
        throw std::invalid_argument("Complete actual Dyn dispatch vtables are required");
    state.box_box.vtable = tables.box_box_00d7a184; // PE 00E17434
    state.terrain_convex_mesh.vtable = tables.terrain_convex_mesh_00d7a18c; // 00E17438
    state.sphere_sphere.vtable = tables.sphere_sphere_00d7a1cc; // 00E17440
    state.box_sphere.vtable = tables.box_sphere_00d7a1d4; // 00E17444
    state.convex_ray.vtable = tables.convex_ray_00d7a1f4; // 00E17448
    state.box_ray.vtable = tables.box_ray_00d7a1fc; // 00E174E8
    state.sphere_ray.vtable = tables.sphere_ray_00d7a204; // 00E174EC
}

DynSceneDispatchObjects dyn_scene_dispatch_objects(DynDispatchGlobalsStorage& state) noexcept {
    return {&state.box_box, &state.terrain_convex_mesh, &state.sphere_sphere,
        &state.box_sphere, &state.general_convex, &state.convex_ray,
        &state.box_ray, &state.sphere_ray};
}

DynGeneralConvexIntersectStorage* dyn_general_convex_construct_00c48fd0(
    DynGeneralConvexIntersectStorage& state, const void* table,
    const CameraAxesCrtAccess& crt) {
    if (!table || !crt.dispatch_bypass_0109dd78 || !crt.except_00c27489)
        throw std::invalid_argument("Actual GeneralConvex vtable and CRT service are required");
    return construct_kernel(&state, table, &crt);
}

void dyn_general_convex_destroy_00cd91d0(DynGeneralConvexIntersectStorage& state) noexcept {
    ::DeleteCriticalSection(&state.critical_section_278);
}

void bind_static_dyn_dispatch_globals_0109ea48(DynDispatchGlobalsStorage& state) {
    if (bound_globals && bound_globals != &state)
        throw std::logic_error("Dyn dispatch globals already have a different owner");
    bound_globals = &state;
}

int initialize_static_dyn_dispatch_00cc8950(const void* table,
    const CameraAxesCrtAccess& crt, DynDispatchAtexit register_atexit) {
    if (!register_atexit) throw std::invalid_argument("Actual CRT atexit registration is required");
    dyn_general_convex_construct_00c48fd0(actual_globals().general_convex, table, crt);
    // Native registration failure leaves the initialized object and lock alive.
    return register_atexit(&destroy_static_dyn_dispatch_00cd91d0);
}

void __cdecl destroy_static_dyn_dispatch_00cd91d0() {
    dyn_general_convex_destroy_00cd91d0(actual_globals().general_convex);
}
} // namespace bsp
