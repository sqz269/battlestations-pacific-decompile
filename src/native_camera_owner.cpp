#include "bsp/native_camera_owner.hpp"
#include "bsp/camera_plane_initialization.hpp"
#include "bsp/system_fog_owner.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera ownership requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(NativeNodeStorage) == 0x174);
static_assert(sizeof(NativeCameraTailStorage) == 0x2e4);
#define BSP_CAMERA_OFFSET(member, native_offset) \
    static_assert(offsetof(NativeCameraTailStorage, member) + 0x174 == native_offset)
BSP_CAMERA_OFFSET(scalar_178, 0x178);
BSP_CAMERA_OFFSET(enabled_17c, 0x17c);
BSP_CAMERA_OFFSET(viewport_180, 0x180);
BSP_CAMERA_OFFSET(fog_184, 0x184);
BSP_CAMERA_OFFSET(clear_flags_188, 0x188);
BSP_CAMERA_OFFSET(clear_depth_18c, 0x18c);
BSP_CAMERA_OFFSET(clear_color_190, 0x190);
BSP_CAMERA_OFFSET(clear_stencil_194, 0x194);
BSP_CAMERA_OFFSET(render_mode_198, 0x198);
BSP_CAMERA_OFFSET(render_mask_19c, 0x19c);
BSP_CAMERA_OFFSET(target_1a0, 0x1a0);
BSP_CAMERA_OFFSET(direction_1ac, 0x1ac);
BSP_CAMERA_OFFSET(zero_1b8, 0x1b8);
BSP_CAMERA_OFFSET(fov_1c4, 0x1c4);
BSP_CAMERA_OFFSET(aspect_1c8, 0x1c8);
BSP_CAMERA_OFFSET(scalar_1cc, 0x1cc);
BSP_CAMERA_OFFSET(scalar_1d0, 0x1d0);
BSP_CAMERA_OFFSET(near_1d4, 0x1d4);
BSP_CAMERA_OFFSET(far_1d8, 0x1d8);
BSP_CAMERA_OFFSET(scalar_1dc, 0x1dc);
BSP_CAMERA_OFFSET(projection_1e0, 0x1e0);
BSP_CAMERA_OFFSET(view_projection_220, 0x220);
BSP_CAMERA_OFFSET(inverse_view_projection_260, 0x260);
BSP_CAMERA_OFFSET(projection_cache_2a0, 0x2a0);
BSP_CAMERA_OFFSET(extra_plane_2e0, 0x2e0);
BSP_CAMERA_OFFSET(valid_flags_2f0, 0x2f0);
BSP_CAMERA_OFFSET(planes_2f4, 0x2f4);
BSP_CAMERA_OFFSET(retained_438, 0x438);
BSP_CAMERA_OFFSET(borrowed_context_43c, 0x43c);
BSP_CAMERA_OFFSET(axis_y_440, 0x440);
BSP_CAMERA_OFFSET(axis_x_44c, 0x44c);
#undef BSP_CAMERA_OFFSET

