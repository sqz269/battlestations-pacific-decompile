# Unit-part entry production and name assignment

Addresses: 00711C60, 0070F900, 004BEB60, 00B6F960, 00711080, 007135C0.

The complete entry producer and node-name setter are now concrete defaults of
`NativeUnitPartConstructionBindings`. The setter receives the constructor's
existing `NativeStringRawPoolContext`, preserving the same publication and
allocation/return domain. No replacement model, vector or pooled-string owner
is introduced. Selected-set release is also implemented through an explicit
virtual slot-zero callback; its target and spatial attachment remain required.

## Entry producer

`00711C60..00711E5B` takes ECX=part and no stack arguments, with plain RET.
A null selected set at part+160 is inert. Otherwise it scans the selected set's
checked vector at+7C: begin+80/end+84, stride8. Record+0 points to the actual
named record containing a legacy SBO at+8; record+4 supplies the captured node
identity. It preserves the initial vector identity, reloads current set/end at
the native continuation sites and permits the SDK invalid-parameter handler
to return after repairing actual storage.

The caller first copies the full name into a temporary, assigns a second
owning SBO, and releases the first copy. It searches for the first underscore
using the one-byte set at CE7890, then constructs a prefix even when the name
will be rejected. A record is emitted only when the underscore index is4 and
the prefix equals the exact case-sensitive `part` literal at CFD7B0. Thus
`part_`, `part_1` and `part__x` match; bare `part`, `Part_x`, `_part` and
`particular_x` do not. Embedded NUL and high bytes retain counted-byte semantics.

For each match it allocates10h bytes through the canonical BF681B allocator,
zeros DWORDs4/8/C, then writes the previously captured node at0. It appends the
record pointer to the existing part+1A0 signed pointer/count/capacity array,
using the already reconstructed append/reserve operations. Existing entries,
duplicates and source order remain intact. Record allocation precedes reserve;
native has no cleanup action for that unpublished record if reserve throws.
The source preserves that failure ordering. The record's other fields are
unnamed; no downstream ownership/destruction semantics are inferred from zeros.

The full-name, copied-name and prefix unwind order is grounded in FuncInfo
DB3100, map DB30E8 and actions C84930/C84938/C84940. The source uses canonical
SBO destruction for completed objects during C++ exceptions and the native
free/reinitialization schedule during normal execution. Three returning-free
gaps in the caller were repaired and saved. The six-byte unreachable alignment
gap at711CAA is retained.

`0070F900..0070F985` is the general legacy-SBO find-first-of helper (ECX string;
stack needle,position,count; RET0C). `004BEB60..004BEBCC` is a checked substring
comparison (ECX string; stack position,count,other,other_count; RET10). Both
complete bodies and callees were inspected. Their general source templates are
not ported: this caller's exact one-byte search and four-byte equality invocation
use the SDK byte operations. The original helper bodies execute in the fixture.

## Name and selected-set reference operations

`00B6F960..00B6F995` takes ECX=node, a source8h string-header pointer on the
stack and RET4. It targets the canonical `NativeNodeStorage::name_54` field.
Self-assignment returns immediately. Otherwise it resizes through the current
raw pool, then reloads source length, destination length, source data and
destination data before the copy. The source uses the overlap-capable SDK copy
boundary already used for BF7680, omitting only a zero-byte copy after its
operands are read. It neither retains the node nor changes its hierarchy.

`00711080..007110A8` takes ECX=an actual selected-set pointer cell and RET.
Null is inert. It captures the owner, performs the real InterlockedDecrement
on owner+4 and, only for a zero result, loads the current vtable and slot0 and
dispatches it with that same captured owner. There are no stack arguments or
deletion flags. After return it unconditionally clears the original cell,
including when the callback replaced that cell. Source callback exceptions
escape before the final clear. The source never casts a native token into a
callable address. Complete selected-set destruction remains outside this caller.

## Paired construction evidence

The fixture retains53 physical original spans and196 relocated operands,
combining the prior group/collision and storage references with these bodies
and the complete356-byte constructor at7135C0. All reference bytes match the
current Ghidra program and unchanged installed PE. Native and source sides use
the same explicit source CRT/SBO/pooled-string boundaries and actual standalone
8AD4A0h pool owners; publication is already populated, so no second manager is
created. The source constructor supplies real allocation, group production,
collision construction, node naming and entry production.

Six paired constructor scenarios cover three name datasets with null and
nonnull unit owners. The nonnull cases use an explicit virtual-name callback
and the actual root pointer at selected-set+0C. Their collision-source vector
is empty, so spatial attachment stays unexecuted. Raw part fields, group
memberships, entry records, node fields and name bytes match. Repeated entry
production preserves existing entries and appends duplicates. Heap-backed
names, missing underscores, case, embedded NUL and high-byte cases are included.
Name checks include self-assignment, overlapping data, large pooled-string
allocation and clearing. There are14 producer calls and15 name-setter calls
per side, including the paired null-set and returning-validation cases.

Six paired selected-set release cases cover null, positive/nonzero, zero,
negative/wrapped counts and replacement of the owner cell from slot0. One
source-only callback exception checks that clearing follows successful return.
The fixture frees its produced raw records explicitly; that is not evidence
for the complete game-owned part destructor. Native FH3, private native stack
identity, hardware faults, original CRT global/exception state, real unit-name
targets, attachment, selected-set destructor targets and gameplay remain unproved.

All33 direct-call rows are checked against live Ghidra ownership/listings.
Three indirect sites are separately recorded: CE2220's imported decrement,
selected-set slot0 and the constructor's unit slot10. Their operands and fixture
dispatches are verified separately, not counted as direct-call passes. Names
are descriptive hypotheses, not recovered symbols; source interfaces have a
new C++ ABI. The build/fixture evidence does not establish game admission.

## Integrated validation

Commit `b392d91d338c080c4296cc0367d80d5af495520e` passes the Win32 build and both existing CTests. The constructor/entry/name/reference fixture passes against that library. The prior AJ storage/unwind fixture also passes with its original result hash; only its unused name-override signature was adapted to the explicit pool parameter. The 120-frame USN01 compatibility run passes finite-trajectory, stationary Airfield2, avoidance, generic-tick, participant, world-list and observer/pending-owner checks. The preserved executable SHA-256 is `2f509dedb905d7b800b198710892f9f1270b3f0233091474da9d575465186985`. An immutable manifest retains original reference bytes, raw results, linked objects, compilation dependencies and mission artifacts. Game admission and gameplay parity remain unproved.
