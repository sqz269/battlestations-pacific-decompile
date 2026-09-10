# Resource and hierarchy-item ownership

Addresses: 00718810, 00718c20, 0071b8d0, 0071b940, 0071b9b0, 00b86990, 00b872f0, 00b87350, 00b87ae0, 00b88180, 00b88430, 00b88760, 00bd30e0.

The resource's primary item array carries one release obligation per stored
entry. Its append and reserve routines copy raw pointers without AddRef; the
base destructor decrements each item's reference count and invokes its deleting
destructor only when the count reaches zero. Hierarchy records have a separate
destruction path that tears down their fields and returns their native pool
slots. These conclusions come from complete disk-matching normal-path bodies.

The three derived classification lists free only their backing arrays. Their
verified append fast paths also copy raw pointers without AddRef. Their slow
growth callees remain outside this packet, so an all-path borrowed-insertion
claim remains gated on those callees.

## Evidence and ABI

[The audit report](../reports/resource_item_ownership_audit.json) records every
complete code/data span, SHA-256 match, decoded instruction, original ABI,
annotation proposal, and unresolved boundary. Every live query went through
`bsp.py ghidra`, which verifies the `bsp` project and
`/battlestationspacific.exe` program. The installed executable's SHA-256 was
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Address and complete inclusive span | Original ABI and role |
| --- | --- |
| `00b872f0-00b8734e` | ECX primary raw-array triplet; requested capacity stack; RET4 |
| `00b87350-00b873ae` | ECX hierarchy raw-array triplet; requested capacity stack; RET4 |
| `00b87ae0-00b87b1a` | ECX resource; raw hierarchy record pointer stack; RET4 |
| `00718810-0071886f` | ECX concrete resource; no stack arguments; tail JMP to base destructor |
| `00b88430-00b886ab` | ECX base resource; no stack arguments; RET; SEH prologue |
| `00b88180-00b881f2` | ECX hierarchy record; no stack arguments; RET; SEH prologue |
| `00718c20-00718c3d` | Concrete resource scalar deleting destructor; ECX object, flags stack; RET4 |
| `00b88760-00b8877d` | Base resource scalar deleting destructor; ECX object, flags stack; RET4 |
| `00b86990-00b869b3` | Fallback item scalar deleting destructor; ECX object, flags stack; RET4 |
| `00bd30e0-00bd30ed` | ECX object; no stack arguments; RET; existing deleting-destructor dispatcher |
| `0071b8d0-0071b938` | ECX classification list at resource+44h; address of item pointer stack; RET4 |
| `0071b940-0071b9a8` | Same ABI for list at resource+54h |
| `0071b9b0-0071ba18` | Same ABI for list at resource+64h |

Thirteen `_free` calls had stale `CALL_RETURN` flow overrides. Read-only flow
queries confirmed the old value, and Capstone decoded the missing fallthroughs
from full spans that matched the installed PE. In particular, reserve pointer
and capacity writes, destructor continuations, and the deleting wrappers'
original-this return values were absent from the initial pseudocode. The report
lists all thirteen sites; the primary integrator owns Ghidra repair and export
refresh. This worker changed no analysis state.

## Raw growth and hierarchy append

The triplet at resource `+10h/+14h/+18h` is data/count/capacity for primary item
pointers. The triplet at `+1Ch/+20h/+24h` has the same layout for hierarchy
record pointers. Both reserve routines clamp their signed requested capacity
to at least 16, return if current signed capacity suffices, otherwise allocate
four bytes per requested slot and copy existing pointers in order. They free
the old backing allocation and commit the new data pointer and capacity.
Count is unchanged, and unused slots are not initialized. No pointee is copied,
retained, or released by either reserve body.

Hierarchy append `00b87ae0` grows only when count equals capacity, requesting
`max(capacity + 16, 16)` through `00b87350`. It stores the same record pointer
and increments count. This completes the storage boundary established by
[hierarchy parsing](STRUCTURED_RESOURCE_HIERARCHY_BOUNDS.md).

These bodies do not validate count/capacity consistency, reject multiplication
or capacity overflow, or provide transactional allocation failure handling.
Their null tests inspect computed destination slots. A null allocation can
still lead to later stores at addresses 4, 8, and so on when count exceeds one.
The allocator's throw/null guarantees remain external; a checked host container
must describe its own failure policy.

