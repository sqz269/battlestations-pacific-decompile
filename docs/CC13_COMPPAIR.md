# CC13 compensating-flip sweep over cc9's pair logs (packet `cc13-comppair`)

This is cc9 priority 3. The packet looks for cc9 ON/OFF pairs whose aggregate death count matches, or nearly
matches, while individual units' outcomes flip in opposite directions. Read-only. No game runs.

## 1. Method

- **Logs.** Every `*.log` in `J:\PROG\battlestations-pacific-decompile-cc9-*\local\` modified within 48 h of
  2026-10-19 13:30Z: 1,243 logs. The Grep tool skips gitignored logs, so the scan is a Python reader
  (`local\cp_scan.py` in this packet's tree; not committed).
- **Pairing.** Two ways:
  - (a) Names that differ only by `off` <-> `on`, in one directory.
  - (b) Two logs whose manifests (`<log>.manifest.json`) have identical `game_args` **and** `env`, but a different
    exe, with the same run prefix (the first `_` token).
  - Pairs across environments are dropped: reference = `BSP_GUNNERY_RNG_STREAMS=1`, default = no stream variable,
    jitter = `BSP_FRAME_JITTER`.
  - The scan formed 2,175 candidate pairs. 2,051 had a death table on both sides; logs still being written (the
    `s60_m2*` set) are locked and skipped.
- **Per-entity table.** Each log's last `unit side guns cats range nearest shots hits dealt taken health sunk_at
  killed_by` block (`BSP_DEATH_TABLE=1`). For each pair the scan compares:
  - the outcome per unit (`sunk_at >= 0`, or alive);
  - the killer, where the unit died on both sides;
  - `sunk_at` shifts, after subtracting the modal offset.

  The modal offset is 0.00 s on every pair below, so no row is a relabel. Duplicate names get `#2`, `#3` suffixes in
  table order.
- **Kills.** The table's `killed_by` is the per-entity kill record. A changed killer is counted only where the unit
  died on both sides.
- **Filter for this report.** Flips must go both ways. The total deaths must differ by at most 1, or the non-plane
  flips must net to 0 or +-1. Planes are names with `_sqn`, `|`, `plane #` or `#<digit>`.
- **Out of scope.**
  - Default-env pairs move through the process-wide stream `00BD2F10` (cc9 records these as RNG-coupled). They are
    listed separately in section 3.
  - Pairs whose two exes come from different commits (`*_rt`, `*_main`, `l66_g_off` against `l66_m2_off`) are not
    controlled pairs and are excluded.

## 2. Ranked table (controlled pairs, same env)

Rank order:
1. The aggregate hides the flip: deaths equal, and the pair's doc names none of the flipped rows.
2. Then by the number of flipped units.

"cc13" means a switch in the flip that cc13 owns or authored.

