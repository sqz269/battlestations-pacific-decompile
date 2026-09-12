# The Blackout fade and its completion callback (packet `cc_mission_blackout`)

Addresses: 008D1340 005B9BA0 005B9800 005BC920 0076D310 005B66D0 00644220 004C40F0 004F71F0 00CF0ED8

`docs/LUA_BINDING_MISSION.md` closed with the mission sitting at `MissionPhase = 0` and named one
gate: "`MissionPhase` becomes 1 only in `luaIn`, and the only route to `luaIn` is a `Blackout`
completion callback". This packet reads that route end to end, reconstructs it, wires it into
`bsp_game.exe`, and the mission then runs through `luaIn` into phase 2. The new stopping point is
`FindEntity`.

## Where the fade lives

`*(00E198C4 + A4h)` is the HUD narrative screen, front-end registry slot 33h, vtable **00CF0ED8**
(`005BA7EF MOV dword ptr [ESI],0xcf0ed8`), E8h bytes, constructor 005BA7B0.
`include/bsp/interface_runtime_tail.hpp` already established that screen and its `Blackout_Icon`
widget at `+BCh`; this packet adds the five fields above the widget pointer.

| Offset | Field | Written by |
| --- | --- | --- |
| `+BCh` | the `Blackout_Icon` widget, bound at 005BB983 | 005BB130, the screen's register virtual |
| `+C0h` | current level, 0 clear to 1 opaque black | 005B9800 only |
| `+C4h` | target level | 005B9BA0 at 005B9BC7 |
| `+C8h` | seconds remaining | 005B9BA0 at 005B9BB9, decremented by 005B9800 |
| `+CCh`/`+D0h` | the callback name as a NativeString `{size, data}` | 005B9BA0 at 005B9BD8/005B9BED, cleared by 005B9800 at 005B994C |

The producer of `+C4h`, `+C8h` and `+CCh` is 005B9BA0 and the producer of `+C0h` is 005B9800, so
the meanings above come from the writers, not from a consumer. 005BA7B0 zeroes `+CCh` and `+D0h`
(005BA839, 005BA83F) and writes **none** of `+C0h`, `+C4h` or `+C8h`; whether the allocation the
constructor runs on is zero filled was not read, and is the one uncertainty in the layout. It does
not matter to any observed path because every run begins with a `Blackout` that overwrites all
three.

`005B5D50`, already documented in `docs/GAME_AWARD_TRACKERS.md` as "a screen transition or fade is
running", is `*(float*)(this + C0h) != 0.0f` on this same screen. That is now a settled reading.

## 008D1340, the `Blackout` binding

`__fastcall(lua_State* in ECX)`, body 008D1340-008D16A7, result count from 00B66400 at 008D163E
with nothing pushed. The decode is 008D142F..008D1612:

| Lua argument | Read at | Meaning |
| --- | --- | --- |
| 1 | 00B66250 at 008D143E | `enable`. Read unconditionally. |
| 2 | 00B662B0 at 008D1490, count > 1 | the callback name, copied into a local NativeString |
| 3 | 00B66000 at 008D1521, count > 2 | duration. A **boolean** true means 0.0 and false means the configured default re-read at 008D156B; anything else is a number read at 008D1598. |
| 4 | 00B66270 at 008D15E8, count > 3 | level. Default 1.0f from 00D7A24C at 008D15B2. |

Order matters: argument 4 sets the level and only then does 008D1602 force it to 0 when argument 1
was false. `Blackout(false, cb, d)` therefore always fades **out** of black.

The default duration is `*(float*)(00432650() + E0h)`, read at 008D14E2. That field was not read by
this packet. Every `Blackout` call in `usn_2_java.lua` and `commandhelpers.lua` passes an explicit
argument 3, so the run never consumes it; the executable reports it if a call ever does.

008D1635 then calls 005B9BA0 with `ECX = *(00E198C4 + A4h)` and the stack order
`(level, duration, &name)` — confirmed from the listing at 008D1612..008D1635, where the FLD at
008D1612 takes `[ESP+1Ch]` (the duration) into the second slot and the FLD at 008D162E takes
`[ESP+14h]` (the level) into the first.

