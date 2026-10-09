# Native World current-storage tick Source

The Source pair implements the complete ordinary-body schedules of
`00904BF0` and `00904600` against actual caller-supplied storage. The public
entries and four-reference `NativeWorldCurrentTickContext` are in
`include/bsp/native_world_current_tick.hpp`; their concrete bodies and narrow
calling adapters are in `src/native_world_current_tick.cpp`.

This packet starts from published main
`af15a550d63ee226f3b72ef5520f7a9e1454f493`. It reuses the complete native
evidence and corrected composition audit. All 28 outer and 323 inner native
instruction sites are marked in the Source, in their original order. The
1,576 native bytes were not recaptured or executed. Worker Original credit
remains zero pending integrator registration, build, complete emitted review
and canonical-ledger assessment.

## Explicit borrowed contract

```cpp
struct NativeWorldCurrentTickContext {
    const volatile float& mission_clock_00f876a4;
    const volatile float& negative_zero_00d7a208;
    const volatile float& one_00d7a24c;
    const SingletonLifetimeCallbacks& validation;
};
```

The public ordinary C++ entries take actual World, float delta and that
context. Both original entries instead take World in ECX and a float stack
argument, returning with RET 4; the matrix entry ignores its float. These
explicit Source APIs are not replacement binary entry points. Names are
descriptive hypotheses.

The caller qualifies and retains the actual common clock cell, its reset/step
authority, current constant storage, intended validator, World fields,
headers/sentinel/records, callable entity tables and later-read lifetimes.
The four references provide none of those owners or publications. In
particular no projected host clock, original numeric table, token entity,
private constant or sentinel value `1` is selected as a fallback.

The outer chain requires real active+5C/next+38 entities and slot+DC methods
taking ECX receiver plus one callee-popped float. The inner pass requires
slot+88 with one callee-popped matrix pointer and slot+D8 with no argument,
plus the raw child+48/sibling+44/+C8/+10C subtree contract. Slot results are
unused. Every matrix node released by `singleton_lifetime_free` belongs to
its matching Source CRT domain. The intended validation callback is nonnull,
may return and may change the fields subsequently reloaded. Its callback
context remains live. The World primary table is never read by these bodies.

## Calling composition and private scratch

Public wrappers obtain the four addresses through normal C++ reference
members into a local, standard-layout, pointer-only `TickBindings`. Compile
assertions describe this newly created local argument block; no layout of
World, entity, native record or reference-member representation is invented.
The delta is copied into a DWORD with `memcpy` before entering assembly, so
there is no additional C++ floating evaluation ahead of the explicit native
FLD/FSTP argument paths.

Two private fastcall bodies receive ECX World, EDX bindings and one DWORD
stack argument. The outer body additionally saves EBX for the binding pointer;
its original delta load moves from ESP+0C to ESP+10. Its unconditional tail
calls the same concrete matrix body used by the public matrix entry, passing
the same captured World and bindings after the native FLD/FSTP copy.

The matrix body retains the binding pointer with one extra PUSH outside the
original 2E8-byte scratch frame. After the four original register saves it
is at ESP+2F8. Every original scratch offset remains unchanged. This is an
explicit Source calling adaptation, not a native private-frame claim.

| Adapted call | Argument/stack contract |
| --- | --- |
| Returning validator | Push the borrowed `SingletonLifetimeCallbacks*`, call the CDECL adapter, then ADD ESP,4. The adapter invokes exactly its `invalid_parameter(context)`, without a null/no-op/throw-only fallback. |
| X/Y rotation | Preserve original ECX destination and EDX angle pointer; push current one and negative-zero addresses. The existing fastcall helpers consume their extra 8 bytes. |
| Z rotation | Same extra-operand fastcall contract. A narrow adapter begins a real trivial `CameraMatrix` in the aligned private scratch with placement default-initialization, calls the existing current-operand void overload, then returns the known destination. |
| Four multiplies | Preserve all eight original pushes and the existing `00413920` ECX-left / stack-destination-right / RET 8 chain. Incoming EDX is unused by that concrete helper. |
| Entity +88 / +D8 and subtree | Retain original direct table loads, indirect CALL instructions, receiver captures and callee cleanup; call the concrete `0042ED50` helper on actual children. |
| Node free | Keep the original PUSH node, call the existing CDECL `singleton_lifetime_free`, then ADD ESP,4 before the current count decrement. |

