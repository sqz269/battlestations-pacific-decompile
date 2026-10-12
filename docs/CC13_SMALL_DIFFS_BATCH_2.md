# Small host-fidelity diffs, batch 2 (packet cc13-smalldiffs1)

Three small divergences between the image and cc9's units host, each written as a unified diff
behind its own switch, default OFF. Nothing here is applied to `src/`: the diffs are cc9's to land.
The listing reads below come from disk bytes (`bsp.py disasm-raw`) and the live Ghidra call graph.
Ghidra was not modified.

Status of every diff:
- **Exported:** yes.
- **Reconstructed:** yes.
- **Build-tested:** yes, with the switch OFF. Each diff was applied, built with
  `./scripts/build.ps1` (exit 0, `bsp_game.exe` linked, 2/2 tests, no warning in
  `game_hosts_units`), then reverted.
- **Stacked:** all three apply together and build (exit 0, 2/2 tests).
- **Checked:** each passes `git apply --check` against origin/main `6f44c8a0b` and again at `06dd3fec6`. The three host
  files are byte-identical to the base `07b694ce8` where the builds ran.
- **Not done:** no diff has been pair-tested or game-validated.

The diffs also apply and build stacked on the two unlanded cadence diffs in
`docs/CC13_TASK_PROFILE_CADENCE.md`, sections 6 and 10, in either order (exit 0, 2/2 tests).

| item | switch | files | image sites |
| --- | --- | --- | --- |
| 1 | `kMoveToLuaAltZeroOffsetBound` | `include/bsp/game_hosts_units.hpp`, `src/game_hosts_units.cpp` | `009BEBAA`-`009BEBB7` |
| 2 | `kStrafeDoubleConstantsBound` | `include/bsp/strafe_task.hpp`, `src/game_hosts_units.cpp` | `009CB87E`, `009CB6F8`, `009CAB7B` |
| 3 | `kStrafeAttackrunSeedDrawBound` | `include/bsp/strafe_task.hpp`, `src/game_hosts_units.cpp` | `009CACDA` |

`reports/cc13_small_diffs_batch_2_calls.json` lists the call sites and instructions cited here.
`tools/verify_report_calls.py` checks it: 7 call rows, 0 failed.

## 1. Moveto: the altitude offset is zeroed while a Lua altitude is live

### 1.1 The image

Read from the listing:

```
009BEBA7  mov eax, [esi+0Ch]            ; the squadron block (ctl)
009BEBAA  cmp byte [eax+3ADh], 0
009BEBB2  jne 009BEBBC
009BEBB4  xorps xmm0, xmm0
009BEBB7  movss [esi+2Ch], xmm0          ; approach+2Ch = 0, a store
...
009BEBF1  fld  [ecx+394h]                ; ctl+394h, the travel altitude
009BEBFA  fsub [esi+2Ch]
009BEBFD  fstp [esi+4Ch]                 ; steer point y
```

The zeroing is a store and not a local. So once a refresh sees `+3ADh == 0`, the task's offset
stays 0 until something redraws `+2Ch`. The only redraws are construction draw 1 (`009C1C8F`) and
activation draw 7 (`009C36A3`); see cadence section 10.

These are the writers of the byte `+3ADh`. The census used two scans:
- `scan-bytes 'C6 ?? AD 03 00 00'` and `'88 ?? AD 03 00 00'` found every literal-displacement store.
- The sub-object stores were found by reading the listing: they go through `esi = block+37Ch`,
  which makes them `[esi+31h]`.

| site | function | value | meaning |
| --- | --- | --- | --- |
| `007F2C1A` | block init `007F2BD0` (`[esi+31h]`, `esi` = block+37Ch) | 1 | no script altitude |
| `0089F729` | `0089F550` `SquadronSetTravelAlt` | 0 | script altitude live |
| `008A2489` | `008A22B0` `SquadronSetAttackAlt` | 0 | script altitude live |
| `008A26B9` | `008A24E0` `SquadronSetReleaseAlt` | 0 | script altitude live |
| `009C375A`, `009C3906` | moveto profile `009C3650`, both arms (`[esi+31h]`) | 1 | written with `+394h` |
| `009C89D6` | dive profile `009C8920` | 1 | written with `+398h` (`009C89CE`) |

