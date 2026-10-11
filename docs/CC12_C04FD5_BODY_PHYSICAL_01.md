# C04FD5 physical body packet 01

Status: **Capture and conditional manual/Capstone comparison complete; Root independent review remains separate.** This packet admits no physical body, ABI, runtime behavior or permanent live baseline.

The Root-launched exclusive capture completed 21 Native commands at live/snapshot count 65052 and epoch 7572. Eight count observations agreed. Three complete ENTRY metadata records matched the accepted payload after removing only its two top-level epoch fields. Two independent typed walks each found four units covering exactly C04FD5 through C04FDD. The original command streams retain all 126 before/after physical CLI source copies. The paired nine-byte reads agree: `6a00e88bffffff59c3`.

| Address | Bytes | Conditional default x86-32 decode |
| --- | --- | --- |
| 00C04FD5 | 6A 00 | Push sign-extended immediate zero as a 32-bit value |
| 00C04FD7 | E8 8B FF FF FF | Near relative call to 00C04F67 |
| 00C04FDC | 59 | Pop a 32-bit value into ECX |
| 00C04FDD | C3 | Near return without an immediate stack adjustment |

The call displacement is signed -117: `00C04FDC - 0x75 = 00C04F67`. This arithmetic does not inspect the callee. Its captured saved name is an identity observation; its semantics, ABI, stack balance and return behavior remain opaque. The following POP is not evidence that ECX becomes zero, and no return-value contract is inferred.

The manual was physically written and pinned before the first fresh Capstone import. The Root-launched Capstone 5.0.7 process decoded only these nine bytes in detailed x86-32 mode: four units, no comparison differences, exit 0 and empty stderr. Root separately reported its first independent decoder review PASS; that receipt remains Root-owned. Raw encoding, operands, groups and register metadata are retained before comparison; library metadata is not complete architectural-effect proof. No per-address CPU context was queried. In addition to the conditional code/operand mode, every full-ESP stack formula requires **SS.B=1 and normal instruction completion**. SS.B=0 uses SP updates and does not justify those full-ESP formulas. C04FDE is arithmetic end-plus-one only and remains unqueried.

The preparation froze actual Main/worktree f7fb43633f7082f188a13abe75042d41c29c1e8f with 16 actual Git objects and ten positive LF/index/blob bindings. Source/include subtree identities were retained without reading their bodies. Origin observations remain separate. The finite accepted history retains 140 originals, with the previously declared 251 worker and 2319 Root batch artifact exclusions unchanged.

Historical states remain distinct: ENTRY 64730/98; the BODY stop at 64747 versus proposed 64742 with actual epoch unqueried; and rebaseline 64927/4984 with one post-seal inspection incident. Their original failure scopes are preserved in the report, including Root FD plan/driver/display incidents and the original inline-stderr retention limitation. Current preparation retains one setup failure and one display truncation. Current capture has zero Native/process/helper failures or control stops; the manual and fresh decoder stages have zero helper/process/comparison failures. Publication completion and commit originals are bound separately by the handoff.

The report includes exact manual semantics, paired-capture identities, original incident scopes and private evidence paths. The finite `publication/original_artifact_manifest.json` copies and pins the original evidence selected before publication; `final_handoff.json` binds it to the two public output blobs and commit. No target/provider/prototype/xref/listing/context query, additional decoder run, Source body read, build/test/probe, archive, guarded final-proof helper, game/OS/audio/runtime operation or GPR write was added during publication.
