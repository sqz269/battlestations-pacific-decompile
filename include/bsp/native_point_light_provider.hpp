#pragma once
#include "bsp/native_point_light_owner.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {
// Dispatch and borrowed live type words only. The hierarchy, type predicates,
// native identities and physical arrays are those of the existing runtimes.
// Every traversed node must have a live actual binding; callbacks may reenter
// but must leave the current node/light alive. No concurrent hierarchy edits.
struct NativePointLightPopulationRuntime {
    NativeNodeDestructionRuntime& nodes;
    const volatile std::uint32_t& group_0109032c;
    const volatile std::uint32_t& object_01090034;
    const volatile std::uint32_t& token_01090344;
    const volatile std::uint32_t& token_010903c8;
    const CameraAxesCrtAccess& crt;
    const float* (*sphere_virtual48)(NativePointLightPopulationRuntime&,
        SceneNodeAttachment&);
    // Borrow live D7A280 for dynamic Group merges only. Cached/static/empty
    // and single-seed Group paths do not dereference this optional binding.
    const volatile double* half_00d7a280{};
};

// Actual B6E8C0 storage adapter; reads local+08 and writes SAME cached+13C.
// Uses the existing affine-sphere kernel and its finite-input, CW007F/027F
// numerical boundary. No second live ModelBounds object or native sphere.
const float* native_model_population_sphere_00b6e8c0(NativeNodeBinding&);
// Concrete current-profile dispatch for Model D62DE8 and Group D634F8.
// Group B8F100 includes dynamic cache misses through the SAME actual Group
// owner/attachment array. Static affine paths retain the kernel domain above;
// dynamic merging requires actual current child48, CRT and live D7A280.
// Other current profiles require their actual recovered virtual48 service.
const float* native_point_light_supported_world_sphere(
    NativePointLightPopulationRuntime&, SceneNodeAttachment&);

// Complete B6EF20 traversal. Native ECX node, stack actual light, RET4.
// Group distance > radius sum prunes children; accepted non-group type needs
// STRICT distance < sum to append. Equality/NaN and live child-next reloads are
// retained. Appends borrow the SAME raw light; there is no retain or dedup.
void populate_native_node_point_light_00b6ef20(NativePointLightPopulationRuntime&,
    CameraTransform&, NativePointLightOwner&);
// Complete B72140: ECX scene root list, stack actual light, RET4.
void populate_native_root_point_light_00b72140(NativePointLightPopulationRuntime&,
    RenderNodeRootList&, NativePointLightOwner&);
// Complete B7B090: ECX light, stack root list, RET4. Test signed count for
// equality with zero; ANY nonzero count skips the entire root-list read.
void populate_native_point_light_if_unlinked_00b7b090(
    NativePointLightPopulationRuntime&, NativePointLightOwner&, RenderNodeRootList&);

// B0CA40 PARTIAL: B0CAE1..B0CBA6, after the particle's virtual18 initializer.
// actual_light_60 is that SAME raw particle slot, possibly changed by world
// refresh. Null is inert. Absolute/relative coordinates write actual light
// +1EC/+1F0/+1F4, then radius8C is COMISS/JA-clamped to live D7A238 at+1F8.
// Relative byte and emitter transform refer to the SAME emitter node; radius
// refers to the captured template's+8C. Neither input is a new default.
// Existing canonical PointLight ownership must cover every reached 200h slot.
// Earlier particle init and B0CBA7..B0CBF5 shared locking/publication are NOT
// implemented here; invoke the complete population body at the real caller's
// synchronization point. These functions do not create a Text source Model.
void initialize_native_particle_point_light_volume_00b0ca40_fragment(
    NativePointLightLinksRuntime&, void* const volatile& actual_light_60,
    CameraTransform& emitter, const volatile std::uint8_t& relative_1b0,
    const float* position, const volatile std::uint32_t& radius_8c,
    const volatile std::uint32_t& minimum_00d7a238);
} // namespace bsp
