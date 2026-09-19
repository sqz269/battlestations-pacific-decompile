# Handoff: the dive-bomb probe `007F0280`, and what is around it

Addresses: `007F0280` (body unread), `009C42B8` (the hole), `009C4220`, `task+41Ch`.

Written for a cold reader. Nothing here needs the session that produced it.

---

## 1. Where the bomber ditches today

Read this first: the successor needs the failure, not the function.

From `local\usn04_terms.log` (4800 mission frames, three aircraft `movieval`, `movieval|.-2`,
`movieval|.-3`, all identical):

```
divebomb movieval  arm_ticks=2117 transitions=5
  states[goaway=43 aimdive=318 flyabove=158 turndown=71 attackrun=1527]
  releases=0 bombs_spawned=0 rounds_left=2
divebomb movieval  aim trace: 009C62B0 ticks=158 heading writes=158
  turndown entry range=3.8 m bearing=2.9100 rad
  aimdive  entry range=472.6 m bearing=3.1219 rad | closest range=472.6 m
plane water contact: unit=movieval alt=-0.36 water=0.00 |v|=44.92 state 7 -> 6
```

The walk is complete and the aircraft still flies into the sea at **-0.36 m and 44.92 m/s**, about
190 s of task in, and the arm stops because the flight state leaves 7. A longer mission changes
nothing: an 8800-frame run stopped at the same tick count. **The chain does not run out of window,
it runs out of aircraft.**

The shape of the failure, in order:

* `attackrun` 1527 ticks closes 11 km to under `approach+B8h` = 1100 m. Fine.
* `flyabove` 158 ticks, every one of them issuing a heading (`009C62B0` bound), brings the aircraft
  to **3.8 m** of its aim point. This works.
* `turndown` 71 ticks is a split-S. It exits on attitude alone - `009C7EA0` wants `pose+C64h` past
  -1.0 rad - with no heading or range term, and 71 ticks at that speed is ~470 m of travel.
* `aimdive` therefore **begins 472.6 m away with the target 178.9 degrees behind**, and 472.6 m is
  the closest the aircraft ever gets: the trace rises monotonically to 1842 m.
* `goaway` 43 ticks of climb-out is too late to recover a dive entered from that geometry.

So the release never fires because the aim error never approaches the 25 m window at `00CE3880` -
closest 374.99 m - and the aim error is a symptom, not the fault.

---

## 2. `007F0280`: the ABI from both ends, and the frame

**Body `007F0280`-`007F0B21`, 2209 bytes, SEH-registered, `0x110` of frame. The body is UNREAD.**
`docs/BOT_PROBE_007F0280.md` carries the caller survey; this section carries the frame.

### Six stack arguments, confirmed two ways

```
007f0280  PUSH -1 / PUSH 0xc8f35b / MOV EAX,FS:[0] / PUSH EAX   ; SEH triple, 12
007f0295  SUB  ESP,0x104                                        ; 260
007f029b  PUSH EBP / PUSH ESI                                   ; 8   -> 280 pushed
007f029d  MOV  ESI,dword ptr [ESP + 0x11c]                      ; [ESP+284] = arg0
007f0b19  ADD  ESP,0x110
007f0b1f  RET  0x18                                             ; 24 = six arguments
```

`RET 18h` says six. The prologue agrees independently: 280 bytes pushed puts the return address at
`[ESP+280]`, so arg0 is `[ESP+284]` = the `[ESP+11Ch]` that `007F029D` reads. `docs/BOT_TASK_STATES.md`
row 356 said five; it is corrected there with this evidence (commit `d067466b6`).

### The six slots, walked with ESP anchored

| arg | slot | first read | width |
| --- | --- | --- | --- |
| 0 | -4 | `007F029D` `[ESP+11Ch]` | m32 |
| 1 | -8 | `007F02E9` `[ESP+124h]` | m32 |
| 2 | -12 | `007F02B8` `[ESP+128h]` | m32 |
| 3 | -16 | `007F02CD` `[ESP+12Ch]` | m32 |
| 4 | -20 | `007F038D` `[ESP+138h]` | **m8, a `CMP`** |
| 5 | -24 | - | **never read through `[ESP+n]`** |

