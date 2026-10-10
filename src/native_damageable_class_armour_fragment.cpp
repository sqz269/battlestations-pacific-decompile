#include "bsp/native_damageable_class_armour_fragment.hpp"
#include <intrin.h>

namespace bsp {
namespace {
struct ArmourCleanup {
    NativeDamageableClassArmourFragmentScratch& scratch;
    unsigned state = 1;

    void release_field() {
        state = 1; // 0087CCD5, before actual Lua owner/index cleanup
        destroy_native_lua_object_00b67700(
            *static_cast<NativeLuaObjectStorage*>(scratch.fresh_field_at_parent_44));
    }
    ~ArmourCleanup() noexcept {
        if (state == 7) release_field();
    }
};
} // namespace

void read_native_damageable_class_armour_fragment_0087cca8(
    void* descriptor, NativeLuaObjectStorage& row,
    NativeDamageableClassArmourFragmentScratch& scratch) {
    ArmourCleanup cleanup{scratch};
    auto* field = native_lua_get_by_name_protected(row, scratch.fresh_field_at_parent_44, "Armour");

    float fallback;
    __asm {
        fldz
        fstp fallback
    }
    cleanup.state = 7; // 0087CCC1, after the native zero-default staging
    // Preserve the genuine provider's C++ float return. The local spill/reload
    // is an explicit source-ABI difference; the actual raw receiver needs no
    // invented C++ float lifetime or copied semantic descriptor.
    float result = native_lua_number_or_00b66330(*field, fallback);
    __asm {
        mov eax, descriptor
        fld result
        fstp dword ptr [eax + 4ch]
    }
    _ReadWriteBarrier(); // publish the HP-adjacent field before lowering state
    cleanup.release_field();
}
} // namespace bsp
