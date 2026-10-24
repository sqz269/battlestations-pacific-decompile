#pragma once

#include "bsp/native_traceline_render.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene rendering requires MSVC Win32 x87/SSE and raw stack words.
#endif

namespace bsp {
struct NativeCameraEnvironment;
struct NativeModelEnvironment;
struct NativeMatrixIndexedModelEnvironment;
struct NativeGroupEnvironment;
struct NativeTracelineLifetimeAccess;

// Prepare before native work from the SAME real owner environments. The model
// environment underlying the actual Traceline must use the identical node and
// model table views. No table contents are copied or fabricated. All reachable
// owners with one numeric profile must use its one canonical actual view.
// Fixed pointer bindings/backing storage outlive traversal and reentry; table
// words themselves remain current reads. This record owns nothing.
struct NativeRender20Profiles {
    const volatile std::uint32_t* const node_00d62c88;
    const volatile std::uint32_t* const camera_00d62cf0;
    const volatile std::uint32_t* const model_00d62de8;
    const volatile std::uint32_t* const traceline_00d0c928;
    const volatile std::uint32_t* const group_00d634f8;
    const NativeGroupEnvironment* const group_environment;
    // Optional genuine derived model view. Its +20 is the SAME B748E0 body;
    // the actual owner/pool/palette are supplied by the matrix model domain.
    const volatile std::uint32_t* const matrix_model_00d63590;

    NativeRender20Profiles(const NativeCameraEnvironment&,
        const NativeModelEnvironment&, const NativeTracelineLifetimeAccess&,
        const NativeModelEnvironment& actual_traceline_model_environment,
        const NativeGroupEnvironment* actual_group_environment = nullptr,
        const NativeMatrixIndexedModelEnvironment* actual_matrix_environment = nullptr);
    // Ordinary model-only application composition. No Traceline lifetime is
    // manufactured: D0C928 remains unsupported in this narrower domain.
    NativeRender20Profiles(const NativeCameraEnvironment&,
        const NativeModelEnvironment&,
        const NativeGroupEnvironment* actual_group_environment = nullptr,
        const NativeMatrixIndexedModelEnvironment* actual_matrix_environment = nullptr);
};

// Complete B72190[84]. Native ECX scene, stack context, RET4; adds EDX access.
// Direct naked entry: its ORIGINAL context word becomes the captured scene+18
// scalar. Capture context/camera even for an empty list. Current profile/slot20
// reads retain their native positions around x87; next is read after dispatch.
void __fastcall traverse_native_scene_00b72190(void* actual_scene,
    const NativeTracelineRenderAccess*, void* actual_context);

// Complete B6D990[105]. Native four public words/RET10; adds EDX access.
// ORIGINAL flags word is the x87 product spill, after capturing flags in EBX.
// Original visibility is reloaded each child, never accumulated/overwritten.
void __fastcall render_native_node_children_00b6d990(void* actual_node,
    const NativeTracelineRenderAccess*, void* actual_context,
    float lod, float visibility, std::uint32_t flags);

// Complete B6FB80[36]. Native four words/RET10, two FLD32/FSTP32 round-trips
// before the actual base-node call. Direct naked entry, not a value wrapper.
void __fastcall render_native_camera_children_00b6fb80(void* actual_camera,
    const NativeTracelineRenderAccess*, void* actual_context,
    float lod, float visibility, std::uint32_t flags);

// Complete B8EF10[110]. Native ECX cGroup, original four stack words/RET10;
// EDX adds the SAME render access. Current sphere/profile/slot reads, native
// x87/SSE clamp and original context/flags argument spill aliases are retained.
void __fastcall render_native_group_children_00b8ef10(void* actual_group,
    const NativeTracelineRenderAccess*, void* actual_context,
    float lod, float visibility, std::uint32_t flags);

// B8F100: byte175 selects the instruction-identical raw Model static/cache
// path or the existing canonical B8EBE0 attachment-array aggregation. The
// dynamic arm needs the SAME Group companion and a bound group_environment;
// its existing C++ companion/array projection is not a raw pointer/FH3 ABI.
const void* __fastcall get_native_group_world_sphere_00b8f100(void* actual_group,
    const NativeTracelineRenderAccess*);

// Shared source-only selector. Naked/integer-only, no profile lookup or FP.
// Old model/Traceline bridge keeps its late profile callback; node/camera routes
// require the new preflighted binding, preserving legacy null-binding fallback.
using NativeRender20Body = void (__fastcall*)(void*,
    const NativeTracelineRenderAccess*, void*, float, float, std::uint32_t);
NativeRender20Body __fastcall select_native_render20_body(std::uint32_t target,
    const NativeTracelineRenderAccess*) noexcept;

// New entries require live access, zero cell, canonical render20_profiles and
// existing real services. Group is optional; D0C8C8/__purecall is not admitted.
// Unknown mapped targets use the existing actual application dispatch after
// balanced FP spills. That extra C++ frame/FP/exception path does not preserve
// original stack alias/virtual ABI/FH3/hardware-fault equivalence. Missing-profile
// diagnostics are source-only; no successful unknown-target fallback exists.
} // namespace bsp
