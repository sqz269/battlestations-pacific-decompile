# Depth charge, flak and catapult: why three categories take no assignment

Addresses: `00E09A5C`, `00E09EE8`, `00E0A374`, `00E0A06C`, `00E0A1F0`, `00E0A4F8`, `00E0A510`,
`00E0A520`, `00E0A528`, `00727BD0`, `00862820`, `00852820`, `008527A0`, `008527E0`, `00853630`,
`00861CD0`, `00861D20`, `00861D70`, `00861DC0`, `008624C0`, `00863A80`, `00864580`, `00861C20`,
`008945F0`, `00865191`, `008651C0`.

`00863990`, `008633D0`, `00864FE0`, `006E9890` and `00826F10` are read as **contracts** from
`docs/UNIT_GUNNERY_PASS.md`, `docs/TORPEDO_CATEGORY_ADMISSION.md`,
`docs/BULLET_ENGAGEMENT_RANGE.md` and `docs/SHIP_HIT_RECORD.md`. They are not leased or
re-derived here.

## Summary: three independent verdicts

| cat | Function | guns | assigns | verdict | the gate that decides it |
| --- | --- | --- | --- | --- | --- |
| `5` | FLAK | 60 | 0 | **faithful zero, two independent causes** | the authored row ranks no class that exists on side 1 within reach; and the classes it does rank sit at 2960 m against a category range of 2000 m |
| `8` | DEPTHCHARGE | 10 | 0 | **faithful zero** | the range gate: 970 m to the nearest submarine against a 240 m category range. In the native build a second, independent gate also refuses: `00862820`'s category-8 arm |
| `0Bh` | CATAPULT | 4 | 0 | **faithful zero, structural** | the authored row is empty across all `61h` entries, so `gunnery_rank(0Bh, *)` is 0 for every class id. A catapult authors no `Bullet` at all; it is a launch device, not a weapon |

No defect was found in any of the three. One genuine host divergence is recorded in section 7;
it does not change any of the three counts on `IJN01`.

## 1. The three authored rows, read whole

`00E092C8`..`00E0A4F8` is twelve rows of `61h` dwords on a `184h` stride, file-initialized in
`.data` (`docs/GUNNERY_TABLES.md` §1). Earlier documents quote only the leading entries. All
`61h` entries of each row below were read, because `00727BD0`'s inner loop scans the whole row
and only *skips* a zero rather than stopping at one.

| row | VA | non-zero of `61h` | ids, best first |
| --- | --- | --- | --- |
| 5 FLAK | `00E09A5C` | 9, at indices 0-8 | `17h 11h 12h 15h 16h 14h 10h 13h 0Eh` |
| 8 DEPTHCHARGE | `00E09EE8` | 2, at indices 0-1 | `08h 41h` |
| 9 DEPTHCHARGELAUNCHER | `00E0A06C` | 2, at indices 0-1 | `08h 41h` |
| `0Ah` BOMBPLATFORM | `00E0A1F0` | **0** | - |
| `0Bh` CATAPULT | `00E0A374` | **0** | - |

Bytes as read (`bsp.py ghidra bytes`, first 48 of 388):

```
00e09a5c  17 00 00 00 11 00 00 00 12 00 00 00 15 00 00 00   FLAK
00e09a6c  16 00 00 00 14 00 00 00 10 00 00 00 13 00 00 00
00e09a7c  0e 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00e09ee8  08 00 00 00 41 00 00 00 00 00 00 00 00 00 00 00   DEPTHCHARGE
00e0a374  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   CATAPULT
```

Through `docs/ENTITY_CLASS_IDS.md`:

- **FLAK admits `MPlaneKamikaze, MPlaneTorpedoBomber, MPlaneDiveBomber, MSmallReconPlane,
  MLargeReconPlane, MReconPlane, MPlaneBomber, MPlaneFighter` and `MTorpedoBoat`.** Eight air
  classes and exactly one surface class. It does **not** admit `MSubmarine`.
- **DEPTHCHARGE admits `MSubmarine` and `NavPoint`, and nothing else.** Category 9 is identical.
- **CATAPULT admits nothing.** The milestone note in `docs/GAME_EXECUTABLE.md` is confirmed
  against the bytes, for the whole row and not only its head.

