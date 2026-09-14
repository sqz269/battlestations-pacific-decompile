#include "bsp/native_unit_part_construction.hpp"
#include "bsp/native_unit_part_groups.hpp"
#include "bsp/native_physical_file_date.hpp"

namespace bsp {
void NativeUnitPartConstructionBindings::call_00713380(void* model) {
    build_native_unit_part_groups_00713380(model);
}
namespace {
template<class T> volatile T& ordered(T& cell) noexcept { return cell; }

void unwind(NativeUnitPartConstructionView v, int state, NativeString& temporary,
    NativeStringRawPoolContext& strings, NativeUnitPartConstructionBindings& b) noexcept
{
    // FuncInfo DB33F4 / map DB3418: each entry's successor is state-1.
    // The native state7 destructor uses the same canonical raw pool getter.
    switch (state) {
    case 7: destroy_native_string_header_0041dd20(&temporary, strings); [[fallthrough]];
    case 6: b.call_00711000(v.entries_1a0); [[fallthrough]];
    case 5: b.call_00710fc0(v.list_194.identity); [[fallthrough]];
    case 4: b.call_00710fc0(v.list_188.identity); [[fallthrough]];
    case 3: b.call_00710f90(v.list_178.identity); [[fallthrough]];
    case 2: b.call_00712b40(v.groups_identity_168); [[fallthrough]];
    case 1: b.call_00711080(&v.selected_set_160); [[fallthrough]];
    case 0: b.call_004e6570(v.identity); [[fallthrough]];
    default: break;
    }
}
}

void* construct_native_unit_part_007135c0(NativeUnitPartConstructionView v,
    void* unit, void* selected_set, NativeStringRawPoolContext& strings,
    NativeUnitPartConstructionBindings& b)
{
    int state = -1;
    NativeString temporary;
    try {
        ordered(v.primary_table_00) = 0x00ce89e8;
        b.call_004e6480(v.identity);
        ordered(v.owner_alias_4c) = unit;
        ordered(v.primary_table_00) = 0x00cfd7b8;
        state = 0;
        ordered(v.selected_set_160) = selected_set;
        ordered(v.owner_164) = unit;
        ordered(v.groups_begin_16c) = nullptr;
        ordered(v.groups_end_170) = nullptr;
        ordered(v.groups_capacity_174) = nullptr;
        state = 2;
        void* head = b.call_007103a0(v.list_178.identity);
        ordered(v.list_178.head_04) = head;
        ordered(v.list_178.count_08) = 0;
        state = 3;
        ordered(v.attached_184) = 0;
        head = b.call_007103c0(v.list_188.identity);
        ordered(v.list_188.head_04) = head;
        ordered(v.list_188.count_08) = 0;
        state = 4;
        head = b.call_007103c0(v.list_194.identity);
        ordered(v.list_194.head_04) = head;
        ordered(v.list_194.count_08) = 0;
        ordered(v.entries_1a0.data_00) = nullptr;
        ordered(v.entries_1a0.count_04) = 0;
        ordered(v.entries_1a0.capacity_08) = 0;
        state = 6;
        b.call_00713380(v.identity);
        if (unit) {
            const auto entry = b.primary_table(unit)[0x10 / 4];
            const char* text = b.call_unit_10(entry, unit);
            construct_native_string_cstring_0041e870(&temporary, text, strings);
            void* current_set = ordered(v.selected_set_160);
            NativeNodeStorage* root = b.render_root_0c(current_set);
            state = 7;
            b.call_00b6f960(root, temporary);
            state = 6;
            destroy_native_string_header_0041dd20(&temporary, strings);
        }
        void* current_owner = ordered(v.owner_164);
        ordered(v.owner_alias_4c) = current_owner;
        if (unit) {
            b.call_00712440(v.identity);
            if (ordered(v.list_188.count_08) != 0) b.call_00710ad0(v.identity);
        }
        b.call_00711c60(v.identity);
    } catch (...) {
        unwind(v, state, temporary, strings, b);
        throw;
    }
    return v.identity;
}
} // namespace bsp
