#pragma once
#include "bsp/native_renderer_generated_model.hpp"
#include "bsp/native_system_constant_gather.hpp"
#include "bsp/native_traceline_render.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/native_material_constant_build.hpp"
#include "bsp/native_material_pass_execution.hpp"
#include "bsp/native_debug_sphere_vertices.hpp"
#include "bsp/native_node_raw_transform.hpp"

namespace bsp {
struct NativeRendererDebugRecords24Context {
    NativeRendererGeneratedModelContext& generated;
    const NativeRendererRawModelBinding& raw_models;
    NativeLogicalBufferMappingContext& mapping;
    const NativeTracelineRenderAccess& render_entry;
    const NativeSystemConstantGatherContext& gather;
    NativeMaterialPassExecutionContext& pass;
    const volatile double& actual_angle_step_00cec730;
};

// Caller supplies one fresh persistent row for EACH active camera record the
// live loop may reach, including rows appended by callbacks. No host allocation
// is inserted into the native loop. The gather and pass frames retain their
// own actual dependency acquisitions and initialized scratch preimages. Each
// row embeds a distinct constant-builder frame and a pass frame bound to it;
// a completed pass/constant frame cannot be aliased across active records.
struct NativeRendererDebugRecord24Frame {
    NativeSystemConstantGatherFrame gather;
    NativeMaterialConstantBuildFrame constants;
    NativeMaterialPassExecutionFrame pass;
    NativeRendererGeneratedModelAcquired generated;
    void* unconsumed_model{};
    bool started{};
    bool complete{};
    explicit NativeRendererDebugRecord24Frame(const void* native_gather_preimage)
        : gather{native_gather_preimage},pass(constants) {}
    NativeRendererDebugRecord24Frame(const NativeRendererDebugRecord24Frame&) = delete;
    NativeRendererDebugRecord24Frame& operator=(const NativeRendererDebugRecord24Frame&) = delete;
};
struct NativeRendererDebugRecords24Frame {
    NativeRendererGeneratedModelAcquired cached_model;
    NativeRendererDebugRecord24Frame* const* records;
    std::uint32_t record_frame_count;
    std::uint32_t used_record_frames{};
};

// Complete B2BB90..B2C274: ECX renderer, plain RET. Source adds borrowed EDX
// context and a stacked persistent diagnostic frame. Actual +1D0C/+1D10/+1D14
// is the native stride18h header; each record has center.xyz/radius, selector,
// camera at+14. Initially zero count returns without accessing bindings.
// Nonzero count lazily publishes+19E4, sets its CURRENT model local transform,
// draws only camera+198==0 records, then reserves on negative capacity and
// drains the CURRENT positive count. Cached/record pointers and count reloads
// follow the listing; negative initial count still performs the cold stage.
//
// Two string groups have the exact normal/unwind destruction schedule. The
// cached and per-record generated models have no outer ownership guard. After
// string cleanup, exceptions retain the transient model, lock/entry/counter and
// all earlier effects; only the reached normal B6DFA0 consumes its creator.
// Acquired frames must survive failures and must never be replayed.
//
// raw_models borrows the same AA8/AA4/AA0 name cells and model bound constants.
// Its model environment keeps actual_names null through destruction. Temporary
// name headers remain live through construction; the raw node copies their data.
// Same actual raw model/geometry/pool/string/device/profile/entry domains are
// required. Full generated-model cold compilation remains an explicit child
// dependency. The canonical model transform borrows its native fields; any
// reached nonnull+A0 uses the canonical CURRENT raw profile resolver and the
// literal B6DBC0 body; unknown targets fail after prior native matrix/flag writes.
// No typed shadow/renderer/geometry copy or successful fallback.
// Original private-stack/FH3/hardware-fault ABI and game rendering are unproved.
void __fastcall draw_native_renderer_debug_records24_00b2bb90(void* actual_renderer,
    NativeRendererDebugRecords24Context*, NativeRendererDebugRecords24Frame*);

} // namespace bsp
