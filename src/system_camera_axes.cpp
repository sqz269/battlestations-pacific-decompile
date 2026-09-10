#include "bsp/system_camera_axes.hpp"
#include <cstddef>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This recovered x87 operation schedule requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(CameraAxesCrtException) == 32);
static_assert(offsetof(CameraAxesCrtException, result) == 24);
static_assert(offsetof(CameraAxesCrtAccess, except_00c27489) == 4);
namespace {
const char sqrt_name[] = "sqrt"; // 00E154E0, preserved text; new pointer identity
const double quieting_one = 1.0; // 00D6A670
const unsigned char negative_nan_80[10] = {0,0,0,0,0,0,0,0xc0,0xff,0xff}; // 00E15C70
const float axis_one = 1.0f; // 00D7A24C
const float axis_fallback_threshold = 0.5f; // 00CE3800

// Reassembled instructions preserve x87 operands/spills, flags and CRT branch
// routing. EBX carries the explicit borrowed CRT access through private calls;
// the length entry saves/restores it. No game code is executed by these kernels.
void __cdecl crt_dispatch_kernel();
void __cdecl crt_restore_kernel();
void __cdecl crt_status_kernel();
void __cdecl crt_control_kernel();
void __cdecl crt_nan_kernel();
void __cdecl crt_classify_kernel();
void __cdecl crt_sqrt_body_kernel();
void __cdecl crt_sqrt_kernel();
float* __fastcall axis_normalize_kernel(float*, const float*, const CameraAxesCrtAccess*);

__declspec(naked) void __cdecl crt_dispatch_kernel() {
    __asm {
        push ebp // 00c08347
        mov ebp,esp // 00c08348
        add esp,-0x20 // 00c0834a
        mov dword ptr [ebp + -0x20],eax // 00c0834d
        fstp qword ptr [ebp + -0x8] // 00c08350
        mov dword ptr [ebp + -0x1c],ecx // 00c08353
        mov eax,dword ptr [ebp + 0x10] // 00c08356
        mov ecx,dword ptr [ebp + 0x14] // 00c08359
        mov dword ptr [ebp + -0x18],eax // 00c0835c
        mov dword ptr [ebp + -0x14],ecx // 00c0835f
        lea eax,[ebp + 0x8] // 00c08362
        lea ecx,[ebp + -0x20] // 00c08365
        push eax // 00c08368
        push ecx // 00c08369
        push edx // 00c0836a
        call dword ptr [ebx + 0x4] // 00c0836b
        add esp,0xc // 00c08370
        fld qword ptr [ebp + -0x8] // 00c08373
        cmp word ptr [ebp + 0x8],0x27f // 00c08376
        jz L_00c08381 // 00c0837c
        fldcw word ptr [ebp + 0x8] // 00c0837e
L_00c08381:
        leave // 00c08381
        ret // 00c08382
    }
}

__declspec(naked) void __cdecl crt_restore_kernel() {
    __asm {
        cmp word ptr [esp],0x27f // 00c0842e
        jz L_00c08439 // 00c08434
        fldcw word ptr [esp] // 00c08436
L_00c08439:
        pop edx // 00c08439
        ret // 00c0843a
    }
}

__declspec(naked) void __cdecl crt_status_kernel() {
    __asm {
        mov ax,word ptr [esp] // 00c0843b
        cmp ax,0x27f // 00c0843f
        jz L_00c08463 // 00c08443
        and ax,0x20 // 00c08445
        jz L_00c08460 // 00c08449
        fstsw ax // 00c0844b
        and ax,0x20 // 00c0844e
        jz L_00c08460 // 00c08452
        mov eax,0x8 // 00c08454
        call crt_dispatch_kernel // 00c08459
        pop edx // 00c0845e
        ret // 00c0845f
L_00c08460:
        fldcw word ptr [esp] // 00c08460
L_00c08463:
        pop edx // 00c08463
        ret // 00c08464
    }
}

__declspec(naked) void __cdecl crt_control_kernel() {
    __asm {
        mov edx,dword ptr [esp + 0x4] // 00c083a5
        and edx,0x300 // 00c083a9
        or edx,0x7f // 00c083af
        mov word ptr [esp + 0x6],dx // 00c083b2
        fldcw word ptr [esp + 0x6] // 00c083b7
        ret // 00c083bb
    }
}

__declspec(naked) void __cdecl crt_nan_kernel() {
    __asm {
        test eax,0x80000 // 00c083bc
        jz L_00c083c9 // 00c083c1
        mov eax,0x7 // 00c083c3
        ret // 00c083c8
L_00c083c9:
        fadd qword ptr quieting_one // 00c083c9
        mov eax,0x1 // 00c083cf
        ret // 00c083d4
    }
}

__declspec(naked) void __cdecl crt_classify_kernel() {
    __asm {
        mov eax,dword ptr [esp + 0x8] // 00c08418
        and eax,0x7ff00000 // 00c0841c
        cmp eax,0x7ff00000 // 00c08421
        jz L_00c08429 // 00c08426
        ret // 00c08428
L_00c08429:
        mov eax,dword ptr [esp + 0x8] // 00c08429
        ret // 00c0842d
    }
}

__declspec(naked) void __cdecl crt_sqrt_body_kernel() {
    __asm {
        push edx // 00bf704d
        fstcw word ptr [esp] // 00bf704e
        mov eax,dword ptr [esp + 0xc] // 00bf7052
        jz L_00bf70a9 // 00bf7056
        cmp word ptr [esp],0x27f // 00bf7058
        jz L_00bf7065 // 00bf705e
        call crt_control_kernel // 00bf7060
L_00bf7065:
        test eax,0x80000000 // 00bf7065
        jnz L_00bf708b // 00bf706a
        fsqrt // 00bf706c
L_00bf706e:
        mov edx, dword ptr [ebx] // live actual global0109DD78 slot
        cmp dword ptr [edx],0x0 // 00bf706e
        jz L_fallthrough_00bf7075
        jmp crt_restore_kernel // 00bf7075
L_fallthrough_00bf7075:
        mov edx,0x5 // 00bf707b
        lea ecx,sqrt_name // 00bf7080
        jmp crt_status_kernel // 00bf7086
L_00bf708b:
        test eax,0x7ff00000 // 00bf708b
        jnz L_00bf70be // 00bf7090
        test eax,0xfffff // 00bf7092
        jnz L_00bf70be // 00bf7097
        cmp dword ptr [esp + 0x8],0x0 // 00bf7099
        jnz L_00bf70be // 00bf709e
        jmp L_00bf706e // 00bf70a0
L_00bf70a2:
        call crt_nan_kernel // 00bf70a2
        jmp L_00bf70cb // 00bf70a7
L_00bf70a9:
        test eax,0xfffff // 00bf70a9
        jnz L_00bf70a2 // 00bf70ae
        cmp dword ptr [esp + 0x8],0x0 // 00bf70b0
        jnz L_00bf70a2 // 00bf70b5
        and eax,0x80000000 // 00bf70b7
        jz L_00bf706e // 00bf70bc
L_00bf70be:
        fstp st(0) // 00bf70be
        fld tbyte ptr negative_nan_80 // 00bf70c0
        mov eax,0x1 // 00bf70c6
L_00bf70cb:
        mov edx, dword ptr [ebx] // live actual global0109DD78 slot
        cmp dword ptr [edx],0x0 // 00bf70cb
        jz L_fallthrough_00bf70d2
        jmp crt_restore_kernel // 00bf70d2
L_fallthrough_00bf70d2:
        mov edx,0x5 // 00bf70d8
        lea ecx,sqrt_name // 00bf70dd
        call crt_dispatch_kernel // 00bf70e3
        pop edx // 00bf70e8
        ret // 00bf70e9
    }
}

__declspec(naked) void __cdecl crt_sqrt_kernel() {
    __asm {
        sub esp,0xc // 00bf7030
        fst qword ptr [esp] // 00bf7033
        call crt_classify_kernel // 00bf7036
        call crt_sqrt_body_kernel // 00bf703b
        add esp,0xc // 00bf7040
        ret // 00bf7043
    }
}

} // namespace