## 005B9BA0, the arm

`__thiscall(screen, float level, float duration, const NativeString* name)`, `RET 0Ch`, body
005B9BA0-005B9C3D.

```
+C8h = (float)((double)duration + 1.0e-4)     ; 005B9BAA FADD double ptr [00D7A268]
+C4h = level                                   ; 005B9BC7 MOVSS
if (&this->name != argument)                   ; 005B9BC5, a self-assignment guard
    +CCh/+D0h = *argument                      ; 0041DD40 then memcpy
005B9800(this, 1.0e-4)                         ; 005B9BF5 loads the float at 00CE3C68
if (*(00E188A8) + 1FE4h == 1)
    0076D310(*(00E188A8) + 1EF0h, level, duration)
```

The two `1.0e-4` constants are the whole reason a zero-duration fade works: `Blackout(true, "", true)`
(`usn_2_java.lua:405`) arms `+C8h = 1.0e-4`, and the immediate step consumes exactly that much, so
the screen is black on the same frame.

0076D310 builds session message kind 2Bh from `(level, duration)` and broadcasts it through
00784790. It is the multiplayer replication of the fade and is unreachable offline.

## 005B9800, the update and the callback

`__thiscall(screen, float step)`, `RET 4`, body 005B9800-005B9A78, `EAX = (+C8h > 0)`. The
remaining time is read once into a local at 005B981E and the routine then takes one of three arms.

**`remaining <= 0` — 005B98E0, the idle and completion arm.** `+C0h = +C4h`, then `+CCh` is
compared against the empty literal at 00CE3A0C case-insensitively (00449AF0 at 005B9908). When the
name is not empty:

```
005B9938  00426060   copy the name into a local
005B994C  0041E350   assign the empty literal to +CCh      <- cleared BEFORE the call
005B9969  00887E50   MissionLuaHost::call_named(self_key = 0, &name, 0, 0, -1)
```

Clearing first is what lets the callback arm the next fade from inside itself, which is exactly
what `luaIngameMovieBOStart` and `luaIn` do. The call passes self key 0, no argument record and no
stack range, so by `docs/MISSION_NAMED_CALL_ARGS.md` (nargs starts at 0 off the self-key path) it is
`_G[name]()` with zero arguments.

**`step < remaining` — 005B98AB, the blend.** Read from the x87 sequence, not the pseudocode:
`FLD +C0h / FSTP scratch / FLD +C4h / FLD scratch / FLD ST0 / FSUBP ST2 / FLD ST2 / FMULP ST2 /
FLD ST3 / FDIVP ST2 / FADDP / FSTP +C0h`, then `FSUBP / FSTP +C8h`. That is

```
+C0h += step * (+C4h - +C0h) / remaining
+C8h -= step
```

every intermediate in the x87 stack, only the results narrowed to float32. The comparison at
005B983E is `FCOMI` + `JC`, strictly less than, so a step equal to the remaining time takes the
third arm instead.

**otherwise — 005B9842, landing on the target.** `+C8h = 0`, `+C0h = +C4h`, and when `+C4h > 0`
with a local player unit at 00E188D8 and `005B66D0` false it runs `00644220(*(00E198C4+40h), 0)`
then `004CC460(00E198C4, 20h, unit)`. It does **not** call back. The callback therefore always fires
one update after the fade visually ends, on the pass that finds `remaining <= 0`.

**The paint tail, 005B9995.** Runs on every arm. `+C0h <= 0` calls the widget's `+34h` with 0;
otherwise `+34h` with 1, `+54h` to read the colour, the level into its alpha, and `+50h` to write it
back. On the `game+1FE4h != 0` arm the alpha is halved by the double 0.5 at 00D7A280 and, above the
0.75f at 00CEE07C, the `_PleaseWait` screen at 00E19698 is forced visible. `EAX` is `+C8h > 0`.

