#pragma once
#include <cstdint>

namespace bsp {
struct NativeRendererTextureBindingContext;
struct NativeInstanceCollectionAccess;

// Borrow the SAME raw bank storage used by pass execution. Each header is
// the DWORD immediately before its bank, as at0108EBF0/0108DBE8. Current
// publications and original-token profiles are never copied or re-owned.
struct NativeMaterialConstantBuildContext {
    void* bank0108EBF4;
    void* bank0108DBEC;
    void* const volatile& actual_renderer_00f8d394;
    void* const volatile& actual_service_00f8d39c;
    void* const volatile* actual_shadow_target_00f8bbf0;
    NativeRendererTextureBindingContext* texture_binding; // Required only when binding is reached.
    const NativeInstanceCollectionAccess& threshold;
    const volatile std::uint32_t* actual_types_010900fc; // Three live DWORDs.
    const volatile float& actual_fraction_00d7a238;
    const volatile float& actual_one_00d7a24c;
    const volatile float& actual_unsigned_bias_00ce3978;
    const volatile std::uint32_t* renderer_profile_00d5f0a8;
    const volatile std::uint32_t* logical_vertex_profile_00d61d6c;
    const volatile std::uint32_t* shadow_profile_00d5b5d8;
    const volatile std::uint32_t* animator_profile_00d62eb0;
    const volatile std::uint32_t* caustics_profile_00d64478;
    const volatile std::uint32_t* shore_profile_00d644b4;
};

// One fresh persistent frame per invocation. Failure preserves its last
// original callsite and every preceding native effect. Existing child owner
// contexts retain their own acquisitions and failure frames; this frame never
// acknowledges, disarms, retries, releases or replaces them.
struct NativeMaterialConstantBuildFrame {
    enum class Phase : std::uint8_t { fresh, running, completed, failed };
    Phase phase{Phase::fresh};
    std::uint32_t reached_callsite{};
};

// Full B42350..B4340F. Original ECX actual pass, stack entry/unused override,
// RET8. New source ABI adds borrowed context/frame. Current fields, unsigned
// loops, x87 float crossings, SSE bit copies and partial writes are retained.
// Reached virtual targets are dispatched only to established concrete bodies.
// Unsupported profiles are explicit source-domain failures at that callsite.
// Raw extents/indices, clear DF and native FP environment remain preconditions.
// Original private stack aliases, incidental registers, native exception ABI,
// whole-parent composition and gameplay equivalence are separately unproved.
void build_native_material_constants_00b42350(void* actual_pass,
    void* actual_entry, void* actual_override,
    NativeMaterialConstantBuildContext&, NativeMaterialConstantBuildFrame&);
} // namespace bsp