__declspec(naked) float __fastcall camera_vector_length_00419440(const float*, const CameraAxesCrtAccess*) {
    __asm {
        push ebx
        mov ebx, edx // explicit borrowed CRT access
        sub esp,0xc // 00419440
        fld dword ptr [ecx] // 00419443
        fstp dword ptr [esp + 0x4] // 00419445
        fld dword ptr [ecx + 0x4] // 00419449
        fstp dword ptr [esp] // 0041944c
        fld dword ptr [ecx + 0x8] // 0041944f
        fstp dword ptr [esp + 0x8] // 00419452
        fld dword ptr [esp] // 00419456
        fld dword ptr [esp + 0x4] // 00419459
        fld dword ptr [esp + 0x8] // 0041945d
        fld st(1) // 00419461
        fmulp st(2),st(0) // 00419463
        fxch // 00419465
        fstp dword ptr [esp + 0x8] // 00419467
        fld dword ptr [esp + 0x8] // 0041946b
        fld st(2) // 0041946f
        fmulp st(3),st(0) // 00419471
        fxch st(2) // 00419473
        fstp dword ptr [esp + 0x8] // 00419475
        fld dword ptr [esp + 0x8] // 00419479
        faddp st(2),st(0) // 0041947d
        fmul st(0),st(0) // 0041947f
        fstp dword ptr [esp + 0x8] // 00419481
        fadd dword ptr [esp + 0x8] // 00419485
        fstp dword ptr [esp + 0x8] // 00419489
        fld dword ptr [esp + 0x8] // 0041948d
        call crt_sqrt_kernel // 00419491
        fstp dword ptr [esp + 0x8] // 00419496
        fld dword ptr [esp + 0x8] // 0041949a
        add esp,0xc // 0041949e
        pop ebx
        ret // 004194a1
    }
}

