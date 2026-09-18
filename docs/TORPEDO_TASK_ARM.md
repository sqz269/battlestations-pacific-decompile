# The torpedo task arm and the ordnance release (packet `cc8_torpedo_task_arm`)

Addresses: `009D4850`, `009D4030`, `009D3F60`, `009D3E40`, `009D3210`, `009D31D0`, `009D31B0`,
`009D3150`, `009D49A0`, `009D2720`, `009D25A0`, `009D2570`, `009D24E0`, `009D2530`, `009D3E80`,
`009D15F0`, `009D15D0`, `009D3420`, `009D1500`, `009D1360`, `009D4E30`, `009D3050`, `009D2DA0`,
`0099ACD0` (`0099AF60`-`0099AFCC`), `007BBBA0`, vtables `00D213C8`, `00D212C4`, `00D212E0`,
`00D212FC`, `00D21320`.

The packet brief listed five "unread release sites". All five are accounted for below, and the
one thing the previous packet left open - what raises `state+98h` on the done/prepare class - is
settled: `009D49A0`, the task vtable's slot `+24h`, reached from `BSP_PilotBot_Tick`.

## (1) `009D4850`, the per-tick arm

Torpedo task vtable `00D213C8`, slot `+64h` (the dword at `00D2142C`). **No Ghidra function.**
Raw listing `009D4850`-`009D4965` inclusive, `INT3` padding from `009D4968`.

ABI: `void __thiscall(BotTaskTorpedo* task, float dt)`, `RET 4`. `ECX` = task; the caller
`009998A0` reserves the argument slot with `PUSH ECX` and fills it with `FSTP [ESP]`, exactly as
`docs/PILOT_TASK_HEADING_ARM.md` records for the other thirteen arms.

| step | rule | address |
| --- | --- | --- |
| 1 | `task->+49Ch = 0FFh` (the depth charge uses `+470h`) | `009D4865` |
| 2 | `009D3420(task+3F8h, dt)` - the approach update | `009D486F` |
| 3 | `speed = (approach->+134h >= [00CF3F20]) ? approach->+7Ch : approach->+80h`; `[00CF3F20]` is the **double** `15.0` | `009D4874`-`009D4895` |
| 4 | `009BDE80(task+544h, approach->+78h + approach->+74h, the same sum again, speed)` - refresh the `moveto` ranges | `009D48AC`-`009D48CF` |
| 5 | `009D4030(task, dt)` - the transition rule, before the state's own tick | `009D48DE` |
| 6 | `task->+310h->vtable[+Ch](dt)` - the current state's tick | `009D48E7`-`009D48F6` |
| 7 | `task->+2E4h = task->+49Ch` | `009D48FF`-`009D4905` |
| 8 | when `task->+424h > 0`, `task->+3FCh != 0`, the embedded object at `unit+72Ch` answers its `vtable[+38h]`, and the state is none of `aim` `+710h`, `attackrun` `+6B4h`, `prepare` `+740h`: `007BBBA0(unit)` and `task->+424h -= 1` | `009D490B`-`009D495B` |

Step 4 is the same shape as the depth charge's step 3 (`009A66C0` step 3), with the range from
`docs/BOT_TASKS.md`'s torpedo row (`approach->+78h + approach->+74h`) and the fourth argument
supplied by step 3 rather than by `task->+438h`.

Step 8 is the **manual-release passthrough**, the counterpart of the depth charge's `009A67AB`.
The torpedo list omits `turnto`, which this class does not have. The receiver is the object
**embedded at** `unit+72Ch`: `009D491D` loads `ECX` with `EAX + 72Ch`, the address of the field,
while `009D4917` loads the vtable from `[EAX + 72Ch]`. The depth-charge doc's phrasing
`(*(unit+72Ch))->vtable[+38h]()` reads as a pointer dereference; on this site it is an embedded
sub-object. See "Corrections".

## (2) The transition rule, `009D4030`

Body `009D4030`-`009D4222`, `__fastcall(task)`. The state pointers it compares are the seven of
`docs/BOT_TASKS.md`: `moveto +544h`, `follow +580h`, `done +618h`, `attackrun +6B4h`,
`goaway +6D8h`, `aim +710h`, `prepare +740h`. `ctl` is `task+404h` = `approach+0Ch` = `unit+9D4h`.

