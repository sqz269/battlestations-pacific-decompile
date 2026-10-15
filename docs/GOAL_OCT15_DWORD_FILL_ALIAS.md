# Actual DWORD fill alias

The complete `00468440..0046845D` body contains 30 bytes / 13 instructions,
five blocks and no outgoing native transfer. Correct descriptive library name
`STL_Fill_00468440` is retained as a behavior hypothesis. No indexed or unindexed
whole-body Source was found before this reconstruction.

Original ABI is cdecl with stacked first/last/value_address at entry+4/+8/+Ch,
plain RET and raw EAX equal to captured last. ESI is pushed and restored only
on the nonempty path; incoming ECX/EDX/ESI have no input role. No semantic return
type is inferred from callers. The new Source API exposes the established raw
pointer result and preserves the canonical ordinary interface's admission.

Native code captures first then last and compares them. If equal, return without
loading the third stacked word or touching its pointed-to storage. If different,
capture value_address once. Each iteration performs a FRESH raw four-byte load
from that address, stores into current cursor, advances cursor by4 modulo32,
then tests equality with captured last. No alignment, order, count, overlap or
null guard is introduced. The source value is not snapshotted before the loop;
valid source/destination overlap retains progressive mutation.

The whole native body is identical to accepted `0043F9E0`. Source delegates to
`fill_native_dword_range_0043f9e0` in `native_dword_fill_ranges.hpp/.cpp`, after
reviewing its complete actual raw implementation and original listing. That
implementation uses raw four-byte memcpy loads/stores and uint32 cursor math;
it captures the value address only inside the nonempty branch and rereads its
current DWORD on every iteration. The ordinary interface takes pointer values;
it exposes no implicit aliases to Source/native argument homes or private frames.
An empty range reads no value backing. All reached raw accesses must be admitted,
including a reachable equality bound for this range form; no stronger ordering
or alignment condition is added. Asynchronous mutation and original native
fault/SEH/register-spill identity remain outside the accepted interface.

Delegation describes Source composition only. The owned native body has no
outgoing transfer, EH frame, allocation, validation, cleanup or rollback.

`0046B122 CALL 00468440` occurs in `0046AFB0`.
It pushes value address EDX, last ESI and first EDI; afterward ADD ESP,0Ch clears
that triple.

`0046B155 CALL 00468440` occurs in `0046AFB0`.
It pushes value address EAX, last ECX and first EDI; subsequent ADD ESP,18h clears
the earlier pending backward-copy triple and the fill triple. Neither continuation
consumes EAX before the containing RET10h. This does not establish a nominal
containing return type or owner. Caller context is recorded in
`local/goal_oct15_dword_fill_alias_caller_context.json`.

## Verification

Whole original ABI/listing, accepted actual provider and both caller setups were
reviewed. All 30 owned live bytes match the installed PE and canonical body.
The two actual incoming typed transfers are checked separately from Source reuse.
Locked annotation preserves names/prior comments/evidence and records old values,
save, refreshed export and readback in the report. Required normal Win32 build,
existing checks and focused compiled alias/provider review are pending. No new
tests are added; packet-specific native differential/game and original ABI remain
unproved.
