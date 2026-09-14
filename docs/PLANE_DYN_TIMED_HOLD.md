# `dyn+C0h` is a countdown in seconds: the timed direction hold

Addresses: 007D83D0 007D83F7 007D81B0 007D81C7 007D81E3 007D8216 007D8470 007D902F 007DB680
007DC6C5 007DC6E6 007DB1F0 007DB2A4 007D9A70 007D9AFC 007D7EA0 007D7EE5 007C6F50 007C705C
007C07A0 007C08F7 007D1360 00CE74F8 00D7A260 00CEFF98 00E188A8

Packet `cc7_dyn_c0_writers`. **Ghidra was read-only: no annotation, no tagging, no write lock, no
ledger record.** Descriptive names below are hypotheses, not recovered symbols. The C++ added by
this packet is **reconstructed and build-tested** only: `./scripts/build.ps1` compiles it into
`bsp_game.exe` and the existing `reconstructed_math` check passes. Nothing here is fixture-tested,
ABI-compatible or game-validated, and no new test was added.

`docs/PILOT_BOT_PLAN_CONTROLS.md` §`dyn+C0h is runtime-only` located three writers and read none of
their arithmetic. This document reads the arithmetic, and **corrects and extends that record** —
see §5.

## 0. The answer, in one paragraph

`dyn+C0h` is **a timer in seconds**, and the arithmetic says so rather than leaving it a guess:
`007D902F` decrements it by the integrator's own `step` argument — the same scalar `007D8F39`
divides a position delta by to produce a velocity — and clamps the result at zero. It is one half
of a two-field group: `007D83D0` writes a 3-float direction at `dyn+B4h` and the duration at
`dyn+C0h` in one call, and `007DB2A4` restores exactly that pair out of a control-state message.
The plane launch path arms it with a normalised direction and the literal **0.8 s** (`00CE74F8`);
two AI flight-task routines arm it with a computed duration. Each core-law tick then clears it
unless a predicate on the plane's `unit+72Ch` sub-object holds, and decays it by one step. The yaw
rate law at `007D9AFC` consumes it as `0.6f * seconds`, so the remaining time *is* the effect's
strength envelope — a linear ramp-down. There are **six** writers, not three, and the two the prior
packet missed are the ones that matter most: the setter and the game-state gate.

## 1. The writer census

Complete for `battlestationspacific.exe`. Every store encoding that can reach displacement `0C0h`
was scanned, not sampled:

| pattern | meaning | hits in the plane code |
| --- | --- | --- |
| `f3 0f 11 ?? c0 00 00 00` | `MOVSS [reg+0C0h], xmm` | `007D7B9B`, `007D7EE5`, `007D81C7`, `007D83F7`, `007D902F`, `007DC6C5` |
| `d9 9? c0 00 00 00` | `FSTP [reg+0C0h]` | `007DB2A4` |
| `c7 ?? c0 00 00 00` | `MOV [reg+0C0h], imm32` | none |
| `89 ?? c0 00 00 00` | `MOV [reg+0C0h], reg32` | none |

`007D7B9B` and `007D7EE5` are in `TRIV_body_007D7A80`'s tail and in
`BSP_PlaneFlightController_Construct`; both were **not read** and are listed for completeness, not
claimed (the construct site is presumably the zero-init). The `d9 8?` forms at `007D7A93`,
`007D83A2`, `007D9AFC` and `007DEF9F` are `FLD`, i.e. the reads the prior packet listed.

**Caveat carried forward, unchanged:** the scan covers this executable only. This installation
carries BSPRM/AlterBSP artefacts, so a loaded module writing the field is not excluded.