### `00727BD0`, the producer, read before the layout was stated

`00727BD0`-`00727C23`, `__cdecl()`, `RET 0`, coverage **complete**:

```
00727bd6  EBP = 0E19BF8h ; ESI = 0E092C8h ; EBX = 0
00727be0  per category: EAX = 0 ; ECX = 61h ; EDI = EBP ; EDX = 1
00727bec    REP STOSD          zero all 61h dwords of this rank row
00727bee    EAX = ESI (row) ; ESI = EDX + 60h = 61h   (inner count, not a terminator)
00727bf3    ECX = [EAX] ; if (ECX == 0) goto next entry     <- zeros are SKIPPED, not final
00727bfb    [00E19BF8 + (ECX + EBX)*4] = EDX ; EDX += 1
00727c0d  EBP += 184h ; EBX += 61h ; loop while EAX < 0E0A4F8h
```

So `rank[category*61h + classId]` is the **one-based position among the non-zero entries**, and
0 means "never engaged". The row is scanned to its full `61h` length, which is why the whole row
had to be read. `00727C16`'s `CMP EAX, 0E0A4F8h` bounds the table at exactly twelve rows.

The one unbounded write is category 0's id `61h`, which lands on `00E19BF8 + 184h`, the first
dword of rank row 1, and is erased by row 1's own `REP STOSD` at `00727BEC`. No other row holds
an id above `46h`, so **no other rank row is corrupted, and row `0Bh` is genuinely all zero**
rather than zeroed by an overflow.

## 2. Is a depth charge driven by the gunnery pass at all? Yes

The question was whether category 8 is swept and assigned like a gun, or released by a separate
launcher or pattern mechanism. Five independent pieces of evidence say it is an ordinary gun:

1. **It is swept.** `docs/UNIT_GUNNERY_PASS.md` step 8.5 runs the recon sweep for every category
   except `i == 7`. `008651D7` is reached for `i == 8` whenever steps 8.1-8.4 pass. Category 7
   is the only category the native code excludes from the sweep; category 8 is not.
2. **It has a dedicated arm in the class gate.** `00862820` (section 3) branches on
   `category == 8 || category == 9`.
3. **It has a dedicated mask setter and a dedicated director flag.** `00861DC0` walks the
   category list `00E0A528 = {8, 9}` and is driven by `director+223h` `depthChargeEnabled`
   through `008624C0` step 5 at `008625D3`.
4. **It has a dedicated scripted inhibit bit.** `unit+634h` bit 3 is the `DC` group of the
   `"All|AA_Flak|Artillery|Torpedo|DC"` descriptor (`docs/UNIT_GUNNERY_PASS.md` §8).
5. **The device is an ordinary turning gun.** Every `Function = "DEPTHCHARGE"` device in
   `classtables/arcade/deviceclasses.lua` has `Type = "Single_Turning_Gun"` and a nested
   `["Bullet"] = { ["Bullet"] = <id> }`. Device class `54`, `US Destroyer DC launcher`, is
   `Function = "DEPTHCHARGE"`, `Type = "Single_Turning_Gun"`, bullet class `54`.

The only depth-charge-specific machinery outside the gunnery pass is on the **damage** side:
`docs/SHIP_HIT_RECORD.md` rule R0 at `00826F38` refuses a shot that passes `vtable[5Ch](2Ch)`
`MDepthCharge` against a hull that fails `vtable[5Ch](8)` `MSubmarine`. That is the same policy
as the firing side, enforced twice.

**So "depth charges are not AI-assigned in this build" is false. They are assigned exactly like
any other gun category, and their zero has to be explained by a gate.**

## 3. `00862820`, the class gate, and the submarine depth arm

`__thiscall(gunneryAi)(int category, Entity* target)`, `RET 8`, body `00862820`-`008628CB`.
Coverage: **complete**. Args confirmed from the frame: `[ESP+8]` is the category (`EDI`),
`[ESP+0Ch]` the target (`ESI`), and the `RET 8` matches two stack arguments.