## What advances it every frame

`BSP_Game_UpdateInterfaceOnly` 004C40F0, reached from `GGame::OnMove` at 004E5259 in the
in-mission branch (`docs/MISSION_STATE_FRAME.md` step 18):

```
004C4273  EAX = [00E198C4]
004C4278  if (EAX->A4h == 0) skip
004C4286  if (screen byte +5h == 0) skip        ; visible
004C428B  if (screen byte +4h == 0) -> 004C42A1 ; active
004C4290  FLD [EDI + 21F0h]                     ; the scaled frame delta
004C429A  CALL 004F71F0                         ; BSP_FrontEndScreen_Update
```

004F71F0 dispatches vtable `+20h`. For 00CF0ED8 that slot (00CF0EF8) is **005BC920**, which reloads
the delta from `*(00E188A8) + 21F0h` into its own argument slot at 005BC9B3, returns early when it
is not above zero (005BC9B0), and calls 005B9800 at 005BC9FC. The screen's `+20h` being the update
rather than the exit slot is settled by aligning 00CF0ED8 against the base table in
`docs/FRONTEND_STATE_MACHINE.md`: `+00h` is 005B9E90 (`MOV EAX,33h`, the registry index 33h),
`+10h` is 005BB130 `BSP_HudNarrativeScreen_Register`, `+1Ch` is 005B5C00 (`RET`, exit takes no
arguments) and `+24h` is 005BB0F0 (collect children).

`game+21F0h` is zero while `game+634h` (cinematic) is set (`docs/GAME_FRAME_CONTROL.md`), so a fade
does not advance during a cinematic.

## The camera movie in between

`luaIntroMovie` (`usn_2_java.lua:617`) calls `luaIngameMovie(tab, luaIntroMovieEnd, true)`, whose
first act is `Blackout(true, "luaIngameMovieBOStart", 1)` (`commandhelpers.lua:7783`). The
completion callback runs `luaCamIngameMovieAuto`, which ends with

```lua
return luaDelay(luaCamOnTargetExt, callbackTime, "CB", callback, "select", false)
```

(`commandhelpers.lua:7728`). **There is no native camera-movie completion.** The movie ends through
the delayed-call scheduler and the think walk 00929460, both already reconstructed by packet
`cc_lua_binding_audit`. The `AddListener("input", "IngameMovieInputListenerID")` calls are the
skip-with-a-button path only, and `IsListenerActive` answering false makes `luaCamOnTargetExt` skip
its `RemoveListener`. A headless run satisfies the movie, and this one did: `callbackTime` is
`starttime - 1` = 19 s for the six intro positions, and `luaIntroMovieEnd` fired 19 simulated
seconds after `luaIngameMovieBOStart`.

## Coverage

| Routine | Address | Reconstruction | Coverage |
| --- | --- | --- | --- |
| `Blackout` binding | 008D1340 | `mission_blackout_binding_008d1340` | complete; the Lua call-frame marshalling stays in the executable's binding trampoline |
| arm | 005B9BA0 | `mission_blackout_arm_005b9ba0` | complete |
| update and callback | 005B9800 | `mission_blackout_update_005b9800` | complete |
| screen update | 005BC920 | none | partial: the fade sub-pass only. 005BBF10, 005BC640, 00735100, 005B8C30 and the four child screens at `+34h`..`+40h` are unread |
| session broadcast | 0076D310 | none | read, not reconstructed |
| transition predicate | 005B66D0 | none | complete: `manager+20h` in {29h, 2Bh, 2Ch, 2Dh} |
| field store | 00644220 | none | complete: `[this+1Ch] = argument` |

Host methods in call order, with native call sites, are the `host_steps` list of
`reports/mission_blackout.json`.

## Run-time evidence

