# The aim-complete test (packet `cc8_torpedo_aim_complete`): a negative result

Addresses: `009D15F0` (`009D15F0`-`009D2377`), the test at `009D22FF`-`009D236E`, `009D31B0`,
`00419010`, `007F0280` (`007F0B1F`), and the frame-walk evidence below.

**This packet did not produce the rule, and it should not have.** The two-clause test that writes
`state+2Ch` reads four stack slots, and a dominator computation over the aim tick's own control-flow
graph shows that **two of the four have no write that dominates the read**. A single-formula
reconstruction would be wrong on at least one path. Worse, the two independent methods used here -
a corrected stack-frame walk and Ghidra's own variable identities - **disagree** about one of the
slots. Publishing either answer would have been a guess dressed as evidence.

What follows is the method, the numbers, and where the disagreement sits, so the next attempt starts
from something checkable rather than from scratch.

## (1) The raw frame walk is not usable as printed

`python tools/stack_frame_walk.py 009d15f0` prints a depth for every instruction, but it stops
accounting at eight call sites and says so:

| address | site | what the walker reports |
| --- | --- | --- |
| `009D1679` | `CALL EDX` | 0 bytes pushed since the last call, cleanup unknown |
| `009D1714` | `CALL EAX` | 4 bytes pushed |
| `009D1721` | `CALL EAX` | 0 bytes pushed |
| `009D175F` | `CALL EDX` | 4 bytes pushed |
| `009D176E` | `CALL EDX` | 4 bytes pushed |
| `009D1A94` | `CALL 007F0280` | unresolved callee |
| `009D1E08` | `CALL EAX` | 4 bytes pushed |
| `009D1E17` | `CALL EAX` | 4 bytes pushed |

Every indirect site is a `__thiscall` virtual, which is callee-clean and pops exactly what the
walker reports as pushed. `007F0280` ends `RET 0x18` at `007F0B1F`, so it pops 24. Applying those
eight cleanups gives a cumulative correction of **44 bytes** by the time control reaches the test,
and only then can a read and a write be matched to one frame slot. `local/slots2.py` in this
worktree does that and prints every access grouped by corrected slot.

Without the correction the walk conflates slots: in the raw output the test's threshold groups with
nineteen accesses that belong to three different variables.

## (2) The four slots the test reads

With the correction applied, writing `K` for the corrected slot key:

```
009D2345  ramp = BSP_Math_InterpolateClamped(0.0, 0.0, 1.0, speed * 0.5, K76)
009D234A  acc  = K68 - ramp
009D235A  if (K36 > acc)  goto set
009D2368  if (K64 > K76)  goto set
009D236E  state->+2Ch = 1
```

`speed` is the `009D3C99` switch on `approach+134h` between `+7Ch` and `+80h`, and `[00D7A280]`
is the `0.5`.

## (3) Dominance: which writes are guaranteed

`local/dom.py` builds the aim tick's basic blocks from the listing, computes dominators, and asks
for each candidate write whether its block dominates the block of the read.

| slot | read at | candidate writes | dominates? |
| --- | --- | --- | --- |
| `K36` | `009D22FF` | `009D214E`, `009D212A`, `009D20FA`, `009D20E6` | **no** |
| `K36` | `009D22FF` | `009D1FA5`, `009D1E9E` | yes |
| `K68` | `009D2310` | `009D16F8` | **no** |
| `K68` | `009D2310` | `009D1673` | yes |
| `K76` | `009D2318`, `009D2360` | `009D1FF6`, `009D1F99` | yes |
| `K64` | `009D2364` | `009D174C`, `009D1723` | **no** |

`009D210E` is the branch that makes this concrete: `JBE 0x009D2167` jumps clean over
`009D2110`-`009D214E`, so the bank threshold that block computes is written on one path and not the
other. Four separate edges reach the tail (`009D2233`, `009D228D`, `009D22A0`, `009D22CA`), and
`K64` has no dominating write at all.

**So the thresholds of the aim-complete test are path-dependent slot reuses, not constants or
single expressions.** The test cannot be lifted out of the tick.

## (4) The one slot that is clean, in full

`K76` is produced by a single dominating block, `009D1F99`-`009D1FF6`:

```
009D1F94/99  K76 = the commanded speed          /* approach+7Ch on one arm */
009D1F9F     a   = <an angle slot>
009D1FA3     c   = cos(a)                        /* FCOS */
009D1FBF     t   = |c|                           /* AND 0x7FFFFFFF */
009D1FED     r   = BSP_Math_InterpolateClamped([00CE3800] = 0.5f, 1.0f, 1.0f,
                                               approach->+84h, t)
009D1FF2/F6  K76 = r * K76
```

