#include "bsp/native_damageable_class_damage_entry_fragment.hpp"

namespace bsp {

NativeDamageableClassDamageEntryFragment::NativeDamageableClassDamageEntryFragment(
    NativeDamageableClassDamageEntryFragmentScratch& scratch) noexcept
    : scratch_(scratch) {}

NativeDamageableClassDamageEntryFragment::~NativeDamageableClassDamageEntryFragment() noexcept {
    close();
}

bool NativeDamageableClassDamageEntryFragment::open(NativeLuaObjectStorage& row) {
    auto* damage = native_lua_get_by_name_protected(
        row, scratch_.fresh_damage_at_parent_44, "Damage");
    state_ = 10;
    if (!native_lua_is_table_00b661b0(*damage)) {
        close();
        return false;
    }

    auto* sections = native_lua_get_by_name_protected(
        *damage, scratch_.fresh_sections_at_parent_80, "Sections");
    state_ = 11;
    if (!native_lua_is_table_00b661b0(*sections)) {
        close();
        return false;
    }
    return true; // Native CDA9; both actual objects remain live.
}

void NativeDamageableClassDamageEntryFragment::close() {
    if (state_ == 11) {
        state_ = 10; // Native D1FF, before actual Sections cleanup.
        destroy_native_lua_object_00b67700(
            *static_cast<NativeLuaObjectStorage*>(scratch_.fresh_sections_at_parent_80));
    }
    if (state_ == 10) {
        state_ = 1; // Native D210, before actual Damage cleanup.
        destroy_native_lua_object_00b67700(
            *static_cast<NativeLuaObjectStorage*>(scratch_.fresh_damage_at_parent_44));
    }
}

} // namespace bsp
