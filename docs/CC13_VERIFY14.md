# CC13_VERIFY14: objective-set membership (O1, O4) and the plane-file leftovers

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-objsets1 on 2026-10-17 against
base fad4dd8b0. No code was changed and no ledger records were added.

Part 1 settles the two UNRESOLVED objective-set items in docs/CC13_VERIFY13.md (O1, O4) and adds one
new item (O5). Part 2 covers the plane files that docs/CC13_VERIFY10.md "Not covered" checked only for
labels.

Line numbers are against the base. "Image" means battlestationspacific.exe, read with
`bsp.py disasm-raw` (Capstone over the PE on disk) and `ghidra bytes`. Mission scripts are this
installation's `scripts/missions` tree (a modded install), scanned with `local/objsets1_scan.py`. That
scan strips `--` line comments and non-nesting `--[[ ]]` block comments the way Lua does.

Items already in earlier passes are not repeated. cc13-verify12 is reporting the game_hosts_units.cpp
callers at the same time:
- the mode-1 roll constants (the 0099E2BA-0099E39D arm with scale 1, band 0 and limit 1);
- the +2B8h divisor;
- the 007C0F40 bomb-load factor.

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, NaN-only arms, width or rounding with no decision effect,
  presentation, or RNG draw values only.
- MISMATCH REAL: a wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL.
- UNRESOLVED.

## Summary

No MISMATCH REAL was found. Nothing here can change whether a mission completes. The reasons:
- The mission Lua never reads the native objective sets. The scripts call only `Objectives_Add`,
  `Objectives_AddUnit`, `Objectives_RemoveUnit`, `Objectives_Completed`, `Objectives_Failed` and
  `Objectives_ClientRefresh` (a grep of `Objectives_[A-Za-z]+` over the scripts tree).
- Completion is decided in Lua on each objective's own `obj.Target` table (commandhelpers.lua
  `luaObj_Add` :5741, `luaObj_AddUnit` :5826).
- The native set is read only through 008DDF90 (`BSP_SzurkeNyil_ContainsUnit`). It tests the set's
  `+18h` tree, size `+20h` (008DDF93).

008DDF90 has three host readers:
- the HUD unit pick (src/hud_warning_screen.cpp:380, gated on `flag_5d` in the same test);
- 00922C80's set arm (ship AI game_hosts_ship_ai.cpp:7887 and script orders
  game_hosts_script_orders.cpp:1582). The image tests `+5Dh` at 00922C8F before the arm, and ships,
  planes, airfields, shipyards and command buildings answer before it;
- the planner, which is inert on every harness row (CC13_VERIFY13).

Part 1 results:

| # | Verdict | Live in missions | Test rows (JM08/USN01/USN04/USN13/LOMP10) |
|---|---|---|---|
| O1 | MISMATCH MINOR | JM03 (three copies); multiplayer competitive05 (static hit, not confirmed) | none |
| O4 | MISMATCH MINOR | every mission with a set member that dies | no gameplay reader sees it |
| O5 (new) | MISMATCH MINOR | about 50 missions with `luaObj_Add("hidden", 1, targets)` | USN13 (KatoriGang, all ships: HUD pick only) |

Part 2: MATCH throughout, apart from the MINOR rounding and latent items in the table there. RNG
00BD2F10: none of the code read here draws.

## Part 1: objective sets

### O1: AddUnit on an objective that does not exist yet

**Image.** 008CDD60 (`Objectives_AddUnit`) dispatches every target to 008DF9B0 (008CE3AE) or 008DF5D0
(008CE42D). The 008DF9B0 path runs:
- 008DF9B0 expands groups;
- its callees are 006D6010, 00770B50, 00848410, 008DC980, 008DD780, 008DF2B0, `_free` and 00BF6713;
- there is no 008E1F80 (`BSP_ObjectiveSet_Add`).

008DF2B0 walks the set's list. Exit and add sequence:
- 008DF2FC `CMP ESI,EBX` / 008DF2FE `JE 008DF3DC` leaves when the name is not found;
- 008DF319 is the 00435C40 compare;
- only a match reaches 008DF358 `CALL 008DEE50`.

So a missing objective gets nothing: no create, no entry, no observer.

**Host.** game_hosts_lua.cpp:742-743 calls `sets.add_unit`. game_hosts_ai.cpp:151 `add_unit` calls
`add_objective` (`:119`), which creates the objective when the walk misses (`:131-136`). A record
created this way also keeps `kind` 0: the `add_objective(slot, name, kind)` overload (`:141`) writes
the kind only when it is the call that created the record.