namespace {
constexpr std::uint32_t camera_table = 0x00d62cf0;
void word(float& destination, std::uint32_t bits) noexcept {
    std::memcpy(&destination, &bits, 4);
}
NativeCameraStorageView prepare_storage(void* slot, std::size_t bytes,
    NativeCameraEnvironment& environment) {
    if (!slot || bytes < 0x45c || reinterpret_cast<std::uintptr_t>(slot) % 4u ||
        !environment.crt.dispatch_bypass_0109dd78 || !environment.crt.except_00c27489 ||
        !environment.vtable_00d62cf0 || !environment.vtable_00d62c88)
        throw std::invalid_argument("camera requires one aligned 45Ch slot and actual CRT/table bindings");
    // Default members (NativeString, atomic, plane records) must not erase the
    // native preimage before the native constructor reaches their stores.
    std::array<std::byte, 0x458> preimage;
    std::memcpy(preimage.data(), slot, preimage.size());
    auto& node = *::new (slot) NativeNodeStorage;
    auto& tail = *::new (static_cast<std::byte*>(slot) + 0x174) NativeCameraTailStorage;
    std::memcpy(slot, preimage.data(), preimage.size());
    return {node, tail};
}
bool camera_is_type(SceneAttachmentRuntime&, SceneNodeAttachment& binding, std::uint32_t token) {
    return static_cast<NativeCameraOwner*>(binding.context)->environment.types.is_type_00b71ce0(token);
}
std::uintptr_t pose_table(void* context, CameraState&) noexcept {
    return static_cast<NativeCameraOwner*>(context)->node.storage.vtable_00;
}
std::uintptr_t pose_slot(void* context, std::uintptr_t table, std::uint32_t offset) noexcept {
    // Only integer operations: the base position setter calls this with ST0
    // live between FLD input.z and FSTP world.z. Unsupported tables are outside
    // this concrete owner profile; they must never acquire a guessed override.
    if (offset != 0x30 && offset != 0x34) std::terminate();
    const auto& environment = static_cast<NativeCameraOwner*>(context)->environment;
    const volatile std::uint32_t* words = table == camera_table ? environment.vtable_00d62cf0
        : table == 0x00d62c88u ? environment.vtable_00d62c88 : nullptr;
    if (!words) std::terminate();
    return words[offset / 4];
}
void pose_position(void* context, std::uintptr_t entry, CameraState& camera, const CameraAxis& position) {
    auto& owner = *static_cast<NativeCameraOwner*>(context);
    if (&camera != &owner.camera || (entry != 0x00b71400u && entry != 0x00b6dae0u))
        throw std::logic_error("camera position virtual entry is outside the concrete profile");
    if (entry == 0x00b71400u) set_camera_world_position_00b71400(camera, position, owner.pose);
    else set_transform_world_position_00b6dae0(camera, position, owner.pose);
}
void pose_world(void* context, std::uintptr_t entry, CameraState& camera, const CameraMatrix& matrix) {
    auto& owner = *static_cast<NativeCameraOwner*>(context);
    if (&camera != &owner.camera || (entry != 0x00b71460u && entry != 0x00b6e870u))
        throw std::logic_error("camera world virtual entry is outside the concrete profile");
    if (entry == 0x00b71460u) set_camera_world_matrix_00b71460(camera, matrix);
    else set_transform_world_matrix_00b6e870(camera.transform, matrix, notify_camera_world_changed_00b6dbe0);
}
CameraFrameBacking frame_backing(NativeCameraTailStorage& tail, CameraViewportResolver& viewports) {
    return {tail.byte_174, tail.enabled_17c, CameraViewportSlot(tail.viewport_180, viewports),
        SystemFogSlotRef(tail.fog_184), tail.clear_flags_188, tail.clear_depth_18c,
        tail.clear_color_190, tail.clear_stencil_194, tail.render_mode_198,
        tail.inverse_view_projection_260, tail.planes_2f4, tail.borrowed_context_43c,
        tail.axis_y_440, tail.axis_x_44c};
}
void camera_phase(NativeCameraOwner& owner) noexcept {
    owner.node.storage.vtable_00 = camera_table;
    owner.node.scene_attachment.is_type = camera_is_type;
    owner.node.scene_attachment.context = &owner;
    owner.node.scene_attachment.attach_scene = set_node_scene_00b6ed80;
    owner.node.scene_attachment.world_changed = native_node_world_changed_00b6dbe0;
    owner.node.scene_attachment.remove_scene = remove_native_node_scene_00b6ee10;
}
void end_tail(NativeCameraOwner& owner) noexcept {
    owner.storage.camera.~NativeCameraTailStorage();
    owner.environment.nodes.scenes.forget_destroyed_binding(owner.node.scene_attachment);
    owner.phase = NativeCameraOwner::Phase::dead;
}
void finish_node(NativeCameraOwner& owner) {
    try {
        auto& nodes = owner.environment.nodes;
        if (nodes.uses_raw_name_pool())
            destroy_native_node_00b6f440(nodes, owner.node, nodes.require_raw_name_pool());
        else
            destroy_native_node_00b6f440(nodes, owner.node);
    } catch (...) {
        end_tail(owner);
        throw;
    }
    end_tail(owner);
}
void release_438(NativeCameraOwner& owner) {
    if (void* captured = owner.storage.camera.retained_438) {
        owner.environment.nodes.release_retained_owner(captured);
        owner.storage.camera.retained_438 = nullptr;
    }
}
void require_live(NativeCameraOwner& owner) {
    if (owner.phase != NativeCameraOwner::Phase::live &&
        owner.phase != NativeCameraOwner::Phase::constructing)
        throw std::logic_error("camera requires a live or constructing native owner");
}
}

NativeCameraOwner::NativeCameraOwner(void* slot, std::size_t bytes, NativeCameraEnvironment& access)
    : NativeCameraOwner(slot, bytes, access, nullptr) {}
NativeCameraOwner::NativeCameraOwner(void* slot, std::size_t bytes, NativeCameraEnvironment& access,
    SceneAttachmentRuntime::BindingAdmission&& admission)
    : NativeCameraOwner(slot, bytes, access, &admission) {}
