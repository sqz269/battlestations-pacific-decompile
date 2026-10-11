# C07C45 saved-body evidence

The saved owner at `00C07C45` covers exactly 20 bytes, `00C07C45..00C07C58`, partitioned into eleven whole instructions. Paired physical captures and three complete typed passes agree. This establishes the encodings and their conditional local effects. The saved name `__SEH_epilog4` does not establish a library implementation, operating-system role, restored-register layout or ABI.

| Address | Bytes | Original context-zero text |
| --- | --- | --- |
| `00C07C45` | `8B 4D F0` | `MOV ECX,dword ptr [EBP + -0x10]` |
| `00C07C48` | `64 89 0D 00 00 00 00` | `MOV dword ptr FS:[0x0],ECX` |
| `00C07C4F` | `59` | `POP ECX` |
| `00C07C50` | `5F` | `POP EDI` |
| `00C07C51` | `5F` | `POP EDI` |
| `00C07C52` | `5E` | `POP ESI` |
| `00C07C53` | `5B` | `POP EBX` |
| `00C07C54` | `8B E5` | `MOV ESP,EBP` |
| `00C07C56` | `5D` | `POP EBP` |
| `00C07C57` | `51` | `PUSH ECX` |
| `00C07C58` | `C3` | `RET` |

## Frozen positives and admission

Baseline `43365d7234a8a40840897241d0ec1b454a99b668` matched Main at both opening and closing freeze observations. The current ENTRY and ENTRY_PRIMARY documents/reports, `config/target.json`, and three CLI producers are retained as eight actual blobs with their commit/root/path trees: fourteen actual Git objects, with eight working LF/index/blob equalities. There was no baseline retry or historical extra-object closure. Source remains identity-only: `src=dfb2ee135e7b642465d448dcc1616fd92686ad51`, `include=7bb7ed2fc76e16eb09ff91be796c5b264aab9ca9`. No Source contents, absence census or previous proof ZIP was read.

The accepted entry contributes the physical prefix `8B 4D F0` and its complete saved 20-byte owner metadata. Its original evidence and separate worker/Root failure histories remain intact in the positive files. Body scope is derived from that saved complete owner, without querying `00C07C59` or an adjacent owner.

The two metadata-only walks independently restart at `00C07C45` and advance only by each fresh whole-unit length. All four listing aliases identify the exact same `InstructionDB` unit at each query, with kind `instruction` and `defined=null`. Both walks terminate arithmetically at `00C07C59` without querying it. Root reviewed all 22 original typed replies before admitting physical capture.

Root's first gate passed. Its 1,976,035-byte receipt has SHA-256 `f0be3365d5b8aace27365ca233489027a107cd0e563d0c6710e5062e35c9889b`; the 2,192-byte control has SHA-256 `262a22fc3bdf5b3d8cea938b2bbe7f0b3f31cf19a1f1993b2b5f2a86453205e6`. All 312 named originals were copied and readback-verified before the first phase-2 Native call: 301 metadata preimages, ten top originals and the control. The complete-set verification chronology was saved before that call; the original admission set remains unchanged.

There were 43 read-only CLI calls: six count brackets, 33 typed captures, two exact 20-byte reads and two context-zero text batches at the eleven established starts. Each call physically read all three CLI producers before and after execution, totaling 129 before and 129 after reads; original invocations, prepins and full stdout/stderr are retained. Project `bsp`, program `/battlestationspacific.exe`, x86 little-endian 32-bit language, default RAM, image base `00400000`, function count 64,730 and modification number 98 remained consistent. GET provenance comes from the frozen producer and response originals, without an independent network trace.

## Metadata and physical fields

All three original direct replies at each start are byte-identical. The complete sole owner is `__SEH_epilog4`, entry `00C07C45`, one inclusive 20-byte body range, no-return/thunk/recursive-thunk flags false and direct thunk target null. Function-at-query equals the owner only at the entry; it is null at internal starts. The containing owner is retained independently throughout.

Default and effective flow independently identify ten `FALL_THROUGH` units with the next established start as fallthrough, followed by a `TERMINATOR` at `00C07C58` with null fallthroughs. Call/jump/conditional/computed flags are false everywhere; terminal is true only at the last unit. Overrides are `NONE`, fallthrough override is false, and default/effective/direct-call target arrays are empty. These are fresh saved-field observations. The saved `is_computed=false` value is retained even though the encoded near `RET` obtains its destination from current stack memory.

The body bytes are `8B 4D F0 64 89 0D 00 00 00 00 59 5F 5F 5E 5B 8B E5 5D 51 C3`. Both 20-byte captures have SHA-256 `39142b8d79823b8b3c0dd534ee99caf6c8cce4347b7608cfc004a14c873f7411`. Both 1,711-byte context-zero originals have SHA-256 `7f1a93036109e3e2e5e474acdb8d897f9385b62977c149ed4b45feb471b76424`.

