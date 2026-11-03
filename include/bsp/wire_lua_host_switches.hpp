#pragma once
// Packet wire-lua: the single OFF switch for the reconstructed bodies wired into
// src/game_hosts_lua.cpp. OFF keeps every stand-in byte for byte; ON runs the
// reconstructed bodies named in each `if constexpr (kWireLuaHostBound)` arm.
// Flip only through tools/pair_export.py --flip kWireLuaHostBound=true.

namespace bsp {

inline constexpr bool kWireLuaHostBound = true;

}  // namespace bsp
