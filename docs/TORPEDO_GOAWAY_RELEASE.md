# From `goaway` to the drop: the break-off distance and the commanded heading

Addresses: `009D0D90` (`009D0D90`-`009D0F04`, the goaway enter), `009D3150`
(`009D3150`-`009D31A5`, the goaway-done predicate), `009D0C00` (a one-byte `RET`), `009D0C10`
(`009D0C10`-`009D0D87`), `009D0F10` (`009D0F10`-`009D1153`, the goaway tick), the registrar
`009D2DA0` (`009D2ECC`-`009D2F09` constructs the goaway state), the goaway vtable `00D212E0`,
`0042E740` (the tuning singleton), `00BD2F10` (`BSP_Random_UniformFloatRange`, `RET 8`),
`007B5BE0`, the transition rule `009D4030` (step `009D4132`), the arm `009D49A0`, the done/prepare
tick `009D2720`, and the heading arm's plan write `009AC40B`/`009AC41B` with the planner's read at
`0099DEB8`.

`coverage: reconstructed` for `009D0D90` and `009D3150`. `coverage: partial` for the goaway tick
`009D0F10`, which is read for its use of `+24h` and `+28h` but not transcribed. `007B5BE0` and
`[00F876B0]` are contracts.

## (1) `goaway+24h`: the field the predicate compares against

`009D3150` returns `approach+90h > goaway_state+24h`. The previous packet named this as the gate
and left `+24h` unexplained. It has exactly one producer, and finding it took ruling out the
obvious candidates:

* **The registrar does not write it.** The goaway state lives at `task+6D8h`, which is
  `approach+2E0h` (`approach` is `task+3F8h`), and `009D2DA0` constructs it inline at
  `009D2ECC`-`009D2F09`. That block writes `+4h`, `+8h`, `+Ch`, `+10h`, `+14h`, `+18h`, `+1Ch`,
  `+20h`, `+28h`, `+30h`, `+34h` and the vtable pointer `00D212E0`. It skips `+24h` and `+2Ch`.
* **The object is not zeroed.** `009D4E30` does `PUSH 7DCh` / `CALL 00BF681B` (`operator new`) and
  then `009D3050`, which calls `0099C6F0` and `009D2DA0`. None of the four contains a `memset` or
  a `REP STOS` over the block.
* **The vtable's other entry slot is empty.** `00D212E0+8h` is `009D0C00`, a single `RET` byte with
  nine `INT3` before it and fifteen after, so it is a distinct function and not the tail of
  `009D0B20`.
* **A disp32 byte scan is not evidence here.** MSVC addresses `+24h` off a pointer to the state
  with a disp8, so scanning for `MOVSS [reg+304h]` or `[reg+6FCh]` finds nothing and proves
  nothing. The three hits those scans do return are in unrelated objects.

What settles it is an exhaustive disassembly of the whole torpedo band `009D0000`-`009D5000`,
grepping for stores to any `[reg+24h]`. Thirteen matches, twelve of them `[esp+24h]`, and one:

```
009d0e37: fstp dword ptr [esi + 0x24]
```

That is inside `009D0D90`, the goaway vtable's slot `+4h`, its enter.

## (2) `009D0D90`, the goaway enter

```
009D0D90  eax = [00F876B0] & 80000001h, with the JNS/DEC/OR/INC signed-remainder fixup
009D0DA7  state->+2Ch = (eax == 0) ? +1.0f /* 00D7A24C */ : -1.0f /* 00D7A260 */
009D0DC1  tuning = 0042E740()
009D0DD1  d = tuning->+438h                      /* Pilot/Torpedo/SafeDist, default 700 */
009D0DDF  if (approach->+CCh && target->vtable[5Ch](5))
009D0DF2      r = 007B5BE0(target)                /* max(target+444h, target+448h) */
009D0E07      if (!(d > r)) d = r
009D0E2C  j = BSP_Random_UniformFloatRange(1.0f, 1.15f /* 00D20CE4 */)
009D0E37  state->+24h = j * d
009D0E3A  009D0C10(state)
```

The enter does not stop there. `009D0E3F`-`009D0F04` goes on to overwrite four of
the values the registrar had just set, each through a second and third
`BSP_Random_UniformFloatRange`:

