# Current shutdown-step service `00874D00`

This read-only audit captures the complete **216-byte / 59-instruction** body
at `00874D00..00874DD7`: **nine CALL instructions and one tail JMP**, reaching
ten direct callees. Four live byte windows, **243 bytes** total, match the
installed PE. Existing `bsp.gpr` and `/battlestationspacific.exe` were verified
by the read-only CLI. No Source implementation or Original-ABI credit is added.

The current host's cached `last_world_active_` is not this body's gate.
Furthermore, the World destructor's preceding `World+4AC=0` does **not** prove
that the gate remains closed: queued Lua and think callbacks run before a new
Game/World lookup, and may change either publication or the current ready byte.
The service remains unready as an actual World shutdown composition.

## Input, schedule and current identities

The only established input is the low byte **CL**, retained in BL. Zero runs
the full extra step; every nonzero byte skips think and the final three rows.
There is no stacked input. EBX is restored. Initial `PUSH ECX` reserves local
scratch, later overwritten by float arguments on the gated path; it is not a
general promise to preserve ECX. Nonzero mode ends with `POP ECX; RET`; zero
mode removes that scratch and tail-jumps to `903610`. No semantic return value,
native exception wrapper, rollback or added synchronization is established.

| Native sites | Ordered operation |
| --- | --- |
| `874D01..874D0F` | Read current shared Game cell `E188A8` as Game A, load `[A+1A08]`, and call queued Lua drain `888230` with that actual host in ECX. There is no initial Game or Lua-host null guard. |
| `874D14..874D22` | If mode is zero, freshly `FLD [D0DE84]`, reserve a stack DWORD, `FSTP` it and call `929460`. Its stacked float is callee-popped. |
| `874D27..874D41` | Read `E188A8` again as Game B. Null B skips the middle block. Otherwise read `[B+19CC]` as the current World and test its actual byte `+4AC`; there is no World-null guard. |
| `874D43..874D68` | If the gate opened, select positive zero for nonzero mode or freshly read float `[D0DE84]` for zero mode. Preserve MOVSS local spill followed by FLD/FSTP stack forwarding. Call `778450` on **B+1EF0** with that float. |
| `874D6D..874D79` | Call `77EC20`, call `874C90`, then set only CL=0 and call `925F20`. All occur once the earlier gate opened; the gate is not rechecked between them. |
| `874D7E..874DAF` | Recompute the float independently after those callbacks. `TEST BL; SETZ DL` supplies the mode low byte. Push the current whole EDX as the second argument and the x87-spilled float as the first. Read `E188A8` a third time as Game C and call `76FFC0` on **C+1EF0**. There is no null or ready recheck here. |
| `874DB4..874DBE` | Restore EBX. Nonzero mode returns. Zero mode calls `926700`, then `9273A0`, regardless of whether the middle gate opened. |
| `874DC3..874DD1` | For zero mode read `E188A8` a fourth time as Game D, load current `[D+19CC]`, remove local scratch and tail-jump to `903610` on that current World. There is no Game/World null guard and no requirement encoded here that this is the destructor's original World. |

The fixed-step operand is raw float `CD CC 4C 3D` (`0.05000000074505806`).
Its three possible loads are separated by callbacks; Source must not replace
them with one earlier snapshot. The x87 load/store paths and their floating
environment effects are part of the evidence, independently of the numerical
value. This routine does not itself read or write mission-clock `F876A4`.

The outbound mode requires an explicit qualification. `SETZ DL` defines only
the low eight bits; the upper24 bits are the **current EDX contents after prior
callbacks**. The direct callee reads the whole stacked DWORD at `00770034` and
forwards it at `0077003C` to `76F210`; its ordinary return is `RET8` at
`007700DC`. The lower helper was not expanded. Treating that argument as a
canonical full-width integer0/1 requires a separately established consumption
contract; the current host's `flag ? 0 : 1` does not prove it.

## Relation to the World destructor

The committed [destructor audit](CC12_WORLD_DESTRUCTOR_AFTER_CURRENT_TICK_READINESS.md)
at `18203a7f5` pins the five CL=0 calls at `904C96`, `904CCA`, `904D05`,
`904D40`, and `904D7D`, after clearing its own ready byte at `904C67`.
If the newly loaded Game B still names that same World and the earlier
callbacks have not changed its byte, the middle block is skipped. Even then,
queued Lua, think, deferred events, pending entities and current-World expiry
still run. Neither a no-op shutdown step nor an unconditional closed-gate
specialization matches the general body. Subsequent calls repeat the complete
lookup and callback schedule.

## Actual Source providers and remaining contracts