```
build/win32/Release/bsp_game.exe --frames 2700 --press-start-frame 30 --menu-select USN02
  --mission-frames 2500 --mission-frame-seconds 0.05 --log local/blackout_run.log
  --xlive-dll build/win32/Release/xlive_stub.dll
  --game-root "I:/SteamLibrary/steamapps/common/Battlestations Pacific"

  Blackout(true, "")                     target=1.000 remaining=0.0000   usn_2_java.lua:405
  Blackout(true, "luaIngameMovieBOStart") target=1.000 remaining=1.0000
  Blackout(false, "")                    target=0.000 remaining=0.5000
  blackout callback luaIngameMovieBOStart ran
  Blackout(true, "luaIn")                target=1.000 remaining=3.0000
  Blackout(false, "")                    target=0.000 remaining=1.0000
  blackout callback luaIn ran
  Blackout(true, "luaMoveToPh2")         target=1.000 remaining=1.0000
  blackout callback luaMoveToPh2 ran
  blackout callback luaIngameMovieBOStart ran

summary mission blackout: arms=8 updates=2500 completions=7 callbacks=4 interface_requests=0
        last_callback=luaIngameMovieBOStart level=0.000 remaining=0.000 configured_default_used=0
summary mission script timers created=11 think_registrations=11 waits=10 clears=0 deletes=9
        passes=2500 fires=79 failures=29
host methods 632 concrete, 478 unimplemented
```

The first arm is `Blackout(true, "", true)` with a **boolean** argument 3, the branch at 008D1521,
and it produced `remaining=0.0000` — the epsilon reading confirmed at run time.

`arms=8` against `callbacks=4` is correct: the other four arms pass an empty name. `completions=7`
against 8 arms counts only the per-frame pass; the zero-duration arm completes inside 005B9BA0's own
step and the executable's counter does not see it.

**Nine bindings the mission had never reached before now run**: `SetSelectedUnit`, `Objectives_Add`,
`DisplayUnitHP`, `GetHpPercentage`, `HideUnitHP`, `Objectives_Completed`, `GenerateObject`,
`AddDamage`, `GetPosition`. `Mission.MissionPhase` reaches 1 in `luaIn` and then 2 in
`luaMoveToPh2`.

**Where it stops now.** `usn_2_java.lua:563`, 29 times:
`***ERROR: luaGetDistance's second param is a non-table parameter`. `Mission.EscapePoint` is
`FindEntity("EscapePoint")` (`usn_2_java.lua:380`) and `FindEntity` 00898E30 is still a record: 38
calls, all neutral. Neither `MissionComplete` nor `MissionFailed` is reached.

One caveat on the phase-1 transition: `GetHpPercentage` answers 0 because the unit health vtable
`+110h` is unimplemented, so `GetHpPercentage(Mission.DeRuyter) < 0.15` was true on the first check.
Phase 1 closed for the wrong reason. The transition machinery is proven; the predicate is not.

The 3000-frame variant (`--frames 3200 --mission-frames 3000`) reaches the same state:
`arms=8 updates=3000 completions=7 callbacks=4`. The short line (`--frames 400
--mission-frames 120`) reaches only the `luaStageInit` arm and ends with the screen black,
`level=1.000`, which is what the script asks for.

## Corrections

- **`+C8h` is remaining time, not a deadline.** Was (`docs/LUA_BINDING_MISSION.md`): 005B9BA0
  "stores `duration + [00D7A268]` at `this+0C8h`", read as a clock deadline, with 00D7A268 taken as
  a time base. Is: 00D7A268 is the double **1.0e-4**, an epsilon, and `+C8h` is a countdown that
  005B9800 decrements by the step. Evidence: the bytes at 00D7A268 are
  `00 00 00 E0 E2 36 1A 3F`, and 005B98D3/005B98D5 store `remaining - step` back into `+C8h`.
- **The fade was never unadvanced; the binding was a record.** Was: "nothing in this process
  advances that object, so the name is never called back". Is: the native advance is 004C40F0 at
  004C429A through the screen's vtable `+20h`, a pass the executable already runs. The name was
  never called back because `Blackout` itself was a record, so nothing was ever armed.
