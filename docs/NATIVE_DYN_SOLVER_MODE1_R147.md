# Native mode-1 solver task and profiler boundaries

Addresses: 00403850, 00C35160, 00C350C0, 00C37C40, 00C42ED0,
00C431D0, 00C437D0, 00C4F140, 00C5C710, 00C5C7A0.

Packet `orch4_dyn_solver_mode1_r147`, 2026-09-18. Source:
`include/bsp/native_dyn_solver_mode1.hpp`, `src/native_dyn_solver_mode1.cpp`.
Evidence: `reports/native_dyn_solver_mode1_r147.json`.

## Result

The second solver has a complete normal task chain over native records and a
callable one-slot table corresponding to `D7A090 -> 00403850`. Alongside R146,
both solver tables required by the world constructor now have source owners.
This packet preserves the second solver's distinct per-manifold storage and
profiling behavior. It does not route mode 1 through the mode-0 implementation.

| Entry and inclusive end | Bytes | Original inputs and return |
| --- | ---: | --- |
| 00403850..00403922 | 211 | ECX task; RET |
| 00C35160..00C35253 | 244 | ESI context; RET |
| 00C350C0..00C35143 | 132 | stack context; RET4 |
| 00C37C40..00C37D28 | 233 | EDX context; RET |
| 00C42ED0..00C431C2 | 755 | EBX context; RET |
| 00C431D0..00C437CA | 1,531 | stack context; RET4 |
| 00C437D0..00C43A91 | 706 | ECX context; RET |
| 00C4F140..00C5030F | 4,560 | stack context/float dt; RET8 |
| 00C5C710..00C5C79F | 144 | EAX context; RET |
| 00C5C7A0..00C5C932 | 403 | EDI context, stack group/float dt; RET8 |

The ten new bodies total 8,919 bytes. The existing abs/sqrt boundaries and
original stack-probe reference add 102 bytes. Table, profiler strings, numeric
constants and profile publication cell bring live/PE verification to 9,076
bytes in 20 spans. The source consumes the platform `__chkstk` and previously
implemented CRT math and profile-child append dependencies.

## Storage and instruction contracts

Task storage is the same 18h producer layout as mode 0: world at 8, inclusive
group range at Ch/10h, dt at 14h. Its private solver context is EAC8h bytes.
The world, body list, group, velocity pointer/capacity and row block/capacity
retain their established leading offsets. Mode 1 uses a different array map:

| Context field | Data |
| --- | --- |
| EA7Ch / EA98h | normal / friction signed16 body-index pairs |
| EA80h / EA84h | normal Jacobians / inverse-mass products, C0h per manifold |
| EA88h / EAA4h | normal / friction point counts per manifold |
| EA8Ch / EAA8h | normal / friction effective masses, 10h per manifold |
| EA90h / EA94h | normal velocity / bias right-hand sides, 10h per manifold |
| EA9Ch / EAA0h | friction Jacobians / inverse-mass products, C0h per manifold |
| EAACh | friction velocity right-hand sides, 10h per manifold |
| EAB0h | friction coefficient, four bytes per manifold |
| EAB4h / EAB8h / EABCh | normal / bias / friction impulses, 10h per manifold |
| EAC0h / EAC4h | velocity body count / manifold count |

Allocation rounds the manifold count up to a multiple of four, then multiplies
by 394h. EA78h is a **byte capacity** here. Array boundaries are recomputed
from the requested rounded count even when capacity is retained. Each manifold
has space for four contacts. The velocity block has 30h per body and clears its
full capacity before warm start. The builder processes manifold body indices
even for zero-point manifolds; static bodies receive index 0. Preserve its
native static-body scratch pointer store at the current next-body slot.

The solve wrapper alternates normal and friction passes for world+38h
iterations, writes velocity changes to native motion records, resets dynamic
stamps, then writes normal/bias impulses into contact records. The complete
x87/SSE operation, spill, branch and signed-index schedule is retained.

## Profiling is part of the task

Both wrappers reload the actual profile publication cell corresponding to
0109E9F8. They use cache slots 70h/74h (ids 19h/1Ah), append a real child of the
current profile node on a cache miss, set its active parent, and publish it as
current. Names are the PE-verified `SolverPreStep` and `SolveConstraints` bytes.
The existing complete C50390 child append handles the real node, string and
child vector; it is an explicitly shared dependency in the comparison fixture.

All four RDTSC boundaries remain explicit. The default call service executes
actual unserialized RDTSC; the instruction audit verifies that body. A borrowed
timestamp provider permits repeatable comparison while the instruction kernels
retain all timestamp subtraction, ADD/ADC carry, last/sum/count and current-node
pop operations. Adapters preserve ECX and flags across the timestamp service.
The six profile loads use only MOV instructions, preserving the carry needed
by the following ADC. Timestamp overrides must retain floating-point state.

## Normal-return boundary and Ghidra repair

The source retains original task stack alignment and all native local offsets.
It stores borrowed context in the task's otherwise-unused stack padding word
originally filled by PUSH ECX at 0040386B. Other context-bearing private kernels
receive an extra final argument, with explicit argument-order/RET adapters.

Two original EH-handler words become zero placeholders and four FS registration
writes are omitted. The copied-native fixture makes those same EH adaptations.
The result covers valid records, successful allocation and normal return, not
original FH3/SEH cleanup, allocation failure or a drop-in exception ABI.

Four false CALL_RETURN overrides were repaired under the Ghidra write lock:
004038F1, 00403905, 00C3519E and 00C5C83D. Each hid its returning ADD ESP,4.
All four three-byte fall-through gaps are now restored. Prior values, repair
receipts, saved annotations and refreshed exports are preserved. Unreachable
alignment gaps in friction and row building remain alignment, not fake calls.

## Validation

- Strict MSVC Win32 build and all three existing CTests passed.
- COFF audit checked 2,518 original instructions (ten bodies plus float abs),
  62 branch targets, three numeric constants, two strings, six profile loads,
  14 adapters and the default RDTSC body. A separate decoded CFG simulation
  checked every borrowed-context offset and original callee cleanup.
- 24,576 copied-native/source pairs matched **379,431,552 bytes**: 256 scenarios,
  12 masked x87 controls, four independent MXCSR rounding modes, and explicit
  versus actual source-table invocation. All ten original body returns ran.
- Compared 150,528 service events per side, including 33,408 shared profile
  allocator events and 117,120 solver allocator/free events. All six native
  solver allocation sites ran, including the repaired growth/free paths.
- Compared full task/world/group/body/motion/manifold/profile/node records,
  all arena allocations including retained row capacity, allocation order,
  controlled timestamp observations, FP control/status/tag and MXCSR. Guards
  remained intact. The task table and profile heap-string pointer are normalized;
  the live heap name bytes including NUL are compared. Unspecified heap-string
  capacity padding, private stacks and FP instruction pointers are excluded.
- Profile fixtures cover cold and prepopulated caches, repeated groups, a nested
  current parent, 64-bit timestamp deltas and sum overflow, and count wraparound.
  Actual profile/body/contact producers are shared; world/group links and
  consumed world-inertia fields are supplied. Full world construction and
  velocity integration are not executed by this fixture.
- Geometry cases retain empty ranges/groups, zero through four points,
  static/dynamic bodies, row and velocity growth/shrink/repartition, iterations
  0/1/2/10, warm starts, restitution, tangent fallback/sliding and bias clamps.

## Remaining work

Complete world/application ownership and consumers, native exception paths,
malformed/overflow input handling, unmasked traps, concurrent profiling/tasks,
ordinary application execution and gameplay validation remain open. Both
solver tables are now available dependencies, not proof that the game has run.