The displacements are not a uniform 4-byte march because the depth differs between the reads. That
is the reason to anchor rather than grep literals.

Two facts worth having before the body: **arg4 is read as a byte by a `CMP`**, the shape of a mode
rather than a float; and **arg5 is never read through `[ESP+n]` in the whole 2209 bytes**, so it is
taken through a pointer or unused.

### How to read the body

Script the listing, anchor ESP on the SEH state stores, propagate **backward**, never resync
forward. A working frame walk is in this session's scratchpad pattern: disassemble from
`007F0280`, track `push`/`pop`/`sub esp`/`add esp`, and report `slot = depth - disp` for every
`[ESP+n]`. Do **not** trust a decompiler here: the handover doc says its own decompiler section is
not to be built on, and an SEH registration is exactly where a decompiler's frame reasoning is
weakest.

### Callers

Eighteen by exhaustive rel32 (`docs/BOT_PROBE_007F0280.md`). Extents differ per caller and are
sometimes computed, and the final argument is a per-caller **mode**:

| caller | extents | mode |
| --- | --- | --- |
| this stream's attack run, `009C4258`/`009C4268`/`009C4287` | 80, 60, 120 | **1** |
| torpedo goaway | 60, 50, 90 | **0** |
| torpedo aim tick | `{72t, min(0.7 x 72t, 150), 1.5 x 72t}` | - |

A shared pure function must take the extents and the mode as parameters. Hard-coding the triple
would be wrong for fifteen of eighteen sites.

