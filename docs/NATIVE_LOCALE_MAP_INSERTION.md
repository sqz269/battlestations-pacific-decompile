# Qualified raw locale map insertion

`insert_or_get_native_locale_value_00a9fc30(context, frame)` binds the existing
game-owned insert-or-get and node-key constructor to actual Win32 storage.
It is dormant ordinary C++, not Original ECX/RET4/EAX/FH3 or native fault ABI.
It supplies no manager, startup/loading caller or new vendor/Native body credit.

Original A9FC30 takes ECX=bucketbase and one stacked raw8h key pointer, returns
node+8 (the wide value header), RET4. A9F460 takes ECX=fresh record and one
stacked raw8h source, returns SAME record, RET4. The new internal F460 body
operates on bytes; it does not cast a raw slot to `NativeString` or use a
semantic `LocaleTables`, STL map/vector or broad host with unrelated stubs.

## Fixed graph and native epochs

The pure Context fixes actual4004h map, raw8h key, SAME actual AA8/AA4/AA0
string publication cells and canonical `GameNativeLocalePoolProcess`. Its
nonmoving/noncopying Frame contains a real prepared node-allocation frame.
Only the exact fresh Context/Frame pair can enter; failures/reentry/replay
poison it before diagnostics throw. Copied progress is historical diagnostic
data, not issuance, storage liveness, cleanup or ownership authority.

Hash captures initial key length once and, only if nonzero, data once. Read
all captured bytes, including embedded NUL; fold only ASCII A..Z and add the
sign-extended folded byte to wrapping hash*83h, then maskFFF. No strlen or
semantic-string conversion is used. Traversal reads each node length then
CURRENT key length. Equalzero matches without data access. Equalnonzero reads
CURRENT right/key data BEFORE CURRENT left/node data and calls actual current
CRT `_stricmp(node,key)`. On inequality load CURRENT node.next14 after return.
A duplicate returns its actual node+8 with no count/allocator/constructor or
pool getter. It does not assign text or add an ownership credit.

On miss, DWORD-RMW map.count4000++ BEFORE allocation. Then, as an explicit
additional Source admission observation, borrow the healthy canonical pool
into a local value. The actual allocator still performs its own genuine
healthy borrow before Enter. Both refer to the SAME stable canonical domain;
no concurrent/reentrant teardown is admitted. Failure in either admission or
allocation retains the incremented count. This extra observation is not a
claim about Original call count/ABI. Allocator progress is compared only as
diagnostics, never used to select cleanup; the captured local pool is used.

Call the real published A9F0B0 binding and retain its immediate slot/slab/table/
lock diagnostics. Its private normal receipt must agree with the actual call
result. Native NULL skips F460 and still reaches null+14/+10 link accesses:
there is no invented graceful NULL outcome or substitute allocation provider.

F460 captures self identity before zeroing key.length0 then key.data4. When
distinct, read CURRENT source length and call actual raw0041DD40 with preserve
true and the SAME string context. After resize, read CURRENT source length;
nonzero selects copy. Capture CURRENT destination length, then source data,
then destination data. Use the existing raw-string overlap-aware memmove
convention. Operands are still read when count0; that zero-byte call alone is
omitted under the established Source policy. Resize supplies its terminator;
no extra terminator/header copy is invented. Only after normal copy zero wide
length8 then dataC, returning SAME node. Links10/14 and pool index18 are untouched.
The published74c E970 byte initializer plus real allocation supply the raw slot
and index18; no `NativeString` object lifetime is manufactured.

After constructor normalreturn, its metadata/sticky fence is OUTSIDE selected
cleanup. Read CURRENT bucket for new.next14, store new.prev10=0, reload CURRENT
bucket a SECOND time and update that head.prev10 ifnonnull, then publish new
head and return node+8. The preallocation head is not reused or snapshotted.
Only whole normal completion issues this frame's private result receipt.

## Selected cleanup and retained failure

The narrow try contains ONLY the actual F460 invocation. An ordinary C++
exception escaping that invocation is saved independently and selects real
`return_native_locale_node_slot_00736090(captured_pool, captured_slot)`.
This matches saved CB6D00's allocatedslot ->7373F0 ->736090 cleanup arm. Entry
authentication, the extra Source pool observation, allocator, and post-normal
metadata fences are outside that try. A started bit or diagnostic never
selects this arm. The normal-returned constructor cannot be relabelled failed
to release its slot after a later Source guard failure.

Latch failure before selected return. Record its actual entered/returned or
secondary exception separately; rethrow the original constructor exception,
retaining secondary failure metadata if necessary. This ordinary C++ policy
does not establish Original native exception origin, FH3/SEH, hardware timing
or double-failure equivalence. No selected return is inferred after allocator
failure or for a slot that was never actually returned by that allocator.

There is NO key/wide/payload destructor, header clear, count rollback, bucket
repair, generic slot return, slab/table free, extra unlock, cancellation or
retry. Count remains incremented after failed construction. Even successful
selected slot recycling does not settle partial header/buffer/map debt or
make the graph safely reusable. After constructor normalreturn followed by
metadata failure, retain the constructed unlinked node and all header debt.
After allocator escape retain its whole partial pool/section/slab/table state.

Keep Context/Frames, initialized native map/key/backing, pools/owner/imports and
actual string domain alive through all effects/cleanup and any unresolved
failure. Do not follow old slot/header/data after selected return: saved pointers
are IDs only. Caller proves disjoint stable metadata/provider/native extents,
readable current headers and reached compare/copy buffers, terminators for CRT
comparison, external serialization and the shared pool/list lifetime window.
Unsupported private aliases, concurrent/reentrant mutation/teardown and import
replacement are excluded. Startup health is not safe trim/exit recovery.

The returned value header is a borrow; duplicate does not create it and new
insertion creates only an empty wide header. Real value assignment, parser,
loaded manager/publication, aggregate retirement and application startup are
separate. Current CRT locale/handler behavior is qualified, not Original
BF7FBF/TLS/0109DE1C equivalence; no fake mode cell/provider or vendor body is
added. Invalid/nullable hardware domains remain outside qualified success.

Evidence: saved FC30..FD46/F460..F4AD/7373F0..FB assembly, guarded read-only
CB6D00..03, frozen contract2087c497, and Source dependency pins in the JSON
report. Validation is one strict Win32 object compile only. Source-ready,
runtime, native ABI, full caller, Page/P4 and production claims remain false.
