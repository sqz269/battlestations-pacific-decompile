# CC12 C11C21 entry epoch diagnosis 01

The corrected read-only diagnosis observed a stable live count and snapshot of **64,927** and epoch **5,324**. Both independently requested entry responses were byte-identical. Their complete payloads matched the previously accepted ENTRY after excluding only `modification_number_before` and `modification_number_after`.

Root review and any new baseline approval remain pending. This records entry metadata; no body walk or physical-byte admission follows from it.

| Chronological observation | Live count | Snapshot | Epoch |
| --- | ---: | ---: | --- |
| Previously accepted ENTRY | 64,730 | 64,730 | 98 |
| Original body capture count stop | 64,740 | 64,730 | Not queried |
| Earlier separate entry diagnosis | 64,742 | 64,730 | 1,614 |
| Earlier restarted body count stop | 64,747 | 64,730 | Not queried |
| This packet's first capture, stopped on producer error | 64,927 | 64,927 | Not queried |
| This packet's corrected four-command diagnosis | 64,927 | 64,927 | 5,324 |

Root reported epoch 4,984 for earlier C04 captures. That separate observation is not the epoch of this C11C21 capture, and no common cross-packet live epoch is inferred.

The corrected sequence was count, typed metadata at `00C11C21`, an independent second typed request at the same address, and closing count. It ran from `2026-10-11T18:35:03.242824Z` through `18:35:05.461721Z` inside Root's second bounded observation window. Both original Root window receipts are retained, including the earlier window used by the failed first attempt.

All four direct before/after epoch values and both wrapper epochs were 5,324. Both wrappers' before/after counts and the opening/closing counts were 64,927. Each query verified project `bsp`, program `/battlestationspacific.exe`, x86 little-endian 32-bit language, and image base `00400000`. The complete four first-instruction aliases, one-byte instruction unit, 49-byte owner at `00C11C21..00C11C51`, all owner flags, default/effective flow fields, fallthroughs, target arrays, and automatic companions matched prior metadata. Companions were retained without interpretation.

Every corrected command has separate physical observations of all three frozen CLI files before and after execution: eight observations and 24 reads, all matching. The packet totals five Native commands and ten observations/30 reads when its original successful opening count is included. Every Native command exited 0.

The first capture producer omitted both typed invocations and stopped while trying to open the first absent response file; it made only its opening count query. A subsequent retry-preparation copier attempted to create an already-declared invocation copy twice. Both original failures, their producers, prepins, invocations, full streams, and partial-copy files remain unchanged. The corrected preparation deduplicated its finite destination list before exclusive writes and required identical expected pins for duplicate declarations. The corrected capture added only the two missing typed invocations, plus the directory routing needed for its distinct output namespace. Its full producer and driver were physically reviewed before execution.

Current packet failures remain separate: first capture producer **1**, first retry preparation **1**, inspection command **1**, display truncation **1**, corrected capture **0**, Native command **0**. The corrected inspection's complete original child output is retained; its display truncation is not represented as a command failure. Earlier ENTRY, PRIMARY, and body-stop histories remain separately preserved.

Eight named positive files were frozen from actual Git commit `f2e9f28b9229d69150237075855843aa03e65674`, with 14 actual Git objects and eight matching LF/index entries. Source identities alone were recorded: `src=764245dc8b8865faea6c9c5ef4403e932b80d58d`, `include=5efef0ab7c6b3f0ccb07527b88269ebf4ccf0025`. Older Source identities remain labeled as earlier observations.

There were no new byte/context, prototype/xref/decompile, successor `00C11C22`, or end-cursor `00C11C52` queries; no Source content reads or absence census; no GPR writes, accepted-helper/decoder execution, or new builds/tests/probes/game/OS/audio/runtime experiments. Saved names and flags establish no helper or service contract.

The paired [report](../reports/cc12_c11c21_entry_epoch_rebaseline_01.json) contains both complete original wrappers, complete old/new direct payloads, all command and source observations, separate failures, original window receipts, and finite manifest schemas and pins. Private evidence and `final_handoff.json` reside under `local/cc12_c11c21_entry_epoch_rebaseline_01/`. No further Native followed the corrected capture.
