# Current-storage entity destroy-state readiness: 00922FD0

This read-only packet establishes the complete 66-byte body at `00922FD0` and
compares its actual-pointer contract with current Source. The routine is suitable
for a small, caller-qualified raw helper. No such helper is implemented here.
Production entity construction, callable current tables, hierarchy ownership and
the complete World destructor remain separate obligations.

The existing canonical reconstruction already credits `scene_node_kill_00922fd0`
in `src/hit_narrowphase.cpp`. This audit adds **zero** Original functions, Source
functions, ABI-compatible functions or game-validation credit. Refining this
address later must reconcile that record instead of adding a second function.

## Evidence and scope

- Baseline: `21590800be946bc1c807b9b3bfaee029831ca147`, which merges published
  `12adefd94008aa41695a7a1a3afd714cab0b1773` while retaining the World tick Source.
- Read-only Ghidra queries verified `C:/Users/sqz269/bsp.gpr`, program
  `/battlestationspacific.exe`, language `x86:LE:32:default`, image base `00400000`.
- Native interval: `00922FD0..00923011`, 66 contiguous bytes / 25 instructions.
  SHA-256: `29eceff51de711a90ebe3201b0f270a6cdc3febb62b1532857b5fba6301d382d`.
- The complete live bytes equal the installed PE bytes. The PE SHA-256 is
  `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
- The companion JSON embeds every byte and decoded instruction, canonical Git
  input hashes, working-file normalization provenance and the existing ledger
  records. Current code searches found no actual-storage helper for this body.
- Only this address was queried by this packet. The `00904C40` caller contract
  comes from the peer's committed audit `18203a7f5be5c0882af2c09404acb91d5378629b`;
  this worker did not independently query that owned address or its callees.

## Actual call and storage contract

The receiver arrives in `ECX`; no incoming `EDX` value or explicit stack argument
is read. The body saves `ESI` and `EDI`, makes one recursive direct-call site, then
restores those registers and **tail-jumps** through the receiver's current table
slot `+84h`. There is no local `RET`, synthetic post-call action, allocator, free,
global read, floating-point instruction or external direct helper.

The final dispatch has `ECX = receiver`, `EAX = current table`, and `EDX = current
slot target`. Its jump uses the original caller's return address. A recursive
invocation's target consequently returns to the parent's `00922FFD` sibling
reload. No explicit stack arguments are supplied to the target. The target's
complete ABI and incidental return values are not established by this body; no
typed return value or concrete virtual implementation is invented here.

| Actual storage | Access established here |
| --- | --- |
| receiver `+48h` | Capture the first child once, before mutation. |
| receiver `+5Eh`, `+5Dh`, `+5Ch` | Byte stores `1`, `1`, `0`, in that order. |
| receiver `+6Ch` | Full DWORD store `1` on every invocation. |
| current child `+6Ch` | Full DWORD comparison with zero before recursion. |
| current child `+44h` | Reload next sibling after the recursive body and its terminal virtual method return, or after skipping the child. |
| receiver `+0`, current table `+84h` | Late loads after every child has been processed. |

These access footprints do not establish a complete object size, C++ layout or
allocation domain. The native routine does not touch byte `+5Fh`.

## Ordered behavior and callback visibility

1. `00922FD4` captures the first child; `00922FD7` tests it. MOV instructions then
   write `+5E = 1` at `00922FDE`, `+5D = 1` at `00922FE1`, `+5C = 0` at
   `00922FE4`, and DWORD `+6C = 1` at `00922FE8`. Those MOVs preserve the initial
   TEST flags consumed by `00922FEB`. Leaf and already-marked receivers still
   receive all four stores and their terminal virtual dispatch.
2. For each captured child, `00922FF0` tests its current DWORD `+6C`. Exactly zero
   enters the same routine recursively. Every nonzero value skips recursion;
   this is not a byte or Boolean test and does not depend on the child's active
   or `+5E` bytes.
3. `00922FFD` reads the same child's current `+44` after recursion returns. A
   child's terminal virtual method can change that sibling link and the new
   value is followed. There is no saved successor, first-child reload, vector
   index, container snapshot or visited set.
4. After the walk, `00923004` reloads the receiver's current table, `00923006`
   loads its current slot `+84`, and `00923010` tail-jumps. Child callbacks can
   therefore replace an ancestor's table or slot before that late dispatch.

Writing the receiver's `+6C` first can suppress recursive backedges to a marked
ancestor. It does not guarantee termination: sibling cycles can still loop and
callbacks can clear a mark. No cycle guard or alternate traversal is justified.

The existing name-ledger prose places the `+5D` write at `00922FE4`; the captured
bytes place it at `00922FE1`, with `+5C` at `00922FE4`. The read-only packet records
this discrepancy without modifying the ledger or Ghidra annotations.

## Lifetime and failure obligations

A caller must supply actual writable receiver/child storage with current readable
links and a callable current `+84` target that accepts the no-stack-argument
receiver dispatch and returns compatibly when traversal is to continue. The
receiver must remain valid through the late table fetch. Each current child must
remain readable through the parent's post-return `+44` reload. The routine itself
performs no receiver read after its final jump, but that does not grant a terminal
method permission to invalidate storage its caller still needs.

The independently owned World destructor audit establishes a call at `00904CBC`
(`E8 0F E3 01 00`) immediately followed by `00904CC1: MOV EDI,[EDI+38h]`
(`8B 7F 38`). That caller therefore additionally needs the same entity readable
after this routine and its final virtual method return. Its gate is current byte
`+5E != 0`, DWORD `+6C == 0`, and absent parent `+3C` or parent byte `+5E == 0`.
That gate belongs to the caller; it must not be added to the raw leaf.

Null receiver storage faults at the initial `+48` load. Failure during later
accesses, recursion or the virtual target leaves prior flag writes and child
effects in place. There is no catch, rollback, null-success policy, exception
translation or new `noexcept` guarantee in the recovered body. Native fault,
unwind and binary-call compatibility remain untested.

## Current Source and the minimum next step

`scene_node_kill_00922fd0(SceneNodeFlags&) noexcept` writes only three projected
Boolean fields. `SceneNodeFlags` has no `+6C` DWORD, hierarchy links or actual
table. The host's `flush_unit_kills_00903670` separately marks an indexed vector counter
and records `Entities::kill_vtable84`; it does not invoke a current entity method
or traverse native children. `GameStepSubsystemsHost::mark_killed_006c` can resize
its vector and writes an indexed element. These behaviors cannot be reinterpreted
as the native field accesses. The activation host callback also only records an
event; `ActivationNode` describes a projected Boolean `+6C` state. Pending
teardown rules that set `+5F` and do not touch `+6C` are a different operation.

The smallest future Source packet is one guarded Win32/MSVC header/source pair,
`include/bsp/native_entity_destroy_state.hpp` and
`src/native_entity_destroy_state.cpp`, implementing a descriptive provisional
`void __fastcall mark_native_entity_destroy_state_00922fd0(void* actual_node)`.
A narrow naked body can preserve all 25 instructions, relocate its recursive
call to itself, and keep the current indirect terminal jump. It needs no new
context, full entity struct, globals, table, owner, callback substitute or helper
implementation. Its actual emitted instructions and target ABI still require
primary review after registration/build; this audit does not supply them.

Concrete production entity tables, the permitted `+84` methods, hierarchy
producers, mutation/lifetime guarantees, application binding and the rest of
World destruction remain open. Their absence blocks production closure, not the
bounded caller-qualified leaf described above. No CMake, Source, ledger, GPR,
build, test or native execution probe was changed or run in this packet.
