#include "bsp/native_damageable_class_chance_threshold_fragment.hpp"
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Chance and Threshold reconstruction requires MSVC Win32 x87.
#endif

namespace bsp {
namespace {
// Retained metadata values, not new reads/bindings of the original constant cells.
const float original_chance_default = -100.0f; // CE65D8, C2C80000h
const double original_chance_divisor = 100.0; // D7A220, 4059000000000000h
const float original_threshold_default = -1.0f; // D7A260, BF800000h

struct NumericFieldCleanup {
    NativeDamageableClassChanceThresholdFragmentScratch& scratch;
    unsigned state = 17;

    ~NumericFieldCleanup() noexcept {
        if (state == 23 || state == 24) {
            state = 17;
            destroy_native_lua_object_00b67700(
                *static_cast<NativeLuaObjectStorage*>(scratch.fresh_field_at_parent_6c));
        }
    }
};
static_assert(sizeof(void*) == 4 && sizeof(unsigned) == 4);
} // namespace

void read_native_damageable_class_chance_threshold_fragment_0087d0fb(
    void* row, NativeDamageableClassChanceThresholdFragmentScratch& scratch) {
    NumericFieldCleanup cleanup{scratch};
    unsigned* const active_state = &cleanup.state;
    auto& value = *static_cast<NativeLuaObjectStorage*>(
        scratch.same_msh_scratch.same_iterator_scratch.fresh_value_at_parent_2c);
    auto* field = native_lua_get_by_name_protected(value,
        scratch.fresh_field_at_parent_6c, scratch.actual_failure_chance_key_00d0ac88);
    float fallback;
    __asm {
        fld original_chance_default
        fstp fallback
    }
    _ReadWriteBarrier();
    cleanup.state = 23; // D11A, after default staging
    float number = native_lua_number_or_00b66330(*field, fallback);
    // The ordinary provider result's local binary32 spill/reload is qualified.
    // No quotient spill occurs between the binary64 FDIV and raw final FSTP.
    __asm {
        mov edx, active_state
        mov eax, row
        fld number
        fdiv original_chance_divisor
        mov dword ptr [edx], 11h // D131: lower23->17 with quotient still in ST0
        fstp dword ptr [eax + 28h]
    }
    _ReadWriteBarrier();
    destroy_native_lua_object_00b67700(*field);

    field = native_lua_get_by_name_protected(value, scratch.fresh_field_at_parent_6c,
        scratch.actual_failure_damage_threshold_key_00d0ac70);
    __asm {
        fld original_threshold_default
        fstp fallback
    }
    _ReadWriteBarrier();
    cleanup.state = 24; // D160, after default staging
    number = native_lua_number_or_00b66330(*field, fallback);
    __asm {
        mov edx, active_state
        mov eax, row
        fld number
        fstp dword ptr [eax + 2ch] // D16D: still24
        mov dword ptr [edx], 11h // D174: lower24->17 only AFTER the row store
    }
    _ReadWriteBarrier();
    destroy_native_lua_object_00b67700(*field);
}
} // namespace bsp
