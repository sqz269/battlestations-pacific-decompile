# Squadron member placement: 007F2920 (packet `cc9_squadron_member_placement`, cc9-lua43, 2026-10-06)

Addresses: 007F2920, 007F2E20, 007F4DB0, 007F23A0, 008A9F90

## 1. Why this was read

SQUADRON_LAND_TASK 5ei, item 1, described Nell6|.-3 on USN01 (`cc9-ships36\local\s36_u1w6.log`).
Its level-bomb `follow law` speed fell from 115.6 to 32.4, it held throttle 0, and it flew into the
sea at 348.8 s.

The speed law is not what is wrong:
- The commanded 32.40 is the level-flight end of 009BEE30's alignment ramp,
  0.9 x LevelFlight x StallSpd.
- 115.6, 83.0 and 44.5 are intermediate alignments.
- The plane gets this speed because it points away from its station while chasing it from
  9 km away.

The 9 km is the actual fault:

| step | log line or source | result |
| --- | --- | --- |
| spawn | `plane spawn` | Nell6|.-2 and Nell6|.-3 are created at the world origin |
| PutTo | `PutTo: Nell6 pos=(-4619.1, 1200.0, 8193.8) placed=1` | only the fused leader slot moves |
| first formation line | `plane formation geometry: ... tick=0 ... pairwise=[0-1=9406 0-2=9409 1-2=3.3]` | the wingmen are 9.4 km from the leader |
| trajectory | `s36_traj_u1w6.Nell6__-3.csv` | x goes from 0 to -3288 m while the leader flies towards +x |

All six USN01 Nell squadrons (Nell1-Nell6) are moved this way.

## 2. The image

**PutTo (008A9F90).** PutTo calls the entity's vtable[118h]. On the PlaneSquadron vtable `00D087C0`,
that slot (`00D088D8`) is `007F2E20`, whose body runs to 007F2EA7 (RET 4, then INT3):
- 007F2E3D-007F2E70 is the session-only arm (`[00E188A8]+1FE4h == 1`).
- 007F2E88 calls the base position set `00489760`.
- 007F2E8F calls `007F2920`.

**`007F2920`** has its body at 007F2920-007F2BC9: `__fastcall(squadron)`, RET, then INT3. It was read
from the listing:

1. With `+3CCh` = 0 it does nothing (007F2929).
2. It sets `+408h` = 0 (007F293C) and keeps the leader speed from vt[38h] (007F294A; on a plane this
   is 007B8E60).
3. Member 0 gets the squadron's `+CCh` through vt[88h] (007F298B), unless `00923810(1)`. It then
   runs `007C6340` and vt[3Ch](speed), followed by `007C18B0(0)`, vt[D8h] and `[+310h]+0Ch`.
4. For each member k >= 1 (007F2A50-007F2BB8):
   - **Matrix:** the leader's `+74h` block (007F2A68 REP MOVSD, 10h dwords).
   - **Translation:** the station `007F23A0(member+9D0h)`, written at 007F2A78.
   - **Height floor** (007F2A83-007F2AEC):
     - If the station's y is below 10.0 (`[00CE38B8]` float) and the leader's y (`+100h`) is above
       it, y becomes the leader's y.
     - If that leader y is also above 10.0 (`[00CE3DC0]` double), y becomes 10.0 instead.
   - vt[88h](matrix) at 007F2B23, `007C6340` at 007F2B27, and vt[3Ch] with the leader's speed at
     007F2B3C (`[ESP+1Ch]`, which is the 007F294C store after the `PUSH EBP`).
   - `007C18B0(0)`, vt[D8h], `[+310h]+0Ch` and the `+804h` physics sync.

**Callers.** `tools/callsite_census.py 007f2920` finds four rel32 calls:

| call site | what it is |
| --- | --- |
| 007F2E8F | PutTo's vt[118h] |
| 007F2F74 | vt[11Ch], the heading set |
| 007F2FC6 | not read |
| **007F4DB0** | pass C `007F4BA0`, skipped when 007F4DA9 finds `+408h` set (a deck launch) |

So an airborne squadron's members are placed on their stations at pass C as well. This
contradicts SQUADRON_SPAWN_SEATS.md section 1 ("the image spawns the wing stacked"). That section
read only 007F4580 and did not follow pass C.

## 3. The host

| item | detail |
| --- | --- |
| routine | `GameUnitsHost::Impl::place_squadron_members_007f2920` (src/game_hosts_units.cpp) |
| coverage | member loop: complete; member-0 arm: fused (see substitutions) |
| the station | from the existing 007F23A0 model, `place_wing_member_on_station_007f23a0`, called without applying it and without its report line |
| site (a): PutTo | `GameUnitsHost::place_squadron_members_007f2920(index)`, called from PutTo in `GameScriptOrdersHost` behind `kSquadronPutToMembersBound` |
| site (b): pass C | `on_squadron_pass_c_initial_command`, after the base-launch arm and before the 007F4E0C command test, behind `Impl::kSquadronPassCPlacementBound` |
| log | one `squadron members placed:` line per call, and a `summary squadron member placement` line |

**SUBSTITUTIONS, labelled:**
- The squadron is fused with member 0, so the member-0 arm is the squadron's own placement.
- `007C6340`, `007C18B0`, vt[D8h], `[+310h]+0Ch` and the `+804h` sync have no counterpart in the
  host. A member keeps its flight state.
- vt[3Ch] is skipped for a plane whose velocity is not seeded yet. Its seed is the class
  TravelSpeed along the copied nose, which is the speed the leader is seeded with.

## 4. Predictions (switch OFF -> ON), written before any ON run

**(a) PutTo, USN01 u1w6 order file:**
- Each of the six Nell squadrons logs one `squadron members placed: ... site=put_to members=2`. The
  members land about 141 m from the leader: the stations are (-100, 0, -100) and (100, 0, -100),
  bomber triple.
