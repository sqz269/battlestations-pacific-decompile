# Native subtree invalidation readiness

`0042ED50` is a viable independent actual-storage Source packet. Its complete
38-byte, 13-instruction body clears two bytes on each reached node and recurses
through actual child/sibling pointers. Its only callee is itself. It needs no
table, token callback, allocator, clock, matrix helper or other service.
This readiness audit changes no Source and awards no Original credit.

The reviewed base is published main `7fcdd0ca5`. All live queries used the
target-verifying `bsp.py ghidra` path for project `bsp`, program
`/battlestationspacific.exe`, x86/32-bit, image base `00400000`; the configured
project is `C:/Users/sqz269/bsp.gpr`. Every byte at `0042ED50..0042ED75` matches
the installed PE. Independent decoding covers all 38 bytes with 13 instructions,
two internal conditional branches, one self-call and plain RET. The live
listing has no gap. Exact Ghidra override metadata is not claimed or changed.

## Complete native schedule and ABI

The native receiver is ECX, with no stack arguments and no defined result.
ESI is saved/restored; the function never reads EDX or touches x87/SSE. Ghidra's
no-parameter prototype and fastcall pseudocode do not replace that assembly
evidence. The complete body is:

```text
0042ED50  PUSH ESI
0042ED51  MOV ESI,[ECX+48h]
0042ED54  TEST ESI,ESI
0042ED56  MOV byte [ECX+C8h],0
0042ED5D  MOV byte [ECX+10Ch],0
0042ED64  JZ 0042ED74
0042ED66  MOV ECX,ESI
0042ED68  CALL 0042ED50
0042ED6D  MOV ESI,[ESI+44h]
0042ED70  TEST ESI,ESI
0042ED72  JNZ 0042ED66
0042ED74  POP ESI
0042ED75  RET
```

First-child capture and its null test precede both stores. The MOV stores leave
the TEST flags intact, so the branch uses that captured child. Both bytes are
always written, including on a leaf or when they are already zero. The first
store is +C8 and the second +10C. Each recursive call receives the captured
child in ECX. Only after that complete recursive subtree returns does the
parent reload the same child's current +44 sibling pointer. It neither saves
next before recursion nor reloads the original receiver's child head.

The complete footprint is a DWORD read at receiver+48, byte writes at
receiver+C8 and receiver+10C, and post-recursion DWORD reads at child+44.
These offsets agree with the current pose/child-offset interfaces and the
already-audited inlined root sequence in `00904600`. +C8 is the existing
pose-valid byte; +10C's broader meaning remains provisional. The last accessed
offset does not establish a complete node allocation size or C++ object type.

## Storage, termination and failure preconditions

The receiver must be actual live storage with those reads/writes valid. Each
nonzero child/sibling pointer must identify another such live receiver; the
current child must remain valid for the parent's +44 read after recursion.
The helper creates no object, owns no node and infers no constructor, table or
allocator domain. It does not read the receiver's table or active byte.

For normal termination, the producer must supply a finite terminating walk.
Native code supplies no visited set, deduplication, depth limit, traversal
repair, cycle break or iterative replacement. Shared descendants are visited
again when reached again; cyclic links retain the native nontermination or
stack-exhaustion risk. No new graph-shape policy belongs in this helper.

A null receiver faults on its initial +48 read; it is not a successful no-op.
The function has no catch, rollback, owning temporary or local EH frame. A
later invalid access can leave the receiver and earlier descendants partially
invalidated. The Source proposal must preserve those prior writes and must not
add a noexcept/termination or recovery policy. General SEH, translated faults,
native exception handling and concurrent mutation are not newly qualified.

## Current Source and minimal proposed packet

The current `EntityLocalMatrixHost::invalidate_subtree_pose` and
`MatrixInterpolatorHost::entity_invalidate_subtree_pose` are abstract host
boundaries. GameWorld's implementation records the site, and other audited
host paths use logging or an empty callback. None is a concrete raw traversal.
Their integer entity/child identifiers must not be cast into pointers here.

Propose exactly these implementation files:

- `include/bsp/native_subtree_invalidation.hpp`
- `src/native_subtree_invalidation.cpp`

The bounded Source signature is:

```cpp
void __fastcall invalidate_native_subtree_pose_0042ed50(void* actual_node);
```

A single-argument MSVC Win32 fastcall entry places the explicit receiver in
ECX and needs no stack cleanup, matching the native normal entry contract
without inventing a class layout. A narrow naked implementation of the exact
13-instruction schedule, with its CALL bound to that same Source function,
can preserve all captures, stores and recursion directly. The header should
guard the Win32/MSVC requirement. No node overlay, allocation, callback class,
process global, current-table dispatcher or owning wrapper is necessary.
38 bytes/13 instructions is the expected body shape, not an emitted-code result;
primary registration/build and complete-object review would follow separately.

This packet qualifies the body/ABI evidence for that proposal. The indexed
`bsp.py lookup 0042ED50` returns a reviewed name but no reconstruction row;
callers with semantic callbacks do not by themselves establish a complete
provider. This audit adds zero function/Original credit. Any future ledger
admission must recheck the canonical current record and avoid duplicate credit
if an existing semantic reconstruction is already counted.

Matrix `00904600` ownership, current mission clock, entity slots +88/+D8,
nonempty matrix records, their allocation/free domain and the complete World
table/lifetime remain external. This helper does not wire or close that pass.
Descriptive names are hypotheses, not recovered original symbols.

The paired report embeds the complete native bytes/listing and current Source
evidence hashes. JSON/integrity and diff checks validate those artifacts. No
Source, CMake, config, ledger or Ghidra state changed; no build, tests, probes,
runtime or gameplay validation ran.
