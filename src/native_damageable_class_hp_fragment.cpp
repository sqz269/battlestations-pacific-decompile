#include "bsp/native_damageable_class_hp_fragment.hpp"
#include <intrin.h>

namespace bsp {
namespace {
// Retained parent audit: original CE3D08 contains float32 100.0 (42C80000h).
// This is the observed HP fallback, not a generic missing-value policy.
const float original_hp_default = 100.0f;

struct HpCleanup {
    NativeDamageableClassHpFragmentScratch& scratch;
    unsigned state = 1;

    void release_field() {
        state = 1; // 0087CC9B, before current Lua owner/index cleanup
        destroy_native_lua_object_00b67700(
            *static_cast<NativeLuaObjectStorage*>(scratch.fresh_field_at_parent_44));
    }
    ~HpCleanup() noexcept {
        if (state == 6) release_field();
    }
};
} // namespace

void read_native_damageable_class_hp_fragment_0087cc66(
    void* descriptor, NativeLuaObjectStorage& row,
    NativeDamageableClassHpFragmentScratch& scratch) {
    HpCleanup cleanup{scratch};
    auto* field = native_lua_get_by_name_protected(row, scratch.fresh_field_at_parent_44, "HP");

    float fallback;
    __asm {
        fld original_hp_default
        fstp fallback
    }
    cleanup.state = 6; // 0087CC83, after original default staging
    // The actual receiver may be raw storage with no C++ float subobject.
    // Keep the provider's ordinary C++ float return, then perform the native
    // float32 publication through x87 without inventing a typed receiver.
    // The extra local float spill/reload is a documented source-ABI boundary.
    float result = native_lua_number_or_00b66330(*field, fallback);
    __asm {
        mov eax, descriptor
        fld result
        fstp dword ptr [eax + 48h]
    }
    _ReadWriteBarrier();
    cleanup.release_field();
}
} // namespace bsp