The *attacking* test the depth charge makes with a call (`009A5420`) is inlined here as the five
pointer compares at the head; `009D31D0` is the same test as a function, used at step 6.

| step | rule |
| --- | --- |
| 1 | `attacking` = the state is one of `aim`, `done`, `goaway`, `attackrun`, `prepare` |
| 2 | `engaged` = `009D3210(task)` |
| 3 | `!attacking && engaged` -> `009D3F60(task)`, the entry chooser |
| 4 | `!attacking && !engaged` -> `SetState(007B8AD0(unit) ? moveto : follow)` |
| 5 | `attacking && !engaged` -> the same `moveto`/`follow` choice, the abort back to approach |
| 6 | `attacking && engaged && ctl->+370h == 0` -> `SetState(prepare)` |
| 7 | state is `done` -> nothing further |
| 8 | `009D31D0(task, state)` and `task->vtable[+1Ch]()` (`009D4C10`) -> `SetState(done)` |
| 9 | `prepare` -> `009D3F60(task)` |
| 10 | `attackrun` -> `task->+529h != 0` -> `SetState(aim)` (the depth charge goes to `turnto`) |
| 11 | `aim` -> stay while `task->+52Ah != 0 && !009D31B0()`; otherwise `SetState(goaway)` |
| 12 | `goaway` -> `009D3150()` -> `SetState(task->+52Ah ? aim : done)` |

### `009D3F60`, the entry chooser

Body `009D3F60`-`009D4023`. The analogue of `009A57D0`.

| order | test | next state |
| --- | --- | --- |
| 1 | `ctl->+370h == 0` | `prepare` `+740h` |
| 2 | `task->+52Ah == 0 && (ctl->+369h == 0 \|\| [00E17BF2] == 0)` | `done` `+618h` |
| 3 | `task->+529h == 0` | `attackrun` `+6B4h` |
| 4 | otherwise | `aim` `+710h` |

Row 2's second clause has no depth-charge counterpart. `009D3E40` (`009D3E40`-`009D3E70`) is the
setter, the same three steps as `009A56C0`.

### The two flags, and where they come from

`task+529h` and `task+52Ah` have **no writer anywhere in the image** under those byte offsets: the
full-image scans for `C6 ?? 29 05 00 00 ??`, `88 ?? 29 05 00 00` and the `+52Ah` pair return
nothing. They are approach-relative: `529h - 3F8h = 131h` and `52Ah - 3F8h = 132h`, and the
writers of `approach+131h`/`+132h` are `009D361E`/`009D3E21` and `009D34CD`, all inside the
approach update `009D3420`, plus `009D05B5` in `009D0380`. The same holds for `task+484h`/`+488h`
= `approach+8Ch`/`+90h`, which `009D3150` and the attackrun tick already read as approach fields.

So **every input of the torpedo state machine is an output of `009D3420`**, a 2592-byte routine
(`009D3420`-`009D3E3F`) this packet did not read: `contract: unread`.

## (3) Where the release range is established

Not in `aim`. `009D15F0` (aim tick, vtable `00D212FC` slot `+Ch`, no Ghidra function, raw listing
`009D15F0`-`009D2377` inclusive, `RET 4`, 3464 bytes) contains **no `007BBBA0` call and no store
to `state+98h`**. `coverage: partial` - it was scanned for its writes, calls and end, not
transcribed. It reads `009D1500` at `009D160A` and writes the command block.

The range lives in the **done/prepare tick `009D2720`** (vtable `00D21320` slot `+Ch`, no Ghidra
function, raw listing `009D2720`-`009D2CF1` inclusive, `RET 4`, `__thiscall(state, float dt)`).
`ESI = state+4` throughout; `EBX = state`.

`state+98h` is a **release countdown**, not a flag. `009D24E0` (the constructor) and `009D2530`
(the enter slot) both set it to `[00D7A260]` = `-1.0f`.

**With `state->+98h > 0`** (`009D2759` onward):

