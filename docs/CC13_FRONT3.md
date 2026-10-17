# CC13 FRONT3: flight and gun wiring blockers (segments 40, 47, 48)

Packet `cc13-front3` (2026-10-17). Code: `src/cc13_front3.cpp`, `include/bsp/cc13_front3.hpp`
(marker `cc13_front3`). Every name is a hypothesis, not a recovered symbol. Each function is a new
C++ interface over the raw native record: every call the image makes goes through a named
`Cc13Front3Host` hook at the same point with the same arguments. Nothing is wired. Status:
reconstructed, build-tested; not ABI-compatible, not game-validated.

Targets came from `local/output/wire_readiness.json` (main checkout). Four were already done and
were dropped after `bsp.py lookup`: 007F8210, 007EDF00 (confirmed outside the struck range
007EE000-007F2FFF), 007F74B0 and 007F7110 (all `reconstructed_build_tested` in
`src/cc13_seg48.cpp`).

None of these functions draws from the shared RNG 00BD2F10. None touches the plane level-bomb or
goaway tasks, torpedoes or submarine AI.

## Functions

| address | name | ABI | what the code does | unblocks (wire_readiness callers) |
| --- | --- | --- | --- | --- |
| 00732F50 | BSP_GunClassBase_Destruct | thiscall(class), RET | vtable 00CE4534; releases +C8h; frees +BCh and +9Ch (00BF65AC) and zeroes the triples; 00442A30(+74h; 0); _free 00BF6989 of [+74h]; base dtor 004420D0 | 0043EA80, 0043F120, 00442D80, 00442E20 |
| 00734DB0 | BSP_MissionNarrative_ApplyMissionInfoMessage | fastcall(message), RET | with [game+21E8h]: NativeString "" (0041E870), 00734870([game+21E8h]; message+18h, &s, 0), inline string destroy (00419CC0 / 00BD1510) | 00777850 (route receive) |
| 0072B540 | BSP_Gun_SetControlledAudioMix | thiscall(gun; bool), RET 4 | with +47Ch: 0072AA10(+47Ch; flag ? 0.0f : 1.0f) | 00954990 |
| 0072D830 | BSP_Gun_HandleNetworkMessage | thiscall(gun; message), RET 4, AL | 4Bh/4Ch true; 68h slot +1C8h; 69h slot +1CCh; ACh 0072D5B0(gun; [msg+20h], 0); ADh slot +1DCh; else tail 0043F860 | 006E5D70, 0084C7E0, 00803480, 006E4290, 006EC070 |
| 0072BD50 | BSP_GunBot_ClearFireTarget | thiscall(bot), RET | 00521EA0 twice then 006952A0(object, bot+1Ch); resets the record at bot+38h (static vector 00F87574) | 008FF2B0, 008FF500 |
| 00729F70 | BSP_Gun_ReleasePlayerControl | thiscall(gun), RET | slot +154h(0, 8), slot +1E8h(0), tail 00728000 | 00953F60, 00954000, 00959C20 |
| 007C11E0 | BSP_Plane_NotifyFlightState | thiscall(plane; byte), RET 4 | two actuator channels of [+DECh] (+28h and +44h); immediate form also sets values and calls 007EB270 with [+72Ch] slot +38h; deferred form resets +9F0h and controller +3Ch..+50h, then 007D9C80, 007DDEA0, 007EB380(0.0f) outside states 3..7; +C0Ch = 1 in 3/6/7; 007BC550 | 007C1430, 007C16F0, 007C3680, 007C6340, 007CB9E0, 007CC7A0, 007CC820, 007D5D20, 007C1680, 007C5F60, 007C7110 |
| 007CB7F0 | BSP_Plane_OnWaterContact | thiscall(plane), RET | gate 007BC5B0 or MinWaterSpd == 0; achievement 0090F6C0(kind 3) gate; 00414DB0; "powerlost" slot +194h; tail: client 007C1430(6) for 7/4/5, host message C3h through 0077C2A0(plane; &m, 7, 0) | 007CC2D0, 007CC2F0 |
| 007B34F0 | BSP_ScenePath_LoadSourceHolder | thiscall(path; holder), RET 4 | kind 1 property bag ("Group", "PathPoints", "Point%002i", "Pos") through 008F2260 / 004F9A80 / slot +8h; kind 2 12-byte point vector with the checked index (00BF6713); kind 3 saved-entity Lua reader (009238A0, 004425C0, 00BD8E20 "_pathBaseEntity", slot +18h, 00BD7A20, 00441A20); tail 007AF150 when +18h is clear | 007B38D0, 007B3920, 007B3940, 007B3730 |
| 007DC830 | BSP_PlaneFlight_FreeFlightStep | thiscall(controller; float step), RET 4 | core law 007DB680; stall / dead-meat force through 007DAFD0 (k 1.5f) with the Dynamics mirror 00F8730C..00F87368; dead-meat ctl+24h..2Ch += step * ctl+F0h..F8h; ctl+94h lift force (k 1000.0f); ctl+8Ch = 0 | 007DCCF0, 007CC2F0 |

