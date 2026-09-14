# Actual resource hierarchy parser BV

Addresses: B7EB90, B7F100, B87AE0, B7D160, B7D220. EH support:
CC1FF0, CC1FF8, CC2000, CC2010, CC2018.

Five complete ordinary bodies total 1,245 bytes. They connect BT child traversal
and BU value reads to the existing actual hierarchy pool, field storage and
pointer-array services. This is raw storage parsing, separate from the earlier
guarded typed hierarchy projection. It still requires the application's bound
and initialized hierarchy pool and actual stream/string services.

| Address | Bytes | Original ABI | Operation |
|---|---:|---|---|
| B7EB90 | 769 | ECX manager; stack handle; RET4 | Allocate and parse one hierarchy record |
| B7F100 | 164 | ECX manager; stack handle; RET4 | Iterate Item children, skip unknown tags |
| B87AE0 | 59 | ECX resource; stack record; RET4 | Append raw record pointer at resource+1C |
| B7D160 | 191 | ECX output; EDX sphere; EAX output; RET | Ordered sphere-to-box arithmetic |
| B7D220 | 62 | ECX output; stack sphere; EAX output; RET4 | Stage leaf result, copy six floats |

B7EB90 obtains a slot through the existing B87A90 binding to the application's
actual 0109022C hierarchy pool. The 84h payload has parent0, name4/8,
matrixC..48, resource-index header4C/50/54, flags58, sphere5C..68 and
bounds6C..80. The final DWORD at slot+84 belongs to the 88h pool. Neither that
marker nor the matrix area is initialized by this parser.

For a nonnull slot, parent defaults to FFFFFFFF, the name/index headers and
flags to zero, and the sphere to (0,0,0,+1e10). Bounds default to (-1e10 XYZ,
+1e10 XYZ). The negative constant is read before field stores, the positive
constant after sphere XYZ stores, matching MOVSS order. Context references
borrow the actual CE4ADC/CE4970 cells; no mutable global copy is introduced.
Null allocation skips initialization but does not add later null protection.

Children are visited in encounter order. Parent, Resource, Matrix and Name use
a nonnull data-pointer check then the current CRT case-insensitive comparison.
Flags, BoundingSphere and BoundingBox use the complete existing 425850 raw
string-header comparison. Each branch rereads the current child fields.
Parent and Flags overwrite one DWORD. Resource appends a numeric index without
deduplication. Matrix overwrites sixteen floats. Name copies the current
temporary header through the same raw string pool, reloading both headers
after resize and using BF7680's memmove semantics. Sphere writes four values
and recomputes bounds; a later box overwrites only bounds. Unknown children
skip/detach. Recognized children are released without seeking unread tails.

Index append captures the value before reserve. At count==capacity it computes
capacity+4 with DWORD wrapping and a signed minimum8, then calls the existing
B7D640 reserve. It reloads current count/data after that call, omits only a
computed-null destination store, and increments current count. There are no
new signedness, overflow, capacity, payload-length or index-range guards.

After the field loop, B7EB90 reloads manager+24 and appends its raw record to
that current resource. B87AE0 captures resource capacity24 then count20, grows
by16 only when equal (signed minimum16 after wrapping), and uses the existing
B87350 actual pointer reserve. It reloads current data/count for the store,
then increments current count. No AddRef, record clone or old-array clear is
added. B7F100 retains the manager/container-handle pointers, calls this parser
for Item, skips other tags, and releases each child. Repeated containers append;
these functions build neither a parent graph nor an instance hierarchy.

## Floating-point boundaries

B7D160 rounds center loads and maxima through the original float32 temporaries
before calculating minima. It retains the x87 stack sequence, including the
radius staging and FSUBP/FXCH order, then writes minima XYZ by FLD/FSTP and
maxima XYZ by MOVSS. B7D220 computes into a separate six-word stack temporary,
then makes six ordered FLD32/FSTP32 copies. The parser makes another six such
copies to record+6C. The assembly entries preserve these boundaries and the
documented register/stack calling conventions under new source names. They
do not reset x87 state or promise whole-game binary replacement compatibility.

## Failure and cleanup

Handler CC2000 loads FuncInfo DFB168 and jumps to BF6B43. Its DFB158 map has
state0->-1 CC1FF0, which releases the child handle at synthetic EBP-34, and
state1->0 CC1FF8, which destroys the current temporary name at EBP-2C. The
temporary is armed only after the string read returns. Normal return captures
its data, disarms it, then loads its current length+1 and returns the captured
buffer. A return failure therefore releases the child but does not retry the
temporary destruction. Child cleanup is disarmed before normal child release.

Handler CC2018 loads DFB194; DFB18C maps its sole state to CC2010, which
releases the Item child handle at synthetic EBP+4. Both previously undefined
10-byte handlers were defined under the write lock and every instruction's
membership checked, then saved/exported. The three existing unwind functions
retain their prior names/comments. No function-flow or no-return change was
needed.

There is no record, field-allocation or pool-slot unwind action in either
parser. Failure leaves an unpublished record allocated, with any already
committed field state. The source preserves this behavior. It cleans only
the armed temporary/child and terminates on a secondary cleanup exception.
It does not turn parser failure into an implicit resource rollback.

## Validation and limits

Strict Win32 compilation and a focused probe passed. The actual-service part
uses the controlled-child data bootstrap, the existing memory stream, raw
string pool, hierarchy pool/list, node reference release and pointer-array
reserve/destruction. It parses repeated Parent/Name/Flags/Resource fields,
matrix, sphere and later box, unknown and empty tags, case variants, an embedded
NUL name, an empty Item and another appended container. Preseeded pool payloads
prove the matrix remains untouched when absent; pool markers remain valid for
return. A forwarding read mutates manager+24 during the first record, and
append selects the current resource.

A source exception after the Parent scalar's actual read leaves the record
unpublished; both active child handles unwind and earlier header debits remain.
The fixture identifies that slot among its preseeded allocations and performs
explicit fixture teardown. This does not attribute rollback to the native
parser. After destroying published/unpublished fields, returning their slots
and releasing pointer backing storage, the slab is fully free, the shared
allocator list is empty after pool destruction, and stream counters are zero.

The math part executes both complete original bodies from private relocated
copies. Only B7D220's one direct-call displacement changes. Across twelve
x87 precision/rounding modes, eight input patterns and three output/input
overlap placements, 576 paired cases match output bits and masked exception
status, retain the control word and leave the x87 stack empty. Patterns include
negative radius, signed zero, NaNs, subnormals, infinities and finite overflow.
Unmasked faults, condition flags and FIP/FDP are not certified by that probe.

The whole native parser bodies and native FH3/SEH were not executed as an
oracle; private-stack aliases, original CRT exception identity, other stream
families, allocation failures and gameplay remain unproved. Raw root/item
dispatch, resource manager/factory/parser registration, B80720 loading and
production queue worker shutdown remain open.