| step | rule | address |
| --- | --- | --- |
| 1 | `009C1FD0(state, dt)` - the follow base's tick | `009D2731` |
| 2 | `approach->+A4h = 3` (the weapon selector; the depth charge uses `approach->+78h`) | `009D2761` |
| 3 | `approach->+1Ch->+40h = 0`, `cmd->+2C8h = [00CE69C8] = 0.3f` | `009D2770`, `009D277A` |
| 4 | `state->+98h -= dt`; on reaching 0 or below, `009D25A0(state, 0)` and `state->+98h = -1.0f`, then return | `009D2782`-`009D27B3` |
| 5 | with the countdown still positive and `ctl->+3D0h` set: `009D1360(approach)`, then the target point through `approach->vtable[0]`, `LIBCRT_atan2`, `unit->vtable[+50h]` for the heading and `BSP_Math_SubtractWrappedAngle`, folded to an absolute error at `009D2882` | `009D27CF`-`009D2890` |
| 6 | `t = 009D1500(approach)` | `009D289A` |
| 7 | `t > [00CF87C8] = 2.5` **or** `error > [00CF8858] = 1.3962635 rad (80 deg)` -> `007BBBA0` at `009D29CB`, `approach->+2Ch -= 1`, return. The countdown is **not** cleared | `009D28B1`, `009D28C4` |
| 8 | `t < [00CE380C] = 1.5`: `allowed = InterpolateClamped([00CE3800] = 0.5, [00D05AAC] = 60 deg, [00CE3814] = 1.2, [00D05AA8] = 15 deg, t)`; when `allowed > error` and `unit->+C68h < [00CEC724] = 30 deg` -> `007BBBA0` at `009D2938`, `approach->+2Ch -= 1`, `state->+98h = -1.0f`, return | `009D28D7`-`009D294E` |
| 9 | otherwise steer: `cmd->+2BCh = 0`, `cmd->+2D0h = 2`; with `state->+98h < 1.0f` also `cmd->+2C4h = 0` and `cmd->+2CCh = 1`; otherwise `009F9E40` on the target point from `approach->vtable[0]` | `009D295F`-`009D29B7` |

`t` is a **time to target in seconds**, not a distance: the `state->+98h <= 0` branch recomputes
the same quantity inline at `009D2A35`-`009D2A52` as
`Vector2Length(target.xz - unit.xz) / ((approach->+14h)->+8h * approach->+24h)`, a length over a
speed. Both cones therefore tighten in time, not in metres: 60 deg at 0.5 s down to 15 deg at
1.2 s in the tick, 60 deg at 1.0 s down to 30 deg at 2.0 s in the helper.

**With `state->+98h <= 0`** (`009D29E0` onward) the routine needs `ctl->+3D0h`, recomputes `t`,
gates on `unit->+100h < [00CE3AE8]` and on `InterpolateClamped(...) * t < [00D0C308]`, writes
`approach->+1Ch->+40h` from the tuning singleton's `+66Ch`, the throttle `cmd->+2C8h`, the weapon
selector `approach->+A4h = 3` again, and finally **`state->+8Ch`** at `009D2CCD` (or `1.0f` at
`009D2CE2`). It never releases and never raises `+98h`. `coverage: partial` - the interpolation
chain between `009D2AE2` and `009D2CBC` is transcribed only as far as its constants.

### `009D25A0`, the gated release helper

Body `009D25A0`-`009D2719`, `__thiscall(state, bool force)`, one pushed argument at `009D27A2`.
Its only caller is the done/prepare tick.

* `force` -> `007BBBA0` at `009D25B3`, `approach->+2Ch -= 1`.
* otherwise: `t = 009D1500(approach) < [00CE3958] = 2.0`, the absolute bearing error below
  `InterpolateClamped(1.0f, [00D05AAC] = 60 deg, [00CE3958] = 2.0, [00CEC724] = 30 deg, t)`, and
  `unit->+C68h <= [00CE74F4] = 35 deg` with equality refused -> `007BBBA0` at `009D26F8`,
  `approach->+2Ch -= 1`.

## (4) What raises `state+98h`: `009D49A0`, task vtable slot `+24h`

Body `009D49A0`-`009D49DE`, `__fastcall(task)`; the only reference is `00D213EC`, slot `+24h` of
the torpedo task vtable. `docs/BOT_TASKS.md` lists slot `+24h` as `contract: unread` with base
`0099B6A0` and the depth-charge override `009A5D00`.

