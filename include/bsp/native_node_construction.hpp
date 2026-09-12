#pragma once
#include "bsp/camera_transform.hpp"
#include "bsp/native_string.hpp"
#include "bsp/scene_attachment.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace bsp {

// The one native +164 pointer/count/capacity array. It borrows actual lights;
// this constructor only initializes it empty and never creates a second list.
struct NativeNodePointLightArray {
    void** begin; // actual point-light identities, never companion pointers
    std::int32_t count;
    std::int32_t capacity;
};

// Exact 0x174-byte prefix on MSVC Win32, placed in the actual pool slot.
// Deliberately no member initializers: constructor-unwritten bytes retain the
// actual slot preimage. NativeString/atomic default construction touches only
// fields that 00B6F5A0 initializes before its first potentially throwing call.
// The four hierarchy DWORDs contain actual node addresses. Host companion
// traversal resolves those same words through the live scene-runtime binding;
// no companion pointer is stored in this native prefix.
struct NativeNodeStorage {
    std::uint32_t vtable_00;
    std::atomic<std::int32_t> references_04;
    std::array<std::byte, 0x28> untouched_08;
    std::uint32_t parent_30;
    std::uint32_t first_child_34;
    std::uint32_t child_count_38;
    std::uint32_t next_sibling_3c;
    std::uint32_t previous_sibling_40;
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
// C++ interface receives the actual slot and existing string storage explicitly.
// ActualNativeStringPoolStorage supplies the real native singleton/ring path.
// slot_bytes must cover the 0x174 prefix; the concrete owner supplies its actual
// allocation extent (plain node178h, directional light1F0h, model188h,
// camera45Ch). No byte at +174 or
// beyond is written, including derived data and a pool's trailing slot ID.
// The caller owns physical storage, including returning it after an exception.
NativeNodeStorage& construct_native_node_00b6f5a0(void* actual_slot,
    std::size_t slot_bytes, const NativeString& name, NativeStringStorage& strings);
// Same body for a borrowed actual8h header, e.g. gameplay definition+1C.
// Preserve its address and post-allocation reloads; no temporary name owner.
NativeNodeStorage& construct_native_node_00b6f5a0(void* actual_slot,
    std::size_t slot_bytes, const void* actual_name_header, NativeStringStorage& strings);

// Existing callers using the semantic SizedStoragePool retain their adapter;
// both overloads execute the same constructor body and member cleanup.
NativeNodeStorage& construct_native_node_00b6f5a0(void* actual_slot,
    std::size_t slot_bytes, const NativeString& name, SizedStoragePool& strings);

// External companion; both members bind references to the SAME raw slot. It
// must remain stable while hierarchy/scene runtimes refer to it. It does not
// retain/register anything and supplies no invented virtual implementation.
struct NativeNodePreconstructionBinding final {};
class NativeNodeBinding final {
public:
    NativeNodeBinding(NativeNodeStorage&, SceneTypePredicate actual_virtual_0c,
        SceneAttachOverride actual_virtual_50, void (*actual_notify_changed)(void*),
        void* dispatch_context = nullptr);
    // Only bind addresses before the native constructor runs. The backing's
    // typed lifetimes must exist, but its bytes are still allocation preimages:
    // do not inspect +A0 or provide a notification callback until initialized.
    NativeNodeBinding(NativeNodeStorage&, NativeNodePreconstructionBinding,
        SceneTypePredicate actual_virtual_0c, SceneAttachOverride actual_virtual_50,
        void* dispatch_context = nullptr);
    NativeNodeBinding(const NativeNodeBinding&) = delete;
    NativeNodeBinding& operator=(const NativeNodeBinding&) = delete;
    NativeNodeStorage& storage;
    CameraTransform transform;
    SceneNodeAttachment scene_attachment;
};

} // namespace bsp
