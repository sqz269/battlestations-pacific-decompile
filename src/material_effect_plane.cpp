#include "bsp/material_effect_plane.hpp"
#include "bsp/camera_inverse.hpp"
#include "bsp/camera_multiply.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Material effect plane reconstruction requires MSVC Win32 x87/SSE assembly.
#endif

namespace bsp {
namespace {
// Native00D7A208 is80000000, not positive zero.00D7A24C is3F800000.
const float negative_zero = -0.0f;
} // namespace

// One stack argument in this host kernel; normalize=false is selected by its
// interface. The arithmetic body and spill offsets are those of0042D0D0.
__declspec(naked) float* __fastcall transform_native_effect_direction_0042d0d0_no_normalize(
    float*, const float*, const float*) {
    __asm {
        sub esp, 0x18
        fld dword ptr [edx + 4]
        fstp dword ptr [esp]
        mov eax, dword ptr [esp + 0x1c]
        fld dword ptr [edx]
        push esi
        fstp dword ptr [esp + 8]
        mov esi, ecx
        fld dword ptr [edx + 8]
        fstp dword ptr [esp + 0xc]
        fld dword ptr [eax + 0x10]
        fld dword ptr [esp + 4]
        fld st(0)
        fmulp st(2), st(0)
        fld dword ptr [eax]
        fld dword ptr [esp + 8]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(3)
        faddp st(1), st(0)
        fld dword ptr [eax + 0x20]
        fld dword ptr [esp + 0xc]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(2)
        faddp st(1), st(0)
        fstp dword ptr [esp + 0x10]
        fld dword ptr [eax + 4]
        fmul st(0), st(3)
        fld dword ptr [eax + 0x14]
        fmul st(0), st(3)
        faddp st(1), st(0)
        fld dword ptr [eax + 0x24]
        fmul st(0), st(2)
        faddp st(1), st(0)
        fstp dword ptr [esp + 0x14]
        fld dword ptr [eax + 8]
        fmulp st(3), st(0)
        fld dword ptr [eax + 0x18]
        fmulp st(2), st(0)
        fxch st(2)
        faddp st(1), st(0)
        fld dword ptr [eax + 0x28]
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp + 0x18]
        movss xmm0, dword ptr [esp + 0x10]
        movss dword ptr [esi], xmm0
        movss xmm0, dword ptr [esp + 0x14]
        movss dword ptr [esi + 4], xmm0
        movss xmm0, dword ptr [esp + 0x18]
        movss dword ptr [esi + 8], xmm0
        mov eax, esi
        pop esi
        add esp, 0x18
        ret 4
    }
}

