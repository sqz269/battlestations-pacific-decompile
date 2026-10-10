#include "bsp/native_damageable_class_explosion_type_fragment.hpp"
#include <cstddef>
#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {
struct ExplosionTypeCleanup {
    NativeDamageableClassExplosionTypeFragmentScratch& scratch;
    unsigned state = 1;

    void release_field() {
        state = 1; // Native CD08/CD40, before actual Lua owner/index cleanup
        destroy_native_lua_object_00b67700(
            *static_cast<NativeLuaObjectStorage*>(scratch.fresh_field_at_parent_44));
    }
    ~ExplosionTypeCleanup() noexcept {
        if (state == 8 || state == 9) release_field();
    }
};
} // namespace

void read_native_damageable_class_explosion_type_fragment_0087cce2(
    void* descriptor, NativeLuaObjectStorage& row,
    NativeDamageableClassExplosionTypeFragmentScratch& scratch,
    const bool& conversion_mode) {
    ExplosionTypeCleanup cleanup{scratch};
    auto* field = native_lua_get_by_name_protected(
        row, scratch.fresh_field_at_parent_44, "ExplosionType");
    cleanup.state = 8;
    const bool integral = native_lua_is_integer_number_00b66a60(*field);
    cleanup.release_field();
    if (!integral) return;

    // The first field is dead. Lua callbacks and tracked-index changes make
    // this second lookup and numeric read distinct from the completed probe.
    field = native_lua_get_by_name_protected(
        row, scratch.fresh_field_at_parent_44, "ExplosionType");
    cleanup.state = 9;
    const std::int32_t value = native_lua_integer_00b66290(*field, conversion_mode);
    std::memcpy(static_cast<std::byte*>(descriptor) + 0x40, &value, sizeof(value));
    _ReadWriteBarrier(); // publish raw integer bits before lowering state
    cleanup.release_field();
}
} // namespace bsp