Some details of the moveto profile `009C3650`:
- It refreshes first: `009C36BF CALL 009BEBA0`.
- Only then does it run the leader gate `009C36CA CALL 007B8AD0` (`JE 009C3947`).
- The `+394h` / `+3ADh` writes sit behind that gate and behind the `+38Dh` freeze, the
  `+380h < 0` countdown and the `+3A9h` lock tests (`009C372E`-`009C3747`).

So a non-leader's activation never sets the byte. Its own refresh reads the byte before any write.

Coverage of the census:
- It is complete for literal `+3ADh` displacements.
- Base-plus-`31h` stores were read only in `007F2BD0` and `009C3650`.
- A third writer through another base register was not swept. So the census is
  `coverage: partial` for those.

### 1.2 The host now

- `moveto_refresh_009beba0` (`src/game_hosts_units.cpp`) steers at `cruise - moveto_alt_offset_2c`
  every time and never zeroes the offset.
- The host splits `+3ADh` into two booleans, `sq_alt_set` and `sq_attack_alt_set`. Each tracks its
  own block's override, and neither stands for the shared byte.
- `SquadronSetReleaseAlt` (`008A24E0`) is listed in `src/mission_lua_host.cpp` but not bound in the
  units host, so the host cannot clear the byte through it.

### 1.3 The diff

- Adds `GameUnitSlot::sq_lua_alt_3ad`. It is true while the byte is 0.
- Both bound natives set the field.
- The dive profile's open write (the host's stand-in for `009C89D6`) clears it.
- The install clears it after its refresh (`009C1D9D`) when the flight leader's stand-in profile
  call wrote `+394h`. That holds whenever `sq_alt_set` is clear after the gate.
- With the switch ON, `moveto_refresh_009beba0` zeroes `moveto_alt_offset_2c` at its head while
  the squadron's byte is live, as `009BEBAA` does.
- With the switch OFF, nothing reads the new field.

What is LABELLED:
- The install stands for the activation, as in cadence section 5.
- The byte write is taken after the install's refresh. This keeps the image's order inside
  `009C3650`: refresh, then leader gate, then write.
- A plane with no squadron slot has no block, so nothing is zeroed.

### 1.4 Expected effect

- This applies to any moveto task in a squadron that had `SquadronSetTravelAlt` or
  `SquadronSetAttackAlt` called and no profile write since.
- Its steer altitude rises by the drawn offset: `U(0, MaxAltOffset = 5)` with draw 1, or
  `U(0, TravelAltRandom = 50)` with cadence section 10 ON. The offset is zeroed for the rest of
  that task.
- The squadron then cruises exactly at the script altitude instead of up to 5 m (or 50 m) below it.
- The diff draws no random numbers.

### 1.5 Missions that would move

The natives are called from `scripts/global/commandhelpers.lua` (`luaAirfieldManager`, lines
8030-8043). Each altitude is set twice there: once with travel and once with attack.

Script counts of `SquadronSet(Travel|Attack|Release)Alt` among the reference rows, in this
installation:

| mission script | calls |
| --- | --- |
| USN01 `usn_01_battle_of_eastern_solomons.lua` | 17 |
| USN04 `usn_04_defend_guadalcanal.lua` | 15 |
| USN12 `usn_12_indochina.lua` | 21 |
| BSM01 | 7 |
| IJN01 `ijn/ijn_1_pearl.lua` | 6 |
| USN13 | 2 |
| JM06 | 1 |
| JM05 | 0, but it calls `luaAirfieldManager` |
| USN02 | 0 |
| JM08 | 0 |

