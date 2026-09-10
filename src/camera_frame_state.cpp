#include "bsp/camera_frame_state.hpp"
#include "bsp/camera_inverse.hpp"
#include "bsp/camera_multiply.hpp"
#include "bsp/native_viewport_owner.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(CameraPlaneRecord) == 0x14 && sizeof(CameraPlaneSet) == 0x144);
static_assert(offsetof(CameraPlaneSet, count) == 0x140);

CameraViewport::CameraViewport() noexcept
    : native_owner(nullptr), x(owned_.x), y(owned_.y), width(owned_.width),
      height(owned_.height), scissor_enabled(owned_.scissor_enabled), scissor(owned_.scissor) {}
CameraViewport::CameraViewport(DWORD x_value, DWORD y_value, DWORD width_value,
    DWORD height_value, std::uint8_t enabled_value, RECT rectangle) noexcept : CameraViewport() {
    x = x_value; y = y_value; width = width_value; height = height_value;
    scissor_enabled = enabled_value; scissor = rectangle;
}
CameraViewport::CameraViewport(NativeViewportOwner& value) noexcept
    : native_owner(&value), x(value.fields_08.x), y(value.fields_08.y),
      width(value.fields_08.width), height(value.fields_08.height),
      scissor_enabled(value.fields_08.scissor_enabled), scissor(value.fields_08.scissor) {}
CameraViewport::CameraViewport(const CameraViewport& value) noexcept : CameraViewport() {
    *this = value;
}
CameraViewport& CameraViewport::operator=(const CameraViewport& value) noexcept {
    if (this != &value) {
        x = value.x; y = value.y; width = value.width; height = value.height;
        scissor_enabled = value.scissor_enabled; scissor = value.scissor;
    }
    return *this;
}
CameraViewportSlot::CameraViewportSlot(const CameraViewport*& slot) noexcept : diagnostic_(&slot) {}
CameraViewportSlot::CameraViewportSlot(NativeViewportOwner*& slot,
    CameraViewportResolver& resolver) noexcept : native_(&slot), resolver_(&resolver) {}
const CameraViewport* CameraViewportSlot::get() const {
    if (diagnostic_) return *diagnostic_;
    auto* const owner = *native_;
    if (!owner) return nullptr;
    const auto* const view = resolver_->resolve_viewport(owner);
    if (!view || view->native_owner != owner)
        throw std::logic_error("camera viewport: actual owner has no matching stable view");
    return view;
}
CameraViewportSlot& CameraViewportSlot::operator=(const CameraViewport* value) {
    if (!diagnostic_)
        throw std::logic_error("camera viewport: use the native owner retention setter for publication");
    *diagnostic_ = value;
    return *this;
}
CameraFrameState::CameraFrameState(CameraState& value) noexcept
    : camera(value), byte_174(owned_.byte_174), enabled(owned_.enabled),
      viewport(owned_.viewport), fog_184(owned_.fog_184), clear_flags(owned_.clear_flags),
      clear_depth(owned_.clear_depth), clear_color(owned_.clear_color), clear_stencil(owned_.clear_stencil),
      render_mode(owned_.render_mode), inverse_view_projection(owned_.inverse_view_projection),
      frustum(owned_.frustum), context_depth_scale_43c(owned_.context_depth_scale_43c),
      axis_y(owned_.axis_y), axis_x(owned_.axis_x) {}
CameraFrameState::CameraFrameState(CameraState& value, CameraFrameBacking backing) noexcept
    : camera(value), byte_174(backing.byte_174), enabled(backing.enabled),
      viewport(backing.viewport), fog_184(backing.fog_184), clear_flags(backing.clear_flags),
      clear_depth(backing.clear_depth), clear_color(backing.clear_color), clear_stencil(backing.clear_stencil),
      render_mode(backing.render_mode), inverse_view_projection(backing.inverse_view_projection),
      frustum(backing.frustum), context_depth_scale_43c(backing.context_depth_scale_43c),
      axis_y(backing.axis_y), axis_x(backing.axis_x) {}

