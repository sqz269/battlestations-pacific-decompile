# Type5 isolated fixture: saved runtime artifact peer

The independent peer passes for the sealed `local/type5p3` family. **Source
credit remains 0.** This peer parsed frozen evidence from Root's one accepted
process; it performed no replay, selected-reader import, Native query, compiler
invocation, provider load or target execution, and authored no Root receipt.

Root's final seal is 197,574 bytes with SHA-256
`698613830924a8624ca133ac31aa2da7f763d2cd78bd9999a38d3cac978ce21e`.
All 762 listed artifacts and the seal were checked before capture: exact family
membership is 763 files. Inspection began only after Root's final post/seal
notification. All original and frozen files remained unchanged through the
peer's final bookend.

An independent standard-library data parser matched the actual process stdout
to `runtime.json`, checked exit 0 and empty stderr, and reconstructed both
binary captures and complete object images. The accepted records report one
Source call, one Original call, two root plus two child allocations, four frees,
and 53 code-gate spans. These totals are cross-checked against artifacts and
reviewed code; they are not a separate allocator trace.

| Saved case | Root / returned EAX | Owned child | ESP before and after | Text pointer |
| --- | --- | --- | --- | --- |
| Source | `00648AB0` | `00641DF0` | `001294CC` | `001A9B40` |
| Original | `006424F0` | `00647340` | `001294CC` | `001A9B68` |

Both 56-byte storage images equal independently constructed expectations.
The specified write mask contains 33 bytes; all 23 other bytes retain their
respective `A6` and `59` poison. Both 40-byte guarded text objects retain their
sentinels and all eight input bytes. Each six-byte child equals its input
through the NUL terminator. The two roots, two children, guarded texts and
Capture objects have disjoint recorded ranges and are disjoint from the raw
call frames.

Both `Capture` files are exactly 132 bytes, with 16-byte pre-sentinels,
25 U32 register fields, and 16-byte post-sentinels. All sentinels, both stack
guards and EBX/ESI/EDI/EBP preservation checks pass. The three dead argument
slots are `001294C0`, `001294C4` and `001294C8`; their captured words match the
actual text pointers and both scalar arguments. ESP is balanced, consistent
with the Source body's `RET 12` and the reviewed raw caller.

Both snapshots have clear DF, post-call EFLAGS `00000246`, and observed ES
`002B` before and after. Recorded post-call ECX is zero in both; EDX is
`619A32D7` for Source and `F825C64B` for Original. These residuals and ES values
are observations of these calls, not universal register-preservation claims.

The executable and gate hashes match the prior static peer. Independently read
Source63 and saved bound-Original63 retain all 59 Native literal bytes outside
CALL operand `[35,39)`. Both calls resolve to the same linked duplicate at
`45001080`; the original Native CALL resolves to `00438E40`. The linked
57-byte duplicate matches its admitted frozen original except for the two
qualified operand ranges `[34,38)` and `[44,48)`. All 53 packed code spans match
the frozen executable bytes; saved artifact inspection does not add a new
ordinary caller entry or broaden duplicate admission.

The four provider records agree with the packed gate, physical-file hashes,
I386 headers and named nonforwarded exports. Recorded malloc/free/`_callnewh`
share UCRT base `767A0000`; memcpy uses VCRUNTIME base `72B10000`. All recorded
IAT targets equal the recorded exports and module base plus export RVA.
An independent physical PE relocation parser reconstructs every 32-byte
adjusted prefix. `_callnewh` contains one overlapping HIGHLOW relocation at
RVA `0008B018`, offset 24 in its prefix: `101052A0` becomes `768A52A0`. The
other three prefixes require no overlapping relocation adjustment.

The provider observation boundary is explicit. Preentry `MEM_IMAGE`,
`AllocationBase`, mapped NT-path and `GetProcAddress` checks rely on reviewed,
gated probe code; their truth is not reconstructed from separate raw artifact
fields. After all four frees, the probe rechecks IAT targets, held-handle file
ID and size, full mapped raw-file SHA-256, and adjusted live prefixes. It does
not repeat `MEM_IMAGE`, `AllocationBase`, NT-path or `GetProcAddress` queries
at that post-free bookend. This peer compares saved records and frozen bytes.

The code/protocol reader, process, enclosing launch, observation reader and
post closed records all report exit 0, no timeout, and passing input bookends;
their recorded close order is consistent. The three selected reader outputs
were read as data only. The post record reports one accepted process, four
executed TUs, and unchanged Native and input evidence.

The first own parser attempt stopped because it incorrectly compared
`normalized_live_prefix` with unrelocated physical bytes. That attempt and its
passing input bookends remain preserved. A fresh parser independently applied
the physical PE HIGHLOW relocation table and passed. No selected artifact
changed during this correction.

The peer family is
`J:/PROG/battlestations-pacific-decompile-cc12_type5_complete_guard_materialization_TEXT/local/t5runPeer01`.
Its manifest lists 2,903 artifacts; including the manifest and seal, exact
membership is 2,905 files. Manifest SHA-256 is
`92c98b85229a73fd398d5cff6efcccd38e179ecc302adb93a865813bad1b314f`;
seal SHA-256 is `05ef3e9b93ecf3f058bdaf21eb4f4174a70ca0eb87b9eb3b2c843e6d746e720b`.
The final bookend checks 2,871 original/copy pairs and 5,742 hashes: 763 Root
files, six Source inputs and 2,102 peer-analysis dependencies, plus exact
membership of 16 analysis scopes and 3,588 scope members. The peer environment
and PowerShell host identity also remain equal.

The companion is `reports/cc12_type5_isolated_runtime_artifact_peer.json`.
The explicit ungated GS-failure frontier remains unadmitted. This packet grants
no private CRT/EH, OOM, class, startup, game or drop-in original ABI equivalence.