Good pair candidates are USN01, USN04 and USN12. JM08 and USN02 are controls.

## 2. Strafe: three double constants loaded as float values

### 2.1 The image

Each constant is a qword load. Bytes are from the image, decoded with Python `struct`.

| site | instruction | constant | bytes | value | strafe state |
| --- | --- | --- | --- | --- | --- |
| `009CB87E` | `FMUL qword [00D05858]` | 20 deg | `00 00 00 60 18 57 D6 3F` | 0.3490658700466156 | goaway bank re-plan `009CB780` |
| `009CB6F8` | `FADD qword [00D05850]` | 10 deg | `00 00 00 60 18 57 C6 3F` | 0.1745329350233078 | goaway pitch re-plan `009CB650` |
| `009CAB7B` | `FLD qword [00D057D8]`, `FCOMI` | 35 deg | `AB AA AA 4A 35 8C E3 3F` | 0.6108652551968893 | gotowards tick `009CA870` |

`docs/CC13_ROCKET_TASK.md` section 8 says all three have a `60000000h` low mantissa word. That holds
only for the first two. `00D057D8` has the low word `4AAAAAABh`, so it is a computed double. It is
still not the host's value.

In this image the three addresses are also read by:
- the rocket twins `007B5BB2`, `007B5A69` and `007B4C8D`, whose reconstruction already uses the
  image values;
- `009BA109` (tightturn) and `009EDF14` (ship AI controls), which are outside this diff.

### 2.2 The host now

| host function | literal |
| --- | --- |
| `st_goaway_heading_009cb780` | `0.3490658402442932` |
| `st_goaway_pitch_009cb650` | `0.1745329201221466` |
| `st_gotowards_tick_009ca870` | `0.6108652353286743` |

Each literal is the float nearest the angle, widened to double.

`tools/const_width_sweep.py --all --load-sites` checked 1072 constants and found 135 mismatches:
- 0 A-WRONG;
- 1 B (`kPilotPitchHalfRange`, not in the strafe code).

The sweep reads only header constants. These three are inline literals in the `.cpp`, so it does
not cover them. The width check above was done by hand.

### 2.3 The diff

`kStrafeDoubleConstantsBound` (in `strafe_task.hpp`, OFF) chooses each literal between the old
value and the image's. With the switch OFF, the arithmetic is unchanged.

### 2.4 Expected effect

- **Goaway bank** `+1Ch`: `U(r-1, r+1) * C * -1` is rounded to float. C grows by 8.5e-8
  relative, which is more than half a float ULP. So most drawn banks move by one ULP.
- **Goaway pitch cap**: `p + C` grows by 1.5e-8. A one-ULP change happens only near a rounding
  boundary.
- **Gotowards alignment** `+20h`: the result flips only when `|err|` is exactly the float
  0.61086524.

All three changes are sub-ULP to one-ULP. They can still grow through the flight integration
over a long strafe.

Missions that would move: those that reach the strafe task. SQUADRON_LAND_TASK section 5bv
records USNOS and ESMP08 as reaching it. `ijn_1_pearl.lua`'s "Strafe" objective is the player's,
and that census records no AI strafe order there. cc13-strafe2's IJN01 audit may say otherwise.

## 3. Strafe attackrun: the `+1Ch` countdown seed

### 3.1 The image

`009CAC80` is the strafe attackrun constructor (vtable `00D21004`). Its only caller is the state
builder `009CC020`, which builds the states in this order:
1. goaway (`009CC154 CALL 009CB500`);
2. attackrun (`009CC165 CALL 009CAC80`).

What the constructor writes:

```
009CACB4  movss xmm0, [00D7A24C]       ; 1.0
009CACC9  fldz / 009CACCE fstp [esp]   ; low 0.0, high 1.0 (009CACB2 fld1)
009CACCB  lea ecx, [eax+1]             ; ECX = 1, stream 1
009CACD5  movss [esi+18h], xmm0        ; +18h = 1.0
009CACDA  call 00BD2F10
009CACDF  fchs
009CACE4  fstp [esi+1Ch]               ; +1Ch = -U(0, 1)
009CACEB  movss [esi+20h], 0
```

