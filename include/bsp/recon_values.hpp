#pragma once

#include <array>
#include <cstdint>

struct lua_State;

namespace bsp {

// Identity-preserving projection of the Lua instance's mutable +04 slot.
// Native ownership: game+1A08 -> mission host, host+04 -> instance,
// instance+04 -> lua_State. This is not the embedded game+1A0C Lua owner.
struct ReconLuaInstanceView {
    lua_State*& state_04;
};

struct ReconValuesHost {
    virtual ~ReconValuesHost() = default;
    // Resolve the actual current game/mission host/instance each time. Return
    // its real state slot, not a copied state pointer or another Lua owner.
    virtual ReconLuaInstanceView current_mission_lua_instance_1a08_04() = 0;
};

inline constexpr std::array<const char*, 19> recon_category_names_00e0b590{{
    "mothership", "destroyer", "torpedoboat", "battleship", "cruiser",
    "cargo", "landingship", "levelbomber", "divebomber", "torpedobomber",
    "fighter", "reconplane", "kamikaze", "submarine", "landvehicle",
    "landfort", "airfield", "shipyard", "path"
}};

struct ReconValuesContext {
    ReconValuesHost& host;
    // Live pointer table with exactly 19 entries, read one entry per category.
    // Source bindings can use recon_category_names_00e0b590.data(). A native
    // binding must preserve the actual table at00E0B590..00E0B5DB.
    const char* const* category_names_00e0b590;
};

// The native 8h stack object holds vtable00D08E5C at+00 and the borrowed
// instance at+04. This C++ projection holds a reference to its real state slot;
// vtable_00 records the native identity and is not a callable C++ vtable.
// There is deliberately no automatic destructor or ownership transfer.
struct ReconTableScope {
    std::uint32_t vtable_00;
    ReconLuaInstanceView instance_04;
};

//006B8190: ECX=instance, stack const char* key, RET4. Pushes the global value;
// only nil creates an empty table. Uses ordinary Lua get/set (metamethods).
void push_recon_global_table_006b8190(ReconLuaInstanceView, const char* key);

//006B8210: ECX=instance; tail JMP006B8218 to lua_settop(state,-2).
void pop_recon_global_table_006b8210(ReconLuaInstanceView);

//00803750 /008037D0: ECX=fresh 8h scope, stack(instance,key), EAX=scope, RET8.
// Raw lookup in the current top table; create only when nil. Captures state
// once for the whole helper, and ignores the native lua_checkstack result.
ReconTableScope push_recon_index_table_00803750(ReconLuaInstanceView,
    std::int32_t key);
ReconTableScope push_recon_named_table_008037d0(ReconLuaInstanceView,
    const char* key);

// Recovered inlined scope destruction: reset vtable then pop through the
// scope's borrowed instance's CURRENT state slot. No registry reference/free.
void pop_recon_table_scope(ReconTableScope&);

//008039E0: no arguments, RET00803A34. Reload current mission instance for
// EACH category, ensure its raw child, then pop through that captured instance.
void install_recon_category_tables_008039e0(ReconValuesContext&);

//00803A40: no arguments, RET00803B4D. Complete normal flow; creates the recon
// hierarchy for integer indices0..2 and enemy/neutral/unknown/own. Installs no
// numeric values. Lua type/stack validity and unprotected error behavior stay
// native preconditions. New C++ interface, not a binary-compatible replacement.
void install_recon_values_00803a40(ReconValuesContext&);

} // namespace bsp
