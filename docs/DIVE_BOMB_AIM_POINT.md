# The dive-bomb aim point: what produces it, and what it does not contain

Packet `cc8_dive_aim`, branch `agent/cc8-dive-aim`. Successor of `cc8_dive_goaway` on the dive-bomb
attack chain. This document answers the question `docs/DIVE_BOMB_APPROACH.md` line 332 left open -
"the aim point `approach+4Ch`/`+50h`/`+54h` still has no producer read" - and it retracts the
premise the packet was framed on.

## 1. The headline, and the retraction

**The image does not lead its dive-bomb aim point.** The packet brief said "the loss is in the LEAD
- where the aim point sits relative to the moving target". That is refuted. Neither half of the
image's dive-bomb solution contains a target-motion term:

* `009C7D71`'s predicted impact point is `own position + own velocity x tf`. The only thing fed to
  `007BCC80 BSP_Weapon_DropFallTime` is a height. Section 3.
* the aim point is the target's **live world pose applied to a body-frame point**, recomputed every
  tick, with no velocity and no fall time anywhere in it. Section 2.

So a released round is aimed at where the ship *is*, and lands about one fall time later, by
construction. A 2.6-3.25 s fall against a ship making 8-18 m/s is 20-60 m of ship movement, which is
the order of the 6.1-57.0 m "miss" `docs/HANDOFF_DIVE_BOMB_FLYOVER.md` (b2) measured. Most of that
number is the image's own behaviour and must not be "corrected".

## 2. The producer: `009FADA0`, through a sub-object at `approach+30h`

The reason no census found the writer is that the aim point is not addressed as a field of the
approach at all. `009C3EDF LEA ECX,[ESI+30h]` in the approach constructor `009C3EA0` shows a
**target-reference sub-object at `approach+30h`**, with its own vtable `00D21CB4` installed by
`009FB200` at `009FB237`. The aim point is that sub-object's `+1Ch`/`+20h`/`+24h`. Any search for a
store to `approach+4Ch` or to `task+444h` is looking at a displacement the writer never uses.

`009C7A80 BSP_BotTaskDiveBombApproach_Update` drives it as its **first act every tick**:

```
009c7a93  LEA   ECX,[ESI + 0x30]          ; the target-reference sub-object
009c7a96  FSTP  float ptr [ESI + 0xc4]    ; (the +C4h dt accumulator, unrelated)
009c7a9c  FSTP  float ptr [ESP]           ; dt
009c7a9f  CALL  0x009fada0                ; refresh the aim point
```

### The per-tick body, `009FAEBD`-`009FAF00`

The sub-object carries **two** reference slots and they are not interchangeable: `sub+14h` is the
one the constructor fills with the validated target (`009FB26C`) and whose `+5Dh` byte the died-path
tests, and `sub+18h` is the one every aiming read goes through - the pose refresh and the matrix
here, and `target->vtable[+100h]` in `009FA260`. `009FB26F` zeroes `+18h` at construction, so
something outside this routine arms it; that arming is NOT established here.

```
009faebd  MOV   EDI,dword ptr [ESI + 0x18]   ; the target entity the aim is taken from
009faec0  CMP   byte ptr [EDI + 0xc8],0x0    ; pose stale?
009faecb  CALL  0x00414db0                   ; BSP_EntityPose_RefreshWorld
009faed0  LEA   EAX,[EDI + 0xcc]             ; the target's world matrix
009faed6  PUSH  EAX
009faed7  LEA   ECX,[ESP + 0x10]
009faedb  PUSH  ECX                          ; destination
009faedc  LEA   ECX,[ESI + 0x28]             ; SOURCE POINT, body frame
009faedf  CALL  0x004142e0                   ; BSP_Vector3f_TransformAffinePoint
009faee4  MOVSS XMM0,dword ptr [ESP + 0xc]
009faeea  MOVSS dword ptr [ESI + 0x1c],XMM0  ; aim point x = approach+4Ch
009faef5  MOVSS dword ptr [ESI + 0x20],XMM0  ; aim point y = approach+50h
009faf00  MOVSS dword ptr [ESI + 0x24],XMM0  ; aim point z = approach+54h
```