| # | pair (tree: OFF -> ON) | switch(es) flipped | deaths | OFF-only deaths | ON-only deaths | net 0? | other per-unit moves | doc names the rows? | cc13 involvement |
|---|---|---|---|---|---|---|---|---|---|
| 1 | physics1: `p23i_off_u1i` -> `p23i_seam_u1i` (USN01 `usn01_p8_u1_i`, ref) | `kFollowReleaseSeamsBound` | 37 -> 37 | ConSBD2 (side 0) | Convoy2 (side 1) | **yes** (side 0: 28 -> 27, side 1: 9 -> 10) | 24 of 36 shared deaths move > 1 s; 10 killers change | No. OCEAN_WAVE_FIELD 98 gives release counts only ("torpedo-task releases 4/9 -> 3/8, dive-bomb releases 5 -> 7") | **Direct**: cc13's follow-release seam switch (`include/bsp/follow_release_seams_cc13.hpp`) |
| 2 | lua52: `l66r_b1_g_off_ref` -> `l66r_b1_cst_on_ref` (BSM01 `bsm01_p6_b1_a`, ref) | `kConstExactImageBitsBound`, `kPilotPitchInputScaleImageBound`, `kGuiCameraTargetZImageBound` | 805 -> 805 | Crane 02 02, Crane 02 23, plus 8 Jap plane sections | LST No. 256, LST No. 122, plus 8 Jap plane sections | **yes** (side 0: 6 -> 8, side 2: 31 -> 29) | 508 of 795 shared deaths move; 458 killers change | Yes, as a count: SQUADRON_LAND_TASK 5ig, "deaths 805 = 805, 627 death rows changed", called trajectory chaos | **Direct**: the three cc13-fix67 constant switches. Ulp-level, so the per-unit spread is chaos, as cc9 says |
| 3 | ships44: `s57_off3_j5` -> `s57_gm_on_j5` (JM05, ref) | `kGarrisonMinLevelBound` + `kShipMoveToCaptureRangeBound` | 31 -> 31 | Fubuki-class 01, Japan Troop Transport 01, Zuikaku, Mogami-class 01 (side 1) | RadarStationFortress 01/02, MainFortress element Round Side 01/02 (side 0) | yes in total (side 0: 25 -> 29, side 1: 5 -> 1) | 11 move; 16 killers change | **Yes**, all 8 rows and the mechanism (SHIP_AI_OPEN_ITEMS, s57 pairs) | None (cc9 switches) |
| 4 | physics1: `p20f_off_jm02` -> `p20f_seed_jm02` (JM02, ref) | `kBotTaskCtorSeedsBound` | 39 -> 39 | Betty 05.-II, Nell 01, Nell 03.-III | Nell 02, Nell 03.-II, Nell 04.-II | yes (all side 1, all planes) | 30 of 36 move; 27 killers change | Yes, as "3 rows swapped" (OCEAN_WAVE_FIELD 81) | None. It adds 00BD2F10 draws, so the plane fights are RNG-coupled |
| 5 | physics1: `p23c_ab_u04` -> `p23c_abc_u04` (USN04, ref) | `kSquadronGeneralStepTailBound` | 59 -> 60 | Lexington-class01_sqn03.-III, Yorktown-class01_sqn04 | Lexington-class01_sqn01.-II, Lexington-class01_sqn03.-II, Yorktown-class01_sqn05 (killer `-`, 230.71 s) | net +1 (all side 0 planes) | 17 move; 22 killers change | Only "USN04: MOVED, 22 tail lines, RNG-coupled" (OCEAN_WAVE_FIELD 95) | **Direct**: cc13-steptail. The killer-less Yorktown_sqn05 death is not explained |
| 6 | ships44: `s57_off3_i8` -> `s57_ap_on_i8` (IJN08, ref) | `kAttackMoveApproachImageBound` | 171 -> 172 | US_Airfield_01_sqn16.-III, US_Airfield_01_sqn17.-II | Static Oscar 01, Oil Tank Small 01 06, Maru2 | net +1 (planes out, ground units in) | 33 move; 4 killers change | Not checked row by row | None |
| 7 | gunnery52: `g53_nfoff_j6` -> `g53_nfon_j6` (JM06, ref) | `kNavigatorForceTorpedoBound` | 23 -> 24 | Narwhal-class Submarine 01, PBY Catalina #1.1 | Lexington-class 01_sqn02, its .-II | net 0 by count (2/2) | 9 move; 4 killers change | Partly: the Narwhal survival and the Lexington timing (UNIT_WEAPON_DEVICES, JM06 pair). The PBY is not named | None |
| 8 | ships44: `s56_off_r0` -> `s56_on_r0` (USNOS `s55_os_r0`, ref) | `kAutoTargetOverrideBound` | 171 -> 183 | **Lexington**, Gear13, Gear15 (side 0); Shimotsuke, House Small Wooden Brown 05, White 06 (side 1) | TroopTrans4 (side 0); Ada3, Zao3, Shima1, Cargo1 (side 1) | non-planes 6/5 (net -1); planes account for the +12 | 56 move; 29 killers change | Half: the ON-only Ada3/Zao3/Shima1 and the Shinyo rows. **The OFF-only Lexington, Gear13/15 and Shimotsuke survivals are not named** | None in the flip. The Shinyo `kamikaze_attack` it releases is the boat path, not cc13's plane latch switches |
| 9 | gunnery36: `g51_f0_a4` / `_c1` -> `g51_f1_a4` / `_c1` (JM06 `jm06_p12_j6_a4` / `_c1`, ref) | not documented in main (exes `g51_a0` / `g51_f1`) | 22 -> 19 / 20 | PBY Catalina 01, **Lexington-class 01**, Fletcher-class 05 (+ 2 Lexington plane sections on a4) | PlayerSub 01/02 (a4); Fletcher-class 09, PlayerSub 02 (c1) | no (-3 / -2) | 7 move | Unknown | Unknown: the exes may differ by more than one switch |

