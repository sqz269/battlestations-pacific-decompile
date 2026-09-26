# E2 torpedo releases: 5 at the DM base, 6 at 88e1a3062 (packet cc9_e2_release_bisect)

2026-09-25. Every run below is E2 (USN04, `--frames 9200 --mission-frames 9000`, idle player), with
`BSP_GUNNERY_RNG_STREAMS=1`. Each binary is a fresh detached build of the named commit, copied to
`local\bisect_<sha7>` (the module directory line was checked in every log). Runs are deterministic
here: a fresh build of a commit reproduced every row of an older binary of the same tree.

## 1. The premise was wrong: DM0 released 5 torpedoes, not 0

- `docs/DOGFIGHT_MANEUVER_BODIES.md` section 4 has a row "releases 0 / 0" for DM0, DM1 and DM2. That
  row is the **dive-bomb** release count and the **player-role** release count, both 0 in every run.
- The torpedo task's own line in `local\DM0_9000.log` reads `aircraft=16 releases=5`. DM2 (bodies on)
  released 4. Only DM1 (all on, with the planner gate) released 0, because its Kates never reached the
  fleet.
- So the question became: what moved the torpedo release count from 5 (DM base) to 4 (the
  dogfight-bodies landing) and then to 6 (`88e1a3062`)?

## 2. The runs

| commit | what it is | torpedo releases | Kate deaths | hit records | Lexington moved |
| --- | --- | --- | --- | --- | --- |
| `8fba85d95` | main at the DM runs (reflog: fast-forward at 2026-09-23 22:22) | 5 | 16 | 553 | 6517.06 m |
| (old `local\dm0`) | DM0, that base plus the bodies with switches OFF | 5 | 16 | 553 | 6517.06 m |
| `b515bc059` | first main commit containing `1b5ce4b90` (the bodies landing, switches ON) | 4 | 16 | 571 | 6184.98 m |
| `6e48234f6` | first parent of `9ee202de7` | 4 | 16 | 571 | 6184.98 m |
| `9ee202de7` | merge of main into agent/cc10, bringing three cc9 commits | **6** | 16 | 592 | 6750.78 m |
| `9ee202de7`, `kFighterFriendlyInLineBound = false` only | the one call-site switch off | **4** | 16 | 571 | 6184.98 m |
| `88e1a3062` | the bad endpoint | 6 | 16 | 592 | 6750.78 m |

- `8fba85d95` equals the old DM0 binary on every row. The fresh build confirms the old one.
- `6e48234f6` equals `b515bc059` on every row. `9ee202de7` equals `88e1a3062` on every row.
- So on main's first-parent line the whole change is the merge `9ee202de7`. Only it and
  `f57e363c5` (a HUD screen landing) touch the game hosts in the range, and `f57e363c5` comes after
  it, where every row already equals the endpoint.