NativeCameraOwner::NativeCameraOwner(void* slot, std::size_t bytes, NativeCameraEnvironment& access,
    SceneAttachmentRuntime::BindingAdmission* admission)
    : storage(prepare_storage(slot, bytes, access)), environment(access),
      node(storage.node, NativeNodePreconstructionBinding{}, access.nodes.node_virtual_0c, set_node_scene_00b6ed80, this),
      projection(CameraProjectionBacking{storage.camera.fov_1c4, storage.camera.aspect_1c8,
          storage.camera.near_1d4, storage.camera.far_1d8, storage.camera.projection_1e0,
          storage.camera.projection_cache_2a0, storage.camera.valid_flags_2f0}),
      camera(node.transform, projection, storage.camera.view_projection_220,
          storage.camera.direction_1ac, storage.camera.target_1a0),
      frame(camera, frame_backing(storage.camera, access.viewport_views)),
      pose{this, pose_table, pose_slot, pose_position, pose_world} {
    node.scene_attachment.world_changed = native_node_world_changed_00b6dbe0;
    node.scene_attachment.remove_scene = remove_native_node_scene_00b6ee10;
    try {
        if (admission) access.nodes.scenes.bind(node.scene_attachment, std::move(*admission));
        else access.nodes.scenes.bind(node.scene_attachment);
    } catch (...) {
        storage.camera.~NativeCameraTailStorage();
        storage.node.~NativeNodeStorage();
        throw; // host association failure: no native constructor has run
    }
}
NativeCameraOwner::~NativeCameraOwner() {
    if (phase == Phase::prepared) {
        environment.nodes.scenes.forget_destroyed_binding(node.scene_attachment);
        storage.camera.~NativeCameraTailStorage();
        storage.node.~NativeNodeStorage();
    }
}

void* construct_native_camera_00b71a80(NativeCameraOwner& owner, const NativeString& name) {
    if (owner.phase != NativeCameraOwner::Phase::prepared)
        throw std::logic_error("camera constructor requires its unused prepared slot");
    auto& name_pool = owner.environment.nodes.require_semantic_name_pool();
    owner.phase = NativeCameraOwner::Phase::constructing;
    try {
        // Same-type placement transparently replaces the prefix; every binding
        // still addresses the actual fields. B6F5A0 owns its own failure cleanup.
        construct_native_node_00b6f5a0(&owner.storage.node, 0x45c, name, name_pool);
    } catch (...) {
        end_tail(owner);
        throw;
    }
    camera_phase(owner);
    auto& tail = owner.storage.camera;
    auto& environment = owner.environment;
    unsigned unwind_state = 0;
    try {
        // Shared allocator wrapper supplies exact state1 raw-allocation cleanup.
        // If the viewport constructor throws, no +180 publication occurs.
        tail.viewport_180 = allocate_native_viewport_owner(environment.viewport);
        initialize_system_fog_camera_slot_00b71ae3(owner.frame.fog_184);
        for (auto& value : tail.zero_1b8) word(value, 0);
        construct_camera_plane_set_00b659d0(tail.planes_2f4, environment.viewport.one_bits_00d7a24c);
        tail.retained_438 = nullptr;
        unwind_state = 2;
        tail.borrowed_context_43c = nullptr;
        const DWORD origin[2] = {0, 0};
        set_native_viewport_origin_00b1f920(*tail.viewport_180, origin);
        auto* const renderer = environment.viewport.renderer_00f8d394;
        if (!renderer) throw std::logic_error("camera constructor requires the actual renderer");
        const DWORD height = environment.viewport.renderer_access.parameters_00b1ff60(*renderer).height_10;
        const DWORD width = environment.viewport.renderer_access.parameters_00b1ff60(*renderer).width_0c;
        const DWORD dimensions[2] = {width, height};
        set_native_viewport_dimensions_00b1f940(*tail.viewport_180, dimensions);
        std::uint32_t depth;
        __asm { fldz }
        __asm { fstp dword ptr [depth] }
        set_native_viewport_min_depth_00b1f750(*tail.viewport_180, depth);
        __asm { fld1 }
        __asm { fstp dword ptr [depth] }
        set_native_viewport_max_depth_00b1f760(*tail.viewport_180, depth);
        const std::uint32_t hundred = environment.constants.hundred_00ce3d08;
        CameraAxis eye, target;
        word(target[0], 0); word(target[1], hundred); word(target[2], hundred);
        word(eye[0], 0); word(eye[1], hundred); word(eye[2], 0);
        set_camera_look_at_00b700e0(owner.camera, eye, target, owner.pose, environment.crt,
            environment.viewport.one_bits_00d7a24c);

        const auto fov = environment.constants.fov_00ce7d20;
        const auto far_plane = environment.constants.far_00d0c5f8;
        const auto one = environment.viewport.one_bits_00d7a24c;
        word(tail.fov_1c4, fov);
        word(tail.aspect_1c8, environment.constants.aspect_00d5bd98);
        word(tail.far_1d8, far_plane);
        const auto scalar = environment.constants.scalar_00ce77fc;
        word(tail.scalar_1cc, 0); word(tail.scalar_1d0, 0);
        word(tail.near_1d4, one); word(tail.scalar_178, scalar);
        tail.valid_flags_2f0 = 1;
        tail.render_mode_198 = 0; tail.render_mask_19c = 1;
        tail.clear_flags_188 = 0;
        const auto scalar_1dc = environment.constants.scalar_00ce3c88;
        tail.clear_stencil_194 = 0; tail.enabled_17c = 1;
        word(tail.clear_depth_18c, one); word(tail.scalar_1dc, scalar_1dc);
        tail.clear_color_190 = 0;
        word(tail.axis_y_440[0], 0); word(tail.axis_y_440[1], one); word(tail.axis_y_440[2], 0);
        word(tail.axis_x_44c[0], one); word(tail.axis_x_44c[1], 0); word(tail.axis_x_44c[2], 0);
        tail.byte_174 = 1;
    } catch (...) {
        try {
            if (unwind_state == 2) release_438(owner); // CC1AE3 ->605FD0
            finish_node(owner); // CC1AD0, no published viewport/fog cleanup
        } catch (...) { std::terminate(); }
        throw;
    }
    owner.phase = NativeCameraOwner::Phase::live;
    return &owner.storage.node;
}

