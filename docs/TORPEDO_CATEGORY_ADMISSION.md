# Why category 7 (TORPEDO) takes no assignment

Addresses: `006E9890`, `00855A90`, `0070C0B0`, `008633D0`, `00862820`, `00731020`,
`00956D63`, `00863990`, `00864FE0`, `006E8770`, `008566B0`, `008624C0`, `00861D70`.
Data `00CFA424`, `00CFA428`, `00CEFF98`, `00CFA1A8`, `00CFA210`, `00D0C5C4`, `00CFA56C`,
`00E09D64`, `00E19BF8`.

Report: `reports/cc7_torpedo_category_admission.json`. No header, no source: see
section 8.

The measured gap: in the rebuilt `bsp_game.exe` on the 3200-frame USN02 mission,
category 7 has 71 guns, **0 assignments, 0 shots, 0 `no_window`, 0 `arc_blocked`**,
while categories 2, 3 and 6 take 1494 assignments between them. `no_window` is
`GameGunRow::angle_refusals` and `arc_blocked` is `GameGunRow::arc_blocks`
(`src/game_hosts_gunnery.cpp:1980`-`1989`); both are aim-side counters that only
move after a gun has a target, so their zeros say only that no candidate reached a
torpedo mount. They do not locate the refusal.

**Verdict: the zero is a reconstruction gap, not faithful behaviour.** The authored
data admits destroyers for category 7, and the native engagement range for a ship
torpedo in this installation is **1852 m**. The host computes **10 m**, because it
takes the mount's range from the Lua key `Range`, which no torpedo bullet class
carries, and the native never reads `Range` for a torpedo.

## 1. What the authored data says

`docs/GUNNERY_TABLES.md` section 1 transcribed the twelve preference rows at
`00E092C8`. Category 7's row is at **`00E09D64`** and holds, best first:

| position | class id | class |
| --- | --- | --- |
| 1 | `09h` | `MMothership` |
| 2 | `0Dh` | `MBattleship` |
| 3 | `0Ah` | `MCruiser` |
| 4 | `07h` | **`MDestroyer`** |
| 5 | `0Ch` | `MLandingShip` |
| 6 | `0Bh` | `MCargo` |
| 7 | `08h` | `MSubmarine` |
| 8 | `41h` | `NavPoint` |

Class ids from `docs/ENTITY_CLASS_IDS.md`. `00727BD0` inverts the row into
`DAT_00E19BF8[classId + 7*61h]`, so `gunnery_rank(7, MDestroyer)` is **4**, not
zero. Category 7 is **not** one of the empty or air-only rows: `docs/GAME_EXECUTABLE.md`
lists categories 1, 5, 8 and 11 as taking zero assignments because their rows at
`00E0944C`, `00E09A5C`, `00E09EE8` and `00E0A374` are air, air, submarine-only and
empty. Category 7 is none of those.

## 2. The two gates that do not refuse

### The category mask, `008633D0`

`__thiscall(gunneryAi)(int category, Entity* target)`, `RET 8`, body
`008633D0`-`00863427`. Coverage: **complete**.

```
008633DF  if (!00862820(this)(category, target))                 return 0
          m = [this + 80h + category*4]
          if ((m & 1) == 0 && target->vtable[5Ch](0Fh))          return 0   ; no planes
          if ((m & 2) == 0 && !target->vtable[5Ch](0Fh))         return 0   ; no non-planes
          return 1
```

`mask[7]` is a whole word, not a bit pair: `00861D70` stores `3` or `0` over
`owner+80h + 7*4` (`00861D8B` / `00861D98`) from `director+222h`, pushed by the
bridge `008624C0` step 4 at `00862581`. The category list it walks is `00E0A520 =
{7}` with the terminator `0Ch` at `00E0A524`. So `mask[7]` is `3` (both bits) or
`0` (nothing), never a partial mask. `director+222h` defaults to `1`
(`docs/DIRECTOR_UPDATE_ARMS.md`, `007202FD`) and the host's `DirectorGunneryStance`
defaults `torpedo = true` (`include/bsp/unit_gunnery_pass.hpp:220`), so **the host's
`mask[7]` is `3` and admits a surface ship**. The mask is not the gate.

