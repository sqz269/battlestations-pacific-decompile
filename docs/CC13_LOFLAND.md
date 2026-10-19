# cc13-lofland: is kLineOfFireLandFromMuzzleBound faithful for every gun?

Packet cc13-lofland, a read-only review. The switch is `kLineOfFireLandFromMuzzleBound`
(`include/bsp/cc13_tier2_switches.hpp`): added OFF by cc13-tier2 in 9e4659617 and flipped ON in
e96489cd3. rev2 ranked it as the top suspect for LOMP10 and BSM01 no longer ending.

**Verdict: no host defect found. Keep it ON.**
- The image starts the land segment at the raw gun position for every gun that has the
  line-of-fire predicate.
- The image caches the answer for the gun's life, as the host does.
- The host's landscape query does not hit more often than 0098ADD0.
- In LOMP10 the switch changed nothing. In BSM01 the extra land blocks follow from the image's
  own geometry.

Names are hypotheses, not recovered symbols. All bytes were read from the disk PE.

## 1. The cache: one record per (gun, target), never re-tested

- **0072F6E0 is the memo** (this = gun+424h, reached through the gun vtable[1D4h] thunk 00730A20).
  It does a linear search over 0Ch records `{targetKey, clear byte, float}` in the vector at
  decision+0Ch/+10h (gun+430h/434h).
- **A hit returns the stored byte** at 0072F821 (`MOV AL,[EAX+4]`). It tests nothing.
- **A miss** calls 00729670 once, which SETEs the predicate's answer (007296EA..007296ED). It then
  draws `U(0.8, 1.2) x [GlobalConfig+8Ch]` (SafeToFireCacheTimeOut, 0072F743..0072F77E) into the
  record's float and appends the record.
- **Nothing reads the float, and nothing erases a record.** A Capstone scan of .text for disp32
  430h/434h/438h and 424h/428h/42Ch operands finds only:
  - in the gun constructor: 0072E59D, 0072E5A3, 0072E5AD, 0072E5B6, 0072E5BC, 0072E5C2;
  - the setup LEA at 0072E702.

  Besides those, the thunk 00730A20 and the destructor 00729600 (which frees the vector,
  00729641..00729649) are the only paths. This agrees with SHIP_AI_OPEN_ITEMS 110.1 and CC13_GPR_40.
- **The key is the target argument only.** Positions are not part of it, and nothing invalidates a
  record when the gun or the target moves or the target dies. The record lasts until the gun
  object is destroyed.
- **Land and friendly blocks share the record.** Both are answers of the same predicate 0072CDD0.
  The land query's `JNE 0072CEE5` (0072CE98) is the same `MOV AL,1` exit as the friendly-unit
  result.
- **The host matches:** `line_of_fire_cache` is keyed by (gun, target index) and kept for the
  mission (`src/game_hosts_gunnery.cpp`, `line_of_fire_refuses`).

## 2. The land segment starts at the raw muzzle, for every gun with the predicate

- **The predicate exists only for some weapon kinds.** 00729560 installs it (vtable 00CFDC04,
  slot 1 = 0072CDD0, 007295CC) for weapon kinds 1, 5 and 6 only (00729588..00729595). Nothing tests
  for a building, land unit or ship. Kinds 2, 3 and 4 have none, and 0072F6E0 answers 1 for them.
- **What 0072CDD0 does with its arguments:**
  - `0072CDF1 MOV EBX,[ESP+28h]` is arg1, the gun position.
  - `0072CE8E PUSH EBX` makes it the start of 0098ADD0 (`0072CE91`).
  - The end is the target's world position, raised by 5.0 and floored at 5.0f (00CE3850;
    0072CE5E..0072CE76, FCOMI / JBE).
- **The +5.0 on the start** (qword 00D7A370, 0072CE42..0072CE52) goes only to the local copy that
  starts the unit walk 0098B130 (0072CECE).
- **The host matches when ON:** `line_of_fire_blocked_0072cdd0` passes `muzzle` to
  `landscape_segment_hit`.

## 3. The muzzle point is gun+FCh, the gun entity's world translation

- **Where the image's point comes from.** 00729670 refreshes the gun with 00414DB0 when gun+C8h is
  0 (00729685). It copies gun+FCh..104h (0072968A..007296B5) and passes the copy's address as arg1
  (007296E3).
  - Here the gun is `[decision+4]`: the gun constructor stores `MOV [ESI+428h],ESI` (bytes
    `89 B6 28 04 00 00`, 0072E5AD).