```
if (task->+52Ah != 0) {
    if (task->+310h == prepare (+740h)) { *(float*)(task + 7D8h) = [00CE3D34]; return 1; }
    if (!009D3210(task)) return 0099B6A0(task);
}
return 1;
```

`task+7D8h` is `prepare + 98h`: the task is `7DCh` and the state is `9Ch` wide, so the countdown
is the task's last dword. A full-image byte scan for every store form to a `+98h`, `+6B0h`,
`+7D8h`, `+2B8h` and `+3E0h` displacement finds this site and no other positive writer.

The caller is `BSP_PilotBot_Tick` `0099ACD0`, at `0099AF90`-`0099AFAF`:

```
0099af2c  if (unit->+9C2h[[00F876B8]*8] != 0) { bot->+74h = [00CE3854]; goto elsewhere }
0099af53  if (unit->+C58h <= 0) goto end
0099af5c  if (!(unit+72Ch)->vtable[38h]()) { unit->+C58h = 0; goto end }
0099af6d  if (unit->+5Ch == 0) { unit->+C58h = 0; goto end }
0099af76  if (!007B9140(0))     { unit->+C58h = 0; goto end }
0099af81  for (task in bot->+58h[0 .. bot->+5Ch))
              if (task && task->vtable[24h]()) { unit->+C58h -= 1; break; }
```

So `unit+C58h` is a **queued release-order count**: the pilot bot spends one per tick by offering
it to each task in turn, and the torpedo task takes it by arming the countdown. `007B9140` and
the `unit+5Ch` byte were not read: `contract: unread`.

## (5) The five "unread" release sites

| site | where it lives | on the ordered-attack path? |
| --- | --- | --- |
| `009D2938` | `009D2720`, the done/prepare tick, the in-cone drop | yes - this is the aimed release |
| `009D29CB` | `009D2720`, the abort drop when the solution decayed | yes |
| `009D26F8` | `009D25A0`, the gated arm, reached only from `009D27A6` on countdown expiry | yes |
| `009D25B3` | `009D25A0`, the `force` arm | no site passes `force != 0`; the one caller passes `0`. Dead for a plane task unless another caller appears |
| `009D3EB6` | `009D3E80`, task vtable slot `+58h`, the dump of every remaining `task+424h` round | not per tick; a task-teardown path |
| `009D4956` | `009D4850` step 8, the manual passthrough | yes, but only outside `aim`/`attackrun`/`prepare` |
| `009D258A` | `009D2570`, the done/prepare exit slot, when `state+98h >= [00D7A218] = 0.0f` | yes - leaving the state with the countdown still armed forces the drop |

## (6) Which command selects kind `Eh`

`009D4E30 BSP_BotTask_MakeTorpedo` has exactly one caller, `0099A170 BSP_Bot_InstallCommandTask`
at `0099A30F`; `ghidra xrefs` and a rel32 scan agree. `docs/ATTACK_COMMANDS.md` gives the gate:
command class `torpedo` `00E08F18`, chosen at `007EEA40`, requires torpedo ordnance (weapon-device
kind `2Bh`, `007ED8D0` -> `007B93F0`), a surface target when the torpedo is a surface runner, and
`00828EC0(target+538h) == 0`.

**A `PilotSetTarget` order on a torpedo-armed plane does build this task.** The run below shows
`007EEC50` choosing `00E08F18` for all five USN01 aircraft and `0099A170` returning a task.

## (7) The three per-tick sub-objects

`docs/PILOT_BOT_TASK_OBJECT.md` puts them after the arm on `009998A0`'s slow path. Read only far
enough to state their ABI; all three are `contract: unread` bodies.

| address | ABI | what it is |
| --- | --- | --- |
| `009FC7C0` | body `009FC7C0`-`009FCEE0`; `task+314h` sub-object | reads `BSP_GameTuning_GetSingleton`, the world matrix and pose, `BSP_UnitInstance_IsAliveAndVisible`, `007B96D0`. Gated on `unit+C24h` |
| `009FD0E0` | body `009FD0E0`-`009FD550`, `RET 4`, `__thiscall(this, float dt)`; `task+38Ch` sub-object | 344 instructions, 27 calls, and it **calls `007BBBA0`**: a release path independent of the task states. Not modelled |
| `009A17D0` | body `009A17D0`-`009A19A2`; called with the task | three callees only (`0099B670`, `0099EC40`, `0099F1C0`) |

