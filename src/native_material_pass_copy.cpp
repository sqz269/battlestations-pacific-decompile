#include "bsp/native_material_pass_copy.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Reserve = void (*)(NativeMaterialStateArray&, std::int32_t);
using CopyRows = NativeMaterialStateArray* (*)(NativeMaterialStateArray&, const NativeMaterialStateArray&);
std::int32_t doubled_capacity(std::int32_t old) noexcept {
    const auto bits = static_cast<std::uint32_t>(old) * 2u;
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value > 1 ? value : 1;
}
NativeMaterialStateArray* append_rows(NativeMaterialStateArray& destination,
    const NativeMaterialStateArray& source, std::size_t stride, Reserve reserve) {
    reserve(destination, source.count_04);
    for (std::int32_t i = 0; i < source.count_04; ++i) {
        if (!source.data_00 || destination.count_04 < 0)
            throw std::logic_error("native row copy requires readable source and valid destination extent");
        // Capture source row BEFORE possible destination growth; reload source
        // header/end on the next iteration. Copy DWORDs in original order.
        const auto* from = static_cast<const std::byte*>(source.data_00) + static_cast<std::size_t>(i) * stride;
        if (destination.count_04 == destination.capacity_08)
            reserve(destination, doubled_capacity(destination.capacity_08));
        auto* to = reinterpret_cast<std::byte*>(reinterpret_cast<std::uintptr_t>(destination.data_00)
            + static_cast<std::size_t>(destination.count_04) * stride);
        if (to) for (std::size_t offset = 0; offset < stride; offset += 4) {
            std::uint32_t word;
            std::memcpy(&word, from + offset, 4);
            std::memcpy(to + offset, &word, 4);
        }
        ++destination.count_04;
    }
    return &destination;
}
NativeMaterialStateArray* copy_rows(NativeMaterialStateArray& destination,
    const NativeMaterialStateArray& source, std::size_t stride, Reserve reserve) {
    if (destination.capacity_08 < 0) reserve(destination, 0);
    while (destination.count_04 > 0) --destination.count_04;
    destination.count_04 = 0;
    return append_rows(destination, source, stride, reserve);
}
void assign_actual(void*& slot, void* incoming, NativeRenderActualOwners& owners) {
    void* const old = slot;
    if (old == incoming) return;
    slot = incoming;
    if (incoming) {
        auto* count = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
            static_cast<std::byte*>(incoming) + 4));
        count->fetch_add(1, std::memory_order_seq_cst);
    }
    if (old) release_native_render_actual_owner(owners, old);
}
void replace_state(NativeMaterialStateOwnerStorage*& destination,
    NativeMaterialStateOwnerStorage* const& source, NativeMaterialStateKind kind,
    CopyRows copy, NativeMaterialPassCopyAccess& access) {
    auto* const old = destination;
    if (old) {
        release_native_render_actual_owner(access.lifetime.retained_owners, old);
        destination = nullptr;
    }
    auto* fresh = initialize_native_material_state_owner_00b5f720_fragment(
        singleton_lifetime_allocate({SingletonAllocationKind::object,20,20}), kind);
    destination = fresh;
    // Source slot is reread AFTER release, allocation and publication. The
    // original plain MOV copies its counter into independent storage, even
    // when the value is greater than one; it does not normalize ownership.
    auto* const current_source = source;
    fresh->references_04.store(current_source->references_04.load(std::memory_order_relaxed), std::memory_order_relaxed);
    access.bind_state(access.binding_context, *fresh);
    copy(fresh->rows_08, current_source->rows_08);
}
} // namespace