| # | site | function | what it does |
| --- | --- | --- | --- |
| 1 | `007D83F7` | `FUN_007D83D0` | **the setter** — writes the direction and the duration together |
| 2 | `007D81C7` | `BSP_PlaneFlight_TickContactTimers` `007D81B0` | forces `0` while `GGame+1FE4h == 2` |
| 3 | `007DC6C5` | `BSP_PlaneFlight_CoreLaw` `007DB680`, commit tail | forces `0` unless the `+72Ch` predicate holds |
| 4 | `007D902F` | `BSP_PlaneDynamics_IntegrateStep` `007D8470`, tail | `max(0, v - step)` while `v > 0` |
| 5 | `007DB2A4` | `FUN_007DB1F0` | restores it from a control-state message |
| 6 | `007D7EE5` | `BSP_PlaneFlightController_Construct` | not read |

## 2. The arithmetic

### 2.1 `007D83D0` — the setter, whole body read

```
007D83D0  void __thiscall FUN_007D83D0(controller /*ECX*/, const float dir[3], float seconds)
007D83D0     dyn = ctl->[10h]
007D83DF     dyn->B4h = dir[0]        007D83E5  dyn->B8h = dir[1]
007D83EE     dyn->BCh = dir[2]        007D83F7  dyn->C0h = seconds
007D83FF     RET 8
```

Eleven instructions, no clamp, no null test. Two call sites, both enumerated through
`ghidra callers` **and** `ghidra xrefs`, which agree here:

* `007C705C`, inside `FUN_007C6F50` — reached from `BSP_Plane_BeginFlying` `007C7110` and
  `BSP_Plane_HandleMessage` `007CCFA0`. The direction is `BSP_Vector3f_Normalize`'s output
  (`007C7026`) and the duration is the constant at `00CE74F8`, `3F4CCCCD` = **0.8f**.
* `007C08F7`, inside `FUN_007C07A0` — reached from `FUN_009B09C0` and `FUN_009B1420`, both in the
  pilot-bot task segment (`follow`, `moveto`, `attackrun`, `circle`) and both users of
  `BSP_Math_SubtractWrappedAngle` and `BSP_Math_HeadingToDirectionXZ`. The controller is
  `EDI+0AB0h` and the duration is `BSP_Math_InterpolateClamped`'s result scaled by `[ESP+20h]`
  (`007C08D1`, `007C08D6`). That interpolation's own inputs (`00D05AA8`, `00D05AAC` = DEG(60),
  `00CE7804`) were **not traced to their meaning**.

So one arming site is a fixed launch duration and the other is an AI-computed one. What the hold is
*for* follows from those two call sites and no further; the name "direction hold" is a hypothesis.

### 2.2 `007D902F` — the decay, read with the x87 stack tracked

`BSP_PlaneDynamics_IntegrateStep` is `__thiscall(dyn /*ECX*/, float step, const Matrix* unit+74h,
const Matrix* ctl+0B0h)`. **Correction to the ledger record:** it says `RET 0x10`; both exits
(`007D903E`, `007D904C`) are `RET 0Ch`, and the call site at `007DC6DA`-`007DC6E6` pushes exactly
three dwords. Everything else in that record holds.

The frame is `SUB ESP,3Ch` then four pushes, so steady `ESP = entry - 4Ch` and `[ESP+50h]` is
**arg 1, `step`**. This matters: the literal `[ESP+54h]` appears twice and means different
arguments each time — `007D8476` reads it at three pushes (`= arg 3`, the `ctl+0B0h` matrix) and
`007D883D` reads it in steady state (`= arg 2`, the `unit+74h` matrix). A grep on the displacement
would conflate them.

Three paths reach the common point `007D8F18`, and all three leave the x87 stack as
`[0.0, step]` — the fall-through pushes them at `007D8F0E`/`007D8F12`, `007D8E3C JBE` arrives with
the pair pushed at `007D8E1B`/`007D8E22`, and `007D8E67 JBE` arrives one deep and pops at
`007D8F16`. Tracking every `FLD`/`FXCH`/`FMULP`/`FADDP` from there through the 3x3 fold at
`007D8F89`-`007D8FEC` leaves the same `[0.0, step]` at `007D8FFE`, and `XMM0` is the zero from
`007D8EF6`. Hence:

