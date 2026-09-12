#include "bsp/native_camera_reference.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
void require_camera_slot(NativeCameraOwner& owner, std::uint32_t offset,
    std::uint32_t expected, bool allow_node_phase = false) noexcept {
    const auto profile = owner.storage.node.vtable_00;
    const volatile std::uint32_t* table = profile == 0x00d62cf0u
        ? owner.environment.vtable_00d62cf0
        : allow_node_phase && profile == 0x00d62c88u
            ? owner.environment.vtable_00d62c88 : nullptr;
    if (!table || table[offset / 4] != expected) std::terminate();
}
}

NativeCameraReference::NativeCameraReference(NativeCameraOwner& owner,
    NativeCameraCompanionDisposal disposal)
    : RenderCommandReference(owner.storage.node.references_04), owner_(owner),
      runtime_(owner.environment.nodes.attachments), disposal_(disposal) {
    if (owner.phase != NativeCameraOwner::Phase::live || !disposal.retire ||
        reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("camera reference requires a live native owner and explicit companion retirement");
    require_camera_slot(owner, 0, 0x00bd30e0u);
    require_camera_slot(owner, 4, 0x00b71fe0u);
    // Uniqueness uses this same transform identity. This association adds no ref.
    runtime_.bind(*this);
}
NativeCameraReference::~NativeCameraReference() {
    // Removing an outstanding interface would leave queued/child references
    // dangling. The terminal callback is the only successful retirement path.
    if (phase_ != Phase::retired) std::terminate();
}
std::uint32_t NativeCameraReference::light_count(void* context) noexcept {
    const auto& array = static_cast<NativeCameraReference*>(context)->owner_.storage.node.point_lights_164;
    if (array.count < 0 || array.capacity < array.count || (array.count && !array.begin))
        std::terminate();
    return static_cast<std::uint32_t>(array.count);
}
void NativeCameraReference::remove_light_backlink(void* context,
    std::uint32_t index, CameraTransform&) noexcept {
    auto& reference = *static_cast<NativeCameraReference*>(context);
    auto& node = reference.owner_.storage.node;
    auto& light = reference.owner_.environment.nodes.point_lights.light(node.point_lights_164.begin[index]);
    remove_native_point_light_backlink_00b7c1a0(light, node);
}
void NativeCameraReference::shrink_lights(void* context) noexcept {
    auto& array = static_cast<NativeCameraReference*>(context)->owner_.storage.node.point_lights_164;
    shrink_native_node_point_lights_to_zero_00b6ec70(array);
}
void NativeCameraReference::release_model_virtual18_00b6f310() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeCameraOwner::Phase::live)
        std::terminate();
    require_camera_slot(owner_, 0x18, 0x00b6f310u);
    release_node_logical_00b6f310({runtime_, owner_.node.transform,
        owner_.storage.node.released_44, *this,
        {this, light_count, remove_light_backlink, shrink_lights}});
    // The shared self release may have retired both companions.
}
void NativeCameraReference::remove_scene_virtual54(SceneResource* expected, bool recurse) noexcept {
    if (phase_ == Phase::retired || owner_.phase == NativeCameraOwner::Phase::dead)
        std::terminate();
    require_camera_slot(owner_, 0x54, 0x00b6ee10u, true);
    remove_native_node_scene_00b6ee10(owner_.environment.nodes.scenes,
        owner_.node.scene_attachment, expected, recurse);
}
void NativeCameraReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeCameraOwner::Phase::live)
        std::terminate();
    // Queue/self helpers already decremented actual +04. BD30E0 only loads the
    // current deleting slot and supplies flag1; it adds no byte44 gate/decrement.
    require_camera_slot(owner_, 0, 0x00bd30e0u);
    require_camera_slot(owner_, 4, 0x00b71fe0u);
    phase_ = Phase::destroying;
    auto& runtime = runtime_;
    const auto disposal = disposal_;
    delete_native_camera_00b71fe0(owner_, 1);
    // The pool slot is dead. unbind compares this host interface pointer only;
    // camera cleanup has already removed the separate scene association.
    runtime.unbind(*this);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
}