The enter `009CAD90` (vt[4]) writes only `+20h = 0` and `+18h = [00CE7804]` (0.4). Nothing else
in the class rewrites `+1Ch` except the tick's own countdown:
- `009CAF52` subtracts dt;
- the period add-back adds `+18h - dt`.

So the seed is drawn once per task install and carries across re-entries into attackrun.

What `+1Ch` feeds: the tick `009CADB0` fires its near-field probe (`007F0280` mode 0, box 100 / 60
/ 120) whenever `dt >= +1Ch`. That probe sets the heading offset `+20h` (`009CAE4D`-`009CAECE`). A
negative seed makes the first attackrun fire on several consecutive ticks, about one extra fire
per 0.4 s of seed magnitude, until the countdown is positive. A seed of 0 fires once.

### 3.2 The host now

- `st_install_009cc230` sets `st_ar_countdown_1c = 0.0f`.
- It also sets `st_ar_period_18 = 0.4f`, where the image constructor stores 1.0. That difference
  is harmless, because the enter always writes 0.4 before the first tick, and the host's enter
  mirror does the same.

### 3.3 The diff

`kStrafeAttackrunSeedDrawBound` (in `strafe_task.hpp`, OFF):
- With it ON, the install draws `-U(0, 1)` through `st_draw("ar1c", ...)`, which is key
  `<unit>#star1c`.
- The draw comes right after goaway's `ga18` draw, which is the image's construction order.

How the draw relates to the RNG streams:
- It never touches the gameplay `00BD2F10` stream 0.
- With `BSP_GUNNERY_RNG_STREAMS=1` it gets its own keyed generator.
- With the variable unset, `st_draw` falls back to the host's shared release generator
  `db_release_rng`, like every other strafe draw. One extra draw then shifts every later release
  draw. Pairs should set the variable.

### 3.4 Expected effect

- The first attackrun of each strafe task re-probes its squadron neighbours on the ticks covered
  by `|seed| / 0.4 s`, instead of once.
- So `+20h` (the weave offset added to the heading) tracks the neighbours from the first ticks.
- Later attackruns of the same task are unchanged once the countdown has gone positive.

Missions that would move: those that reach strafe, as in section 2.4 (USNOS, ESMP08).

## 4. Overlap with cc13-strafe2 (re-attack audit, not landed)

- At the time of writing, cc13-strafe2's tree has an uncommitted edit only in
  `src/game_hosts_mission_frame.cpp`. `docs/CC13_AUDIT_REATTACK_AND_SKILL.md` does not exist yet.
- Items 2 and 3 touch:
  - `st_install_009cc230` (one line: the `+1Ch` seed);
  - `st_goaway_heading_009cb780` and `st_goaway_pitch_009cb650` (the constant only);
  - `st_gotowards_tick_009ca870` (the constant only);
  - two new switches in `strafe_task.hpp`.
- None touches the state transitions, the re-attack test, the attackrun tick or the enter mirror
  (`009CAD90`).
- Possible textual overlap: a strafe2 hunk that edits `st_install_009cc230` near the attackrun
  fields, or the two switch spots in `strafe_task.hpp`.
- Possible behavioural overlap: item 3 changes how often the first attackrun probes. A re-attack
  measurement taken with item 3 ON is a different baseline.

## 5. Diffs

### 5.1 `kMoveToLuaAltZeroOffsetBound`

