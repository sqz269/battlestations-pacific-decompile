# Raw command-list clear

Address: 0046A9F0.

This is the whole110B/43-instruction control flow over actual list storage,
using the integrated raw record destructor and concrete current CRT free.
Native ECX is the actual12-byte {count0,head4,tail8} header, EDX is unused,
and plain RET has no semantic result. The new C++ API takes that header,
explicit child saved-this4 backing and the concrete raw string-pool context;
it is not an Original binary replacement or a SceneDatabase owner wrapper.

| Routine | Coverage | Source |
| --- | --- | --- |
| 0046A9F0 | Whole110B/43ops, no alignment inflation | Ordinary raw-storage Source; review/build pending |

The existing list producer confirms actual12-byte nodes {prev0,next4,record8}
and header offsets. Older Seg6Host and semantic command-queue projections stay
separate. No container abstraction, new owner, callback, guard or rollback is
introduced. The clear routine itself has no dynamic call or diagnostic path.

The initial CURRENT count read controls an exact nonzero loop, including high
bits and DWORD wrap. A zero count skips head/tail, every record and child saved
cell. Each iteration freshly reads current head and captures its node, then
captures node+8 record once. A nonnull record invokes the real destructor,
frees that CAPTURED record and only then zeros CAPTURED node+8. Null records
skip the child, record free and slot write. No pointee substitution is allowed
after a returning provider.

Unlink then reads CURRENT node.prev. Its nonnull branch freshly reads next and
stores prev.next; its null branch freshly reads next and stores header.head.
After that write, it reloads CURRENT node.next for the second branch test.
If nonnull, it reads CURRENT node.next AGAIN, reads CURRENT node.prev, then
stores next.prev. If null, it reads CURRENT node.prev and stores header.tail.
These ordered reloads and writes remain explicit even for overlapping cells;
there is no single cached next value or simplified pop-front operation.

Only after the link writes does it read CURRENT count, subtract1 with DWORD
wrap and write count. It frees the CAPTURED node, then reloads CURRENT count
for the loop condition and CURRENT head for a reached next iteration. It does
not use the decrement result as a cached loop condition or zero/repair the
header after the loop.

| Native transfer | Role |
| --- | --- |
| 0046AA0C CALL 00469010 | Actual captured-record destructor |
| 0046AA12 CALL 00BF65AC | Current free of captured record |
| 0046AA4D CALL 00BF65AC | Current free of captured node |
| 0046DF33 CALL 0046A9F0 | Scene-reader incoming call |
| 0046AC34 JMP 0046A9F0 | Resolve-pass tail incoming edge |

Both frees have one push and ADD ESP,4; the child and list routine consume no
native stack arguments. The two incoming setups add14Ch to their current
database pointer before the edge. The tail caller restores its saved entry
pointer on the populated path; its empty path retains the entry ECX directly.
Its E9 JMP has a preserved CALL_RETURN flow override: default unconditional
jump, effective CALL_TERMINATOR. The report records a tail jump, not an
invented native call. No flow or prototype repair is needed.

The child is the integrated whole actual-storage record destructor in
native_scene_command_record_destroy.cpp. Current CRT free is the existing
singleton_lifetime_free/std::free provider. All direct and transitive provider
Source hashes are pinned, along with the child's reviewed/build-tested report.
There are no missing-call substitutes or new graph/text/clone admissions.
Each reached node, header, record, pool and current-heap access must satisfy
those existing actual-storage contracts.

Binding metadata and context objects remain stable and disjoint from reached
raw accesses. The actual cells and payloads retain their raw aliases. Child
saved-this4 backing is accessed only for a reached nonnull record; implicit
compiler/private-unwind/outgoing homes are not supplied. A child C++ throw
propagates before subsequent record/node free, slot zero, link or count writes.
No parent EH cleanup or rollback is added. The child's standalone cleanup,
current C++ exception transport and current CRT are qualified; Original heap,
FH3/SEH/private frame, ABI, application and game equivalence remain unproved.

Guarded analysis verifies C:/Users/sqz269/bsp.gpr and program
/battlestationspacific.exe. Whole live bytes equal the installed image. The
report retains five true transfers, both incoming setups, original ABI,
provider hashes and prior native annotation values. Correct native name and
comments are preserved with locked save/export/readback. Complete primary
Source review precedes the primary combined registration/build; there is no
worker CMake queue, worker build, new test or native-entry execution claim.