- `9ee202de7` brings `dd2b462e2` (the kill-credit damage gate), `2c2496df2` (the HUD root screen) and
  `6467f73b1` (the fighter friendly-in-line hold's call site). Turning off only the last one
  reproduces `6e48234f6` exactly on every row. So the kill-credit gate
  and the HUD screen are neutral on this mission, and **`6467f73b1` is the whole move from 4 to 6**.
- The kill-credit-off variant could not run. The RDP session disconnected at about 16:09, and every
  later launch failed in FMOD at `sound/gui/error.fsb`. It is not needed: the friendly-in-line-off
  variant already equals the parent.

## 3. The mechanism

`6467f73b1` applies `kFighterFriendlyInLineBound`: `007B96D0`'s answer feeds the fighter gun's
`finder_busy` at `009FC7C0` (docs/FIGHTER_GUN_LEAD.md section 5). The switch had been ON since
`b95055af8` with no call site, so its landing was the first run where it acted.

- **The fighters fire on more ticks, not fewer:** 216 fire ticks and 11 bursts without the hold, 366
  and 17 with it. Fighter rounds stay 0 in this host (the gunFire consumer is not established), so no
  Kate is shot down by a fighter either way.
- **The fighters' published gunfire reaches the Kates' reaction to fire** (`009A17D0`, packet
  cc9_gunfire_avoidance). Kate #6.1|.-2 shows 572 detections, 22 flagged, 77 repairs without the hold,
  and 793 / 36 / 31 plus 30 exempt with it.
- **Without the hold that Kate evades into the sea**, water contact at |v| = 90.5 m/s, dead at
  164.26 s, never released. With the hold it lives to 189.61 s, runs an attackrun and releases. The
  rest of squadron #6.1 dies 184-199 s instead of 164-207 s, and #8.1|.-4 also releases before its
  death at 216.71 s.
- Releases by Kate, without and with the hold: #2.1|.-4, #4.1|.-4, #6.1|.-3 and #8.1|.-2 release in
  both; #6.1|.-2 and #8.1|.-4 release only with it.

**Verdict.** No host bug. The move is the landed friendly-in-line hold changing the fighters' fire
cadence, and through the Kates' gunfire avoidance, which of them survive long enough to release. Kate
deaths stay 16 of 16. The earlier move from 5 to 4 is the dogfight-bodies landing itself (DM0 to DM2,
measured in that packet). No code change.

## 4. Side question: the list 007F0280 walks in mode 0

- The torpedo attackrun calls `007F0280` with `ECX = [approach+0Ch]` (`009D0854`) and mode 0.
- Mode 0 walks `this+3D0h` for `this+3CCh` entries, skipping arg0, the unit itself (`007F0481`, and
  docs/BOT_PROBE_007F0280.md section 0.2).
- A store census of `+3CCh` (`tools/store_census.py 0x3cc`) finds its writers only on the plane
  squadron:
  - `007F2D9D` in `BSP_PlaneSquadronTickableEntity_Construct`;
  - `ADD` at `007F39ED` in `BSP_Squadron_RemovePlane`;
  - `ADD` at `007F4B60` and `007F4FCE` in `BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes`.
- So `approach+0Ch` is the plane squadron (as docs/TORPEDO_RUN_IN_PATH.md already says of `ctl`),
  and mode 0 probes **the unit's own live squadron mates**, with no kind filter. Dead members leave
  the array through `007F3970`.
- The host already keeps this list: `plane_squadron_registry()` `member_units`, which removes a
  member at death (`007BCAA0 -> 007F3970`, packet cc9_val_squadron_registry).
- The call shape is the dive-bomb one: extents 80/60/120, zero weights, and the product
  `-out3.x * out2.y * out2.z`, read at `009D085C`-`009D0868` with the stack 8 bytes deeper.
- **Not bound in this packet.** `src/game_hosts_units.cpp` is leased by agent/cc9-platform2
  (cc9_gunner_role_take) until 2026-09-26T06:05Z. The binding is a squadron-member candidate list for
  `nf_probe_007f0280` passed at the attackrun heading call site, under `kNearFieldProbeBound`, and
  measured as a pair.

## Correction from docs/FIGHTER_GUNFIRE_CONSUMER.md (packet cc9_fighter_gunfire_consumer)

- **Was:** "Fighter rounds stay 0 in this host (the gunFire consumer is not established)" (section 3).
  **Is:** the consumer is `007CE9F4` (`docs/PLANE_GUN_PASS.md` section 2), bound under
  `kPlaneGunfireHooked`. On main `27f082d38`, E2 fires 940 fighter rounds with 86 hits, and the
  fighters kill five aircraft: three D3A Vals of #1.1 and two of the movieval flight. No
  fighter hits a Kate.
  **Evidence:** `local/fgB_e2.log` (`summary mission gunnery plane guns trigger_ticks=1908
  rounds=940`; five `death row` lines with `killer_cat=0`), and `THR1_9000.log` in the
  dogfight-engaged tree (1946 category-0 rounds, 96 hits).