### The class gate, `00862820`

`__thiscall(gunneryAi)(int category, Entity* target)`, `RET 8`, body
`00862820`-`008628CB`. Coverage: **complete**. Category 7 has its own arm:

```
00862869  if (category == 7) {
              if (!target->vtable[5Ch](6))     return 0    ; must be in the ship subtree
              if ( target->vtable[5Ch](0Eh))   return 0    ; must not be MTorpedoBoat
          }
0086288C  if (target->vtable[5Ch](8)) {                    ; a submarine
              if (category == 8 || category == 9) { if (00852820(target)) return 0 }
              else if (category != 7 && !00852820(target))  return 0
          }
```

Class `06h` is the unnamed ship base (`docs/ENTITY_CLASS_IDS.md`), and a destroyer's
compiled chain is `07, 06, 05, 04, 02, 01, 0`, so **`IsKindOf(6)` is true for a
destroyer** and the category-7 arm passes it. Two secondary readings of that arm:

- `0Eh` is `MTorpedoBoat`, which is absent from row `00E09D64`, so `rank == 0`
  already refuses it at step 3. The explicit `IsKindOf(0Eh)` test is belt and
  braces for a hypothetical `MTorpedoBoat` subclass.
- `41h` `NavPoint` has parent `01`, outside the class-6 subtree, so
  `IsKindOf(6)` is the only thing that keeps a NavPoint out of a torpedo's
  candidate list even though the row ranks it eighth.
- The `00852820` depth test exempts category 7, so **torpedoes may engage a
  submerged submarine** where every other non-depth-charge category may not.

The host implements neither the category-7 arm nor the submarine arm
(`score_candidate_00863990` in `src/game_hosts_gunnery.cpp:731` calls
`bsp::target_is_engageable_00862820(liveness)`, which carries only the four liveness
bytes). That omission makes the host *more* permissive, so it cannot be the gate
either. It is recorded in section 7 as a correction.

## 3. The gate that does refuse: the per-category engagement range

`00863990` keeps a candidate only when the distance is **strictly below**
`unit + 430h + category*4` (`00863A34`; the host's rule is
`src/unit_gunnery_pass.cpp:194`, `if (!(in.distance < in.category_range)) reject`).
`00956C20` step 11 seeds each row with the float `10.0f` at `00CE38B8` and maxes it
over the category's guns with `00731020(gun+3F4h)` at `00956D9F`
(`docs/GUNNERY_TABLES.md` section 2).

`00731020`, `__fastcall(GunClass* desc)`, `RET 0`, body `00731020`-`00731032`,
coverage **complete**, is eight instructions:

```
00731020  CMP dword ptr [ECX + 78h], 0
00731024  JNZ 00731029
00731026  FLD1 / RET                        ; no ammunition variant: 1.0f
00731029  MOV EAX, [ECX + 74h]              ; the variant array
0073102C  MOV ECX, [EAX + 34h]              ; the projectile class descriptor
0073102F  FLD float ptr [ECX + 60h]         ; the engagement range
```

So the whole question is **what `descriptor+60h` holds for a torpedo**.

### `+60h` is derived, not read from Lua

`006E8770 BSP_BulletClass_ReadLuaFields` reads 32 Lua keys and **writes neither
`+60h` nor `+64h`**; `docs/WEAPON_CLASS_DESCRIPTOR.md` already records that
"`+58h`, `+5Ch`, `+60h`, `+64h`, `+D8h` ... are untouched by this reader". The
keys it does write that matter here:

| key | offset | default | site |
| --- | --- | --- | --- |
| `V0` | `+50h` | `0.0f` (`FLDZ` at `006E94D6`) | `006E94EB` |
| `FlyTime` | `+54h` | `00D7A248` = `FLT_MAX` | `006E9257` |
| `Range` | `+68h` | `0.0f` (`FLDZ` at `006E950F`) | `006E9524` |

