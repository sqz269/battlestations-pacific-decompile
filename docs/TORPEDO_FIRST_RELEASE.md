# Who authorises the first ordnance release (packet `cc8_torpedo_first_release_authority`)

Addresses: `007BBBA0`, `007BB110`, `007CCFA0`, `007CD0B2`, `007CD0B4`, `009FA3A0`, `009FA3A3`,
`009FA3A9`, `009FA3BB`, `009FA3C5`, `009FA3D0`, `009FA3EA`, `009FA3FB`, `009FA405`, `009FA40A`,
`009D15F0`, `009D2021`, `009D2027`, `009D2272`, `009D2279`, `009D2282`, `009D2287`, `009D2720`,
`009D2753`, `009D2938`, `009D29CB`, `009D25A0`, `009D2DA0`, `009D3E80`, `009D4850`, `009D48F8`,
`009D4923`, `009D4956`, `009D49A0`, `009D49C2`, `009B9420`, `009B9480`, `007ED3C0`, `007EFB30`,
`007F2D1E`, `0099B6A0`, `00CE3D34`, `00D7A260`, `00D7A24C`, `00D21320`.

## The question

`docs/TORPEDO_ISSUE_TIMING.md` closed a loop: the flight-wide budget `unit+C58h` is issued by a
stage that needs `unit+C20h`, and `unit+C20h` is raised only by `007BBC00` at the tail of
`007BBBA0 BSP_Unit_RequestOrdnanceRelease`. Something must call `007BBBA0` without a budget, or
no torpedo flight ever drops.

## The answer

**The aim state's own release timer.** `009D15F0 BSP_BotStateTorpedoAim_Tick` holds a timer object
at `state+18h`. It arms it at `009D2287` and ticks it at `009D2027` through `009FA3A0`, which
calls `007BBBA0` at `009FA3D0` behind nothing but `007BB110`, the unit's own can-release
predicate. No queued release order, no `prepare` state, no manual passthrough count.

## The call-site census of `007BBBA0`

`python tools/callsite_census.py 007bbba0`: 34 sites, rel32 and absolute dword, image wide. By
owner:

| site | owner | guard in front of it | needs |
| --- | --- | --- | --- |
| `007CD0B4` | `007CCFA0 BSP_Plane_HandleMessage`, message `0C4h` | **none**: `007CD0B2 MOV ECX,EDI`, `CALL`, `MOV AL,1`, `RET 4` | nothing |
| `009FA3D0` | `009FA3A0`, the aim state's release timer | the enable byte, a negative countdown, `007BB110` | **nothing** |
| `009D4956` | `009D4850 BSP_BotTaskTorpedo_TickArm` | `009D48F8 CMP task+424h,0`; the unit; `[unit+72Ch]->vtable[38h]`; state is none of `task+710h`, `+6B4h`, `+740h` | `task+424h` |
| `009D3EB6` | `009D3E80 BSP_BotTaskTorpedo_DumpRemainingRounds` | `[unit+72Ch]->vtable[38h]`, then one call per `task+424h` | `task+424h` |
| `009D25B3`, `009D26F8` | `009D25A0`, the `torpedo/prepare` release | geometry only, but the state runs it only with `prepare+98h > 0` | the budget |
| `009D2938`, `009D29CB` | `009D2720`, the `torpedo/prepare` tick | geometry only, behind `009D2753 JBE` on `prepare+98h > 0` | the budget |
| `009D258A` | `009D2570`, the `torpedo/prepare` exit | the state leaving | the budget |
| `0099B6CD` | `0099B6A0` | `task+2FCh` non-null and `[unit+72Ch]->vtable[38h]` | reached only from `009D49A0`, so the budget |
| `009A4620`, `009A48EA`, `009A67AB` | the depth-charge task and its states | not read | **partial** |
| `00464905`, `007B5700`, `0089E9A0`, `0089ECC9`, `009A20DB`, `009A4909`, `009A4A89`, `009A5746`, `009AD2F0`, `009AD601`, `009AE106`, `009AE97E`, `009B5A1A`, `009B67E9`, `009B81B6`, `009B8C38`, `009C5777`, `009C60F1`, `009C8026`, `009C88C4`, `009FD503` | other bot tasks and unread owners | not read | **partial** |

Coverage is **partial** by design: every torpedo-owned site is read to its end, the depth-charge
trio and the twenty-one others are named but not read.

## Correction: the `torpedo/prepare` state was recorded as "done"