- The tick-0 `pairwise` falls from about 9400 to about 141/141/200.
- Nell6|.-3 no longer flies into the sea at 348.8 s.
- Nell wingmen reach aim and release sooner and more often: more `levelbomb` releases, and an
  earlier first release per squadron.
- Enterprise's sinking time moves; the expected direction is earlier, because more bombs arrive.
- No other row's PutTo targets a squadron: USN13l (Maru), USNRM01 (PT, Rescue) and LOMP10 (PT)
  are unchanged.

**(b) Pass C, USN04 / LOMP10:**
- Every airborne squadron logs `site=pass_c` at spawn.
- Tick-0 pairwise distances go from 0.000 (stacked) to the station spacing.
- Avoidance between wingmates at spawn drops.
- A deck launch (`base launch inside start`) logs no pass-C placement.

## 5. Measured: both **ON** (cc9-lua43, 2026-10-06)

The pairs are same-tree at 926a8f826, each flip on its own. Logs are in `local\l43_*`.

**(a) PutTo, USN01 14200/14000, lua42's p5 order file.**
- pair_diff exits 3 (moved).
- Six `site=put_to` lines, two members each.
- The tick-0 Nell6 pairwise distance went from 9295/9298/3.3 to 141.4/143.8/200.0, as predicted.

| row | OFF | ON |
| --- | --- | --- |
| mission end | none | **completed at 692.95 s** (`Mission.MissionStatus`, "We showed we can fight back! - Mission Complete!") |
| deaths | 89 | 94 |
| plane water contacts | 23 | 25 |
| units | 123 | 126 |

- **New deaths.** In ON, Nell5|.-3 and Nell6 also die; these are the two survivors in 5ei item 2. Every Nell is down, so the script ends the mission. Three Wildcats die as well: Enterprise_sqn01, Enterprise_sqn01|.-2 and Enterprise_sqn02|.-2.
- **Why the deaths moved.** In OFF, each wing arrived strung out over 9 km behind its leader. In ON, a wing arrives together, so the Wildcats meet three bombers' return fire at once. Two examples from the killer columns:
  - Nell6|.-3 killer range: 636 -> 143.
  - Nell4|.-3 killer range: 293 -> 78.
- **The exec guard.** The mission completes, and the guard's line is in the ON log: `bsp: refused a mission script's process launch: sus_prog.exe`.
- **Not predicted:** the extra three units in ON come from the later phase the script reaches.

**(b) Pass C.**

USN04 4700/4500:
- 17 `site=pass_c` placements of 34 members, `unseeded=0`.
- The 4 `base launch inside start` squadrons log no placement, so the deck-launch skip holds.
- Deaths 48 -> 50: D3A Val #7.1|.-2 and |.-3 die only in ON.
- Torpedo releases 3 -> 5 of 16, drops 3 -> 5.
- Hypothesis, not traced per plane: without the stacked start, the wings skip the avoidance
  break-up at spawn and reach the fleet in formation.

LOMP10 9200/9000:
- 3 placements of 7 members.
- Deaths 5 -> 4: Warhawk 01|.-4 survives.
- Dive-bomb releases 14 -> 16.

**Verdict.** Both mechanisms match the listing: the members land at the 007F23A0 stations, and the deck launches are excluded. Both are ON.

This reverses SQUADRON_SPAWN_SEATS.md 6a ("the image spawns the wing stacked"). That conclusion came from 007F4580 alone; pass C's 007F4DB0 places the wing.

`kPlaneFormationPlacementEnabled`, the first-step stand-in, stays OFF. Pass C now does in the image's place what that stand-in did.


## 6. Follow-up pairs on HEAD (lead's re-check list, cc9-lua43, 2026-10-06)

OFF is `local\l43_noplace`: HEAD 52dc012b4 with both switches false. ON is this tree's build at cc996267d. Its
only other difference is two diagnostic log lines in the units host, which print only on a takeoff-site
denial or a lift-off that misses an occupant.

**USN01 20200/20000 with ships37's `s37_u1_p5.txt`: the win is lost in ON, and the cause is the harness.**

| | OFF | ON |
| --- | --- | --- |
| mission end | completed at 712.75 s | none |
| deaths | 91 | 35 |
| units | 123 | 64 |

- The difference comes from the order file's line 2, `release ScoutDauntless` at frame 2400:
  - OFF: applied at 133.65 s.
  - ON: refused, "the sight never came within 15.0 m (nearest 16.5 m)".
- With no hit on Convoy1, the script never generates the convoy torpedo bombers (ConTBD1 and the rest), so
  the mission stalls.
- Pass C put ScoutDauntless|.-2 on its station at (-140, 675, -70), next to the player's ScoutDauntless.
  The player's own path then differs, and the scripted release, which was tuned on a stacked spawn, misses
  by 1.5 m.
- This is a timing miss in scripted player input, not a mechanism failure. The order file needs re-tuning
  (cc9-ships37's harness lane).
- lua42's `l42_u1_p5.txt` still completes with placement ON: 692.95 s, section 5.

**USN01 with s36's w5 order file (the w6 row): no reach on current main, before or after placement.** Line 2
(`target ConTBD1 -> Convoy4`) finds no ConTBD1 at frame 3000, the same stall. The Nell rows of s36_u1w6.log
are reproduced only with lua42's p5 order file.

**JM08 3200/3000, the intended control, is not a control.**
- Nine pass-C placements, all of one-plane Mavis/Movie squadrons (`members=0`) except Gekko 01's wing.
- Deaths 22 -> 20: Ki-43 Oscar 01 and Gekko 01|.-2 survive in ON.
- Shots 2779 -> 1592.