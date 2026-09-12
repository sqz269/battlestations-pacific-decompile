# The commanded speed at `*(unit+73Ch)` and the director's command stages

Addresses: 00890D30, 008A3600, 00835BF0, 00835B40, 00836920, 00822B70, 0081ED40, 009E1170, 0071BE60, 0071C130, 0071D810, 0071D9E0, 007788B0, 007788D0, 00465080, 0071ECF0, 009EC7C0, 00F876A4 (data), 00D09F58 (vtable), 00D09EC0 (vtable), 00D094C4 (string).

Packet `cc_commanded_speed`. Every name here is a hypothesis, not a recovered symbol.
`docs/CRUISE_COMMAND.md` left the producer of the commanded-speed pair `contract: unread`
and said no binding string named it. Both statements are wrong and are corrected below.

## Answer to the packet question

The block at `*(unit+73Ch)` is the unit's **`navigatorParams`**. The name is the literal
string at `00D094C4`, which `00818920` and `0082007B` pass to the property registration
alongside the block pointer. Its `+24h`/`+28h` pair has **two producers, both Lua
bindings**, and they store the identical pair:

| Producer | Site | Store |
| --- | --- | --- |
| `luaMW_SetShipSpeed` (`00890D30`) | `00890E86`, `00890E97` | `+24h = max(arg1, 0)`, `+28h = DAT_00F876A4` |
| `luaMW_NavigatorMoveOnPath` (`008A3600`) | `008A3901`, `008A3912` | the same, with the path order's speed |

`+28h` is **not a boolean enable**: it is the **mission clock** `DAT_00F876A4` at the moment
the order was given. `00835C28` subtracts it from the clock and compares the difference with
`1.0f`, so it is a timestamp with an age test. `-1.0f` (`00D7A260`) means "no speed
commanded"; every consumer only ever tests `>= 0.0f`, so a clock of `0.0f` still reads as
active.

The pair matters because of the director's **idle tail** at `00836DC9`. When a weapon
director runs out of commands it re-issues a default one, and the commanded speed is what
decides which:

```
00836E0B  dir->vtable[6Ch](1)                       ; expire a stale commanded speed first
00836E13  if 007788B0(unit)  -> issue `follow` on 007788D0(unit)      ; 00E08F60
00836E45  else if unit+184h  -> issue `cruise` on the unit            ; 00E08F70
00836E59  else if params+28h >= 0.0f -> issue `cruise` on the unit    ; 00E08F70
          else                       -> issue `stop`   on the unit    ; 00E08F88
```

So `SetShipSpeed` does not write a throttle anywhere. It puts a speed on `navigatorParams`,
which makes the idle director choose `cruise` over `stop`, and `009E12BD` in the ship AI's
cruise state then turns `+24h` into a throttle by dividing it by `0080FC30`'s reference
speed. That is the whole path from the script call to the ring.

A commanded speed also **ends a standing `stop`**: `00836A8B`'s arm raises the primary
command stage to 2 when the primary command is `00E08F88` and the pair is active.

## The block

`0081F283..0081F28C` in `BSP_UnitVehicleBase_Construct` allocates it and stores the pointer
at `unit+73Ch`; `0081F4A7..0081F4B2` frees it in the unit's vector deleting destructor. Its
`+0h` holds a float (`00822B95`), so it carries no vtable and is a plain struct. It is four
bytes past the weapon director pointer at `unit+738h` and is a separate allocation.

| Offset | Type | Producer | Evidence |
| --- | --- | --- | --- |
| `+0h`..`+18h` | 7 floats | `00822B70` from the game tuning singleton `00424C40` `+160h`..`+178h` | `00822B95`..`00822C0E` |
| `+10h` | float | also written by the constructor, from `XMM1` | `0081F269` |
| `+1Ch` | float | constructor, `-1.0f` | `0081F26E` |
| `+20h` | byte | `00822B70` writes `1`; `luaMW_NavigatorAllowMaxDepth` writes the Lua boolean; `00822C20` writes `AL` | `00822B84`, `008A33FF`, `00823A8E` |
| `+21h` | byte | constructor writes `1`; `luaMW_NavigatorForceMoveCloseToTarget` writes the Lua boolean | `0081F27D`, `008A35A4`; read at `009EEAB3` |
| `+24h` | float | the commanded speed in m/s | `00890E86`, `008A3901`; constructor `-1.0f` at `0081F273` |
| `+28h` | float | the mission clock when the speed was commanded, `-1.0f` for none | `00890E97`, `008A3912`; constructor at `0081F278` |

