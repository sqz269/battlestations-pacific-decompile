#include "bsp/native_damageable_class_section_iterator_setup_fragment.hpp"

namespace bsp {

NativeDamageableClassSectionIteratorSetupFragment::NativeDamageableClassSectionIteratorSetupFragment(
    NativeDamageableClassSectionIteratorSetupFragmentScratch& scratch) noexcept
    : scratch_(scratch) {}

NativeDamageableClassSectionIteratorSetupFragment::~NativeDamageableClassSectionIteratorSetupFragment() noexcept {
    close();
}

bool NativeDamageableClassSectionIteratorSetupFragment::open() {
    auto* key = construct_native_lua_object_00b65f50(scratch_.fresh_key_at_parent_58);
    state_ = 12;
    auto* value = construct_native_lua_object_00b65f50(scratch_.fresh_value_at_parent_2c);
    state_ = 13;
    native_lua_iterate_first_protected(scratch_.live_sections_at_parent_80, *key, *value);
    // Native B66420 reads its stacked key; its table/ECX input is ignored.
    if (native_lua_is_unbound_00b66420(*key)) {
        close();
        return false;
    }
    return true; // Native CE00; actual key/value and both outer fields stay live.
}

void NativeDamageableClassSectionIteratorSetupFragment::close() {
    if (state_ == 13) {
        state_ = 12; // Native D1D3, before VALUE cleanup.
        destroy_native_lua_object_00b67700(
            *static_cast<NativeLuaObjectStorage*>(scratch_.fresh_value_at_parent_2c));
    }
    if (state_ == 12) {
        state_ = 11; // Native D1E4, before KEY cleanup.
        destroy_native_lua_object_00b67700(
            *static_cast<NativeLuaObjectStorage*>(scratch_.fresh_key_at_parent_58));
    }
}

} // namespace bsp
