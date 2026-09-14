# Actual resource child traversal (BT)

Addresses: `00BEA250`, `00BEA680`, `00BF02A0`, `00BF03E0`, `00BE9A40`,
`00BE9C40`, `00715BF0`. Source: `native_resource_node_traversal`.

Seven complete ordinary bodies total 582 bytes. These new C++ interfaces use
the original 24h node and 70h reader storage, the BS raw stream/string reads,
existing string pool and memory/physical/adopted/inflate stream dispatch, and
the BS node destructor and reference release. They do not construct a second
owner or use the guarded typed structured-reader abstraction.

| Address | Bytes | Original ABI | Behavior |
|---|---:|---|---|
| BEA250 | 300 | ECX node; stack parent; EAX node; RET4 | Borrow parent/reader, read child headers and publish path |
| BEA680 | 121 | ECX parent handle; stack output; EAX output; RET4 | Allocate24h, reload parent, construct, publish |
| BF02A0 | 27 | ECX reader; stack budget; EAX DWORD; RET4 | Current slot38 scalar and actual-count debit |
| BF03E0 | 21 | ECX reader; stack low/unused pointer; forwarded EAX; RET8 | Current slot1C seek(low,0,1) |
| BE9A40 | 22 | ECX node handle; RET | Store control into captured reader+64 |
| BE9C40 | 70 | ECX node handle; RET | Debit parent, seek remaining, pop path, detach |
| 715BF0 | 21 | ECX handle; EAX0/1; RET | Nonnull node and nonzero remaining |

BEA250 writes ref1 and profile D68BB4. It borrows the input parent's reader and
stores the parent, clears tag10/14 and sets depth from the original parent's
current depth+1. The first read uses current node.parent/reader; after the tag
copy and captured temporary-buffer return it reloads those fields separately
for the declared-length read. Each header helper subtracts the actual transfer
count from that parent's current remaining20. The child declared1C and
remaining20 receive the returned DWORD. Parent payload debit happens later,
when the child detaches or is destroyed while attached.

The first tag copy reads current destination length, destination data, then
source data after resize. The path copy reads destination length, source data,
then destination data. Both use BF7680's memmove semantics. Path selection uses
the current reader+60 index; the increment is published before resizing or
copying the path slot. There is no capacity, depth, magic, null-parent, signed
remaining, full-transfer or rollback guard. Borrowed parents/readers are not
retained. BEA680 reloads the parent's handle after allocation; null allocation
publishes null without dereferencing that handle. Constructor failure leaves
the output untouched and frees the captured allocation after child cleanup.

BF02A0 initially seeds its actual-count local with the reader pointer bits,
captures current stream and slot38, then subtracts actual from current budget
with unsigned wrapping. Known scalar targets BE42E0/BE4300 call the existing
complete source bodies. Other reached targets throw as an explicit unresolved
binding. BF03E0 consumes no second pointer and forwards the seek result. It
passes the low DWORD unchanged with high0 and relative-origin1. BE9A40 captures
the node and its reader before dispatch and writes reader+64 to that same
captured reader even if a call changes node.reader.

BE9C40 captures the node, subtracts its declared1C from a nonnull parent's
current remaining20, then seeks the captured remaining20 when nonzero. Only
normal return clears remaining20. It reloads node.reader after the seek,
decrements that reader's current path index and clears node.reader. It ignores
the seek result. Zero remaining skips seeking but still debits, pops and
detaches. There is no attached check: calling skip again on a detached node is
not made safe. A seek exception preserves the earlier parent debit and does
not execute subsequent clear/pop/detach instructions. The existing destructor
sees detached reader0 and avoids a second parent debit or path pop.

## Cleanup evidence

Child handler CC716B loads FuncInfo E01B94 and jumps to BF6B43. Its E01B7C map
has state2->1 CC7163 (temporary header at synthetic EBP-14), state1->0 CC7158
(saved node at EBP-18, tag+10), and state0->-1 CC7150 (base BD30F0). Normal
temporary return has already disarmed that temporary. A later failure cleans
the node tag and base without retrying the temporary return or rolling back
the path index. Secondary C++ cleanup failure terminates the source process.

Child allocation handler CC71FB loads E01C64; E01C5C maps state0->-1 to CC71F0,
which frees the allocation at EBP-10. CC71F4 had a spurious call-flow override
that omitted POP ECX/RET from function membership. The override was cleared,
the function recreated without clearing bytes or changing BF65AC's no-return
flag, and all five instruction owners and prior evidence verified. Both
10-byte FH3 handlers were defined, verified, saved and exported.

## Validation and limits

The strict MSVC Win32 source compile and one actual-service fixture passed.
The fixture uses the existing suspended-child data bootstrap, original PE
read-only data, actual string pool, memory stream, reader and reference paths.
It constructs Resource/Child/G nodes, checks header and payload debits, control
values, depths and paths, then exercises nonzero and zero remaining skips and
release without double debit. Stream allocation counters finish at zero.

Forwarding dispatch hooks additionally confirm the captured control destination
and post-seek current-reader reload after real service calls. One source
exception injected after the actual child payload-header read confirms the
prior tag-header debit remains, the payload debit/path publication do not run,
and child cleanup stamps the base profile. This is a C++ source exception
check, not execution of native FH3/SEH or an original-body differential oracle.
Allocation-null, path-copy failure and secondary-cleanup termination paths were
inspected but not dynamically exercised. Private stack aliases and hardware
faults are outside these interfaces. No gameplay run was performed for BT.

See `NATIVE_RESOURCE_NODE_TRAVERSAL_INTEGRATION_BT.md` and the two matching JSON
reports for exact build, instruction ownership, call and artifact receipts.
Raw root/item/hierarchy dispatch, manager/factory/parser construction, B80720
loading and production queue worker shutdown remain open.
