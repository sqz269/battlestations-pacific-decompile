#pragma once

// cc13-launchaxes (census S8, src/game_hosts_script_orders.cpp
// create_air_ops_squadron_006c5050). 006C5050 places a launched squadron with its
// owner's (the carrier's or airfield's) whole world matrix:
//  - 006C5267-006C5275: owner = [block+7Ch]; when the pose-valid byte owner+C8h is
//    clear, 00414DB0 refreshes the pose first;
//  - 006C527A-006C5294: ESI = owner+CCh, ECX = 10h, EDI = esp+130h, REP MOVSD. All
//    16 dwords are copied bit for bit: +CCh right, +DCh up, +ECh forward (each with
//    its w lane) and +FCh the translation. No transform, no re-orthonormalisation;
//  - 006C5298-006C52C5 (flag set only) then replace dword 13 (y), nothing else;
//  - 006C52D7-006C52E3: &esp+130h goes to 004F0AD0 as the new unit's world 4x4.
// So the squadron's body axes are the owner's rows 0..2 on both flag arms.
// The ON arm copies lanes 0..2 of those rows only; the w lanes (world[3], [7], [11])
// keep the host record's identity values (0) rather than owner+D8h/+E8h/+F8h
// (SUBSTITUTION, labelled: unit_pose returns three-lane rows; it is not checked here
// whether the image's owner w lanes always hold 0).
//
// The host built the squadron record with identity axes (world[0], [5], [10] = 1),
// a SUBSTITUTION labelled at the kBaseLaunchChainBound arm.

namespace bsp {

// True: record.world rows 0..2 take the owner's pose rows (GameUnitsHost::unit_pose,
// the host's unit+CCh/+DCh/+ECh), copied unchanged; the wing records copy the
// squadron record, so they take the same axes. Only inside the kBaseLaunchChainBound
// arm, which models the 006C5294 owner copy (its OFF arm is the older airborne
// over-the-deck placement and keeps the identity). False: identity axes, as before.
// Movers when ON: the initial heading, pitch and roll of every air-ops launched
// squadron and its wings (carrier and airfield launches, both flag arms), so the
// first steps of their flight, the inside-start deck placement and launch timing
// downstream of it.
inline constexpr bool kBaseLaunchAxesCc13Bound = false;

}  // namespace bsp
