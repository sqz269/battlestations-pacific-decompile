#pragma once
#include "bsp/camera_pose.hpp"
#include "bsp/camera_type_bootstrap.hpp"
#include "bsp/native_camera_pool.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/native_viewport_owner.hpp"

namespace bsp {

// Exactly slot+174..457. The node prefix and pool identity at +458 are separate
// typed objects in the SAME 45Ch allocation. Construction preserves unwritten
// cache/plane/padding bytes, including values visible to early callbacks.
struct NativeCameraTailStorage {
    std::uint8_t byte_174;
    std::array<std::byte, 3> preserved_175;
    float scalar_178;
    std::uint8_t enabled_17c;
    std::array<std::byte, 3> preserved_17d;
    NativeViewportOwner* viewport_180;
    SystemFogOwner* fog_184;
    DWORD clear_flags_188;
    float clear_depth_18c;
    D3DCOLOR clear_color_190;
    DWORD clear_stencil_194;
    std::uint32_t render_mode_198, render_mask_19c;
    CameraAxis target_1a0, direction_1ac, zero_1b8;
    float fov_1c4, aspect_1c8, scalar_1cc, scalar_1d0;
    float near_1d4, far_1d8, scalar_1dc;
    CameraMatrix projection_1e0, view_projection_220, inverse_view_projection_260;
    CameraMatrix projection_cache_2a0;
    CameraPlane extra_plane_2e0;
    std::uint32_t valid_flags_2f0;
    CameraPlaneSet planes_2f4;
    void* retained_438;
    const CameraPlane* borrowed_context_43c;
    CameraAxis axis_y_440, axis_x_44c;
};
struct NativeCameraStorageView {
    NativeNodeStorage& node;
    NativeCameraTailStorage& camera;
};
struct NativeCameraConstants {
    const volatile std::uint32_t& hundred_00ce3d08;
    const volatile std::uint32_t& fov_00ce7d20;
    const volatile std::uint32_t& far_00d0c5f8;
    const volatile std::uint32_t& aspect_00d5bd98;
    const volatile std::uint32_t& scalar_00ce77fc;
    const volatile std::uint32_t& scalar_00ce3c88;
};
struct NativeCameraEnvironment {
    NativeCameraPool& pool_0108ffb0;
    NativeNodeDestructionRuntime& nodes;
    CameraTypeBootstrap& types;
    NativeViewportEnvironment& viewport;
    CameraViewportResolver& viewport_views;
    const CameraAxesCrtAccess& crt;
    NativeCameraConstants constants;
    // Actual immutable D62CF0 profile, at least22 words. The pose adapter loads
    // live +30/+34 words from this table after selecting the actual owner word.
    // Supported calls are B71400/B71460; another profile requires its own owner.
    const volatile std::uint32_t* vtable_00d62cf0;
    // Same actual node table during base construction/destruction reentry.
    const volatile std::uint32_t* vtable_00d62c88;
};

class NativeCameraOwner final {
public:
    // Establish typed lifetimes and stable borrowed views over an unused actual
    // pool slot, preserving its entire preimage. Registers only the external
    // scene association. Call B71A80 below before using any native fields.
    NativeCameraOwner(void* actual_slot, std::size_t slot_bytes, NativeCameraEnvironment&);
    NativeCameraOwner(const NativeCameraOwner&) = delete;
    NativeCameraOwner& operator=(const NativeCameraOwner&) = delete;
    // Abandoned preparation only removes its host association and ends the
    // preconstruction typed lifetimes. Live native destruction/pool return stay
    // explicit; a live owner must be destroyed before its companion goes away.
    ~NativeCameraOwner();

    enum class Phase { prepared, constructing, live, dead };
    NativeCameraStorageView storage;
    NativeCameraEnvironment& environment;
    NativeNodeBinding node;
    CameraProjection projection;
    CameraState camera;
    CameraFrameState frame;
    const CameraPoseAccess pose;
    Phase phase{Phase::prepared}; // external lifetime bookkeeping, no native word
};

// Original ECX=actual slot, stack name pointer, EAX=same slot, RET4. Runs the
// native node constructor, viewport allocation, plane setup and live pose chain
// in native order. Constructor unwind follows states0/1/2; it does not release
// published +180/+184. The caller still owns the physical slot on failure.
void* construct_native_camera_00b71a80(NativeCameraOwner&, const NativeString&);
// Actual header forwarded unchanged; the persistent runtime selects raw cleanup.
// Context, publication cells, providers and companions outlive retained references.
// Node and viewport must borrow the same actual D7A24C cell; mode/identity
// rejection leaves prepared lifetimes intact. Direct cleanup can throw; queue
// callbacks retain their noexcept boundary.
void* construct_native_camera_00b71a80(NativeCameraOwner&, const void* actual_name_header,
    const NativeNodeRawConstants&);
// Actual raw +180 publication, retain new, release captured old; same identity
// skips both counts. Supports the concrete D5E5F8 viewport profile.
void set_native_camera_viewport_00b71990(NativeCameraOwner&, NativeViewportOwner*);
// D62CF0 phase, live +180/+184/+438 releases, node base, dead association removal.
// Fog requires its concrete D63180 profile; nonnull +438 uses the runtime's real
// retained-owner binding and actual +04 count. Borrowed +43C is never released.
void destroy_native_camera_00b71f10(NativeCameraOwner&);
// Native ECX=owner, stack flags, EAX=original slot, RET4; return actual pool slot
// only when flags&1. A returned address can already designate free storage.
void* delete_native_camera_00b71fe0(NativeCameraOwner&, std::uint32_t flags);

} // namespace bsp
