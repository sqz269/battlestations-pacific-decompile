# CC12 BFBB61 direct-target entry evidence

The admitted four bytes at `00BFBB61` are `8B 44 24 04`: `MOV EAX,dword ptr [ESP+4]` in the saved x86-32 interpretation. On normal successful completion, the four-byte value read through current SS at `(ESP+4) mod 2^32` replaces EAX. ESP and EFLAGS remain unchanged. All 56 remaining saved-owner bytes stay unqueried.

The accepted incoming instruction at `00C04F11` is `E8 4B 6C FF FF`. Its signed displacement is -37,813; adding it to encoded return offset `00C04F16` gives numeric target `00BFBB61`. This is rederived from the current accepted C04EFB body report, without opening previous proof archives or querying another target.

## Admission and original evidence

Actual Main is frozen at `dbc8e29f7fdf922f35f3b133967795884c2aa37f`. Eight positive inputs are the current C04EFB BODY_METADATA and BODY_METADATA_PRIMARY document/report pairs, target configuration and three CLI sources. Fourteen actual Git commit/tree/blob objects establish frozen membership, working bytes after LF normalization and exact stage-zero index blobs. Source subtree identities are retained without reading their payloads or conducting an absence census.

Phase 1 captured count bookends and two complete typed replies at `00BFBB61`: four CLI invocations and 16 original HTTP responses. All four listing aliases agree on a four-byte InstructionDB unit. The complete containing owner is one 60-byte range `00BFBB61–00BFBB9C`, named `__get_winmajor`, with `no_return=false` and no thunk. Independently retained default/effective flow is FALL_THROUGH to `00BFBB65`, override NONE, empty target arrays and no direct-call companions. Names and flags remain saved metadata.

Root's full metadata gate passed its first child execution and admitted exactly `00BFBB61–00BFBB64`. Before phase 2, all 218 Root artifacts were physically copied and verified: 207 metadata preimages, ten listed top files and the admission manifest. Each preimage also equals its worker original. Phase 2 captured paired exact four-byte reads, paired context-zero text only at `00BFBB61`, closing typed metadata and count bookends: seven CLI invocations and 23 HTTP responses. Both byte/text pairs and all three original typed replies match.

All 11 Native invocations physically reread each of the three CLI source files before and after execution, retaining the 22 observations and raw payloads. Project/program remained `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`, language `x86:LE:32:default`, modification number 98 and function count 64,730.

## Physical decode and current-state contract

| Field | Value |
| --- | --- |
| Prefixes | None |
| Opcode | `8B`, MOV register from r/m |
| ModR/M | `44`: mod 1, destination register 0 (EAX), r/m 4 (SIB) |
| SIB | `24`: scale bits 0, index 4 (none), base 4 (ESP) |
| Displacement | Signed disp8 `04`, value +4, physical offset 3 |
| Immediate / relative transfer | Absent |
| Destination | Four-byte EAX write |
| Source | Four-byte memory read, ESP base, no index, scale 1, default current SS |
| Register access | ESP read, EAX write; no implicit registers |
| Address size / operand size | 32 / 32 bits |
| EFLAGS / groups | Zero effects / empty |
| Numeric next cursor | `00BFBB65`, unqueried |

The complete independent manual record was physically written before importing or constructing fresh detailed Capstone 5.0.7. It agrees with Capstone on every physical field, operand and access, encoding offset/size, explicit and implicit register access, EFLAGS and groups. Both raw byte captures equal SHA-256 `ab6146f1fb8c4ab2e4f26067d3eb71395dbe376365faf10f2377f74e9282b414`.

The explicit address calculation uses current ESP with 32-bit address size and default SS. The MOV itself neither performs an implicit stack operation nor requires SS.B=1 for that address calculation. No linear address, flat-segment equality, memory readability or loaded value is established. Exceptional transitions lie outside the normal-completion claim.

Optional composition with the preceding accepted E8 requires uninterrupted normal CALL-to-entry execution and applicable current code/segment predicates, including SS.B=1 for CALL and preserved relevant state. Under those predicates only, the MOV source offset equals the pre-CALL ESP numerically. This does not identify an argument, frame, prototype, calling convention, getter behavior or library/provider binding. The saved `__get_winmajor` label is not authenticated by this instruction.

## Histories and validation boundary

Current Native captures, decoder comparison and captured producer processes passed their first executions. One current worker template-inspection display truncation is retained separately. Root's current gate-phase history retains two inspection-command incidents and one display truncation, with zero gate-child failures. The accepted C04EFB body worker group-only decoder correction and Root manual-producer AND failure remain scoped parent history in the report; they are not current failures. No original failed stream was resampled or redressed.

No Source reconstruction or absence census, Ghidra mutation, build, new test, probe, Original/game execution or OS/audio experiment occurred. The next cursor `00BFBB65`, remaining 56 bytes through `00BFBB9C`, end-exclusive `00BFBB9D`, cells, providers and prototypes remain unqueried. This is static physical and provenance evidence, without ABI or game validation.

The private proof binds the actual final commit, its single frozen parent, exactly two public outputs and their working/index blobs. The fully pinned helper is supplied outside the ZIP. Original/copy strict receipts permit one ZIP read, one execution of that exact precompiled helper and one exclusive receipt write, with every other audit event denied. The helper imports no decoder under the hook, executes no archived code and opens no earlier archive.