```
00862825  liveness: [t+5Ch] != 0 && [t+5Dh] == 0 && [t+60h] == 0 && [t+5Eh] != 0
00862843  EAX = category*61h + [t+0C4h]                       ; classId
00862855  if ([this + EAX + 0CCh] == 0)                return 0   ; per-(cat,class) permission
0086285f  if (DAT_00E19BF8[EAX] == 0)                  return 0   ; the rank
00862869  if (category == 7) { need IsKindOf(6); reject IsKindOf(0Eh) }
0086288c  if (t->IsKindOf(8)) {                                   ; a submarine
0086289b      if (category == 8 || category == 9) { if ( 00852820(t)) return 0 }
008628b7      else if (category != 7)              { if (!00852820(t)) return 0 }
          }
008628c8  return 1
```

`00852820` (`BSP_Entity_IsAboveDepthChargeDepth`, `__thiscall(sub)`, `RET 0`, body
`00852820`-`0085285A`, coverage **complete**) is

```
return [sub+100h] > ([sub+1204h] + [sub+1200h]) / 3.0      ; 3.0 is the double at 00D7A2B0
```

Its two siblings settle which way it points, without relying on the existing ledger name:

| routine | test | consumer |
| --- | --- | --- |
| `008527A0` | `[sub+100h] > [sub+1200h] - 3.0` | `008627A0`, the class-8 category gate, lets a submarine's **own categories 1, 2, 5 and 6** run only when this holds - deck guns need the boat near the surface |
| `008527E0` | `[sub+100h] > [sub+1204h] - 3.0` | the same gate lets the submarine's **category 7** run only when this holds - torpedoes reach deeper than deck guns |
| `00852820` | `[sub+100h] > ([sub+1204h] + [sub+1200h]) / 3.0` | `00862820`'s submarine arm |

`[sub+1200h]`..`[sub+120Ch]` are four floats copied at unit init by `BSP_SubmarineUnit_SEntityInit`
`00853630`: `00853A87  LEA ECX,[ESI+1200h]` with the source running `EAX = 80Ch` to `81Ch`
(`00853AC0  CMP EAX, 81Ch`) out of the class descriptor at `[ESI+538h]`. The class reader
`BSP_SubmarineClass_ReadLuaFields` `00854230` writes `class+810h` from **`PeriscopeDepth`**
(falling back to `SwimDepth1`) and `class+814h` from **`SwimDepth2`**. So `+1200h` is the
surfaced reference and `+1204h` the periscope reference, and `[sub+100h]` is the boat's own
vertical coordinate, increasing toward the surface.

**The policy the three routines encode is coherent and complete:** every category except 7
engages a submarine only while it is shallow; categories 8 and 9 engage it only while it is
**not** shallow; torpedoes engage it at any depth. **Category 8 and every gun category are
mutually exclusive on the same submarine at the same instant.**

## 4. What actually runs, and what refuses: the four gates before the range

Each was read and each **passes** for categories 5, 8 and `0Bh`. None of them is the answer.

| gate | site | rule | why it passes |
| --- | --- | --- | --- |
| 8.1 record present | `0086516D` | `[unit + 394h + i*0Ch] != 0` | the run shows 60 / 10 / 4 guns in the three categories |
| 8.2 enable and mask | `00865191`, `00865198` | `[this+70h+i] != 0 && [this+80h+i*4] != 0` | `BSP_UnitGunneryAi_Construct` `00864580` seeds **every** category: `0086462D  [ESI+EDI+70h] = 1` and `00864632  [EBP] = 3` with `EBP = ESI+80h` stepping `4`, for `i = 0..0Bh` |
| 8.4 category gate | `008651C0` | `[this+60h]->vtable[4h](i)` | `008636A0` installs the default gate `00D0D31C` on any unit that is not class 8; its `vtable[4h]` is `00861BE0`, `mov al,1` |
| class permission | `00862855` | `[this + cat*61h + classId + 0CCh] != 0` | the same constructor loop memsets `61h` bytes per category to `1` (`00864623  PUSH 61h / PUSH 1 / PUSH EBX`) |

### The four mask setters