__declspec(naked) float* __fastcall camera_vector_cross_004f9b30(float*, const float*, const float*) {
    __asm {
        sub esp,0xc // 004f9b30
        fld dword ptr [edx + 0x4] // 004f9b33
        mov eax,ecx // 004f9b36
        mov ecx,dword ptr [esp + 0x10] // 004f9b38
        fstp dword ptr [esp] // 004f9b3c
        fld dword ptr [ecx + 0x8] // 004f9b3f
        fstp dword ptr [esp + 0x10] // 004f9b42
        fld dword ptr [edx + 0x8] // 004f9b46
        fstp dword ptr [esp + 0x8] // 004f9b49
        fld dword ptr [ecx + 0x4] // 004f9b4d
        fstp dword ptr [esp + 0x4] // 004f9b50
        fld dword ptr [esp + 0x10] // 004f9b54
        fld st(0) // 004f9b58
        fld dword ptr [esp] // 004f9b5a
        fld st(0) // 004f9b5d
        fmulp st(2),st(0) // 004f9b5f
        fld dword ptr [esp + 0x4] // 004f9b61
        fld st(0) // 004f9b65
        fld dword ptr [esp + 0x8] // 004f9b67
        fld st(0) // 004f9b6b
        fmulp st(2),st(0) // 004f9b6d
        fxch st(4) // 004f9b6f
        fsubrp st(1),st(0) // 004f9b71
        fstp dword ptr [eax] // 004f9b73
        fld dword ptr [ecx] // 004f9b75
        fstp dword ptr [esp + 0x10] // 004f9b77
        fld dword ptr [edx] // 004f9b7b
        fstp dword ptr [esp + 0x8] // 004f9b7d
        fld dword ptr [esp + 0x10] // 004f9b81
        fld st(0) // 004f9b85
        fmulp st(4),st(0) // 004f9b87
        fld dword ptr [esp + 0x8] // 004f9b89
        fld st(0) // 004f9b8d
        fmulp st(6),st(0) // 004f9b8f
        fxch st(4) // 004f9b91
        fsubrp st(5),st(0) // 004f9b93
        fxch st(4) // 004f9b95
        fstp dword ptr [eax + 0x4] // 004f9b97
        fmulp st(2),st(0) // 004f9b9a
        fmulp st(2),st(0) // 004f9b9c
        fsubrp st(1),st(0) // 004f9b9e
        fstp dword ptr [eax + 0x8] // 004f9ba0
        add esp,0xc // 004f9ba3
        ret 0x4 // 004f9ba6
    }
}

