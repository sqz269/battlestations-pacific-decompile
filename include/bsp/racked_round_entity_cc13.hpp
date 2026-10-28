#pragma once

// Packet cc13-rackround: torpedo rounds on a plane's racks as Lua-addressable
// entities, for 008BEF80 GetSquadTorpedoes(squadron, true) and Kill(round, true)
// (USN11 / PRCP11 PrepareS1Phase4 and PrepareS2Phase4; docs/CC13_FIX21.md
// section 1). Part A, this commit: the units-host API. Part B wires the Lua side.

namespace bsp {

// kRackedRoundEntityCc13Bound (OFF). ON: GameUnitsHost::racked_torpedo_rounds
// lists the single racks of a plane that hold a torpedo round, and
// GameUnitsHost::kill_racked_round empties one, as the image's Kill of that round
// does. OFF: both return without reading or writing anything.
//
// Image (read for this packet):
// - The round. 006E3E70 (single rack vtable[224h]) creates it through the bullet
//   class [[rack+3F8h]+34h]->vtable[20h], for a torpedo 00856420
//   BSP_TorpedoClass_CreateProjectile, which returns the +310h subobject. The
//   round is then parented to the rack by round->vtable[ACh] (00924F90
//   BSP_EntitySetParent: +3Ch = rack, appended to the rack's +48h child list),
//   bound to the plane by 006E6BB0, and +484h is decremented.
// - (c) Registration. 00856420 calls entity vtable[98h] = 00928860
//   BSP_GameEntity_PlaceInWorld (torpedo entity vtable 00D0C3E8, slot +98h at
//   00D0C480) with the world [[00E188A8]+19CCh]. For a new entity the parent
//   differs from +30h, so 009288E9-009288F1 calls vtable[130h] = 00856360, which
//   pushes the round onto the world lists +30h, +210h and +21Ch. 00924F90 changes
//   only +3Ch and the parent's +48h chain, not +30h, so a racked round stays on
//   +21Ch: GetSquadTorpedoes' list.
// - (b) The flag. (e+310h)->vtable[38h] is 008561F0 (the +310h vtable 00D0C378,
//   slot +38h at 00D0C3B0): `MOV AL,[ECX+148h]`, the byte at entity+458h. The
//   torpedo constructor 00856050 clears it (00856146) and the launch 006E1F00 (the
//   drop's mover->vtable[34h], 006E529C) sets it (006E1FA3). So it means
//   "launched": a racked round answers 0 and is listed only with the flag set.
// - (a) The kill and the rack. Kill(round, true) is 008AC5C0 -> 00926D90(round, 2):
//   +5Fh = 1, vtable[70h](1) while +60h is clear, its children killed, and the
//   round pushed on the kill list 00F899B4. The next 009273A0 flush sets
//   +5Dh/+5Eh/+5Fh, clears +5Ch and dispatches vtable[80h]
//   (docs/UNIT_DAMAGE_AND_DEATH.md rows 9-10; for the torpedo, slot +80h at
//   00D0C468 is 00779AF0, which destroys nothing). From then on +5Eh keeps the
//   round out of GetSquadTorpedoes. UNREAD: where the released round leaves the
//   rack's +48h chain (the base destructor 00925780 unlinks a child through
//   00924710; what deletes a released entity was not traced). 006E3FE0 walks +48h
//   with no flag test, so the rack answers the dead round until that unlink.
//   Nothing re-attaches a round after the kill. A single rack has no child-death observer (006E3840 is the
//   multiple rack's), and vtable[224h] is reached only from 006E3350 (the loadout
//   bind), 006E3410 (rearm), the drop tail 006E55FA, and the messages 00758EF5
//   (MotherShip) and 00822149 (UnitInstance). So after the kill:
//     - with +484h == 0 (one round, the usual torpedo rack) the rack is empty:
//       006E3570 answers it, 006E3FE0/006E4060 find no 2Ah child, and 006E41C0
//       reloads it after ReloadTime only while [00E17BF2] is set;
//     - with +484h > 0 the rack keeps that reserve but holds no round: 006E3FE0
//       and 006E4060 find no child (their descriptor arm still needs the reload
//       byte and the loadout flag), the drop finds no 2Ah child (006E4EAF) and so
//       never reaches its re-attach, and 006E41C0 never reloads it because
//       006E3570 is false. Only a rearm (006E3410) gives it a round again.
//
// Host model, ON:
// - racked_torpedo_rounds runs the rack census (Impl::plane_rack_census) and the
//   first-issue seed (Impl::rack_release_issue_seed_007c0de3), then lists every
//   single rack whose bullet class is 2Bh with a seeded count above 0. These are
//   the counts the drops (006E4D50 sites) and rack_holds_round read.
// - kill_racked_round sets that rack's count to 0 and takes the same rounds off
//   rack_ammo (floored at 0). When no 2Bh rack has a round left it clears the
//   torpedo bit 2Bh of the stored ordnance mask, as a torpedo drop does
//   (game_hosts_gunnery.cpp, packet cc8_torpedo_breakoff). The 007C1D80 rearm
//   puts both back.
// - LABELLED, +484h > 0: the host keeps one count per rack, so "a reserve and no
//   round" is stored as 0. Answers and drops then match the image. The device
//   reload differs: the host refills a 0 rack while [00E17BF2] is set, and the
//   image never reloads that rack. USN11/PRCP11 call SetDeviceReloadEnabled(false)
//   right after the kill loop, so this does not arise there.
// - LABELLED, timing: the image's rack answers its dead child until the child is
//   unlinked (after the 009273A0 flush; the exact point is unread above). The
//   host empties the rack at the Kill call.
// - LABELLED: a MultiBombPlatform's rounds (+514h slots) are not listed.
//
// Idempotence: the census and the seed run once. Census: `rack_census_done`.
// Seed: every seeding writer of rack_ammo / rack_ammo_per_rack is guarded by
// `rack_ammo < 0`: the first-issue seed (rack_release_issue_seed_007c0de3), the
// level bomber's lb_rack_state_seed (007C0D90 path), and the census's own bag-0
// branch. kill_racked_round keeps rack_ammo >= 0, so the normal first issue that
// runs after a Lua-time seed seeds nothing, and a killed rack stays at 0. The only
// writers that raise the count again are the image's rearms: 007C1D80
// (plane_reload_bomb_platforms_007c1d80) and the device-reload slice 006E57A4.
// Calling the census and the seed at Lua time is ON-only. OFF, the moved members
// run exactly where they did before the move.
inline constexpr bool kRackedRoundEntityCc13Bound = false;

}  // namespace bsp