**Missions.** The static scan found one live case: ijn/JM/jm03.lua and its COTP-IJN and COTP-IJN/PRCPIJN
copies.
- The comment `--[[elseif Mission.MissionPhase == 3 then` (:918) closes at the inner `end]]` (:924),
  because Lua block comments do not nest.
- That leaves `luaObj_Add("primary",3)` (:921) commented out.
- `luaObj_AddUnit("primary",3,v)` (:929) stays live in the phase-2 arm and runs on every check.
- In the image the units go nowhere. In the host they enter a "primary 3" objective in slot 0.
- The units are `USNStageGroup1` ships, so only the HUD pick membership changes.

The multiplayer scans were checked by hand:
- competitive04 and the siege missions were false hits (keys added in a loop, or calls inside block
  comments);
- competitive05 `primary 9` (:671-672) was not checked by hand.

On the test rows, every AddUnit follows its Add:
- JM08 :476 before :478/:481;
- USN13 :1134 before :1138, and :1799/:1807 before :1801/:1813;
- USN01 :1047/:1055 before :1049/:1061;
- USN04 (usn_19_coralus.lua, or COTP-USN/usn_04_defend_guadalcanal.lua) and LOMP10 have no AddUnit.

**Verdict: MISMATCH MINOR.** Presentation (HUD pick) only. RNG: none.

### O4: what removes a dead unit from a set

**Image.** The observer is the SET, not the objective. 008DF330-008DF345 load `EBX = [ESP+10h]` (the
set, saved from ECX at 008DF2D2), `EDX = EBX`, `ECX = EBP` (the unit). Then:
- 008DF338 calls 00694AF0 (is the pair registered);
- 008DF345 calls 00694A60 (register).

