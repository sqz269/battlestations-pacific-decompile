#pragma once

// Wiring switch for src/game_hosts_units.cpp (packet wire-units).
//
// OFF (the default): every wired site runs the host's existing code, byte for byte.
// ON: each wired site calls the reconstructed body named in the comment above its
// `if constexpr (kWireUnitsHostBound)` arm, with host state mapped into its parameters.
// tools/pair_export.py flips this line with --flip kWireUnitsHostBound=true.

namespace bsp::game {

inline constexpr bool kWireUnitsHostBound = true;

}  // namespace bsp::game