## ABI summary

| address | ABI | evidence |
| --- | --- | --- |
| `009D4850` | `void __thiscall(task, float dt)`, `RET 4` | `009D4965 RET 4`; caller's `PUSH ECX`/`FSTP [ESP]` |
| `009D4030` | `void __fastcall(task)` | no stack cleanup at the call site `009D48DE`... the site pushes `dt`, and the callee is `RET 4`; the decompiler drops the unused float |
| `009D2720` | `void __thiscall(state, float dt)`, `RET 4` | `009D295A`, `009D29A6`, `009D29C3`, `009D2CD7`, `009D2CF1` |
| `009D25A0` | `void __thiscall(state, char force)`, `RET 4` | one `PUSH 0` at `009D27A2` |
| `009D49A0` | `bool __fastcall(task)`, `RET 0` | called as `task->vtable[24h]()` with no push at `0099AF9B` |
| `009D15F0` | `void __thiscall(state, float dt)`, `RET 4` | `009D2377` |
| `009D2530` | `void __thiscall(state)`, `RET 0` | `009D256E` |
| `009D24E0` | `state* __thiscall(state, owner)`, `RET 4` | `009D2510`, `EAX = ESI` |

## Host methods

One row per native call site the reconstruction models.

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `009D4865` | - | `set_plan_step_scratch` | `task / 0FFh / void` | always |
| `009D486F` | `009D3420` | `update_torpedo_approach_009d3420` | `task+3F8h / dt / void` | always |
| `009D48CF` | `009BDE80` | `refresh_move_to_ranges` | `task+544h / range, range, speed / void` | always |
| `009D48DE` | `009D4030` | `read_transition_inputs` + `set_state` | `task / dt / void` | always |
| `009D48F6` | `state->vtable[0Ch]` | `tick_state` (or the done/prepare rule) | `state / dt / void` | always |
| `009D4905` | - | `commit_plan_step_result` | `task / - / void` | always |
| `009D4923` | `(unit+72Ch)->vtable[38h]` | `manual_release_requested` | `unit+72Ch / - / bool` | `task+424h > 0` |
| `009D4956` | `007BBBA0` | `request_ordnance_release` | `unit / - / void` | step 8 of the arm |
| `009D2731` | `009C1FD0` | `follow_base_tick_009c1fd0` | `state / dt / void` | done/prepare tick |
| `009D27D1` | `009D1360` | `approach_committed_hook_009d1360` | `approach / - / void` | countdown positive |
| `009D289A`, `009D160A` | `009D1500` | `time_to_target_009d1500` | `approach / - / float` | countdown positive |
| `009D27EB`-`009D2890` | `approach->vtable[0]`, `LIBCRT_atan2`, `unit->vtable[50h]`, `00438B10` | `bearing_error_to_target` | `approach, unit / - / float` | countdown positive |
| `009D29B7` | `009F9E40` | `steer_toward_target_009f9e40` | `approach / point / void` | countdown >= 1.0f |
| `009D26F8`, `009D2938`, `009D29CB` | `007BBBA0` | `request_ordnance_release` | `unit / - / void` | the three release gates |
| `009D49C2` | - | `write_drop_timer` | `prepare / [00CE3D34] / void` | `task+52Ah` and state == prepare |
| `009D4C10` | `task->vtable[1Ch]` | `should_break_off` | `task / - / bool` | transition step 8 |

The consumer of `007BBBA0` - the device at `unit+DECh` and the projectile spawn behind
`dev->+11h` - is this packet's **contract**, logged through the unimplemented-host mechanism.

## Corrections

### Correction to `docs/BOT_TASK_STATES.md` (packet `cc7_bot_task_states`)

That doc's torpedo section says `009D25A0` "is its sibling with an explicit `bool` argument gating
the same two lines (`CMP byte ptr [ESP+1Ch],0` at `009D25A3`, `RET 4`)". The function is
`009D25A0`-`009D2719`, `379` bytes, and its non-forced arm is the whole range/bearing release
gate transcribed in section (3) above. Only the `force` arm is the two lines.