The first MOV has opcode `8B`, ModRM `4D` (mod 01, reg ECX, r/m EBP), and signed disp8 `F0=-16` at offset 2. The FS store has prefix `64`, opcode `89` at offset 1, ModRM `0D` at offset 2 (mod 00, reg ECX, r/m 101), and a four-byte absolute displacement zero at offset 3. It has no base/index/SIB; FS overrides the otherwise-default DS. `MOV ESP,EBP` has opcode `8B` and ModRM `E5` (mod 11, reg ESP, r/m EBP). The POP/PUSH operands are encoded in the low three opcode bits. `C3` is near RET without an immediate adjustment. Every instruction uses 32-bit operand/address decoding; there are no immediates or SIB bytes in this body. The JSON retains every unit's complete prefix/opcode/ModRM/displacement/operand record.

## Conditional instruction effects

At `00C07C45`, normal completion loads ECX from four little-endian bytes at actual current `SS:((EBP-16) mod 2^32)`. This is an explicit memory load and does not require an implicit-stack address-size premise. At `00C07C48`, normal completion stores the current ECX through actual current `FS:0`. Segment descriptors, mappings, memory contents, aliasing and access/fault outcomes are unknown; the FS offset does not authenticate an operating-system or exception-chain role.

For each stack operation at `00C07C4F`, `00C07C50`, `00C07C51`, `00C07C52`, `00C07C53`, `00C07C56`, `00C07C57` and `00C07C58`, renew the predicate at that instruction: operand size 32, actual current SS stack-address size 32, and normal completion. Let S denote ESP immediately before that individual instruction. Each POP loads its named register from actual current `SS:S` and sets ESP to `(S+4) mod 2^32`; both consecutive POP EDI encodings remain represented. PUSH ECX stores current ECX at `SS:((S-4) mod 2^32)` and sets ESP to that offset. RET loads EIP from `SS:S` and increments ESP by four, with no imm16 adjustment. Its target value, validity, caller relationship and current CS attributes remain unknown.

`MOV ESP,EBP` copies the then-current EBP into ESP; it proves no incoming frame relationship. All eleven instructions leave arithmetic EFLAGS unchanged on normal completion. Registers not named by an instruction's effects remain unchanged by that instruction. The formulas do not assert that a complete invocation succeeds, restores an earlier caller state or implements a binary-compatible epilog.

## Decoder and finite replay

The complete manual artifact was saved before the first fresh Capstone 5.0.7 import and comparison. All eleven actual decodes agree on physical fields and operands, with differences empty. Ancillary decoder access/register/group/flag records are retained separately, along with module/library identities and original streams. Manual artifact: 20,756 bytes, SHA-256 `c81edcb2600b1a2fef8060bf3c145982e2f3ab21e6e8f6264ab99687b8227d99`. Actual decoder artifact: 10,400 bytes, SHA-256 `3e13e363be85746674e8b7fd62cc7bd9f5aa3fc05bc8d4f0f679619a4bcdee49`.

The original finite evidence ZIP and a distinct physical byte-identical copy each passed their first strict replay: one supplied ZIP read, one exact pinned helper-code execution, one exclusive receipt write and zero denied events, including caught attempts. The fully read pure helper verifies actual Git objects, complete saved replies, all Root copies, the pre-query chronology and the manual/actual decoder comparison without importing or executing Capstone or querying the workspace under the audit hook.

Evidence ZIP: 750 members, 8127802 bytes, SHA-256 `0ad3b64ef1352801d3b28f0c7343eea9d93e2e004bb3f1289f1faabd54edf390`. Pure helper: 39991 bytes, SHA-256 `973f43b9217d04551fd8d686bf7e7e9a2cd04c0bab625058664c0cb448d6d759`. Guard: 3325 bytes, SHA-256 `49aa2f8c102ba0fd32d38d7ace97a8041d17fdfd3ca7bbb7d6cecbb8c44bdb1e`. Final delivery adds the two public outputs, original publication/commit streams and six actual Git objects for a twenty-object current closure and ten final LF/index/blob checks. Postcreation package streams and strict receipts remain separate original artifacts.

## Failure scopes and limits

Current worker Native, evidence-helper, fresh-decoder, strict-evidence and inspection failures are zero. Two setup commands failed before capture: colon range syntax in the lease command, then a missing parent-directory option in authoring setup. One pre-Native positive-field display was truncated. The report retains the complete visible setup-result transcriptions and the scoped display incident; unavailable original stream bytes or omitted stdout are not fabricated.

The immutable Root admission records contain zero gate failures, one read-only guessed-filename inspection failure and one display truncation. Root separately reported a later manual-producer failure because its copied field helper omitted the FS prefix, before manual output or Capstone import; originals were retained and a separate minimal retry preceded its first actual passing fresh comparison. Root also reported one later display truncation. Those later histories remain separate for Root's PRIMARY review and are outside the original 312 admission copies. Accepted ENTRY and earlier C1737B histories remain in their own original positive reports.

No Source body, absence census, Ghidra mutation, new test/probe, build, game/OS/audio/runtime execution, provider, cell, referent, prototype, target or end-cursor query occurred. This is metadata and physical instruction evidence; ABI compatibility and live function/game behavior remain unestablished.
