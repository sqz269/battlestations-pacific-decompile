#pragma once
#include "bsp/camera_frame_state.hpp"
#include "bsp/material_entry_dispatch.hpp"

namespace bsp {
using EffectPlaneVector = std::array<float, 3>;

// These mappings return the actual retained owners, without changing state.
// Only queue_singleton_004c11f0 is an operation: it must use the actual shared
// queue service, including its lazy acquisition if required. No alternate queue,
// camera, scene transform, renderer cache or frame companion may be created.
// The bindings are a host identity boundary, not newly inferred native virtuals.
class MaterialEffectPlaneBindings {
public:
    virtual ~MaterialEffectPlaneBindings() = default;
    virtual bool queue_singleton_004c11f0(RenderCommandQueue*&, std::string&) = 0;
    virtual bool scene_transform(RenderCommandReference&, CameraTransform*&, std::string&) = 0;
    virtual bool camera_frame(CameraTransform&, CameraFrameState*&, std::string&) = 0;
    virtual bool renderer_frame(D3D9StateCache&, D3D9CameraFrameAccess*&, std::string&) = 0;
};

// Native ECX=queue, EAX=[ECX+30], RET. Borrowed, without a reference increment.
RenderCommandContext* get_material_plane_context_00b1bfa0(const RenderCommandQueue&) noexcept;

// ONLY normalize=false branch of0042D0D0. Native ECX=destination, EDX=vector,
// stack=matrix,normalize byte in DWORD slot, RET8, EAX=destination. Preserves
// x87 order/spills and final MOVSS copies; no translation or normalization.
// Direct actual-storage entry for callers that select normalize=false. Source
// ECX=destination, EDX=vector, one stacked matrix pointer, EAX=destination, RET4.
// Its unchanged native arithmetic kernel accepts the caller's actual float
// storage and preserves alias order. The native normalize=true path is absent.
float* __fastcall transform_native_effect_direction_0042d0d0_no_normalize(
    float* destination, const float* source, const float* matrix);
void transform_effect_direction_0042d0d0_no_normalize(EffectPlaneVector&,
    const EffectPlaneVector&, const CameraMatrix&);
// Native ECX=destination, EDX=source, EAX=destination, RET. Forward REP MOVSD
// then six mixed FLD/FSTP and MOVSS swaps, including asymmetric NaN handling.
void copy_transpose_effect_matrix_00b23360(CameraMatrix&, const CameraMatrix&);

// Delegates the existing MaterialEntryOperations plane methods to concrete math
// and the SAME companion of the renderer captured at00B4485A. New C++ ABI.
bool restore_material_effect_planes_00b25080(MaterialEffectPlaneBindings&,
    D3D9StateCache& captured_renderer, std::string&);
// Selected branch00B448A3..00B44A53. The caller already performed effect-change,
// extra_clip_plane and positive-distance gates. gated_distance is deliberately
// not the arithmetic operand:00B448FC reloads effect.clip_distance using the
// captured entry camera's LIVE render_mode after queue/scene-world operations.
// Owners stay alive across callbacks. Actual entry.camera is captured before
// queue acquisition; current_context.scene is captured after it. Neither is
// replaced with context.camera or with an entry/model transform.
// Failure denotes a missing/inconsistent binding, invalid mode, or an append
// beyond fourteen planes; earlier refreshes persist. COM failure does not stop append.
bool construct_and_append_effect_plane_00b448a3(MaterialEffectPlaneBindings&,
    D3D9StateCache& captured_renderer, MaterialEntryEffect&, InstanceRenderEntry&,
    float gated_distance, std::string&);
}
