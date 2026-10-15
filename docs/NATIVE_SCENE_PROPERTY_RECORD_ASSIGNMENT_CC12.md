# Whole ordinary scene-property assignment (CC12)

`assign_native_scene_property_record_008f0700` reconstructs the complete
193-byte/69-instruction body and the behavior selected by its twelve-entry,
48-byte table. The new ordinary C++ interface returns the actual destination.
Its name is a hypothesis; Original calling-convention and class identity are
not supplied.

The dispatcher reads only destination tag `+4`. An unsigned tag above 11 or
tag 6 returns without reading the source. No source-tag guard or self-assignment
shortcut is added. Active calls in the admitted owning-graph domain use
distinct disjoint actual 0x38-byte records with matching tags and valid selected
payloads. Record phase, tag, owner `+30` and ordinal `+34` remain unchanged.

Tags 0 and 4 copy a physical DWORD at `+0C`; tag 4 preserves destination
identity `+28`. Tag 3 copies one byte without boolean normalization. Tag 5
calls the already qualified physical `008F0340` on the two `+18` payloads and
leaves destination `+08` unchanged. Tag 7 performs three sequential DWORD
load/store pairs at `+0C`, `+10`, `+14`, without FP or a bulk snapshot. Tags
8–11 call the complete `008F03B0` byte-copy helper on `+20`, without stride
conversion or ownership repair. Their destination owning flag `+2C` remains one.

Tag 2 calls complete free-and-clear `004357F0` on destination `+0C` before
physically rereading source `+0C`, then calls the current physical string
duplicate `00438E40` and stores its result. The read order matters: exact self
would clear its own source pointer before the reread. That observation does
not qualify blanket self-assignment. Reference and array copy capture source
storage before freeing destination storage and can use freed memory when old
owning storage is shared. The initial whole-assignment domain excludes those
aliases and keeps the literal operation order without a repair branch.

Tag 1 is an actual x87 conversion, despite the integer-copy pseudocode. A
private helper emits exactly `D9 42 0C D9 59 0C C3`: `FLD m32[source+0C]`,
`FSTP m32[destination+0C]`, return. All binary32 inputs are admitted with an
empty x87 stack, all six exception masks already set, valid PC 24/53/64 and any
rounding control, and no pending unmasked exception. Hardware NaN quieting and
sticky status are retained. Production performs no C++ float conversion or
state reset. Undefined condition bits, popped register contents, environment
address fields, and Original parent register/stack identity are not oracles.

All allocating/freeing leaves use the actual current canonical heap. Their
nullable-text or positive-array-span contracts apply; source payloads, record
headers and active frames are disjoint from old freed allocations and new
outputs. Require DF zero, successful providers, sufficient stack, stable live
storage, and no mutation, reentry, overflow or invalid access. Allocation may
reuse the old address. No failure cleanup or zero-size CRT behavior is added.

The first strict Win32 compile and full COFF inspection passed: five emitted
functions, 466 section bytes, consisting of 415 instruction bytes/135
instructions, a 48-byte table and one three-byte NOP. All table targets, normal
control-flow edges and relocations are accounted for. The only FP instructions
are the private helper's load/store. Independent review checked the complete
source/object and the fresh original 193-byte body/table.

The required worker normal build passed both configured existing tests. That
build did not register the new assignment or leaf-storage TUs, so the focused
probe linked separately compiled current objects with frozen dependencies from
the completed build. Fresh leaf code/CFG/relocations exactly match its accepted
whole-object evidence. Whole linked inspection checked both public storage
helpers, assignment and its FP helper: 686 section bytes, 635 instruction
bytes/197 instructions, the table and padding. Six other emitted helper
functions have positive linker discard records.

After independent prelaunch Source review, one actual Source process exercised
owned string replacement, positive owned byte-array replacement and one SNaN
case. It exited zero with no stderr. Source roots/payloads and every unassigned
destination byte stayed unchanged; same-context cleanup completed. The string
replacement reused its old address, which is allowed. `7F812345` became
`7FC12345` under CW `0F7F` with status `0001`, matching a direct hardware
reference; CW, empty stack and MXCSR checks passed. All twenty selected inputs
and file stamps remained stable across the process. This selected case is not
an all-input or PC/RC runtime matrix, and cleanup completion is not an
independent per-free trace or leak counter.

Registered-library integration is separate integrator-owned evidence. No
Original Native target was executed. Historical private CRT, register/stack/
flags transport, SEH/EH/OOM, class/vtable dispatch and game behavior remain
outside this ordinary contract.