`docs/DIRECTOR_UPDATE_ARMS.md`'s coverage table lists `00861CD0`/`00861D20`/`00861D70`/`00861DC0`
as "**unread**". That entry is stale: the ledger records from packet `cc2_unit_gunnery_pass`
already carry each setter's category list and bit operation. All four were re-read here from the
listing, and what is new is the last two bullets below - the `refreshAll` asymmetry and the
consequence of the constructor's defaults. Each walks a category list terminated by the first
entry `>= 0Ch`; the lists sit immediately after the twelve preference rows:

| setter | list | VA | categories | what it writes |
| --- | --- | --- | --- | --- |
| `00861CD0` artillery | `00E0A4F8` | `01 02 03 04 06 | 0C` | 1, 2, 3, 4, 6 | `OR 2` / `AND ~2` on `mask[cat]` - the **non-plane** bit |
| `00861D20` aa | `00E0A510` | `01 05 06 | 0C` | 1, 5, 6 | `OR 1` / `AND ~1` - the **plane** bit |
| `00861D70` torpedo | `00E0A520` | `07 | 0C` | 7 | whole word `3` or `0`, plus `[this+77h]` |
| `00861DC0` depth charge | `00E0A528` | `08 09 | 0C` | 8, 9 | whole word `3` or `0`, plus `[this+78h]` |

Two consequences that matter here:

- **Categories 0, `0Ah` and `0Bh` appear in no list at all.** No setter ever touches their mask
  or enable byte, so they keep the constructor's `1` and `3` for the life of the unit.
  `008624C0` step 3's `allowFire` fill covers `[this+70h]`..`[this+7Bh]`, so it can clear them,
  but on `IJN01` nothing does. **Category `0Bh` is therefore swept every tick; it simply never
  ranks anything.**
- **`00861DC0` is called on a change only**, with no `refreshAll` bypass (`008625D9  CMP AL,
  [ESI+10h] / JZ`), unlike the other three. The bridge record is 20 bytes allocated at
  `00863ABB` with `[+0Ch]`..`[+10h]` all zero (`00863AD1`-`00863ADD`) and then called once with
  `force = 1`; `director+223h` defaults to `1` (`007202FD`), so the first call always fires
  `00861DC0(1)` and sets `mask[8] = mask[9] = 3`. **Depth charges are enabled, not disabled.**

### The per-(category, class) permission table is inert in this installation

`[gunneryAi + 0CCh]` is `12 * 61h` bytes, one per (category, class id) pair, all `1` from the
constructor. Its only writer is `00861C20` (`__thiscall(int classId, int category, bool)`,
`RET 0Ch`; a negative category writes all twelve rows through the `ADD EAX, 61h` loop at
`00861C52`). `00861C20` has **exactly one** call site, `008947D0` in `FUN_008945F0`, whose error
literal is `"luaMW_SetWeaponDirectorTargetEnable failed:"` and whose binding name string is
`SetWeaponDirectorTargetEnable` at `00D0FF64`, referenced from the descriptor row at `00E0BF54`.

`grep -rn "SetWeaponDirectorTargetEnable" --include=*.lua` over the whole installation returns
**nothing**. The table is all-`1` for every unit in every mission here. Negative result,
labelled as one.

## 5. What IJN01 actually contains

Run: `local/ijn01_500.log` in the main checkout, `--frames 700 --press-start-frame 30
--menu-select IJN01 --mission-frames 500 --mission-frame-seconds 0.05`, the same 500-tick run the
packet's table was taken from. **A fresh run could not be taken this session**: `bsp_game.exe`
built from this worktree faults with `0xC0000005` immediately after
`Phase 5 load_game_settings`, in `sound_->core.startup()` /
`Phase 5 sound_system_initialize 0073DAFD` (`src/game_hosts.cpp:1378`-`1381`), reproducibly and
also with a bare `--frames 3` and a private `--settings-personal-root`.
`docs/PLANE_UNIT_TICK.md` §6 records the identical fault from the main checkout's own build, so
it is the session's environment. Everything in this section is quoted from the existing log.

The unit table has 81 rows: 46 side 0 (US), 30 side 1 (Japanese), 1 side 2, and four unnamed.
**Side 1 is 29 aircraft plus the two Ko-hyoteki midget submarines `No18` and `No19`** - `No19` is
filtered out of the print because it ends the run with no guns and no hits. There is no
`MTorpedoBoat` on side 1: `PT1`-`PT4` are side 0.

