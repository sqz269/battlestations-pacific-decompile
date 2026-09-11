#include "bsp/point_effect_teardown.hpp"
#include "bsp/point_effect_entry_array.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Point-effect teardown requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(PointEffectInstanceStorage) == 0x114);
static_assert(offsetof(PointEffectInstanceStorage, entries_0c) == 0x0c);
static_assert(offsetof(PointEffectInstanceStorage, auxiliary_18) == 0x18);
static_assert(offsetof(PointEffectInstanceStorage, template_84) == 0x84);
static_assert(offsetof(PointEffectInstanceStorage, parent_8c) == 0x8c);
static_assert(offsetof(PointEffectInstanceStorage, node_110) == 0x110);

void release_parent(PointEffectInstanceStorage& effect,
    PointEffectInstanceLinks& links) noexcept {
    CameraTransform* const captured = effect.parent_8c;
    if (captured) {
        release_render_command_reference(links.parent_reference(*captured));
        effect.parent_8c = nullptr; // native clears AFTER terminal reentry
    }
}

void release_template(PointEffectInstanceStorage& effect) noexcept {
    RenderCommandReference* const captured = effect.template_84;
    if (captured) {
        release_render_command_reference(*captured);
        effect.template_84 = nullptr;
    }
}

void destroy_base(PointEffectInstanceStorage& effect) noexcept {
    volatile auto& actual = effect;
    actual.original_vtable_identity_00 = 0x00ceb130u; // complete BD30F0
}

class MemberUnwind final {
public:
    MemberUnwind(PointEffectInstanceStorage& effect,
        PointEffectInstanceLinks& links) noexcept : effect_(effect), links_(links) {}
    // DC6D70 map: state4 parent ->3 template ->2 auxiliary ->1 entries ->0 base.
    // No node release, F87604 rollback or physical free exists in that map.
    ~MemberUnwind() noexcept {
        if (state >= 4) release_parent(effect_, links_);
        if (state >= 3) release_template(effect_);
        if (state >= 2) destroy_point_effect_entry_array_008675b0(effect_.auxiliary_18);
        if (state >= 1) destroy_point_effect_entry_array_008675b0(effect_.entries_0c);
        if (state >= 0) destroy_base(effect_);
    }
    int state{4};
private:
    PointEffectInstanceStorage& effect_;
    PointEffectInstanceLinks& links_;
};
} // namespace

void destroy_point_effect_entry_array_008675b0(PointEffectReferenceArray& array) {
    resize_point_effect_entry_array_008672a0(array, 0);
    // Resize may invoke real terminal callbacks. Reload CURRENT backing after it.
    volatile auto& actual = array;
    singleton_lifetime_free(actual.begin);
}

void destroy_point_effect_instance_00867680(PointEffectInstanceStorage& effect,
    PointEffectTeardownBindings bindings) {
    volatile auto& actual = effect;
    actual.original_vtable_identity_00 = 0x00d0d3ecu;
    NativeNodeBinding* const unlinked_node = actual.node_110; // 8676A5, before SUB
    --bindings.counters.actual_00f87600; // native unsigned DWORD wrap; F87604 untouched
    MemberUnwind unwind(effect, bindings.parent_links);
    unlink_and_release_render_model_00b6dfa0(
        bindings.node_lifetimes.resolve(unlinked_node->transform));

    NativeNodeBinding* const released_node = actual.node_110; // 8676BF, after virtual18
    // Current node is mandatory even when native allocation had returned null.
    // Capture its actual raw identity; no binding/storage access after terminal.
    release_native_render_actual_owner(bindings.node_terminal_owners,
        &released_node->storage);

    unwind.state = 3;
    release_parent(effect, bindings.parent_links);
    unwind.state = 2;
    release_template(effect);
    unwind.state = 1;
    destroy_point_effect_entry_array_008675b0(effect.auxiliary_18);
    unwind.state = 0;
    destroy_point_effect_entry_array_008675b0(effect.entries_0c);
    unwind.state = -1;
    destroy_base(effect);
}

PointEffectInstanceStorage* delete_point_effect_instance_00867ce0(
    PointEffectInstanceStorage* effect, std::uint32_t flags,
    PointEffectTeardownBindings bindings) {
    destroy_point_effect_instance_00867680(*effect, bindings);
    if ((flags & 1u) != 0) singleton_lifetime_free(effect);
    return effect;
}

} // namespace bsp
