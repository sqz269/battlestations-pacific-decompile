# CC13 bunkerboat: why a JM08 bunker now kills torpedo boats

Packet `cc13-bunkerboat` (read-only, cc13-seg67n, 2026-10-19). The questions came from
docs/CC13_JM08_END.md:
1. Does the image's land-fort gun (bunker kind 1Bh, category 1) engage torpedo boats at about
   500 m? At what rate, and through which filter?
2. Which switch flip made "Medium Bunker, Concrete 05" start killing boats in our host?

**Answer.** Nothing in the gun filter changed. Bunker 05 now lives long enough to meet the boats.
The cause is `kInferiorFailureArmBound` ON (`5c5d57b05`, 2026-10-17 12:57, GUNNERY_OPEN_ITEMS
272.4). Read from the bytes, the image does the same, so this doc proposes no fix.

## 1. The log trail

Scripts are in the packet tree's `local\`: `bb_scan.py` lists bunker kills per log, and
`bb_b05.py` prints bunker 05's unit row. About 370 JM08 logs under `cc9-*\local` were scanned
in mtime order.

**Until the 10-17 14:47 logs, every completed run loses bunker 05 early.**
- **p7a1_j8 and p8c2off_j8:** Japanese Patrolboat 01 kills it at 61.05 s. It fires 422 to 430 shots.
- **l62\*, l63rt, l64\*, l65rt and g50:** Japanese AA truck 06 kills it at 36.05 s, after 239 shots.
- Bunker 05 is dead before any Gyoraitei is built.

**From g51_d0_j8 (10-17 15:04) on, bunker 05 is never killed.**
- Every row shows `sunk_at -1` and 3100 to 5200 shots.
- In `p24s3_j8_def` it took 246 damage and ends at health 224 of 300.

**The pair is `g51_i0_j8` / `g51_i1_j8`.** These are cc9-gunnery36's OFF/ON logs for
`kInferiorFailureArmBound` at `376d089f6`.
- OFF: bunker 05 dies at 36.05 s.
- ON: it survives, fires 4613 shots and kills Patrolboat 01.
- The first Gyoraitei kills come after the flip, in `g51_a2_j8` and `g51_d0_j8s` (10-17 16:02,
  at 558 m and 508 m).

**Mechanism.**
- Headquarter 01 adopts bunker 05 into its garrison:
  - The HQ is party 1. `prcpijn_08_defend_guadalcanal.scn` authors it InferiorRange 100000.
  - Bunker 05 carries the `CommandBuildingInferior` bag. It authors no `Party`, so it keeps
    Common's default, Allied 0 (docs/SENTITY_INIT_ATTACH_ORDER.md 23.3).
  - The log reports `members=349` for the HQ.
- With the switch ON, `00958A30` breaks the bunker at health 0 instead of destroying it.
- `0095DA00` then refuses damage while the 20 s window lasts.
- The HQ's repair heals the bunker back to full.
- The p24s3 summary line reads `first_breaks=28 rearms=785 refusals=7753 restores=9`.

**The other gunnery flips in the window, and why they are not the cause.**
- **wire1's `kLineOfFireLandFromMuzzleBound` (`e96489cd3`)** is ON from 10-19 00:46. That is after
  the first Gyoraitei kills, so it is not the cause.
- **No effect on a fort's death or its target filter:**
  - `5eecdcfd6`, `843efc336`, `fb31ab760`, `e9a17b4db` and `cf30bd420`;
  - the 272.6-272.9 flips;
  - the 266.4 draw.
- **`kGarrisonMinLevelBound` (`ca0bb132b`, 10-17 18:17)** comes after the change.
- **Already ON in p7a1:** `kCommandBuildingGarrisonBound` (`2d2a83dc3`, 10-06) and the garrison
  revive (`38d3a41f2`, 10-06).

## 2. The image, from disk bytes

### 2.1 Adoption: no party test