```diff
diff --git a/include/bsp/game_hosts_units.hpp b/include/bsp/game_hosts_units.hpp
index 44f37c5cd..b9fadad95 100644
--- a/include/bsp/game_hosts_units.hpp
+++ b/include/bsp/game_hosts_units.hpp
@@ -455,6 +455,13 @@ inline constexpr bool kSquadronTravelAltBound = true;  // ON: pairs held (docs/L
 inline constexpr bool kPlaneRowPositionBound = true;  // ON: mechanism held, one premise miss (docs/CONTROLLED_UNIT.md)
 
 inline constexpr bool kSubmarineDiveTeleportBound = true;  // ON: pairs held (docs/SUBMARINE_MODEL.md section 15)
+// Packet cc13-smalldiffs1 (docs/CC13_SMALL_DIFFS_BATCH_2.md item 1). The moveto
+// refresh 009BEBA0 zeroes approach+2Ch (009BEBAA-009BEBB7) while the squadron
+// byte +3ADh is 0, which the Lua altitude natives 0089F550 / 008A22B0 /
+// 008A24E0 store and the profile writes 009C375A / 009C3906 / 009C89D6 undo.
+// True: the steer altitude drops the random offset while a script altitude is
+// live. False: cruise - offset always.
+inline constexpr bool kMoveToLuaAltZeroOffsetBound = false;  // OFF: not pair-tested
 
 class GameUnitsHost {
 public:
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index f8634b115..3dbfa6d68 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -1741,6 +1741,10 @@ struct GameUnitSlot {
     float sq_alt_398{0.0f};
     bool sq_freeze_38c{false};
     bool sq_lock_3aa{false};
+    // The one byte +3ADh both natives clear: true while it is 0 (a script
+    // altitude live). 007F2C1A sets it to 1 at block init; the moveto and dive
+    // profiles set it to 1 with their +394h / +398h writes.
+    bool sq_lua_alt_3ad{false};
     double sq_timer_expiry_37c{-1.0};
     unsigned long long sq_attack_alt_reads{0};
     float sq_last_attack_alt{0.0f};
@@ -3940,6 +3944,7 @@ struct GameUnitsHost::Impl {
                 if (open) {
                     block.sq_profile_398 = value;               // 009C89CE
                     block.sq_profile_398_valid = true;
+                    block.sq_lua_alt_3ad = false;               // 009C89D6 +3ADh = 1
                     ++block.sq_profile_writes;
                 }
             }
@@ -15855,6 +15860,7 @@ bool GameUnitsHost::set_squadron_attack_alt_008a22b0(std::size_t unit_index, flo
     sq->sq_alt_398 = altitude;                                        // +398h
     sq->sq_lock_3aa = true;                                           // +3AAh = 1
     sq->sq_attack_alt_set = true;                                     // +3ADh = 0
+    sq->sq_lua_alt_3ad = true;                                        // 008A2489
     host.done("PlaneSquadron::set_attack_alt_008a22b0", 0x008a22b0u);
     return true;
 }
@@ -15869,6 +15875,7 @@ bool GameUnitsHost::set_squadron_travel_alt_0089f550(std::size_t unit_index, flo
     sq->sq_alt_394 = altitude;                                        // +394h
     sq->sq_lock_3a9 = true;                                           // +3A9h = 1
     sq->sq_alt_set = true;                                            // +3ADh = 0
+    sq->sq_lua_alt_3ad = true;                                        // 0089F729
     sq->sq_ever_set = true;
     host.done("PlaneSquadron::set_travel_alt_0089f550", 0x0089f550u);
     return true;
@@ -26372,6 +26379,19 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 n + "#mt54c", 0.0f, 1.0f);
                             unit_.moveto_dwell_60 = 0.0f;       // 009C1CFD
                             moveto_refresh_009beba0();          // 009C1D9D
+                            if constexpr (kMoveToLuaAltZeroOffsetBound) {
+                                // The activation profile 009C3650 refreshes first
+                                // (009C36BF), then, for the flight leader only
+                                // (009C36CA 007B8AD0), stores +3ADh = 1 with its
+                                // +394h write (009C375A / 009C3906). LABELLED: the
+                                // refresh above stands for that call, and its gate
+                                // leaves sq_alt_set clear exactly when it writes.
+                                GameUnitSlot* blk =
+                                    owner_.squadron_slot_of(unit_.process_index);
+                                if (leader && blk != nullptr && !blk->sq_alt_set) {
+                                    blk->sq_lua_alt_3ad = false;
+                                }
+                            }
                             if (leader) {
                                 // 009C1D32-009C1D9A: min(planar distance to the
                                 // target, TurnCircleRadius) / TravelSpeed.
@@ -26409,6 +26429,15 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                     // 009C3650's per-call redraw of +2Ch is not. 009FD050 and the
                     // +28h path leg (009BD400 / 009FC260) are records.
                     void moveto_refresh_009beba0() {
+                        if constexpr (kMoveToLuaAltZeroOffsetBound) {
+                            // 009BEBAA-009BEBB7: +2Ch = 0 while ctl+3ADh == 0. It
+                            // is a store, so the offset stays 0 for this task.
+                            const GameUnitSlot* blk =
+                                owner_.squadron_slot_of(unit_.process_index);
+                            if (blk != nullptr && blk->sq_lua_alt_3ad) {
+                                unit_.moveto_alt_offset_2c = 0.0f;
+                            }
+                        }
                         if (unit_.moveto_target_plus_one != 0 &&
                             unit_.moveto_target_plus_one - 1 < owner_.slots.size()) {
                             const GameUnitSlot& t =
```