- **Follow-up 2 `ingame_movie_listener` is not a gate.** Was: "the camera-movie listener
  `luaIngameMovie` depends on ... the second gate, immediately after the first". Is: the movie ends
  through `luaDelay` and the think walk, both already reconstructed; the listener is the skip path.
  Evidence: `commandhelpers.lua:7719-7732` and `7824-7856`, and the run, in which
  `luaIntroMovieEnd` fired without any listener being implemented.
- **The `Blackout_Icon` widget has three virtuals, not two.** Was
  (`docs/INTERFACE_RUNTIME_TAIL.md` and `include/bsp/interface_runtime_tail.hpp`): "the two virtuals
  it drives are the widget's set-visible `+34h` and set-colour `+50h`". Is: 005B9800 also calls
  `+54h` at 005B99C9 to read the colour back before overwriting its alpha. Evidence:
  005B99B9..005B99C9. The header was not edited; it is leased elsewhere and its own statement about
  005B6960 remains true.
- **00CF0ED4 is not this screen's vtable.** Was (a reading available from the raw xrefs): 005BC920
  sits at 00CF0EF8, which is `+24h` of the vtable stored at 005B917E and 005BABD7, so the routine
  looked like `collect_children`. Is: those two stores belong to a different, one-slot class; the
  screen's own vtable pointer is 00CF0ED8 (005BA7EF), and under that base 00CF0EF8 is `+20h`, the
  update. Evidence: the slot-by-slot alignment against the base table in
  `docs/FRONTEND_STATE_MACHINE.md`, listed in the "What advances it every frame" section.

## Follow-up packets

1. **`mission_find_entity`** (00898E30). `FindEntity` returns nil, so `Mission.EscapePoint`,
   `FindEntity("Witte")` and 36 other lookups are nil. It is now the single gate between the run and
   phase 2's victory test, and the exact successor to this packet.
2. **`unit_health_accessor`** (00923BE0 and the unit vtable `+110h`), already follow-up 4 of
   `docs/LUA_BINDING_MISSION.md`. Until it is read, `GetHpPercentage` answers 0 and phase 1 closes
   on a false positive.
3. **`hud_narrative_screen_update`** (005BC920 with 005BBF10, 005BC640, 00735100, 005B8C30 and the
   four child screens at `screen+34h`..`+40h`). The rest of the per-frame pass this packet read only
   as far as the fade.
4. **`blackout_other_callers`** (0045DB40, 004660B0, 0076F4C0). The three non-Lua callers of the
   arm. They decide the fades the front end and the session driver run, and none was read here.
5. **`ingame_interface_field_1c`** (00644220's other callers 0068C1F0, 0068CC70, 007712F0). The
   store's body is trivial; what `*(00E198C4+40h) + 1Ch` means is not.

## no_ghidra_function

| Start | Inclusive end | Evidence |
| --- | --- | --- |
| 005BC920 | 005BCA16 | **Start:** `BSP_VoiceManager_Update` ends at 005BC918 and 005BC91A-005BC91F is `int3` padding; 005BC920 opens `PUSH ECX / PUSH EBX / PUSH ESI / MOV ESI,ECX / ... / PUSH EDI`, a `__thiscall` prologue with one spilled local. Its address is referenced exactly once, from the vtable slot 00CF0EF8, which is how the routine is entered. **End:** `RET 4` at 005BCA14, three bytes `C2 04 00`, so the inclusive end is 005BCA16 and 005BCA17 begins `int3` padding. `RET 4` matches the one stack argument that the base table in `docs/FRONTEND_STATE_MACHINE.md` gives slot `+20h`. All bytes read with `bsp.py disasm-raw` from the disk image; Ghidra state not consulted. |

`reports/mission_blackout.json` carries this site as `undefined_call_sites` rather than as an
`address`/`native` row, because `tools/verify_report_calls.py` cannot bind a call site that lies in
no Ghidra function. After the integrator runs
`python tools/ghidra_define_function.py 005bc920 005bca17`, the row `005BC9FC -> 005B9800` becomes
checkable and can move into `host_steps`.