// Keep the native continuation literal: its volatile load and alias schedule
// must remain identical to the established semantic entry above.
namespace {
void* construct_native_camera_raw(NativeCameraOwner& owner, const void* actual_name_header,
    const NativeNodeRawConstants& constants, NativeViewportRegistry::Admission* admission) {
    if (owner.phase != NativeCameraOwner::Phase::prepared)
        throw std::logic_error("camera constructor requires its unused prepared slot");
    auto& name_pool = owner.environment.nodes.require_raw_name_pool();
    if (&constants.one_00d7a24c != &owner.environment.viewport.one_bits_00d7a24c)
        throw std::logic_error("camera and node require the same actual D7A24C cell");
    NativeViewportRegistry::Admission prepared;
    const bool has_admission = admission != nullptr;
    if (admission) {
        auto& registry = admission->require_registry();
        if (&owner.environment.viewport_views != &registry)
            throw std::logic_error("camera viewport admission requires its exact installed resolver");
        prepared = std::move(*admission); // caller token is empty before native callbacks
    }
    owner.phase = NativeCameraOwner::Phase::constructing;
    try {
        // Same-type placement transparently replaces the prefix; every binding
        // still addresses the actual fields. B6F5A0 owns its own failure cleanup.
        construct_native_node_00b6f5a0(&owner.storage.node, 0x45c, actual_name_header, name_pool, constants);
    } catch (...) {
        end_tail(owner);
        throw;
    }
    camera_phase(owner);
    auto& tail = owner.storage.camera;
    auto& environment = owner.environment;
    unsigned unwind_state = 0;
    try {
        // Shared allocator wrapper supplies exact state1 raw-allocation cleanup.
        // If the viewport constructor throws, no +180 publication occurs.
        tail.viewport_180 = has_admission
            ? allocate_native_viewport_owner(environment.viewport, std::move(prepared))
            : allocate_native_viewport_owner(environment.viewport);
        initialize_system_fog_camera_slot_00b71ae3(owner.frame.fog_184);
        for (auto& value : tail.zero_1b8) word(value, 0);
        construct_camera_plane_set_00b659d0(tail.planes_2f4, environment.viewport.one_bits_00d7a24c);
        tail.retained_438 = nullptr;
        unwind_state = 2;
        tail.borrowed_context_43c = nullptr;
        const DWORD origin[2] = {0, 0};
        set_native_viewport_origin_00b1f920(*tail.viewport_180, origin);
        auto* const renderer = environment.viewport.renderer_00f8d394;
        if (!renderer) throw std::logic_error("camera constructor requires the actual renderer");
        const DWORD height = environment.viewport.renderer_access.parameters_00b1ff60(*renderer).height_10;
        const DWORD width = environment.viewport.renderer_access.parameters_00b1ff60(*renderer).width_0c;
        const DWORD dimensions[2] = {width, height};
        set_native_viewport_dimensions_00b1f940(*tail.viewport_180, dimensions);
        std::uint32_t depth;
        __asm { fldz }
        __asm { fstp dword ptr [depth] }
        set_native_viewport_min_depth_00b1f750(*tail.viewport_180, depth);
        __asm { fld1 }
        __asm { fstp dword ptr [depth] }
        set_native_viewport_max_depth_00b1f760(*tail.viewport_180, depth);
        const std::uint32_t hundred = environment.constants.hundred_00ce3d08;
        CameraAxis eye, target;
        word(target[0], 0); word(target[1], hundred); word(target[2], hundred);
        word(eye[0], 0); word(eye[1], hundred); word(eye[2], 0);
        set_camera_look_at_00b700e0(owner.camera, eye, target, owner.pose, environment.crt,
            environment.viewport.one_bits_00d7a24c);

        const auto fov = environment.constants.fov_00ce7d20;
        const auto far_plane = environment.constants.far_00d0c5f8;
        const auto one = environment.viewport.one_bits_00d7a24c;
        word(tail.fov_1c4, fov);
        word(tail.aspect_1c8, environment.constants.aspect_00d5bd98);
        word(tail.far_1d8, far_plane);
        const auto scalar = environment.constants.scalar_00ce77fc;
        word(tail.scalar_1cc, 0); word(tail.scalar_1d0, 0);
        word(tail.near_1d4, one); word(tail.scalar_178, scalar);
        tail.valid_flags_2f0 = 1;
        tail.render_mode_198 = 0; tail.render_mask_19c = 1;
        tail.clear_flags_188 = 0;
        const auto scalar_1dc = environment.constants.scalar_00ce3c88;
        tail.clear_stencil_194 = 0; tail.enabled_17c = 1;
        word(tail.clear_depth_18c, one); word(tail.scalar_1dc, scalar_1dc);
        tail.clear_color_190 = 0;
        word(tail.axis_y_440[0], 0); word(tail.axis_y_440[1], one); word(tail.axis_y_440[2], 0);
        word(tail.axis_x_44c[0], one); word(tail.axis_x_44c[1], 0); word(tail.axis_x_44c[2], 0);
        tail.byte_174 = 1;
    } catch (...) {
        try {
            if (unwind_state == 2) release_438(owner); // CC1AE3 ->605FD0
            finish_node(owner); // CC1AD0, no published viewport/fog cleanup
        } catch (...) { std::terminate(); }
        throw;
    }
    owner.phase = NativeCameraOwner::Phase::live;
    return &owner.storage.node;
}
} // namespace

