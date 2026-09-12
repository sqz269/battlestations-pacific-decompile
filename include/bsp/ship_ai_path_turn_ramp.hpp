#pragma once

#include "bsp/ship_ai_path_search.hpp"

#include <string>

struct lua_State;
namespace bsp {
struct GameplayTuningSettings;
struct NativeLuaObjectStorage;

// Partial projection of0083B5E0: complete three-field read/store/release
// sequence0083D492..0083D575, not a standalone native function. Requires the
// actual selected ShipGlobals.Navigator.PathFinderParams Lua object and its
// live owner. Reuses native14h tracked objects and00B67800/00B66330/00B67700.
// Stores knee_x,limit_x,limit_y in that order, mapping settings+6F0/+6F4/+6F8.
// Exact NUMBER only; absent/non-number fields use original binary32 fallbacks
// 3E860A92,3FC90FDB,44BB8000. Numeric strings take the fallback. No degree
// conversion, finite check, clamp or ordering repair occurs here.
// Earlier writes remain visible if a later Lua lookup raises an error.
void load_ship_ai_path_turn_ramp_0083d492(NativeLuaObjectStorage& path_finder,
    ShipAiPathSearchTurnRamp& output);

// C++ adapter for an already-loaded real Lua5.1.1 ShipGlobals environment.
// Selects ShipGlobals.Navigator.PathFinderParams through native lookup helpers
// inside lua_pcall, then executes the fragment above. Metamethods are honored.
// Parent-table lookup errors return false with error text, preserving stores
// already completed; the caller must reject that failed load. The stack top
// is restored on both paths. True clears error. Does not execute scripts,
// replace DoFile, invent a tuning table, or cache installed-script literals.
// Uses an isolated C-call frame and temporary tracking owner; this adapter is
// not the original singleton/loader initialization or native exception ABI.
bool read_ship_ai_path_turn_ramp_lua(lua_State&, ShipAiPathSearchTurnRamp&,
    std::string& error);

// Value projection of an actual loaded settings snapshot, reusing its existing
// asserted field layout. These are009EC310/323/336 reads after00424C40 calls;
// this helper does not reconstruct the singleton's ownership or initialization.
ShipAiPathSearchTurnRamp ship_ai_path_turn_ramp_from_settings(
    const GameplayTuningSettings&) noexcept;
} // namespace bsp
