#pragma once
#include "bsp/camera_transform.hpp"
#include "bsp/native_string.hpp"
#include "bsp/scene_attachment.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace bsp {
struct GeneratedModelPointLightLinks;

// The one native +164 pointer/count/capacity array. It borrows light bindings;
// this constructor only initializes it empty and never creates a second list.
struct NativeNodePointLightArray {
    GeneratedModelPointLightLinks** begin;
    std::int32_t count;
    std::int32_t capacity;
};

// Exact 0x174-byte prefix on MSVC Win32, placed in the actual pool slot.
// Deliberately no member initializers: constructor-unwritten bytes retain the
// actual slot preimage. NativeString/atomic default construction touches only
// fields that 00B6F5A0 initializes before its first potentially throwing call.
// Native-layout scalar storage does not make host pointers a binary-compatible
// replacement: hierarchy pointers refer to the shared C++ binding companions.
struct NativeNodeStorage {
    std::uint32_t vtable_00;
    std::atomic<std::int32_t> references_04;
    std::array<std::byte, 0x28> untouched_08;
    CameraTransform* parent_30;
    CameraTransform* first_child_34;
    std::uint32_t child_count_38;
    CameraTransform* next_sibling_3c;
    CameraTransform* previous_sibling_40;
    std::uint8_t released_44;
    std::array<std::byte, 3> untouched_45;
    std::uint32_t mask_48;
    std::uint32_t scalar_4c;
    std::uint32_t bounds_scalar_50;
    NativeString name_54;
    std::uint32_t valid_flags_5c;
    CameraMatrix view_60;
    void* notification_context_a0;
    RenderNodeRootList* root_list_a4;
    std::uint32_t field_a8;
    std::uint32_t scalar_ac;
    CameraMatrix local_b0;
    CameraMatrix world_f0;
    void* retained_130;
    std::uint8_t enabled_134;
    std::array<std::byte, 3> untouched_135;
    std::uint32_t auxiliary_flags_138;
    std::array<float, 4> world_sphere_13c;
    std::array<float, 3> bounds_min_14c;
    std::array<float, 3> bounds_max_158;
    NativeNodePointLightArray point_lights_164;
    SceneResource* scene_170;
};

// ECX native raw owner, stack NativeString*, EAX same owner, RET4. This new
// C++ interface receives the actual slot and existing string pool explicitly.
// slot_bytes must cover a 0x1F0 pool slot; nothing at +174..1EF is written.
// The caller owns physical storage, including returning it after an exception.
NativeNodeStorage& construct_native_node_00b6f5a0(void* actual_slot,
    std::size_t slot_bytes, const NativeString& name, SizedStoragePool& strings);

// External companion; both members bind references to the SAME raw slot. It
// must remain stable while hierarchy/scene runtimes refer to it. It does not
// retain/register anything and supplies no invented virtual implementation.
class NativeNodeBinding final {
public:
    NativeNodeBinding(NativeNodeStorage&, SceneTypePredicate actual_virtual_0c,
        SceneAttachOverride actual_virtual_50, void (*actual_notify_changed)(void*),
        void* dispatch_context = nullptr);
    NativeNodeBinding(const NativeNodeBinding&) = delete;
    NativeNodeBinding& operator=(const NativeNodeBinding&) = delete;
    NativeNodeStorage& storage;
    CameraTransform transform;
    SceneNodeAttachment scene_attachment;
};

} // namespace bsp