The argument order is settled by the ledger's recovered ABI for `004142E0
BSP_Vector3f_TransformAffinePoint`: *"ECX = source XYZ; stack destination XYZ, matrix; RET 8"*. So
`ECX = sub+28h` (= `approach+58h`) is the **source point in the target's body frame**, and the
result is that point carried into the world by the target's own live transform.

**aim point = target_world_matrix x body_frame_offset, every tick.** No velocity. No fall time.

### Where the body-frame offset comes from: `009FA260`

```
009fa266  MOV   ECX,dword ptr [ESI + 0x18]   ; the target
009fa272  MOV   EAX,dword ptr [EAX + 0x100]  ; target->vtable[+100h]
009fa29f  PUSH  EDX                          ; out
009fa2a0  CALL  EAX
009fa2a4  FSTP  float ptr [ESI + 0x28]       ; -> the body-frame offset
009fa2ad  FSTP  float ptr [ESI + 0x30]
```

**The target itself chooses where on its hull the bomber aims**, through `target->vtable[+100h]`,
given `&sub+48h`, `&sub+54h` and four floats from `sub+64h`..`+70h`. That virtual is UNREAD; this
document makes no claim about what point it returns.

`009FB200` initialises that four-float block at `009FB2F1`-`009FB310`: `sub+64h` = `00D7A260` =
**-1.0**, and `sub+68h`/`+6Ch`/`+70h` = `00D7A24C` = **1.0** each. A `-1` beside three `+1`s reads
like a selector followed by a per-axis extent, which is the shape a "pick a point within this box on
my hull" call would take - but that is a **hypothesis about an unread virtual** and nothing in this
packet tests it.

And the aim point does not start empty. When the constructor is given a target,
`009FB32A`-`009FB346` seeds `sub+1Ch/+20h/+24h` **directly from the target's raw position**
`EDI+FCh/+100h/+104h`, after the same pose refresh. So the aim point begins as the target's origin -
exactly what this host uses for all time - and only diverges from it once `009FA260` has put a
non-zero offset in `sub+28h`.

The offset is re-picked on three triggers, `009FAE26`-`009FAEAB`:

| trigger | evidence | value |
| --- | --- | --- |
| a timer `sub+60h += dt` past a threshold, then re-armed from a uniform draw | `009FAE29`-`009FAE66`, `00BD2F10 BSP_Random_UniformFloatRange` | threshold `00CE3958` = **2.0**, re-arm range `00CE69D0`..`00CE3800` = **-0.5 .. +0.5**, so every **1.5-2.5 s**. The FIRST one is sooner and differently distributed: `009FB2D8`-`009FB2EC` seeds `sub+60h` from `Uniform(0.0, 2.0)`, so the first re-pick lands uniformly in the first 2 s |
| the target refuses the current offset | `009FAE98`-`009FAEAB`, virtual `target->vtable[+104h]` over `sub+28h..30h - sub+34h..3Ch`; `AL == 0` -> `009FA260` | - |
| the dirty byte `sub+41h` | `009FAEB0`-`009FAEB8` | set to 1 by `009FB272` at construction |

### The target-died path, `009FADAE`-`009FAE18`

When `byte [sub+14h + 5Dh]` is set (`009FADB5`), the tick takes one final sample through the same
transform - reading the pose from `sub+18h`, not from `+14h` - calls
`006952A0 BSP_Observer_UnregisterPair`, and nulls `+14h`/`+18h` and the dirty byte. The aim point
then **freezes at the last known point** and `009FADAC`/`009FADB3` skip the block forever after.
This is a weak reference being released, not an aim behaviour.

### What is NOT established

* `target->vtable[+100h]` is unread, so the magnitude and distribution of the body-frame offset are
  unknown. On a 180-270 m hull it could be tens of metres from the origin, or it could be zero.
* `009FB200` seeds the offset from `00F87574`/`+78h`/`+7Ch`, which `tools/pe_const_read.py` reports
  as `.data` **past raw size** - loader zero-fill. So the seed is `(0,0,0)` and the offset is
  non-zero only once `009FA260` has run. Per the standing rule about `.data` past raw size, that is
  a statement about the image on disk, not proof that nothing writes it at runtime.
* `sub+44h` (= `approach+74h`), the gate at `009FAF0A` against `00D7A218` = **0.0**, is initialised
  to 0.0 by `009FB25F`, so the tail block past `009FAF13` is off until something sets it. Unread.

## 3. The predicted impact point has no target term either

`009C7A80`'s tail, already in `docs/DIVE_BOMB_TASK.md` and re-confirmed here from the decompilation:
`tf = 007BCC80(unit, unit.y - aimPoint.y) + 0.1` (`00D7A3A0`), `v = unit->vtable[+34h]()`, then
`+D8h = unit.x + tf*v.x`, `+E0h = unit.z + tf*v.z`, `+DCh = aimPoint.y` (`009C7E33` overwrites the
`009C7DF1` store). `v` is the **aircraft's** velocity; the target appears only as `aimPoint.y`, a
height. The aimdive then steers to drive `+D8h/+E0h` onto the aim point, which is what makes the
25 m gate at `00CE3880` a CCIP window.

## 4. The host's defect, in one sentence

`src/game_hosts_units.cpp` aims at the commanded target's **origin**, where the image aims at
`target_world_matrix x target->vtable[100h]`'s body-frame hull point (`009FA260` -> `009FAEDF`), so
the host loses the target-chosen aim spot and its 1.5-2.5 s re-roll cadence - while the **absence of
a lead**, which this packet was framed around, is faithful and must not be changed.

## 5. The census this packet adds, and the prediction made before the run

`docs/HANDOFF_DIVE_BOMB_FLYOVER.md` (b2) warned that the aimdive summary's `tf=` column (3.35 s for
`movieval`, 7.62 s for `#7.1`, 10.64 s for `#3.1`) is **last-sampled**, read after the aircraft has
left the dive, and so cannot be quoted as the fall time in force at release. This packet closes that
with one print-only field set, every label naming when it was sampled:

* `tf@release` - `009C7D71`'s own `tf` on the tick the round left (`GameProjectileRow::release_fall_time`)
* the target's speed and hull heading **at release**
* the target's position and hull heading **at impact**
* the miss against the target **at impact**, resolved into along-course and across-course components

Print-only, so the pair must be a null on every behavioural column.

### PREDICTION, recorded before `local\aim_before.log` was run

1. **Null pair.** `total_damage=10188.4`, `deaths=9`, `bomb_impacts=18`, `bomb_drops=19` and every
   dive-bomb state count identical to `goaway_after.log`. A single moved behavioural number
   falsifies the "print-only" claim.
2. **`tf@release` will be 2.6-3.3 s for every round, not 7.6 or 10.6 s.** The three-times values are
   an artefact of last-sampling, so `007BCC80` and its feed are NOT the defect and the error is in
   the geometry. If `tf@release` is instead ~3x the measured fall, this prediction is wrong and the
   defect is in `007BCC80` or what feeds it.
3. **`miss_along` will be predominantly negative** - the bomb lands ASTERN of where the ship got to -
   because neither the aim point nor the impact point leads. Magnitude of order
   `target_speed_release x life`, i.e. 20-60 m for a ship making 8-18 m/s.
4. **`|miss_across|` will be much smaller than `|miss_along|`** on the accurate squadron. A large
   across-course component would mean the aim point is off the hull line, which is the signature the
   unread `vtable[100h]` offset would leave - and is what would justify a later packet on it.

### MEASURED, `local\aim_before.log`

**Prediction 1 (null pair) CANNOT BE TESTED FROM THIS RUN, and the fault is mine.** I compared
against `goaway_after.log`, which was built in a different worktree at commit `b7be4aca1`; this tree
is at main `4e02a7a78`, many packets later. The two are not a pair and never were, and the numbers
duly moved: `bomb_drops` 19 -> 23, `bomb_impacts` 18 -> 20, `deaths` 9 -> 14, `total_damage` 10188.4
-> 14042.2, `first_hit` 17.15 -> 13.35 s. **None of that is evidence about this census.** The change
is print-only *by inspection* - added struct fields, one extra parameter, read-only position and
heading samples, one log line, nothing feeding back into the simulation - but "print-only" is a
claim that must be MEASURED, and it is not measured here. A true before needs a run from this tree
with the census reverted. Left for the successor; see the handoff.

**Prediction 2 CONFIRMED, and it is the packet's real result.** `tf@release` is **2.38-3.24 s** on
every round, against measured falls of 2.6-3.25 s. The aimdive summary's 7.62 s (`#7.1`) and 10.64 s
(`#3.1`) are therefore confirmed **last-sampling artefacts** and nothing else. **`007BCC80
BSP_Weapon_DropFallTime` and its feed are cleared**; the error is in the geometry, which is where
sections 1-3 put it.

**Prediction 3 CONFIRMED, quantitatively.** Against the moving target (speed 16.7 m/s) `miss_along`
is negative on every single round, -45.9 to -77.0 m - the bomb lands **astern**. `16.7 m/s x ~3 s`
is about 50 m, which is the along-course miss to within the spread. That is the no-lead signature,
measured.

