# Actual default scalar-tree range erasure

Addresses: `0069FE70` only. The retained library identity is
`STL_Tree_EraseRange_IntKey_N18B_0069fe70`; the new Source name is provisional.

| Entry | Coverage | Physical native interface | Source interface |
| --- | --- | --- | --- |
| `0069FE70` | Complete: 200 defined bytes / 76 instructions | ECX tree; EDX unused; five stack words output, first owner/node, last owner/node; RET14h; EAX output | Same seven-formal fastcall shape, `void*` result |

The body has ranges `0069FE70..0069FEEE` and `0069FEF0..0069FF38`; the single
`90` at `0069FEEF` is outside the typed body. The entire 201-byte envelope matches
the accepted scalar reference after only six independently decoded E8 operands.
All other instructions, branch displacements, accesses, output order and the
excluded NOP remain unmasked. Source directly composes the accepted whole
service declared in `native_input_scalar_tree_range.hpp`; no loop is duplicated.
The full and partial native paths are both covered.

```text
0069FE8A CALL 00BF6713
0069FEA6 CALL 00BF6713
0069FEBA CALL 0069D3F0
0069FEFA CALL 00BF6713
0069FF09 CALL 0069C370
0069FF17 CALL 0069F690
0055730A CALL 00BF6713
00557326 CALL 00BF6713
0055733A CALL 00554F10
0055737A CALL 00BF6713
00557389 CALL 005531F0
00557397 CALL 00556DC0
```

The initial first owner is captured before reading head.left. That bound survives
a returning first-owner diagnostic; first node is read afterward. If it matches,
the current head is captured BEFORE the last-owner diagnostic and retained for
the last-node comparison. No fresh-bound repair or extra validation is added.

The full path captures a fresh root, destroys its subtree, then reloads current
heads in root-reset, count-zero, left-reset and right-reset order. It captures the
final head.left result before writing output owner then node. It does not free
the head. The partial path checks captured first owner against current last
owner, compares captured first node against current last node, increments the
ACTUAL Source first-argument iterator slot, then erases a separate OLD captured
iterator with a private eight-byte output. That output is ignored. It reloads
first node before first owner and continues. Final output values are captured
before ordered writes, preserving the accepted service's external raw aliases.

Child closure is established from complete bodies, not library names. Iterator
99-byte pairs differ only in the two verified diagnostic CALL/tail-JMP operands.
The entire 679-byte / 231-instruction eraser pair matches after 13 proved CALL
operands and its own private handler immediate. Both rotations and both extrema
pairs have identical unmasked envelopes. Max has 25 typed bytes in a 28-byte
envelope with a skipped three-byte alignment LEA; those bytes are kept separate
from body coverage. The complete assembly includes the two-child transplant
path discarded by the current decompiler output.

The eraser's full raw EH proof includes handler, 36-byte FuncInfo, unwind record
and eight-byte action: one state, no try blocks, state0 to -1, the same completed
message slot and actual legacy string destruction. The exact 27-byte message,
owning 28h error profile, vtable and throw metadata match. Assignment precedes
cleanup arming. The accepted owning C++ transport retains the actual legacy
construction/copy/destruction services. No generic exception, callback or fake
public EBP action is supplied. The range itself has no local catch or rollback.

Actual header fields are opaque0/head4/count8; nodes have links0/4/8, unowned
payloadC/10 and color14/nil15. No new node overlay, pool/context or payload
destructor is needed. The existing AV fragment and host/table/prefix interfaces
remain unchanged. Root's separately owned clear and `006AA460` are excluded.

Reached storage must support the accesses, including continuation after returning
diagnostics. Current CRT ownership and SDK validation apply. Whole delegation
changes Source argument/private frames and child/recursive origins. Original
spill aliases, volatile-register residue, FH3/RTTI/SEH/fault/stack-exhaustion,
heap identity, binary ABI and game behavior remain unproved. A matching physical
signature does not recover the original C++ declaration or prove a drop-in ABI.

Fresh native/provider evidence, emitted-object inspection, build results and
limits are recorded in `reports/oct15_native_input_default_scalar_tree_range.json`.
The default Win32 build passed all three existing suites in 13.71 seconds. The
complete actual CMake wrapper is nine bytes / four instructions, with a balanced
compiler frame and sole verified REL32 tail delegation to the accepted service.
Matching decorated signatures and unchanged registers/argument slots preserve
the Source output/RET14h contract; all six focused object checks pass. No new
tests are introduced; existing suites do not constitute a new native range-
erasure fixture or game validation.
