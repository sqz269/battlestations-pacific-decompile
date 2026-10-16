# Whole preset-tree unique insertion

The complete ordinary `006A1050..006A1163` body is reconstructed directly:
276 bytes/111 instructions. Its established descriptive identity
`STL_Tree_InsertUnique_StrKeyCI_N2C_006a1050` is preserved. Native uses ECX tree,
unconsumed incoming EDX, two stack words (output12B, pair28B), EAX actual output,
and three RET8 epilogues. Source adds borrowed RawCtx, actual feature/locale
references and the existing CRT-parent supplied callees: six words/RET18h.
The new interface is not an Original binary replacement.

The minimal measured result embeds the published real8B iterator, followed by
inserted BYTE8 and untouched9..B. Size12, align4, offsets, trivial copy and standard
layout are asserted. Caller borrows actual result storage; no value initialization,
12B snapshot or padding store. The private8B position is separate from outer12B
output. It initializes node before owner, then is passed to actual predecessor
and used as child output. Original candidate stays separate from selected node.

Capture the original pair before initialhead/root/nil29 reads. Empty root takes
the leftmost insertion route without unique key/header comparison reads. At each
visited node, current pair length is read before retaining that node as candidate.
Empty key compares negative for any nonzero current node length, including high
bits; nonempty key/empty node compares positive. Both nonempty capture node data
before key data, then compare KEY,NODE through actual BF7FBF. Signed negative
selects current left, otherwise current right; test the new node's nil29.

After search the left route compares original candidate to current tree.head.left.
Equality immediately links direction1. Otherwise decrement the actual private8B
position and reload its current node. The final supplied comparator uses selected
node+C and original pair. Successful linking always retains the ORIGINAL candidate
parent, same pair and last direction low byte. Full published FBC0 consumes only
that byte, so no upper private-flag preimage input is needed.

The actual Native calls owned by this whole body are:

| Actual Native instruction | Actual provider/domain |
| --- | --- |
| `006A1096 CALL 00BF7FBF` | Existing CRT parent supplied-callee domain, KEY,NODE |
| `006A10D8 CALL 0069FBC0` | Published raw whole insertion, immediate direction1 |
| `006A10FF CALL 0069AEC0` | Published actual private iterator predecessor |
| `006A110F CALL 00443D00` | Published supplied-cell/callee header comparator |
| `006A1126 CALL 0069FBC0` | Published raw whole insertion, retained low direction |

Both comparator calls receive the SAME borrowed actual0109DE1C and SAME existing
NativeCrtStricmpCallees. Each reached BFparent samples the cell anew. Zero mode
validates inputs, obtains actual errno cell, writes22, calls the five-zero invalid
service and returns INTMAX only after its normal return; valid inputs use its
actual ASCII leaf. Nonzero mode delegates unchanged inputs and null locale to the
provided actual locale service. No default cell/service, CurrentCRT comparator or
ASCII-only replacement. The older two-argument API remains independent.

Both FBC0 calls receive SAME RawCtx/actual109EEA4, original candidate and pair.
Predecessor diagnostics may return, throw or terminate under current SDK policy;
its published current-node/capture schedules are inherited. This function adds no
localEH, guard, cleanup, validation, repair, count/topology write or rollback.
Child/provider exceptions preserve their completed progress. FBC0 retains its
existing CurrentCpp owning length-error transport; Original BF6885 may physically
RET8 if RaiseException returns, a continuation not implemented by Hostthrow.

On each child success, capture child owner, CURRENT outer output, then child node
before any output store. Duplicate captures current output then current private
owner and retains selected node across the comparator. Both publish node4, flag
BYTE8, owner0 and return actual output. Bytes9..B survive even under output alias.

Fresh verified bsp.gpr/program PE/live proof covers all276B/111 starts,16 encoded
edges, five calls and115 current contexts. Both publication gates are resolved at
Main048d7dd5. All46 provider LF files match current Main. Preserved historical
3a5583/10cc72/11f66/f5f readiness, all prior136 artifacts/pins/closeouts and the
FBC0 nine frozen Git blobs/current quartet/17 artifacts/Root23+29 seals/annotations
remain unchanged. Current shared Main ledgers/Core retain legitimate merged appends.

Default MSVC Win32 build exited zero; all three existing suites passed in8.37s.
No new tests/probes or owned Native execution. Full actual CMake object review:
341B/150 instructions across seven CODE sections; public262B110 plus six complete
uncalled helpers79B40. All10 sections/684 declaredB and four code REL32 bindings
were read. There is no EH, BSS, literal, table, data relocation, INT3 or NOP padding.
Compiler factors both successes to one FBC0 call and two physical RET18 epilogues;
Native retains five call sites and three RET8. Actual owner/output/node captures,
node/BYTEflag/owner stores and padding preservation were independently verified.

Source compiler EBP owns its frame. Original candidate uses the dead Source pair
argument home; original pair remains separately captured. Private initial flag/
candidate operations and extra provider argument pushes differ from Native.
Existing supplied CRT/SDK/heap/locale/TLS/errno/feature/DF0/lifetime requirements,
private arguments/frame aliases, flags/registers, fault/partialwrite, Original
throw/FH3SEH/RTTI/binaryABI/runtime/game identity remain unproved. Static/build
checks do not establish Native execution or game equivalence.