`006F5CC0` pass 1 (006F5D00..006F5EB2) adopts an entity on four tests only:
- not the building itself (006F5D03);
- `IsKindOf` 1Bh, 45h or 46h (006F5D10, 006F5D1F, 006F5D2E);
- squared distance not above `FILD(R*R)`, with R = building+7C8h (006F5DC9);
- entity+724h non-zero (006F5DCF).

It then stores the building in entity+71Ch (006F5E9B). Nothing reads +54h. An Allied bunker
joins the Japanese HQ's garrison in the image too.

### 2.2 The break at health 0: no party test

`00958A30` reads +71Ch, +720h and +728h. Nothing in 00958A30..00958CBE reads +54h.

### 2.3 The repair: no party test

`006F47F0` (the per-unit heal) has no +54h read in its body. It has two callers that loop over
garrison records. The record array is building+778h, its count is +77Ch, and records are 64h
bytes apart.

**`006F4D10`, the D3h capture handler.**
- 006F502D..006F5073: for every record whose +14h entity is non-null, it calls
  `006F47F0(entity, 1.0, 1.0)` (006F5062).
- That arm is reached when +7B0h is 2, or when the +7B0h / +7B4h party compare at
  006F4FC2..006F4FE3 does not clear `BL`. Both of those are about the building's own state.
- No per-record party test.

**`006F7360`, the fixed-step repair.** In this function `EDI` = building+310h.
- 006F7432 gates on building+378h.
- 006F7480..006F7555 visits every record whose +14h entity is non-null (006F748E).
- It picks a rate by kind:
  - class+184h by default;
  - class+188h when the member answers 45h, through `IsKindOf` or, for a 1Bh fort, `vtable[204h]`;
  - class+18Ch for 46h, by the same two tests.
- It calls `006F47F0` at 006F7542.
- No per-record party test.

So a broken Allied garrison member is healed by the Japanese HQ, as in the host.

### 2.4 The gun's target filter

- **The gun.** Bunker 05 has one gun: platform 1, category 1 (AAMACHINEGUN), with a category
  range of 960 m in the host's gun table.
- **The ranks.** The category-1 preference list `00E0944C` is built into `00E19BF8` by `00727BD0`.
  Its order:
  - the air classes 17h, 11h, 12h, 15h, 16h, 10h, 13h;
  - then 0Eh;
  - then 0Ch, 0Bh, 08h and 41h.
- **The boats are class 0Eh.**
  - Gyoraitei #Y5: `class_id=14`, `type_id=77`, party 1, built by Shipyard 01's `TBoatGen`
    (`p24s3_j8_def` lines 690587..694965).
  - Japanese Patrolboat 01: `type_id=77`, kind 14.
- **The filter, `00863990`**, applies these tests in order:
  1. the liveness bytes (`00862820`);
  2. a non-zero rank;
  3. the class arms (00862869..008628C5);
  4. the category mask (`008633D0`);
  5. distance below the category range at owner+430h+cat\*4.
- **Verdict.** A torpedo boat at 500 m, and at 750 to 820 m in some logs, is a legitimate
  category-1 target whenever no plane is in range.
- **Rate** (`p24s3_j8_def`): 253 target assignments and 3642 shots, with the first shot at 4.45 s.
  Each boat took 80 to 90 hits before dying. Gyoraitei health is 206; Patrolboat health is 349.
- **Trajectory dependence.** With `BSP_GUNNERY_RNG_STREAMS=1` (`p24s3_j8`), only Patrolboat 01
  dies to the bunker (319 s, 372 m). Which Gyoraitei die depends on the boats' paths.

## 3. Status

- Read-only: no code change.
- Verdict: the host matches the image on the adoption, the break and the repair. A Japanese HQ
  keeping an Allied bunker alive comes from the installation's authoring.
- Diagnostic, if one is wanted: a JM08 run with `kInferiorFailureArmBound` OFF should restore
  p7a1's early bunker death.
