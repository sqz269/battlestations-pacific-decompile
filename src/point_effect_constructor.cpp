#include "bsp/point_effect_constructor.hpp"
#include <stdexcept>

namespace bsp {
namespace {
class MemberUnwind final {
public:
    MemberUnwind(PointEffectInstanceStorage& value, PointEffectInstanceLinks& projections)
        : effect(value), links(projections) {}
    ~MemberUnwind() noexcept {
        if (!armed) return;
        if (auto* const parent = effect.parent_8c) {
            release_render_command_reference(links.parent_reference(*parent));
            effect.parent_8c = nullptr;
        }
        if (auto* const definition = effect.template_84) {
            release_render_command_reference(*definition);
            effect.template_84 = nullptr;
        }
        destroy_point_effect_entry_array_008675b0(effect.auxiliary_18);
        destroy_point_effect_entry_array_008675b0(effect.entries_0c);
        effect.original_vtable_identity_00 = 0x00ceb130u;
    }
    bool armed{true};
private:
    PointEffectInstanceStorage& effect;
    PointEffectInstanceLinks& links;
};
void retire_constructed_companion(void*, NativePointEffectReference& reference, bool) noexcept {
    delete &reference; // native storage ownership stays in its scalar/caller path
}
}

NativePointEffectReference& construct_point_effect_instance_008680b0(void* raw,
    RenderCommandReference* consumed_template, CameraTransform* parent,
    std::uint32_t third_word, const CameraMatrix& matrix,
    std::uint8_t transform_byte, std::uint8_t option_byte, std::uint32_t tail_word,
    const NativeString& name, PointEffectConstructorBindings& bindings) {
    return construct_point_effect_instance_008680b0(raw, consumed_template, parent,
        third_word, matrix, transform_byte, option_byte, tail_word, &name, bindings);
}
NativePointEffectReference& construct_point_effect_instance_008680b0(void* raw,
    RenderCommandReference* consumed_template, CameraTransform* parent,
    std::uint32_t third_word, const CameraMatrix& matrix,
    std::uint8_t transform_byte, std::uint8_t option_byte, std::uint32_t tail_word,
    const void* name, PointEffectConstructorBindings& bindings) {
    PointEffectTemplateArgument argument(consumed_template);
    try {
        auto& effect = initialize_point_effect_instance_008680d9(raw, argument,
            option_byte, tail_word, bindings.counters);
        MemberUnwind unwind(effect, bindings.links);
        auto* const physical = construct_point_effect_node_00868193(
            bindings.actual_node_pool_0108ff58, name, bindings.strings);
        // Native immediately increments returned+04, including the invalid null
        // result. This typed interface requires the same nonnull live backing.
        auto& reference = bindings.node_companions.bind_actual_constructed_node(
            *physical, bindings.nodes, bindings.strings, bindings.actual_node_pool_0108ff58);
        auto& node = reference.node_binding();
        if (&node.storage != physical || &reference.reference_count != &physical->references_04 ||
            &bindings.nodes.scenes.resolve(node.transform) != &node.scene_attachment)
            throw std::invalid_argument("Point constructor requires the exact constructed node's canonical companions");
        register_point_effect_node_and_parent_008681be(effect, node, parent,
            third_word, bindings.nodes, bindings.links);
        if (transform_byte)
            set_point_effect_relative_matrix_0072aa80(effect, matrix, bindings.nodes.scenes);
        else
            set_point_effect_world_matrix_0053d9c0(effect, matrix, bindings.nodes.scenes);
        cache_point_effect_initial_world_0086826e(effect);

        // Host association only, before a real row callback or insertion can
        // invoke the actual owner's terminal path. Does not retain the owner.
        auto* const result = new NativePointEffectReference(effect, bindings.effects,
            {bindings.nodes.attachments, bindings.node_terminal_owners, bindings.links, bindings.counters},
            bindings.actual_effect_table_00d0d3ec, {nullptr, retire_constructed_companion});
        initialize_point_effect_rows_008682d5(effect, bindings.rows);
        auto* const manager = bindings.effects.live_manager_004d1100();
        insert_live_effect_00867500(manager->effects_10, &effect,
            bindings.actual_insertion_lock_00f87650, bindings.insertion_lifetime, bindings.effects);
        unwind.armed = false;
        return *result; // incoming argument release follows on function exit
    } catch (...) {
        // The native member map has run; remove only any registered host
        // companion. Raw effect, successful node and counters are untouched.
        bindings.effects.retire_failed_constructor_binding(raw);
        throw; // incoming argument is released after all member cleanup
    }
}
} // namespace bsp
