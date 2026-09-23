# Fly-over speed: the paired frame slot, the desired-speed arm, and the four-run table

Addresses: 009C62B0, 009C6453, 009C641C, 009C6AD9, 009C6B3A, 009C6B75, 009C6BB4, 009C6BC4, 009C6F97, 009C6FB0, 009C6FB9, 009C6FFB, 009C7062, 007C4810, 007E413B, 007E41DF, 007F0280, 0042E740, 00438B10

Packet `cc9_flyover_speed`, 2026-09-22. It follows `docs/AIMDIVE_RESPONSE.md` section 2. The
report is `reports/flyover_speed.json`. All names are hypotheses, not recovered symbols.

**Result.**
- **The slot is paired.** Every candidate writer targets the same frame slot. The value that
  survives to the speed arm is 0.0, unless the near-field avoidance probe fires.
- **The desired speed is 0.95 x MaxSpd**, held in speed mode. The minimum-controllable-speed term
  only matters when avoidance is active.
- **Bound behind `kFlyoverSpeedBound`,** together with feeding approach+50h the aim point's height.
- **Nearly inert.** It moves USN04 by a few ticks on the Yorktown attackers only, and releases stay
  at 23. It does not narrow the aim-error swing or the sideways hand-off.
- **Decision:** see section 5.

## 1. The ESP method and its anchors

`tools/stack_frame_walk.py 009C62B0` walks the listing linearly. It subtracts a callee's cleanup
only when the callee is a direct call with a known `RET imm`. So its depth is right up to the first
indirect call, and it over-counts after each one.

**Anchor.** The epilogue at 009C7062-009C706C is POP EDI, POP ESI, POP EBP, POP EBX, ADD ESP,88h.
So the true depth before `POP EDI` is 88h plus four saved registers, 152 bytes. The walk reports
196 there, which is 44 bytes too many. The body base depth after the prologue (009C62B0-009C62C6)
is also 152.

**Where the 44 bytes come from:**

| call | pushes before it | callee cleanup | evidence |
|---|---|---|---|
| 009C62CF CALL EDX, unit vtable+34h | 1 | 4 | the out-pointer velocity getter, like 009FA2E0 (RET 4) |
| 009C6342 CALL EDX, approach vtable[0] | 1 | 4 | 009C40A0 ends `RET 4` |
| 009C647B CALL EDX | 1 | 4 | balance |
| 009C64EC CALL EAX (PUSH 14h) | 1 | 4 | balance |
| 009C6705 CALL EDX | 1 | 4 | balance |
| 009C6B3A CALL 007F0280 | 6 | 24 | 007F0B1F `RET 18h` |

These sum to 44. The last row is proven from the callee's own RET, and the first two from known
bodies. The three "balance" rows are the only assignment that sums to the anchor with one push per
call.

**Back-propagating** from the epilogue anchor gives a true depth of 152 at each point of interest:

| instruction | walk depth | known cleanups not subtracted | true depth | frame coordinate |
|---|---|---|---|---|
| 009C6453 MOVSS [ESP+2Ch] | 160 | 8 | 152 | entry-108 |
| 009C6AD9 MOVSS [ESP+2Ch] | 172 | 20 | 152 | entry-108 |
| 009C6BB4 / 009C6BC4 FSTP [ESP+2Ch] | 196 | 44 | 152 | entry-108 |
| 009C6FB9 MOVSS XMM1,[ESP+2Ch] (the read) | 196 | 44 | 152 | entry-108 |

**All three writers target the read's slot.** None of them is an argument push. This corrects the
framing in `docs/AIMDIVE_RESPONSE.md` section 2, which called the last two argument-window writes;
a correction is appended there.

## 2. What survives to the read

* **009C6453** stores |the wrapped difference between the fly-over's lead bearing and the
  heading|. That difference comes from 00438B10 at 009C641C. This is the value when the tick
  enters the heading arm.
* **009C6AD9** stores 0.0: XMM0 is zeroed at 009C6A9C. A python scan of all 109 jumps in the body
  finds none that crosses 009C6AD9 from before it, and the body's only RETs are at 009C706C and
  009C7083. So this store runs on every tick and overwrites the lead-bearing value.
* **009C6BB4 / 009C6BC4** run only if the near-field probe 007F0280 (called at 009C6B3A) returns
  |[ESP+68h]| above 00D7A270 = 0.05 (double, 009C6B3F-009C6B75). The stored value is
  `-[ESP+68h] x ... x [ESP+70h] x [ESP+74h] x 1.2 (00CEC160, double)`, and 0.2 (00CE3D10, double)
  is subtracted when the flyabove byte +18h is set. It is an avoidance speed trim.

So **the slot is 0.0 unless another unit is in the near field.** This host leaves 007F0280 unbound,
with its three out-slots at zero, so the test fails and the host's slot is 0.0, exactly as the
image's is with no neighbour.

## 3. The arm's law

**The speed arm, 009C6F97-009C6FFB:**
- `m = approach+A4h - 007C4810(approach+8h)`. approach+A4h is a float promoted to double
  (009C6FA2-009C6FAC); the call is at 009C6FB0 and 009C6FB5 is FSUBR.
- `d = s > 0 ? s x 40.0 : m x s`. The 40.0 is 00D7A378 (double). The compare is at 009C6FC2
  (COMISS) and 009C6FC9 (JBE).
- The stores: cmd+2B4h = `approach+A4h + d` (009C6FE1 FADD, 009C6FFB FSTP float), cmd+2B0h = 0
  (009C6FEA, byte) and cmd+2D8h = 1 (009C6FF1, dword). That is speed mode, with the speed
  correction enabled.
- approach+A4h is 0.95 x MaxSpd, set once in the constructor at 009C3F16.