Callers listed as `[wired]` in the report (00959C20, 006EC070, 007B3730, 007C1680, 007C5F60,
007C7110, 007CC2F0) are already wired through a host and only lose one missing callee.

## Floating-point notes

- 007C11E0: `a` is 0 in state 7 unless 5.0f (00CE3850) >= +908h (COMISS/JB, a NaN takes 0). The
  immediate form rewrites a channel when `!(value == v)` (FUCOMPI, LAHF/TEST 44h/JNP: a NaN value
  rewrites).
- 007CB7F0: MinWaterSpd == 0.0f via UCOMISS/LAHF/JP (unordered is not equal); 8.0f (00CE3918) >
  slot +204h and 0.1 (double 00D05E28) > +9F0h via FCOMI/JBE (unordered skips); +E0h > 0.8f as
  double 00CE3D40 via FCOMI/JA.
- 007DC830: the products g*m*d, f*desc+17Ch and q*u*h are kept wide (double) and rounded to float
  once, as the x87 stores them; `m = t unless m > t` (FCOMPI/JA, NaN takes t); the clamps keep a
  NaN (FCOMI/JBE, COMISS/JBE). Constants: -1.0f 00D7A260, 1.5f 00CE380C, 1000.0f 00CE3804, 4.0f
  00CE3D34, 69.444443f 00D0688C (250 km/h), 20.0f 00CE3930, -0.0f 00D7A208, doubles 1.0 00D7A210,
  0.800000011920929 00CE3D40, 0.6000000238418579 00CEFF98. The 00F873xx Dynamics mirror is
  zero-filled in the image and read through `dynamics_mirror_float(address)`.

## Adoption notes (hosts are read-only; nothing here edits them)

1. **007C11E0, `src/game_hosts_units.cpp` `wing_fold_notify_007c11e0` (14118-14145).**
   Labelled substitution. The host models only channel +44h (the wing fold) and detects a change of
   +900h once per fixed step instead of the image's calls at each site. It omits channel +28h
   (`a` above, including the state 7 / +908h > 5.0f test), the deferred reset of +9F0h and the
   controller velocity block with 007D9C80 / 007DDEA0 / 007EB380(0.0f) when the new state is
   outside 3..7, the immediate-form 007EB270 call, the +C0Ch latch for 3/6/7 and 007BC550.
   Live in a normal mission: yes, every flight-state change. No RNG.
2. **007CB7F0, `src/game_hosts_units.cpp` 19227-19305 (the free-flight arm, packet
   cc9_water_surface_law).** Labelled substitution. The host keeps the 007BC5B0 / MinWaterSpd
   gate and the SetFlightState(6) result. It omits the achievement call 0090F6C0 (007CB8A2), the
   pose refresh 00414DB0 (007CB8B6), the +E0h > 0.8 test, the "powerlost" slot +194h call
   (007CB903) and, on a host session ([game+1FE4h] != 2), the C3h message routed through
   0077C2A0 (007CB994). The comment at 19244 says the tail calls BSP_Plane_SetFlightState(6) when
   +900h is 7, 4 or 5; in the image that direct call is the client arm only (007CB941 -> 007CB9BD);
   a host routes C3h with the state byte and 6 instead, and any state other than 6 sends it.
   Live: yes, a plane touching the sea. No RNG.
3. **00729F70, `src/game_hosts_gunnery.cpp` 13377.** The host models the caller 00959C20's
   release as field writes (seat 8, trigger off, target cleared, a counter), not the three calls.
   The image calls slot +154h(0, 8), slot +1E8h(0) and 00728000. Live: yes, a player seat leaving
   a gun's arc. No RNG.
4. **007B34F0, `src/game_hosts_scene_contents.cpp` `retain_path_points`.** A partial projection of
   the kind-1 branch only (ledger fragment). The full function is in `cc13_front3`; the host's
   projection is unchanged.
5. **007DC830.** The units host counts the step and runs the core law directly
   (`src/plane_flight.cpp` 007DB680); the stall, dead-meat and ctl+94h forces of this function
   are not hosted. Live: every free-flight step (007CC2F0) and the ground-roll fallback (007DCCF0).

## Uncertainty

- 0090F6C0 is unnamed (strings SA_OC / SA_IM); the hook name `achievement_record` is a guess.
- 00734870 is unnamed (STL-probable); its three stack arguments and RET 0Ch come from the caller's
  stack balance.
- 007CB7F0 leaves the C3h message undestroyed on the normal path, as the image does (EH state 1
  only covers an unwind).
- The holder object layouts in 007B34F0 (kind-2 object +114h/+118h, 12-byte elements) are read
  only at the offsets the image uses.
