# Actual fixed-nil21 controller-tree leaves

Source: `src/native_input_controller_tree_leaves.cpp` and
`include/bsp/native_input_controller_tree_leaves.hpp`. These descriptive C++ names
are hypotheses. Established STL identities and their earlier evidence remain.

| Entry | Preserved library identity | Coverage |
| --- | --- | --- |
| `0069AA80` | `STL_Tree_Min_0069aa80` | complete: `0069AA80..0069AA9A`, 27 bytes/10 starts, including native LEA |
| `0069A6A0` | `STL_Tree_Rotate_0069a6a0` | complete: `0069A6A0..0069A6F1`, 82 bytes/30 starts; right rotation |
| `0069AA60` | `STL_Tree_Max_0069aa60` | complete: `0069AA60..0069AA6C` and `0069AA70..0069AA7B`, 25 defined bytes/10 starts |
| `0069AD50` | `STL_TreeIter_Inc_0069ad50` | complete: `0069AD50..0069ADB2`, 99 bytes/38 starts |
| `0069C280` | `STL_Tree_Lrotate_Unk_N24_0069c280` | complete: `0069C280..0069C2CD`, 78 bytes/30 starts; left rotation |

Total owned coverage is 311 defined bytes/118 current instruction starts. Maximum's
three bytes between its two defined ranges remain undefined and outside that
count; native control skips them. Their raw bytes are separately verified for envelope matching.
No Ghidra repair or mutation was performed.

## Actual storage and ABI

The five functions consume raw DWORD node links at `0/4/8`, nil BYTE `21h`, tree
head DWORD `4`, and an actual eight-byte iterator owner/node pair. They do not
consume color, payload, pool context, count, allocator or a typed node overlay.
The literal nil displacement is established by every reached native byte test,
not inferred from an instantiation label. Old nil19/29/15/99 and typed/AV Source
interfaces retain their separate domains.

Native ECX supplies tree, subtree or iterator; incoming EDX is unconsumed.
Rotations take one stacked node and use `RET 4` on all three paths. Successor and
extrema have no stacked arguments and use plain `RET`, with successor's nil path
tail-delivering the diagnostic. Extrema return the selected actual node in EAX.
New Source returns void for rotations/successor, excluding incidental native EAX,
flags and register residue. Calling-shape inspection is not original binary ABI
equivalence.

## Ordered contracts

Successor first checks owner DWORD zero and invokes the accepted diagnostic if
needed. That service may return; read CURRENT iterator.node afterward. A nil node
reports and exits. A nonnil right subtree follows progressive left links and nil
tests, then publishes the selected node. Otherwise capture the parent; each climb
reloads CURRENT iterator.node before the CURRENT parent.right comparison. Publish
a matched parent into iterator.node before reading its CURRENT parent link. Keep
the final parent store on nil or mismatch. No cached iterator-node snapshot or
additional validation is supplied.

Right rotation captures the initial left pivot, stores the pivot's current right
child into node.left, then REREADS pivot.right after that store. Check this current
child's nil byte before its parent update. Publish pivot.parent from CURRENT
node.parent; then capture CURRENT tree.head once for the root comparison. On a
nonroot path reload CURRENT node.parent and test parent.right first. Publish root
or parent side before the ordered final pivot.right=node and node.parent=pivot.
Left rotation mirrors this with initial right pivot/current pivot.left and a
parent.left first test. Public aliases retain these load/store sequences.

Extrema capture the initial subtree, repeatedly read its current left or right
child and that child's nil byte, and return the last selected current node. There
is no new initial-node nil/null check, endpoint guard or ownership operation.

All public accesses are volatile raw DWORD/BYTE operations. These are fixed
literal-nil21 bodies derived from the accepted nil19 Source; they do not delegate
to a different layout, accept a nil parameter, or use callbacks/container models.
No extra guard, rollback, owning cleanup, local catch or noexcept bridge is added.

## Exact Native and Source evidence

Published Source reference pin is `676bce181ecc723b2bb9965a963a5f9f747b443f`.
Read-only native references are min `0069AAE0`, right `0069A780`, max `0069AAC0`,
successor `0069AE50`, and left `0069C300`. All five complete envelopes compare equal
after only operand-proved nil BYTE `21h` to `19h` edits and independently resolved
same-target diagnostic rel32 normalization. No other field, register, branch,
store-order or instruction-byte difference is masked. Reference code is another
311 defined bytes/118 starts; the two maximum gaps contribute six separate raw
bytes, without becoming defined instructions. All 236 starts match current Ghidra.

| Scope | Containing function | Site | Actual target |
| --- | --- | --- | --- |
| owned | `0069AD50` | `0069AD58` | `00BF6713` |
| owned | `0069AD50` | `0069AD67` | `00BF6713` |
| read-only reference | `0069AE50` | `0069AE58` | `00BF6713` |
| read-only reference | `0069AE50` | `0069AE67` | `00BF6713` |

```text
0069AD58 CALL 00BF6713
0069AD67 JMP 00BF6713
0069AE58 CALL 00BF6713
0069AE67 JMP 00BF6713
```

These four canonical transfer rows are the complete host table. Loop iterations
and private Source helpers do not invent additional native host calls. The only
external Source provider is the accepted current-SDK diagnostic entry.

Ghidra project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, was
verified before live batches. The report freezes the accepted Main baseline,
Source/provider hashes, full PE/live bytes, typed ranges/current starts, and the
ignored evidence paths. The separate frozen readiness receipt retains controller
owner/pair gaps; neither is reconstructed by this packet. Earlier populated AV
providers and fixtures remain unchanged.

## Build, emitted object and limits

Default `./scripts/build.ps1` exited 0. Existing suites passed 3/3 in 8.18 seconds;
no new test or standalone probe was added. Those suites do not execute these leaves.
The existing unrelated duplicate-symbol LNK4006 warning remains in the log.

The actual CMake I386 object contains 394 code bytes/168 instructions: five public
functions total 332/136 and five helpers total 62/32. All code, helpers, physical
returns, alignment NOPs and relocations were reviewed. There is no terminal INT3
padding or local C++ EH. Two code REL32 relocations bind the actual diagnostic
Source adapter; three type7 debug-frame relocations are separate metadata. All ten
code sections equal the actual accepted nil19 reference object's sections after
only 13 proved nil-displacement changes. Sixteen focused COFF checks pass.

The current SDK policy does not reproduce native encoded handler/debugger/Watson
selection. All reached storage must be valid and walks finite. Source private
arguments, frames, registers and flags differ; asynchronous mutation and original
private-stack aliases are excluded. Original heap/CRT/FH3/RTTI/SEH/fault identity,
native binary ABI, application adoption and game behavior remain unproved.
