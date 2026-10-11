# CC12 C11C21 body physical observation 01

The saved 49-byte owner `00C11C21..00C11C51` was captured with two independent 20-unit metadata walks and two exact byte reads. The controlled capture observed live/snapshot count **65,052** and epoch **7,524** throughout 48 Native commands. This is a finite observation, not a permanent baseline or physical admission.

The complete manual interpretation was written and pinned before the first fresh detailed Capstone import. That one decoder execution matched all 20 units and selected CPU/control fields, exited 0, and retained its complete actual detail payload before comparison. No Native rerun or extra decode followed.

All 20 physical boundaries agree with the four saved instruction/code-unit aliases. Default/effective flow categories and fallthroughs agree with the applicable instruction/control category; every flow override is `NONE` and every fallthrough override is false. The full per-unit comparisons, owner flags and companion records are in the report.

| Physical unit | Encoded control fact | Saved default/effective metadata |
| --- | --- | --- |
| `00C11C32` | JNE to `00C11C47`; next offset `00C11C34` | Conditional jump; both target arrays and fallthroughs agree |
| `00C11C35` | CALL offset `00C11B5E`; return-address offset `00C11C3A` | Unconditional call; saved companion `LIBCRT_unmatched_00c11b5e`, `no_return=false` |
| `00C11C3D` | JNE to `00C11C47`; next offset `00C11C3F` | Conditional jump; both target arrays and fallthroughs agree |
| `00C11C41` | CALL offset `00BFBA09`; return-address offset `00C11C46` | Unconditional call; saved companion `__amsg_exit`, `no_return=false` |
| `00C11C49` | Indirect CALL through memory operand `[00CE2218]`; return-address offset `00C11C4F` | Computed call; default targets empty, effective target `EXTERNAL:0000015b`, saved companion `EnterCriticalSection` |
| `00C11C51` | Near RET; dynamic stack destination, no direct fallthrough | `TERMINATOR`, null fallthroughs, empty targets; saved `is_computed=false` is an analysis category |

The indirect memory operand does not authenticate the effective external association or provider. Encoded CALL return-address facts are separate from saved no-return flags and fallthroughs; neither establishes callee effects, return behavior, ABI or service contracts. The owner name `__lock` and companion names remain saved metadata identities.

Interpretation is conditional on default x86 32-bit code mode. Every full-ESP push/pop/CALL/RET effect additionally requires **SS.B=1 and normal completion**; code mode alone does not establish SS.B. Encoded/effective offsets do not establish live linear addresses: applicable CS/DS/SS bases and mappings remain unqueried. `[EBP+8]` and stack accesses use the applicable stack segment; `[ESI]` and `[00CE2218]` use the applicable data segment. LEA computes an offset without a memory read or segment-base addition. No fault behavior, per-address CPU context, context-verified physical admission, ABI compatibility or runtime/gameplay behavior is established.

The unchanged original manual and separate stack qualification are both retained. Publication adds the segment-base clarification without rewriting those originals or their canonical rows.

Earlier observations remain separate: accepted ENTRY 64,730/epoch 98; original BODY stop 64,740/epoch unqueried; earlier ENTRY diagnosis 64,742/epoch 1,614; restarted BODY stop 64,747/epoch unqueried; rebaseline first producer failure 64,927/epoch unqueried; corrected ENTRY rebaseline 64,927/epoch 5,324. The earlier C04 epoch 4,984 belongs to a separate capture.

Inherited producer/preparation/inspection/display failures and Root's C11 publication first failure/minimal retry remain intact. This body packet separately retains one preparation tool-argument rejection and three display truncations; the original full inline stdout was not physically saved for those truncations. Capture and decoder each passed on their first execution. The SS.B clarification is a scope qualification, not a failure.

Ten finite inputs were frozen at `f7fb43633f7082f188a13abe75042d41c29c1e8f`: sixteen actual Git objects and ten matching LF/index/working bindings. A bounded selection preserves 125 historical originals, with exclusions recorded explicitly. Current Source/include tree identities were recorded without reading their bodies. Older identities remain labeled as older observations.

No Source body/absence census, target/provider/prototype/xref/listing/context query, GPR mutation, build/test/probe/game/OS/audio/runtime experiment, archive, guarded final-proof helper, or `00C11C52` query was performed. No new C++ reconstruction was made.

The [report](../reports/cc12_c11c21_body_physical_01.json) retains the full conditional manual/actual decode, every saved-flow comparison, conditions, history and scope. Private originals and the bounded `final_evidence_manifest.json` reside under `local/cc12_c11c21_body_physical_01/`; `final_handoff.json` separately pins the completed publication streams and commit delivery. No admission, ABI or runtime claim follows from this packet.