namespace {
//00B448FC..00B44959; retain the native distance spill and simultaneous x87
// products/SSE negative-zero subtraction. Inputs/destinations here are disjoint.
void scale_and_negate(const float& live_distance, const EffectPlaneVector& direction,
    EffectPlaneVector& scaled, EffectPlaneVector& normal) {
    const float* distance = &live_distance;
    const float* src = direction.data();
    float* dst = scaled.data();
    float* normal_words = normal.data();
    float spill;
    __asm {
        mov eax, distance
        mov edx, src
        mov ecx, dst
        fld dword ptr [eax]
        movss xmm0, negative_zero
        fstp spill
        movaps xmm1, xmm0
        fld dword ptr [edx]
        subss xmm1, dword ptr [edx]
        fld spill
        movaps xmm2, xmm0
        subss xmm2, dword ptr [edx + 4]
        fld st(0)
        subss xmm0, dword ptr [edx + 8]
        fmulp st(2), st(0)
        fxch st(1)
        mov eax, normal_words
        movss dword ptr [eax], xmm1
        movss dword ptr [eax + 4], xmm2
        fstp dword ptr [ecx]
        movss dword ptr [eax + 8], xmm0
        fld dword ptr [edx + 4]
        fmul st(0), st(1)
        fstp dword ptr [ecx + 4]
        fmul dword ptr [edx + 8]
        fstp dword ptr [ecx + 8]
    }
}

//00B44972..00B449F4. Do not contract the dot product or move its final
// float32 spill past FCHS; native y*x87normal.y then x then z order is observable.
void finish_world_plane(CameraPlane& plane, const CameraMatrix& world,
    const EffectPlaneVector& scaled, const EffectPlaneVector& normal) {
    const float* translation = world.data() + 12;
    const float* scaled_source = scaled.data();
    const float* normal_words = normal.data();
    float* dst = plane.data();
    float copied[3], point[3], spill;
    __asm {
        mov edx, translation
        mov ecx, normal_words
        mov eax, dst
        fld dword ptr [edx]
        movss xmm0, dword ptr [ecx + 8]
        movss xmm1, dword ptr [ecx]
        movss xmm2, dword ptr [ecx + 4]
        fstp dword ptr copied[0]
        fld dword ptr [edx + 4]
        fstp dword ptr copied[4]
        fld dword ptr [edx + 8]
        movss dword ptr [eax], xmm1
        fstp dword ptr copied[8]
        movss dword ptr [eax + 4], xmm2
        mov edx, scaled_source
        fld dword ptr [edx]
        movss dword ptr [eax + 8], xmm0
        fadd dword ptr copied[0]
        fstp dword ptr point[0]
        fld dword ptr copied[4]
        fadd dword ptr [edx + 4]
        fstp dword ptr point[4]
        fld dword ptr copied[8]
        fadd dword ptr [edx + 8]
        fstp dword ptr point[8]
        fld dword ptr point[4]
        fmul dword ptr [ecx + 4]
        fld dword ptr point[0]
        fmul dword ptr [ecx]
        faddp st(1), st(0)
        fld dword ptr point[8]
        fmul dword ptr [ecx + 8]
        faddp st(1), st(0)
        fstp spill
        fld spill
        fchs
        fstp dword ptr [eax + 12]
    }
}

bool bound_renderer_frame(MaterialEffectPlaneBindings& bindings, D3D9StateCache& renderer,
    D3D9CameraFrameAccess*& frame, std::string& error) {
    if (!bindings.renderer_frame(renderer, frame, error)) return false;
    if (!frame || &frame->cache() != &renderer) {
        error = "Effect plane requires the captured renderer's actual frame companion";
        return false;
    }
    return true;
}
}

RenderCommandContext* get_material_plane_context_00b1bfa0(const RenderCommandQueue& queue) noexcept {
    return queue.current_context;
}

void transform_effect_direction_0042d0d0_no_normalize(EffectPlaneVector& destination,
    const EffectPlaneVector& source, const CameraMatrix& matrix) {
    transform_native_effect_direction_0042d0d0_no_normalize(
        destination.data(), source.data(), matrix.data());
}

void copy_transpose_effect_matrix_00b23360(CameraMatrix& destination, const CameraMatrix& source) {
    float* dst = destination.data();
    const float* src = source.data();
    __asm {
        mov eax, dst
        mov edx, src
        push esi
        push edi
        mov ecx, 0x10
        mov esi, edx
        mov edi, eax
        rep movsd
        fld dword ptr [eax + 0x10]
        movss xmm0, dword ptr [eax + 4]
        pop edi
        fstp dword ptr [eax + 4]
        movss dword ptr [eax + 0x10], xmm0
        movss xmm0, dword ptr [eax + 8]
        fld dword ptr [eax + 0x20]
        fstp dword ptr [eax + 8]
        movss dword ptr [eax + 0x20], xmm0
        movss xmm0, dword ptr [eax + 0xc]
        fld dword ptr [eax + 0x30]
        fstp dword ptr [eax + 0xc]
        movss dword ptr [eax + 0x30], xmm0
        movss xmm0, dword ptr [eax + 0x18]
        fld dword ptr [eax + 0x24]
        fstp dword ptr [eax + 0x18]
        movss dword ptr [eax + 0x24], xmm0
        movss xmm0, dword ptr [eax + 0x1c]
        fld dword ptr [eax + 0x34]
        fstp dword ptr [eax + 0x1c]
        movss dword ptr [eax + 0x34], xmm0
        fld dword ptr [eax + 0x38]
        movss xmm0, dword ptr [eax + 0x2c]
        fstp dword ptr [eax + 0x2c]
        movss dword ptr [eax + 0x38], xmm0
        pop esi
    }
}