The getter is `00B66330 BSP_LuaReference_GetFloatOrDefault`, which loads the pushed
default when the key is absent or not a number, so **an absent `Range` leaves
`+68h` at `0.0f`**.

`+60h` is written by the class-descriptor **finalise** virtual, slot `+10h` of the
descriptor table (`00CFA138 + 10h` for `MBullet`, `00CFA440 + 10h` for
`MArtilleryBullet`, `00CFA56C + 10h` for `MTorpedo`, and eight more). The base
implementation is `006E9890`, `__fastcall(classDesc)`, `RET 0`, body
`006E9890`-`006E9A47`, coverage **complete** for the `+60h` computation:

```
006E9896  if ([this+64h]) return 1                  ; once-only latch
006E98A1  kind = [this+8h] ; [this+64h] = 1
006E98BC  kind in {4,5,6,7}:   [this+60h] = [this+68h]                 ; Range, bombs and rockets
                               [this+50h] = sqrt([this+68h] * 9.81)    ; and V0 becomes impact speed
006E9941  kind in {1,2,3,10h}: if ([this+68h] > 0) [this+54h] = [this+68h] / [this+50h]
                               [this+60h] = [this+54h] * [this+50h]    ; FlyTime * V0
006E9923  kind == 0Bh:         [this+60h] = DAT_00CFA428 = 240.0f      ; DepthCharge
006E9932  else:                [this+60h] = DAT_00CFA424 = 3000.0f     ; Torpedo and the rest
```

Three of the four `+60h` stores are `MOVSS dword ptr [EDI+60h],XMM0`
(`006E98F0`, `006E992B`, `006E993A`) and only the gun arm's is
`FSTP float ptr [EDI+60h]` (`006E9965`), so a byte scan for the x87 store form
alone finds one of the four. The constants are read from the image:
`00CFA424` is `00 80 3B 45` = `3000.0f` and `00CFA428` is `00 00 70 43` =
`240.0f`; `00D7A218`, the threshold the gun arm compares `Range` against, is
`0.0f`.

The sub-type at `+8h` is the projectile kind of `docs/PROJECTILE_KINDS.md`
(`1` Bullet, `4` Artillery, `0Ah` Torpedo, `0Bh` DepthCharge, `10h` Flak). For a
gun bullet with an authored `Range` the middle arm round-trips: `FlyTime` is set to
`Range / V0` and `+60h` comes back out as `Range`. **That round trip is the only
reason the host's `bc.Range` model works for the artillery categories at all.**

`MTorpedo` overrides the slot with `00855A90`, `__thiscall(classDesc)`, `RET 0`,
body `00855A90`-`00855AF5`, coverage **complete** (the same body
`docs/TORPEDO_TICK.md` reads as the finalise hook):

```
00855A95  ok = 006E9890(this)                       ; the base sets +60h = 3000.0f
00855A9A  FLD  float [this + 0E4h]                  ; WaterTravelSpeed
00855AA0  FMUL float [this + 54h]                   ; FlyTime
00855AA5  FMUL double [00CEFF98] = 0.6
00855AAB  FSTP float [this + 60h]                   ; the torpedo's engagement range
00855AAE  [this+0ECh] = sqrt(2 * [this+0E0h] * 9.81)  ; MaxFall terminal speed
00855AD1  return ok && [this+0ECh] > 0.0f
```

`+0E4h` is `WaterTravelSpeed` and `+0E0h` is `MaxFall`, both from
`008566B0 BSP_TorpedoClass_ReadLuaFields` (`00856761` and `0085672E`, key strings
`00D0C5C4` and `00CFBCC4`). `MFlakBullet` overrides the same slot with `0070C0B0`,
which calls the base and then derives `+0D8h = [+58h] / [+50h]`, leaving `+60h`
alone.

**So a torpedo's engagement range is `WaterTravelSpeed * FlyTime * 0.6`, and the
Lua key `Range` is never consulted for it.**

### What that is worth in this installation

