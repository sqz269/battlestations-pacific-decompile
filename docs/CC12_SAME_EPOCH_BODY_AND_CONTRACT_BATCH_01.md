# Same-epoch C04EF3 body and conditional AND/RET contract

Fresh paired reads captured all eight bytes at listing `00C04EF3..00C04EFA`: `83 25 a8 ee 09 01 00 c3`. Eight complete metadata controls agreed at epoch `10371` and live function count `65174`. Root checked all 52 raw HTTP records, 72 CLI source reads, the complete owner record, opening/closing controls and both full count stdios. The snapshot count 65149 is historical. A fresh manual preceded the single detailed x86-32 decoder comparison.

This closes the earlier physical coverage gap, which joined seven bytes from one historical epoch to one byte from another. It establishes a current bracketed Ghidra body under the program-language condition; it does not establish live CPU context, an actual caller or a callable ABI.

The conditional contract preserves instruction order: AND reads and zeros four bytes at DS offset `0109EEA8`, then RET reads its four-byte EIP offset from the resulting SS memory. Full overlap with the return slot zeros the popped offset; partial overlap zeros only the overlapping bytes. Disjointness is sufficient for preservation. Ordinary RAM, stable descriptors/translations, applicable instruction fetches, normal completion and environment/control conditions remain assumptions. Full ESP plus four requires `SS.B=1`; the 16-bit SP case is separate. CPU EIP is a segment offset whose relation to saved listing addresses remains unverified.

The immutable candidate and peer review predate the fresh capture. Original historical manuals and results remain unchanged. Root found a possible default-HTTP redirect path before launch; the sealed original was retained unexecuted and a separately named correction rejects redirects. The corrected capture and decoder each ran once. Inspection/display incidents and historical failures are retained separately and are not relabeled as capture failures.

The two worker packets and their exact reviewed artifacts are bound in `reports/cc12_same_epoch_body_and_contract_batch_01.json`. Its status describes pre-integration authoring; the separately retained final handoff binds the actual commits, push and bounded readback. Root integrates reviewed packets as they close and pushes once for the batch.

No game Source bodies or absence census, C++ edits, builds, tests, probes, runtime actions or GPR writes occurred in this batch. Startup and gameplay remain unvalidated. The overall reconstruction goal remains active, and the earlier Source/build/runtime scope question remains unanswered.
