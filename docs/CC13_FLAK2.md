# A category 6 flak gun's second shell in the unit's category range (packet cc13-flak2)

The gunnery host's 00956C20 rebuild binding passed `flak_alternate_range = gun.max_range`.
`category_ranges[6]` (unit+448h) therefore carried only the first ammunition, while the image
also offers a Function 6 gun's second ammunition there. For a dual-purpose mount that is 1500
(the 5" Artillery shell) against the image's 2000 or 2400 (the Flak shell). This packet adds one
OFF switch, `kFlakSecondAmmoRangeCc13Bound`, that fills the image's value at rebuild. It lists
every reader of the value and spells out what a flip moves, per mission.

Status: reconstructed, build-tested with the switch OFF and ON. Not run: OFF is identical by
construction (`if constexpr`), and no game runs were made for this packet.

## 1. The binary: how 00956D63 reads the second ammunition

Disk bytes (`bsp.py disasm-raw`), inside BSP_Unit_RebuildWeaponCategoryIndex (00956C20). ESI is
the category, EDI the unit, EBP the current category node's gun.

```
00956d97  mov ebx, [ebp+3F4h]          ; the gun's device record
00956d9f  call 00731020                ; BSP_Gun_MaxWeaponRange, ECX = record
00956da4  fstp dword [esp+14h]         ; first = the result
00956da8  mov edx, [ebp+3F4h]
00956dae  xorps xmm2, xmm2             ; alternate = 0.0f
00956db1  mov ecx, 6
00956db6  cmp [edx+80h], ecx           ; record+80h: Function == 6 ?
00956dc5  jne 00956ddc
00956dc7  cmp esi, ecx                 ; category == 6 ?
00956dc9  jne 00956ddc
00956dcb  mov eax, [ebx+74h]           ; the ammunition record array
00956dce  mov edx, [eax+7Ch]           ; record 1 (+48h) +34h: its bullet class
00956dd1  movss xmm0, [edx+60h]        ; that class's +60h
00956dd6  movss [esp+18h], xmm0        ; alternate
00956ddc..00956df3  range = max(unit+430h+cat*4, first)    FCOMI / JBE
00956dfc..00956e0d  range = max(range, alternate)          FCOMPI / JBE
```

00731020 is `[rec+78h] == 0 ? 1.0 : [[[rec+74h]+34h]+60h]`, which is ammunition record 0's
bullet class +60h. The alternate is the same field one record further on: `7Ch = 48h + 34h`, and
48h is the record stride (`kDualPurposeSecondAmmoBound`'s `+74h + 1*48h`, 00729BC0 / 00729B90
variant 1). So:

- **Field:** bullet class +60h. This is the derived engagement range, which 006E9941 writes at
  class finalise (`bullet_engagement_range.hpp`: "engagement_range +60h, what 00731020 answers
  with"). It is not the authored `Range`. For Flak and Bullet classes the two round-trip to the
  same value (`Range / V0 * V0`); for Artillery the derived range is `Range` unchanged.
- **Entry:** the device's second ammunition record, that is DeviceClass `Bullet[2]`.
- **Condition:** only a Function 6 (LIGHTARTILLERYFLAK) gun, and only in the category 6 pass. A
  Function 6 gun can sit only in category 6, because the category is the Function. There is no
  count check at 00956DCB. A one-shell Function 6 device would read past its record array (see
  Uncertainties).
- **Arithmetic:** none. Float loads, `FCOMI`/`FCOMPI` compares and `MOVSS` stores only, so the
  24-bit and 53-bit x87 precision models give the same result.

**Host mapping.** `GameGunneryHost::Impl::second_ammo_by_gun[g]` (packet cc9_aa_lead) is keyed by
the gun index `g` into `guns`. `build_guns` fills it from the same flatten chunk's
`bullet2`/`2v0`/... keys (VehicleClass `Platforms[k].Gun[1]` -> DeviceClass `Bullet[2]`), before
`guns.push_back(gun)`. Its `max_range` is `weapon_class_derive_engagement_range(fin2).engagement_range`,
or the authored Range when that is 0. That is 006E9941's +60h for the second class, the value
00956DD1 loads. The rebind uses it: in `device_at`, `slot = owned[index]` is that gun index. The
existing `unit_gun_range_sq_049c` already reads the category 6 second range the same way.

## 2. The change (src/game_hosts_gunnery.cpp, one switch, false)

```cpp
constexpr bool kFlakSecondAmmoRangeCc13Bound = false;
...
out.flak_alternate_range = gun.max_range;
if constexpr (kFlakSecondAmmoRangeCc13Bound) {
    const auto second = owner.second_ammo_by_gun.find(slot);
    if (second != owner.second_ammo_by_gun.end()) {
        out.flak_alternate_range = second->second.max_range;
    }
}
```

- **The rest of the path is unchanged.** `category_engagement_range_00956d63` already applies
  `flak_alternate_range` only for Function 6 in category 6 and takes the max, so a second shell
  shorter than the first changes nothing. That matches the image.
- **No second entry.** The OFF stand-in `gun.max_range` is kept, so such a gun's alternate equals
  its first range.
- **Unaffected outputs.** `artillery_max_range` (+490h) and `any_weapon_max_range` (+494h) read
  only `max_range` (00956E35..00956E59). This matches the image, which does not offer the
  alternate there.
- **OFF identity.** OFF compiles to the same binding as before.

**N1 not folded.** This commit does not touch `unit_rows()`, so the brief's condition for folding
N1 is not met.

## 3. Every reader of category_ranges[6] and flak_alternate_range, all switches OFF

`flak_alternate_range` has one reader, `category_engagement_range_00956d63`
(`gunnery_tables.cpp:190`). `category_ranges` lives in two places: `UnitState::category_ranges`
(gunnery-internal) and `GameGunneryUnitRow::category_ranges` (a copy made at rebuild, served by
`unit_rows()` and `gunnery_unit_row()`). Grep of `src/` and `include/`:

| reader | image site | index | LIVE with all switches OFF? | what a flip moves |
| --- | --- | --- | --- | --- |
| `score_candidate_00863990` binding, `game_hosts_gunnery.cpp` `in.category_range = state_.category_ranges[slot]` | 00863A34 FCOMIP / JC | every category, so 6 included | **LIVE**. The unit gunnery pass, every unit with guns. | A target at `first <= d < second` is now accepted for category 6. For a plane, a dual-purpose gun's slot test then uses `dp_air_ammo`'s `max_range` (`kDualPurposeSecondAmmoBound` and `kAaSlotRangeOriginBound` both ON), so it is assigned and fires the Flak shell. **The main mover.** A ship in that band is accepted, but the slot test (first ammunition) skips it. Because 008657C0 walks to the next candidate, nearer targets are not lost. But a higher-rank band plane now sorts ahead of nearer lower-rank targets. |
| same, `state_.row.best_range` | none (log) | every category | log only | The `range` column of the gunnery unit table rises for mover units. |
| `fly_to_obstacles_009fd63c`, `game_hosts_units.cpp` | 009FD6B1..009FD6D3 | max(c6, c5) then c1 | **LIVE** (`kFlyToObstacleListBound` ON). Torpedo and dive goaway fly-to. | The obstacle radius `max + 100` grows from 1600/1700 to 2100/2500 for an enemy mover unit, and more enemies pass the `r*r > d2` membership test. |
| `goaway_target_extent_007b5be0` / `goaway_target_term`, `game_hosts_units.cpp` | 007B5BE0 at 009C49A5 / 009D0DF2 / 009B56DF | max(c6, c5) then c1 | Log only: the three target-term switches are OFF. The `goaway target term ... aa_max=` exit line changes. | With a target-term switch ON, the standoff takes the image column of CC13_TORP_GOAWAY C.5: Missouri, the US transports, Nevada and Oklahoma, the Marus 1500/1600 -> 2000. That restores BSM01's 35 Oklahoma dives and USN21's Haruna dive to "term moves". |
| `FirepowerBinding::category_max_range`, `game_hosts_ship_ai.cpp`, read by `ship_ai_firepower_rating_0095eb40` | 0095EBCB JA | loop over 0..11, so 6 included (counted as artillery) | **LIVE** (`kShipFirepowerBound` ON; approach avoid weights and 009E5DA0 ring scan) | **Nothing, up to an ulp.** Category 6 is enabled only when `allow_artillery` is set, and then 0095ECB0 does not select the flak alternate. The projectile is the first shell, and 0095ECE9 rejects `range > Bullets.Range`. Range is the first shell's derived range to within the `Range / V0 * V0` round trip, so the newly admitted band is rejected one step later. |
| torpedo bot gates A/B, `game_hosts_gunnery.cpp` (unit+44Ch) | 009001A8.. | `kUnitGunneryTorpedoCategory` = 7 | live, not index 6 | none |
| `torpedo_range_044c`, `game_hosts_ship_ai.cpp` | 009F2C43.. | `kTorpedoStandoffWeaponKind` = 7 | live, not index 6 | none |
| `unit_gun_range_sq_049c` | +49Ch | reads `second_ammo_by_gun` directly, not `category_ranges` | live | none (already uses the second range) |

## 4. Reach of a flip, per mission (from data and the current OFF logs)

**Method** (`local/flak2_reach.py`, re-using CC13_TORP_GOAWAY C.5's Lua parser):
- For every `gunrow` line, the device class (`dev N`) leads to DeviceClass `ArcadeTable[N]`
  (`GameMode = 0`). From that come `Bullet[1]` and `Bullet[2]`.
- Ranges are the logged derived `bullet class N ... range=R`, falling back to the Lua `Range`.
- host c6 = first shells only; image c6 adds Function 6 second shells. The AA reach is
  `max(max(c6, c5), c1)` with the seed 10, as 007B5BE0 and 009FD6B1 compute it.
- "dp guns" are category 6 guns whose second shell outranges the first. "assigns" is their OFF
  `assigns` total. The count does not check `2v0 > 0`, which `dp_air_ammo` also requires before
  a gun's slot test takes the second range. `dp_air_rounds` and the fly-to `rebuilds` come from the mission summaries.
- Logs: each row names two (cc9-physics1 `local\`, except IJN08 from cc9-lua52). The first
  feeds the gunrow columns (units, ranges, dp guns and their assigns): the `_def` logs print no
  `gunrow` lines, so those columns use the plain run of the same p24 set. The second feeds the
  `dp_air_rounds` and fly-to `rebuilds` columns, from that log's mission summaries. The two are
  different runs where they differ, so read the columns as one set each, not as one run.

| mission | units whose c6 moves (side 0 / 1) | c6 host -> image | units whose AA reach moves | dp guns (OFF assigns) | OFF dp_air_rounds | fly-to goaway rebuilds torp / dive | flip moves | logs: gunrow columns; dp_air_rounds and fly-to |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| JM08 | 38 (21 / 17) | 1500 -> 2000 x37, -> 2400 x1 | 38: Missouri, Nevada, USTroopTransport, San Francisco, Helena, Hosho, Fubuki ... | 107 (8304) | 2525 | 23 / 163 | AA gunnery, fly-to; goaway term when flipped | p24_off_j8; p24_off_j8_def |
| USN04 | 16 (16 / 0) | 1500 -> 2000 x16 | 15: Fletcher-, Northampton-, York-class | 83 (471) | 297 | 0 / 0 | AA gunnery only (the US ships' flak against the Japanese strike) | p24_off_u04; p24_off_u04 |
| USN21 | 49 (28 / 21) | 1500 -> 2000 x49 | 45: Haruna, Kongo, Nagato, Myoko, Iowa, NJ, Miami ... | 234 (4639) | 4340 | 0 / 2 | AA gunnery, fly-to (2 dive rebuilds); goaway term (the census set's Haruna dive) when flipped | p24_off_u21f; p24_off_u21f_def |
| USN01 | 6 (5 / 0, plus CB2 on side 2) | 1500 -> 2000 x5; CB2 1500 -> 2400 | 6: Blue, Dunlap, McCall, Ralph, Salt Lake City, CB2 | 28 (59) | 24 | 50 / 105 | AA gunnery (small); fly-to only where CB2 (side 2, the coastal battery) is within 2500 of a US flyer's lead point. The flyers' target Katori is 2000 / 2000, not a mover. | p24_off_u1i; p24_off_u1i |
| BSM01 | 7 (7 / 0) | 1500 -> 2000 x7 | 7: Nevada, Oklahoma, Phoenix, Medusa, Sacramento, Tautog, Whitney | 29 (4944) | 3441 | 191 / 0 | AA gunnery; fly-to (Japanese torpedo flyers near Nevada and Oklahoma); goaway term (35 census Oklahoma dives) when flipped | p24_off_b1; p24_off_b1_def |
| BSM09 | 9 (4 / 5) | 1500 -> 2000 x9 | 8: Cleveland, Fletcher, Jenkins, Shokaku, Zuikaku, Akebono, Asagiri, Ushio | 42 (656) | 766 | 0 / 0 | AA gunnery only | p24_off_b9; p24_off_b9_def |
| JM06 | 14 (12 / 2) | 1500 -> 2000 x14 | 14: Fletcher-class, Narwhal-class, USTroopTransport, Placeholder #3 | 44 (330) | 46 | 0 / 0 | AA gunnery only | p24_off_j6; p24_off_j6_def |
| IJN08 | 61 (40 / 21) | 1500 -> 2000 x60, -> 2400 x1 | 61: Maru, Kongo, Haruna, Hiei, Kirishima, Aoba, Kako, Zuiho, Heavy AA ... | 216 (1154) | 579 | 0 / 72 | AA gunnery, fly-to (dive); goaway term (level bombers on the Marus) when flipped | l70r_w_i08_main_ref; l70r_w_i08_main_ref |
| USNOS | 64 (32 / 32) | 1500 -> 2000 x57, -> 2400 x3, 2300 -> 3150 x4 | 58: TroopTrans, Midway, HQ, Zao, Aki, Ada, Shima, Convoy, Cargo ... | 257 (4420) | 1167 | 0 / 196 | AA gunnery, fly-to (dive; the targets are hangars but movers stand within 2100 of the lead points); goaway term (level bombers on TroopTrans) when flipped | p23h_off_os; p23h_off_os |

**Reading the table.**
- **The flip is a mover in every mission of the set.** Each one has category 6 dual-purpose guns
  on both or one side, enemy planes engaged by them (`dp_air_rounds > 0`), and so new category 6
  plane candidates in the 1500-2000 band.
- **Why the effect spreads.** The extra Flak rounds draw from the process-wide 00BD2F10 stream
  (memory: shared RNG stream), so every later draw shifts. Expect every mission's aggregates to
  move, not only AA kills.
- **What a pair isolates.** The fly-to and goaway-term changes are separable in a pair only by
  their own counters:
  - fly-to: `avoid_ticks` and `side_writes`;
  - goaway term: the `goaway target term` exit line's `aa_max`.
- **The largest direct effects.**
  - JM08: 107 dp guns, the most assigns.
  - USN21 and USNOS: about 250 dp guns each, both sides.
  - BSM01: 4944 assigns on only 29 guns, the Pearl Harbor battleships against the strike.
- **USNOS's 2300 -> 3150.** Four units whose category 6 first shell is a 2300 Artillery class and
  whose second is a 3150 class. Their reach grows by 850, the largest single step.
- **Not computable without runs:** how many band candidates each mission produces. OFF logs never
  score a category 6 plane past the first range, because 00863A34 rejects it. The `cand` trace
  (`aa_trace_unit`) and `AaTargetStats.min_reject_dist` would measure it in an ON run.

## 5. ON data lifetime

- **Producer.** `second_ammo_by_gun[g]` is written once, in `build_guns(first_unit)`, for each
  new gun before `guns.push_back`. It is never erased or rewritten, and its key is the gun's
  permanent index into `guns`. The rebuild that reads it runs later in the same `build_guns` call,
  over `first_unit..count`. So every key it looks up was filled in the same call, or in an earlier
  call for an earlier unit that can no longer be rebuilt.
- **Rebuild timing.** The host runs 00956C20 exactly once per unit, when the unit's guns are
  built (new units only, see the comment above the rebuild loop). The image runs it again:
  - from BSP_Unit_ReadCommonLuaProperties (0095E5F5);
  - on gun activation (00729FBF tail jump);
  - on SetDestroyed and ClearDestroyed failures (0072B631, 007280DF).
- **No refresh after a weapon change.** The host does not refresh `category_ranges` when weapons
  change: a destroyed or repaired gun leaves the host's range as built, where the image drops or
  restores that gun's contribution. That gap already existed for the first-shell ranges, and this
  switch neither adds nor closes it. The alternate itself is static per device class: record 1 at
  `+74h` is a fixed slot, and 0095CF80's flak-alternate selection does not rewrite the array.
- **The copies.** `GameGunneryUnitRow::category_ranges` (`unit_rows()`, `gunnery_unit_row()`) is
  copied from `UnitState` right after the rebuild, so both copies carry the ON value for the
  unit's life. `fly_to_obstacles_009fd63c` and the goaway helper read the copy through
  `unit_rows()`, which rebuilds the vector on each call but copies the same stored ranges.

**The stale key (rev2 nit b, packet cc13-flak2b).** `build_guns` writes
`second_ammo_by_gun[guns.size()]` while it reads Bullet[2]. That happens before the arc guard,
`if (gun.arcs.size() <= 1 && windows > 0 && traverses) continue;`, which skips
`guns.push_back`. A dropped gun's entry therefore stays under the index the NEXT pushed gun
takes. That gun may be on the same unit or on the next one built. If that gun has its own
Bullet[2], the stale entry is overwritten. If not, the stale entry is read as its second shell by:
- `dp_air_ammo` (category 6 against planes: reload, muzzle speed, range and the flak tick);
- `unit_gun_range_sq_049c`;
- with `kFlakSecondAmmoRangeCc13Bound` ON, the rebuild's flak alternate.

**Does any mission hit the guard? Not on this installation's data.**
- **Method** (`local/flak2b_arcs.py`): replay the host's seed `[-kGunArcBoundLimit, +kGunArcBoundLimit]`
  and `gun_add_authored_arc_007f6b10` / `gun_split_insert_arc_007f5a10` over every platform. That
  covers 3675 platforms with a `Gun[1]` in 307 VehicleClass rows, with `DEG(n)` evaluated.
- **Platforms that traverse:** 3474 author a non-zero window, and every one of them ends with at
  least 2 arcs.
- **Platforms left with one arc:** the 201 with a single arc are exactly the 201 whose windows are
  all zero. Those are the BOMBPLATFORM case, kept by `traverses == false`.
- **What would trip the guard:** an authored window that replaces the seed whole, a full
  `[-180, 180]` traverse within the half-degree epsilon, or one the containment search refuses.
  No platform authors either.
- **Result:** zero drops, so zero stale keys, in every mission. `dp_air_ammo` is unaffected today.

**What the commit adds.**
- **Fix:** `kSecondAmmoArcDropKeyCc13Bound` (false) erases the entry the dropped gun wrote. The
  fix belongs to the host's bookkeeping and has no image site. It cannot be unswitched, because
  where the guard does fire it changes `dp_air_ammo`'s answer.
- **Log-only counters:** they always run and are printed as
  `summary mission gunnery arc guard drops= second_keys= stale_inherited= cat6= erase=`:
  - `drops`: guns the guard dropped;
  - `second_keys`: drops that had already written a key;
  - `stale_inherited`: pushed guns that found a key they did not write;
  - `cat6`: the subset of `stale_inherited` in category 6, where `dp_air_ammo` reads the key.
- **Expected values:** all zero on the next pair runs, which would confirm the replay. A non-zero
  value means modded data or a later arc change.

## Uncertainties

- **One-shell Function 6 device.** The image does not check the ammunition count at 00956DCB.
  For a one-shell device (DeviceClass 522 "Napoli Turret" is the only one of 95), it reads record
  1 past the array. The host keeps `gun.max_range` there. No gunrow in the nine logs uses
  device 522.
- **Record order.** Record 1 is taken to be `Bullet[2]` in authored order, as
  kDualPurposeSecondAmmoBound already assumes.
- **Round trip.** The firepower verdict "nothing moves" depends on the `Range / V0 * V0` round
  trip being exact or rounding up. Where it rounds down by an ulp, a zero-width band is admitted.
- **Names.** The reach table's units are taken from gunrow names, and `side` from the gunnery unit
  table. A unit name over 20 characters may have missed its side.
