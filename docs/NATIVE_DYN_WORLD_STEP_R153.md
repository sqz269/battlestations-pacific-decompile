# Native Dyn world step (R153)

The complete normal world simulation now connects the reconstructed collision
pass, group creation, native solver tasks, integration and pending-body removal.
The actual process fixture no longer supplies their call order or solver task
ranges. The existing semantic host projection remains separate.

## Recovered bodies

| Entry | Bytes | Original contract | Source function |
| --- | ---: | --- | --- |
| `00C5BB30` | 2575 | stack world, float dt; `RET 8` | `run_native_dyn_world_substep_00c5bb30` |
| `00C5C540` | 455 | ECX world, stack float dt; `RET 4` | `simulate_native_dyn_world_00c5c540` |
| `00C4D980` | 235 | ECX world; `RET` | `flush_native_dyn_pending_bodies_00c4d980` |
| `00C321B0` | 56 | ECX profile node; `RET` | `reset_native_dyn_profile_tree_00c321b0` |

The four bodies total 3,321 bytes. The 68-byte existing profile-enter entry,
five strings, epsilon and publication cells bring the live/PE evidence to
3,460 bytes in 12 spans. Descriptive names remain hypotheses, not recovered
original symbols. The source is `native_dyn_world_step.hpp/.cpp`; the detailed
report is `reports/native_dyn_world_step_r153.json`.

## Simulation and substeps

Simulation resets the actual engine profile tree's accumulated ticks and call
counts, increments its frame counter, enters Simulate, and increments world
`2Ch`. It flushes pending bodies before copying 12 transform words from each
remaining dynamic body's `+8` into its motion record's `+84h`.

The incoming dt is added to accumulator `48h`. Full substeps run while the
remainder is **strictly greater** than the current world step duration and the
integer budget at `34h` is nonzero. The duration is reloaded after callbacks.
A remaining budget permits a partial substep above the native double epsilon
`4.999999873689376e-5`. The accumulator is then zeroed, including budget-exhausted
and below-epsilon paths. Native x87 spills, comparisons and all three RDTSC
instructions are retained; the middle timestamp's unused result is not removed.

A substep runs velocity integration, collision and group creation. Mode 0 or 1
selects its own native solver task array and pointer vector. Inclusive task
ranges divide the active groups, with any remainder assigned to the last task.
Other mode values skip solver execution and contact reporting. Position
integration and group sleep/cleanup follow in every normal mode path.

The actual profile cells and existing profile append/enter implementations are
borrowed through the context. Cache offsets are `10h` for Simulate, `64h` for
CreateGroups, `6Ch` for Solve, `78h` for UpdatePosition and `7Ch` for SleepGroups.
The default scheduler, memory and CRT services are the existing implementations.

## Contact report and removal details

The optional callback at world `24h` receives an array of 88-byte records and
a count through ECX/vslot 0 with two stack arguments. Allocation reserves four
records per active manifold. Each populated record contains both transformed
local points, its normal, positive/negative `point+2Ch` times that normal, and
zeroes from `3Ch` through `50h`. Word `54h` retains its allocation preimage.
The two solver modes use distinct x87 instruction schedules; both are preserved.
The callback also runs with null storage and count zero when the active list is
empty. Storage is freed before position integration.

Ghidra's `_free` call-site override hid the three-byte continuation at
`00C5C44E..00C5C450`. It was repaired under the write lock, with prior evidence
saved. This removes a false early return from the decompilation. The library
callee annotation and four unreachable padding gaps were left unchanged.

Pending removal snapshots signed count `43Ch`, then reloads queue `438h` for
each entry. For bodies with shapes it calls the real broadphase removal slot,
clears flag 8 and the proxy pointer, captures each shape's next pointer before
scalar deletion, and clears the shape head. Existing manifold and body-storage
cleanup precede body-list unlinking and return to the static or dynamic free
list. Only the pending count is zeroed; queue allocation and capacity remain.

Profile reset recursively reloads child vector bounds and clears only the
accumulated sum and count at `38h/3Ch/40h`. Last-duration fields remain intact.

## Validation and limits

- Strict MSVC Win32 build and all three existing CTests pass.
- Independent object audit checks 990 native instructions, 53 branches and
  25 adapters, plus exact constants and strings.
- 576 copied-native/source pairs compare 64,257,888 bytes across 48 record
  configurations and 12 x87/MXCSR control combinations. Both sides execute
  5,904 substeps, 3,120 controlled solver batches, 1,560 report callbacks with
  6,456 point records, and 61,920 allocation/free events. Complete raw records,
  report bytes, allocation preimages and floating-point state match.
- The comparison controls velocity/collision/position, solver dispatch and
  virtual SAP/shape destruction boundaries. It uses actual group and body
  cleanup. The existing profile-enter boundary uses the copied helper on both
  sides with a deterministic clock; the production default is checked by the
  actual-owner lifecycle below.
- That separate lifecycle runs four simulations/four substeps through both
  solver modes, actual collision/report callbacks, previous-transform copying,
  and native pending removal of a static and a dynamic body. It produces six
  contact records and six listener callbacks, runs three real solver batches
  with eleven worker allocations overall, and completes full physics teardown.
  The fixture closes 101 native-unclosed handles; actual pool trim/process
  atexit leave zero tracked allocations.
- The source application launch was refused because another harness held the
  game slot. Combined-build and any later runtime evidence are recorded
  separately in the report.

These are explicit source interfaces with private context adapters, not certified
original ABI entries. The comparison uses finite inputs and masked FP exceptions.
FH3/unwind, allocation failure, hardware faults, concurrent mutation, arbitrary
malformed records and exhaustive IEEE behavior remain unproved. The game fixed-step
owner still needs to admit these raw world records; rendering and gameplay are
not established by the process fixture or an application startup smoke test.