void* construct_native_camera_00b71a80(NativeCameraOwner& owner, const void* actual_name_header,
    const NativeNodeRawConstants& constants) {
    return construct_native_camera_raw(owner, actual_name_header, constants, nullptr);
}
void* construct_native_camera_00b71a80(NativeCameraOwner& owner, const void* actual_name_header,
    const NativeNodeRawConstants& constants, NativeViewportRegistry::Admission&& admission) {
    return construct_native_camera_raw(owner, actual_name_header, constants, &admission);
}

void set_native_camera_viewport_00b71990(NativeCameraOwner& owner, NativeViewportOwner* value) {
    require_live(owner);
    auto* const previous = owner.storage.camera.viewport_180;
    if (previous == value) return;
    owner.storage.camera.viewport_180 = value;
    if (value) retain_native_viewport_owner(*value);
    if (previous) release_native_viewport_owner(*previous);
}
void destroy_native_camera_00b71f10(NativeCameraOwner& owner) {
    require_live(owner);
    camera_phase(owner);
    unsigned unwind_state = 1;
    try {
        if (auto* viewport = owner.storage.camera.viewport_180) {
            release_native_viewport_owner(*viewport);
            owner.storage.camera.viewport_180 = nullptr;
        }
        clear_system_fog_camera_slot_00b71f68(owner.frame.fog_184);
        unwind_state = 0; // CC1B28 must not repeat a throwing explicit +438 release
        release_438(owner);
    } catch (...) {
        try {
            if (unwind_state == 1) release_438(owner);
            finish_node(owner);
        } catch (...) { std::terminate(); }
        throw;
    }
    finish_node(owner);
}
void* delete_native_camera_00b71fe0(NativeCameraOwner& owner, std::uint32_t flags) {
    void* const slot = &owner.storage.node;
    destroy_native_camera_00b71f10(owner);
    if (flags & 1u) owner.environment.pool_0108ffb0.return_raw_slot_00b711e0(slot);
    return slot;
}

} // namespace bsp