```
009D0E42  if (approach->+132h)                       /* still carrying ordnance */
009D0E71      a = UniformFloatRange([00CEB4D4] = 50.0f, [00CE3D08] = 100.0f)
009D0E76      a += approach->+78h + approach->+74h
          else
009D0E7F      a = ctl->+394h                          /* Pilot/Torpedo/CruisingAlt */
009D0E88  state->+1Ch = a                             /* the break-off altitude */
009D0EA3  state->+28h = UniformFloatRange(v->+10h, v->+14h)   /* v = approach->+14h */
009D0EB1  state->+34h = 0;  state->+30h = 0
009D0EBB  t = approach->+78h + approach->+74h + [00CE7630]
009D0EDD  state->+20h = (t <= [00CE3938]) ? [00CEB4D4] = 50.0f : t
009D0EF4  ret                                         /* and a second RET at 009D0F04 */
```

So the constructor's `100.0f` at `+28h` and `120.0f` at `+1Ch`/`+20h` are
placeholders: every entry into `goaway` replaces them. Only `+24h` and `+2Ch`
have no constructor value at all.

So `goaway+24h` is a **break-off distance in metres**, 700 to 805 at the stock tuning, raised to
the target's own extent when the target is larger. `goaway+2Ch` is the break-off **side**, `+1` or
`-1`, alternating on the low bit of the global `[00F876B0]`.

`00D20CE4` is `1.15f`; `00CE3D08` (the `+28h` the constructor sets) is `100.0f`; `00D05804` (the
`+1Ch`/`+20h` pair) is `120.0f`.

## (3) `009D3150`, the predicate

```
009D3151  d = state->+24h
009D315C  if (ctl->+369h != 0 && [00E17BF2] != 0) {
009D3173      if (approach->+132h == 0) return false      /* 009D317C XOR AL,AL */
009D3183      d *= 0.4                                    /* 00CE65D0, the widened 0.4f */
          }
009D318F  return approach->+90h > d                       /* 009D3195, 009D3199 JBE clears */
```

The scaling arm needs both the pilot control block's `+369h` and the global `[00E17BF2]`. This
host reports `+369h` as `0` (`read_control_block`), so the test is the bare range comparison.

## (4) The commanded heading `cmd+2C0h`

`docs/TORPEDO_AIM_TICK.md` flagged that all five aircraft left `aim` through clause 2 with a
2.5 rad steering delta, because nothing consumed the heading the tick writes. The path was already
documented and simply not connected:

* `docs/PILOT_TASK_HEADING_ARM.md` establishes that `ctx->+18h` is the plan, that the per-kind arm
  writes `plan+2C0h` at `009AC40B` with `plan+2CCh = 2` at `009AC41B`, that the value is an
  **absolute** ground bearing in radians, and that a bot state's tick may overwrite the same pair.
* The planner's yaw base term is `SubtractWrappedAngle(plan+2C0h, unit+C6Ch)` at `0099DEB8`. It
  reads the plan field, never the raw target bearing.
* The torpedo aim tick writes exactly that pair: `cmd+2C0h` at `009D1D16` and `cmd+2CCh = 2` at
  `009D1D1E`, with `cmd` = `approach+18h`.

The host was computing the yaw arm's input from `plane_bearing_to_target_009ac190` every think,
which is the arm's own value and discards whatever the state wrote. It now carries `plan+2C0h` and
`plan+2CCh` on the slot, the aim tick publishes them, and `plan_yaw_0099d300` prefers them
whenever the mode is 2. That is the native's own precedence, not a new rule.

## ABI

`009D0D90`: `void __fastcall BotStateTorpedoGoAway::OnEnter(BotStateTorpedoGoAway* ECX)`, `RET`.
`009D3150`: `bool __fastcall BotStateTorpedoGoAway::IsComplete(BotStateTorpedoGoAway* ECX)`, `RET`.
`009D0C10`: `void __fastcall(BotStateTorpedoGoAway* ECX)`, `RET`.
`009D0F10`: `void __thiscall(BotStateTorpedoGoAway* ECX, float dt)`, `RET 4`.
`00BD2F10`: `float __cdecl(float lo, float hi)`, `RET 8` (two stack floats, x87 result).

State layout, goaway (`task+6D8h`): `+0` vtable `00D212E0`, `+4` approach, `+18h` a float
`009D0C10` writes, `+1Ch`/`+20h` `120.0f`, `+24h` the break-off distance, `+28h` `100.0f` counted
down by the tick, `+2Ch` the break-off side, `+30h`/`+34h` accumulators.

## Host methods

| host binding | native | site |
| --- | --- | --- |
| `safe_distance_438()` | `0042E740()+438h` | `009D0DC1`, `009D0DD1` |
| `torpedo_goaway_enter_009d0d90` on `set_state` to `kGoAway` | `009D0D90` | vtable `00D212E0+4h` |
| `torpedo_goaway_complete_009d3150` into `goaway_done_009d3150` | `009D3150` | `009D4132` |
| the slot's `plan_heading_2c0` / `plan_heading_mode_2cc` | `cmd+2C0h` / `+2CCh` | `009D1D16`, `009D1D1E` |
| `plan_yaw_0099d300` reading the plan heading | `plan+2C0h` | `0099DEB8` |