### The seven units that carry a category-8 gun - complete, 10 of 10

| unit | cats | `nearest` |
| --- | --- | --- |
| `Cassin` | `1:4 6:5 7:3 8:2` | **966 m** |
| `Downes` | `1:4 6:5 7:3 8:2` | 1126 m |
| `Mona` | `1:4 6:5 7:2 8:2` | 3220 m |
| `PT1` / `PT2` / `PT3` / `PT4` | `1:3 8:1` | 3481 / 3348 / 3274 / 3511 m |

`2+2+2+1+1+1+1 = 10`, the whole category. The closest is `Cassin`, and the run names the target
directly:

```
gunnery: first shot at t=0.40 s, Cassin platform 4 (LIGHTARTILLERYFLAK, bullet 15)
         at No18, range 970 m, horz 187.4 deg, vert 2.2 deg
```

**970 m is the distance from the nearest depth-charge-carrying ship to the nearest submarine.**

### The eight units that carry a flak gun - complete, 60 of 60

`Enterprise 5:8`, `Lexington 5:8`, `Arizona 5:8`, `Pennsylvania 5:8`, `West Virginia 5:7`,
`Maryland 5:7`, `Tennessee 5:7`, `California 5:7`. Their `nearest` runs 1938 m to 3120 m.

**`Enterprise` is the isolation case**: its only categories are `1:11` and `5:8`, and its
`range` column - `state_.row.best_range`, the largest `category_range` any *scored* candidate was
measured against - reads **2000**. The AAMACHINEGUN maximum over every device in the game is
1600 m (bullet 42, 46 and 108 at `FlyTime 2.0 * V0 800`; ids 36/40/41/43/95/107 give 960 and
84/89/91 give 800). So 2000 is category 5's own range, **and the fact that it was recorded at
all proves a ranked candidate reached the range comparison for category 5**.

### The 29 aircraft

