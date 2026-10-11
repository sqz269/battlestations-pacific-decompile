# CRT dispatch first direct callee: exact entry at `00C05B68`

The two-byte entry is `6A 03`: a 32-bit-operand `PUSH` of sign-extended immediate `+3`. This closes only the exact saved instruction at `00C05B68`. The numeric continuation `00C05B6A` and the remaining 55 bytes of the saved owner were not queried.

## Positive input and saved ownership

The frozen accepted body at `00BFBA09` starts `E8 5A A1 00 00`. Independently decoding its signed displacement gives `41306`, return address `00BFBA0E`, and numeric destination `00C05B68`. This packet uses the BODY and BODY_PRIMARY document/report pairs, `config/target.json`, and three existing CLI tools as exactly eight selected positive Git inputs. It does not recursively open an earlier proof archive.

Frozen actual Main at worktree creation is `7b3676abb46b448af7a38cf1912e1de676f4e7a3` (tree `e138972c91fe0c8e00abfa76fefb5869a0551e61`). During capture Main advanced to `a8430e60ef99c3ea7269041e3476b0979b0f5d26`. The retained actual Git ancestry command succeeds; the frozen commit is not represented as current Main. The root-tree references for `src` and `include` are identity evidence only, without Source-body reads or an absence census.

Three exact typed responses (paired opening metadata and closing metadata) are byte-identical. All four listing aliases identify an exact two-byte `InstructionDB` at `00C05B68`; `kind=instruction`, `defined=null`. Saved function-at and containing-owner entry are both `00C05B68`, with one complete 57-byte range `00C05B68..00C05BA0`, `is_thunk=false`, `no_return=false`. The saved label `__FF_MSGBANNER` is not authenticated type, service, preservation, argument or nonreturn evidence; `no_return=false` is not a proof of return.

The saved override is `NONE`; default and effective flow are `FALL_THROUGH`, with all call/jump/conditional/computed/terminal flags false, no targets, and unoverridden default/effective fallthrough `00C05B6A`.

## Physical instruction and local CPU effect

| Field | Exact evidence |
| --- | --- |
| Bytes / SHA-256 | `6a03` / `413d574c3b171871faa327f83f0ca55e6ffbbf2bf281b4146219a7e59bc14173` |
| Prefix / opcode | No prefix; `6A` |
| ModR/M / SIB / displacement | None / none / none |
| Immediate | One byte at offset 1: `03`, signed `+3` |
| Operand size / sign extension | 32 bits; `00000003`, stored bytes `03 00 00 00` |
| Code address size | 32 bits; does not establish the stack segment B attribute |
| Implicit operation | Current stack pointer and four-byte write through current `SS` |
| Flags | Unchanged by this instruction; their existing values remain unknown |
| Successful continuation | `00C05B6A`; arithmetic and saved metadata only |

The manual opcode/operand decode agrees with paired exact bytes, paired context-zero `PUSH 0x3` text, typed length/flow, and separately executed Capstone Python 5.0.7 / engine 5.0.1280 in x86-32 detail mode. Full actual decoder output, implicit register accesses, immediate offsets and sizes, prefix/opcode arrays, groups and zero flag mask are retained. Capstone is never imported or constructed inside the pure replay helper.

Let `T` mean ESP immediately before the accepted incoming near CALL. Only for successful execution with actual current `SS.B=1`, that CALL writes return `00BFBA0E` through current SS at `(T-4) mod 2^32` and enters `00C05B68` with ESP at that value. This is a local CALL effect.

Renew the actual current `SS.B=1` predicate at the entry PUSH. If `S` is ESP at this instruction and the operation succeeds, it writes dword `3` through current SS at `(S-4) mod 2^32`, sets ESP to that offset, and continues at `00C05B6A`. Only when joining those two successful adjacent operations with both stack predicates and no intervening state change may this be written `ESP=(T-8) mod 2^32`. The two local writes do not recover an argument list or stable frame.

All preceding opaque return state remains unknown: registers including EBP/ESP, flags, segments and descriptors, stack attributes, memory and providers. A different or unknown stack-address attribute does not support the full-ESP equations. Segment translation, limits, permissions, faults and actual runtime values are unverified. This instruction's unchanged non-stack registers and flags do not establish any callee-wide preservation or return contract.

## Capture and verification boundary

The read-only captures verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, modification number `98`, and 64,730 functions. Phase one used only count / exact typed A / exact typed B / count. Root reviewed all four listing aliases, owner coverage, complete ranges and flow before admitting two bytes. Its first execution passed; 62 finite Root gate files were copied unchanged (43 preimages, 14 actual frozen Git objects, five gate/manifest files).

Phase two used count / two exact two-byte reads / two context-zero reads / closing exact typed metadata / count: seven commands. All 11 Native commands exited zero with empty stderr. The physical bytes of all three existing CLI tools were pinned before and after every command. Original JSON wrappers retain URL, HTTP status, Base64/raw response, SHA-256 and verified program identity. GET provenance is the frozen producer's explicit request method; no independent wire capture is claimed.

One initial offline precondition failed because Main advanced after worktree creation. It executed zero Native commands. The failed producer, exact preimage and original full stdout/stderr are preserved. A distinct pinned retry used the accepted frozen base and actual Git ancestry evidence. No Native, phase-two, decoder or Root-gate failure occurred.

The finite core ZIP and a distinct byte-identical copy both passed the same pure helper on their first execution. Each strong audit run recorded one ZIP read, one execution of the exact precompiled helper code object, one exclusive receipt creation, and `denied=[]`. Unknown audit events are rejected and denial telemetry is checked again after execution, including exceptions a library might catch. All archived producers remain inert.

The public output is exactly this document and its [report](../reports/cc12_crt_dispatch_first_direct_callee_entry.json). The private final proof seals actual postcommit Git objects, proves those two changes against the frozen root/subtrees, and verifies working bytes, index stage/mode/blob and stored Git blobs. It is replayed on the original and a distinct archive copy. Evidence is under `local/cc12_crt_dispatch_first_direct_callee_entry/` (ignored); the tracked report pins the immutable core evidence and physical producers, with final archive/receipt identities in the private handoff.

No Source change, build, test, probe, GPR mutation, annotation/ledger edit, Original execution, ABI compatibility, game, OS/audio or live runtime credit is claimed. The next byte at `00C05B6A`, broader owner body, prototypes, providers and target cells remain outside this packet.