bool restore_material_effect_planes_00b25080(MaterialEffectPlaneBindings& bindings,
    D3D9StateCache& captured_renderer, std::string& error) {
    D3D9CameraFrameAccess* frame = nullptr;
    if (!bound_renderer_frame(bindings, captured_renderer, frame, error)) return false;
    frame->restore_pending_planes_00b25080();
    return true;
}

bool construct_and_append_effect_plane_00b448a3(MaterialEffectPlaneBindings& bindings,
    D3D9StateCache& captured_renderer, MaterialEntryEffect& effect, InstanceRenderEntry& entry,
    float gated_distance, std::string& error) {
    static_cast<void>(gated_distance); // Native re-reads distance after owner/world operations.
    CameraTransform* const captured_camera = entry.camera; // EBX at00B4487E
    if (!captured_camera) {
        error = "Effect plane entry has no camera owner";
        return false;
    }
    CameraFrameState* camera_frame = nullptr;
    if (!bindings.camera_frame(*captured_camera, camera_frame, error)) return false;
    if (!camera_frame || &camera_frame->camera.transform != captured_camera) {
        error = "Effect plane requires the captured entry camera's actual frame state";
        return false;
    }
    RenderCommandQueue* queue = nullptr;
    if (!bindings.queue_singleton_004c11f0(queue, error)) return false;
    RenderCommandContext* const context = queue ? get_material_plane_context_00b1bfa0(*queue) : nullptr;
    RenderCommandReference* const scene = context ? context->scene : nullptr; //00B448AF
    if (!scene) {
        error = "Effect plane requires the actual queue current context scene owner";
        return false;
    }
    CameraTransform* transform = nullptr;
    if (!bindings.scene_transform(*scene, transform, error)) return false;
    if (!transform) {
        error = "Effect plane context scene has no transform binding";
        return false;
    }
    if (!(transform->valid_flags & 2u)) refresh_camera_world_00b6db70(*transform);
    const EffectPlaneVector basis{0.0f, 0.0f, 1.0f};
    EffectPlaneVector direction, scaled, normal;
    transform_effect_direction_0042d0d0_no_normalize(direction, basis, transform->world);
    const bool world_was_valid = (transform->valid_flags & 2u) != 0; // TEST00B448F2
    const auto mode = camera_frame->render_mode; // Reload00B448F6, not gate-time mode.
    if (mode >= effect.clip_distance.size()) {
        error = "Effect plane camera mode exceeds the actual fourteen distance words";
        return false;
    }
    scale_and_negate(effect.clip_distance[mode], direction, scaled, normal);
    if (!world_was_valid) refresh_camera_world_00b6db70(*transform);
    CameraPlane world_plane, clip_plane;
    finish_world_plane(world_plane, transform->world, scaled, normal);
    CameraMatrix inverse_view, inverse_projection, product, transposed;
    invert_camera_affine_00b63b30(inverse_view, get_camera_view_00b6fcb0(*captured_camera));
    invert_camera_general_00b632d0(inverse_projection,
        get_camera_projection_00b6fcf0(camera_frame->camera.projection));
    multiply_camera_matrices_00413920(product, inverse_projection, inverse_view);
    copy_transpose_effect_matrix_00b23360(transposed, product);
    transform_camera_plane_00b65ba0(clip_plane, world_plane, transposed);
    D3D9CameraFrameAccess* renderer_frame = nullptr;
    if (!bound_renderer_frame(bindings, captured_renderer, renderer_frame, error)) return false;
    if (renderer_frame->active_plane_count() >= renderer_frame->device_clip_planes().size()) {
        error = "Effect plane append exceeds the renderer's fourteen native plane slots";
        return false;
    }
    renderer_frame->append_plane_00b25040(clip_plane);
    return true;
}
}
