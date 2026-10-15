# Actual input scalar-map full cleanup

This packet reconstructs only the full-selection prefix00444B10..00444B8E
(127bytes/49instructions) and whole00445900..00445933 (52bytes/23instructions).
Descriptive names are hypotheses. Existing actual soldier-loop allcase APIs and
their explicit partial-path callbacks remain preserved; their algorithms exist.
The new prefix directly uses accepted actual444760 subtree Source, whose
provider-free raw pooled-string binding needs no fabricated diagnostic callback.

The native prefix takes ECX tree, unusedEDX and stacked output/first-owner/
first-node/last-owner/last-node, returns EAX output and RET14h. The admitted
storage has first_owner==last_owner==nonnull tree, first_node==initial head.left,
last_node==fresh head, with selection storage exclusively stable. Its TEST/CMP
and head-load schedule is retained. Out-of-scope diagnostic/partial outcomes
are preconditions, not replacement guards or a claim of a complete ordinary
range eraser. Output is the writable private8B iterator used by the consumer.

After actual444760 subtree completion, reload CURRENT head and set parent=self;
reload head BEFORE tree.count=0 and left=self; reload for right=self; reload
head.left before output.owner=tree then output.node. No result reread after its
first publication. No empty-root shortcut is added. ActualN1Ch nodes have nil19h
and pooled key at0C/10. Child traverses right first, captures key data then left,
reads length only when data is nonnull, obtains its actual pool, releases key,
then frees node. RawPoolContext getters may throw; earlier effects persist and
later resets/output/free/clear stop. No noexcept adapter/callback/rollback/retry.

Whole445900 has nativeECX actual12B tree, unconsumedEDX, no stack inputs, plain
RET and DEFINED EAX0 despite pseudocode void. It captures head thenhead.left,
selects that prefix with an actual private8B output, reloads CURRENT head for
free, zeroes+4 then+8, preserves opaque+0, and returnsDWORD0. No null or repeated
destruction guard is added. It has no native local EH/catch; caller FH3 is
separate. New APIs add the actual raw-string service context explicitly.

Fresh typed metadata verifies saved bsp project/program before the batch.
All179 owned bytes/72starts, child82B/34starts and two accepted references
agree with installedPE/live bytes/export/currentstarts. Total440B/178starts;
15 current CALL sites also match installedPE. Full prefix and destructor
comparisons differ only at their three/two proven call-displacement fields.
The parent member+24 consumer is evidence only, not parent Source promotion.

| Site | Target | Evidence role |
| --- | --- | --- |
| 00444b2a | 00bf6713 | Owned CALL in00444b10 |
| 00444b46 | 00bf6713 | Owned CALL in00444b10 |
| 00444b5a | 00444760 | Owned CALL in00444b10 |
| 00445916 | 00444b10 | Owned CALL in00445900 |
| 0044591f | 00bf65ac | Owned CALL in00445900 |
| 00444777 | 00444760 | Supporting CALL in00444760 |
| 0044478f | 00419cc0 | Supporting CALL in00444760 |
| 00444796 | 00bd1510 | Supporting CALL in00444760 |
| 0044479c | 00bf65ac | Supporting CALL in00444760 |
| 0055730a | 00bf6713 | Supporting CALL in005572f0 |
| 00557326 | 00bf6713 | Supporting CALL in005572f0 |
| 0055733a | 00554f10 | Supporting CALL in005572f0 |
| 0055b4a6 | 0055b230 | Supporting CALL in0055b490 |
| 0055b4af | 00bf65ac | Supporting CALL in0055b490 |
| 0055b7f1 | 00444b10 | Supporting CALL in0055b690 |

Validation is recorded in reports/oct15_input_scalar_map_full_cleanup.json.
The existing Win32 suites are required; no new tests/native invocation are
claimed. Added C++ context/private argument/result frames, register and flag
residue, originalCRT/heap/FH3/RTTI/SEH/fault/stack exhaustion/binaryABI and game
equivalence remain unproved. These are Source interfaces, not drop-in binaries.
The full ordinary444B10 partial eraser remains a separate qualified API.
