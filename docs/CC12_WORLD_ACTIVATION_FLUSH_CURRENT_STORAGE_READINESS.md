# Current-storage World activation flush: 00903670

The complete native caller is **57 bytes / 22 instructions**, below the requested
300-byte limit. Its sole direct service is `00922FD0`, now implemented as the
concrete actual-storage `mark_native_entity_destroy_state_00922fd0`. That closes
the direct Source service prerequisite for a bounded, caller-qualified raw flush.
The current projected flush does not implement its actual-pointer walk.

This is a read-only readiness audit. It implements no Source and adds no Original
function/byte, ABI or game-validation credit. The canonical address already has
a counted `flush_entity_activations_00903670` reconstruction; future refinement
must preserve/reconcile that record instead of adding another function.

## Evidence boundary

- Current published and worker baseline:
  `042826a42c4ec424b79a829e5c9b8194cc830a77`.
- Read-only Ghidra CLI checks verified `C:/Users/sqz269/bsp.gpr`, program
  `/battlestationspacific.exe`, `x86:LE:32:default`, image base `00400000`.
- Metadata established `00903670..009036A8` before the whole-body read. Cached
  pseudocode/listing were checked against all live bytes and a complete decode.
- Complete body SHA-256:
  `1711b2cb01a1341d6bbecc164337978395150304375ea23838060ec319e769b1`.
  All 57 live bytes equal the installed PE span; the PE SHA-256 is
  `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
- The companion report embeds every native byte/instruction, branch/call targets,
  current canonical Source pins and the accepted concrete callee proof.
  No other native body or forbidden address was queried in this packet.

## Receiver, storage and ABI

`ECX` supplies the actual World-like receiver. The first instruction reads its
`+4` pointer, then the function saves `ESI` and reads the captured header's first
pointer at `+0`. Both pointer loads happen exactly once. An empty first pointer
returns normally; a null header is not an empty-list representation and is still
dereferenced. Neither a World vtable nor header last/count fields are read.

The only explicit save/restore is `ESI`; there are no explicit stack arguments,
incoming `EDX` inputs, floating-point instructions, globals or direct allocation/
free operations. `0090367A` is the six-byte no-op `LEA EBX,[EBX+00000000]`
(`8D 9B 00 00 00 00`), which preserves EBX and flags. The body ends with `POP ESI`
and plain `RET`. It has no meaningful typed return value established here.

The sole call at `0090369B` loads `ECX` with the current actual entity and calls
`00922FD0` without stack arguments. Its terminal virtual method must return with
the compatible stack and callee-saved-register behavior, including the caller's
current `ESI`, for the flush to continue. The concrete helper already expresses
that qualified contract; this audit invents no virtual target implementation.

| Storage | Exact read schedule |
| --- | --- |
| receiver `+4` | Capture header before `PUSH ESI`; never reload. |
| captured header `+0` | Capture first entity once. |
| current entity `+5E` | Byte comparison with zero; zero skips all remaining predicates. |
| current entity `+6C` | Full DWORD comparison with zero, only when `+5E != 0`; every nonzero pattern skips the parent reads/call. |
| current entity `+3C` | Capture parent only after both earlier predicates pass. |
| captured parent `+5E` | Read one byte only for nonnull parent; nonzero skips the call. |
| current entity `+38` | Reload after the entire call and its terminal virtual method return, or after skipping that entity. |

There is no local entity-field store. The callee performs the reviewed flag,
DWORD mark, recursive hierarchy and current virtual-dispatch operations. The
existing `NativeWorldChainHeader::first` member agrees with the header footprint;
this body neither allocates the header nor establishes its remaining layout or
the full receiver/entity sizes.

## Current predicates, callback effects and lifetime

A reached entity is called exactly when byte `+5E != 0`, DWORD `+6C == 0`, and
either parent `+3C` is null or that parent's current byte `+5E == 0`. The native
code applies those predicates in that order. The parent need not be a member of
the same World chain; there is no vector bound, parent-index validation or added
active-byte gate.

After `00922FD0` returns, `009036A0` reads the same entity's current `+38`. A
callback can change that successor and the new link is followed. Changes to
World `+4` or header first are not re-read. Later entities' marks and parent flags
are read only when those entities are reached, so recursive callee mutations can
make a later entity fail the full-word `+6C == 0` gate. No successor snapshot,
head restart, index loop, count bound or visited set is added.

The receiver and captured header must permit their initial loads. Each current
entity must remain valid through its predicates and its post-call `+38` read;
a nonnull parent must remain readable through its single `+5E` read. The callee's
own current-child `+44` and callable `+84` lifetime requirements also apply.
No later World/header read occurs in this body, but external callers may impose
additional lifetimes. A cycle in the `+38` chain can loop even after every mark
is nonzero. Invalid storage can fault; callee effects already performed remain.
There is no local exception handler, recovery, rollback or new `noexcept` policy.

## Current Source dependency closure

The callee header/source hashes match the published primary review. That review
records a normal Win32 build and three existing checks, a unique concrete core
definition, the resolved self-call and whole 66-byte equality with the installed
PE. `CMakeLists.txt` currently registers the callee in `bsp_core`. These are
accepted prior build/artifact results, not checks rerun by this worker.

The existing `flush_entity_activations_00903670(WorldTickState&, WorldTickHost&)`
loops through a vector of `ActivationNode` values. Its `+6C` projection is Boolean
and its parent is a bounded vector index. The current host callback only records
an event. The alternate `flush_unit_kills_00903670` loops unit slots, assumes no
parent/children, writes projected flags plus a separate vector counter, and logs
the `+84` event. Its null/identity guards, unit index order and bookkeeping are
not the actual native walk. Neither path calls the new concrete raw helper.

The minimum future Source packet is `native_world_activation_flush.hpp/.cpp`,
with a guarded provisional
`void __fastcall flush_native_world_activations_00903670(void* actual_world)`.
A narrow naked 22-instruction body can preserve the complete schedule, retain
the six-byte LEA encoding explicitly, and bind its only CALL directly to
`mark_native_entity_destroy_state_00922fd0`. No new context, World/entity overlay,
global, allocation, callback substitute, owner or fake table is needed.

The direct dependency is therefore concrete. Actual World/header/entity/parent
provenance, callable current virtual methods, mutation/termination guarantees,
application binding and full teardown remain caller/production obligations.
Those limits must stay explicit in the future helper. Registration, build and
whole emitted instruction/relocation review would belong to the integrator.
This audit changes only its two evidence files; no Source, CMake, ledger, GPR,
build, tests, probes or native execution were changed or run.