`009D2DA0`, the torpedo task's state construction, ends with `BSP_BotStateRegistry_Add` calls that
name every state. `param_1 + 0xD2` is registered **`"torpedo/prepare"`** and carries the vtable at
`00D21320`, whose slots are `009D2D00`, `009D2530`, `009D2570`, `009D2720`, `007B3DE0`,
`009BE590`. So `009D2570`, `009D25A0` and `009D2720`, carried in the ledger as
`BSP_BotStateTorpedoDone_*`, are the **prepare** state's exit, gated release and tick. `"torpedo/done"`
is a different state at `param_1 + 0x88`. The names are corrected in the ledger, with the old
values preserved in the appended evidence.

## The branch that a decompiler reverses

`009D2720`'s pseudocode reads `if (*(float *)(param_1 + 0x98) <= 0.0) { ... }` with the drop
inside. The listing says the opposite:

```
009d273c: MOVSS XMM1, dword ptr [EBX + 0x98]
009d2744: XORPS XMM0, XMM0
009d2747: COMISS XMM1, XMM0
009d2753: JBE 0x009d29e0
```

`JBE` leaves for `009D29E0` when `prepare+98h <= 0`, so the drop calls at `009D2938` and
`009D29CB` are in the `> 0` arm. `009D49C2` raising `prepare+98h` to `[00CE3D34] = 4.0` is what
**enables** the prepare drop, and `009D24E0`, `009D2530`, `009D27B3` and `009D294E` parking it at
`[00D7A260] = -1.0` is what disables it. The budget chain is therefore intact and needs the
escape above.

## `009FA3A0`, the release timer

`void __thiscall(timer, float dt)`, `RET 4` at `009FA40A`, `ECX` the timer, which the aim state
holds at `state+18h` (`009D2019 LEA EBP,[ESI+18h]`). Fields read from the listing: `+0h` the unit
(the `ECX` of both `007BB110` and `007BBBA0`), `+4h` and `+8h` the reseed bounds (`+4h` pushed
first at `009FA3E7`, so it is the low one), `+0Ch` the enable byte, `+0Dh` a notify byte, `+10h`
the countdown.

```
009fa3a3: CMP byte [ESI+0Ch], 0 / JZ out      ; a disabled timer does not advance
009fa3a9: FLD [ESI+10h] / FSUB step / FST back ; FST, so the value stays on the stack
009fa3bb: FLDZ / FCOMIP ST0,ST1 / JBE out      ; fires only on a strictly negative countdown
009fa3c3: MOV ECX,[ESI] / CALL 007BB110 / JZ out
009fa3d0: CALL 007BBBA0
009fa3d5: countdown = UniformFloatRange(1, [ESI+4h], [ESI+8h])
009fa3fb: CMP byte [ECX + slot*8 + 9C0h], 0 / JNZ out / MOV byte [ESI+0Dh], 1
```

Note `9C0h` at `009FA3FB`, not the `9C3h` the decompiler renders.

`007BB110` is the predicate form of `007BBBA0`'s own head: the device at `unit+DECh` enabled, its
`+64h` not `1.0`, and the per-slot byte clear.

## The arm at `009D2287`

```
009d2265: CALL 00903860                  ; the ground probe
009d2272: COMISS XMM0(1.0), [ESP+50h] / JBE skip
009d2279: CMP byte [EBP+0Ch], 0 / JNE skip  ; refuses to re-arm
009d2282: MOVSS [EBP+10h], XMM0(0.0)        ; the countdown starts at ZERO
009d2287: MOV byte [EBP+0Ch], 1
```

The countdown starting at zero is the point: the drop lands on the **next** aim tick, not after a
delay. `docs/TORPEDO_AIM_TICK.md` already records that all five release flags must coincide to
reach `009D2287`; this packet adds what the arm then does.

## `ctl+378h`, the force flag

`python tools/store_census.py 0x378` returns 27 sites, of which three are on the pilot control
block:

- `007F2D1E` in `BSP_PlaneSquadronTickableEntity_Construct`: `MOV byte [ESI+378h], 1`. **The flag
  is raised at construction**, so `007EEF30`'s follow-target test at `007EEF62` is bypassed from
  the squadron's first moment.
