# Classification growth and resource-name erase

Addresses: 00443d00, 00718350, 00718380, 007183b0, 007188c0, 007188f0, 00718920, 0071a5a0, 0071a760, 0071a920, 0071b230, 0071b2c0, 0071b350, 00b7dfa0, 00b7e7b0, 00b7fa60, 00b801c0.

Normal classification append borrows item pointers on both the fast and growth
paths. Growth copies the existing pointer bytes, writes the new pointer value,
and frees only the old backing allocation. The resource's name-erase callback
removes at most one equivalent name from its tree; erased-node cleanup frees
the key and node storage without releasing the mapped resource value.

[The audit report](../reports/resource_classification_cache_erase_audit.json)
contains 17 complete function-body matches, original ABIs, old/proposed names,
full decoded instructions, and the matched invalid-iterator literal. All live
reads used the verifying `bsp.py ghidra` CLI against `bsp` and
`/battlestationspacific.exe`. Every listed span matched the installed PE with
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
This packet supplements the unchanged
[resource ownership audit](RESOURCE_ITEM_OWNERSHIP.md).

## Classification append route

| List offset | Outer append, previously audited | Insert-one wrapper | Growth body | Raw copy / fill |
| --- | --- | --- | --- | --- |
| Resource+44h | `0071b8d0` | `0071b230` | `0071a5a0` | `00718350` / `007188c0` |
| Resource+54h | `0071b940` | `0071b2c0` | `0071a760` | `00718380` / `007188f0` |
| Resource+64h | `0071b9b0` | `0071b350` | `0071a920` | `007183b0` / `00718920` |

Each list has begin/end/end-capacity pointers at relative +4/+8/+Ch. The
insert-one wrapper receives ECX list and four stack arguments: output iterator,
input iterator owner, insertion position, and address of the pointer value. It
passes count 1 to the growth body and returns an owner/position iterator in the
output storage, with EAX pointing there and RET16. Normal outer append supplies
the list's end as the insertion position. Iterator-owner/range checks call
`00bf6713` on inconsistency.

The growth bodies have ECX list, then iterator owner, position, unsigned count,
and value-address on the stack, with RET16. They snapshot the pointer value
before allocating, so a readable value-address within the old array is not
used after that allocation is freed. They enforce a maximum required element
count of `3FFFFFFFh` through their existing typed length-error helper. New
capacity is the larger of required size and old capacity plus half the old
capacity, rounded down. If that growth candidate exceeds the maximum, it is
discarded and required size is used. An empty list therefore first grows to
capacity 1 on this append route.

After allocation, the body copies the prefix before the insertion position,
fills the new slot, and copies the suffix. The suffix is empty for normal
append. Each copy helper takes source begin/end and destination on the stack,
uses known CRT `memmove_s` at `00bf67a7`, ignores its status, and returns the
computed destination end with RET12. Each fill helper uses direct four-byte
loads/stores and RET12. Neither helper reads the pointed item or calls any
item method. The enclosing body then frees nonnull old backing storage and
commits the new begin/end/end-capacity values.

Order and duplicate pointer values survive. Null pointer **values** are also
copied unchanged by these helpers, although the previously audited outer
classifier skips null items. A null value-**address** is invalid: the growth
body dereferences it before checking the insertion count. There is no
deduplication, item reference increment, release, or ownership transfer in
these item-copy operations. Combined with the prior derived destructor's
backing-array-only cleanup, the normal classification lists are borrowed
aliases of the primary items.

This contract requires well-formed, aligned pointer ranges, successful native
allocation, and successful CRT copying. Allocators `00714120`, `00714180`, and
`007141e0` receive capacity in ECX and zero in EDX; their internals remain
external. Generic middle/multiple-insertion branches are included in matched
code but their separate shifting helpers are outside this append audit.

## Name lookup and one-node erase

`00b801c0` receives ECX manager and one native-name pointer on the stack, with
RET4. The tree begins at manager+14h, its sentinel pointer is at manager+18h,
and its size is at manager+1Ch. It calls `00b7e7b0` to find a name-equivalent
iterator, validates the returned owner, and calls `00b7fa60` only when the node
is not the sentinel. A missing name is a no-op. There is no removal loop or
mapped-resource pointer argument/comparison.

Find uses lower-bound `00b7dfa0` and comparator `00443d00` to reject a greater
candidate. A native name is a length/data pair. Length zero is the empty marker
regardless of the data pointer, and empty sorts before nonempty. Two nonempty
names compare through CRT `stricmp` at `00bf7fbf`; stored length does not bound
the comparison. The comparator returns its Boolean in AL with RET8; upper EAX
bits are not normalized. Null name-pair pointers and nonempty names with null
or invalid data lie outside valid input. Empty names can have null data.

Removal is solely by name equivalence. It does not verify that the found value
is the particular resource whose destructor issued the callback. If equivalent
duplicate keys exist outside the assumed map invariant, only the found node is
targeted. The insertion code and its duplicate-key invariant are not audited.

`00b7fa60` is a tree iterator erase operation, despite its prior name
`STL_xlen_throw_00b7fa60`. Its complete `00b7fa60-00b7fd2b` body has a normal
RET12 path. A sentinel iterator enters the separate
`invalid map/set<T> iterator` exception branch. On the normal path, it obtains
the successor, relinks the tree, and applies red-black repair. The two-child
case relinks the successor node and swaps colors; it does not copy the mapped
payload into the erased node. Successor/minimum/maximum/rotation helper
internals remain external.

Node links occupy +0/+4/+8, key length/data occupy +Ch/+10h, and color/sentinel
bytes occupy +18h/+19h. The matched erase body never reads or destroys the
mapped slot at +14h. Its explicit cleanup passes the erased key through
`00419cc0`/`00bd1510`, frees the erased node through `00bf65ac`, decrements a
nonzero tree size, and returns the successor owner/node pair through its output
iterator. No item AddRef, release, interlocked decrement, or item virtual call
appears. This establishes non-releasing erased-node cleanup; it does not prove
that every cache insertion is non-retaining.

## Analysis repairs and limits

Read-only flow queries confirmed stale `CALL_RETURN` overrides at `0071a69c`,
`0071a85c`, `0071aa1c`, and `00b7fced`. Complete matched bytes include the
fallthroughs that commit vector pointers and finalize tree erase. The report
records exact complete bounds for primary repair. Ghidra also omitted the
real tree two-child branch from pseudocode; the assembly passes a stack
iterator to the successor helper and subsequently reloads its node before
branching there. The audit retains that machine-code branch.

Suggested names and comments are in the report; this worker changed no Ghidra
state. The primary-owned manager accessor `004c1400`, native allocators, string
allocation-manager internals, tree structural helpers, insertion ownership,
and exception internals remain external. No C++ implementation, build, fixture,
native ABI compatibility, or game validation is claimed.
