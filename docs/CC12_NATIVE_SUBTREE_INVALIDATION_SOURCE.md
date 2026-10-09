# Native subtree invalidation Source

The bounded implementation provides the complete instruction schedule of
`0042ED50` through an actual-pointer MSVC Win32 entry:

```cpp
void __fastcall invalidate_native_subtree_pose_0042ed50(void* actual_node);
```

The declaration is in `include/bsp/native_subtree_invalidation.hpp` and the
naked body is in `src/native_subtree_invalidation.cpp`. The header rejects
non-MSVC/non-x86 targets. Its sole CALL names that same Source function.
No registration or caller wiring is included in this packet.

## Evidence and native ABI

The packet starts from published main
`d5f2598c4021c24a77f783cbf0d23966792ae5f2`. The prior
`CC12_NATIVE_SUBTREE_INVALIDATION_READINESS.md` and its paired report contain
the complete target-verified native capture, installed-PE comparison and
contiguous decode. The integrator independently replayed that evidence before
authorizing this Source packet. This worker reuses that evidence; it does not
claim a new live capture, build or execution result.

The captured body occupies `0042ED50..0042ED75`: 38 bytes, 13 instructions,
SHA-256 `9dbf958d6149b06a4ac17be48853600686b35b405dbdc8812d185f78056b539d`.
The receiver arrives in ECX, there are no stack arguments or defined result,
ESI is saved/restored, and return is plain RET. EDX is not an input; the body
has no x87/SSE operation, external callee, global read or vtable dispatch.
Ghidra's no-parameter prototype is not the recovered calling contract.
The name is descriptive, not a recovered original symbol.

## Preserved schedule

| Native site | Source instruction or effect |
| --- | --- |
| `0042ED50` | PUSH ESI |
| `0042ED51` | Capture DWORD `[ECX+48h]` into ESI |
| `0042ED54` | TEST that captured child |
| `0042ED56` | Clear byte `[ECX+C8h]` |
| `0042ED5D` | Clear byte `[ECX+10Ch]` |
| `0042ED64` | JZ to epilogue using the earlier TEST flags |
| `0042ED66` | Place captured child in ECX |
| `0042ED68` | CALL the same Source function |
| `0042ED6D` | Reload DWORD `[ESI+44h]` after recursive return |
| `0042ED70` | TEST that current sibling |
| `0042ED72` | JNZ to the child call sequence |
| `0042ED74` | POP ESI |
| `0042ED75` | RET |

Both clear stores execute even for leaves and already-zero flags. Their MOV
instructions preserve the earlier TEST flags. The recursive call completes
the child's subtree before the parent reads that same child's current sibling
link; there is no early next-pointer copy or original child-head reload.

The actual footprint is receiver+48 DWORD read, receiver+C8 and +10C byte
writes, and child+44 DWORD read after recursion. +C8 is the current pose-valid
byte; the broader meaning of +10C remains provisional. These offsets do not
establish a full node type, allocation size or ownership/lifetime model.

## Validity, termination and failure boundary

The receiver and every nonzero reached child/sibling pointer must identify
live storage supporting the observed accesses. Each child must remain valid
for the parent's +44 read after the recursive return. Normal termination
requires a finite terminating walk. Shared descendants are visited again;
cycles retain native nontermination or stack-exhaustion behavior. This helper
does not allocate, construct, retain or free nodes, and introduces no guard,
visited set, depth limit, traversal repair or iterative replacement.

A null receiver faults at the first +48 read. Invalid storage reached later
can leave earlier byte stores visible. There are no owning temporaries, local
EH frame, catches, rollback or `noexcept` declaration. No new fault recovery,
SEH, translated-exception or concurrent-mutation guarantee is claimed.

## Validation and admission boundary

Worker checks cover JSON parsing, prior evidence/source hash revalidation,
embedded native-byte/listing integrity, the Source schedule correspondence
and `git diff --check`. No compilation, test, execution probe, Ghidra change,
ledger change, CMake registration, runtime or gameplay validation occurred.
Only the implementation pair and this dedicated document/report were added.

The 38-byte/13-instruction emitted shape is an expectation pending integrator
registration/build and independent whole-object inspection, including the
CALL relocation resolving to this same function. Native evidence and the
written assembly do not by themselves prove the compiler's emitted object.
The integrator must separately inspect the current canonical reconstruction
record before admitting any function credit. Worker Original credit is zero.

This helper does not close `00904600`'s current clock, entity slots +88/+D8,
actual nonempty matrix-list producer, allocation/free domain, World
construction/table/lifetime, or application callers. Existing semantic or
token callbacks are unchanged; this packet introduces no adapters for them.

## Primary registration and emitted-body review

The integrator registered the source in bsp_core and ran the normal MSVC Win32 build. All three existing checks passed. Complete object inspection found exactly 38 bytes and 13 instructions. The only relocation is I386_REL32 at operand offset 25, targeting the same physical function symbol at section offset 0. Resolving that self-call produces the entire Original body byte-for-byte; no external callee, frame or added instruction is present.

The current canonical shard contained no function or fragment record at 0042ED50; the integrator admits one complete reconstructed function with 38 Original bytes. Static assembly and emitted code qualify its ECX receiver, absence of stack arguments and plain RET. The root is absent from the game map. Native entry execution, fault/SEH behavior, actual hierarchy production, complete World behavior and gameplay remain unvalidated.

Evidence: [cc12_sample_subtree_primary_review.json](../reports/cc12_sample_subtree_primary_review.json).