## Concrete destructor dispatch

Disk-matching vtable words establish these paths:

| Object | Vtable | Slot +0 | Slot +4 |
| --- | --- | --- | --- |
| Concrete game resource | `00cfd8cc` | `00bd30e0` | `00718c20` |
| Base resource | `00d63228` | `00bd30e0` | `00b88760` |
| Eight-byte fallback item | `00d631c0` | `00bd30e0` | `00b86990` |

`00bd30e0` calls slot +4 with deleting flag 1 for a nonnull object; it does not
decrement a reference count. Both resource scalar deleting wrappers first call
their field destructor, free the object through `00bf65ac` if flags bit 0 is
set, and return the original object pointer. The fallback wrapper installs
intermediate vtable `00d5c104`, invokes the known reference-counted base
destructor, and conditionally frees the object. It contains no owned payload.

## Primary item release and hierarchy destruction

Base resource destructor `00b88430` first installs `00d63228`, then traverses
the primary pointer array in order. The installed PE import table identifies
IAT `00ce2220` as `KERNEL32.dll!InterlockedDecrement`. The destructor calls it
on each item `+4`, and calls item virtual +0 only when the result is zero.
There is no null check before this decrement. Duplicate pointers incur one
decrement per array entry.

Together with the previously audited no-AddRef append and lack of parser-local
release, this establishes the resource's release obligation. It does not prove
that every registered parser returns a fresh owned reference. A host adapter
can explicitly adopt a caller-supplied owned reference, but must establish that
contract at each actual parser boundary. The earlier allocation-null fallback
path can append a null item; that item is not safe for this native destructor.

After item releases, the destructor passes the resource name at `+8h` to an
external manager operation through `004c1400` and `00b801c0`. Its exact cache or
deregistration meaning remains unproven and cannot be silently omitted from a
native-equivalent teardown.

Next, every nonnull hierarchy pointer goes through `00b88180`. That function
calls `00b7d7d0(record+4Ch, 0)`, frees the raw numeric-reference array data, and
tears down the name when its pointer at `+8h` is nonnull. The latter supplies
the pointer, length at `+4h` plus one, and flag 1 through `00419cc0` and
`00bd1510`. Those native allocation-manager internals remain external.

The enclosing resource destructor then returns each record's slot to its pool.
The 84h-byte user payload has a hidden block index at `+84h`; slot stride is
88h. Under critical section `01090238`, the code uses block table `01090254`,
computes `(record - block) / 88h`, stores the 16-bit slot index into the block's
free-index table at `+4400h`, and increments its 16-bit count at `+4500h`. It
also lowers earliest-available-block index `01090260` when appropriate.
Hierarchy records are therefore destroyed and returned to the pool, rather
than individually passed to CRT free. Null hierarchy pointers are skipped;
duplicate nonnull pointers are not deduplicated and cannot safely represent
distinct owned records.

Finally, the resource destructor frees both pointer-array backing allocations,
tears down its own name through the same manager pair using `+8h/+Ch`, and
calls `00bd30f0` to destroy its reference-counted base. The unusual negative
count/capacity branches remain recorded in the matched disassembly; ordinary
nonnegative array state does not enter their reserve/reset paths.

## Derived classification storage

`00718810` frees data at `+68h`, `+58h`, and `+48h` in that order and zeros each
begin/end/end-capacity pointer triplet. It neither calls methods on list
contents nor decrements their reference counts, then tail-calls `00b88430`.
These derived triplets hold pointers, unlike the base arrays' integer
count/capacity words.

All three append helpers shallow-copy the referenced item pointer when room
exists and advance end by four bytes. Their growth paths call `0071b350`,
`0071b2c0`, or `0071b230`, respectively. Those callees are still named
dependencies, so the proven boundary is non-releasing derived teardown and
non-retaining fast-path insertion. Their type names are not recovered.

This packet supplies matched evidence and proposed names, with no C++ changes,
build, fixtures, native ABI compatibility, exception-unwind equivalence, or
game validation. Registered item parsers, concrete typed-item destructors,
classification growth, manager hooks, native allocation details, and exception
funclets remain external.
