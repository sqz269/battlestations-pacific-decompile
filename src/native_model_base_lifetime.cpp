#include "bsp/native_model_base_lifetime.hpp"
#include "bsp/native_node_pool_allocation.hpp"
#include "bsp/point_effect_matrix_setters.hpp"

#include <exception>
#include <stdexcept>

namespace bsp {

bool native_model_base_is_type_00b743e0(const NativeModelBaseTypeTokens& cells,
    std::uint32_t token) noexcept {
    if (cells.actual_01090044 == token) return true;
    if (cells.actual_01090048 == token) return true;
    return cells.actual_0109004c == token;
}

NativeNodeStorage& construct_native_model_base_00b743c0(void* slot,
    std::size_t bytes, const void* name, NativeStringRawPoolContext& strings,
    const NativeNodeRawConstants& constants) {
    auto& node = construct_native_node_00b6f5a0(slot, bytes, name, strings, constants);
    node.vtable_00 = 0x00d62d78u;
    return node;
}

void* allocate_native_model_base_00b74ec0(void* pool) {
    return allocate_native_node_pool_slot_00b6eb00(pool);
}
void return_native_model_base_00b748d0(void* slot, void* pool) {
    return_native_node_pool_slot_00b6e490(pool, slot);
}
NativeNodeStorage* create_native_model_base_00b86720(void* pool, const void* name,
    NativeStringRawPoolContext& strings, const NativeNodeRawConstants& constants) {
    void* const slot = allocate_native_model_base_00b74ec0(pool);
    if (!slot) return nullptr;
    try {
        return &construct_native_model_base_00b743c0(slot,
            native_node_pool_slot_bytes, name, strings, constants);
    } catch (...) {
        return_native_model_base_00b748d0(slot, pool);
        throw;
    }
}

void* delete_native_model_base_00b74b60(NativeNodeDestructionRuntime& nodes,
    NativeNodeBinding& binding, NativeStringRawPoolContext& strings, void* pool,
    std::uint32_t flags) {
    void* const original = &binding.storage;
    try {
        destroy_native_node_00b6f440(nodes, binding, strings);
    } catch (...) {
        nodes.scenes.forget_destroyed_binding(binding.scene_attachment);
        throw;
    }
    nodes.scenes.forget_destroyed_binding(binding.scene_attachment);
    if (flags & 1u) return_native_node_pool_slot_00b6e490(pool, original);
    return original;
}

NativeModelBaseReference::NativeModelBaseReference(NativeNodeBinding& node,
    NativeNodeDestructionRuntime& nodes, NativeStringRawPoolContext& strings, void* pool,
    const volatile std::uint32_t* table, NativeModelBaseTypeTokens tokens,
    NativeModelBaseCompanionDisposal disposal)
    : RenderCommandReference(node.storage.references_04), node_(node), nodes_(nodes),
      strings_(strings), pool_(pool), table_(table), tokens_(tokens), node_phase_context_(node.scene_attachment.context), disposal_(disposal) {
    if (!pool || !table || !disposal.retire || node.storage.vtable_00 != 0x00d62d78u ||
        table[0] != 0x00bd30e0u || table[1] != 0x00b74b60u ||
        table[3] != 0x00b743e0u || table[13] != 0x00b6e870u ||
        table[16] != 0x00b6dbe0u || table[20] != 0x00b6ed80u || table[21] != 0x00b6ee10u ||
        reference_count.load(std::memory_order_relaxed) <= 0 ||
        &nodes.scenes.resolve(node.transform) != &node.scene_attachment)
        throw std::invalid_argument("model base reference requires actual constructed owner, pool, table, scene binding and retirement");
    if (&strings != &nodes.require_raw_name_pool())
        throw std::invalid_argument("model base reference requires identical actual raw name context");
    nodes.attachments.bind(*this);
    node.scene_attachment.context = this;
    node.scene_attachment.is_type = is_type;
    node.scene_attachment.set_world_matrix = set_native_node_world_matrix_00b6e870;
    node.scene_attachment.world_changed = native_node_world_changed_00b6dbe0;
    node.scene_attachment.attach_scene = set_node_scene_00b6ed80;
    node.scene_attachment.remove_scene = remove_native_node_scene_00b6ee10;
}

bool NativeModelBaseReference::is_type(SceneAttachmentRuntime&,
    SceneNodeAttachment& binding, std::uint32_t token) {
    auto& self = *static_cast<NativeModelBaseReference*>(binding.context);
    self.require_slot(0x0c, 0x00b743e0u);
    return native_model_base_is_type_00b743e0(self.tokens_, token);
}

NativeModelBaseReference::~NativeModelBaseReference() {
    if (phase_ != Phase::retired) std::terminate();
}

void NativeModelBaseReference::require_slot(std::uint32_t offset,
    std::uint32_t address) const noexcept {
    if (phase_ == Phase::retired || node_.storage.vtable_00 != 0x00d62d78u ||
        table_[offset / 4] != address) std::terminate();
}

std::uint32_t NativeModelBaseReference::light_count(void* context) noexcept {
    const auto& array = static_cast<NativeModelBaseReference*>(context)->node_.storage.point_lights_164;
    if (array.count < 0 || array.capacity < array.count || (array.count && !array.begin))
        std::terminate();
    return static_cast<std::uint32_t>(array.count);
}
void NativeModelBaseReference::remove_light_backlink(void* context,
    std::uint32_t index, CameraTransform&) noexcept {
    auto& reference = *static_cast<NativeModelBaseReference*>(context);
    auto& node = reference.node_.storage;
    auto& light = reference.nodes_.point_lights.light(node.point_lights_164.begin[index]);
    remove_native_point_light_backlink_00b7c1a0(light, node);
}
void NativeModelBaseReference::shrink_lights(void* context) noexcept {
    auto& array = static_cast<NativeModelBaseReference*>(context)->node_.storage.point_lights_164;
    shrink_native_node_point_lights_to_zero_00b6ec70(array);
}

void NativeModelBaseReference::release_model_virtual18_00b6f310() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_slot(0x18, 0x00b6f310u);
    release_node_logical_00b6f310({nodes_.attachments, node_.transform,
        node_.storage.released_44, *this,
        {this, light_count, remove_light_backlink, shrink_lights}});
}

void NativeModelBaseReference::remove_scene_virtual54(SceneResource* scene,
    bool recurse) noexcept {
    require_slot(0x54, 0x00b6ee10u);
    remove_native_node_scene_00b6ee10(nodes_.scenes, node_.scene_attachment, scene, recurse);
}

void NativeModelBaseReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_slot(0, 0x00bd30e0u);
    require_slot(4, 0x00b74b60u);
    phase_ = Phase::destroying;
    auto& lifetime = nodes_.attachments;
    const auto disposal = disposal_;
    node_.scene_attachment.context = node_phase_context_;
    delete_native_model_base_00b74b60(nodes_, node_, strings_, pool_, 1);
    lifetime.unbind(*this);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}

} // namespace bsp
