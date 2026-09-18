# Why the ring scan never scores a slot

Addresses: 009E7FC0, 009E80AB, 009E80B0, 009E80BD, 009E80CD, 009E80DF, 009E8116, 009E8197,
009E81A7, 009E5DA0, 009E6400, 009E6870, 009E6640, 0095EB40, 0085B7D0, 00864FD0, 00864D90, 00864680

Packet `cc8_ship_ai_approach_slot_scorers`, worker `agent/cc8-ship-approach-curves`. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; Ghidra was **read-only**. Every
descriptive name below is a hypothesis, not a recovered symbol.
`reports/ship_ai_approach_slot_scorers.json` carries the rows (3 call rows, 0 failures). It is the
fourth packet in a chain and it ends it, by finding the gate the other three were all sitting
behind.

## The four scorers did not need reconstructing

`009E5DA0`, `009E6400`, `009E6870` and `009E6640` are already whole on main, in
`include/bsp/ship_ai_ring_scan.hpp` and `src/ship_ai_ring_scan*.cpp`, from packet `cc_ai_ring_scan`.
`slot+18h` is not computed by a scorer at all: `009E5DA0` is a thin adapter that replaces word 5 of
its copy of the query block with `wrap(word5 - slot.angle_08)` and calls the firepower rating
`0095EB40`, whose answer lands in `slot+18h`. `009E7FC0` then divides every `slot+18h` by the frame
maximum and scales it by `tune+0h` into `slot+2Ch`.

That rating has had a real producer since packet `cc8_ship_ai_firepower_inputs`. So the question was
never "what fills `slot+18h`". It was "why is the answer still a tie", and the run answers it
directly: **`firepower=0` against `ring_scans=8400`**. The adapter is never called.

## `009E7FC0` returns at its first gate

The mode-0 arm has four gates before `009E81A7` scores anything:

| address | gate | what the host answered before |
| --- | --- | --- |
| `009E80AB` | probe the target kind, result discarded | recorded, 8400 calls, so the arm IS entered |
| `009E80B0` | `brain+0B28h` | **false, unconditionally** |
| `009E80BD` / `009E80CD` / `009E80DF` | `nested+127Ch > [unit+494h]` ends the pass | `nested+1290h` and `0.0f` |
| `009E8116` | `00864FD0` on the target | **false** |

Two of those were wrong in a way the codebase could have caught, because each contradicted another
binding of the same thing:

- `brain+0B28h` is read at `009E80B0` here and at `009E8747` in the attack binding. The attack
  binding returns the produced goal-vector flag; this one returned `false`.
- `00864FD0` is a thunk onto `BSP_UnitGunneryVisibility_Test` (`00864D90`), the gunnery pass's own
  cached line-of-sight test. `src/game_hosts_gunnery.cpp` answers that routine with `true`, for a
  stated reason that applies identically here: `00864680`, the sight test itself, is unread, and
  over open water with no terrain in this process the answer is yes. This one returned `false`.

A third was a mismatched field: `nested+127Ch` is word 0 of the firepower query block, the planar
range to the attackmove destination that `009F2A04` copies from `nested+11E0h`, but the binding
answered with `nested+1290h`. The fourth, `[unit+494h]`, is the same max weapon range `0095EB62`
gates on, which the gunnery host now produces.

All four are fixed. The census did not move, and the gate census added here says exactly why.

## The block the adapter gets

`009E8197..009E81A2` copies the seventeen dwords at `nested+127Ch` into the adapter's block and
`009E813D..009E8178` overwrites words 5, 6 and 7 and two bytes. The host was passing an **all-zero**
block, which would have starved the rating even once it was reached: with the four allow bytes
clear `0095EBD7` skips every category, and with `damage_cap` zero the output cap is zero too.

It is now built the way the image builds it: word 0 from `nested+11E0h`, word 5 from `nested+11DCh`
(`009E8153`), words 6 and 7 the `20.0f` and `60.0f` of `009E814B` and `009E8161`, and
`require_bearing` and `use_ready_rounds` both set (`009E8171`, `009E8178`). Words 1 to 4 use the
image's own no-target constants, the same labelled substitution the range-profile path makes.

Because the ring path sets `require_bearing`, `0085B7D0` is reached from it, and the firepower
binding answered that with a blanket `false`. Two of its arms are now transcribed: `0085B7E0` reads
the weapon Function and Function 8 returns true with no test at all (`0085B7E5`), and `0085B7F1`
wraps everything else in `range <= [projectile+60h]`, so a range past the class maximum falls
through to false. **Partial**: Function 7's traverse filter and the gravity-arc solve behind
everything else were not read, so a mount in range is allowed to bear.

## Validation

Build: `scripts/build.ps1`, Win32 Release, `/W4 /WX`, clean. Tests: 2 of 2 passing.

USN02, 3000 mission frames, against the tune packet's after-run: **identical on every number**.
Winner slot 0 everywhere, standoff 1450 / 1550 / 200, `shots=734 hull=180 deaths=2
total_damage=18525.6`, `heading_changes` 78 / 67 / 94 / 83, and `firepower=0` both times.

Nothing moved because nothing downstream ran. The new gate census says it in one line per ship:

```
gate Haguro   flag_0b28=0 ref_127c=0.0 look_0494=0.0 flag_stops=600 range_stops=0
gate Jintsu   flag_0b28=0 ref_127c=0.0 look_0494=0.0 flag_stops=600 range_stops=0
```

`flag_stops=600` of 600 scans, for all fourteen ships. The pass returns at `009E80B0` every single
time, which is why `ref_127c` and `look_0494` read back as zero: they are never reached.

### The next blocking input, by address and value

**`brain+0B28h`**, read at `009E80B0`, maintained by `009F1420`. Value: **false, on 600 of 600
scans, every ship.** Its producer is `src/ship_ai_goal_vector.cpp` lines 135 to 146, which sets it
true on arms whose conditions do not hold in this process. Nothing clears it after use, so it is
not a lifetime problem: it is simply never set.

That one boolean is the single gate between four packets of produced inputs and the ring scan that
would use them.

## Corrections

Appended to `docs/SHIP_AI_APPROACH_SLOT_TUNE.md` (its follow-up named the wrong blocker) and to
`docs/SHIP_AI_RING_SCAN.md` (words 0 to 4 of the `nested+127Ch` block have producers now).

## no_ghidra_function

none.

## Follow-up packets

- `ship_ai_goal_vector_visibility`: why `brain+0B28h` is never true. Everything else in this chain
  is now produced and waiting on it.
- `ship_ai_can_bear_arc`: `0085B7D0`'s Function 7 traverse filter and its gravity-arc solve.
- `ship_ai_approach_point_zone`: `00864BA0`, the no-target arm of the visibility gate, still false.
