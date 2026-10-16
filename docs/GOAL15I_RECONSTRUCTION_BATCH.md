# GOAL15I raw avoidance providers

This batch adds seven complete raw Source entries in three registered modules. It also records complete physical analysis of the constructor and group-add parents; those two parents are not newly implemented. The full clipping function remains a separate in-progress packet.

| Original entry | Source behavior | Physical bytes |
| --- | --- | ---: |
| `00415010` | Alias-sensitive bounds inclusion using the actual incoming argument cell | 127 |
| `004167C0` | Free current counted corner records; retain array and metadata | 56 |
| `00412EF0` | Free the pointer currently stored in the actual cell, without reset | 14 |
| `00412DC0` | Free the captured owner pointer and clear only its pointer field | 26 |
| `0041A200` | Complete x87 geometry, winding and conditional recursive reversal | 729 |
| `00419260` | Complete x87 reciprocal-length schedule and spills | 117 |
| `00438B10` | Complete x87 wrapped-angle subtraction and repeated wraps | 110 |

The 1,179 physical bytes comprise 1,173 admitted body bytes and six skipped padding bytes. COFF comparisons account separately for 1,095 fixed bytes, eleven actual call operands and ten immutable-data operands. Every relocated operand must have its actual Source symbol or exact read-only payload verified; these operands are not arbitrary masks.

The bounds helper preserves the caller's reused point argument word and reloads the aliased point before each comparison. Cleanup preserves the original count reloads, current-cell ownership and asymmetric clearing. The winding routines retain the actual x87 instructions, float spills, low-byte flag test, signed reversal and outer return value. Their actual current UCRT `_CIatan2` and `_CIsqrt` services consume hidden x87 operands directly. No callback or ordinary double bridge substitutes for those calls.

The Native private CRT bodies, dispatch/state/exception policy, binary replacement ABI and installed game behavior are not established by this Source boundary. The math fixture compares owned instructions against complete captured bodies rebound to the same current CRT services. It does not execute the installed private CRT.

The registered MSVC Win32 build, all three existing CTests and three archive-only fixtures passed. All 48 report call relationships and 16 explicit cleanup-document call claims passed, with no failures, unchecked claims or warnings. Nine physical Original prototypes were corrected with old values retained, the project was saved, 67 affected function/direct-caller exports were refreshed, and the snapshot/index were rebuilt. Independent reviews found no Source discrepancy; the constructor documentation's world-field order was corrected.

Parent analysis records actual input-cell reuse, late world-pointer value loading, interleaved world-field loads, all allocation branches and the one-state constructor/group cleanup maps. A future constructor binding must borrow the actual world pointer-cell address and load its value at the observed late point. No default world, copied early bounds, fabricated virtual target, new cleanup guard or completed parent adoption is implied.

See [the batch receipt](../reports/goal15i_reconstruction_batch.json), [bounds evidence](GOAL15I_AVOID_BOUNDS_INCLUDE.md), [cleanup evidence](GOAL15I_AVOID_CLIP_CLEANUP.md), [winding evidence](GOAL15I_AVOID_ZONE_WINDING.md), and [parent readiness](GOAL15I_AVOID_CONSTRUCTION_READINESS.md). Final registered build, archive-only fixture, annotation and export results are recorded in the receipt.