```
007D8FFE  prev = dyn->C0h
007D9006  COMISS prev, 0.0
007D900F  JBE  -> 007D9041: pop both x87 values and RETURN WITHOUT STORING
007D9015  d = prev - step                       ; FSUBRP against the retained step
007D9023  FCOMIP 0.0, d
007D9027  JA   -> store 0.0                     ; d went negative
007D9029  else store d
007D902F  dyn->C0h = <that>
```

`dyn->C0h = (prev > 0) ? max(0, prev - step) : prev`. The guard is strict, so the field is never
written once it reaches zero, and never goes negative. Because `COMISS` is unordered-false, a NaN
also takes the no-store exit.

The same block proves the units: `007D8F35`-`007D8F4F` computes
`(dyn[64h..6Ch] - dyn[70h..78h]) / step` and stores it as the velocity at `dyn[7Ch..84h]`. The
divisor of a position delta that yields a velocity is a time, so `step` is seconds and so is
`dyn+C0h`.

### 2.3 `007DC6C5` — the core law's commit, and a correction

```
007DC692  XORPS XMM0,XMM0
007DC696  dyn = ctl->[10h];  007DC69D  dyn->C4h = [ESP+0Fh]      ; a byte, unrelated
007DC6A3  owner = ctl->[8]
007DC6A6  TEST owner,owner
007DC6A8  JZ  -> 007DC6C2                                        ; store, XMM0 still 0.0
007DC6AA  p = (*(void**)(owner+72Ch))->[38h](owner+72Ch)         ; bool
007DC6BD  JNZ -> 007DC6CD                                        ; p true: SKIP the store
007DC6BF  XORPS XMM0,XMM0                                        ; p false: re-zero
007DC6C2  ECX = ctl->[10h]
007DC6C5  dyn->C0h = XMM0
```

**Every path that reaches `007DC6C5` stores zero.** `XMM0` is cleared at `007DC692`, and the
re-clear at `007DC6BF` exists precisely because the virtual call at `007DC6B9` clobbers the volatile
`XMM0`. The three entries into `007DC692` (`007DBE58 JBE`, `007DBEA5 JMP`, `007DC577 JBE`,
`007DC68C JMP`) all land on the `XORPS`, and nothing else jumps into `007DC6C2` or `007DC6C5`.

`docs/PILOT_BOT_PLAN_CONTROLS.md` says "an earlier `JE` path stores a computed `XMM0`". That is
**wrong**: this site is a conditional clear, never a set. The only site that sets a non-zero value
is `007D83D0` (and `007DB2A4` on the replication path).

### 2.4 `007D81B0` `BSP_PlaneFlight_TickContactTimers` — the missed gate, whole body read

Twenty-nine instructions, `__thiscall(controller, float step)`, `RET 4`. Called by the core law as
its **first** act, at `007DB690`.

```
007D81B5  if (GGame->[1FE4h] == 2)       ; GGame = *00E188A8, docs/APP_FRAME_GAME_STATE.md
007D81C7      dyn->C0h = 0.0f
007D81D2  if (dyn && dyn->C8h > 0) {
007D81E3      dyn->C8h -= step
007D81F6      p = (*(void**)(owner+72Ch))->[38h](owner+72Ch)
007D8209      if (!p) dyn->C8h = -1.0f   ; 00D7A260 is BF800000
          }
```

Two things fall out. First, `dyn+C8h` is a **sibling timer of identical shape** under the **same**
`+72Ch` predicate, and its `-1.0f` sentinel is what tells you these are timers rather than
magnitudes. Second, the predicate's polarity is consistent across both fields: *true* preserves the
timer, *false* ends it — as `-1` for `+C8h`, as `0` for `+C0h` in the core law.

Note the asymmetry in defensive coding: `007D81D2` null-tests `ctl->[10h]` but `007D81F3` does
**not** null-test `ctl->[8]` before dereferencing `owner+72Ch`, where the core law at `007DC6A6`
does. Observed, not explained.

The same `*(00E188A8 + 1FE4h) != 2` comparison guards the air-operations update per
`docs/AIR_OPERATIONS.md`; what state `2` is remains unread there and here.