Installed game: `I:/SteamLibrary/steamapps/common/Battlestations Pacific`.
`gamemode.lua` (mtime 2024-07-13 11:27) sets `GameMode = 0`, so
`scripts/datatables/autoload/bulletclasses.lua` (2024-07-13 08:26) loads
`Scripts/datatables/classtables/arcade/bulletclasses.lua` (**mtime 2026-05-09
23:04 - modded, BSPRM / "Kantai Kessen"**) as the global `Bullets`.
`Scripts/datatables/classtables/arcade/deviceclasses.lua` is 2026-05-09 22:37,
also modded; `.../realistic/bulletclasses.lua` is 2026-06-02.

All twelve `Type = "Torpedo"` bullet classes in the loaded arcade table, with
`WaterTravelSpeed * FlyTime * 0.6`:

| id | name | `Range` | `FlyTime` | `WaterTravelSpeed` | `+60h` |
| --- | --- | --- | --- | --- | --- |
| 4 | Fido homing airplane torpedo | absent | 60 | 30.8664 | 1111.2 |
| 27 | 17.7 Type 91 Mod3 airplane torpedo | absent | 60 | 51.444 | 1852.0 |
| 29 | 22.4 Mark 13 airplane torpedo | absent | 60 | 51.444 | 1852.0 |
| 61 | 21. Mark 14 submarine torpedo | absent | 60 | 51.444 | 1852.0 |
| **62** | **21. Mark 15 ship torpedo** | **absent** | **60** | **51.444** | **1852.0** |
| 63 | 22.4 Mark 13 airplane torpedo | absent | 60 | 51.444 | 1852.0 |
| 64 | 19. Mark 24 airplane torpedo | absent | 60 | 51.444 | 1852.0 |
| 65 | 21. Mark 14 submarine torpedo | absent | 60 | 51.444 | 1852.0 |
| 66 | 21. Type 95 Mod1 submarine torpedo | absent | 60 | 51.444 | 1852.0 |
| 67 | 24. Type 93 Mod1 Long Lance ship torpedo | absent | 60 | 170.444 | 6136.0 |
| 69 | 17.7 Type 91 Mod3 airplane torpedo | absent | 60 | 51.444 | 1852.0 |
| 70 | 21. Type 95 Mod1 submarine torpedo | absent | 60 | 51.444 | 1852.0 |

Bullet class 62 is the round of device class 62 in `deviceclasses.lua`, "Torpedo
Tube 2X US", `Function = "TORPEDO"`, whose `Bullet[1].Bullet` is `62`. The
realistic table also has no
`Range` on any torpedo class, so the absence is not an arcade-only or mod-only
artifact of one file. 1852 m is one nautical mile and 51.444 m/s is 100 knots;
the numbers are authored, not accidental.

**Native category-7 engagement range for a US destroyer: `max(10.0, 1852.0)` =
1852 m.** The run's nearest enemy at 637 m is well inside it.

### What the host computes instead

```
src/game_hosts_gunnery.cpp:270-300   f[q..'range'] = num(bc.Range, 1000) or 0
src/game_hosts_gunnery.cpp:437       gun.max_range = flat_scaled(type_id, "p<N>_range", 1000, 0.0f)
src/game_hosts_gunnery.cpp:560       out.max_range = gun.max_range;   // "00731020"
src/gunnery_tables.cpp:172-196       category_engagement_range_00956d63: seed 10.0f, max over max_range
src/game_hosts_gunnery.cpp:759       in.category_range = state_.category_ranges[slot]
src/unit_gunnery_pass.cpp:194        reject unless distance < category_range
```

`bc.Range` is absent for every torpedo class, so `gun.max_range` is `0.0f` for all
71 torpedo mounts, `category_ranges[7]` is the bare seed `10.0f`, and
`score_candidate_00863990` refuses every candidate at every distance above 10 m.
The same `0.0f` refuses a second time in `gun_inputs`
(`src/game_hosts_gunnery.cpp:888`, `in_range = length3(delta) <= row.max_range`),
which stands in for `00729BC0`. Two independent host gates, one root cause.

