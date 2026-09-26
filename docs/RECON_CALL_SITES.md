# The recon triples at their three HUD readers (packet `cc9_recon_call_sites`)

Addresses: 004C3CF8, 004C3D61, 004C3EB4 (004C3CB0's walk heads), 00526E01, 00526FF1 (00526A40's
list heads), 005C1610 (the minimap's list head).

The recon record of a slot publishes five lists: own +DD8h (head +DDCh), enemy +DE4h (+DE8h),
neutral +DF0h, unknown +DFCh (+E00h) and their union +E08h (+E0Ch). The gunnery host now builds
them (`GameGunneryHost::recon_triple_units`, packet `cc9_recon_team_lists`). This packet points
the three readers at them under one switch, `kReconUnitListSourcesBound`
(`include/bsp/game_hosts_world.hpp`). Off, the stand-ins stay: walk 0 gets every created unit,
walks 1 and 2 are empty, the pick screen filters the created units by the controlled unit's party,
and the minimap walks every unit.

## 1. The readers

| Reader | Load | Reads | Rebuilt |
|---|---|---|---|
| 004C3CB0 walk 0 | 004C3CF8 `MOV EBP,[EAX+0DDCh]` | triple 0, own | once per scene load |
| 004C3CB0 walk 1 | 004C3D61 `MOV EBP,[EAX+0DE8h]` | triple 1, enemy | once per scene load |
| 004C3CB0 walk 2 | 004C3EB4 `MOV EBP,[EAX+0E00h]` | triple 3, unknown | once per scene load |
| 00526A40, first list | 00526E01 `MOV EAX,[EAX+1974h]` | game+1970h, walk 0's non-ordnance | read every call |
| 00526A40, second list | 00526FF1 `MOV EAX,[EAX+19BCh]` | game+19B8h, the merged list | read every call |
| 005C1600 minimap walk | 005C1610 `MOV EBX,[ECX+0E0Ch]` | triple 4, union | read every frame |

`EAX` at 004C3CF5 and `ECX` at 005C160D are `[game+18CCh + game+18ECh*4]+30h`, the local slot's
recon record. 004C3CB0 runs once after a load: its latch `game+193Ch` is cleared only by
004DFB70 (docs/IN_MISSION_SUBSYSTEM_TICK.md). In that load, step 19 runs 008073C0 immediately
before 004C3CB0 (docs/MISSION_SCENE_LOAD.md). So the pick screen's two lists hold the triples of
one pass at load time for the whole mission, while the minimap reads the union live.

**Substitutions.**
- The slot record is taken as the triples of the controlled unit's side (+54h), through
  `game_local_recon_triple`. The image indexes by slot. In single player they are the same
  record; this is not verified for other sessions.
- The host has no load-time pass. The first frame's in-mission tick runs call 1, the fixed-step
  driver (whose gunnery step runs 008073C0 on its first step, timer 0), before call 2, 004C3CB0.
  So the lists are built from the first frame's pass, not from the load's.
- The lists are exposed through `game_local_player_unit_lists()`, a process-wide accessor. The
  image's lists are fields of the process-wide game object at [00E188A8].

## 2. Every test between a triple and each consumer

Publishing the triples (008073C0, the gunnery host):
- 008074D5..008074EE: the unit's four gate bytes (present);
- 008074F2: IsKindOf(2);
- 00806480: only the 22 scanned class ids;
- 008065FF: the relation of the unit's side to the slot's;
- steps 9 and 10: level 2 keeps a unit in its relation's triple, level 1 copies it into unknown,
  level 0 drops it. Own units need no detection.

004C3CB0 (`src/local_player_unit_lists.cpp`, already reconstructed):
- all walks: +5Ch set, +5Dh, +60h and +5Eh clear (004C3D0C..004C3D1E, 004C3D79..004C3D94,
  004C3ECB..004C3EE6);
- walk 0: ordnance (2Ah) dropped at 004C3D2D; ships (6) go to +1964h at 004C3D45; everything
  else kept goes to +1970h at 004C3D51;
- walk 1, first match wins: ships 004C3DA7; squadrons (18h) 004C3E09; planes (0Fh) dropped
  004C3E23; airfields 004C3E36; shipyards 004C3E4D; land forts 004C3E64; dummy targets (35h) into
  the merged list 004C3E7B; the local objective set (008DDF90) into the merged list 004C3E97;
- walk 2: the same without 0Fh, 35h and 008DDF90 (004C3EF9..004C4033);
- merge 004C405B..004C4093: ships, squadrons, airfields, shipyards and forts appended to +19B8h.

00526A40, for each entry (`src/hud_warning_screen.cpp`):
- 00526E40: IsKindOf(1), otherwise skipped;
- 00526F33: a squadron (18h) stands for up to five members (+3D0h, count +3CCh);
- per member: not the ray exclusion, +5Dh clear (00526E9A), IsKindOf(5) (00526EA9), and either in
  the grey-arrow set (008DDF90) or neither 1Ah (00526EC0) nor 19h (00526ED3);
- the nearest by screen distance wins (00526F9C, strict); 0052721A..0052723A drop a pick that is
  not alive and visible.

The minimap walk 005C1600: alive and visible (005C1628..005C164A), not the camera unit
(005C1650), IsKindOf(5) (005C165D), vtable +B8h (005C1675), then the distance cull (005C16DB).

## 3. What the readers will show once group records exist

The gunnery worker is building the squadron and convoy aggregates (00805490/00805680). When class
18h and 1Ah records enter the triples:
- A squadron (chain [18h,2,1,0]) is not ordnance. Walk 0 keeps it in +1970h, and walk 1 files it
  under squadrons, which merge into +19B8h. It is the only kind-1 entry, so it is what 00526A40
  can pick. Each one contributes up to five members. The host's `member_3d0` is still a
  substitution answering 0, so until members are exposed a squadron adds no candidate.
- A squadron fails the minimap's IsKindOf(5), so it draws no icon; its planes, if they are in the
  union as units of their own, do.
- A 1Ah record is dropped by walk 1 and walk 2 unless it matches a later test (none of 6, 18h,
  0Fh, 45h, 46h, 1Bh, 35h), and the pick excludes 1Ah members outside the grey-arrow set.

## 4. Predictions (written before the pairs; the switch committed OFF)

One tree, `local\bin\rc_off` against `local\bin\rc_on`, `BSP_GUNNERY_RNG_STREAMS=1`, 1600x900.
USN04 4700/4500 and USN02 9200/9000.
- **Census line** `local-player unit lists sources`: on both sides it logs the triples at the
  build. Own holds the player side's present units of the scanned classes. Enemy and unknown are
  0, because one pass after one frame cannot reach the blip threshold.
- **The lists.**
  - Without the binding, +1970h holds every created unit, both sides: 21 in USN04, 28 in USN02.
  - With it, +1970h holds the own triple only, minus ordnance.
  - +19B8h is empty on both sides.
- **The pick screen.** Its rows become `UnitPickScreen::list_1970` and `list_19b8` (concrete),
  replacing `team_unit_list` and `kind35_list`. No pick changes: ships and planes fail 00526E40's
  IsKindOf(1) on both sides, and there are no squadron records yet. `UnitPickScreen::owner_140`
  keeps its count.
- **The minimap.**
  - `HudMinimap::team_unit_list` is replaced by `HudMinimap::union_triple`.
  - The per-unit rows after it (`alive_and_visible`, `is_kind_of`, `unit_shows_on_minimap`,
    `world_position`) fall. Undetected enemies drop out of the walk, and detected ones appear
    only after a refresh.
- **Gameplay.** No gameplay row, per-entity row or summary line moves. The readers are HUD
  reads and a list only the pick screen reads. If one moves, that is the finding.