At this stream's own call site the three out-pointers and the mode are **pushed**
(`009C4260` `PUSH 1`, then `009C4276`/`009C4280`/`009C4293` pushing LEA'd pointers) while the three
extents are **stored** into the argument window by `MOVSS` at `009C4262`/`009C4281`/`009C4294`.
That is why counting pushes gives five and counting the cleanup gives six. **Which push lands in
which of the six slots is not established** and belongs to the body read.

---

## 3. The `009C42B8` hole, and where the offset is consumed

`src/game_hosts_units.cpp` passes `in.sampler_result = 0.0f; in.sampler_ran = false;` for the
`007F0280` call at `009C42B8`. **This is a hole, not a contract**, and the distinction matters:

* At the torpedo goaway's `009D0CBC` a zero is a **proof**: the caller zeroes the three out-slots
  itself at `009D0C96`-`009D0CAA` and the only use is a strict sign test on `-p[0]*p[1]*p[2]`, so a
  no-hit answer cannot fire it. **Leave that one.**
* Here the result feeds the lateral offset, so zero asserts the probe always returns zero.

Consumed at `src/dive_bomb_task.cpp:395-419`, and checked from this side rather than assumed:

```cpp
out.lateral_offset_20 = in.lateral_offset_20;                 // 399, carried on entry
...
} else {                                                       // the RE-ROLL arm only
    out.lateral_offset_20 = -in.sampler_result * kLateralOffsetScale;   // 412
}
// 009C42DC-009C4305, run on BOTH arms:
out.commanded_heading_2c0 =
    wrapped_angle_add_00438aa0(in.target_bearing_c0, out.lateral_offset_20);   // 418
```

The offset reaches the commanded heading on **every** path (the heading line is outside the `if`),
and it is only **recomputed** on the re-roll arm, so a zero persists between re-rolls. The census
reports `rerolls=153` over the run-in, so **the commanded heading is the bare bearing to the target
for the whole approach**: the aircraft flies straight at its target and never weaves.

**Replace this stand-in first** when the probe's body is read.

---

## 4. `task+41Ch`, scoped and deliberately not bound

`src/game_hosts_units.cpp` has `in.speed_ratio_41c = 1.0f;` - a stand-in with a **known formula**,
not an unread one:

```
009F9D37  FLD   float ptr [EAX + 0x188]     ; desc+188h MaxSpd
          FDIV  the stacked reference       ; 009C3EC4 loads it from [EAX+4D8h]
          FLD1 / FCOMIP / JBE               ; max(., 1.0)
009F9D61  MOVSS dword ptr [ECX + 0x24],XMM0 ; approach+24h, which is task+41Ch
```

It scales the in-range latch: `009C8A1B`-`009C8A5E` makes `task+4B0h = max(itself, AttackDist x
task+41Ch)`. Every run so far reports `approach+B8h` as exactly **1100.0 m**, which is the bare
`Pilot/DiveBomb/AttackDist` and is **a measurement of the stand-in, not a recovered value**.

**It must get its own window, and after the weave is fixed.** Binding it moves `approach+B8h`, which
moves where the latch closes, where `flyabove` starts, and every geometry number in the tables -
and a before/after taken on a path that still flies straight instead of weaving would have to be
taken again the moment the probe lands.

---

## 5. Every retraction from this stream, so none is re-raised

| retracted | commit |
| --- | --- |
| "`0099E4DC`-`0099E512` is probably a delta on the target, not a floor" - the listing shows a max, no `FADD` anywhere in the block | `8647b8c02` |
| "`class_gain` is zero for this class" and the `Select-String -List` search that appeared to confirm no aircraft carries `DropAngle` - there are 74, and the measured gain is 0.577 | `b8a3fd790`, `9f57e7d0e` |
| "`kWreckAnchorLateralDivisor`'s sites are none of them death-sink code, so it is an address error" - `008250F3` is inside the region printed six lines above the constant; address right, value wrong | `39abb2fbe` |
| "a longer run will show a second `goaway` -> `flyabove` circuit" - the 8800-frame run stops at the same tick; the aircraft ditches | `ae2463b13` |
| "the image overshoots this pass too" - withdrawn with it | `ae2463b13` |
| "176 `DropAngle` rows" in a commit message - it is 74 | noted in `b8a3fd790`'s doc |

Also **not** a defect, corrected by the lead and recorded here so it is not re-opened: the
`009FBA50` command sitting at its ceiling through the run-in. `base + distance x tan(angle)` clamped
to a ceiling **is** a glide-slope law; at 9.7 km it is supposed to be on the ceiling, and it only
comes off when `span x scale x gain < 450`, i.e. inside about 810 m - which `approach+B8h` = 1100
prevents the run-in from ever reaching. The law is not shown wrong; it simply cannot explain the
ditch in this regime. Both settling facts are now named:

* **the 1450** is the authored `Dynamics/Ceiling` = **1500** in this installation's
  `scripts/datatables/planeglobals.lua` ("ez a plafon") with a 50 m margin in the clamp - a separate
  authored altitude, **not** `base + 450`. The exact form of the 50 m margin is the one detail not read.
* **`class+518h` IS `tan(DropAngle)`**: `include/bsp/bot_task_states.hpp:330` records
  `gain = tan(desc+1F0h DropAngle)` with the store site `007C4A3F`/`007C4A44`. The measured 0.577 is
  `tan(30 deg)`, and this class's `DropAngle` is 0.5236.

---

## 6. Run rules

* Launch **only** through `tools/run_game.ps1`, which takes one of three numbered slots and passes
  that slot's `--instance-tag` and `--affinity-core`. An untagged run takes the game's own
  `MidwayThreadMutex` and stops the user starting the retail game.
* **Never pipe the launcher through a short-circuiting cmdlet.** `./tools/run_game.ps1 ... |
  Select-Object -First N` kills the run silently: exit 0, no log, nothing to read.
* Every run gets its **own** `-Log` path; the launcher refuses a path another live slot is using.
* Runs outlast the 600 s foreground cap and are backgrounded, and **nothing wakes you** when they
  land. Note the expected end and check the log then.
* `query session` must show the console **Active**; a disconnected session fails at FMOD init.
* Judge a run by its log contents, never by the exit code.
* Merge `main` before any run, and say which side of `67e8ac821` a log is on: that commit holds back
  every `Hidden` authored object, so USN04 creates 19 units at load instead of 53 and every aircraft
  census moves with it. Logs from the two sides must not share a table.
