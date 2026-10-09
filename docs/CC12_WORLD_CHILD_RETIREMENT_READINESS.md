# Actual child retirement: 009035E0

The complete native routine is **42 bytes / 16 instructions**. Its only direct
dependency is recursion into itself, followed by an actual current slot-zero
virtual call. A small, caller-qualified raw Source leaf is ready to implement.
Real node/table provenance, child unlink/count/head mutation and lifetime remain
explicit external requirements; this audit creates no owner or synthetic service.

No Source implementation or new Original/ABI/game-validation credit is added.
The existing `destroy_child_subtree_009035e0(WorldExpiryHost&, void*)` is already
counted, so future actual-storage work must refine that address without adding
a second Original function.

## Evidence boundary

- Published and worker baseline:
  `3b9c2575e65bd32a0f45f9727ee7a4a06107054c`.
- Verified Ghidra target: `C:/Users/sqz269/bsp.gpr`, program
  `/battlestationspacific.exe`, `x86:LE:32:default`, image base `00400000`;
  live/snapshot counts both 64,729.
- Metadata established `009035E0..00903609`, 42 bytes, before the complete-body
  read. It fits the requested 300-byte limit. Full live pseudocode, listing and
  bytes were captured; the register and call schedule comes from assembly.
- All 42 live bytes equal the installed PE span. SHA-256:
  `a84482b641eec58c063a5e47549586e95b17485ffb141239c13634654f55e2f8`.
  Installed PE SHA-256:
  `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
- The JSON embeds every byte/instruction, both branches, both CALLs and current
  canonical Source pins. No other native body, caller, descendant, table or
  exception funclet was queried. Prior caller evidence remains attributed.

## Complete ordinary call schedule

1. `009035E0` saves `ESI`, and `009035E1` retains the actual ECX receiver in it.
   `009035E3` compares the receiver's full DWORD `+50` with zero. Zero goes
   directly to the receiver's own virtual call; its `+48` is not read.
2. The nonzero path executes the seven-byte no-op
   `LEA ESP,[ESP+00000000]` at `009035E9` (`8D A4 24 00 00 00 00`). It changes
   neither ESP nor flags. `009035F0` then loads the receiver's current `+48`
   child pointer into ECX and recursively calls this entry at `009035F3`.
3. After the entire child invocation and its final virtual method return,
   `009035F8` rereads the same parent's current full DWORD `+50`. If nonzero,
   `009035FC` branches back to a fresh `+48` load. There is no saved child or
   successor, sibling read, cached count, mark test, head advance or local unlink.
4. Once the gate is zero, `009035FE` loads the receiver's current vptr and
   `00903600` loads current slot zero. The function pushes full DWORD `1`,
   restores ECX to the receiver, and `CALL EDX` at `00903606` invokes that target.
5. On return it performs only `POP ESI` and plain `RET` at `00903608/00903609`.
   This ordinary call/return schedule includes no own-receiver field access after
   its virtual method. It must not be changed to a terminal JMP.

The receiver arrives in ECX; no explicit incoming stack argument or incoming EDX
value is read. The recursive call also passes only ECX. The virtual call carries
one four-byte argument with value `1`; no caller adjustment follows, so a
compatible target must consume that argument space. EAX holds the late table
pointer and EDX the late target immediately before dispatch. No typed return
value or complete concrete virtual-method ABI is recovered here.

There are no object-field stores, global/FPU accesses, direct allocator/free calls
or local EH frame. The only local data accesses are receiver `+50`, receiver
`+48`, receiver vptr `+0` and table slot `+0`. These establish an access footprint,
not a full object type, size, allocation domain or production table.

## Current hierarchy, retirement and lifetime

"Child count" denotes the existing descriptive name of DWORD `+50`. Every
nonzero bit pattern enters the child loop, including negative signed patterns.
A nonzero gate with null `+48` is recursively forwarded; no null-success policy
is present. A null receiver faults at its initial `+50` comparison.

Every child return is followed by a read of the **parent's** `+50`, then possibly
that parent's current `+48`. The returned child is not dereferenced. The child
may have retired itself if the actual virtual method and external ownership
contracts permit it. The parent must remain valid for its post-call reads and
late own dispatch. Child callbacks can replace the parent's head/gate or current
vptr/slot-zero method; those changes are observed at the native reload points.

The function itself does not unlink nodes or decrement counts. The real virtual
methods and hierarchy producers must make the intended loop progress and leave
valid parent storage. Cycles can recurse indefinitely, and a nonprogressing gate
can repeat or reach invalid storage. No visited set, mark-based suppression,
synthetic head/count update, guard or fallback is justified.

The already-pinned `00903610` caller loads its current entity's child `+48`, calls
this entry, then rereads that same parent entity's `+50`. This agrees with the
internal recursive contract: parent storage must survive; the returned child is
not read by those continuations. No native caller body was re-queried here.

After its own current virtual call this routine reads only its saved stack state.
Other callers/owners may impose additional lifetimes. Earlier child effects remain
on later failure; there is no local rollback, exception translation or new
`noexcept` guarantee. Native EH/unwind/fault and runtime compatibility are untested.

## Current Source and the minimum implementation

The existing `WorldExpiryHost` version expresses recursion through virtual
`child_count`, `first_child` and `destroy_entity_vtable0` services. Its current
`WorldExpiryBinding` uses index tokens/vector counters, reports zero children and
logs deletion. It is not an actual-pointer provider. The new raw helper needs
none of those host interfaces: its sole direct edge can bind to its own Source
entry, and the indirect edge remains the supplied object's actual current table.

The minimum future pair is `native_world_child_retirement.hpp/.cpp`, with guarded
MSVC Win32 provisional interface
`void __fastcall retire_native_world_child_subtree_009035e0(void* actual_node)`.
A narrow naked body can retain all 16 instructions, explicitly preserve the
seven-byte LEA, relocate the sole direct CALL to itself, and keep the ordinary
virtual CALL followed by POP/RET. It adds no layout overlay, context, pointer
token, global, callback substitute, table, owner or synthesized unlink operation.

After separate Source implementation, registration and whole emitted/self-call
review, that helper can close `00903610`'s direct-service hold. The present audit
does not close it by itself. The published `00903670` caller and `00922FD0` helper
now have prior complete emitted/binding/build evidence; neither is a callee of
this routine. Their availability does not supply actual slot-zero methods or
hierarchy lifetime/progress guarantees. Those production obligations, application
wiring and complete teardown remain open. Only the two evidence files changed;
no Source, CMake, ledger, GPR mutation, build, test or probe was performed.
