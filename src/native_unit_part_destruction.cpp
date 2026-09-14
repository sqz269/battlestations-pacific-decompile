#include "bsp/native_unit_part_destruction.hpp"
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/native_spatial_index_publication.hpp"
#include "bsp/native_unit_part_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = volatile std::uint32_t;
using Byte = volatile std::uint8_t;
using Pointer = void* volatile;
void* offset(void* object, std::uint32_t bytes) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(object) + bytes);
}
template<class T> T& field(void* object, std::uint32_t bytes) noexcept {
    return *static_cast<T*>(offset(object, bytes));
}
NativeRenderPointerArrayStorage& array(void* object, std::uint32_t bytes) noexcept {
    return field<NativeRenderPointerArrayStorage>(object, bytes);
}
void unwind_entry(void* entry) noexcept {
    destroy_native_instance_entry_pointers_00b1d1d0(array(entry, 4));
}
void unwind_part(void* part, int state, const NativeUnitPartDestructionAccess& access) noexcept {
    switch (state) {
    case 6: destroy_native_instance_entry_pointers_00b1d1d0(array(part, 0x1a0)); [[fallthrough]];
    case 5: destroy_native_unit_part_shape_list_007108e0(offset(part, 0x194)); [[fallthrough]];
    case 4: destroy_native_unit_part_shape_list_007108e0(offset(part, 0x188)); [[fallthrough]];
    case 3: destroy_native_unit_part_list_00710870(offset(part, 0x178)); [[fallthrough]];
    case 2: destroy_native_unit_part_group_rows_00712b40(offset(part, 0x168)); [[fallthrough]];
    case 1: release_native_unit_part_selected_set_00711080(offset(part, 0x160), access.selected_set); [[fallthrough]];
    case 0: destroy_native_collision_node_base_004e6570(part); [[fallthrough]];
    default: break;
    }
}
} // namespace

void __fastcall destroy_native_unit_part_entry_007112e0(void* entry,
    const NativeUnitPartDestructionAccess* access) {
    bool member_live = true;
    try {
        for (std::uint32_t i = 0; static_cast<std::int32_t>(i) <
            static_cast<std::int32_t>(field<Word>(entry, 8)); ++i) {
            void* const slot = offset(field<Pointer>(entry, 4), i * 4u);
            if (field<Pointer>(slot, 0)) {
                void* const effect = field<Pointer>(slot, 0);
                stop_point_effect_00867b10(*static_cast<PointEffectInstanceStorage*>(effect),
                    access->effects, access->effect_manager_00f87650, access->effect_lifetime);
                field<Byte>(effect, 9) = 1;
                field<Pointer>(field<Pointer>(entry, 4), i * 4u) = nullptr;
            }
        }
        member_live = false; // native state -1 precedes the member destructor
        destroy_native_instance_entry_pointers_00b1d1d0(array(entry, 4));
    } catch (...) {
        if (member_live) unwind_entry(entry);
        throw;
    }
}

void __fastcall destroy_native_unit_part_00712c80(void* part,
    const NativeUnitPartDestructionAccess* access) {
    field<Word>(part, 0) = 0x00cfd7b8u;
    int state = 6;
    try {
        if (field<Pointer>(part, 0x164) && field<Byte>(part, 0x184)) {
            const auto& spatial = access->spatial;
            void* const index = get_native_spatial_index_0042e630(
                *spatial.manager_publication_01090aa0, *spatial.index_publication_00f8a0d8);
            detach_native_spatial_node_0098a500(index, nullptr, part);
            field<Byte>(part, 0x184) = 0;
        }
        void* const selected = field<Pointer>(part, 0x160);
        field<Word>(part, 0xf8) = 0;
        if (selected && field<Pointer>(selected, 0x0c)) {
            void* const root = field<Pointer>(selected, 0x0c);
            if (access->listener.controlled_listener_00e188dc == root)
                publish_native_controlled_listener_004bca80(nullptr,
                    access->listener, access->listener_renderer);
        }
        release_native_unit_part_selected_set_00711080(offset(part, 0x160), access->selected_set);
        field<Pointer>(part, 0x160) = nullptr; // second unconditional native store
        for (std::uint32_t i = 0; static_cast<std::int32_t>(i) <
            static_cast<std::int32_t>(field<Word>(part, 0x1a4)); ++i) {
            void* const entry = field<Pointer>(field<Pointer>(part, 0x1a0), i * 4u);
            if (entry) {
                destroy_native_unit_part_entry_007112e0(entry, access);
                singleton_lifetime_free(entry);
            }
        }
        resize_native_instance_entry_pointers_00b1c770(array(part, 0x1a0), 0);
        state = 5;
        destroy_native_instance_entry_pointers_00b1d1d0(array(part, 0x1a0));
        destroy_native_unit_part_shape_list_007108e0(offset(part, 0x194));
        destroy_native_unit_part_shape_list_007108e0(offset(part, 0x188));
        destroy_native_unit_part_list_00710870(offset(part, 0x178));
        destroy_native_unit_part_group_rows_00712b40(offset(part, 0x168));
        state = 0;
        release_native_unit_part_selected_set_00711080(offset(part, 0x160), access->selected_set);
        destroy_native_collision_node_base_004e6570(part);
    } catch (...) {
        unwind_part(part, state, *access);
        throw;
    }
}

void* __fastcall delete_native_unit_part_00712fd0(void* part,
    const NativeUnitPartDestructionAccess* access, std::uint32_t flags) {
    destroy_native_unit_part_00712c80(part, access);
    if (flags & 1u) singleton_lifetime_free(part);
    return part;
}
} // namespace bsp