- `007ED3C7` in `BSP_PilotControl_ClearReleaseOrders`, called once, from `009B944D`.
- `007EFB4C` in `007EFB30`: `BSP_PilotControl_RefreshArmedFraction` then `ctl+378h = 1` when
  `ctl+390h <= ctl+374h`, exactly the condition under which the issue gate at `007EEF40` would
  refuse. Tail-jumped to once, from `009B9493`.

The two call sites belong to a virtual pair with no Ghidra function, `009B9420`-`009B947B` and
`009B9480`-`009B9498`, which sit at the adjacent vtable slots `00D206AC` and `00D206B0`: a stand
down that clears the count and the flag, and a free-fire that raises it. Neither is a first-drop
authority; both are flight-wide modifiers.

## `task+424h`, the manual passthrough

`009D48F8 CMP dword [ESI+424h], 0` guards `009D4956`, and `009D3E80` drains the count one
`007BBBA0` call at a time. Both also require `[unit+72Ch]->vtable[38h]`, the device's
release-requested predicate, which is the player's control.

**Its raiser exists, and the earlier packet's store census missed it for the same reason it
missed `007BBC00`:** it is an `ADD` with a register source. `local/scan_disp.py 0x424`, every
operand with that displacement, reads included, returns 85 sites; the raiser is

```
009c8216: lea ecx, [eax + 0x72c]        ; the device
009c821e: mov eax, dword ptr [eax + 0x38]
009c8221: call edx                      ; the release-requested predicate
009c8225: je  0x9c8252
009c8227: mov eax, 1
009c822c: add dword ptr [esi + 0x424], eax
```

in `FUN_009C8200`, behind the same `[unit+72Ch]->vtable[38h]` predicate. So `task+424h` counts
the frames the player has asked to release, and the manual passthrough is a player path, as
suspected. `009C7FF0` and `009C83E0`, two of the four callers the earlier packet left unexamined,
are the siblings that spend it (`009C802B` and `009C88C9` decrement it around their `007BBBA0`
calls). `FUN_009C8200`'s own caller chain is **not read**: partial.

## Message `0C4h` has no producer with an immediate

`local/scan_imm.py 0xc4`, sixteen alignments with resync over `.text`, returns eleven
instructions carrying `0C4h` as an immediate. None builds a message: `007E587D` and `007E9749`
are stack writes inside `BSP_GameTuning_LoadFromPlaneGlobals`, `008707F1` a push inside
`BSP_GameplayEffectDefinition_LoadComponents`, and the rest are in unrelated loaders. So the
unguarded arm at `007CD0B4` is reached only with a message whose kind byte comes from a variable,
the same shape `docs/TORPEDO_ATTACK_MODE.md` found for message `0BCh`. That makes `0C4h` a
player or network binding, not an AI path. **Partial**: the binding table was not found.

## ABI

| address | ABI | evidence |
| --- | --- | --- |
| `009FA3A0` | `void __thiscall(timer, float dt)`, `RET 4` | `009FA3A1 MOV ESI,ECX`; `009FA40A RET 4` |
| `007BB110` | `bool __fastcall(unit)` | `unit+DECh` at the head; called with `MOV ECX,[ESI]` |
| `007CCFA0` | `int __thiscall(unit, msg)`, `RET 4` | `007CD0C0 RET 4` |
| `009D49A0` | `int __fastcall(task)` | body `009D49A0`-`009D49DE`, task vtable `00D213C8` slot `+24h` |

## Host methods

- `bsp::release_timer_tick_009fa3a0` and `bsp::release_timer_arm_009d2287`
  (`src/torpedo_first_release.cpp`): the two rules.
- `GameUnitsHost::Impl::release_ordnance_007bbba0` (`src/game_hosts_units.cpp`): the single place
  a host release request raises `unit+C20h`, now shared by the task's manual passthrough and the
  aim timer.
- `AimTickBinding::accumulate_timer_009fa3a0` **had the sign backwards**. It did
  `torpedo_aim_timer += dt` and dropped the release entirely. It now subtracts and fires.
- `AimTickBinding::arm_release_timer_009d2287` now performs the arm rather than only counting it.

Contracts logged: `Unit::can_release_007bb110` (`007BB110`), `ReleaseTimer::reseed_bounds`
(`009FA3EA`), `Unit::slot_byte_9c0` (`009FA3FB`).

## Corrections

### To `docs/TORPEDO_AIM_TICK.md`