namespace {
__declspec(naked) float* __fastcall axis_normalize_kernel(float*, const float*, const CameraAxesCrtAccess*) {
    __asm {
        push ecx // 00419510
        push esi // 00419511
        push edi // 00419512
        mov esi,edx // 00419513
        mov edi,ecx // 00419515
        mov ecx,esi // 00419517
        mov edx, dword ptr [esp + 0x10] // explicit CRT access stack argument
        call camera_vector_length_00419440 // 00419519
        fstp dword ptr [esp + 0x8] // 0041951e
        fldz // 00419522
        fld dword ptr [esp + 0x8] // 00419524
        fcomi st(0),st(1) // 00419528
        fstp st(1) // 0041952a
        jbe L_00419538 // 0041952c
        fld1 // 0041952e
        fdivrp st(1),st(0) // 00419530
        fstp dword ptr [esp + 0x8] // 00419532
        jmp L_00419543 // 00419536
L_00419538:
        xorps xmm0,xmm0 // 00419538
        fstp st(0) // 0041953b
        movss dword ptr [esp + 0x8],xmm0 // 0041953d
L_00419543:
        fld dword ptr [esi] // 00419543
        mov eax,edi // 00419545
        fld dword ptr [esp + 0x8] // 00419547
        fld st(0) // 0041954b
        fmulp st(2),st(0) // 0041954d
        fxch // 0041954f
        fstp dword ptr [edi] // 00419551
        fld dword ptr [esi + 0x4] // 00419553
        fmul st(0),st(1) // 00419556
        fstp dword ptr [edi + 0x4] // 00419558
        fmul dword ptr [esi + 0x8] // 0041955b
        fstp dword ptr [edi + 0x8] // 0041955e
        pop edi // 00419561
        pop esi // 00419562
        pop ecx // 00419563
        ret 0x4 // 00419564
    }
}

// Native cache stores use x87, unlike the later MOVSS prefix copies. In
// particular these loads/stores may quiet NaNs and set x87 exception flags.
void store_axis(CameraAxis& destination, const CameraAxis& source) {
    float* dst = destination.data();
    const float* src = source.data();
    __asm {
        mov eax, src
        mov edx, dst
        fld dword ptr [eax]
        fstp dword ptr [edx]
        fld dword ptr [eax + 4]
        fstp dword ptr [edx + 4]
        fld dword ptr [eax + 8]
        fstp dword ptr [edx + 8]
    }
}

bool below_fallback_threshold(const CameraAxis& axis, const CameraAxesCrtAccess& crt) {
    const float* value = axis.data();
    const CameraAxesCrtAccess* access = &crt;
    bool result;
    __asm {
        mov ecx, value
        mov edx, access
        call camera_vector_length_00419440
        fld axis_fallback_threshold
        fcomip st(0), st(1)
        fstp st(0)
        seta result // unordered follows native JBE: no fallback
    }
    return result;
}

void refresh_axes(CameraFrameState& frame, const CameraAxesCrtAccess& crt) {
    if (frame.camera.projection.valid_flags & 0x100u) return;
    if (!crt.dispatch_bypass_0109dd78 || !crt.except_00c27489)
        throw std::invalid_argument("Camera axes require actual CRT state and __87except binding");
    auto& transform = frame.camera.transform;
    if (!(transform.valid_flags & 2u)) refresh_camera_world_00b6db70(transform);
    // Stack arguments prove forward x up, then intermediate x forward. No
    // normalization of forward itself occurs before these cross products.
    const CameraAxis forward{transform.world[8], transform.world[9], transform.world[10]};
    const CameraAxis world_up{0.0f, axis_one, 0.0f};
    CameraAxis first, second, normalized;
    camera_vector_cross_004f9b30(first.data(), forward.data(), world_up.data());
    camera_vector_cross_004f9b30(second.data(), first.data(), forward.data());
    axis_normalize_kernel(normalized.data(), second.data(), &crt);
    store_axis(frame.axis_y, normalized);
    camera_vector_cross_004f9b30(normalized.data(), forward.data(), frame.axis_y.data());
    axis_normalize_kernel(first.data(), normalized.data(), &crt);
    store_axis(frame.axis_x, first);
    if (below_fallback_threshold(frame.axis_y, crt)) {
        frame.axis_y = {0.0f, axis_one, 0.0f};
        frame.axis_x = {axis_one, 0.0f, 0.0f};
    }
    frame.camera.projection.valid_flags |= 0x100u;
}
} // namespace

const CameraAxis& get_camera_axis_y_00b70ea0(CameraFrameState& frame, const CameraAxesCrtAccess& crt) {
    refresh_axes(frame, crt);
    return frame.axis_y;
}
const CameraAxis& get_camera_axis_x_00b70fe0(CameraFrameState& frame, const CameraAxesCrtAccess& crt) {
    refresh_axes(frame, crt);
    return frame.axis_x;
}

FrameClock* write_system_camera_axes_00b46c50(CameraFrameState& frame, float* prefix,
    std::size_t capacity, FrameClock* const volatile& global_01090ab0,
    const CameraAxesCrtAccess& crt) {
    if (!prefix || capacity < 131)
        throw std::invalid_argument("Camera axis prefix requires 131 initialized float words");
    const float* x = get_camera_axis_x_00b70fe0(frame, crt).data();
    __asm {
        mov eax, x
        mov edx, prefix
        movss xmm0, dword ptr [eax]
        movss dword ptr [edx + 496], xmm0 // c31.x; 00B46C59
        movss xmm0, dword ptr [eax + 4]
        movss dword ptr [edx + 500], xmm0
        movss xmm0, dword ptr [eax + 8]
        movss dword ptr [edx + 504], xmm0
    }
    const float* y = get_camera_axis_y_00b70ea0(frame, crt).data();
    FrameClock* const volatile* timer_slot = &global_01090ab0;
    FrameClock* captured;
    __asm {
        mov eax, y
        mov edx, prefix
        mov ecx, timer_slot
        movss xmm0, dword ptr [eax] // 00B46C85: load Y.x first
        mov ecx, dword ptr [ecx] // 00B46C89: capture actual timer, no call
        movss dword ptr [edx + 512], xmm0 // c32.x; 00B46C8F
        movss xmm0, dword ptr [eax + 4]
        movss dword ptr [edx + 516], xmm0
        movss xmm0, dword ptr [eax + 8]
        movss dword ptr [edx + 520], xmm0
        mov captured, ecx
    }
    return captured;
}
} // namespace bsp