**007C4810, `BSP_PlaneClass_MinControlSpeed`.** It is `__thiscall(class)`, returns ST0, and has no
stack arguments. It computes tuning+28Ch x class+184h (StallSpd): 007C4819 is the FLD, 007C481F the
FMUL, 007C4826 the FSTP float. tuning+28Ch is derived at 007E413B-007E41DF:

```
min(LevelFlight, max(CRMin + (CRMax - CRMin) * 0.85, StallRangeMax * 1.25, LevelFlight * 0.9))
```

The three constants are 00CF0B58, 00CF87C0 and 00D7A390, all doubles. The host never computed
`derived_28c`, so the rule derives it from the loaded fields.

**With s = 0 the law reduces to `desired speed = 0.95 x MaxSpd` in speed mode.** The host's
fly-over instead kept the attackrun's manual full throttle (cmd+2D8h = 0, throttle 1.0).

**approach+50h** is sub+20h, the aim point's height, which 009FADA0 stores at 009FAEF5. Behind the
same switch, `update_dive_bomb_approach` now feeds it with the fed point's y. The previous source,
an enter-time 0.0, rested on a census that predates the sub-object.

## 4. Predictions, then the runs

**Predictions, made before the runs:**
- **Exit speed.** The fly-over's exit speed falls by at most about 5%, where an aircraft actually
  reaches 0.95 x MaxSpd.
- **Aim-error swing.** It stays near ±200 m.
- **Turndown entry.** It barely moves, because the arm does not steer.
- **Sideways hand-off.** It stays at 72-270 m.
- **The height feed.** It moves only the Yorktown attackers, because the Lexington's origin y is
  0.0 and the Yorktown's is 0.1.

All five runs use the before-log parameters with the trace on. The logs are in this tree's `local\`:

```
./tools/run_game.ps1 -Exe local\bin_<v>\bsp_game.exe -Log local\<v>_usn04.log -- --frames 5000 --press-start-frame 30 --menu-select USN04 --mission-frames 4800 --mission-frame-seconds 0.05
```

| log | fly-over speed + height | aimdive tail | hull offset | releases | impacts | aborts | torpedo swims |
|---|---|---|---|---|---|---|---|
| `v0_usn04.log` (control) | off | off | off | 23 | 20 | 2 | 12 |
| `v1_usn04.log` | on | off | off | 23 | 20 | 2 | 12 |
| `v2_usn04.log` | on | on | off | 3 | 2 | 8 | 10 |
| `v3_usn04.log` | on | on | on | 0 | 0 | 2 | 12 |
| `v4_usn04.log` | height feed only | off | off | 23 | 20 | 2 | 12 |

**The control.** `v0` equals the previous packet's `coff_usn04.log` on every line. Its 12 extra
lines are new census prints from packets merged into main since then; there are no removed or
changed lines.

**`v1` against the control:**
- **Dive-bomb task rows.** Only D3A Val #7.1 and #7.1|.-3 move, by 1-2 ticks between done and
  aimdive, and their releases are unchanged.
- **Turndown entry.** movieval|.-2 is identical. #7.1 moves 0.2 m and 0.0007 rad of bank.
- **Bomb impacts.** They move about 0.1 m, and only for #3.1 and #7.1, the Yorktown attackers.
- **Consequent counters.** The gun sweeps go from 15080 to 15110 and the bodies from 5480 to 5490.
  The recon sensor pass moves by 6 observers. `torpedo_gate ticks` goes from 138762 to 139169.
  That counter is `++torpedo_gun_ticks` per torpedo-gun evaluation inside a sweep
  (src/game_hosts_gunnery.cpp:2138), so it follows the sweep count. `targeted`, `accepted`,
  `sent`, `drops` and `swims` are unchanged.

So the predictions held:
- The speed arm is inert on this mission's aimdive geometry. movieval's fly-over never reaches the
  target speed.
- The swing and the hand-off are unchanged.
- The sideways hand-off is not a speed effect.

**`v4` attributes the moves.** It feeds the height but keeps the speed arm disabled, and it is
identical to `v1` on every census line. So everything that moved in `v1` comes from the
approach+50h feed, 0.0 to 0.1 m on the Yorktown's aim height, and the speed arm moves nothing on
USN04. That is consistent with this mission's dive bombers never reaching 0.95 x MaxSpd in the
fly-over, where speed mode would hold full throttle anyway. I did not print the fly-over's speed,
so that last step is inferred, not measured.

## 5. Decision

* **`kFlyoverSpeedBound` = true.** `v1` is explained term by term against the control: every moved
  line belongs to the height feed on the two Yorktown squadrons, shown by `v4`. The one moved
  torpedo number, the gate's evaluation tick count, follows the gun-sweep count, and no torpedo was
  targeted, sent or swum differently. The default build therefore now carries both terms.
* **`kAimDiveTailBound` stays false.** On top of the speed arm it still takes releases from 23 to
  3, with aborts from 2 to 8. The fly-over speed was not the missing term that made the dive's
  pitch command saturate.
* **`kHullAimOffsetEnabled` stays false.** With everything on, releases go to 0.

**Still unread.** The aim-error swing of ±200 m is not caused by the fly-over's speed. The two
remaining candidates are the fly-over's steering and the unbound near-field probe:
* **Steering:** the heading arm steers at a point 3 s ahead, and the fly-over leaves the point
  72-270 m to the side at turndown entry.
* **Near-field probe:** 007F0280 is unbound in this host, and it is the only thing that makes the
  fly-over trim speed. It also runs in the run-in at 009C42B8 as the lateral-offset sampler.

The next read is the turndown-entry geometry. Which lead point does the heading arm hold at the
fly-over's last tick, and does the image's fly-over end on the point or deliberately off it? The
answer decides whether the swing belongs to the image or to this host.