- **What the host passes** (`gun_muzzle_point`) is the unit origin plus R times the mount offset.
  - For ships and planes, the mount is the 0095F500 slot origin.
  - For land classes under `kLandPlatformAttachmentBound`, it is the slot origin, or the unit
    origin when the platform has no slot (007F7110's identity frame).
- **No other raise.** The image path has no building-height term and no +5.
- **Open item (SHIP_PLATFORM_ATTACHMENT item 5):** the store that puts a TURNING gun's entity at its
  platform is unread. 0072E99C / 0072E9A1 hand the 0072DD20 frame to the model root gun+3BCh, not
  to the gun's local matrix gun+74h.
- **Indirect evidence that gun+FCh is the mount, not the unit centre:** CanFire refuses kinds 1..6
  whose gun+FCh y is below 1.0 (00427EB0, docs/GUN_SHOT_CADENCE.md). A ship gun at the ship's
  centre could then never fire.

## 4. The host's landscape query against 0098ADD0

0072CE91 calls `0098ADD0(from, to, exclude = 0, record = predicate+4, kind = 44h)`. 0098ADD0 walks
the 96h x 96h grid cells under the segment's box, then the overflow list at +80h.

| Item | Image | Host | Direction |
|---|---|---|---|
| Kind filter | IsKindOf(44h) on `[obj+4Ch]` (0098AEC7..0098AEDA, 0098B00E..0098B021); 44h is Landscape (docs/ENTITY_CLASS_IDS.md) | Landscape entries only | match |
| Objects without an entity | `[obj+4Ch] == 0` skips the kind test and is traced (0098AECB / 0098B012 JE) | none | host can only hit less |
| Child nodes | skipped when the kind is nonzero (0098ACC0) | not walked | match |
| Exclusion | arg3, pushed as 0 (0072CE87) | none | match |
| Record | the output: hit point at +8..+10 (0098AC8B..0098ACAE), entity through 00470370 (0098ACB4) | `hit_point`, `landscape_index` | match |
| Nearest hit | the working end shortened to each hit (0098AF95..0098AFB5, 0098B0DC..0098B100, 0098AD21..0098AD41), then "any hit" (0098B11C) | smallest fraction over the entries | same boolean |
| Prefilter | world AABB +13Ch..+150h against the segment's box (0098AEEA..0098AF60), then 0085CAD0 (0098AF6C) | none; the quadtree clips to the ray length (00AE9E02) and the node y range | equivalent where the bounds agree |
| Terrain trace | 00ADA240, then 00AEA2B0, 00AE9D80, 00AE9BD0, 00ADF1B0 (32 x 32 cells of 9.375, [00D5D410]) and 00ADEB80 | cc9's byte-level reconstruction of the same chain (`src/game_hosts_scene_contents.cpp`, `kTerrainSegmentQuadtreeBound`) | match |
| End handling | the DDA tests the last cell, then stops at i == i1 / j == j1 (00ADF553..00ADF55D) | same | match |
| y floor | only the target end, at 5.0 (0072CE62) | `std::max(5.0f, y + 5)` | match (differs only for a NaN y) |

- **00ADEB80 is a LINE test.** I re-read its bytes (00ADEB80..00ADED40): sign tests on the
  scalar triple products, then barycentrics, and no clip to [p, q]. The host's
  `line_quad_00adeb80` says and does the same.
- **Consequence:** a cell the DDA walks can report a hit on the line before the segment's start.
  In the muzzle's own cell, the back-extension of a rising segment descends and can pierce a
  shallow seabed. Starting 5 m lower (ON) shortens that back-extension. If this drives the BSM01
  growth, it is the image's own behaviour. This is not measured.

## 5. Log readings (cc9-lua52 local\, read with Select-String)

| Log | Build | Switch | LOF queries / blocked / refusals | Land LOF calls / hits |
|---|---|---|---|---|
| l69r_w_l10_main_ref (LOMP10) | 4b74dd6b4 | absent (OFF) | 1105 / 121 / 894 | 1105 / 3 |
| l70r_n_w_l10_main_ref (LOMP10) | c95d107da | ON | 1164 / 461 / 5011 | 1164 / 0 |
| l69r_b1d_main_ref (BSM01) | ec0df436c | absent (OFF) | 7863 / 2695 / 18982 | 7863 / 8 |
| l70r_n_b1d_main_ref (BSM01) | c95d107da | ON | 25350 / 8245 / 56423 | 25350 / 783 |

- **LOMP10: the switch is inert in the ON run.**
  - No land query from any consumer hit land: pick 34989/0, projectile 478249/0, slot3c walks
    361561/0, leaves 0.
  - Terrain is a height field, and ON's segment lies below OFF's with the same end. So if ON never
    hits, OFF never hits either.
  - All 461 blocks come from the unit walk 0098B130.
  - CB4's cat-6 guns (platforms 4 and 5, local y 15.66) had assigns 0, refusals 0, shots 0 in
    l70r, against 67/66 assigns and 86/76 hits in l69r. They were never offered a target, which is
    not a line-of-fire refusal.
  - The LOMP10 change therefore comes from another commit between 4b74dd6b4 and c95d107da, or from
    the divergence it starts. Ashigara and Oyodo engage far less in l70r.
- **BSM01 (Pearl Harbor): the land blocks are real.**
  - Land LOF hits went from 8 to 783, over a run about 2.4x longer (pick 40000 vs 16393).
  - The log has no land-mounted guns ("land mounts slot=0 identity=0"), so the blocks come from
    ship and plane guns.
  - The moored ships gain most (gunrow refusals, all reasons): Raleigh cat1 1081 -> 11460,
    Nevada cat6 2077 -> 9332, Medusa cat1 3137 -> 10445.
  - One block refuses that (gun, target) pair for the gun's life, as in the image.

## 6. Open items

1. **SHIP_PLATFORM_ATTACHMENT item 5:** the store that puts a turning gun's entity (gun+74h, and so
   gun+FCh) at its platform is unread. The host takes gun+FCh as the slot origin carried by the
   unit pose.
2. **The 0085CAD0 prefilter's tolerance** (segment vs the entity's AABB, 0098AF6C / 0098B0B3) was
   not re-read. A prefilter can only remove image hits, so the host could at most hit more where a
   landscape's AABB and its quadtree bounds disagree.
3. **Whether the Landscape shape slot 0087FF80 reads record+54h** (`[gun+3Ch]`, stored at 007295C9)
   was not traced. Neither 0098ADD0 nor 0098AC20 reads it. The terrain slot 00ADA240 takes
   only (from, to, out).
4. **The vertical case** 00AECC40 (|dx|, |dz| < 0.001) is not reached by line-of-fire segments and
   was not reviewed here.