| Boundary | Current evidence and qualification |
| --- | --- |
| Shared Game publication | `GameNativeSettingsProcess::game_00e188a8()` returns a real process-lifetime volatile pointer cell. The settings profile context borrows the same member. `NativeGameConstructionContext::actual_game_00e188a8` is published by the actual constructor; `NativeGameLifetimeContext::game_00e188a8` is cleared by lifetime code. These are reusable mechanisms **if all contexts bind that same cell**. This audit found no production caller of the public accessor beyond its definition, and does not establish a live Game/Lua/World/session graph. Borrow the actual common cell; do not manufacture a second publication or cache its value for the whole step. |
| `888230`, queued Lua | `drain_named_call_queue` operates `MissionLuaCallQueue::entries`, a C++ vector of copied calls. `GameStepSubsystemsHost` owns a private `lua_queue_`; its `NamedCallBinding` contains diagnostic Lua operations. It is not a raw `[current Game+1A08]+8` queue drain with its actual producer, node/call destruction and Lua runtime. |
| `929460`, think | `run_entity_think_list_00929460` is explicitly a projected vector/list interface. The production step host supplies private countdown/live/pending fields and an empty entity-field vector; callbacks include diagnostic think/GC operations. Actual shared list identities, entity fields, Lua dispatch, countdown and GC gate remain required. |
| `778450`, session pump | Existing `GameFixedStepHost` uses a private countdown and projected command queues, and logs the unimplemented current global-object virtual step. This does not qualify arbitrary actual embedded session `B+1EF0`. Real lower transport components do not establish the complete parent method. |
| `77EC20`, deferred creates | The host records the empty-global-list case without an actual pending-create owner/drain. A real current owner, record lookup/create actions and cleanup are still needed if the gate opens. |
| `874C90`, registration splice | The host merely records the assumed empty pending-list case. Live metadata identifies a **111-byte, 31-instruction, zero-callee** Native body. Its actual shared list/head/tail/count identities and splice/reset order are not implemented by that host. This is a bounded independent read-only successor candidate. |
| `925F20`, InitAll | `sentity_init_all_00925f20` supplies the reconstructed algorithm through `SEntityInitAllHost`; `GameMissionLuaHost` binds its own pending entity collection and host operations. This does not establish a generic actual pending-list/entity/profile/pool owner for the current Game. The exact caller passes low CL=0, with no null-runner skip. |
| `76FFC0`, outbound | The current fixed-step host discards both arguments and records a single-player empty-send case. A real actual current session, its current mode/state/transport work and the full-width mode forwarding contract remain external. Lower native transport helpers are not the complete `76FFC0` service. |
| `926700`, deferred events | The abstract algorithm exists, but `DeferredEventBinding` returns size0 and only logs the pending-flag write. It supplies neither the actual `F899C8` queue nor dispatch, node/record cleanup or the actual `E18684` byte mutation. |
| `9273A0`, pending drain | **A real actual-storage body exists:** `drain_native_pending_entities_009273a0`. The process `game_pending_entity_owners()` supplies the real destroy/kill owners, with explicit startup and exit wrappers. Reuse still requires complete copy-list `926FA0`, the same canonical unit/observer aliases and live byte lvalues, actual selected slot74, observer delivery, controlled-listener `E188DC`/renderer operations and current lifecycle virtuals. No production `NativePendingEntityDrainAccess` subclass was found. The older host's projected ship/death path does not satisfy those contracts. |
| `903610`, current-World expiry | The host algorithm exists, but `WorldExpiryBinding` uses index tokens, private counters, no children, and a diagnostic deleting call followed by setting counter -1. It is not actual `[World+4]` storage or entity lifetime. The real current-slot scalar dispatcher is reusable mechanics; the actual subtree/destructor unlink, child-count updates, allocation and resume-anchor lifetimes still need qualification. |

Usable current-storage pieces therefore include the real process publication
cell, actual pending-list owners/initializers, the actual pending-drain loop,
and existing current-table lifetime dispatch. None constitutes a complete
`874D00` production binding. Ordinary callback failures must propagate with
their preceding effects; this audit proposes no fabricated empty queues,
diagnostic-success callbacks, null fallback or speculative cleanup wrapper.

## Next bounded work and evidence limits

The smallest independently scoped direct leaf is a **read-only contract audit
of `00874C90..00874CFE` (111 bytes)**. Its current metadata reports31
instructions and zero direct callees; the concrete host gap is an assumed
empty pending registry. Establish actual shared owner identities, complete
pointer/count reset schedule and producer compatibility before proposing
Source. Its body was not expanded here, and metadata is not a whole-body
implementation proof. It can advance independently of the deeper Lua, entity
lifecycle and session services; it does not close shutdown by itself.

The [report](../reports/cc12_world_current_shutdown_step_readiness.json) retains
all59 body instruction rows, exact raw windows/hashes, ten direct boundaries,
four publication reads, Source pins and the inherited destructor evidence.
Direct-callee footprint descriptions are existing ledger boundaries; only the
explicit outbound argument/return windows were freshly byte-checked outside
the target body. No whole-callee sweep, Source/CMake/ledger/Ghidra write,
flow retry, build, test, probe, runtime run or application admission occurred.