### 2.5 `007DB2A4` `FUN_007DB1F0` — the source object, identified

The prior packet expected this to be the hard one. It is the easy one: sixty-six instructions,
`__thiscall(controller, const Msg* /*one stack arg, RET 4*/)`, sole caller
`007D1360 BSP_Plane_ApplyControlStateMessage`. So the "source object" is the **replicated
control-state message**, and the routine is a state *restore*, not a physics producer:

| message | controller / dyn | site |
| --- | --- | --- |
| `+00h`/`+04h`/`+08h`, or `0042D0D0(msg+0Ch, ctl->[8]+74h)` when `msg+26h != 0` | `ctl+18h..20h`, copied on to `ctl+30h..38h` | `007DB216`-`007DB248` |
| `+18h`/`+1Ch`/`+20h` | `ctl+24h..2Ch` | `007DB24B`-`007DB25A` |
| `+25h`, `+24h` (bytes) | `ctl+4`, `ctl+5` | `007DB26F`-`007DB27B` |
| `+28h` | `ctl+ACh` | `007DB27E` |
| `+30h`/`+34h`/`+38h` | **`dyn+B4h..BCh`** | `007DB28C`-`007DB29A` |
| `+2Ch` | **`dyn+C0h`** | `007DB2A0`-`007DB2A4` |

The last two rows are the same `{vec3, scalar}` group `007D83D0` writes, which is the second
independent piece of evidence that `dyn+B4h` and `dyn+C0h` are one object. It also means the field
is **replicated**, so a networked host must carry it across the wire, not merely recompute it.

## 3. The per-tick rule

`BSP_PlaneFlight_CoreLaw` has a single exit (`007DC828`) and touches the field three times, in this
order:

```
007DB690   TickContactTimers      : if (GGame->[1FE4h] == 2) seconds = 0
   ...     the flight law
007DC6C5   commit                 : seconds = (owner && hold) ? seconds : 0
007DC6E6   IntegrateStep -> 007D902F : if (seconds > 0) seconds = max(0, seconds - step)
```

Because the commit runs before the decay, a false predicate zeroes the field and the decay then
finds it non-positive and leaves it alone — the two never fight. The core law is reached from
`BSP_PlaneFlight_FreeFlightStep` `007DC830`, `BSP_PlaneFlight_GroundRollLaw` `007DCCF0` and
`BSP_PlaneFlight_WaterSurfaceLaw` `007DCDD0`, so the sequence is the same in all three regimes.

**Bounded:** the single `RET` and the unconditional reach from `007DC6CD` to `007DC6E6` make it
very likely that both the commit and the integrate run on every invocation, but the 1214-instruction
body's branches were **not** fully enumerated, so "on every invocation" is not proved here.

## 4. What the scalar means, and what is still unnamed

**Named, with evidence.** It is a duration in seconds: decremented by the timestep, clamped at zero,
armed with a literal `0.8` on the launch path, paired with a `-1.0f`-sentinel sibling. The yaw law's
`v = c + seconds * 0.6` (`007D9AFC`, `00CEFF98`) therefore reads a linearly decaying envelope, and
the prior packet's saturation note still holds arithmetically: above `1.667 s` the `* 0.6` term
alone reaches the law's cap of 1.

**Not named.** What the hold is *for*. The two arming call sites say "just after BeginFlying, for
0.8 s" and "from an AI flight task, for a computed time", which is consistent with a launch
direction hold, a post-spawn settling window, or a commanded-turn authority boost. The read at
`007D9A70` uses the scalar **alone** — it does not read `dyn+B4h` — so the direction half is
consumed somewhere this packet did not find, and without that consumer the group's purpose stays
open. I am not picking a reading.

**Unresolved inputs**, carried into the C++ as named parameters rather than guesses:

* `hold` — `(*(void**)(owner+72Ch))->[38h](owner+72Ch)`. `docs/AIR_OPERATIONS.md` and
  `docs/AIRFIELD_TAXI.md` identify `unit+72Ch` in plane code as the air-operations block, and warn
  that `+72Ch` means different things for ships, forts and airfields. The slot `+38h` (index 14)
  was not resolved to a body.