namespace {
const std::uint32_t negative_zero_bits = 0x80000000u;
const float positive_one = 1.0f;
// Reassembled private kernels preserve native spills, x87/SSE arithmetic and
// branch ordering. Every absolute data operand is a verified typed constant.
// No original game address is executed by this implementation.

__declspec(naked) std::int32_t __cdecl x87_truncate_kernel() {
    __asm {
        push ebp // 00bf7456
        mov ebp, esp // 00bf7457
        sub esp, 0x20 // 00bf7459
        and esp, 0xfffffff0 // 00bf745c
        fld st(0) // 00bf745f
        fst dword ptr [esp + 0x18] // 00bf7461
        fistp qword ptr [esp + 0x10] // 00bf7465
        fild qword ptr [esp + 0x10] // 00bf7469
        mov edx, dword ptr [esp + 0x18] // 00bf746d
        mov eax, dword ptr [esp + 0x10] // 00bf7471
        test eax, eax // 00bf7475
        je L_00bf74b5 // 00bf7477
L_00bf7479:
        fsubp st(1), st(0) // 00bf7479
        test edx, edx // 00bf747b
        jns L_00bf749d // 00bf747d
        fstp dword ptr [esp] // 00bf747f
        mov ecx, dword ptr [esp] // 00bf7482
        xor ecx, 0x80000000 // 00bf7485
        add ecx, 0x7fffffff // 00bf748b
        adc eax, 0 // 00bf7491
        mov edx, dword ptr [esp + 0x14] // 00bf7494
        adc edx, 0 // 00bf7498
        jmp L_00bf74c9 // 00bf749b
L_00bf749d:
        fstp dword ptr [esp] // 00bf749d
        mov ecx, dword ptr [esp] // 00bf74a0
        add ecx, 0x7fffffff // 00bf74a3
        sbb eax, 0 // 00bf74a9
        mov edx, dword ptr [esp + 0x14] // 00bf74ac
        sbb edx, 0 // 00bf74b0
        jmp L_00bf74c9 // 00bf74b3
L_00bf74b5:
        mov edx, dword ptr [esp + 0x14] // 00bf74b5
        test edx, 0x7fffffff // 00bf74b9
        jne L_00bf7479 // 00bf74bf
        fstp dword ptr [esp + 0x18] // 00bf74c1
        fstp dword ptr [esp + 0x18] // 00bf74c5
L_00bf74c9:
        leave  // 00bf74c9
        ret  // 00bf74ca
    }
}

__declspec(naked) float* __fastcall inverse_kernel(float*, const float*) {
    __asm {
        sub esp, 0xb0 // 00b632d0
        xorps xmm0, xmm0 // 00b632d6
        push ebx // 00b632d9
        mov ebx, ecx // 00b632da
        push ebp // 00b632dc
        movss xmm1, dword ptr [positive_one] // 00b632dd
        push esi // 00b632e5
        movss xmm5, dword ptr [negative_zero_bits] // 00b632e6
        mov esi, edx // 00b632ee
        lea edx, [esp + 0x50] // 00b632f0
        mov eax, edx // 00b632f4
        mov dword ptr [esp + 0x4c], eax // 00b632f6
        mov eax, 0x2c // 00b632fa
        sub eax, ebx // 00b632ff
        mov dword ptr [esp + 0x98], eax // 00b63301
        lea eax, [esp + 0x54] // 00b63308
        sub eax, ebx // 00b6330c
        mov dword ptr [esp + 0xb8], eax // 00b6330e
        lea eax, [esp + 0x58] // 00b63315
        sub eax, ebx // 00b63319
        mov dword ptr [esp + 0x94], eax // 00b6331b
        push edi // 00b63322
        mov eax, 0xfffffffc // 00b63323
        sub eax, ebx // 00b63328
        mov ecx, 0x10 // 00b6332a
        lea edi, [esp + 0x54] // 00b6332f
        rep movsd  // 00b63333
        mov dword ptr [esp + 0xb4], eax // 00b63335
        xor edi, edi // 00b6333c
        mov ebp, edx // 00b6333e
        sub ebp, ebx // 00b63340
        mov eax, 4 // 00b63342
        mov esi, edx // 00b63347
        lea ecx, [ebx + 4] // 00b63349
        sub eax, esi // 00b6334c
        movss dword ptr [ebx], xmm1 // 00b6334e
        movss dword ptr [ecx], xmm0 // 00b63352
        movss dword ptr [ebx + 8], xmm0 // 00b63356
        movss dword ptr [ebx + 0xc], xmm0 // 00b6335b
        movss dword ptr [ebx + 0x10], xmm0 // 00b63360
        movss dword ptr [ebx + 0x14], xmm1 // 00b63365
        movss dword ptr [ebx + 0x18], xmm0 // 00b6336a
        movss dword ptr [ebx + 0x1c], xmm0 // 00b6336f
        movss dword ptr [ebx + 0x20], xmm0 // 00b63374
        movss dword ptr [ebx + 0x24], xmm0 // 00b63379
        movss dword ptr [ebx + 0x28], xmm1 // 00b6337e
        movss dword ptr [ebx + 0x2c], xmm0 // 00b63383
        movss dword ptr [ebx + 0x30], xmm0 // 00b63388
        movss dword ptr [ebx + 0x34], xmm0 // 00b6338d
        movss dword ptr [ebx + 0x38], xmm0 // 00b63392
        movss dword ptr [ebx + 0x3c], xmm1 // 00b63397
        mov dword ptr [esp + 0x20], edi // 00b6339c
        mov dword ptr [esp + 0x1c], edi // 00b633a0
        mov dword ptr [esp + 0x24], edx // 00b633a4
        mov dword ptr [esp + 0x4c], 3 // 00b633a8
        mov dword ptr [esp + 0x94], ebp // 00b633b0
        mov dword ptr [esp + 0xac], eax // 00b633b7
        jmp L_00b633c4 // 00b633be
L_00b633c0:
        mov edi, dword ptr [esp + 0x20] // 00b633c0
L_00b633c4:
        cmp dword ptr [esp + 0x4c], 4 // 00b633c4
        lea esi, [edi + 1] // 00b633c9
        mov dword ptr [esp + 0xa4], esi // 00b633cc
        jl L_00b635b0 // 00b633d3
        mov eax, dword ptr [esp + 0x9c] // 00b633d9
        mov ebp, dword ptr [esp + 0x1c] // 00b633e0
        mov edx, dword ptr [esp + 0xb4] // 00b633e4
        add eax, ecx // 00b633eb
        mov dword ptr [esp + 0x48], eax // 00b633ed
        mov eax, dword ptr [esp + 0xac] // 00b633f1
        add eax, ebp // 00b633f8
        mov ebp, dword ptr [esp + 0x48] // 00b633fa
        add edx, ecx // 00b633fe
        add eax, dword ptr [esp + 0x20] // 00b63400
        lea eax, [esp + eax + 0x54] // 00b63404
        lea eax, [esp + eax*4 + 0x64] // 00b63408
        mov dword ptr [esp + 0x10], eax // 00b6340c
L_00b63410:
        mov eax, dword ptr [esp + 0x10] // 00b63410
        movss xmm1, dword ptr [eax - 0x10] // 00b63414
        comiss xmm1, xmm0 // 00b63419
        jbe L_00b63426 // 00b6341c
        movss dword ptr [esp + 0x14], xmm1 // 00b6341e
        jmp L_00b63433 // 00b63424
L_00b63426:
        movaps xmm2, xmm5 // 00b63426
        subss xmm2, xmm1 // 00b63429
        movss dword ptr [esp + 0x14], xmm2 // 00b6342d
L_00b63433:
        mov eax, dword ptr [esp + 0x1c] // 00b63433
        add eax, edx // 00b63437
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b63439
        comiss xmm1, xmm0 // 00b6343f
        lea eax, [esp + eax + 0x54] // 00b63442
        jbe L_00b63450 // 00b63446
        movss dword ptr [esp + 0x18], xmm1 // 00b63448
        jmp L_00b6345d // 00b6344e
L_00b63450:
        movaps xmm2, xmm5 // 00b63450
        subss xmm2, xmm1 // 00b63453
        movss dword ptr [esp + 0x18], xmm2 // 00b63457
L_00b6345d:
        fld dword ptr [esp + 0x18] // 00b6345d
        fld dword ptr [esp + 0x14] // 00b63461
        fcomip st(0), st(1) // 00b63465
        fstp st(0) // 00b63467
        jbe L_00b63470 // 00b63469
        mov edi, esi // 00b6346b
        lea edx, [ebp - 0x20] // 00b6346d
L_00b63470:
        mov eax, dword ptr [esp + 0x10] // 00b63470
        movss xmm1, dword ptr [eax] // 00b63474
        comiss xmm1, xmm0 // 00b63478
        jbe L_00b63485 // 00b6347b
        movss dword ptr [esp + 0x14], xmm1 // 00b6347d
        jmp L_00b63492 // 00b63483
L_00b63485:
        movaps xmm2, xmm5 // 00b63485
        subss xmm2, xmm1 // 00b63488
        movss dword ptr [esp + 0x14], xmm2 // 00b6348c
L_00b63492:
        mov eax, dword ptr [esp + 0x1c] // 00b63492
        add eax, edx // 00b63496
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b63498
        comiss xmm1, xmm0 // 00b6349e
        lea eax, [esp + eax + 0x54] // 00b634a1
        jbe L_00b634af // 00b634a5
        movss dword ptr [esp + 0x18], xmm1 // 00b634a7
        jmp L_00b634bc // 00b634ad
L_00b634af:
        movaps xmm2, xmm5 // 00b634af
        subss xmm2, xmm1 // 00b634b2
        movss dword ptr [esp + 0x18], xmm2 // 00b634b6
L_00b634bc:
        fld dword ptr [esp + 0x18] // 00b634bc
        fld dword ptr [esp + 0x14] // 00b634c0
        fcomip st(0), st(1) // 00b634c4
        fstp st(0) // 00b634c6
        jbe L_00b634d0 // 00b634c8
        lea edi, [esi + 1] // 00b634ca
        lea edx, [ebp - 0x10] // 00b634cd
L_00b634d0:
        mov eax, dword ptr [esp + 0x10] // 00b634d0
        movss xmm1, dword ptr [eax + 0x10] // 00b634d4
        comiss xmm1, xmm0 // 00b634d9
        jbe L_00b634e6 // 00b634dc
        movss dword ptr [esp + 0x14], xmm1 // 00b634de
        jmp L_00b634f3 // 00b634e4
L_00b634e6:
        movaps xmm2, xmm5 // 00b634e6
        subss xmm2, xmm1 // 00b634e9
        movss dword ptr [esp + 0x14], xmm2 // 00b634ed
L_00b634f3:
        mov eax, dword ptr [esp + 0x1c] // 00b634f3
        add eax, edx // 00b634f7
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b634f9
        comiss xmm1, xmm0 // 00b634ff
        lea eax, [esp + eax + 0x54] // 00b63502
        jbe L_00b63510 // 00b63506
        movss dword ptr [esp + 0x18], xmm1 // 00b63508
        jmp L_00b6351d // 00b6350e
L_00b63510:
        movaps xmm2, xmm5 // 00b63510
        subss xmm2, xmm1 // 00b63513
        movss dword ptr [esp + 0x18], xmm2 // 00b63517
L_00b6351d:
        fld dword ptr [esp + 0x18] // 00b6351d
        fld dword ptr [esp + 0x14] // 00b63521
        fcomip st(0), st(1) // 00b63525
        fstp st(0) // 00b63527
        jbe L_00b63530 // 00b63529
        lea edi, [esi + 2] // 00b6352b
        mov edx, ebp // 00b6352e
L_00b63530:
        mov eax, dword ptr [esp + 0x10] // 00b63530
        movss xmm1, dword ptr [eax + 0x20] // 00b63534
        comiss xmm1, xmm0 // 00b63539
        jbe L_00b63546 // 00b6353c
        movss dword ptr [esp + 0x14], xmm1 // 00b6353e
        jmp L_00b63553 // 00b63544
L_00b63546:
        movaps xmm2, xmm5 // 00b63546
        subss xmm2, xmm1 // 00b63549
        movss dword ptr [esp + 0x14], xmm2 // 00b6354d
L_00b63553:
        mov eax, dword ptr [esp + 0x1c] // 00b63553
        add eax, edx // 00b63557
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b63559
        comiss xmm1, xmm0 // 00b6355f
        lea eax, [esp + eax + 0x54] // 00b63562
        jbe L_00b63570 // 00b63566
        movss dword ptr [esp + 0x18], xmm1 // 00b63568
        jmp L_00b6357d // 00b6356e
L_00b63570:
        movaps xmm2, xmm5 // 00b63570
        subss xmm2, xmm1 // 00b63573
        movss dword ptr [esp + 0x18], xmm2 // 00b63577
L_00b6357d:
        fld dword ptr [esp + 0x18] // 00b6357d
        fld dword ptr [esp + 0x14] // 00b63581
        fcomip st(0), st(1) // 00b63585
        fstp st(0) // 00b63587
        jbe L_00b63591 // 00b63589
        lea edi, [esi + 3] // 00b6358b
        lea edx, [ebp + 0x10] // 00b6358e
L_00b63591:
        add dword ptr [esp + 0x10], 0x40 // 00b63591
        add ebp, 0x40 // 00b63596
        add esi, 4 // 00b63599
        cmp ebp, 0x30 // 00b6359c
        jl L_00b63410 // 00b6359f
        mov ebp, dword ptr [esp + 0x94] // 00b635a5
        mov edx, dword ptr [esp + 0x24] // 00b635ac
L_00b635b0:
        cmp esi, 4 // 00b635b0
        jge L_00b6365b // 00b635b3
        mov ebp, edi // 00b635b9
        mov eax, esi // 00b635bb
        shl ebp, 4 // 00b635bd
        shl eax, 4 // 00b635c0
        mov dword ptr [esp + 0x10], eax // 00b635c3
        mov eax, dword ptr [esp + 0x20] // 00b635c7
        lea eax, [eax + esi*4] // 00b635cb
        lea eax, [esp + eax*4 + 0x54] // 00b635ce
        mov dword ptr [esp + 0x24], eax // 00b635d2
        jmp L_00b635e0 // 00b635d6
        lea esp, [esp] // 00b635d8
L_00b635e0:
        mov eax, dword ptr [esp + 0x24] // 00b635e0
        movss xmm1, dword ptr [eax] // 00b635e4
        comiss xmm1, xmm0 // 00b635e8
        jbe L_00b635f5 // 00b635eb
        movss dword ptr [esp + 0x14], xmm1 // 00b635ed
        jmp L_00b63602 // 00b635f3
L_00b635f5:
        movaps xmm2, xmm5 // 00b635f5
        subss xmm2, xmm1 // 00b635f8
        movss dword ptr [esp + 0x14], xmm2 // 00b635fc
L_00b63602:
        mov eax, dword ptr [esp + 0x1c] // 00b63602
        add eax, ebp // 00b63606
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b63608
        comiss xmm1, xmm0 // 00b6360e
        lea eax, [esp + eax + 0x54] // 00b63611
        jbe L_00b6361f // 00b63615
        movss dword ptr [esp + 0x18], xmm1 // 00b63617
        jmp L_00b6362c // 00b6361d
L_00b6361f:
        movaps xmm2, xmm5 // 00b6361f
        subss xmm2, xmm1 // 00b63622
        movss dword ptr [esp + 0x18], xmm2 // 00b63626
L_00b6362c:
        fld dword ptr [esp + 0x18] // 00b6362c
        fld dword ptr [esp + 0x14] // 00b63630
        fcomip st(0), st(1) // 00b63634
        fstp st(0) // 00b63636
        jbe L_00b63640 // 00b63638
        mov ebp, dword ptr [esp + 0x10] // 00b6363a
        mov edi, esi // 00b6363e
L_00b63640:
        add dword ptr [esp + 0x10], 0x10 // 00b63640
        add dword ptr [esp + 0x24], 0x10 // 00b63645
        add esi, 1 // 00b6364a
        cmp dword ptr [esp + 0x10], 0x40 // 00b6364d
        jl L_00b635e0 // 00b63652
        mov ebp, dword ptr [esp + 0x94] // 00b63654
L_00b6365b:
        fld dword ptr [edx] // 00b6365b
        mov esi, dword ptr [esp + 0xbc] // 00b6365d
        shl edi, 4 // 00b63664
        cmp dword ptr [esp + 0x20], 0 // 00b63667
        mov eax, edi // 00b6366c
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b6366e
        fstp dword ptr [esp + eax + 0x54] // 00b63674
        movss xmm2, dword ptr [esp + eax + 0x58] // 00b63678
        fld dword ptr [ecx + ebp] // 00b6367e
        fstp dword ptr [esp + eax + 0x58] // 00b63681
        movss xmm3, dword ptr [esp + eax + 0x5c] // 00b63685
        movss xmm4, dword ptr [esp + eax + 0x60] // 00b6368b
        fld dword ptr [esi + ecx] // 00b63691
        fstp dword ptr [esp + eax + 0x5c] // 00b63694
        mov edi, dword ptr [esp + 0x98] // 00b63698
        fld dword ptr [edi + ecx] // 00b6369f
        fstp dword ptr [esp + eax + 0x60] // 00b636a2
        movss dword ptr [edi + ecx], xmm4 // 00b636a6
        fld dword ptr [ecx - 4] // 00b636ab
        movss xmm4, dword ptr [eax + ebx + 0xc] // 00b636ae
        movss dword ptr [ecx + ebp], xmm2 // 00b636b4
        movss xmm2, dword ptr [eax + ebx + 4] // 00b636b9
        movss dword ptr [edx], xmm1 // 00b636bf
        movss xmm1, dword ptr [eax + ebx] // 00b636c3
        fstp dword ptr [eax + ebx] // 00b636c8
        fld dword ptr [ecx] // 00b636cb
        movss dword ptr [esi + ecx], xmm3 // 00b636cd
        movss xmm3, dword ptr [eax + ebx + 8] // 00b636d2
        lea edi, [eax + ebx + 4] // 00b636d8
        mov dword ptr [esp + 0x48], edi // 00b636dc
        lea edi, [eax + ebx + 8] // 00b636e0
        mov dword ptr [esp + 0x24], edi // 00b636e4
        lea edi, [eax + ebx + 0xc] // 00b636e8
        mov eax, dword ptr [esp + 0x48] // 00b636ec
        fstp dword ptr [eax] // 00b636f0
        mov eax, dword ptr [esp + 0x24] // 00b636f2
        fld dword ptr [ecx + 4] // 00b636f6
        fstp dword ptr [eax] // 00b636f9
        mov eax, dword ptr [esp + 0x50] // 00b636fb
        fld dword ptr [ecx + 8] // 00b636ff
        fstp dword ptr [edi] // 00b63702
        movss dword ptr [ecx - 4], xmm1 // 00b63704
        fld dword ptr [eax] // 00b63709
        movss dword ptr [ecx], xmm2 // 00b6370b
        fstp dword ptr [esp + 0x48] // 00b6370f
        movss dword ptr [ecx + 4], xmm3 // 00b63713
        movss dword ptr [ecx + 8], xmm4 // 00b63718
        fld dword ptr [ecx - 4] // 00b6371d
        fld dword ptr [esp + 0x48] // 00b63720
        mov edi, dword ptr [esp + 0x98] // 00b63724
        fld st(0) // 00b6372b
        fdivp st(2), st(0) // 00b6372d
        fxch st(1) // 00b6372f
        fstp dword ptr [ecx - 4] // 00b63731
        fld dword ptr [ecx] // 00b63734
        fdiv st(0), st(1) // 00b63736
        fstp dword ptr [ecx] // 00b63738
        fld dword ptr [ecx + 4] // 00b6373a
        fdiv st(0), st(1) // 00b6373d
        fstp dword ptr [ecx + 4] // 00b6373f
        fld dword ptr [ecx + 8] // 00b63742
        fdiv st(0), st(1) // 00b63745
        fstp dword ptr [ecx + 8] // 00b63747
        fld dword ptr [edx] // 00b6374a
        fdiv st(0), st(1) // 00b6374c
        fstp dword ptr [edx] // 00b6374e
        fld dword ptr [ecx + ebp] // 00b63750
        fdiv st(0), st(1) // 00b63753
        fstp dword ptr [ecx + ebp] // 00b63755
        fld dword ptr [esi + ecx] // 00b63758
        fdiv st(0), st(1) // 00b6375b
        fstp dword ptr [esi + ecx] // 00b6375d
        fdivr dword ptr [edi + ecx] // 00b63760
        fstp dword ptr [edi + ecx] // 00b63763
        je L_00b63839 // 00b63766
        mov eax, dword ptr [esp + 0x1c] // 00b6376c
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b63770
        ucomiss xmm1, xmm0 // 00b63776
        lahf  // 00b63779
        test ah, 0x44 // 00b6377a
        movss dword ptr [esp + 0xb0], xmm1 // 00b6377d
        jnp L_00b63839 // 00b63786
        fld dword ptr [esp + 0xb0] // 00b6378c
        fstp dword ptr [esp + 0x10] // 00b63793
        fld dword ptr [ecx - 4] // 00b63797
        fld dword ptr [esp + 0x10] // 00b6379a
        fld st(0) // 00b6379e
        fmulp st(2), st(0) // 00b637a0
        fxch st(1) // 00b637a2
        fstp dword ptr [esp + 0x38] // 00b637a4
        fld dword ptr [ecx] // 00b637a8
        fmul st(0), st(1) // 00b637aa
        fstp dword ptr [esp + 0x3c] // 00b637ac
        fld dword ptr [ecx + 4] // 00b637b0
        fmul st(0), st(1) // 00b637b3
        fstp dword ptr [esp + 0x40] // 00b637b5
        fld dword ptr [ecx + 8] // 00b637b9
        fmul st(0), st(1) // 00b637bc
        fstp dword ptr [esp + 0x44] // 00b637be
        fld dword ptr [ebx] // 00b637c2
        fsub dword ptr [esp + 0x38] // 00b637c4
        fstp dword ptr [ebx] // 00b637c8
        fld dword ptr [ebx + 4] // 00b637ca
        fsub dword ptr [esp + 0x3c] // 00b637cd
        fstp dword ptr [ebx + 4] // 00b637d1
        fld dword ptr [ebx + 8] // 00b637d4
        fsub dword ptr [esp + 0x40] // 00b637d7
        fstp dword ptr [ebx + 8] // 00b637db
        fld dword ptr [ebx + 0xc] // 00b637de
        fsub dword ptr [esp + 0x44] // 00b637e1
        fstp dword ptr [ebx + 0xc] // 00b637e5
        fld dword ptr [edx] // 00b637e8
        fmul st(0), st(1) // 00b637ea
        fstp dword ptr [esp + 0x28] // 00b637ec
        fld st(0) // 00b637f0
        fmul dword ptr [ecx + ebp] // 00b637f2
        fstp dword ptr [esp + 0x2c] // 00b637f5
        fld dword ptr [esi + ecx] // 00b637f9
        fmul st(0), st(1) // 00b637fc
        fstp dword ptr [esp + 0x30] // 00b637fe
        fmul dword ptr [edi + ecx] // 00b63802
        fstp dword ptr [esp + 0x34] // 00b63805
        fld dword ptr [esp + 0x54] // 00b63809
        fsub dword ptr [esp + 0x28] // 00b6380d
        fstp dword ptr [esp + 0x54] // 00b63811
        fld dword ptr [esp + 0x58] // 00b63815
        fsub dword ptr [esp + 0x2c] // 00b63819
        fstp dword ptr [esp + 0x58] // 00b6381d
        fld dword ptr [esp + 0x5c] // 00b63821
        fsub dword ptr [esp + 0x30] // 00b63825
        fstp dword ptr [esp + 0x5c] // 00b63829
        fld dword ptr [esp + 0x60] // 00b6382d
        fsub dword ptr [esp + 0x34] // 00b63831
        fstp dword ptr [esp + 0x60] // 00b63835
L_00b63839:
        cmp dword ptr [esp + 0x20], 1 // 00b63839
        je L_00b63913 // 00b6383e
        mov eax, dword ptr [esp + 0x1c] // 00b63844
        movss xmm1, dword ptr [esp + eax + 0x64] // 00b63848
        ucomiss xmm1, xmm0 // 00b6384e
        lahf  // 00b63851
        test ah, 0x44 // 00b63852
        movss dword ptr [esp + 0xa8], xmm1 // 00b63855
        jnp L_00b63913 // 00b6385e
        fld dword ptr [esp + 0xa8] // 00b63864
        fstp dword ptr [esp + 0x10] // 00b6386b
        fld dword ptr [ecx - 4] // 00b6386f
        fld dword ptr [esp + 0x10] // 00b63872
        fld st(0) // 00b63876
        fmulp st(2), st(0) // 00b63878
        fxch st(1) // 00b6387a
        fstp dword ptr [esp + 0x38] // 00b6387c
        fld dword ptr [ecx] // 00b63880
        fmul st(0), st(1) // 00b63882
        fstp dword ptr [esp + 0x3c] // 00b63884
        fld dword ptr [ecx + 4] // 00b63888
        fmul st(0), st(1) // 00b6388b
        fstp dword ptr [esp + 0x40] // 00b6388d
        fld dword ptr [ecx + 8] // 00b63891
        fmul st(0), st(1) // 00b63894
        fstp dword ptr [esp + 0x44] // 00b63896
        fld dword ptr [ebx + 0x10] // 00b6389a
        fsub dword ptr [esp + 0x38] // 00b6389d
        fstp dword ptr [ebx + 0x10] // 00b638a1
        fld dword ptr [ebx + 0x14] // 00b638a4
        fsub dword ptr [esp + 0x3c] // 00b638a7
        fstp dword ptr [ebx + 0x14] // 00b638ab
        fld dword ptr [ebx + 0x18] // 00b638ae
        fsub dword ptr [esp + 0x40] // 00b638b1
        fstp dword ptr [ebx + 0x18] // 00b638b5
        fld dword ptr [ebx + 0x1c] // 00b638b8
        fsub dword ptr [esp + 0x44] // 00b638bb
        fstp dword ptr [ebx + 0x1c] // 00b638bf
        fld dword ptr [edx] // 00b638c2
        fmul st(0), st(1) // 00b638c4
        fstp dword ptr [esp + 0x28] // 00b638c6
        fld st(0) // 00b638ca
        fmul dword ptr [ecx + ebp] // 00b638cc
        fstp dword ptr [esp + 0x2c] // 00b638cf
        fld dword ptr [esi + ecx] // 00b638d3
        fmul st(0), st(1) // 00b638d6
        fstp dword ptr [esp + 0x30] // 00b638d8
        fmul dword ptr [edi + ecx] // 00b638dc
        fstp dword ptr [esp + 0x34] // 00b638df
        fld dword ptr [esp + 0x64] // 00b638e3
        fsub dword ptr [esp + 0x28] // 00b638e7
        fstp dword ptr [esp + 0x64] // 00b638eb
        fld dword ptr [esp + 0x68] // 00b638ef
        fsub dword ptr [esp + 0x2c] // 00b638f3
        fstp dword ptr [esp + 0x68] // 00b638f7
        fld dword ptr [esp + 0x6c] // 00b638fb
        fsub dword ptr [esp + 0x30] // 00b638ff
        fstp dword ptr [esp + 0x6c] // 00b63903
        fld dword ptr [esp + 0x70] // 00b63907
        fsub dword ptr [esp + 0x34] // 00b6390b
        fstp dword ptr [esp + 0x70] // 00b6390f
L_00b63913:
        cmp dword ptr [esp + 0x20], 2 // 00b63913
        je L_00b639f3 // 00b63918
        mov eax, dword ptr [esp + 0x1c] // 00b6391e
        movss xmm1, dword ptr [esp + eax + 0x74] // 00b63922
        ucomiss xmm1, xmm0 // 00b63928
        lahf  // 00b6392b
        test ah, 0x44 // 00b6392c
        movss dword ptr [esp + 0xb8], xmm1 // 00b6392f
        jnp L_00b639f3 // 00b63938
        fld dword ptr [esp + 0xb8] // 00b6393e
        fstp dword ptr [esp + 0x10] // 00b63945
        fld dword ptr [ecx - 4] // 00b63949
        fld dword ptr [esp + 0x10] // 00b6394c
        fld st(0) // 00b63950
        fmulp st(2), st(0) // 00b63952
        fxch st(1) // 00b63954
        fstp dword ptr [esp + 0x38] // 00b63956
        fld dword ptr [ecx] // 00b6395a
        fmul st(0), st(1) // 00b6395c
        fstp dword ptr [esp + 0x3c] // 00b6395e
        fld dword ptr [ecx + 4] // 00b63962
        fmul st(0), st(1) // 00b63965
        fstp dword ptr [esp + 0x40] // 00b63967
        fld dword ptr [ecx + 8] // 00b6396b
        fmul st(0), st(1) // 00b6396e
        fstp dword ptr [esp + 0x44] // 00b63970
        fld dword ptr [ebx + 0x20] // 00b63974
        fsub dword ptr [esp + 0x38] // 00b63977
        fstp dword ptr [ebx + 0x20] // 00b6397b
        fld dword ptr [ebx + 0x24] // 00b6397e
        fsub dword ptr [esp + 0x3c] // 00b63981
        fstp dword ptr [ebx + 0x24] // 00b63985
        fld dword ptr [ebx + 0x28] // 00b63988
        fsub dword ptr [esp + 0x40] // 00b6398b
        fstp dword ptr [ebx + 0x28] // 00b6398f
        fld dword ptr [ebx + 0x2c] // 00b63992
        fsub dword ptr [esp + 0x44] // 00b63995
        fstp dword ptr [ebx + 0x2c] // 00b63999
        fld dword ptr [edx] // 00b6399c
        fmul st(0), st(1) // 00b6399e
        fstp dword ptr [esp + 0x28] // 00b639a0
        fld st(0) // 00b639a4
        fmul dword ptr [ecx + ebp] // 00b639a6
        fstp dword ptr [esp + 0x2c] // 00b639a9
        fld dword ptr [esi + ecx] // 00b639ad
        fmul st(0), st(1) // 00b639b0
        fstp dword ptr [esp + 0x30] // 00b639b2
        fmul dword ptr [edi + ecx] // 00b639b6
        fstp dword ptr [esp + 0x34] // 00b639b9
        fld dword ptr [esp + 0x74] // 00b639bd
        fsub dword ptr [esp + 0x28] // 00b639c1
        fstp dword ptr [esp + 0x74] // 00b639c5
        fld dword ptr [esp + 0x78] // 00b639c9
        fsub dword ptr [esp + 0x2c] // 00b639cd
        fstp dword ptr [esp + 0x78] // 00b639d1
        fld dword ptr [esp + 0x7c] // 00b639d5
        fsub dword ptr [esp + 0x30] // 00b639d9
        fstp dword ptr [esp + 0x7c] // 00b639dd
        fld dword ptr [esp + 0x80] // 00b639e1
        fsub dword ptr [esp + 0x34] // 00b639e8
        fstp dword ptr [esp + 0x80] // 00b639ec
L_00b639f3:
        cmp dword ptr [esp + 0x20], 3 // 00b639f3
        je L_00b63ae8 // 00b639f8
        mov eax, dword ptr [esp + 0x1c] // 00b639fe
        movss xmm1, dword ptr [esp + eax + 0x84] // 00b63a02
        ucomiss xmm1, xmm0 // 00b63a0b
        lahf  // 00b63a0e
        test ah, 0x44 // 00b63a0f
        movss dword ptr [esp + 0xa0], xmm1 // 00b63a12
        jnp L_00b63ae8 // 00b63a1b
        fld dword ptr [esp + 0xa0] // 00b63a21
        fstp dword ptr [esp + 0x10] // 00b63a28
        fld dword ptr [ecx - 4] // 00b63a2c
        fld dword ptr [esp + 0x10] // 00b63a2f
        fld st(0) // 00b63a33
        fmulp st(2), st(0) // 00b63a35
        fxch st(1) // 00b63a37
        fstp dword ptr [esp + 0x38] // 00b63a39
        fld dword ptr [ecx] // 00b63a3d
        fmul st(0), st(1) // 00b63a3f
        fstp dword ptr [esp + 0x3c] // 00b63a41
        fld dword ptr [ecx + 4] // 00b63a45
        fmul st(0), st(1) // 00b63a48
        fstp dword ptr [esp + 0x40] // 00b63a4a
        fld dword ptr [ecx + 8] // 00b63a4e
        fmul st(0), st(1) // 00b63a51
        fstp dword ptr [esp + 0x44] // 00b63a53
        fld dword ptr [ebx + 0x30] // 00b63a57
        fsub dword ptr [esp + 0x38] // 00b63a5a
        fstp dword ptr [ebx + 0x30] // 00b63a5e
        fld dword ptr [ebx + 0x34] // 00b63a61
        fsub dword ptr [esp + 0x3c] // 00b63a64
        fstp dword ptr [ebx + 0x34] // 00b63a68
        fld dword ptr [ebx + 0x38] // 00b63a6b
        fsub dword ptr [esp + 0x40] // 00b63a6e
        fstp dword ptr [ebx + 0x38] // 00b63a72
        fld dword ptr [ebx + 0x3c] // 00b63a75
        fsub dword ptr [esp + 0x44] // 00b63a78
        fstp dword ptr [ebx + 0x3c] // 00b63a7c
        fld dword ptr [edx] // 00b63a7f
        fmul st(0), st(1) // 00b63a81
        fstp dword ptr [esp + 0x28] // 00b63a83
        fld st(0) // 00b63a87
        fmul dword ptr [ecx + ebp] // 00b63a89
        fstp dword ptr [esp + 0x2c] // 00b63a8c
        fld dword ptr [esi + ecx] // 00b63a90
        fmul st(0), st(1) // 00b63a93
        fstp dword ptr [esp + 0x30] // 00b63a95
        fmul dword ptr [edi + ecx] // 00b63a99
        fstp dword ptr [esp + 0x34] // 00b63a9c
        fld dword ptr [esp + 0x84] // 00b63aa0
        fsub dword ptr [esp + 0x28] // 00b63aa7
        fstp dword ptr [esp + 0x84] // 00b63aab
        fld dword ptr [esp + 0x88] // 00b63ab2
        fsub dword ptr [esp + 0x2c] // 00b63ab9
        fstp dword ptr [esp + 0x88] // 00b63abd
        fld dword ptr [esp + 0x8c] // 00b63ac4
        fsub dword ptr [esp + 0x30] // 00b63acb
        fstp dword ptr [esp + 0x8c] // 00b63acf
        fld dword ptr [esp + 0x90] // 00b63ad6
        fsub dword ptr [esp + 0x34] // 00b63add
        fstp dword ptr [esp + 0x90] // 00b63ae1
L_00b63ae8:
        mov eax, dword ptr [esp + 0xa4] // 00b63ae8
        add dword ptr [esp + 0x50], 0x14 // 00b63aef
        add dword ptr [esp + 0x1c], 4 // 00b63af4
        mov dword ptr [esp + 0x20], eax // 00b63af9
        mov eax, dword ptr [esp + 0x4c] // 00b63afd
        sub eax, 1 // 00b63b01
        add edx, 0x10 // 00b63b04
        add ecx, 0x10 // 00b63b07
        cmp eax, -1 // 00b63b0a
        mov dword ptr [esp + 0x4c], eax // 00b63b0d
        mov dword ptr [esp + 0x24], edx // 00b63b11
        jg L_00b633c0 // 00b63b15
        pop edi // 00b63b1b
        pop esi // 00b63b1c
        pop ebp // 00b63b1d
        mov eax, ebx // 00b63b1e
        pop ebx // 00b63b20
        add esp, 0xb0 // 00b63b21
        ret  // 00b63b27
    }
}

__declspec(naked) float __fastcall length_kernel(const float*) {
    __asm {
        sub esp, 0xc // 00419440
        fld dword ptr [ecx] // 00419443
        fstp dword ptr [esp + 4] // 00419445
        fld dword ptr [ecx + 4] // 00419449
        fstp dword ptr [esp] // 0041944c
        fld dword ptr [ecx + 8] // 0041944f
        fstp dword ptr [esp + 8] // 00419452
        fld dword ptr [esp] // 00419456
        fld dword ptr [esp + 4] // 00419459
        fld dword ptr [esp + 8] // 0041945d
        fld st(1) // 00419461
        fmulp st(2), st(0) // 00419463
        fxch st(1) // 00419465
        fstp dword ptr [esp + 8] // 00419467
        fld dword ptr [esp + 8] // 0041946b
        fld st(2) // 0041946f
        fmulp st(3), st(0) // 00419471
        fxch st(2) // 00419473
        fstp dword ptr [esp + 8] // 00419475
        fld dword ptr [esp + 8] // 00419479
        faddp st(2), st(0) // 0041947d
        fmul st(0), st(0) // 0041947f
        fstp dword ptr [esp + 8] // 00419481
        fadd dword ptr [esp + 8] // 00419485
        fstp dword ptr [esp + 8] // 00419489
        fld dword ptr [esp + 8] // 0041948d
        fsqrt // 00419491: finite nonnegative CRT00bf7030 core; diagnostics open
        fstp dword ptr [esp + 8] // 00419496
        fld dword ptr [esp + 8] // 0041949a
        add esp, 0xc // 0041949e
        ret  // 004194a1
    }
}

__declspec(naked) float* __fastcall normalize_kernel(float*) {
    __asm {
        push ecx // 00b650b0
        push esi // 00b650b1
        mov esi, ecx // 00b650b2
        call length_kernel // 00b650b4
        fstp dword ptr [esp + 4] // 00b650b9
        fldz  // 00b650bd
        fld dword ptr [esp + 4] // 00b650bf
        fcomi st(0), st(1) // 00b650c3
        fstp st(1) // 00b650c5
        jbe L_00b650d3 // 00b650c7
        fld1  // 00b650c9
        fdivrp st(1), st(0) // 00b650cb
        fstp dword ptr [esp + 4] // 00b650cd
        jmp L_00b650de // 00b650d1
L_00b650d3:
        xorps xmm0, xmm0 // 00b650d3
        fstp st(0) // 00b650d6
        movss dword ptr [esp + 4], xmm0 // 00b650d8
L_00b650de:
        fld dword ptr [esi] // 00b650de
        mov eax, esi // 00b650e0
        fld dword ptr [esp + 4] // 00b650e2
        fld st(0) // 00b650e6
        fmulp st(2), st(0) // 00b650e8
        fxch st(1) // 00b650ea
        fstp dword ptr [esi] // 00b650ec
        fld dword ptr [esi + 4] // 00b650ee
        fmul st(0), st(1) // 00b650f1
        fstp dword ptr [esi + 4] // 00b650f3
        fld st(0) // 00b650f6
        fmul dword ptr [esi + 8] // 00b650f8
        fstp dword ptr [esi + 8] // 00b650fb
        fmul dword ptr [esi + 0xc] // 00b650fe
        fstp dword ptr [esi + 0xc] // 00b65101
        pop esi // 00b65104
        pop ecx // 00b65105
        ret  // 00b65106
    }
}

__declspec(naked) float* __fastcall extract_kernel(float*, void*, const float*) {
    __asm {
        mov eax, dword ptr [esp + 4] // 00b653f0
        fld dword ptr [eax] // 00b653f4
        push ebx // 00b653f6
        fadd dword ptr [eax + 0xc] // 00b653f7
        push ebp // 00b653fa
        push esi // 00b653fb
        mov esi, ecx // 00b653fc
        fstp dword ptr [esi] // 00b653fe
        push edi // 00b65400
        fld dword ptr [eax + 0x1c] // 00b65401
        lea edi, [esi + 0x10] // 00b65404
        fadd dword ptr [eax + 0x10] // 00b65407
        lea ebx, [esi + 0x20] // 00b6540a
        lea ebp, [esi + 0x30] // 00b6540d
        fstp dword ptr [esi + 4] // 00b65410
        fld dword ptr [eax + 0x2c] // 00b65413
        fadd dword ptr [eax + 0x20] // 00b65416
        fstp dword ptr [esi + 8] // 00b65419
        fld dword ptr [eax + 0x3c] // 00b6541c
        fadd dword ptr [eax + 0x30] // 00b6541f
        fstp dword ptr [esi + 0xc] // 00b65422
        fld dword ptr [eax + 0xc] // 00b65425
        fsub dword ptr [eax] // 00b65428
        fstp dword ptr [edi] // 00b6542a
        fld dword ptr [eax + 0x1c] // 00b6542c
        fsub dword ptr [eax + 0x10] // 00b6542f
        fstp dword ptr [esi + 0x14] // 00b65432
        fld dword ptr [eax + 0x2c] // 00b65435
        fsub dword ptr [eax + 0x20] // 00b65438
        fstp dword ptr [esi + 0x18] // 00b6543b
        fld dword ptr [eax + 0x3c] // 00b6543e
        fsub dword ptr [eax + 0x30] // 00b65441
        fstp dword ptr [esi + 0x1c] // 00b65444
        fld dword ptr [eax + 0xc] // 00b65447
        fsub dword ptr [eax + 4] // 00b6544a
        fstp dword ptr [ebx] // 00b6544d
        fld dword ptr [eax + 0x1c] // 00b6544f
        fsub dword ptr [eax + 0x14] // 00b65452
        fstp dword ptr [esi + 0x24] // 00b65455
        fld dword ptr [eax + 0x2c] // 00b65458
        fsub dword ptr [eax + 0x24] // 00b6545b
        fstp dword ptr [esi + 0x28] // 00b6545e
        fld dword ptr [eax + 0x3c] // 00b65461
        fsub dword ptr [eax + 0x34] // 00b65464
        fstp dword ptr [esi + 0x2c] // 00b65467
        fld dword ptr [eax + 4] // 00b6546a
        fadd dword ptr [eax + 0xc] // 00b6546d
        fstp dword ptr [ebp] // 00b65470
        fld dword ptr [eax + 0x1c] // 00b65473
        fadd dword ptr [eax + 0x14] // 00b65476
        fstp dword ptr [esi + 0x34] // 00b65479
        fld dword ptr [eax + 0x2c] // 00b6547c
        fadd dword ptr [eax + 0x24] // 00b6547f
        fstp dword ptr [esi + 0x38] // 00b65482
        fld dword ptr [eax + 0x3c] // 00b65485
        fadd dword ptr [eax + 0x34] // 00b65488
        fstp dword ptr [esi + 0x3c] // 00b6548b
        fld dword ptr [eax + 8] // 00b6548e
        fadd st(0), st(0) // 00b65491
        fstp dword ptr [esi + 0x40] // 00b65493
        fld dword ptr [eax + 0x18] // 00b65496
        fadd st(0), st(0) // 00b65499
        fstp dword ptr [esi + 0x44] // 00b6549b
        fld dword ptr [eax + 0x28] // 00b6549e
        fadd st(0), st(0) // 00b654a1
        fstp dword ptr [esi + 0x48] // 00b654a3
        fld dword ptr [eax + 0x38] // 00b654a6
        fadd st(0), st(0) // 00b654a9
        fstp dword ptr [esi + 0x4c] // 00b654ab
        fld dword ptr [eax + 0xc] // 00b654ae
        fsub dword ptr [eax + 8] // 00b654b1
        fstp dword ptr [esi + 0x50] // 00b654b4
        fld dword ptr [eax + 0x1c] // 00b654b7
        fsub dword ptr [eax + 0x18] // 00b654ba
        fstp dword ptr [esi + 0x54] // 00b654bd
        fld dword ptr [eax + 0x2c] // 00b654c0
        fsub dword ptr [eax + 0x28] // 00b654c3
        fstp dword ptr [esi + 0x58] // 00b654c6
        fld dword ptr [eax + 0x3c] // 00b654c9
        fsub dword ptr [eax + 0x38] // 00b654cc
        fstp dword ptr [esi + 0x5c] // 00b654cf
        movss xmm0, dword ptr [negative_zero_bits] // 00b654d2
        movaps xmm1, xmm0 // 00b654da
        subss xmm1, dword ptr [esi + 0xc] // 00b654dd
        movss dword ptr [esi + 0xc], xmm1 // 00b654e2
        movaps xmm1, xmm0 // 00b654e7
        subss xmm1, dword ptr [esi + 0x1c] // 00b654ea
        movss dword ptr [esi + 0x1c], xmm1 // 00b654ef
        movaps xmm1, xmm0 // 00b654f4
        subss xmm1, dword ptr [esi + 0x2c] // 00b654f7
        movss dword ptr [esi + 0x2c], xmm1 // 00b654fc
        movaps xmm1, xmm0 // 00b65501
        subss xmm1, dword ptr [esi + 0x3c] // 00b65504
        movss dword ptr [esi + 0x3c], xmm1 // 00b65509
        movaps xmm1, xmm0 // 00b6550e
        subss xmm1, dword ptr [esi + 0x4c] // 00b65511
        subss xmm0, dword ptr [esi + 0x5c] // 00b65516
        movss dword ptr [esi + 0x4c], xmm1 // 00b6551b
        movss dword ptr [esi + 0x5c], xmm0 // 00b65520
        call normalize_kernel // 00b65525
        mov ecx, edi // 00b6552a
        call normalize_kernel // 00b6552c
        mov ecx, ebx // 00b65531
        call normalize_kernel // 00b65533
        mov ecx, ebp // 00b65538
        call normalize_kernel // 00b6553a
        lea ecx, [esi + 0x40] // 00b6553f
        call normalize_kernel // 00b65542
        lea ecx, [esi + 0x50] // 00b65547
        call normalize_kernel // 00b6554a
        pop edi // 00b6554f
        mov eax, esi // 00b65550
        pop esi // 00b65552
        pop ebp // 00b65553
        pop ebx // 00b65554
        ret 4 // 00b65555
    }
}

__declspec(naked) float* __fastcall transform_kernel(const float*, void*, float*, const float*) {
    __asm {
        sub esp, 0x10 // 00b62d10
        fld dword ptr [ecx + 4] // 00b62d13
        mov eax, dword ptr [esp + 0x14] // 00b62d16
        fstp dword ptr [esp] // 00b62d1a
        fld dword ptr [ecx] // 00b62d1d
        fstp dword ptr [esp + 4] // 00b62d1f
        fld dword ptr [ecx + 8] // 00b62d23
        fstp dword ptr [esp + 8] // 00b62d26
        fld dword ptr [ecx + 0xc] // 00b62d2a
        mov ecx, dword ptr [esp + 0x18] // 00b62d2d
        fstp dword ptr [esp + 0xc] // 00b62d31
        fld dword ptr [ecx + 0x10] // 00b62d35
        fld dword ptr [esp] // 00b62d38
        fld st(0) // 00b62d3b
        fmulp st(2), st(0) // 00b62d3d
        fld dword ptr [esp + 4] // 00b62d3f
        fld st(0) // 00b62d43
        fmul dword ptr [ecx] // 00b62d45
        faddp st(3), st(0) // 00b62d47
        fld dword ptr [ecx + 0x20] // 00b62d49
        fld dword ptr [esp + 8] // 00b62d4c
        fld st(0) // 00b62d50
        fmulp st(2), st(0) // 00b62d52
        fxch st(4) // 00b62d54
        faddp st(1), st(0) // 00b62d56
        fld dword ptr [ecx + 0x30] // 00b62d58
        fld dword ptr [esp + 0xc] // 00b62d5b
        fld st(0) // 00b62d5f
        fmulp st(2), st(0) // 00b62d61
        fxch st(2) // 00b62d63
        faddp st(1), st(0) // 00b62d65
        fstp dword ptr [eax] // 00b62d67
        fld dword ptr [ecx + 4] // 00b62d69
        fmul st(0), st(2) // 00b62d6c
        fld dword ptr [ecx + 0x14] // 00b62d6e
        fmul st(0), st(4) // 00b62d71
        faddp st(1), st(0) // 00b62d73
        fld dword ptr [ecx + 0x24] // 00b62d75
        fmul st(0), st(5) // 00b62d78
        faddp st(1), st(0) // 00b62d7a
        fld dword ptr [ecx + 0x34] // 00b62d7c
        fmul st(0), st(2) // 00b62d7f
        faddp st(1), st(0) // 00b62d81
        fstp dword ptr [eax + 4] // 00b62d83
        fld dword ptr [ecx + 8] // 00b62d86
        fmul st(0), st(2) // 00b62d89
        fld dword ptr [ecx + 0x18] // 00b62d8b
        fmul st(0), st(4) // 00b62d8e
        faddp st(1), st(0) // 00b62d90
        fld dword ptr [ecx + 0x28] // 00b62d92
        fmul st(0), st(5) // 00b62d95
        faddp st(1), st(0) // 00b62d97
        fld dword ptr [ecx + 0x38] // 00b62d99
        fmul st(0), st(2) // 00b62d9c
        faddp st(1), st(0) // 00b62d9e
        fstp dword ptr [eax + 8] // 00b62da0
        fld dword ptr [ecx + 0xc] // 00b62da3
        fmulp st(2), st(0) // 00b62da6
        fld dword ptr [ecx + 0x1c] // 00b62da8
        fmulp st(3), st(0) // 00b62dab
        fxch st(1) // 00b62dad
        faddp st(2), st(0) // 00b62daf
        fld dword ptr [ecx + 0x2c] // 00b62db1
        fmulp st(3), st(0) // 00b62db4
        fxch st(1) // 00b62db6
        faddp st(2), st(0) // 00b62db8
        fmul dword ptr [ecx + 0x3c] // 00b62dba
        faddp st(1), st(0) // 00b62dbd
        fstp dword ptr [eax + 0xc] // 00b62dbf
        add esp, 0x10 // 00b62dc2
        ret 8 // 00b62dc5
    }
}

std::int32_t scale_and_truncate(float value, bool sse2) {
    const double scale = 255.0; // verified native double00ce4b48
    double spill;
    std::int32_t result;
    __asm {
        fld value
        fmul scale
        cmp sse2, 0
        je x87_path
        fstp spill
        cvttsd2si eax, spill
        jmp converted
    x87_path:
        call x87_truncate_kernel
    converted:
        mov result, eax
    }
    return result;
}
DWORD low_plane_mask(std::uint32_t count) { return (1u << (count & 31u)) - 1u; }
}

