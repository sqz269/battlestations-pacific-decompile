#include "bsp/native_damageable_class_section_iterator_advance_fragment.hpp"

namespace bsp {

bool advance_native_damageable_class_section_iterator_0087d1a0(
    NativeDamageableClassSectionIteratorSetupFragment&,
    NativeDamageableClassSectionIteratorSetupFragmentScratch& scratch) {
    auto& key = *static_cast<NativeLuaObjectStorage*>(scratch.fresh_key_at_parent_58);
    auto& value = *static_cast<NativeLuaObjectStorage*>(scratch.fresh_value_at_parent_2c);
    native_lua_iterate_next_protected(scratch.live_sections_at_parent_80, key, value);
    // Native B66420 ignores ECX/Sections and tests its stacked key, not value.
    return !native_lua_is_unbound_00b66420(key);
}

} // namespace bsp
