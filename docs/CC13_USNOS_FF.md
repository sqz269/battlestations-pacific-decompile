# USNOS friendly fire: Midway's gun and TroopTrans6 (cc13-usnosff)

This packet comes from cc9 priority 1(i), which cites SHIP_AI_OPEN_ITEMS 305 (the ships60 table, row `s60_p(j)_c20`):
"Midway's own gun sank TT6 at 2856.96 s (ref)".

The question: how does the image keep an allied ship's guns from hitting a friendly, and does the host lack any part
of it?

Verdict: **no host gap found.** No switch was added. The c20 hit itself cannot be classified, because its logs are
gone (section 1). Ghidra was read only.

## 0. Who is who

- **"Midway" is a ship, not a base.**
  - The scene places it as `entity "Midway" (MotherShipGen)`, template `Japan\SEA\Midway-class`, GuiName "USS Midway".
    That is `us_osumi.scn` in this installation.
  - `us_osumi.lua` puts it in `Mission.EnterpriseGrp`.
  - The host's mount lines give it 18 category-6 mounts and 14 category-1 mounts. They are the
    `gunnery: mount Midway ...` lines in `cc9-ships44\local\s58_off4_os.log`.
- **TT6 is `TroopTrans6`.** It is `(DestroyerGen)`, template `USA\SEA\USTroopTransport`, GuiName "USS Fuller", and is
  one of `Mission.Troops`.
- **Both are Allied.** The only `SetParty` calls in the script act on `Mission.JapHQs`, so neither ship ever
  changes side.
- **Where TT6 is.** From c18c on, the plan parks TT6 at (16000, -9000), beside Enterprise's ENT_AWAY point
  (SHIP_AI_OPEN_ITEMS, row `s60_b_c18c`). Midway starts 500 m from Enterprise.
- **What c20 adds.** Compared with c19 (`s60_os_c20.txt` against `s60_os_c19.txt`), the only added lines are
  `launch Enterprise|Midway 0 810 3 TroopTrans6`, alternating every 750 frames from 34005 to 199755. These are
  fighters flying cover over TT6.
- **So at 2856.96 s**, the Japanese strikes on TT6 arrive over a transport parked next to the carriers. Midway's AA
  is firing at aircraft that are low over a friendly hull.

## 1. The TT6 hit lines: not recoverable

- **The logs are deleted.** `local\s60_p_c20.log` and `s60_pj_c20.log` in `cc9-ships44` no longer exist; only their
  manifests remain.
  - Run: started 2026-10-19 08:39:22Z, exit 0.
  - Exe: `local\s60_post_bin\bsp_game.exe`, sha256 c63ae683..., head 2835f8720.
  - Environment: `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`.
  - Orders: archived `s60_os_c20.txt`, sha256 f9d2cb18....
  - `local\s60_launch_s60_p_c20.txt` holds only aggregate counts.
- **No other log has the event.** I scanned every readable `*os*.log` and `*c20*.log` under `cc9-*\local` for death
  rows with an allied victim and an allied killer (`local\usff_scan.py`). Not one row has Midway as the killer.
  - Five `s60_m2*_os.log` files were still being written and could not be opened (sharing violation).
  - Those runs use c19 orders and 60000 frames, so they have no Midway cover launches.
- **Unknown, therefore:**
  - the shooter's gun row and category;
  - direct hit or blast;
  - Midway's fire-gate target at that moment;
  - the range.
- **Rerun request (for cc9, not run here).**
  - Same exe, orders and environment as above.
  - Add `BSP_AA_TRACE_UNIT=Midway`, which is observation only (`aa_trace_matches`). It prints Midway's `fire gate`
    lines (with the target) and an `aa hit ... on <victim> direct|blast` line for each of its hits, before the
    death row.
  - The death row's `killer_gun`, `killer_cat`, `killer_blast` and `killer_range` then classify the 2856.96 s hit.

## 2. The image's three layers, and the host's

