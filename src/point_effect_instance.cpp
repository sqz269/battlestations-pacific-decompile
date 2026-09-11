#include "bsp/point_effect_instance.hpp"
#include "bsp/native_node_pool_allocation.hpp"

#include <cstring>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Point effect instance storage requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(PointEffectReferenceArray) == 12);
static_assert(sizeof(PointEffectInstanceStorage) == 0x114);
static_assert(offsetof(PointEffectInstanceStorage, references_04) == 0x04);
static_assert(offsetof(PointEffectInstanceStorage, option_08) == 0x08);
static_assert(offsetof(PointEffectInstanceStorage, entries_0c) == 0x0c);
static_assert(offsetof(PointEffectInstanceStorage, auxiliary_18) == 0x18);
static_assert(offsetof(PointEffectInstanceStorage, tail_word_24) == 0x24);
static_assert(offsetof(PointEffectInstanceStorage, field_28) == 0x28);
static_assert(offsetof(PointEffectInstanceStorage, fields_30) == 0x30);
static_assert(offsetof(PointEffectInstanceStorage, field_48) == 0x48);
static_assert(offsetof(PointEffectInstanceStorage, field_4c) == 0x4c);
static_assert(offsetof(PointEffectInstanceStorage, fields_50) == 0x50);
static_assert(offsetof(PointEffectInstanceStorage, fields_5c) == 0x5c);
static_assert(offsetof(PointEffectInstanceStorage, untouched_74) == 0x74);
static_assert(offsetof(PointEffectInstanceStorage, field_80) == 0x80);
static_assert(offsetof(PointEffectInstanceStorage, template_84) == 0x84);
static_assert(offsetof(PointEffectInstanceStorage, owner_88) == 0x88);
static_assert(offsetof(PointEffectInstanceStorage, parent_8c) == 0x8c);
static_assert(offsetof(PointEffectInstanceStorage, cached_world_90) == 0x90);
static_assert(offsetof(PointEffectInstanceStorage, relative_d0) == 0xd0);
static_assert(offsetof(PointEffectInstanceStorage, node_110) == 0x110);

PointEffectTemplateArgument::PointEffectTemplateArgument(
    RenderCommandReference* reference) noexcept : reference_(reference) {}
PointEffectTemplateArgument::~PointEffectTemplateArgument() {
    if (reference_) release_render_command_reference(*reference_);
}

PointEffectInstanceStorage& initialize_point_effect_instance_008680d9(
    void* actual_storage, const PointEffectTemplateArgument& argument,
    std::uint8_t option_byte, std::uint32_t tail_word,
    PointEffectInstanceCounters counters) {
    if (!actual_storage || reinterpret_cast<std::uintptr_t>(actual_storage) %
        alignof(PointEffectInstanceStorage) != 0)
        throw std::invalid_argument("point effect requires aligned actual114h storage");

    // Default initialization, not value initialization; preserve native gaps.
    auto& effect = *::new (actual_storage) PointEffectInstanceStorage;
    effect.original_vtable_identity_00 = 0x00ceb130u;
    effect.references_04.store(1, std::memory_order_relaxed);
    effect.original_vtable_identity_00 = 0x00d0d3ecu;
    effect.option_08 = option_byte;
    effect.field_09 = 0;
    effect.field_0a = 0;
    effect.entries_0c = {nullptr, 0, 0};
    effect.auxiliary_18 = {nullptr, 0, 0};
    effect.tail_word_24 = tail_word;
    effect.field_28 = 1;
    effect.field_2c = 1;
    effect.fields_30.fill(0.0f);
    effect.fields_50[0] = 1.0f; // MOVSS from00D7A24C, exact3F800000
    effect.fields_50[1] = 1.0f;
    effect.fields_50[2] = 0.0f;
    effect.field_80 = 0.0f;
    effect.template_84 = nullptr;
    if (argument.get()) {
        effect.template_84 = argument.get();
        retain_render_command_reference(*argument.get());
    }
    effect.owner_88 = nullptr;
    effect.parent_8c = nullptr;
    ++counters.actual_00f87604;
    ++counters.actual_00f87600;
    return effect;
}

NativeNodeStorage* construct_point_effect_node_00868193(void* pool,
    const NativeString& name, NativeStringStorage& strings) {
    return construct_point_effect_node_00868193(pool, &name, strings);
}
NativeNodeStorage* construct_point_effect_node_00868193(void* pool,
    const void* name, NativeStringStorage& strings) {
    void* const slot = allocate_native_node_00b6ed70(pool);
    if (!slot) return nullptr;
    try {
        return &construct_native_node_00b6f5a0(slot,
            native_node_pool_payload_bytes, name, strings);
    } catch (...) {
        return_native_node_00b6e670(slot, pool);
        throw;
    }
}

void register_point_effect_node_and_parent_008681be(PointEffectInstanceStorage& effect,
    NativeNodeBinding& constructed_node, CameraTransform* parent,
    std::uint32_t third_stack_word, NativeNodeDestructionRuntime& nodes,
    PointEffectInstanceLinks& links) {
    effect.node_110 = &constructed_node;
    constructed_node.storage.references_04.fetch_add(1, std::memory_order_seq_cst);
    if (third_stack_word != 0 || parent != nullptr) {
        auto* root = links.root_e188a8_19ec();
        // Native reloads+110 after obtaining the current root word.
        propagate_native_node_root_00b6d890(nodes, effect.node_110->transform, root);
    }
    auto* previous = effect.parent_8c; // reload AFTER registration callbacks
    if (previous) {
        release_render_command_reference(links.parent_reference(*previous));
        effect.parent_8c = nullptr; // overwrites any terminal callback replacement
    }
    effect.parent_8c = parent;
    if (parent) retain_render_command_reference(links.parent_reference(*parent));
}

void cache_point_effect_initial_world_0086826e(PointEffectInstanceStorage& effect) {
    auto& transform = effect.node_110->transform; // captured ESI; no reload after refresh
    if ((transform.valid_flags & 2u) == 0) refresh_camera_world_00b6db70(transform);
    CameraMatrix snapshot;
    std::memcpy(snapshot.data(), transform.world.data(), sizeof(snapshot));
    copy_camera_matrix_004134f0(effect.cached_world_90, snapshot);
    constexpr std::uint32_t native_00d7a238 = 0x3c23d70au;
    std::memcpy(&effect.field_4c, &native_00d7a238, sizeof(native_00d7a238));
    effect.field_48 = 0.0f;
    effect.fields_5c.fill(0.0f);
}

} // namespace bsp