* `game_state_is_two` — `GGame+1FE4h == 2`. `00E188A8` is the GGame singleton pointer per
  `docs/APP_FRAME_GAME_STATE.md`; the state enum is unread.
* the `007C08D1` interpolation that produces the AI path's duration.

## 5. Corrections to `docs/PILOT_BOT_PLAN_CONTROLS.md`

| was | is | evidence |
| --- | --- | --- |
| three writers | **six** | the four-pattern scan in §1; `007D83F7`, `007D81C7` and `007D7EE5` were missed |
| `007DC6C5`: "an earlier `JE` path stores a computed `XMM0`" | it stores **only zero**, on both reaching paths | `007DC692` and `007DC6BF` both `XORPS`; no other entry into `007DC6C2`/`007DC6C5` |
| `007DB2A4`'s source object unidentified | the **replicated control-state message** from `BSP_Plane_ApplyControlStateMessage` | sole caller `007D1360`; the field map in §2.5 |
| "the comparand at `007D9006`" unknown | `0.0f`, the `XORPS` at `007D8EF6` | §2.2 |
| the meaning "not picked" | it is a **duration in seconds**; the *purpose* is still not picked | §2.2, §2.4, §4 |

The ledger record for `007D8470` gives `RET 0x10`; the listing gives `RET 0Ch` at both exits.
Ghidra is read-only for this packet, so the ledger was not edited.

## 6. Wiring contract

What a host already running the flight law must do to maintain the field. Everything below is
**reconstructed and build-tested**, not fixture-tested and not game-validated.

1. Hold `{float direction[3]; float seconds;}` per plane. `include/bsp/plane_flight.hpp`'s
   `PlaneTimedDirectionHold` is that pair.
2. On the launch path, call `arm_timed_direction_hold_007d83d0(normalised_dir,
   kPlaneLaunchHoldSeconds)`. On an AI-commanded turn, pass the computed duration instead — this
   packet did not recover that computation.
3. Each core-law tick, in order: `gate_direction_hold_007d81c7`, then
   `commit_direction_hold_007dc6c5`, then `decay_direction_hold_007d902f`.
4. Supply `hold` and `game_state_is_two` from the host. Defaulting `hold` to `true` and
   `game_state_is_two` to `false` reproduces "the hold is never cut short"; defaulting `hold` to
   `false` pins the field at zero, which is what the build does today and is why the yaw law's
   `v` term has been dead.
5. On a networked host, carry `seconds` and `direction` in the control-state message: `007DB2A4`
   proves they are replicated, so recomputing them locally would desynchronise.

## 7. Coverage

Read instruction by instruction: `FUN_007D83D0` whole; `BSP_PlaneFlight_TickContactTimers` whole;
`FUN_007DB1F0` whole; `BSP_PlaneDynamics_IntegrateStep` `007D8470`-`007D84A8`,
`007D8E0C`-`007D904E` with the x87 stack tracked across all three entries to `007D8F18`;
`BSP_PlaneFlight_CoreLaw` `007DC645`-`007DC6F4`; the two `007D83D0` call sites
(`007C7010`-`007C7062`, `007C08A2`-`007C08FC`).

Read in outline: `FUN_007D9A70` (the consumer) — only the `+C0h` term and the calls, enough to show
`dyn+B4h` is **not** read there.

Not read: `BSP_PlaneFlight_CoreLaw` outside its commit tail (1214 instructions); the middle of
`BSP_PlaneDynamics_IntegrateStep` (`007D84A8`-`007D8E0C`); `TRIV_body_007D7A80`'s `007D7B9B` and
`BSP_PlaneFlightController_Construct`'s `007D7EE5`; the `+72Ch` vtable slot `+38h`; `FUN_007C07A0`
and `FUN_007C6F50` outside their call sites; `FUN_009B09C0` and `FUN_009B1420` entirely.