`CameraMatrix` is the existing 64-byte, four-byte-aligned
`std::array<float,16>`. The Z placement expression allocates no heap storage
and leaves the float values uninitialized before the real rotation provider
writes them. Compile assertions require trivial default construction and
destruction. There is no additional matrix copy, constructor policy for native
objects, or typed overlay of an entity/World. Repeated placement construction
reuses only this private scratch; no destructor work is required at return.

Every adapted call returns to the original scratch baseline. The matrix
epilogue removes its four saved registers, original scratch and extra binding
word before RET 4; the outer epilogue restores EDI, ESI and added EBX before
RET 4. Actual emitted cleanup and all adapter targets still require primary
build/object review.

## Retained numeric and mutation schedule

The complete native schedule is present, including the unreachable validator
after `CMP EDI,EDI`. The matrix body still has 29 call sites: 18 validators,
three rotations, four multiplies, two entity virtuals, subtree and free.
The private assembly contains 394 written instructions after adding explicit
bindings; the outer contains 32. These are Source counts, not emitted counts.

- World+4 is captured once. Active entities load their current table, FLD
  delta, select +DC, reserve/store the float and invoke on the actual receiver.
  The same entity+38 is reloaded afterward. The concrete matrix tail always
  runs, including for an empty chain.
- Each reached matrix record copies the current borrowed clock with MOVSS
  and spills it before validation. It then subtracts the current start through
  x87 and stores binary32 elapsed. That captured elapsed survives later
  callbacks. Phase divides by the current duration and spills to binary32.
  FCOMIP/JBE lower and COMISS/JBE upper branches retain unordered phase and
  signed zero. The current one operand is loaded at its original sites.
- Translation keeps the native x87 phase value through all three product
  spills and matrix stores. The failed-sentinel path explicitly pops it at
  `0090478A`, invokes the returning validator and reloads at `00904791`.
  X/Y/Z angles retain multiply, binary32 spill/reload, FCHS and final spill.
  The concrete helpers retain their own current-operand arithmetic. All three
  ordered sixteen-DWORD copies remain in the main body as MOVSS loads/stores.
- Entity capture `009046A1` survives subsequent validators. Table capture
  `00904A1E` remains inside the Z-result copy. Its +88 cell address is retained
  across the four multiplies; its function is loaded only at `00904ADD`.
  The final multiply reads the actual record.base, after `Rz * Ry * Rx * T`.
- After +88, validation precedes fresh record.entity/child-head reads and
  ordered +C8/+10C clears. Each actual child is invalidated recursively, then
  its current sibling is reloaded. +D8 reloads entity and its current table.
  Expiry reloads entity active state and current duration while retaining
  elapsed; only ordered strict JA expires an active record.
- Erase preserves node and successor captures across returning validators,
  current-sentinel rechecks, link reloads after the first unlink store, node
  free and the subsequent current DWORD-count decrement. Count wraps as a
  DWORD; there is no clamp, removal cap, entity deletion or early-success path.

No catch, `noexcept`, rollback, node repair, cycle guard, fake table/global,
projected callback, unresolved callee stub or application binding was added.
Completed native-field writes remain on a throwing Source call. General
fault/SEH/private-frame equivalence, concurrent mutation, full World ownership,
caller provenance, native binary entry execution and gameplay remain outside
this qualified consumer implementation.

## Worker validation and next admission boundary

Static review matches all 351 marked native sites to the cached complete
listing, allowing only declared call/global/argument relocations, branch labels
and equivalent assembler spelling. The complete branch labels, 29/2 call
inventories, numeric groups, captures and stack adaptations were inspected.
This is Source transcription evidence, not compilation or execution evidence.

JSON, cached native-byte/listing integrity, canonical baseline and owned-file
hashes, and `git diff --check` pass. Baseline inputs are pinned to published
`af15a550d` Git bytes, with differing physical line endings recorded rather
than silently pinning a worker-only CRLF copy. New files use LF.

No CMake, config, ledger or Ghidra changes occurred; no build, new tests or
execution probes ran. The integrator must register the pair, run the normal
Win32 build, inspect both complete emitted bodies and every added adapter
target/argument cleanup, and check the canonical address records before any
Original-function admission. This packet adds no Original credit.