**The control is in the same run and it is decisive.** `D3A Val #1.1`'s target is **stationary**
(`speed=-0.0 m/s`, and its position is identical at release and at impact). Its two rounds miss by
**25.5 m and 11.3 m**, with `along` **+11.1** and **+3.8** - the along-course miss collapses to
nothing the moment the target stops moving, while the moving-target rounds sit at -45 to -77 m. A
stationary-vs-moving contrast inside one run, on the same binary and the same tick, is much stronger
evidence than the before/after pair I failed to set up.

**Prediction 4 FALSIFIED.** `|miss_across|` is NOT much smaller than `|miss_along|`: it runs -28.0
to -72.6 m, comparable throughout. The reason is visible in the census's own new column - the target
is **turning**, its heading moving from -1.216 to -1.295 rad between release and impact - so its
displacement during the fall has a large cross-course component too, and both components are target
motion rather than an aim-point error. This does NOT show the unread `vtable[100h]` hull offset; the
stationary control's across term is only -23.0 and -10.7 m, which is the prediction-error scale.

| round | target | `tf@release` | speed | miss vs target at impact | along | across |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | moving | 3.24 s | 16.7 m/s | 78.7 m | -52.8 | -58.4 |
| 2 | moving | 2.73 s | 16.7 m/s | 97.2 m | -64.6 | -72.6 |
| 3 | moving | 3.21 s | 16.7 m/s | 67.4 m | -45.9 | -49.4 |
| 4 | moving | 2.67 s | 16.7 m/s | 82.9 m | -59.8 | -57.3 |
| 5 | moving | 3.22 s | 16.7 m/s | 81.9 m | -77.0 | -28.0 |
| 6 | moving | 2.38 s | 16.7 m/s | 67.9 m | -47.0 | -49.1 |
| 7 | **stationary** | 3.24 s | -0.0 m/s | **25.5 m** | **+11.1** | -23.0 |
| 8 | **stationary** | 2.72 s | -0.0 m/s | **11.3 m** | **+3.8** | -10.7 |

**A census bug this run exposed, now fixed.** The `target` column of these rows printed the
**bomber's** name, not the target's - the rows read `target D3A Val #3.1`, which is the aircraft.
That is precisely the trap the packet rules name ("a summary column's meaning comes from its
printing code"), walked into while adding a census meant to close another instance of it. The row
now carries `GameBombImpactRow::target_name`, filled from the ordered target. The positions, speeds
and headings in the table above were always the target's and are unaffected; only the name was
wrong. The fix is a format argument and is itself unmeasured.

## 6. Item 4: the long run's summary is not windowed, it is a different simulation

The integrator asked whether a bounded record buffer, a windowed counter or a reset on some event
caps the end-of-run summary, because `goaway_long.log` reports `bomb_impacts=0` and
`total_damage=3596.8` against the 4800-frame run's 18 and 10188.4. **Nothing caps it**, and the
logs say the run diverged rather than the summary mis-reporting:

* `bomb_impacts` is an unbounded `std::vector`, appended at `src/game_hosts_gunnery.cpp` in the
  projectile sweep, never cleared, printed as `rows.size()`.
* `GameGunnerySummary summary{}` is a member initialised once and never reset. `++h.summary.bomb_drops`
  has no window; the `<= 6` test beside it is a log-spam limiter on the per-drop note, not the counter.
* `goaway_long.log` reports **`bomb_drops=0`**, and that counter is incremented at the DROP, long
  before any impact record exists. Zero drops, not zero records.
* `first_hit` is **earlier** in the long run, 15.10 s against 17.15 s - impossible for a run that is
  a prefix-superset of the short one.
* the two censuses disagree inside the long log itself: `summary mission dive-bomb task: aircraft=24
  releases=30` against `bomb_drops=0`.
* the launch parameters differ by more than `--mission-frames`: `frames=9200 mission_frames=9000`
  against `frames=5000 mission_frames=4800`. The pre-mission budget is the same 200 frames either
  way, so the predecessor's first candidate is closed, but the runs are not comparable.
* what **is** identical is the torpedo arm - `moveto=522/536/522`, `attackrun=262`, `releases=12` in
  both - which is the determinism the `gunnery:` event lines showed. It does not extend to the bombs.

No fix is warranted; a change here would be tuning a non-bug. The consequence for the chain is that
the second-attack-run demonstration in `docs/HANDOFF_DIVE_BOMB_GOAWAY.md` section (a) (`releases`
1 -> 2, `rounds_left` 1 -> 0) rests on a log whose gunnery side says no bomb was ever dropped, and
should be re-run with `--instance-tag`/`--affinity-core` before it is quoted again.