void invert_camera_general_00b632d0(CameraMatrix& dst, const CameraMatrix& src) {
    inverse_kernel(dst.data(), src.data());
}
void transform_camera_plane_00b65ba0(CameraPlane& dst, const CameraPlane& src,
    const CameraMatrix& matrix) {
    CameraPlane temporary;
    transform_kernel(src.data(), nullptr, temporary.data(), matrix.data());
    std::memcpy(dst.data(), temporary.data(), sizeof(temporary));
}
void extract_camera_frustum_00b653f0(std::array<CameraPlane, 6>& dst,
    const CameraMatrix& matrix) {
    extract_kernel(dst.front().data(), nullptr, matrix.data());
}
const CameraMatrix& get_camera_inverse_view_projection_00b70510(CameraFrameState& frame) {
    auto& flags = frame.camera.projection.valid_flags;
    if (!(flags & 0x20u)) {
        CameraMatrix temporary;
        invert_camera_general_00b632d0(temporary,
            get_camera_view_projection_00b70490(frame.camera));
        copy_camera_matrix_004134f0(frame.inverse_view_projection, temporary);
        flags |= 0x20u;
    }
    return frame.inverse_view_projection;
}
void assign_camera_frustum_planes_00b658e0(CameraPlaneSet& output,
    const std::array<CameraPlane, 6>& planes, std::uint32_t flags) {
    for (std::size_t i = 0; i < planes.size(); ++i) {
        const float* source = planes[i].data();
        float* destination = output.planes[i].coefficients.data();
        __asm {
            mov eax, source
            mov ecx, destination
            fld dword ptr [eax]
            fstp dword ptr [ecx]
            fld dword ptr [eax+4]
            fstp dword ptr [ecx+4]
            fld dword ptr [eax+8]
            fstp dword ptr [ecx+8]
            fld dword ptr [eax+12]
            fstp dword ptr [ecx+12]
        }
        output.planes[i].flags = flags;
    }
}
const CameraPlaneSet& get_camera_frustum_00b70710(CameraFrameState& frame) {
    auto& flags = frame.camera.projection.valid_flags;
    if (!(flags & 4u)) {
        flags |= 4u; // native publishes before the nested view-projection getter
        std::array<CameraPlane, 6> planes;
        extract_camera_frustum_00b653f0(planes,
            get_camera_view_projection_00b70490(frame.camera));
        assign_camera_frustum_planes_00b658e0(frame.frustum, planes, 7u);
    }
    return frame.frustum;
}
D3DCOLOR convert_camera_ambient_004fb850(const CameraPlane& rgba, bool sse2) {
    DWORD result = 0;
    constexpr unsigned shifts[] = {16, 8, 0, 24};
    for (std::size_t i = 0; i < rgba.size(); ++i) {
        const auto integer = scale_and_truncate(rgba[i], sse2);
        const auto channel = static_cast<DWORD>(std::clamp(integer, 0, 255));
        result |= channel << shifts[i];
    }
    return result;
}

