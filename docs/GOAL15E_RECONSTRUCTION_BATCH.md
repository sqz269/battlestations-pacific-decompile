# GOAL15E avoidance ownership and lifetime reconstruction

This batch supplies seven original entry paths over the existing raw avoidance
storage: sentinel allocation and list construction, sorted group creation,
standalone hull-list clearing, and manager construction, destruction and scalar
deletion. It composes the previously accepted full manager/group/cache cleanup
with genuine current allocation, hull release and Windows critical-section
providers.

The earlier semantic and typed reconstructions remain separate historical
records. Descriptive names remain hypotheses rather than recovered symbols.

## Owned entries

| Entry | Source module | Contract |
| --- | --- | --- |
| 0041BF50 | native_hull_list_head.cpp | Allocate 10h and self-link the two pointer words; leave payload words untouched. |
| 0041E1B0 | native_hull_list_head.cpp | Construct the raw 0Ch list header without changing its proxy word. |
| 00417CA0 | native_avoid_group_factory.cpp | Find the first exact signed key or allocate and insert a matching Source group in signed order. |
| 0041E200 | native_avoid_manager_lifetime.cpp | Clear hull nodes and data while retaining the sentinel and proxy. |
| 00421500 | native_avoid_manager_lifetime.cpp | Construct the raw 78h manager, sentinel and actual tracked critical section. |
| 0041F980 | native_avoid_manager_lifetime.cpp | Full normal destruction plus recovered current list/vector/base unwind operations. |
| 00420680 | native_avoid_manager_lifetime.cpp | Destroy and conditionally release the receiver; retain original low-bit flag test and RET4. |

## Integration verification

The registered MSVC Win32 build and all three existing CTests passed. After
merging current main, the incremental build and tests passed again; both probes
were relinked against that final registered archive and passed.

The lifecycle probe uses genuine current heap objects, a real Windows tracked
critical section and the actual Source deleting slots. It covers signed group
insertion and repeated-key identity, populated standalone hull-list clearing,
populated group cleanup, body-release queuing, recreation after clearing,
retained flag-0 manager destruction and flag-1 manager release. It checks
unchanged proxy words, retained queue storage, bounded heap releases and
unconditional clearing of a publication cell holding another pointer. The small
sentinel probe checks self-links, header/result/count and matching heap release
without reading uninitialized sentinel payload words.

Five complete raw kernels match all 325 Native bytes after exactly seven
verified CALL operand redirections and the factory's Source-vtable operand.
The Native constructor/destructor main bodies are 171/145 bytes; the current
Source main bodies are 208/230 bytes. Their current emitted EH/provider/guard
paths and cleanup phases were reviewed; these bodies were not byte-equated.
Primary and independent peer reviews found no correctness blockers.

All 17 direct-call report rows and seven parsed documentation claims passed.
The factory documentation also has three unparsed table/historical-correction
claims; its allocator instruction and both actual direct callers were checked
explicitly. Seven Original ABI prototypes and evidence comments were saved in
the existing Ghidra project with prior values recorded. Eleven owned/dependent
exports and the function/callgraph snapshot were refreshed.

The reconstruction ledger change adds seven records and removes none, retaining
the earlier typed/projection records. See
[the machine-readable receipt](../reports/goal15e_reconstruction_batch.json)
and the three bounded worker reports for artifact hashes, exact commands and
the tested Source revision.

## Evidence boundaries

The group factory installs the real Source group deleting slot accepted in
GOAL15D. The manager derived table likewise contains a real Source deleting
entry, but its typed caller must supply the explicit publication-cell context
in EDX. The Native `CALL EDX` dispatch is not compatible with that added context.
The final CE3818 base word is retained as a numeric Native profile marker on a
destructed receiver; it is not advertised as a callable Source base table.

Current C++ unwind guards preserve the recovered cleanup phase operations.
They do not transplant or establish equivalence of the original FH3/SEH frame,
private CRT, allocator, exception identity, or double-exception behavior.
Build and Source heap fixtures are distinct from original runtime ABI and
gameplay validation. No production installation or live game validation is
claimed by this batch.