Contracts, stated rather than invented: `007B5BE0`'s target extent is not modelled, so
`has_extent_target` is false and the distance is the tuning value alone; `[00F876B0]` is a global
this host does not carry, so the break-off side alternates on the slot's own entry count, which
changes the side but not the gate, since `009D3150` never reads `+2Ch`; and
`BSP_Random_UniformFloatRange(1.0, 1.15)` takes the low end, matching the binding's existing
`random_between`, so the distance is the unjittered `SafeDist`.

## Corrections

### Correction to `docs/TORPEDO_AIM_TICK.md` (packet `cc8_torpedo_aim_tick`)

Appended, not rewritten. That document's "Follow-up packets" item 1 asks for `goaway_state+24h`'s
producer and item 2 for the `cmd+2C0h` consumer; both are answered here. Its "Uncertainty" note
that the host exercises clause 2 while the native would likely exercise clause 1 is now testable,
because the steering delta is fed by the tick's own heading. The Validation section below records
which clause fires once it is.

### Correction to `docs/TORPEDO_TASK_ARM.md`

Appended, not rewritten. Its ledger note for `009D3150` says the predicate returns
`d < approach+90h` without saying what `d` is. `d` is `goaway+24h`, written once per entry by
`009D0D90` as `UniformFloatRange(1.0, 1.15) * max(Pilot/Torpedo/SafeDist, 007B5BE0(target))`.

## no_ghidra_function

| address | inclusive end | what it is |
| --- | --- | --- |
| `009D0C00` | `009D0C00` | goaway vtable `00D212E0` slot `+8h`, a bare `RET` between `INT3` runs |
| `009D0C10` | `009D0D87` | the goaway geometry helper, final `RET` at `009D0D87` |
| `009D0D90` | `009D0F04` | the goaway enter; two exits, `009D0EF4` and `009D0F04` |
| `009D0BF0` | `009D0BF6` | an adjacent one-line getter returning `[00CE4BC4] = 600.0f`, noted because it shares the padding run |

## Validation

* `build-tested`: `./scripts/build.ps1`, Win32 `/W4 /WX`, clean; `ctest` 2/2.
* Tree base: this worktree merged `main` at `eed7e744b` before the packet, so both
  before-runs were measured on the merged tree and carry the ship AI ring-scan change.

**USN01 before**, 3200 frames, 3000 mission frames at 0.05 s:

| aircraft | aim ticks | aim-complete at | clause | goaway enters | break-off 24h | peak range in goaway | goaway done | releases |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Mav1 | 181 | 181 | turn | - | 0.0 | -1.0 | no | 0 |
| Mav2 | 167 | 167 | turn | - | 0.0 | -1.0 | no | 0 |
| Mav3 | 171 | 171 | turn | - | 0.0 | -1.0 | no | 0 |
| Mav4 | 173 | 173 | turn | - | 0.0 | -1.0 | no | 0 |
| Mav5 | 178 | 178 | turn | - | 0.0 | -1.0 | no | 0 |

**USN01 after**:

| aircraft | aim ticks | aim-complete at | clause | goaway enters | break-off 24h | peak range in goaway | goaway done | releases |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Mav1 | 371 | 276 | turn | 96 | 700.0 | 697.4 | no | 0 |
| Mav2 | 357 | 267 | turn | 91 | 700.0 | 702.3 | no | 0 |
| Mav3 | 365 | 276 | turn | 90 | 700.0 | 697.0 | no | 0 |
| Mav4 | 358 | 271 | turn | 87 | 700.0 | 766.5 | yes | 0 |
| Mav5 | 372 | 275 | turn | 98 | 700.0 | 700.4 | no | 0 |

State sequences after:

* `Mav1`: moveto=188 attackrun=614 goaway=126 aim=371, transitions 195
* `Mav2`: moveto=191 attackrun=599 goaway=151 aim=357 prepare=1, transitions 186
* `Mav3`: moveto=139 attackrun=624 goaway=170 aim=365 prepare=1, transitions 184
* `Mav4`: moveto=268 attackrun=586 goaway=87 aim=358, transitions 176
* `Mav5`: moveto=234 attackrun=594 goaway=99 aim=372, transitions 197

Torpedo task summary after:

* summary mission torpedo task: aircraft=5 releases=0 blocked_engaged_009d3210=1020 blocked_arm_009d49a0=0
* summary mission torpedo task: no release. 009D15F0 writes the aim-complete byte state+2Ch at 009D236E (5 of 5 aircraft) and 009D4030 step 11 leaves aim for goaway. 009D3150 is now computed from the 009D0D90 break-off distance goaway+24h=700.0; it went true for 1 aircraft, and the furthest any got inside goaway was 766.5 m. closest approach 285.2 against 8Ch=2200.0, 6490 offers, aim ran 1823 ticks

Ordnance: summary mission gunnery ordnance units_with torpedo=14 general_bomb=11 drop_kamikaze=0 paratrooper=0 (of 41 units with guns); summary mission gunnery torpedo_ranges_derived=39 swims_started=0 snaps=0 bullet_ranges_derived=336 base_tick_timers_live=0 expired=0

**USN02**, same invocation, against this tree's own merged-tree before-run:

```
before: summary mission gunnery damage queued_hits=120 dispatched=120 hit_records=120 hull=119 part=0 fires=0 floods=0 attributions=120 deaths=3 kill_credits=3 total_damage=12463.2 first_hit=41.90 s
after:  summary mission gunnery damage queued_hits=120 dispatched=120 hit_records=120 hull=119 part=0 fires=0 floods=0 attributions=120 deaths=3 kill_credits=3 total_damage=12463.2 first_hit=41.90 s
```


### What the two changes moved, and the gate that is left

The goaway binding is live: each aircraft now enters `goaway` between 87 and 98 times against a
break-off distance of 700 m (`Pilot/Torpedo/SafeDist` unjittered), the state machine cycles
`aim` to `goaway` and back, and transitions per aircraft rose from 3-6 to 176-197. `009D3150` went
true for one of the five, Mav4, which reached 766.5 m.

The heading wiring moved the steering delta but did not change which clause fires. At the tick the
aim-complete byte first went true, `F18` fell from 2.53-2.58 rad to 2.28-2.34 rad, and the mission's
mean final heading error fell from 1.191 rad to 0.918 rad. Clause 2 (`F18 > F0C`) still fires first
for all five, because `F0C` fell with it: both are near 2.28 at the crossing. Clause 1 would need
the range inside about 330 m, and the closest approach rose from 5.8 m to 285.2 m precisely because
the aircraft now fly the run-in heading the tick plans instead of pointing at the target.

**The next gate, by address and value: `ctl+370h`, the pilot control mode.** `009D3F60` step 1
(`009D3F6E`) and `009D4030` step `009D4084` both send the task to `prepare` only while that mode is
`0`, and `009D49A0` arms `prepare+98h` only in `prepare` (`009D49C2`). `009D4132` sends a completed
`goaway` to `aim` while `task+52Ah` holds, never to `prepare`, so the cycle cannot reach the
countdown on its own. This host raises `ctl+370h` to 1 on the flight leader's cruise tick
(`0099B740`) and nothing lowers it again. What lowers it is the remaining unknown on the release
path.

`summary mission gunnery ordnance` reports `torpedo=14` units carrying torpedoes and
`swims_started=0`: no release was requested through `007BBBA0`, so there is no ordnance line to
attribute and no damage from a torpedo.

## Follow-up packets

1. `0099B740` and `ctl+370h`: `009D3F60` step 1 and `009D4030` step `009D4084` both send the task
   to `prepare` only while the pilot control mode is `0`, and `009D49A0` arms `prepare+98h` only in
   `prepare`. What lowers the mode again is the remaining unknown on the release path.
2. `007B5BE0` and the entity extents `+444h`/`+448h`, which raise the break-off distance against a
   large target.
3. The goaway tick `009D0F10` proper, including `+28h`'s countdown and `+30h`/`+34h`.

## Correction from `docs/TORPEDO_ATTACK_MODE.md` (packet `cc8_torpedo_attack_mode_lowering`)

Appended, not rewritten. This document's follow-up 1 asks what lowers `ctl+370h`. The answer is
that **nothing a torpedo task runs does**. Both routes to `0` belong to other objects: the
`closetoship` task's countdown `009A2810`, which arms only after Lua's `PilotStopCloseToShip` sets
mode `2`, and the `BCh` message arm at `007F0068`, whose producer is unread. On USN01 neither
fires, in the native as much as in the host.

Measured consequence: the mode is `0` for exactly one arm tick, before the flight leader's raise
propagates, and two of the five aircraft enter `prepare` on that tick. It is also the one tick the
release-order queue `unit+C58h` is empty, so `009D49A0` takes its no-order arm and `prepare+98h` is
never set.