The 21 that carry a category-1 gun report their own nearest ranked US unit; the minimum over all
21 is **2960 m** (`JillSpawn12`, matched by `LST1`'s own `nearest` of 2960). The other 8 are the
`A7M`, which report `0` because category 0's row is inert and they never score a candidate
(`docs/PLANE_UNIT_TICK.md`); `docs/AA_VERTICAL_WINDOW.md` places them at `z = 3500..3800` against
the Jills' `z = 3000+`, so they are farther out still.

**No aircraft is within 2960 m of any unit on this mission.**

### The four catapults

`Helena 11:2`, `Massachusetts 11:1`, `Iowa 11:1` - 4 of 4.

## 6. The three verdicts

### DEPTHCHARGE, category 8: faithful zero

The whole chain was walked for `Cassin`, which has both the guns and the target:

1. `category_record_present(8)` - 2 guns. Passes.
2. `enabled[8] && mask[8] != 0` - `1` and `3` from the constructor, re-affirmed by
   `00861DC0(1)` on the bridge's first call. Passes.
3. the category gate's `vtable[4h]` - `00861BE0`, `mov al,1`. Passes.
4. `[this + 8*61h + 8 + 0CCh]` - `1`, and nothing in the installation ever writes it. Passes.
5. `gunnery_rank(8, 08h)` - `1`, the row's first entry. Passes.
6. the mask's plane/non-plane bits against a non-plane - `3 & 2`. Passes.
7. **the engagement range.** `00863990` keeps a candidate only when the distance is strictly
   below `unit + 430h + 8*4`, the maximum weapon range over that unit's category-8 guns.

Every category-8 device in the installation authors one of bullet classes `12`, `32`, `54`, `57`
(the real launchers) or `144`, `154` (two `Watermine Test Launcher` devices). All four real ones
are `Type = "Depthcharge"` with no `FlyTime`, so `006E9890`'s depth-charge arm at `006E9923`
writes the constant `240.0f` from `00CFA428`. Bullet `12` and `32` do not even author a `Range`;
`54` and `57` do not either. The `Mahan 1941` class `VehicleClass[309]`, which is `Cassin` and
`Downes` (`unit hull input unit=Cassin type_id=309 ... key=Destroyer`), carries device class `54`
`US Destroyer DC launcher`, bullet class `54`. **The category-8 range is 240 m.**

**970 m against 240 m. Every category-8 gun on this mission is at least four times its own reach
from the nearest thing its row ranks.** The other six units are 3.2 km to 3.5 km out.

In the native build a **second, independent** gate also refuses, and it is worth stating because
it makes the verdict hold whichever way the submarines' depth resolves:

- if `No18`/`No19` are shallow, `00852820` is true and `00862820`'s `008628AE` refuses category 8
  outright, before any distance is computed;
- if they are deep, category 8 clears the class gate and is refused at `00863A34` on 970 m
  against 240 m.

**Category 8 takes zero assignments on IJN01 under either reading. The host reaches the same zero
through the range gate alone, because it does not implement the submarine arm (section 7).**

### FLAK, category 5: faithful zero, and it is a consequence of the planes not flying

Two sufficient causes, each proven separately.

**(a) Nothing the row ranks exists within reach.** The row admits eight aircraft classes and
`MTorpedoBoat`. Side 1 has no `MTorpedoBoat`. The eight flak ships' `nearest` values of
1938-3120 m are not aircraft: `California`'s 1938 m is `No19`, named in the per-gun table -
`California platform 1, cat 4, range 3000, 13 assigns, 2 shots, first_shot 0.70, target No19` -
and `MSubmarine` is absent from the flak row, so `gunnery_rank(5, 08h) == 0` refuses it at
`0086285F` and again at `00863990` step 3.

**(b) The classes it does rank are out of reach.** No aircraft comes within 2960 m of any unit.
`Enterprise`'s isolated category-5 range is 2000 m; the largest any flak device in the game can
reach is 2400 m (bullet `39`, `FlyTime 3.0 * V0 800`; `14`/`44`/`49` give 2000, `19` is
`Artillery` with an authored 1500, `5` is the AA rocket at 1000). **2960 m against 2000 m, and
against a game-wide ceiling of 2400 m.**

`Enterprise`'s `range` of 2000 shows category 5 *did* score a ranked candidate and was refused on
distance, so this is a positive observation and not an absence.

**Flak's zero is a consequence of the aircraft never taking off**, the condition
`docs/PLANE_UNIT_TICK.md` owns - exactly as `docs/AA_VERTICAL_WINDOW.md` found for
AAMACHINEGUN, and for the same reason. It is not a defect in the flak path, and the flak path is
not exercised at all on this mission.

### CATAPULT, category `0Bh`: faithful zero, and a catapult is not a weapon

The row at `00E0A374` is zero in all `61h` dwords, so `00727BD0` writes no rank for it and
`gunnery_rank(0Bh, classId) == 0` for every class id in `0..60h`. The category is still swept -
its enable byte and mask keep the constructor's `1` and `3`, and no setter list contains `0Bh` -
but `00862820` refuses at `0086285F` and `00863990` refuses at step 3 on the first candidate,
every tick. The `2000` in its `no_window` column is `4 guns * 500 ticks` of
`gun_set_target_angles_0085aba0` refusals with no target, the tally
`docs/AA_VERTICAL_WINDOW.md` proved carries no information about targets.

The data agrees that this is deliberate. Of the 422 device records with a `Function` in
`classtables/arcade/deviceclasses.lua`, **all 20 with `Function = "CATAPULT"` have no `Bullet`
key of any kind**; every one of the other 402 has one. A catapult authors `Type = "Catapult"`,
`PlaneStartPosY`, `PlaneStartPosZ` and `ReloadTime` instead. With no bullet there is no
`006E9890` derivation and no projectile: it is an aircraft launch device that the category index
carries because the index *is* the device's `Function`, exactly as category 0 `PLANEGUN` carries
the pilot's fixed battery. **The gunnery pass should never assign it, and it never does.**

## 7. The one real divergence, and its prescription

`src/game_hosts_gunnery.cpp:822` implements `00862820` as
`bsp::target_is_engageable_00862820(liveness)`, which carries only the four liveness bytes. The
**category-7 arm and the whole submarine arm are missing**, so the host neither refuses a
surfaced submarine to a depth charge nor refuses a submerged submarine to a gun. This is already
recorded as a correction by `docs/TORPEDO_CATEGORY_ADMISSION.md` §7; this packet confirms it from
the listing and adds the sign convention for `00852820`.

It changes nothing on `IJN01` - the range gate refuses category 8 anyway, and categories 4 and 6
would need the submarines to be shallow, which is the state the mission plainly intends at Pearl
Harbor - but it would change a mission with a submerged submarine.

**Prescription** (the integrator owns the host; this packet publishes no source):

1. Give the gunnery host a `bool target_is_shallow_00852820(std::size_t unit)` query on the unit
   binding, answering `[sub+100h] > ([sub+1204h] + [sub+1200h]) / 3.0f` from
   `PeriscopeDepth`/`SwimDepth2` and the unit's vertical coordinate. A unit that is not
   `IsKindOf(8)` never reaches it.
2. In `score_candidate_00863990`, after the liveness test and before the rank test, add
   `00862820`'s submarine arm: for `category == 8 || category == 9` refuse when the query is
   true; for any other `category != 7` refuse when it is false.
3. Add `00862820`'s category-7 arm at the same point: require `IsKindOf(6)` and reject
   `IsKindOf(0Eh)`.
4. Add the `[this + cat*61h + classId + 0CCh]` permission byte as an all-`1` array with a
   `SetWeaponDirectorTargetEnable` setter, or record it as deliberately omitted because no
   mission script in this installation writes it.

## 8. Proven, assumed, and not established

**Proven.**

- The three rows' full `61h` entries and what each admits, from the bytes.
- `00727BD0`'s rule, from the listing, including that a zero entry is skipped rather than final.
- Categories 0, `0Ah` and `0Bh` are in none of the four mask category lists, and the constructor
  seeds every category's enable byte to `1` and every mask word to `3`.
- The four mask setters' lists and bit operations, previously recorded as unread.
- `00862820`'s complete body, including the category-8/9 submarine arm.
- The direction of `00852820`, from its two siblings' consumers and from the class reader that
  writes `PeriscopeDepth` into the field it compares against.
- `00861C20`'s single call site, its Lua binding name, and that no script in this installation
  calls it.
- That every `CATAPULT` device authors no bullet, and every `DEPTHCHARGE` device is a
  `Single_Turning_Gun` with one.
- The complete unit sets for categories 5, 8 and `0Bh` on `IJN01` (60, 10 and 4 guns, all
  accounted for) and their `nearest` distances.

**Assumed or derived rather than observed.**

- The category-8 engagement range of 240 m on `Cassin` is **derived**: from
  `006E9890`'s depth-charge arm (`docs/BULLET_ENGAGEMENT_RANGE.md`'s contract), from device class
  `54`'s bullet class `54`, and from that bullet's `Type = "Depthcharge"`. The run does not print
  a per-category range, and 240 is below `Cassin`'s `best_range` of 1500, so it cannot be read
  off the log. `Enterprise`'s 2000 for category 5 **is** read off the log.