The block's total size is not established here: the allocation call was not read. Offsets
above `+28h` are not read by this packet.

`BSP_GameSettings_LoadFromLuaGlobals` at `008412EE` and `008413A6` also uses displacement
`73Ch`, but on the settings singleton, not on a unit. It is not related.

## The producers, read

Both bindings share one store idiom, byte for byte:

```
00890E6F  FLD   [ESP+10h]          ; the requested speed
00890E73  FLDZ
00890E75  FCOMPI ST(1)             ; compare 0.0 with the request
00890E79  JBE   00890E80           ; request >= 0 -> keep it
00890E7B  XORPS XMM0,XMM0          ; otherwise 0
00890E86  MOVSS [ESI+24h],XMM0
00890E8B  MOVSS XMM0,[00F876A4]    ; the mission clock
00890E97  MOVSS [ESI+28h],XMM0
```

`008A38D5..008A3912` in `luaMW_NavigatorMoveOnPath` is the same sequence on the same two
offsets. Its speed defaults to the unit's class `MaxSpeed` (`*(*(unit+538h) + 500h)`, the
`MaxSpeed` field `src/ship_motion_probe.cpp` reads from the vehicle class table as
`class+500h`) and Lua argument 4 overrides it; arguments 2
and 3 are integers the path message carries. The store happens after the binding has already
routed its `5Bh` session message, so the speed is a side effect of the path order rather
than part of it.

`DAT_00F876A4` is the mission clock the rest of the reconstruction already uses
(`docs/WORLD_TIMED_ATTACHMENTS.md`, `docs/UNIT_INSTANCE_UPDATE.md`,
`docs/MISSION_RESULT_DECISION.md`); `00874640` sets it at mission load and teardown.

## The expiry, `00835BF0` (director vtable `6Ch`)

`__thiscall(director)(char primary)`, `RET 4`, body `00835BF0-00835C6F`, **no Ghidra
function**. Coverage: complete.

```
00835BFA  0071C130(director, primary)         ; the base clear: +44h/+48h or +4Ch/+50h
00835C05  params = [[director+24Ch]+73Ch]
00835C0B  t0 = params[28h]
00835C12  al = (primary == 0)
00835C15  if (t0 < 0.0f) goto 00835C48        ; inactive
          ; active:
00835C26  if (!primary) goto clear
00835C28  if ((float)(int)DAT_00F876A4 - t0 > 1.0f) goto clear   ; else keep
00835C48  ; inactive: primary keeps, non-primary clears
00835C54  params[28h] = -1.0f
00835C65  if (primary) 00822B70(unit, 0)      ; the literal 0 makes that body a no-op
```

The `CVTTSS2SI` at `00835C28` truncates the clock to a signed int and `FILD` at `00835C34`
converts it back, so the age is measured from the whole-second floor of the clock, not from
the clock itself. `FLD1` at `00835C3C` is the one-second budget.

`00836E0B` is the only call site this packet found, and it passes a literal `1`. So a
commanded speed survives only about a second of an idle director unless the script refreshes
it; while a `cruise` command is standing the tail is not reached and nothing expires it.

## The command stages

`0071C130` settles the field pair: `+44h`/`+48h` are the **primary** command's flag and
stage, `+4Ch`/`+50h` the **secondary** (override) pair. `0071D810` raises `+48h` and
`0071D9E0` raises `+50h`, each only upward, and each sends a completion message through
`BSP_Session_RouteMessage(.., 7, 0)` when the new stage is 2 and the session is not in mode
2. `0071BE60` counts filled command slots: the array at `director+54h` with stride `1Ch`
(`0x54 == 3 * 0x1C`, which is why `008369DB` reaches slot *i* as element *i+3*), stopping at
the first null or at 10.