Pairs whose per-unit outcomes are identical despite a switch flip, included as controls:
- `p23c_a_u04` -> `p23c_ab_u04` (`kSquadronDogfightLeashBound`): 59 = 59, no flips, no shifts.
- `g54_ccoffs_u4` -> `g54_ccons_u4` (ref): 58 = 58, identical.
- `l66r_u04_m2_off_ref` -> `l66r_u04_id_on_ref`: 53 = 53, identical.

These confirm that the reader reports zero where cc9's pair_diff says gameplay is identical.

## 3. Default-env pairs (RNG-coupled; listed, not ranked)

| pair | switch | deaths | flips OFF-only / ON-only | note |
|---|---|---|---|---|
| lua52 `l66r_b1_g_off_sh` -> `l66r_b1_g_on_sh` (BSM01) | the four gunnery switches of GUNNERY 272.9 | 809 -> 817 | 17 (Crane 02 02, Crane 01 13, LST 157, LST 122, 13 planes) / 23 (Crane 01 10, LST 156, LST 158, **Neosho**, 19 planes) | 750 of 792 shared deaths move: a different battle |
| gunnery52 `g54_ccoffd_u4` -> `g54_ccond_u4` (USN04) | g54 cc switch | 62 -> 56 | 7 / 1, all US plane sections | cc9 records it: "USN04 deaths 62 -> 56", RNG-coupled |
| physics1 `p23j_off_u1i_def` -> `p23j_on_u1i_def` (USN01) | `kPlaneDeviceWalkSharedDrawBound` | 30 -> 33 | Convoy2 / Convoy4 + 3 plane sections | the shared-draw switch moves the stream by design |
| gunnery52 `g54_boffd_u4` -> `g54_hqond_u4`, `g54_ccond_u4` -> `g54_hqond_u4` | g54 b/hq switches | 56 = 56 | 2 / 2 | equal aggregate, default env |

## 4. Findings

1. **One cc13 switch hides a ship-level swap behind an equal aggregate.**
   - Pair: `kFollowReleaseSeamsBound` on USN01, reference env.
   - Effect: ConSBD2 (US) survives and Convoy2 (Japanese) dies. Each side's death count changes by one in opposite
     directions, so the mission total stays 37.
   - cc9's verdict ("Gameplay MOVED", release counts) stands, but the swap is not named. If USN01 is ever a WINS
     row, Convoy2's death is the change that matters.
   - A 1:1 swap is what release timing would produce. The seam arm changes which torpedo/dive releases fire, which
     moves who is hit.
2. **cc13-steptail on USN04 swaps US plane sections and adds a killer-less death.**
   - Pair: `kSquadronGeneralStepTailBound`.
   - Yorktown-class01_sqn05 dies at 230.71 s with `killed_by -`. Every other ON death in the pair has a killer.
   - This is the one row that looks like a crash or retire, not combat. It is worth one look: the tail issues
     0084E595 moveto-self loiter orders, and a loitering squadron at low altitude could meet terrain. That is a
     hypothesis, not read.
3. **cc9's `s56` note on `kAutoTargetOverrideBound` lists half the flip.**
   - OFF-only deaths: Lexington (875.86 s, Shima2), Gear13, Gear15 (Ada3) and Shimotsuke (NH). These four do not
     happen ON, because Ada3, Zao3 and Shima1 now die early.
   - This matters for the USNOS plan: ON, Lexington survives the `s55_os_r0` window.
4. **The other equal-aggregate flips are already described by cc9.**
   - Fully: s57 GM (all 8 rows) and p20f (3 swapped rows).
   - Explained as trajectory chaos: cst (`627 death rows changed`).
   - No new action for them.
5. **No compensating flip is attributable to a cc13 switch that cc9 still holds OFF.** The cc13 switches in the
   flips above (seam, step tail, the fix67 three) are ones cc9 has already flipped ON by mechanism.

## 5. Limits

- 48 h window only. The `s60_p_c20` and `s60_p_c21` logs are deleted, and the `s60_m2*` set was locked, so none of
  them were read.
- Plane classification is by name pattern. A plane named without `_sqn`, `|` or `#` (for example "PBY Catalina 01")
  counts as a non-plane.
- The killer comparison covers only units that died on both sides. A kill credited to a unit that itself flips is not
  separated out.
- `g51_f*`: the switch is not documented in main, so row 9's attribution is open.