The callback is the set's vtable 00D1610C slot 1 = 008DF1E0. It:
- unregisters the pair (00694AF0 / 006952A0);
- calls 008DC3E0 (erase that unit's entries) on every objective in the set's list (`+28h`);
- erases the unit from the `+18h` tree (008DBBA0 / 008DB950 / 008DE7E0).

Slot 1 is invoked only through 00696330 -> 00693550 (`MOV EDX,[EAX+4]` / `CALL EDX`, owner vtable+4).
00696330 has three callers:
- 009273A0's kill-list block. 009274CE-009274DA set `+5Dh`, `+5Eh` and `+5Fh` to 1 and `+5Ch` to 0.
  009274F5-00927510 then call 00696330 when the unit's edge count `+8h` is non-zero. A ship gets
  there when its wreck reaches KillDepth; see docs/CC13_VER_13.md section 1.
- 00925C40, from `BSP_SceneNode_Remove` 009263C0.
- 00A2D440, the AI group destructor.

The unit's own destructor (00925780 -> 00695760 -> 006953C0) only deletes edges and calls no owner.

So in the image a sunk ship stays a set member through its whole wreck descent and leaves at the kill
list. A unit that is never killed or removed stays.

**Host.** There is no observer. A unit leaves only through `remove_unit` (Lua `RemoveUnit`) or
`set_status` (Completed or Failed), so a dead unit stays after its kill list.

**Effect.** Every host reader with gameplay effect rejects a dead unit before it consults the set:
- 00922C80 tests `+5Dh` at 00922C8F, mirrored by `tf.not_engageable = unit_flag_005d` in both
  callers;
- the HUD pick tests `flag_5d` (hud_warning_screen.cpp:381).

The extra members are therefore invisible. **Verdict: MISMATCH MINOR** (no reader sees it). RNG: none.

### O5 (new): units of a hidden objective are not in the `+18h` tree

**Image.** 008DF2B0, after the 008DEE50 entry push:
- 008DF367 `MOV ESI,[ESI+8]`;
- 008DF36A `CMP [ESI+18h],2` / 008DF36E `JNE 008DF376`;
- 008DF370 `CMP [ESI+1Ch],1` / 008DF374 `JNE 008DF3DC` (return).

A hidden objective (kind 2) that is not completed therefore skips both the 006DE3D0 marker stub and
008DF3D7 `CALL 008DE6A0` with `ECX = set+18h`. That call is the insert into the tree 008DDF90 reads.

**Host.** `units_in_slot` (game_hosts_ai.cpp:207) returns every unit of every objective in the slot,
hidden ones included. All three live readers use it.

**Missions.**
- About 50 missions add units to `hidden 1`. Most are ships, which answer before the set arm.
- Land or structure targets, which could reach 00922C80's set arm, are found in: COTP-IJN/PRCPIJN
  prcpijn_midway (`OilTanks`), COTP-IJN/prelude (`PrisonGuns`), COTP-USN usn_09_leyte and
  PRCPUS/usn_09_leyte (`Fortresses`), COTP-USN/us_fiji (`LandInstallations`), COTP-USN/us_osumi
  (`RadioTowers`) and ijn/ijn_29_hawaii (`RadioTower`).
- In those missions the host ship AI and the chooser can call such a target a surface target where the
  image falls to the 00922C80 tail. That needs the target to be IsKindOf(5) (008DDFA9).
- Test rows: USN13 :1251 `luaObj_Add("hidden", 1, Mission.KatoriGang)`. The members (Katori, Maikaze,
  Nowaki, AkagiMaru, ShonanMaru; :506-511) are ships, so only the HUD pick changes. JM08 :472,
  USN01 :940 and LOMP10 :436 add `hidden 1` with no units.

**Verdict: MISMATCH MINOR** on the test rows. It is a gameplay difference in the seven missions named
above, through 00922C80's set arm. RNG: none.

### Side note for the gunnery owner (not judged here)

game_hosts_gunnery.cpp:7085-7107 `unit_is_surface_target` never consults the set. On
`kUnreadSetBranch` it takes the tail (`:7106`) under the comment "The set branch 008DDF90 is not built
in this process". That comment is stale: the set is built and the ship AI and script orders read it.
The file is read-only for this lane.

## Part 2: plane files

Precision: all of this runs in the plane unit's step. Per docs/CC13_FPU_PRECISION.md that may run at
24 or 53 bits. "1 ulp at 53 bits" means the host and the image agree at 24 bits.

| Function | Host | Image | Verdict |
|---|---|---|---|
| `body_to_world_007d9c80` | plane_angular_velocity.cpp:228-232 | 007D9C80-007D9CD1: two 0042D0D0 calls with matrix `[ESI+8]+74h`, normalise 0 (007D9C89, 007D9CAC), inputs `+3Ch` / `+48h`, outputs `+18h..+20h` / `+24h..+2Ch`. 0042D0D0 sums `(m0x + m4y) + m8z` and rounds once at the FSTP | MATCH |
| `plane_squadron_wing_count_007f4754` | plane_squadron_entity.cpp:8-14 | 007F473A stores the default 3 (`[ESP+1Ch]` after the push, the `[ESP+18h]` slot); 007F4761 `CMP EAX,1` / `JL` keeps 1; 007F4778 stores to `+3C8h` | MATCH |
| `plane_squadron_attach_plane_007f4b43` | `:16-31` | 007F4B3D-007F4B6E: `plane+9D8h` = count, `plane+9D4h` = squadron, `[+3CCh*4+3D0h]` = plane, count+1, `+3ECh` = 1 | MATCH; MINOR guards (null plane, negative count, the five-slot refusal; labelled). `plane+9D4h` is not part of this projection |
| `plane_squadron_excluded_007eda90` | `:76-81` | 007EDA91 `[+3D0h]`, 007EDAA0 IsKindOf(17h), 007EDAAA `+C24h` | MATCH |
| `set_flight_state_007c1430` | plane_ground_ops.cpp:62-116 | Jump table 007C1550 maps states 3..7 to 007C1532 / 007C147F / 007C147F / 007C150A / 007C14D0. State 2 zeroes `+9F0h` before the equality test (007C143C). `+C04h` = -1.0. The Runway arm tests `class+198h` with `UCOMISS`/`JNP` (NaN writes), then `+904h = +908h > 5.0` (00CE3850) and `+C18h = 3`. Flying resets `+908h` to 0 only from 4 or 3, else 3600 (00CFDEB0). 007C11E0(0) runs on every arm | MATCH |
| `water_drown_test_007cbd79` | `:283-292`; caller game_hosts_units.cpp:36069 `interpolate_y1 = 0.5f` | 007CBD79-007CBDE0: `00419010(+B8h, 1.0, +24Ch x class+184h, 0.5 (00CE3800), speed)`, x `+C4h`, then `FCOMI depth,threshold` / `JBE` (NaN does not drown) | MATCH |
| `water_accumulator_step_007cbe7a` | `:294-305` | 007CBE7A: + double 0.5 (00D7A280), stored as a float. 007CBE9B / 007CBEB5: two `JBE` exits, NaN exits. 007CBEB7-007CBECB: `(1 - speed/min) x depth + acc`. The quotient and the product are unrounded | MATCH; MINOR (1 ulp at 53 bits: the host rounds `speed/min` and `depth x shortfall` to float) |
| `wire_rope_band_007dbeee` (the part VERIFY10 P3 did not cover) | `:226-251` | 007DBEF6 WheelBrake x load. 007DBF16 0.1 (double 00D7A3A0) `JBE` hold. 007DC004-007DC015 `len_xz x scale x tuning+518h + acc`. 007DC024 min with `+51Ch`. 007DC078 `00419010(160, 1.4, 250, 1.0, owner+B4h)` x acc (00D04A10, 00D06874, 00CE77B0, `FLD1`) | MATCH; MINOR (1 ulp at 53 bits, product rounding) |
| `plane_class_part_anim_duration_007d3883` | plane_class_fields.cpp:235-239; caller game_hosts_units.cpp:14256 | 007D3893 `FSUBP`, 007D389F `FCOMPI` with 0, `JBE` to `-0.0 - d` (00D7A208) | MATCH (operand order cannot matter under the fold) |
| `plan_yaw_0099e81a` | plane_ai_control.cpp:175-199 | Base 0099E82B-0099E884: `num / ((YawSpd x cos) x [EBX+9Ch])`, clamped (00415690), x gain. Blend 0099E8C8 `00419010([EBX+7Ch], 0, [EBX+80h], 3.0 (00CE3854), |bank|)`, min 1.0 (00415510). Turn 0099E8F6-0099E939. 0099E94A-0099E95C: `t x turn` and `(1-t) x base` each stored as float, then summed (0099E96C) and clamped (0099E98D). Both gates are `COMISS`/`JBE` against 0 | MATCH. MINOR: 1 ulp at 53 bits (the image does not round `1 - t` and the two denominators); sign of zero when `turn_num <= 0` (the image sums `+0 + base`, the host returns `base`, so a `-0` base comes out `+0` in the image) |
| `pilot_plan_roll_0099e2ba`, max bank and ramp | `:483-492` | 0099DFAA `[ESI+2E8h] x class+25Ch`. 0099DFCC 0047B880 picks `[EBX+64h]` or `[EBX+68h]`; 0099DFF3 `JA` keeps the product, so the cap wins on equal or NaN. 0099E07E `00419010([EBX+8Ch], 0, [EBX+90h], 1.0, |h|)` | MATCH (NaN-only arm differs) |
| the same, sweep floor and deadband | `:496-513` | 0099E08B x 1.5 (00CE3D78) as float, `min` with max bank (0099E0A1 `JBE`). 0099E0D4-0099E104: the floor applies for `-0.01 <= c <= 0.01` (00D7A300, 00D7A238), giving -0.1 (00CE3CB4) when negative and 0.01 otherwise; NaN keeps `c`. 0099E110-0099E17B: `00419010(-c, f(-0.3c), c, f(0.3c), h)`. 0099E180 `h - s` uses the double copy of `h`, then `/ c` (0099E18D), clamp ±1 (00415620), x max bank | MINOR: **00CE3DC8 is `3FD3333340000000h`, the float 0.3f widened**, but `:508-509` multiply by the double literal `0.3`. The two endpoints can differ by 1 float ulp. The `c != 0` guard (`:510`) cannot trigger. Rounding of `h - s` is 1 ulp at 53 bits only |
| the same, pitch-scheduled limit | `:531-546` | 0099E1CC `00419010([EBX+6Ch], [EBX+78h], [EBX+70h], [EBX+74h], pitch error)`. 0099E1E1 `JA` keeps the limit, else `|bank|`. 0099E218 clamps to [-2, 2] (00CE7D7C, 00CE3958). 0099E23E clamps to `±limit`. 0099E28F compares against 00CE3D28 = `400921FB60000000h` (float pi widened); the host's `3.14159274` sorts every float the same way | MATCH |
| the same, servo (mode 2) | `:550-564` | 0099E2CE 00438B10(`+2C4h`, bank), then **0099E2D3 `FMUL [ESP+28h]`**. That slot is 0099D4E6's `1 / max(float(class+340h x 0.4 (00CE65D0)), 1.0)`; the frame was traced through 0099DA07, whose IsKindOf(6) is `RET 4`. 0099E30E/0099E314: `v x [EBX+44h]` when `[EBX+40h] > |v|`. Otherwise 0099E328 writes `+2ECh` = [00E0E2F4], then `v -/+ [EBX+48h]`. 0099E344-0099E367: `t = A x f((A / class+1BCh) / [EBX])`, A = class+1A8h. 0099E390 `00419010(-t, 1, t, -1, a)` -> `+290h`; `+294h` = 1; `+2CCh` = 0 | see L1 and L2. The `+2ECh` write is labelled, not modelled (plane_ai_control.hpp:507-508) |
| `plane_control_targets_007da710` tail | plane_control_rate.cpp:127-212 | 007DA8D9 mode 1 zeroes roll. 007DA8EB-007DA922: pitch `(PitchSpd x f1) x latched`, x `+1D8h` when latched < 0 (`COMISS 0,x`/`JBE`). 007DA926-007DA95F: yaw `(YawSpd x latched) x +BC4h`, then x f2. 007DA963-007DA9EB is the `+BC4h` relax (`UCOMISS 1.0` / `LAHF` / `JNP`), hosted at game_hosts_units.cpp:35665-35675 with the read-before-relax order. 007DAA00-007DAA58: the slide term `((+1B8h x YawSpd) x sin) x cos`, `FSUBP`. 007DAA5C-007DAA78: `roll += (yaw_raw x f1) x +1B4h`. 007DAA85 yaw negated against -0.0. The accels are `+1C0h x f1`, `+1C4h x -f2` and `+1BCh x f1` | MATCH. MINOR: `:187` multiplies `(YawRollRatio x f1) x yaw_raw` where the image does `(yaw_raw x f1) x YawRollRatio`. That association can differ by 1 ulp **at 24 bits too**. The slide product is unrounded (1 ulp at 53 bits) |

### L1: dt scale is divided by, where the image multiplies (latent)

- Host: plane_ai_control.cpp:551 `v /= in.dt_scale` (roll servo) and `:390` `error / in.dt_scale`
  (pitch arm).
- Image: 0099E2D3 and 0099E604 `FMUL [ESP+28h]`.
- The header names `dt_scale` as `1 / max(unit+340h x 0.4, 1.0)` (plane_ai_control.hpp:183, :408),
  which is the slot's value.
- Both callers pin it to 1.0 (game_hosts_units.cpp:34900, :35108). docs/PILOT_BANK_DOWNSTREAM_DEFECT.md
  item 3 explains why: unit+340h is the Lua CheatTurbo multiplier, and the scale is exactly 1.0 at or
  below 2.5.
- The result is identical today. If the pin is ever replaced by the real value, the division inverts it.

**MISMATCH MINOR (latent).**

### L2: the servo's map limit

`:562-563` computes `roll_spd^2 / (roll_accel x waggle)` with a zero guard (`t = 0`, so the map returns
+1.0).

The image computes `A x f((A / B) / C)` with no guard:
- the order of rounding differs, by 1-2 ulp of `t` at either precision;
- with `B` or `C` zero the image gets `inf` and the map gives NaN, where the host gives 1.0. That
  happens only for an authored zero.

**MISMATCH MINOR.**

## Labels

| Where | Label | Verdict |
|---|---|---|
| game_hosts_gunnery.cpp:7087-7089 | "The set branch 008DDF90 is not built in this process" | STALE LABEL (see the side note) |
| plane_bow_wave.cpp | no caller outside the file | not live; the arithmetic was not read |
| plane_class_fields.cpp `read_plane_class_fields_007d1f70` | no caller outside the file | not live; only 007D3883 and 007D20F3 are used |
| plane_ground_ops.cpp: 007CCB30, 007CEC30, 007C6340, 007DCCF0, 007CC0C9, 007CBFF0, 007DA540, 007CC1B3, 007CBB91, 007CBED9, 007CBFA0 | no caller outside the file | not live; not read |

## Not covered

- `begin_flying_007c7110` (one caller) was read only in the host.
- In plane_angular_velocity.cpp, the look-at, rotation, multiply and `rotate_about_axis_equivalent`
  helpers are outside `body_to_world`, and their liveness was not traced.
- The two plane_squadron_entity.cpp bodies with callers (007ED610 promote and 007F39ED remove) were
  not byte-checked.
- 00415690's NaN arm against `clamp_unit`.
- competitive05 `primary 9` (O1, multiplayer).
- Whether the seven O5 land-target missions actually route a ship-AI or chooser query to such a
  target. That needs a run, and runs are out of scope.