| layer | image | host | same? |
| --- | --- | --- | --- |
| target selection | Gun picks come from the side's published enemy contact list (`[recon+DE8h]`, read by `008053C0`). A unit of the shooter's own party is never a candidate. | `recon_contact_count_008053c0` walks `recon_triples_for(own_side)->lists[1]`, the enemy triple | yes |
| friendly in the line of fire | Gun `vtable[1D4h]` = `00730A20` -> `0072F6E0`, the decision `00729560` installs for weapon kinds 1, 5 and 6 only (GUNNERY_OPEN_ITEMS 164.2). `0072CDD0` raises both ends 5 m (the target end to at least 5 m) and refuses when `0098B130`'s nearest unit on the segment (firer and target excluded) has the firer's `+54h` (E2_ZERO_ORDNANCE, correction of 2026-09-23). Each (gun, target) answer is cached until the gun is destroyed and never re-tested (164.1). | `line_of_fire_refuses` / `line_of_fire_blocked_0072cdd0`, under `kAaLineOfFireBound` (ON): kinds 1, 5 and 6, the same raise, a `row.side` compare, and a (gun, target) cache that is never erased | yes |
| the round and its blast | The projectile sweep excludes only the round's own owner (`0098AEE4`, `projectile+238h`; KILL_CREDIT.md). `0084BAD0` excludes only the source entity (`0084BBCF`), and neither `00826F10` nor `008777D0` reads a party (GUNNERY_OPEN_ITEMS 272.3). | the sweep's `args.exclude_entity = shot.owner_unit`; `apply_impact_blast` skips the shooter alone | yes |

**What this means for Midway.** All 32 of its mounts are category 1 or 6, so the image does test the line of fire
for each of them. That test refuses a shot only when, at the first pick of that target, a friendly box lies on the
straight segment from the muzzle to the target. The image still lets a friendly be hit in three cases:
- **The friendly moves into the line later.** The verdict is cached per (gun, target) and never re-tested.
- **The friendly is off the segment, but the round reaches it anyway.** The segment ends at the target. A round that
  misses flies on past it, and a flak burst or impact near a low aircraft reaches anything within its radius. The
  segment is also aimed at the target's position, not along the round's path.
- **The weapon is artillery (kinds 2-4).** It has no line-of-fire test at all (GUNNERY 63 and 164.2). This does not
  apply to Midway.

## 3. Friendly kills in the surviving logs

These are the death rows found by the scan, with Cargo victims left out (their party was not checked).
`killer_range` is the distance from victim to killer.

| logs | victim | killer, cat, blast, range |
| --- | --- | --- |
| 32 logs (g35_*, s42_*, s43_*, l53_A1 and others) | Enterprise | DM1, cat 3, blast in 20 and direct in 12, 41-89 m |
| l53_K4_osx | TroopTrans6 | Gear16, cat 1, blast, 690 m |
| l65rt_os*_ref, l66r_os_*_ref | TroopTrans6 | TroopTrans3, cat 1, direct, 948-1123 m |
| l70r_vos17_*_ref | TroopTrans1 / TroopTrans5 | Gear1, cat 1, blast, 927 m / 339 m |

The scan output is in `local\usff_scan_out.txt` (up to four rows per log; Enterprise dies at most once per log).

- **DM1 and Enterprise.** DM1 is the Des Moines heavy cruiser (cat 3), sitting 46-89 m from Enterprise. That is the
  artillery case: no line-of-fire test exists.
- **Totals.** 62 logs have at least one such row. Cargo victims also die to TroopTrans3/5/6 and Gear14; those rows
  are left out of the table above.
- **The cat-1 kills.** In each, the killer sat hundreds of metres from the victim. In `l65rt`, the nearest enemy ship
  (`nearest_ship=unit #5.7`) was 125 m from TT6, and TT3 was 948 m away. The shots' targets are not logged, so the
  bystander reading (fire at an enemy close to the victim) is an inference, not a measured fact.
- **The c20 Midway kill.** It plausibly belongs to the same class, but section 1's rerun is what would show it.

## 4. Verdict

- **Not a host gap.** All three layers match the image, as far as the image has been read.
- **What happened in c20 is the image's rule exposed by the plan.** TT6 is parked beside the carriers, and the
  Japanese strikes on it draw the carriers' AA through and around it.
- **The fix belongs to the plan, not to gunnery.**
  - Park TT6 well clear of Enterprise/Midway, out to beyond their AA engagement ranges.
  - Or put the cover flights elsewhere.
- **The ruling is the same as GUNNERY 63 and 272.3** (BSM01, HenryPT and Sacramento).

## Uncertainty

- **The c20 event is unclassified.** The log is deleted. Section 1's rerun would name the gun, the category, direct
  or blast, and whether TT6 was the target. Section 2 says it cannot have been the target.
- **Target selection was checked only on the gun contact path.**
  - The host's contacts come from the enemy triple.
  - Not re-read: Lua `attack` orders and the director's commanded target, which could in principle name a
    friendly. The c20 orders add only `launch` lines, so neither applies to Midway here.
- **The image's own contact list was taken from the existing recon bindings** (`kReconTeamListsBound`), not re-read
  from the image in this packet.
- **The `0072CDD0` geometry and the cache rule come from the cited packets** (E2_ZERO_ORDNANCE, GUNNERY 164). I did
  not re-read them from bytes here.
