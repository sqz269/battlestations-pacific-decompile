# Actual scene record map range erasure

Complete0046B570 behavior is in `src/native_scene_record_map_erase_range.cpp`.
The live typed function has two ranges: [0046B570,0046B5EF) and
[0046B5F0,0046B639),200 instruction bytes/76 instructions. The201-byte envelope
includes one unreachable NOP at0046B5EF. Exact whole-envelope live bytes match
the preserved installed PE. Target is existing `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, x86 base400000. Names remain descriptive hypotheses.

## Interface and reached private backing

Native ECX is map, incoming EDX unused; five stacked DWORDs are output,
first-owner/first-node,last-owner/last-node. RET14h returns CURRENT output in EAX.
Source exposes live actual20B argument backing plus explicit private output8B
and a pointer to the accepted single-erase frame. Frame metadata is stable and
disjoint from cells/payload. Child metadata is dereferenced only AFTER a reached
partial iteration's increment returns. Full/empty paths admit null child metadata
and unbacked private output. Each actually reached load/store/release still needs
valid matching backing. No Source upfront domain guard or new callback exists.

With P=native range entry ESP, outer cells are P+4..P+17h; private child output8B
starts P-8. Child call words are out at P-24h, owner P-20h, node P-1Ch, ending
before P-18h. Child entry ESP is P-28h and its saved-map cell is P-7Ch. Source
models explicit backing, not original physical cross-frame coincidence.

## Full-range path

Capture first owner BEFORE reading current head/minimum. A returning initial
validation report does not reload owner/minimum. Load CURRENT first node only
after that report returns. If it equals the captured minimum, load CURRENT last
owner and capture CURRENT head before the second validation report; compare
CURRENT last node to that captured head afterward.

A full match destroys CURRENT root before reset. Reuse complete0046B460's
exact native inlined operation schedule: AD10 then parent self; capture head
before count0 and left self; reload head and right self. This is a Source reuse
of identical concrete raw operations, not a claim of a native call to0046B460.
Then reread head/minimum BEFORE reading current output pointer. Publish captured
map OWNER first, captured minimum NODE second. No private child frame is touched.

## Partial and empty paths

Loop with captured first owner/node. Owner zero skips the last-owner load and
reports through real current-CRT BF6713; otherwise compare captured owner to
CURRENT last owner and report if unequal. A returning report is followed by
comparison of captured node to CURRENT last node. Equality exits with captured
register-like owner/node, not new first-slot snapshots.

On inequality increment the actual OUTER first iterator cells through complete
00467980. Then prepare the single-erase call words in native push order: OLD
captured node, OLD captured owner, private output address. Call complete0046A470
using its live arguments and saved-map cell; ignore its EAX. Reload CURRENT outer
first node FIRST, first owner SECOND and repeat. Failures propagate at the child
frontier without invented rollback, cleanup or output publication.

Empty/report-return exit reads CURRENT output pointer and publishes captured
OWNER then NODE. This differs from recapturing first cells after a handler.
Output and payload aliases retain the listed read/write order.

## Evidence and limits

All6 direct body transfers and4 callers have exact typed rows. Callers are
0046C500 atC516,0046CED0 atCEE6,0046EF70 atEF86 and0046EFB0 atF0EE. There is no
extra allocation, error carrier, native EH action or new cleanup in this wrapper.
Ghidra body/listing/call setup and actual child contracts were inspected. Build,
annotation and independent-review results are recorded in the report as completed.
Source uses explicit live private cells and current raw/CRT providers. Original
binary/CRT/heap/private-frame/register identity, application adoption and game
behavior remain unvalidated. Existing tests are regression checks, not new-body
native differential execution.