| Stage | Meaning | Evidence |
| --- | --- | --- |
| 0 | the command has not started | `0071C130` clears to 0 |
| 1 | running | `00836962` and `0083698A` test for it |
| 2 | finished; the completion message goes out | `0071D82D`, `0071D9FD` |

`00836920` (vtable `7Ch`) is where every raise happens. Its stage spine:

1. `00836941..00836957` sets the local flag at `[ESP+0Bh]` when the director has no primary
   command **and** no filled slot at all. The idle tail reads it at `00836DE0`.
2. `00836962..00836985`: primary stage 1 finishes when more than one slot is filled or the
   unit is player-controlled.
3. `0083698A..00836993`: secondary stage 1 always finishes.
4. `008369A1..00836A7E`: with the primary stage still 0, a non-weapon primary command and a
   queued weapon command (category 1 or 2) whose target resolves farther than
   `4.0e6` m² (2000 m, the double at `00D09FE8`) finishes the primary command.
5. `00836A81`: a primary stage already at 2 skips every per-command arm.
6. the per-command arms, keyed on `[director+54h]`: `stop` (`00836A8B`), `follow`
   (`00836ADC`), `attackmove` (`00836B45`), `moveonpath` (`00836BF0`).
7. `00836D67`: a secondary `attackmove` raises the **secondary** stage through `0071D9E0`.
8. `00836DC9..00836EA7`: the idle tail above.

The `stop` arm, complete:

```
00836A8E  if (primary command != 00E08F88) -> next arm
00836A9D  if (0071BE60(director) > 1)         -> raise 2
00836AAD  if (unit+184h != 0)                 -> raise 2
00836AC1  if (params[28h] >= 0.0f)            -> raise 2
          else                                -> leave the stop running
```

## The other arms, summarised

Read but not projected into C++ (`coverage: partial` on `00836920`):

* `follow` (`00836ADC..00836B40`) finishes unless the unit has a controller
  (`unit+284h`) whose owner is another entity, that owner is also the primary slot's
  resolved target, and only one slot is filled.
* `attackmove` (`00836B45..00836BEB`) finishes when the target fails
  `target->vtable[5Ch](41h)`/`(1Ch)`, is not alive (`0043F080`), or `005457C0` rejects the
  unit/target pair; on the alive-and-matching path it issues `00E08F68` first.
* `moveonpath` (`00836BF0..00836D66`) builds an arrival radius from `unit+9C8h`, the class
  speed and the two doubles at `00CE3DE0` and `00CEC160`, walks the path cursor through
  `0071BFF0`, and on arrival (`007ADD70`) raises the stage to 2 and fires the mission event
  named by the string `"finished"` at `00D09FD8`.
* `0083693C` calls `00835F60` before any of this; that callee is `contract: unread`.

## The ship AI cruise state, `009E1170`

`009E1170` is leased to `agent/cc-exe-2l`; this packet adds no ledger record and no Ghidra
annotation for it and only reads it. Its arm selection, from the live listing:

| Arm | Entry | Condition | What it does to the commanded speed |
| --- | --- | --- | --- |
| return | `009E1188`, `009E1196` | `[state+4]` null, or `[unit+740h]` null | nothing |
| detached | `009E13B4` | `[[unit+740h]+50h]+1B0h != 8` and `00927F10` rejected it | `009E13F8` stores `-1.0f` into `+28h` |
| player | `009E11CE` | `unit+184h` set | nothing |
| cruise | `009E1265` | otherwise | reads `+28h`, and `+24h` at `009E12BD` |

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| `docs/CRUISE_COMMAND.md`: "**The producer of `+24h` is `contract: unread`**: no direct-displacement store to it was found and no Lua binding string names it (`luaMW_LandConvoySetSpeed` and `luaMW_SquadronSetSpeed` are the only `SetSpeed` bindings in the image...)" | There are two producers and a third `SetSpeed` binding string. `luaMW_SetShipSpeed` at `00890D30` and `luaMW_NavigatorMoveOnPath` at `008A3600` both store the pair through a plain `MOV reg,[reg+73Ch]`. `luaMW_LandConvoySetSpeed` writes `convoy+368h` (`00741140`) and `luaMW_SquadronSetSpeed` calls `member->vtable[3Ch]` per squadron member; neither touches this block. | `00890E86`/`00890E97`, `008A3901`/`008A3912`; the binding strings at `00890D30` and `008A3600`; `00741140`'s three-instruction body |
| `docs/CRUISE_COMMAND.md`: "`+28h` its enable; `>= 0.0f` active, `-1.0f` inactive" | Correct as a test, wrong as a type. The value stored is the mission clock `DAT_00F876A4`, and `00835C28` measures its age against `1.0f`. It is a timestamp. | `00890E8B`, `008A3906`, `00835C28..00835C44` |
| `docs/CRUISE_COMMAND.md`: "`009E11C6` stores `-1.0f`" (cited twice, and as `kCruiseSpeedSettingInactive`'s evidence in `include/bsp/cruise_command.hpp`) | The store is at `009E13F8`. `009E11C6` is not an instruction start: the instruction there is `CMP byte ptr [ECX+0x184],BL` at `009E11C2`. The arm that clears the pair begins at `009E13B4`, not at `009E11A9`. | the live listing of `009E1170`, `009E13F2..009E13F8` |
| `docs/CRUISE_COMMAND.md`: "`00836920` ... reads the same enable at `00836AA7..00836AC8` and raises the command stage to 2 when it is active and the unit is not player-controlled" | The arm raises the stage when **any** of three holds: more than one slot filled, the unit **is** player-controlled, or the pair is active. Being player-controlled raises the stage, it does not block it. The arm only runs for the `stop` command `00E08F88`. | `00836A9D..00836AD2` |
| `docs/CRUISE_COMMAND.md` Corrections: "`Cruise` writes nothing into the ring ... from a producer this packet could not find" (for `unit+0FC4h`/`+0FDCh`) | Still open. This packet did not find that producer either and makes no claim about it. | - |
| the packet brief: "`unit+73Ch` ... a parameter block `00822B70` fills" | `00822B70` only **refills** `+0h`..`+18h` and sets `+20h`; the block is allocated and initialised in `BSP_UnitVehicleBase_Construct`, and `00822B70`'s whole body is skipped when its `char` argument is zero, which is what its only call site in this packet (`00835C65`) passes. | `00822B70`'s body; `0081F283..0081F28C` |

## Coverage

| Routine | Address | Coverage |
| --- | --- | --- |
| `navigator_commanded_speed_store_00890e6f` | `00890E6F..00890E97`, `008A38D5..008A3912` | complete |
| `navigator_commanded_speed_stale_00835c28` | `00835C28..00835C44` | complete |
| `navigator_commanded_speed_active` | `009E12AC`, `00836AC1`, `00836E59` | complete |
| `weapon_director_reset_command_stage_00835bf0` | `00835BF0..00835C6F` | complete |
| `weapon_director_step_prepass_00836941` | `00836941..00836997` | complete |
| `weapon_director_abandon_for_far_weapon_target_008369a1` | `008369A1..00836A7E` | partial: the stage decision only. `00521EA0` (target resolve), `00427EB0` (world position) and the two `vtable[0Ch]` category calls are caller-supplied, and the squared planar distance is passed in rather than computed |
| `weapon_director_stop_arm_00836a8b` | `00836A81..00836AD7` | complete |
| `weapon_director_idle_reissue_00836dc9` | `00836DC9..00836EA7` | complete |
| `ship_cruise_state_arm_009e1188` | `009E1178..009E11CE`, `009E1265`, `009E13B4` | complete for the arm selection; the arms' bodies are `docs/CRUISE_COMMAND.md`'s |
| `ship_cruise_state_commanded_speed_after_arm_009e13f8` | `009E13F2..009E13F8` | complete |
| `00836920` as a whole | `00836920..00836EA7` | partial: `0083693C`'s callee `00835F60` unread, and the `follow`, `attackmove` and `moveonpath` arms `00836ADC..00836D66` are read and summarised above but not projected |
| `luaMW_SetShipSpeed` `00890D30` | body | partial: the Lua frame plumbing is the shared binding prologue; only the store is projected |
| `luaMW_NavigatorMoveOnPath` `008A3600` | body | partial: the `5Bh` session message it routes is not read |

## Host table

| Step | Site | Callee | this | args | Gate |
| --- | --- | --- | --- | --- | --- |
| clear the stage pair | `00835BFA` | `0071C130` | the director | `primary` | always (inside `00835BF0`, no Ghidra function) |
| store the cleared timestamp | `00835C54` | - (a store, not a call) | - | `-1.0f` | the clear rule above |
| refill from tuning | `00835C65` | `00822B70` | `[director+24Ch]` | literal `0` | `primary` set (inside `00835BF0`, no Ghidra function) |
| filled slots | `0083694E` | `0071BE60` | the director | none | `[director+54h] == 0` |
| filled slots | `0083699A` | `0071BE60` | the director | none | primary stage == 1 |
| raise primary | `00836985` | `0071D810` | the director | `2` | crowded or player-controlled |
| raise secondary | `00836993` | `0071D9E0` | the director | `2` | secondary stage == 1 |
| filled slots | `00836A9D` | `0071BE60` | the director | none | the `stop` arm |
| raise primary | `00836AD2` | `0071D810` | the director | `2` | the `stop` arm fired |
| filled slots | `00836DD6` | `0071BE60` | the director | none | primary stage == 2 |
| filled slots | `00836DED` | `0071BE60` | the director | none | the tail's gate passed |
| reset the stage | `00836E0B` | `00D09F58+vtable6c` = `00835BF0` | the director | `1` | the tail runs |
| group check | `00836E13` | `007788B0` | `[director+24Ch]` | none | the tail runs |
| group owner | `00836E28` | `007788D0` | `[director+24Ch]` | none | the unit is in another's group |
| build the target | `00836E32` | `00465080` | a stack `SceneCommandTarget` | the owner, `0.0f` | same |
| build the target | `00836E6D` | `00465080` | a stack `SceneCommandTarget` | the unit, `0.0f` | the `stop` branch |
| build the target | `00836E85` | `00465080` | a stack `SceneCommandTarget` | the unit, `0.0f` | the `cruise` branch |
| issue | `00836E92` | `0071ECF0` | the director | the command object, the target | the tail reached a choice |
| raise primary | `00836A7C` | `0071D810` | the director | `2` | the far weapon target test passed |
| raise secondary | `00836DC4` | `0071D9E0` | the director | `2` | the secondary `attackmove` arm |

`00835BFA` and `00835C65` sit inside `00835BF0`, which has no Ghidra function, so
`tools/verify_report_calls.py` cannot check them; `reports/commanded_speed.json` records
them under `native_unverified` with that reason. Define the function first with
`python tools/ghidra_define_function.py 00835bf0 00835c70` and the rows become checkable.

## The probe

`src/ship_motion_probe.cpp` gained `--commanded-speed <m/s>`. Under `--cruise` it stores the
pair the two producers store at the latch frame and lets `cruise_ordered_values_009e1170`
take the override branch. For `VehicleClass[20]` (DeRuyter class 1935, a cruiser, `MaxSpeed`
16.4622 m/s) with `--rudder 0 --steps 200`:

| Run | `+24h` | `+28h` | throttle at the latch frame | peak fwd speed | final fwd speed |
| --- | --- | --- | --- | --- | --- |
| `--cruise` | 0.0 | -1.0 (inactive) | 1.000000 | 16.4622 | 16.4622 |
| `--cruise --commanded-speed 6.0` | 6.0 | 1.0 (the run's clock) | 0.364471 | 6.0000 | 6.0000 |

`0.364471 = 6.0 / 16.4622`, the division at `009E12BD`, and the ship settles on the
commanded speed instead of the class maximum. The `+28h` the probe stores is its own clock
at the latch frame, not `DAT_00F876A4`; the probe has no director step, so the expiry at
`00835BF0` never runs and the pair stands for the whole run.

## no_ghidra_function

| Start | Inclusive end | Evidence per boundary |
| --- | --- | --- |
| `00835B40` | `00835BEA` | Start: `CC` padding at `00835B38..00835B3F` ends the previous function (`BSP_WeaponDirector_LatchCruiseFields`, Ghidra body `00835AC0-00835B37`, whose `RET 0Ch` is `C2 0C 00` at `00835B35`), and `00D09FC8` (director vtable slot `70h`) points here; the first bytes `53 55 8B 6C 24 0C` are a `push ebx / push ebp / mov ebp,[esp+0Ch]` prologue. End: `RET 8` = `C2 08 00` at `00835BE8..00835BEA`, followed by `CC` padding at `00835BEB..00835BEF`. |
| `00835BF0` | `00835C6F` | Start: the `CC` padding above, and `00D09FC4` (director vtable slot `6Ch`) points here; the first bytes `51 53 8B 5C 24 0C` are a prologue. End: `RET 4` = `C2 04 00` at `00835C6D..00835C6F`, immediately followed by `00835C70`, the Ghidra function `BSP_WeaponDirector_BeginCurrentCommand` (`83 EC 20`, `sub esp,20h`). |

Both are reachable only through the derived director vtable at `00D09F58`; the base vtable
`00D09EC0` holds `0071C130` at the same slot `6Ch`, which is what `00835BF0` calls at
`00835BFA`.

## Follow-up packets

* `navigator_params_tuning` - the seven floats at `+0h..+18h` and the game tuning singleton
  fields `00424C40 +160h..+178h` they come from, plus the block's total size and the offsets
  above `+28h`. `009EC7C0 BSP_UnitBot_ComputeThrottleCeiling` and `009ED6B0` read `+21h` and
  the same speed pair against a different reference (`[unit-bot+3C4h]`, not `0080FC30`).
* `director_command_arms` - the `follow`, `attackmove` and `moveonpath` arms of `00836920`
  (`00836ADC..00836D66`), `00835F60`, and the path cursor helpers `0071BFF0` and `007ADD70`.
* `director_retarget_00835b40` - `0071EDD0`, the delegate `00835B40` calls when the incoming
  target differs from the slot's current one, and the command class `00E08F68` it issues.
* `unit_autopilot_pair` - still the open one from `docs/CRUISE_COMMAND.md`: who writes
  `unit+0FC4h` and `unit+0FDCh`, the pair `00825F20` copies into the ring under the
  `unit+61h` gate. Not touched here.
* `navigator_lua_surface` - the other eighteen `luaMW_Navigator*` bindings; three of them
  (`AllowMaxDepth`, `ForceMoveCloseToTarget`, `MoveOnPath`) write this block and the rest
  were not read.

## Uncertainties

* `00836E0B` is the only call site of vtable slot `6Ch` this packet found, by scanning
  `.rdata` for the address and the listing for the slot. An indirect call through a base
  pointer elsewhere would not have shown up, so "the only site" is a bounded claim, not a
  proof.
* The one-second budget at `00835C3C` is read from the instruction `FLD1`; whether it is a
  frame budget, a script-refresh window, or a rounding guard against the `CVTTSS2SI`
  truncation is not established.
* `00822B70(unit, 0)` at `00835C65` does nothing. The projection makes the call anyway
  because the native does; whether the literal `0` is a deliberate disable or a shipped bug
  is not established.
* `luaMW_NavigatorMoveOnPath`'s Lua argument order is taken from the decompiler's
  `BSP_LuaObject_ArgumentAt` indices (2, 3 integers, 4 the speed). The Lua side was not read.
