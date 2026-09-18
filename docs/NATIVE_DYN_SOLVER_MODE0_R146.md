# Native default solver task and complete row pipeline

Addresses: 00403720, 00C31C30, 00C35020, 00C37B50, 00C42230,
00C42530, 00C42BA0, 00C4DE40, 00C4F040.

Packet `orch4_dyn_solver_mode0_r146`, 2026-09-18. Source:
`include/bsp/native_dyn_solver_mode0.hpp`, `src/native_dyn_solver_mode0.cpp`.
Evidence: `reports/native_dyn_solver_mode0_r146.json`.

## Result and scope

The mode-0 solver now has a complete normal task chain over the actual native
records, plus a callable one-slot source table corresponding to
`00D7A088 -> 00403720`. The existing semantic `DynConstraintBatch` API remains
separate. This implementation does not project native parallel arrays into it.

| Entry and inclusive end | Bytes | Original inputs and return |
| --- | ---: | --- |
| 00403720..0040381D | 254 | ECX task; RET |
| 00C31C30..00C31CBF | 144 | ESI context, EDI requested rows; RET |
| 00C35020..00C350A2 | 131 | stack context; RET4 |
| 00C37B50..00C37C38 | 233 | EDX context, retained for next call; RET |
| 00C42230..00C42525 | 758 | ECX context; RET |
| 00C42530..00C42B6E | 1,599 | ECX context, retained for friction call; RET |
| 00C42BA0..00C42EC3 | 804 | EAX context; RET |
| 00C4DE40..00C4F03C | 4,605 | ESI context, stack float dt; RET4 |
| 00C4F040..00C4F13E | 255 | ECX context, EAX group, stack float dt; RET4 |

These nine bodies total 8,783 bytes. References comprise the existing 28-byte
float-abs boundary, 31-byte float-sqrt boundary, and 43-byte original CRT stack
probe. The source consumes the platform `__chkstk`; it does not copy the CRT
stack-probe implementation. With the table and three numeric constants,
the collector verified 8,905 live/PE bytes in 16 spans.

## Native storage and execution

The task is 18h bytes, as produced by `dyn_world_runtime.cpp`: table at 0,
scheduler word at 4, world at 8, inclusive first/last group at Ch/10h, float dt
at 14h. A group is `{manifold** array, int count, int capacity}`, stride Ch,
under world+44Ch. World+38h supplies the iteration count.

The task allocates an EAAC-byte private context on its stack. Its first fields
are world, body count, and 15,000 body-pointer slots. Group at EA68h precedes
velocity pointer/capacity at EA6Ch/EA70h, row allocation/capacity at EA74h/EA78h,
the nine parallel-array pointers at EA7Ch..EA9Ch, velocity count at EAA0h,
normal row count at EAA4h and friction base at EAA8h.

Row storage requests eight rows per manifold, 7Ch bytes per requested row.
Every call recomputes array boundaries from the **requested** count, including
when an existing allocation has a larger capacity. Each row has a 30h Jacobian,
a 30h mass-weighted Jacobian, signed16 body indices, and seven total four-byte
lanes including that index pair. The velocity buffer uses 30h per body and clears
its entire capacity each prestep.

The builder assigns all static bodies index 0; dynamic body+58h stamps and
body+5Ch indices deduplicate bodies within a group. It builds the normal and
sliding-friction rows, preserves native x87 spills and sqrt boundaries, and
loads prior contact impulses. Warm start precedes alternating normal/friction
iterations. Write-back starts at body index 1, adds real and bias velocity
changes to native motion records, and resets each dynamic stamp to FFFFFFFFh.
The final pass stores normal and bias impulses back into manifold contacts.

## Call and exception boundaries

The source retains register inputs and original stack offsets in private
instruction kernels. Borrowed allocator/call/CRT context is an extra final
argument only where needed. Small adapters duplicate dt at the original offset
and preserve the native FLD/FSTP schedule. Service calls retain their six native
CALL-site identities and the distinct row/velocity free functions. The runtime
owns no shared progress state and introduces no TLS. Its table and borrowed
pointees must remain alive and stable throughout every call.

The original task installs an FH3/SEH record. The source preserves its normal
stack geometry but replaces the handler word with zero and omits the two FS
registration writes. The fixture makes precisely the same exception-related
adaptations to the original task. It consequently proves **normal-return
behavior only**, with successful allocation and valid records; it does not
prove original unwinding, exception cleanup, OOM behavior or a drop-in ABI.

Correction to the earlier context description: `[ESP+EAC0h]` at 00403769 is
the native EH state at entry-ESP minus four, outside the EAAC-byte context.
It is not a context member at EAB4h.

## Ghidra repair

The returning free at 00C4F081 had a false CALL_RETURN flow override, hiding
`83 C4 04` at 00C4F086. The locked repair removed that override and restored
ADD ESP,4; the complete prestep now has 70 listed instructions and no gaps.
Prior values, exact bytes, saved project, readback and refreshed exports are
recorded. The unreachable alignment bytes at C42278..C4227F are deliberately
left as an alignment gap in Ghidra and retained in the linear source audit.

## Validation

- Strict MSVC Win32 build and all three existing CTests passed.
- Independent COFF audit checked 2,451 original instructions (nine bodies plus
  float abs), 52 branch destinations, three constants and nine adapters.
  Stack offsets were independently simulated over decoded control flow,
  including every join, callee cleanup and the EAAC-byte stack reservation.
- 6,144 copied-native/source task pairs matched 78,497,664 bytes and 31,488
  native service events. Both the explicit interface and real source task table
  ran under all 12 combinations of masked x87 precision/rounding controls.
- Cases include empty task ranges, empty groups, zero through four points,
  static/dynamic bodies, group growth/shrink, row repartition within capacity,
  velocity reallocation, iterations 0/1/2/10, warm starts, restitution, tangent
  fallback/sliding directions, and negative/zero/interior/clamped bias scalars.
- All nine original return sites and all six allocator CALL sites executed.
  In particular, the repaired velocity-growth free at C4F081 ran 3,288 times
  on each side. Allocated-block guards remained intact.
- Compared full body/motion/manifold/world/task storage, every allocated row
  and velocity block including unused capacity, allocation/free order, and
  x87 control/status/tag plus MXCSR. The task table pointer is normalized;
  private context stack addresses and FP instruction pointers are excluded.
- Fixture inputs use the existing real body initializer and contact inserter.
  World/group links and consumed world-inertia fields are supplied explicitly;
  the fixture does not execute world construction or velocity integration.

## Remaining work

The mode-1 solver table, full world/application ownership and dependencies,
native exception behavior, malformed/overflow input behavior, unmasked traps,
concurrency and ordinary game/gameplay validation remain open. This packet
supplies a callable default solver dependency; it does not claim the rebuilt
game has reached this path.