struct D3D9CameraFrameAccess::Guard {
    D3D9StateCache& owner;
    bool entered{};
    explicit Guard(D3D9StateCache& cache) : owner(cache) {
        if (owner.synchronization_.enabled) entered = owner.enter_00b33ad0();
    }
    ~Guard() {
        // Native leave tests current mode and ignores its saved enter result.
        if (owner.synchronization_.enabled) owner.leave_00b33b00(entered);
    }
};
HRESULT D3D9CameraFrameAccess::bind_viewport_00b26770(const CameraViewport& value) {
    Guard guard(cache_);
    const D3DVIEWPORT9 viewport{value.x, value.y, value.width, value.height, 0.0f, 1.0f};
    const auto viewport_result = cache_.device_.SetViewport(&viewport);
    ++viewport_calls_;
    viewport_ = &value;
    cache_.set_render_state_00b24460(D3DRS_SCISSORTESTENABLE, value.scissor_enabled);
    // Re-read after the setter, as the native getter does.
    if (value.scissor_enabled) {
        const auto scissor_result = cache_.device_.SetScissorRect(&value.scissor);
        if (FAILED(viewport_result)) return viewport_result;
        return scissor_result;
    }
    return viewport_result;
}
HRESULT D3D9CameraFrameAccess::clear_00b21430(DWORD count, const D3DRECT* rectangles,
    DWORD flags, const D3DCOLOR* color, float depth, DWORD stencil) {
    if (!flags) return S_FALSE; // no guard and no color dereference on this path
    Guard guard(cache_);
    float copied_depth;
    __asm {
        fld depth
        fstp copied_depth
    }
    const auto result = cache_.device_.Clear(count, rectangles, flags, *color, copied_depth, stencil);
    ++clear_calls_;
    return result;
}
HRESULT D3D9CameraFrameAccess::set_clip_plane_00b23e50(UINT index,
    const CameraPlane& plane) {
    Guard guard(cache_);
    const float* source = plane.data();
    float* destination = clip_planes_[index].data();
    // Native copies via x87: masked signaling NaNs quiet in the cache, while
    // SetClipPlane still receives the original input pointer and its bits.
    __asm {
        mov eax, source
        mov edx, destination
        fld dword ptr [eax]
        fstp dword ptr [edx]
        fld dword ptr [eax+4]
        fstp dword ptr [edx+4]
        fld dword ptr [eax+8]
        fstp dword ptr [edx+8]
        fld dword ptr [eax+12]
        fstp dword ptr [edx+12]
    }
    return cache_.device_.SetClipPlane(index, plane.data());
}
void D3D9CameraFrameAccess::restore_pending_planes_00b25080() {
    cache_.set_render_state_00b24460(D3DRS_CLIPPLANEENABLE, low_plane_mask(pending_plane_count_));
    active_plane_count_ = pending_plane_count_;
}
void D3D9CameraFrameAccess::append_plane_00b25040(const CameraPlane& plane) {
    set_clip_plane_00b23e50(active_plane_count_, plane);
    ++active_plane_count_;
    cache_.set_render_state_00b24460(D3DRS_CLIPPLANEENABLE, low_plane_mask(active_plane_count_));
}
void D3D9CameraFrameAccess::prepare_camera_00b285a0(CameraFrameState& frame) {
    cache_.set_render_state_00b24460(D3DRS_CLIPPLANEENABLE, 0);
    get_camera_inverse_view_projection_00b70510(frame);
    const auto& camera_planes = get_camera_frustum_00b70710(frame);
    static_assert(sizeof(CameraPlaneRecord) == 20 && sizeof(CameraPlaneSet) == 324);
    for (std::size_t i = 0; i < plane_set_.planes.size(); ++i) {
        const float* source = camera_planes.planes[i].coefficients.data();
        float* destination = plane_set_.planes[i].coefficients.data();
        __asm {
            mov eax, source
            mov edx, destination
            fld dword ptr [eax]
            fstp dword ptr [edx]
            fld dword ptr [eax+4]
            fstp dword ptr [edx+4]
            fld dword ptr [eax+8]
            fstp dword ptr [edx+8]
            fld dword ptr [eax+12]
            fstp dword ptr [edx+12]
        }
        plane_set_.planes[i].flags = camera_planes.planes[i].flags;
    }
    plane_set_.count = camera_planes.count;
    active_plane_count_ = 0;
    pending_plane_count_ = 0;
    if (user_clip_planes_supported) {
        for (std::uint32_t i = 0; i < plane_set_.count; ++i) {
            const auto flags = plane_set_.planes[i].flags;
            if ((flags & 4u) || !(flags & 2u)) continue;
            CameraMatrix inverse_view, inverse_projection, product;
            invert_camera_affine_00b63b30(inverse_view,
                get_camera_view_00b6fcb0(frame.camera.transform));
            invert_camera_general_00b632d0(inverse_projection,
                get_camera_projection_00b6fcf0(frame.camera.projection));
            multiply_camera_matrices_00413920(product, inverse_projection, inverse_view);
            for (std::size_t row = 0; row != 4; ++row)
                for (std::size_t column = row + 1; column != 4; ++column)
                    std::swap(product[row * 4 + column], product[column * 4 + row]);
            CameraPlane plane;
            transform_camera_plane_00b65ba0(plane, plane_set_.planes[i].coefficients, product);
            set_clip_plane_00b23e50(pending_plane_count_, plane);
            ++pending_plane_count_;
        }
        restore_pending_planes_00b25080();
    }
    if (frame.fog_184)
        cache_.set_render_state_00b24460(D3DRS_AMBIENT,
            convert_camera_ambient_004fb850(frame.fog_184->color_08, sse2_color_truncation));
}
void D3D9CameraFrameAccess::execute_camera_command_00b71360(CameraFrameState& frame) {
    if (!frame.enabled) return;
    prepare_camera_00b285a0(frame);
    bind_viewport_00b26770(*frame.viewport);
    const float* source_depth = &frame.clear_depth;
    float command_depth;
    __asm {
        mov eax, source_depth
        fld dword ptr [eax]
        fstp command_depth
    }
    clear_00b21430(0, nullptr, frame.clear_flags, &frame.clear_color,
        command_depth, frame.clear_stencil);
}
}