Its table row for `009D2027` reads `009FA3A0(&state+18h, dt)` and its host-method row calls
`arm_release_timer_009d2287` "the `state+18h` store". Both are right as far as they go. What was
missing, and what made this host unable to drop, is that `009FA3A0` is not an accumulator: it
counts the step **down** and calls `007BBBA0` when the countdown goes negative. The five-flag
chain the doc already documents is therefore the AI's whole release authority.

### To `docs/TORPEDO_RELEASE_ORDERS.md` and `docs/TORPEDO_ATTACK_MODE.md`

Both read `unit+C58h` as the gate between an ordered aircraft and its drop. It is the gate on the
`prepare` state's drop only. The aim state drops without it, so an aircraft that never reaches
`prepare` is not thereby unable to release. Neither doc's text is changed; this is the appended
correction.

### To the ledger

`009D2570`, `009D25A0` and `009D2720` were named `BSP_BotStateTorpedoDone_*`. They are the
`torpedo/prepare` state's routines, per the registry call in `009D2DA0`.

## no_ghidra_function

| routine | range (inclusive) | role |
| --- | --- | --- |
| `009B9420` | `009B9420`-`009B947B` | the stand-down at vtable `00D206AC`: clears `task+414h`, calls `007ED3C0` and `007BCBE0(unit, 0)` |
| `009B9480` | `009B9480`-`009B9498` | the free-fire at vtable `00D206B0`: clears `task+414h`, tail-jumps `007EFB30` |

Both are bounded by `INT3` padding: `009B9411`-`009B941F` before the first, `009B947C`-`009B947F`
between them, `009B9499` after the second.

## Validation

This tree's own before-run is `local/usn01_after.log` at commit `0fc6c89c9`, the previous packet's
after-run, because no other commit landed in this tree between the two. `main` was not merged.

| measure | before | after |
| --- | --- | --- |
| `009D2287` arms | 0 | 0 |
| release timer enabled | not modelled | 0 |
| timer fires | not modelled | 0 |
| `007BB110` refusals | not modelled | 0 |
| `timer_009FA3A0` at end | 37.20 (accumulated) | 0.00 (never started) |
| releases | 0 | 0 |
| queued release orders `unit+C58h` | 0 | 0 |
| gunnery damage | hull 23, total 220.0, deaths 1 | unchanged |

The timer field going from 37.20 to 0.00 is the sign fix: it was accumulating 3000 steps of a
disabled timer, and now a disabled timer does not advance at all, which is what `009FA3A3` does.

### The next gate

**The cone flag of the five-flag release chain**, `009D2209`. The arm at `009D2287` needs all
five flags and the run reports `release_arm_009D2287=0` on every aircraft. The aim census gives
the reason: `F18`, the turn magnitude, is `2.28` rad (131 degrees) on all five aircraft at the
end of the run, while the cone the chain interpolates is at most a few tens of degrees, so
`cone_rad > F18` is never true. The aircraft are never pointed at the target.

That is a steering result, not an authority result: `heading_error_last_mean` is `0.918` rad and
`range_last_mean` `920.9` m, so the flight closes to under a kilometre while still turning. The
release authority is now wired and will fire the moment the steering puts the nose on the target.

### USN02

USN02 has no ordered aircraft carrying torpedo ordnance, so the aim tick never runs there. The
mission was re-run against this tree's own before-run and is **identical** on the gunnery damage, the
ordnance census and the host-method counts:

```
summary mission gunnery ordnance units_with torpedo=29 general_bomb=0 drop_kamikaze=0 paratrooper=0 (of 32 units with guns)
summary mission gunnery damage queued_hits=126 dispatched=126 hit_records=126 hull=125 part=0 fires=0 floods=0 attributions=126 deaths=3 kill_credits=3 total_damage=13673.7 first_hit=41.85 s
host methods 707 concrete, 489 unimplemented
```

Nothing to attribute. The first attempt at this run was cut off at frame 1679 of 3200 and never reached the
mission summary, so it was discarded and the mission re-run to completion rather than compared
from a partial log.

## Follow-up packets

1. **The cone flag `009D2209` and `F18`.** Why the commanded heading never converges is the whole
   remaining distance to a drop.
2. **The depth-charge trio** `009A4620`, `009A48EA`, `009A67AB`: the same question for the other
   ordnance task, and a check that the aim-timer pattern is shared.
3. **`FUN_009C8200`'s callers**, to say which input path drives the manual passthrough.
4. **The message binding table** that produces kind `0C4h`, and `0BCh` with it.
5. **The twenty-one unread `007BBBA0` callers.**
