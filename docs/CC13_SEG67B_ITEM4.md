# CC13_SEG67B_ITEM4: seg67 plane-state mismatches, cc9 adoption plan item 4

Addresses: 009A76E0, 009B7C90, 009B5C80, 009C3100, 007B9500, 007BCCF0, 009A771C, 009A7932, 009A78BB.

Packet cc13-seg67b (worker cc13-undef, tree agent/cc13-undef), 2026-10-19.
The method was static reads of the disk bytes, the exports and the current host. There were no game runs.
The cc9 host files were edited only under a one-commit lease. The switch is in
`include/bsp/cc13_seg67b_switches.hpp`, and it is OFF.

| Address | Host | Verdict |
| --- | --- | --- |
| `009A76E0` dogfight aim tick | `dogfight_task.cpp` `dogfight_aim_tick_009a76e0`, wrapper `game_hosts_units.cpp` (kAim arm) | **mismatch, live**: plan+2B0h = 1 is not stored. Wired behind `kSeg67bDogfightAimSpeedCorrOffBound` (OFF). Two minor notes follow. |
| `009B7C90` level-bomb approach update | `plane_level_bomb_task.cpp` `level_bomb_approach_update_009b7c90` | matches, except the 31h (MParatrooper) arm, which was already labelled (VERIFY8 L6, VERIFY11 S14). Not wired: it needs the paratrooper rack path. |
| `009B5C80` level-bomb aim tick | `plane_level_bomb_task.cpp` `level_bomb_aim_tick_009b5c80` | matches (the release and abort tail 009B639A-009B6465 was re-read against the bytes) |
| `009C3100` moveto arrival | `game_hosts_units.cpp` `moveto_arrival_end_command_009c3100` | matches, with the labelled substitutions (per-member command rows; no squadron gives an early return) |

## 009A76E0: plan+2B0h in the not-head-on arm

The bytes, at 009A790B-009A794B: `ECX = [approach+18h]` (the pilot plan). The arm then does these stores:

- `MOV byte [ECX+2B0h], 1` at `009A7932`;
- `MOV dword [ECX+2D8h], 1` at `009A7939`;
- `FSTP [ECX+2B4h]` at `009A7947`, which stores `(approach+D4h - [approach+14h]+20Ch) + target vtable[38h]()`.

The host wrapper writes `plane_desired_speed_2b4` and `plane_air_brake_mode_2d8 = 1`. It never writes
`plane_trg_speed_corr_off_2b0`, so the field keeps the dogfight follow's 0 (`009C1C0D`).
`plane_trg_speed_corr_off_2b0` is the gate on the speed-correction raise in the pilot think,
`0099D924` (the host reader is under `kPilotSpeedCorrDivisorBound`, which is ON). With +2B0h = 0 and
+2D8h = 1, the host raises `plane_speed_corr_2b8` on every aim tick. The image's aim does not.

A byte scan of `C6 /0 [reg+2B0h], imm8` over `00996000-009FF000` found 35 stores, and `009A7932` is among them.
Within the dogfight states (`009A7000-009AB000`), only `009A7932` writes the byte. It writes 1, and no dogfight
state writes 0. So the 1 persists through maneuver and avoid until the follow (`009C1C0D`) or
another task's state writes 0. The host's dogfight states do not write the field either, which matches.

Switch `bsp::kSeg67bDogfightAimSpeedCorrOffBound` (OFF). When it is true, the not-head-on aim arm also stores
`unit_.plane_trg_speed_corr_off_2b0 = 1`. It does no lookup, so there is no container lifetime to trace,
and it makes no RNG draw.

Reach: `dogfight-engaged ... aim=` is non-zero in the current host logs for USN04 (A6M Zero #1.2
aim=164), JM02 (Buffalo 01 aim=897) and campaign 07 (Donald aim=293).

Predicted effect when ON: fighters in the aim state stop accumulating the pitch-driven speed correction
(`0099D924`), and the stored `+2B8h` decays (`0099D756`) instead. Their commanded speed in pursuit is then
`(distance - FollowDist) + target speed`, without the correction multiplier. The aim and engage timing
changes, and so do the dogfight outcomes. Effects reach maneuver and avoid until the next follow tick.
The `summary planner laws speed_corr raises=` counter falls.

Minor notes (not wired):

- `009A771C` `AND dword [approach+100h], 0FBh` (imm32 `000000FBh`) runs when local z > 1. The host does not
  model it (as at 009A7A50, CC13_SEG67B). No reader of bit 2 was found: the scan for TEST/OR forms on
  `+100h` with imm 4 is empty, and the `AND ... FBh` form occurs once, at 009A771C itself. The effect
  is unknown.
- The pitch at `009A78BB` passes `approach+D8h` raw as 009F9ED0's distance (`[ESP+28h]`, stored at
  `009A787C`). The host passes `max(df_horizontal_d8, 1.0f)`, a guard the image lacks. 009F9ED0 is
  `atan2(d, a)`, so the two differ only when the horizontal range is under 1 m.

## 009B7C90: the 31h arm (URGENT note sent to the lead)

`007B9500` is `007B91C0(plane; 31h, 0)`, and 31h is MParatrooper (ORDNANCE_KIND_IDENTITY.md); the
ledger name "HasLevelBombOrdnance" is a misnomer. The host sets `in.has_paratroopers = false`
(game_hosts_units.cpp, `lb_approach_update_009b7c90`). It differs from the image in two places:

- at 009B7CBF-009B7CF7, +CEh (has ordnance) also counts a 31h load. A paratrooper-only plane is
  "spent" in the host;
- at 009B7D7D-009B7E1F, with 31h, the impact point is `007BCCF0(unit)` (the release point) plus
  velocity x +A8h, with no +A4h. The host uses position + velocity x (+A8h + +A4h).

Everything else matches the bytes, including the NaN arms. That covers the latch and its 1.1 x +B8h band, the projection-time clamp, and
the leader-distance tail 009B803C-009B80E6 (JBE: unordered clears +CDh).

This is not wired. The host has no 31h rack path (006E3F90, and 006E541F with its stream-0 U(-50, 50) draws),
so turning on the +CEh half alone would put C-47s into attackrun and aim with no drop. Paratroopers are
scripted in JM05, JM11 and JM13 (and their PRCP copies) only.

## 009B5C80 and 009C3100

- `009B5C80`: CC13_AUDIT_PLANE_BOT_STATES row 10 (FAITHFUL) stands. The re-read covered 009B639A-009B6465 (the near-impact
  `JA`, the 1.0 > ratio abort gate, ctl+3AEh, the release bearing, and the step 3/1 with
  `b4 x 0.5 > d` `JA`). The wrapper's `force_release_3ae = false` holds because no script calls
  SquadronSetForceRelease (CC13_VERIFY8 L5).
- `009C3100` (`009C3100-009C3135`, `RET`, ECX = approach): `task vt[40h]` non-zero leads to
  `0071E430(squadron vt[114h](), 00E08F68, 0)`. The host's labelled substitutions (CC13_MOVETO1) stand. The
  image's no-squadron arm would dereference a null `approach+0Ch`, so the host's early return
  diverges only where the image would fault.