So `K76` is a **distance**: an interpolation between `1.0` at `|cos| = 0.5` and `approach+84h` at
`|cos| = 1.0`, multiplied by the commanded speed. The same quantity feeds the `approach+130h` write
at `009D2021`, where `[00CE4D70] = 200.0` is added to it.

That also settles a loose end: `K76` is **not** a time, so reading the second clause as "time below
the turn magnitude" - which this packet's predecessor guessed in
`docs/TORPEDO_APPROACH_UPDATE.md` - is wrong.

## (5) Where the two methods disagree

For `K68`, the minuend of clause 1:

* the corrected frame walk puts the dominating write at `009D1673`, which stores the **commanded
  altitude floor** (`max(over_land ? 30 : 5, approach+78h + approach+74h)`, the max computed at
  `009D165B`-`009D1665`);
* Ghidra's decompiler names the same operand `fStack_5c` and assigns it the **bearing error**,
  `BSP_Math_SubtractWrappedAngle(approach+94h, unit->vtable[50h]())`.

Those are different quantities. One of the two methods is wrong, and this packet does not establish
which. The most likely cause is that the corrected walk is still off inside one of the four
`SUB ESP,0x14` argument windows, where the same `[ESP+n]` denotes an argument slot on one side of
the call and a local on the other; `009D2085` and `009D217D` are visible instances, both writing
`[ESP+0x64]` into what is otherwise the local area.

**No rule is reconstructed and nothing is wired**, so `state+2Ch` is still unproduced and the host's
behaviour is unchanged.

## ABI summary

| address | ABI | evidence |
| --- | --- | --- |
| `009D15F0` | `void __thiscall(state, float dt)`, `RET 4` | `009D2377`, and `ADD ESP,0x64` at `009D2374` |
| `009D31B0` | `char __fastcall(state)` | `009D31C2` |
| `00419010` | `float __cdecl(x0, y0, x1, y1, t)`, caller-clean 20 bytes | the walker resolves the cleanup at `009D2345` and `009D1FED` |
| `007F0280` | `RET 0x18`, pops 24 | `007F0B1F` |

## Corrections

### Correction to `docs/TORPEDO_APPROACH_UPDATE.md` (packet `cc8_torpedo_approach_update`)

Its section (4) reconstructs `torpedo_aim_tick_009d15f0` with an `aim_solution_130` derived from a
bearing error compared against a time to target, and describes the tick's interpolant as a time.
Section (4) above shows the quantity at `K76` is a **distance**, an interpolation on `|cos|` of the
commanded altitude scaled by the commanded speed. That reconstruction's `aim_solution_130` rule is
therefore not supported; it is marked `coverage: partial` there already, and this is the specific
part that is wrong.

### Correction to `docs/TORPEDO_RUN_IN_PATH.md` (packet `cc8_torpedo_run_in_path`)

Its section (5) describes the second clause as "a time against the absolute turn offset". The right
operand is `K76`, the distance of section (4) above. The left operand `K64` has no dominating write,
so its identity is open.

## `no_ghidra_function`

None. Every routine read here has a Ghidra function.

## Validation

`./scripts/build.ps1` succeeds and `ctest` passes both suites; the tree is unchanged from
`afb2fb184` apart from this document, its report and the analysis scripts under `local/`. **No game
run is attributed to this packet: no source changed, so there is no "after" to measure.** The USN01
and USN02 numbers stand where `docs/TORPEDO_RUN_IN_PATH.md` left them.

## Follow-up packets

1. **Reconstruct `009D15F0` whole**, branch by branch, rather than lifting the test out of it. That
   is the only sound way to produce `state+2Ch`, and it subsumes the four throttle interpolation
   folds between `009D1B00` and `009D1D2E` that are still `coverage: partial`.
2. **Settle the `K68` disagreement** first, since it decides whether the corrected frame walk can be
   trusted for the rest of the body. A single-step trace under the debugger over one aim tick would
   do it directly; `tools/stack_frame_walk.py` could also grow an option for callee-clean indirect
   calls so the correction in section (1) does not have to be applied by hand.
3. **`approach+84h`**, the interpolation's upper bound, and `unit+C64h`, the bank the skipped block
   at `009D2110` compares against.