NativeMaterialStateArray* copy_native_material_render_rows_00b41c10(
    NativeMaterialStateArray& destination, const NativeMaterialStateArray& source) {
    return copy_rows(destination, source, 8, reserve_native_material_render_states_00b40ac0);
}
NativeMaterialStateArray* copy_native_material_third_rows_00b41ce0(
    NativeMaterialStateArray& destination, const NativeMaterialStateArray& source) {
    return copy_rows(destination, source, 12, reserve_native_material_third_states_00b40b40);
}
NativeMaterialStateArray* copy_native_material_sampler_rows_00b41dd0(
    NativeMaterialStateArray& destination, const NativeMaterialStateArray& source) {
    return copy_rows(destination, source, 12, reserve_native_material_sampler_states_00b40be0);
}
NativeMaterialStateArray* copy_native_material_pass_pairs_00b41e80(
    NativeMaterialStateArray& destination, const NativeMaterialStateArray& source) {
    resize_native_material_pass_pairs_00b40e00(destination, 0);
    return append_rows(destination, source, 8, reserve_native_material_pass_pairs_00b40c80);
}
NativeMaterialPassBindingStorage* initialize_native_material_pass_binding_00b44690(
    void* raw, std::uint32_t word, void* retained, const NativeString& source,
    NativeMaterialPassDestructionAccess& access) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % alignof(NativeMaterialPassBindingStorage))
        throw std::invalid_argument("native binding requires aligned fresh storage");
    auto* binding = ::new(raw) NativeMaterialPassBindingStorage;
    binding->retained_04 = nullptr;
    binding->word_00 = word;
    try {
        if (&binding->name_08 != &source) {
            resize_native_string_header_0041dd40(&binding->name_08, access.strings, source.length(), true);
            if (source.length()) std::memcpy(binding->name_08.data(), source.data(), binding->name_08.length());
        }
        // String callbacks can have changed the destination retained slot.
        assign_actual(binding->retained_04, retained, access.retained_owners);
    } catch (...) {
        destroy_native_string_header_0041dd20(&binding->name_08, access.strings);
        throw;
    }
    return binding;
}
void copy_native_material_pass_00b455c0(NativeMaterialPassStorage& destination,
    const NativeMaterialPassStorage& source, NativeMaterialPassCopyAccess& access) {
    if (!access.bind_state) throw std::invalid_argument("native pass copy requires canonical state registration");
    copy_native_material_pass_pairs_00b41e80(destination.base.pairs_24, source.base.pairs_24);
    replace_state(destination.base.render_18, source.base.render_18, NativeMaterialStateKind::render,
        copy_native_material_render_rows_00b41c10, access);
    replace_state(destination.base.sampler_20, source.base.sampler_20, NativeMaterialStateKind::sampler,
        copy_native_material_sampler_rows_00b41dd0, access);
    replace_state(destination.base.third_1c, source.base.third_1c, NativeMaterialStateKind::third,
        copy_native_material_third_rows_00b41ce0, access);
    assign_actual(destination.base.retained_54, source.base.retained_54, access.lifetime.retained_owners);
    assign_actual(destination.base.retained_58, source.base.retained_58, access.lifetime.retained_owners);
    assign_actual(destination.vertex_shader_70, source.vertex_shader_70, access.lifetime.retained_owners);
    assign_actual(destination.pixel_shader_74, source.pixel_shader_74, access.lifetime.retained_owners);
    destination.index_78 = source.index_78;
    destination.index_7c = source.index_7c;
    for (std::uint32_t i = 0; i < source.binding_count_6c; ++i) {
        if (i >= source.bindings_5c.size() || !source.bindings_5c[i])
            throw std::logic_error("native pass copy requires readable source bindings");
        const auto* const from = source.bindings_5c[i];
        const auto word = from->word_00;
        void* const retained = from->retained_04;
        const auto* const name = &from->name_08;
        void* const raw = singleton_lifetime_allocate({SingletonAllocationKind::object,16,16});
        try {
            auto* fresh = raw ? initialize_native_material_pass_binding_00b44690(raw, word, retained, *name, access.lifetime) : nullptr;
            const auto index = destination.binding_count_6c;
            if (index >= destination.bindings_5c.size())
                throw std::logic_error("native pass append exceeds four actual binding slots");
            destination.bindings_5c[index] = fresh;
            ++destination.binding_count_6c;
        } catch (...) {
            // B455C0 state0 owns only the raw binding allocation. Its ctor's
            // own member unwind handles the name; no top-level rollback.
            singleton_lifetime_free(raw);
            throw;
        }
    }
    set_native_material_pass_effect_00b172b0(destination.base.root, source.base.root.borrowed_effect_14);
}
void build_native_material_secondary_pass_00b45e00(NativeMaterialEffectStorage& effect,
    NativeMaterialPassConstructionAccess& construction, NativeMaterialPassCopyAccess& copy,
    NativeMaterialSecondaryPassRegistration registration) {
    if (&construction.lifetime != &copy.lifetime || !registration.bind_pass)
        throw std::invalid_argument("secondary pass requires one canonical lifetime and pass registration");
    for (std::uint32_t i = 0; i < effect.secondary_100.size(); ++i) {
        if (i == 0 && effect.passes_c8[0]) {
            void* const raw = copy.lifetime.pool.allocate_slot_00b41210();
            NativeMaterialPassStorage* pass;
            try { pass = raw ? initialize_native_material_pass_00b44b10(raw, construction) : nullptr; }
            catch (...) { if (raw) copy.lifetime.pool.return_slot_00b40a40(raw); throw; }
            auto* const source = static_cast<const NativeMaterialPassStorage*>(effect.passes_c8[0]);
            effect.secondary_100[0] = pass;
            if (!pass || !source) throw std::logic_error("secondary pass requires live allocation and primary after construction");
            registration.bind_pass(registration.context, *pass);
            copy_native_material_pass_00b455c0(*pass, *source, copy);
            set_native_material_render_state_00b5ec40(effect.secondary_100[0], 0xf, 0);
            set_native_material_render_state_00b5ec40(effect.secondary_100[0], 0x1b, 1);
            set_native_material_render_state_00b5ec40(effect.secondary_100[0], 0x13, 5);
            set_native_material_render_state_00b5ec40(effect.secondary_100[0], 0x14, 6);
            static_cast<NativeMaterialPassStorage*>(effect.secondary_100[0])->base.root.word_08 = 1;
        } else effect.secondary_100[i] = nullptr;
    }
}
} // namespace bsp