- The identification of `Cassin`/`Downes` with `VehicleClass[309]` `Mahan 1941` is from the run's
  `type_id=309` line; the `Mona` and `PT` classes were not looked up, so their category-8 bullet
  class is assumed to be one of the four real depth-charge classes. All four give 240, and all
  four units are over 3.2 km from anything, so the verdict does not turn on it.

**Not established.**

- **Whether `No18` and `No19` are above or below the depth-charge threshold on `IJN01`.** The
  host models no submarine depth, `ForceSubmarinePeriscope` is commented out in every mission
  script in the installation, and `ijn_1_pearl.lua` never calls it. The depth-charge verdict was
  written to hold either way; the *reason* the native build refuses is not.
- The absolute sign of `[sub+100h]`, `[sub+1200h]` and `[sub+1204h]`. The copy loop at
  `00853A87` writes `entity+1200h` verbatim from `class+80Ch` but writes the next three as
  `XMM1 - classValue` when the class value is non-negative (`00853A9F  COMISS XMM2,XMM0 / JC`);
  `XMM1`'s producer and `class+80Ch`'s reader were not chased. The *relative* reading - three
  routines comparing the same coordinate against surfaced, periscope and averaged references - is
  what the verdict uses, and that is complete.
- A fresh 500-tick run. See section 5; the session's build faults in FMOD startup.