### 5.2 `kStrafeDoubleConstantsBound`

```diff
diff --git a/include/bsp/strafe_task.hpp b/include/bsp/strafe_task.hpp
index 1a9968f7f..c1117b97a 100644
--- a/include/bsp/strafe_task.hpp
+++ b/include/bsp/strafe_task.hpp
@@ -102,6 +102,13 @@ void strafe_approach_update_009cced0(StrafeApproachUpdate& ap, float dt,
 // runs and resumes when it ends. False: the gate is only counted.
 inline constexpr bool kStrafeBreakoffBound = true;   // ON: SQUADRON_LAND_TASK 5cm.1
 
+// Packet cc13-smalldiffs1 (docs/CC13_SMALL_DIFFS_BATCH_2.md item 2). The goaway
+// bank 009CB87E (FMUL qword [00D05858]), the goaway pitch cap 009CB6F8 (FADD
+// qword [00D05850]) and the gotowards alignment test 009CAB7B (FLD qword
+// [00D057D8]) load doubles that are not a widened float. True: the image's
+// doubles. False: the float values the host used before.
+inline constexpr bool kStrafeDoubleConstantsBound = false;   // OFF: not pair-tested
+
 // 009BAFC0's pitch reference for tightturn (009BB018-009BB12B, read from the
 // listing). altitude = unit+100h, speed = unit vtable[38h], pitch = unit+C64h,
 // climb_angle = desc+1ECh. Below 250 m the floor +400h is the climb angle;
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index f8634b115..3a18947f5 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -20988,7 +20988,8 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         // [00D7A250] (-1.0): a bank away from the target side.
                         unit_.st_ga_bank_1c = static_cast<float>(
                             static_cast<double>(st_draw("ga1c", r - 1.0f, r + 1.0f)) *
-                            0.3490658402442932 * -1.0);
+                            (bsp::kStrafeDoubleConstantsBound ? 0.3490658700466156
+                                                              : 0.3490658402442932) * -1.0);
                     }
                     // 009CB650 (009CB650-009CB77E): the goaway pitch re-plan.
                     void st_goaway_pitch_009cb650() {
@@ -21017,7 +21018,9 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                             }
                             const float lo = 0.0f < p ? 0.0f : p;
                             float hi = unit_.plane_climb_angle_1ec;
-                            const float ph = static_cast<float>(p + 0.1745329201221466);  // [00D05850]
+                            const float ph = static_cast<float>(p +                     // [00D05850]
+                                (bsp::kStrafeDoubleConstantsBound ? 0.1745329350233078
+                                                                  : 0.1745329201221466));
                             if (ph < hi) hi = ph;
                             unit_.st_ga_pitch_20 = st_draw("ga20", lo, hi);
                             return;
@@ -21183,7 +21186,9 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         // 009CAB1B-009CAB94: +20h when |wrap(h - heading)| < 35 deg.
                         const float err = bsp::wrapped_angle_subtract_00438b10(
                             h, unit_.plane_heading_c6c);
-                        unit_.st_gt_aligned_20 = std::fabs(err) < 0.6108652353286743;
+                        unit_.st_gt_aligned_20 = std::fabs(err) <                    // [00D057D8]
+                            (bsp::kStrafeDoubleConstantsBound ? 0.6108652551968893
+                                                              : 0.6108652353286743);
                         // 009CAB90-009CAC2D: the glide-limited height above the aim
                         // point, then 009F9ED0(-that, +1Ch).
                         float yp = -dy;
```