The same section says the five remaining sites "none of the five was read" and that the torpedo
release "fires on leaving `prepare`/`done` rather than from `attackrun`". The exit-slot drop is
real, but it is the **fallback**: the primary release is the done/prepare **tick** `009D2720`, and
the exit only fires when a countdown is still outstanding.

Its `009A66C0` step 7 reads the manual passthrough's receiver as `(*(unit+72Ch))->vtable[+38h]()`.
On the torpedo arm the equivalent site loads `ECX` with `unit+72Ch` itself (`LEA ECX,[EAX+72Ch]`
at `009D491D`), so the object is embedded at `unit+72Ch` rather than pointed to from it. The
depth-charge site was not re-read here; the two may differ.

### Correction to `docs/BOT_TASKS.md` (packet `cc2_bot_tasks`)

Slot `+24h` of the task vtable is listed as `contract: unread`. For the torpedo it is
`009D49A0`, the ordnance-arming entry, called from `BSP_PilotBot_Tick` `0099ACD0` at `0099AF9B`
over the bot's whole task vector. That is also the consumer of `unit+C58h`.

### Correction to the packet brief

The brief places the aim tick "`009D15F0` inside `FUN_009d15d0`". Ghidra's `FUN_009d15d0` is
`009D15D0`-`009D15EE`, the aim state's **enter** slot (`00D21300`); `009D15F0` starts after it and
has no Ghidra function of its own.

## `no_ghidra_function`

Routines named from the raw listing that Ghidra does not define, with inclusive end addresses:

| start | end (inclusive) | name |
| --- | --- | --- |
| `009D4850` | `009D4965` | `BSP_BotTaskTorpedo_TickArm` |
| `009D2720` | `009D2CF1` | `BSP_BotStateTorpedoDone_Tick` |
| `009D15F0` | `009D2377` | `BSP_BotStateTorpedoAim_Tick` |
| `009D24E0` | `009D2510` | `BSP_BotStateTorpedoDone_Construct` |
| `009D2530` | `009D256E` | `BSP_BotStateTorpedoDone_Enter` |
| `009D2520` | `009D2523` | an unnamed two-instruction `return this->+4h` getter |
| `009D2938`, `009D29CB`, `009D27A6` | - | call sites inside `009D2720` |

## Validation

`./scripts/build.ps1` (MSVC Win32, `/W4 /WX`) succeeds; `ctest` passes both existing suites
(`reconstructed_math`, `tool_tests`). No test was added.

Runtime validation needed a local XLive stand-in at the time these runs were made. `main` at
`dd0d274df` did not run: commit `e5a4842ec` made `src/native_renderer_end_frame.cpp` resolve
ordinal `5002` out of the XLive library and `tools/xlive_stub/xlive_stub.def` did not export it,
so every fresh build died with `startup failed: loaded XLive library lacks ordinal 5002`. That
file was leased to `agent/cc8-bank-inputs`, so this packet did not touch it; the runs below used
a throwaway copy of the stub with `XLiveRender @5002` added, built under `local/xstub/` and never
installed. The stub itself was fixed on `agent/cc8` (`651b58bd1`, see `docs/XLIVE_STUB.md`), and
the fixed-stub USN02 baseline reproduces these numbers exactly.

| run | result |
| --- | --- |
| USN02 before (`main` build of 2026-09-14) | the standing baseline |
| USN02 after | `shots=734 first_shot=1.40 s`, `hull=180 part=0 deaths=2 total_damage=18525.6`, `projectiles created=734` - **identical to the milestone 2t baseline** |
| USN02 torpedo census | `no ordered aircraft carries torpedo ordnance (kind 2Bh), so 0099A170 builds no kind Eh task` |
| USN01 | five aircraft ordered by `PilotSetTarget`; `007EEC50 -> 00e08f18` (torpedo) for all five, `torp=1` |
| USN01 torpedo census | `Mav1`..`Mav5`, each `arm_ticks=1299 transitions=0 states[moveto=1299] releases=0` |
| USN01 summary | `aircraft=5 releases=0 blocked_engaged_009d3210=6495 blocked_arm_009d49a0=0` |