## 9. Corrections to earlier documents

| document | was | is |
| --- | --- | --- |
| `docs/GAME_EXECUTABLE.md`, the twelve-category table | "Categories 1, 5, 8 and 11 ... list plane classes, air classes, the submarine and nothing at all. **No plane and no submarine exists on this mission**, so `00862820` step 3 refuses every candidate on a zero rank." | That was a different mission. On `IJN01` **two submarines do exist and are engaged** by categories 4 and 6; category 8's zero is the 240 m range, not a zero rank. Category 8's row ranks `MSubmarine` first. The statement stands only for category 11 |
| `docs/DIRECTOR_UPDATE_ARMS.md`, §"coverage" | "`008624C0` complete as a decision table; **the four setters `00861CD0`/`00861D20`/`00861D70`/`00861DC0` and the panel record are unread**" | stale. The ledger records from `cc2_unit_gunnery_pass` already carry each setter's list and bit operation; the coverage table was never updated. New here: only the depth-charge call site lacks the `refreshAll` bypass, and the constructor's all-`3` seed means the AA setter's single bit can never make a flak mask refuse a non-plane |
| `config/names` record for `00852820` (packet `cc2_unit_gunnery_pass`) | "`00862820` **requires it** before letting the depth-charge categories 8 and 9 take a target answering `IsKindOf(8)`" | **inverted**. `008628AE  JZ 008628C7` sends a **zero** answer to the `MOV AL,1` return and falls a non-zero answer through to `008628B0  XOR AL,AL / RET 8`. Categories 8 and 9 require `00852820` to be **false**; the opposite arm at `008628C5` is the one that requires it true, for every category except 7. A refutation is appended to the record |
| `docs/GUNNERY_TABLES.md`, §1 | the rows are quoted by their leading entries | all `61h` entries of rows 5, 8, 9, `0Ah` and `0Bh` were read; the quoted heads are the whole content |
| the packet brief | the installation's modded files are `vehicleclasses.lua` and `bulletclasses.lua` | `classtables/arcade/deviceclasses.lua` is modded too (`2026-05-09 22:37`), alongside `classtables/arcade/bulletclasses.lua` (`2026-05-09 23:04`) and `autoload/vehicleclasses.lua` (`2026-05-09 21:52`). `classtables/realistic/bulletclasses.lua` is `2025-06-02`; the three `realistic` others and both `reconclasses.lua` are the untouched `2024-07-13`. This installation, not retail |

## 10. Follow-up packets

1. **`submarine_depth_state`** - `[sub+100h]`, `[sub+1200h]`..`[sub+120Ch]`, `00853A68`-`00853AC5`
   and `XMM1`'s producer. Settles the absolute sign convention and lets a host answer
   `00852820`, which turns this packet's either-way depth-charge verdict into a single one and
   makes `00862820`'s submarine arm implementable.
2. **`weapon_director_target_enable`** - `00861C20`, `008945F0`, `00E0BF54` and the rest of the
   `luaMW_` binding family. The `[gunneryAi+0CCh]` table is a real per-(category, class)
   permission surface that no mission in this installation uses; the retail scripts may.
3. **`depth_charge_launcher_category_9`** - `00E0A06C`, the single `DEPTHCHARGELAUNCHER` device
   and bullet class `2` (`Hedgehog Projector Depth Charge`, `V0 50`, `FlyTime 20`, still the
   240 m constant arm). No unit on `IJN01` carries one, so category 9 is untested.
4. **`catapult_launch_device`** - what a `Function = "CATAPULT"` device actually does with
   `PlaneStartPosY`/`PlaneStartPosZ`/`ReloadTime`, given that it holds a per-category gun record
   and a gun tick but no bullet.
5. **A mission with a submerged submarine** - the only measurement that can exercise category 8
   at all. `IJN01` cannot, at any tick count.