### 5.3 `kStrafeAttackrunSeedDrawBound`

```diff
diff --git a/include/bsp/strafe_task.hpp b/include/bsp/strafe_task.hpp
index 1a9968f7f..a219d9a0c 100644
--- a/include/bsp/strafe_task.hpp
+++ b/include/bsp/strafe_task.hpp
@@ -36,6 +36,12 @@ enum class StrafeState : int {
 // goaway and script dogfights are unmeasured (5cb).
 inline constexpr bool kStrafeTaskBound = true;   // ON: SQUADRON_LAND_TASK 5cb
 
+// Packet cc13-smalldiffs1 (docs/CC13_SMALL_DIFFS_BATCH_2.md item 3). The
+// attackrun constructor 009CAC80 seeds its countdown +1Ch = -U(0, 1)
+// (009CACDA, stream 1); the enter 009CAD90 resets only +18h and +20h. True: the
+// install draws it on the "#star1c" key. False: +1Ch starts at 0.
+inline constexpr bool kStrafeAttackrunSeedDrawBound = false;   // OFF: not pair-tested
+
 // 007EEC50's guns inputs in the PilotSetTarget choice (src/
 // game_hosts_script_orders.cpp): guns_available = PilotFires (plane+C24h,
 // 007EEB08 / 007EEBB7) and guns_suppressed = 0047B850 (kind 10h or 16h,
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index f8634b115..68a792c40 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -20757,7 +20757,10 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         // asks the head's vt[40h] 0099C2C0, not read).
                         unit_.st_mv_kind = 0;
                         unit_.st_ar_period_18 = 0.4f;
-                        unit_.st_ar_countdown_1c = 0.0f;
+                        // 009CACDA: -U(0, 1), after goaway's draws (009CC154,
+                        // then 009CC165 builds the attackrun).
+                        unit_.st_ar_countdown_1c = bsp::kStrafeAttackrunSeedDrawBound
+                            ? -st_draw("ar1c", 0.0f, 1.0f) : 0.0f;
                         unit_.st_ar_offset_20 = 0.0f;
                         // 008A4C41 -> 007ED430(2) at the order; the leader's first
                         // think lowers it to 1 (as db_attack_mode_370 is seeded).
```

## 6. Open items

- **Base-plus-`31h` writers.** A sweep for `[reg+31h], imm8` stores whose base is a squadron
  `+37Ch` pointer would close the `+3ADh` census.
- **`SquadronSetReleaseAlt` (`008A24E0`).** It is not bound in the units host. When it is bound,
  it should set `sq_lua_alt_3ad` (`008A26B9`).
- **Profile cadence.** Item 1 takes the install as the activation. If cadence section 6 lands ON,
  the leader's gate runs only then, which this diff already assumes.
- **Pairs.**
  - Item 1: USN01, USN04 and USN12 with OFF/ON.
  - Items 2 and 3: USNOS and ESMP08, with `BSP_GUNNERY_RNG_STREAMS=1` for item 3.