**No release happens, and the blocking gate is `009D3420`.** `009D3210` refuses on every tick
because its inputs `task+484h`/`+488h` (`approach+8Ch`/`+90h`) are both `0.0f` and
`task+529h` (`approach+131h`) is `0`, so `0.0f * [00D05AC8] <= 0.0f` fails the range test at
`009D325B`. With `engaged` false the rule never leaves `moveto`, `009D49A0` never sees `prepare`,
`prepare+98h` stays at `-1.0f`, and `009D2720` never reaches `007BBBA0`. Every one of those values
is produced by `009D3420`, which this packet did not read.

## Follow-up packets

1. **`009D3420`, the torpedo approach update** (`009D3420`-`009D3E3F`, 2592 bytes). It writes
   `approach+131h`/`+132h`, `+8Ch`/`+90h`, `+74h`/`+78h`, `+7Ch`/`+80h`, `+134h` and `+98h`.
   Nothing in the torpedo state machine moves until it is read. This is the single highest-value
   follow-up.
2. **`009D15F0`, the aim tick** in full (3464 bytes). It has no release, so it is the heading and
   throttle producer of the run-in.
3. **`009D1500` and `009D1360`** - the time-to-target metric and the committed hook.
4. **`unit+C58h`**, the queued release-order count `0099AF53` spends: who raises it, and whether
   the player's fire button and the AI order share it.
5. **`009FD0E0`**, the `task+38Ch` sub-object, which calls `007BBBA0` on a path independent of the
   task states.
6. **The device at `unit+DECh`** and the projectile spawn behind `dev->+11h`, the consumer this
   packet contracts out.

## A note on `tools/verify_report_calls.py`

The checker's rule 2 requires a call site to lie inside a Ghidra function. Twenty-one of this
packet's call sites lie inside `009D4850`, `009D2720` and `009D15F0`, which Ghidra does not
define and which this worker may not create (Ghidra is read-only for the packet). Those rows sit
in `reports/torpedo_task_arm.json` under `calls_in_undefined_regions`, keyed `site`/`callee` so
the checker skips them, with their enclosing undefined region named on every row. The seven rows
the checker can validate pass: `5 call rows checked, 0 failed`, two more reported `indirect`.

## Correction from docs/TORPEDO_APPROACH_UPDATE.md (packet cc8_torpedo_approach_update)

1. Section (1) step 4 calls `approach->+78h + approach->+74h` "the moveto ranges". The aim tick
   `009D15F0` reads the same sum at `009D1631` as a floor on the commanded **altitude**, next to
   the over-water `5.0` and over-land `30.0` constants and the ground height; for the torpedo class
   the pair is an altitude band.
2. Follow-up packet 1 lists `+98h` among `009D3420`'s writes. `009D3420` only reads it;
   `009D1360` writes it at `009D14E7`, as the torpedo's own run time from release to impact.
3. The Validation section names `009D3420` as the USN01 blocking gate because `approach+8Ch`/`+90h`
   were both `0.0f`. `+90h` was zero because the host had no approach object to run, and `+8Ch` is
   not `009D3420`'s output at all: its one producer in the image is `009D4A70`, the task's cruise
   profile, which clamps it to `Pilot/Torpedo/AttackDist` times a speed ratio. The host binding of
   this packet also filled `TorpedoEngagedInputs::engage_range_scale` with `1.0f` where the native
   multiplies by the double `2.2` at `009D324F` (`[00D05AC8]`); that alone kept every tick blocked
   and is corrected on `main`. With `009D3420` wired, USN01's five ordered aircraft leave `moveto`
   for `prepare`, and the gate that remains is `unit+C58h` at `0099AF53`, the queued release-order
   count, zero on every tick.

## Correction from `docs/TORPEDO_GOAWAY_RELEASE.md` (packet `cc8_torpedo_goaway_release`)

Appended, not rewritten. This document records `009D3150` as returning `d < approach+90h` without
saying what `d` is. `d` is `goaway+24h`, the break-off distance, written once per entry into
`goaway` by the state's enter `009D0D90` at `009D0E37`:

```
d = BSP_Random_UniformFloatRange(1.0f, 1.15f) * max(0042E740()+438h, 007B5BE0(target))
```

`0042E740()+438h` is `Pilot/Torpedo/SafeDist`, default 700, so the break-off distance is 700 to 805
metres at the stock tuning, raised to the target's own extent when the target is larger. The
`009D3183` scale on the `ctl+369h` / `[00E17BF2]` arm is the double `0.4` at `00CE65D0`.