## 4. Steps 8.3 and 8.5 do not participate

Step 8.2 at `00865191` computes `enabled = [this+70h+i] && [this+80h+i*4]`, and
step 8.3 at `008651B4` replaces it with `[this+7Ch]` when `i == 7`. The listing:

```
008651B2  JZ  008651C0        ; enabled clear: skip 8.3
008651B4  CMP [ESP+14h], 7
008651B9  JNZ 008651C0
008651BD  MOV BL, [EAX + 7Ch]
008651C0  ... [this+60h]->vtable[4h](i) ; JZ 00865773 on a refusal
008651DB  TEST BL, BL
008651DD  MOV EBX, [ESP+0Ch]  ; BL destroyed here
008651E1  JZ  00865442
008651F5  CMP ESI, 7
008651F8  JZ  00865442
```

`BL` is read once, at `008651DB`, and overwritten two instructions later; both its
`JZ` and the unconditional `i == 7` test at `008651F8` land on the same target
`00865442`. **`this+7Ch` therefore has no observable effect on category 7's gather
at all** - it can only skip a sweep that `008651F8` skips anyway. The host's
`torpedo_category_enabled()` is likewise inert, and its value (`true`) is not the
gate.

Step 8.4's gate at `008651C0` jumps to `00865773`, which is the **first**
instruction of step 8.8, not past it: a refused gate skips the gather and then
still walks the category's guns with an empty candidate list, clearing each one
through `00728000` at `0086586C`. The host's comment at
`src/unit_gunnery_pass.cpp:386`-`387` ("the native code jumps past `00865773` as
well") is wrong about that, though the `continue` it guards produces the same
assignment count. The gate itself is `[this+60h]->vtable[4h]`, `00861BE0`
(`MOV AL,1`) on a unit that is not class 8, so it does not refuse here.

## 5. There is no separate native torpedo feed

`00727F10` has exactly one direct caller in the call graph, `00864FE0` at
`00865833`, and the graph is not truncated (a 20-row query returned one row).
Step 8.5 excludes category 7 outright at `008651F5`, so the **only** native route
from the gunnery pass to a torpedo mount is step 8.7: the director's command
target (`008654D4`) and fire target (`00865626`), each expanded into sub-entities,
each scored by `00863990`. Step 8.8 then requires, per candidate:

| site | callee | rule for a torpedo mount |
| --- | --- | --- |
| `008657C0` | `005459E0` | false for a category-7 gun (`[[gun+3F4h]+80h]` is `7`, not `5` or `6`), so the air-minimum arm is skipped |
| `008657E1` | `00729B90` | not reached |
| `008657F7` | `00729BC0` | rejects when the recomputed distance exceeds `[proj+60h]` - the **same** `+60h` as section 3 - then dispatches projectile kind `0Ah` to the bot slot at `gun+39Ch` and returns `slot->vtable[1Ch](t)` |
| `00865809` | - | `i == 7 && candidate == fireTarget && ![this+7Dh]` refuses; `+7Dh` is `1` in the constructor (`008645D6`) |
| `00865833` | `00727F10` | the assignment |

So the answer to "is a torpedo mount fed through the same `00727F10` assignment as
a gun" is **yes**, with the recon sweep removed: a torpedo takes only what the
weapon director points at. The torpedo bot at `gun+39Ch` and its intercept solver
(`torpedo_intercept_point_008FBB00` in the reconstruction) sit **behind**
`00729BC0`'s `vtable[1Ch]`, they are not an alternative feed. This is a negative
result: **no separate native torpedo target path was found**, and the one path
that exists is gated on `[proj+60h]`, which the host currently computes as zero.

## 6. What the host would need

All of it is in files this packet does not own; the change is mechanical.

1. `src/game_hosts_gunnery.cpp`, the flatten chunk near line 296: also publish the
   keys the finalise hook needs -
   `f[q..'kind'] = <Type>`, `f[q..'fly'] = num(bc.FlyTime, 1000)`,
   `f[q..'wts'] = num(bc.WaterTravelSpeed, 1000)`. `Type` is a string
   (`"Torpedo"`, `"DepthCharge"`, ...); the sub-type numbers are in
   `docs/PROJECTILE_KINDS.md`, and the gun's own category already distinguishes
   `TORPEDO` (7) from `DEPTHCHARGE` (8) and `DEPTHCHARGELAUNCHER` (9) without
   reading `Type` at all.
2. `src/game_hosts_gunnery.cpp:437`: replace
   `gun.max_range = <p_range>` with the `006E9890` / `00855A90` rule:
   - torpedo mount: `WaterTravelSpeed * FlyTime * 0.6f`
   - depth-charge mount: `240.0f` (`00CFA428`)
   - bomb / rocket kinds `4`..`7`: `Range`
   - gun kinds `1`, `2`, `3`, `10h`: `Range` when `Range > 0`, else
     `FlyTime * V0` (with `FlyTime` defaulting to `FLT_MAX`)
   Everything else: `3000.0f` (`00CFA424`).
   The existing behaviour of categories 2, 3 and 6 is unchanged, because for those
   the rule returns `Range`.
3. `src/game_hosts_gunnery.cpp:731` `score_candidate_00863990`: add the
   `00862820` category-7 arm (`IsKindOf(6)` required, `IsKindOf(0Eh)` refused) and
   the submarine depth arm, so the newly reachable category does not over-admit.

Expected effect: `category_ranges[7]` becomes 1852 m for a Mk 15 mount, the
director's fire target at 637 m passes `00863990`, `00729BC0`'s stand-in passes,
and `00727F10` assigns. Whether a shot follows then depends on the torpedo bot's
own aim path, which this packet did not exercise.

## 7. Corrections to existing material

| where | was | is | evidence |
| --- | --- | --- | --- |
| `src/game_hosts_gunnery.cpp:560` comment `out.max_range = gun.max_range; // 00731020` | the ammunition record's `+60h` is the Lua `Range` | `+60h` is derived by the finalise hook `006E9890`; `Range` lives at `+68h` and reaches `+60h` only for the gun and bomb kinds | `006E9524` stores `Range` to `+68h`; `006E9890` and `00855A90` are the only writers of `+60h` |
| `src/unit_gunnery_pass.cpp:386` | "the native code jumps past `00865773` as well" | `008651D1 JZ 00865773` lands **on** step 8.8, which clears every gun in the category | the listing at `008651C0`-`008651D1` and `00865773`-`0086587E` |
| `src/game_hosts_gunnery.cpp:562`-`564`, `GunneryRebuildDevice::has_blast` / `blast_inner` / `blast_outer` fed from `b->blast_range` | `00956C20`'s `unit+460h` term is a blast pair | `[[d+34h]+0ACh]` is `DamageMin` and `+0B0h` is `DamageMax` (`docs/WEAPON_CLASS_DESCRIPTOR.md`), so `unit+460h+c*4` sums **average direct damage**, not blast radius | the descriptor key table; `docs/GUNNERY_TABLES.md` section 2 quotes the offsets correctly but the host binds the wrong source |
| `docs/TORPEDO_TICK.md` line 22 | the class-descriptor finalise slot is `+0Ch` | it is `+10h`: `00CFA56C + 10h = 00CFA57C` holds `00855A90`; `00CFA578` holds `006EA520` | `00CFA574` reads `50 A5 6E 00  20 A5 6E 00  90 5A 85 00  F0 97 6E 00` |

None of these files are owned by this packet; the corrections are recorded here and
in the report for their owners.

## 8. Proven, assumed, and not read

**Proven from the listing and the installed data.**

- `mask[7]` is `3` or `0` and defaults to `3`; it does not refuse a ship.
- Row `00E09D64` ranks `MDestroyer` fourth for category 7.
- `00862820`'s category-7 arm admits a destroyer and excludes `NavPoint`.
- `006E8770` writes `Range` to `+68h`, not `+60h`, and defaults an absent key to
  `0.0f` through `00B66330`.
- `006E9890` and `00855A90` are the only writers of descriptor `+60h`; the torpedo
  value is `WaterTravelSpeed * FlyTime * 0.6`.
- No torpedo bullet class in either loaded table carries `Range`; all twelve carry
  `FlyTime` and `WaterTravelSpeed`.
- `00727F10` has one direct caller; step 8.5 excludes category 7; `this+7Ch` is
  dead for the category's gather.

**Assumed.**

- That the finalise hook actually runs before `00956C20` reads `+60h`. Its
  invocation site is **contract: unread** - `006E9890` is installed in eleven
  descriptor tables at slot `+10h` and is reached only through that slot, and
  `+64h` is its once-only latch. The supporting argument is that without it
  `+60h` would be zero for *every* kind and no category would have an engagement
  range above the `10.0f` seed, which the working artillery categories contradict.
- That the USN02 destroyers mount `ArcadeTable[62]`-class rounds. The formula and
  the "no `Range` on any torpedo class" result do not depend on it; only the
  specific figure 1852 m does.

**Not read.**

- The finalise hook's caller (above).
- `006E9890`'s tail, read from pseudocode only and not from the listing: a
  `kind == 1` arm that searches the descriptor's name for `"AA"` and rewrites
  `+8h` to `2` or `3`, and a `kind == 4` arm that compares `+0ACh` and `+0B4h`
  against `00CF0B50` and `00CE3808` and rewrites `+8h` to `5`, `6` or `7`.
  Coverage of `006E9890` is therefore **partial: `006E9968`-`006E9A47`, the `+8h`
  rewrite arms, is pseudocode-only**. `006E9890`-`006E9967`, the whole `+60h`
  computation, is read in the listing and complete. Neither rewrite arm touches
  `+60h`, and both run after it is written.
- `00729BC0`'s `slot->vtable[1Ch]` for the torpedo bot at `gun+39Ch`.
- Whether any run-time consumer other than `00731020` and `00729BC0` reads `+60h`.

**No run-time evidence of the fix.** The only run data in this document is the
measured table supplied with the packet (`0` assignments for category 7). The
`max_range == 0` step is proven statically from the authored file plus the flatten
chunk; the packet built nothing, because it publishes no source.

**No module published.** The one new rule here - the per-kind engagement range - has
to be called from `src/game_hosts_gunnery.cpp`, which this packet does not own, and
a header plus source that nothing calls would be an uncalled duplicate of a rule
that belongs next to `category_engagement_range_00956d63` in
`include/bsp/gunnery_tables.hpp`. Section 6 states it precisely enough to wire in
one edit.

## 9. Follow-up packets

1. **`projectile_engagement_range`** - move the `006E9890` / `00855A90` /
   `0070C0B0` rule into `include/bsp/gunnery_tables.hpp` next to
   `category_engagement_range_00956d63`, call it from
   `src/game_hosts_gunnery.cpp`'s gun build, and re-run the 3200-frame USN02
   mission. Owns `00731020`, `006E9890`, `00855A90`, `0070C0B0`,
   `include/bsp/gunnery_tables.hpp`, `src/gunnery_tables.cpp`,
   `src/game_hosts_gunnery.cpp`.
2. **`gunnery_class_gate_completion`** - implement `00862820` steps 4 and 5 in the
   host's `score_candidate_00863990` (the category-7 ship / torpedo-boat arm and
   the `00852820` submarine depth arm). Needs `00852820` and the class-id
   predicates the host already has.
3. **`category_damage_sum`** - correct `unit+460h + c*4` from a blast pair to the
   `(DamageMin + DamageMax) * 0.5` sum, and find its consumer; nothing in the
   gunnery pass reads it, so the consumer is elsewhere.
4. **`descriptor_finalise_call_site`** - find who calls descriptor slot `+10h`,
   which settles the assumption in section 8 and dates the derivation relative to
   `00956C20`.
5. **`torpedo_bot_accept`** - read `00729BC0`'s `gun+39Ch` slot `vtable[1Ch]` and
   the torpedo bot's aim path, which is what decides whether an assignment turns
   into a shot.
