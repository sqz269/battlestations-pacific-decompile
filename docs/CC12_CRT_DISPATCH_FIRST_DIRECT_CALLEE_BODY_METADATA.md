# CRT first direct callee: complete saved body at `00C05B68`

The saved body is exactly 57 bytes, `00C05B68..00C05BA0`, comprising 19 whole instructions. Independent manual decoding agrees with paired physical captures, paired context-zero saved text, three matching original metadata payloads per start, and actual Capstone 5.0.7 detail output. This establishes the bounded bytes and local CPU behavior; call targets, memory contents, service meaning and runtime behavior remain unverified.

The accepted entry was `6A 03`, a PUSH of sign-extended `+3`. Eight positive inputs were frozen from actual Main-at-creation `582a0e533bc13cfb3a97bac0eedc4530dac78369` (tree `e717b98f66d4366a7843fe3f4308a06ef27d9665`): ENTRY and ENTRY_PRIMARY document/report pairs, config, and three existing CLI tools. Actual Git objects and current working bytes agree. The capture records an actual ancestry check against observed Main `582a0e533bc13cfb3a97bac0eedc4530dac78369` without requiring Main to remain stationary. Source subtree identities are inherited root references only.

## Saved listing and physical encoding

All four exact listing aliases identify `InstructionDB` units with `kind=instruction`, `defined=null`. The saved owner starts at `00C05B68`, with one complete 57-byte range, `is_thunk=false`, `no_return=false`, and saved label `__FF_MSGBANNER`. That label and no-return field do not authenticate service, types or whether the function returns. Every saved flow override is `NONE`, default/effective flow agrees, and fallthrough is unoverridden; this saved state is recorded separately from physical effects.

| Address | Bytes | Physical x86-32 instruction |
| --- | --- | --- |
| `00C05B68` | `6a03` | `push 3` |
| `00C05B6A` | `e8e80f0200` | `call 0xc26b57` |
| `00C05B6F` | `83f801` | `cmp eax, 1` |
| `00C05B72` | `59` | `pop ecx` |
| `00C05B73` | `7415` | `je 0xc05b8a` |
| `00C05B75` | `6a03` | `push 3` |
| `00C05B77` | `e8db0f0200` | `call 0xc26b57` |
| `00C05B7C` | `85c0` | `test eax, eax` |
| `00C05B7E` | `59` | `pop ecx` |
| `00C05B7F` | `751f` | `jne 0xc05ba0` |
| `00C05B81` | `833d8c55e10001` | `cmp dword ptr [0xe1558c], 1` |
| `00C05B88` | `7516` | `jne 0xc05ba0` |
| `00C05B8A` | `68fc000000` | `push 0xfc` |
| `00C05B8F` | `e814feffff` | `call 0xc059a8` |
| `00C05B94` | `68ff000000` | `push 0xff` |
| `00C05B99` | `e80afeffff` | `call 0xc059a8` |
| `00C05B9E` | `59` | `pop ecx` |
| `00C05B9F` | `59` | `pop ecx` |
| `00C05BA0` | `c3` | `ret` |

Body SHA-256: `ab792e6c00b1c833ff4ea50316b43c6501bca25308ef4479caafd0f20c8cbebf`. No prefixes or SIB bytes occur. `83 F8 01` is group `/7`, register EAX comparison with sign-extended imm8 `1`; `85 C0` tests EAX against itself. `83 3D 8C 55 E1 00 01` is group `/7`, ModR/M `mod=0, r/m=5`, four-byte absolute displacement `00E1558C`, default current DS, and one-byte immediate `1`. `59` writes ECX from the current stack. `68 FC000000` and `68 FF000000` push full imm32 values 252 and 255; they are not sign-extended imm8 values -4 and -1.

The relative CALL displacements are `+135144` at `C05B6A` and `+135131` at `C05B77`, both targeting numeric `C26B57`; and `-492` at `C05B8F` and `-502` at `C05B99`, both targeting numeric `C059A8`. Their encoded returns are `C05B6F`, `C05B7C`, `C05B94`, and `C05B9E`. Conditional rel8 displacements are `+21` at `C05B73` to `C05B8A`, `+31` at `C05B7F` to `C05BA0`, and `+22` at `C05B88` to `C05BA0`. No destination or encoded cell contents were queried.

## Conditional local behavior

Every statement below assumes the relevant instruction is reached and succeeds without fault. Every implicit PUSH, CALL, POP and RET separately requires actual current `SS.B=1` before using a full-ESP equation. Decode width does not prove that attribute. All returned register values, including EBP/ESP, flags, segments/descriptors/attributes, memory and providers become unknown at each opaque call continuation. No callee is proved to return.

At entry, PUSH writes `3` through current SS, then the first CALL enters numeric `C26B57`. If it returns at `C05B6F`, CMP EAX,1 sets subtraction flags; POP ECX reads the current stack and preserves those flags. JZ takes `C05B8A` exactly when that returned EAX equals 1. Otherwise execution reaches another PUSH `3` and CALL to `C26B57`.

If the second call returns at `C05B7C`, TEST EAX,EAX establishes ZF from the new current EAX. POP ECX preserves that flag, and JNZ takes the RET at `C05BA0` when EAX is nonzero. If EAX is zero, CMP reads current dword `M` through current DS at encoded offset `00E1558C`; JNZ reaches the RET when `M != 1`, otherwise execution reaches `C05B8A`. No value or provider is assigned to `M`.

At `C05B8A`, PUSH writes imm32 252 and CALL enters numeric `C059A8`. If it returns, the new state is unknown. PUSH then writes imm32 255 and another CALL enters the same numeric target. If that fourth call resumes at `C05B9E` with current ESP `Q`, two successful POP ECX operations read current SS at `Q` and `Q+4`; the latter overwrites ECX from the former. RET then reads its target from current SS at `Q+8`. Only with the stack predicate renewed at each operation do the local final ESP equations give `Q+8` before RET and `Q+12` afterward, modulo 2^32. `Q` has no established relation to earlier stack pointers, and these reads are not authenticated argument cleanup or original return slots.

For CMP's current 32-bit input `X`, let `R=(X-1) mod 2^32`: CF is unsigned `X<1`, ZF is `R==0`, SF is bit 31 of R, PF is even parity of its low byte, AF is `((X xor 1 xor R)&0x10)!=0`, and OF is `(((X xor 1)&(X xor R))&0x80000000)!=0`. TEST preserves EAX, sets SF/ZF/PF from EAX, clears CF/OF, and leaves AF undefined. JZ/JNZ read ZF without writing flags. The intervening POP instructions preserve comparison flags. PUSH/CALL/POP/RET do not establish callee-wide preservation.

Memory reads and writes remain in physical instruction order. Current DS memory may alias current stack memory because segment bases and addresses are unknown. Earlier stack writes or opaque call effects can affect later reads. There is no explicit ADD ESP in these bytes. No frame, argument count/type, calling convention or stable return stack is recovered.

## Capture, replay and failure evidence

Read-only Ghidra identity remained `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, modification 98, 64,730 functions. Two independent walks restarted at the saved owner entry and advanced only by each newly captured exact unit length. Each covered all 57 bytes without gap/overlap and stopped before querying `C05BA1`. Root's whole-listing gate preceded paired exact 57-byte reads, paired context-zero text at exactly 19 starts, and closing typed capture of all 19 starts. All 49 Native commands exited zero with empty stderr. Original direct responses, Base64, URLs, digests, full commands and actual streams are retained; all three CLI physical files were pinned before/after each command. HTTP GET provenance comes from the frozen producer's explicit method and actual response URL, without a separate wire-trace claim.

Failure history is explicit:

- One initial worker scaffolding command failed before any metadata producer or Native call because the private parent directory did not yet exist. Its exact attempted inline script and original combined tool output are retained; separate original stdout/stderr and a preexecution pin for that inline failed setup are not claimed. The distinct pinned setup retry and both capture producers succeeded.
- Root gate attempt one expected string `0` where the actual autostart schema is Boolean false. Its exact failed producer, prepin, empty stdout and 378-byte stderr remain under `Root_gate_attempt01`. Attempt two passed after correcting that field. Failed Root attempt in-memory Git streams were not separately persisted. Root also disclosed one preliminary truncated schema display; later bounded inspection and the full gate supplied the conclusion. All 206 actual Root artifacts were copied unchanged.
- The first worker strict core checker treated `git cat-file -t` type output as raw object bytes and exited 1 before a receipt. Its exact helper/guard/runner, command record, empty actual stdout/stderr, original archive and copy are preserved. Denial telemetry was unavailable for that first run. One unguarded offline diagnostic retained the full assertion traceback; one separately pinned strong guarded diagnostic wrote an explicit FAIL receipt with the same failing helper line, one read/exec/write and `denied=[]`. Neither diagnostic is PASS credit.
- The distinct retry checker dispatches the actual Git operation correctly. Its pinned diff also records failure-evidence validation, truthful accounting and distinct receipt names; manual decode and CPU semantics are unchanged. The retry archive and its distinct copy both passed, with identical receipts, each one archive read, one exact precompiled code-object execution, one exclusive receipt, and `denied=[]`. Actual Capstone output is inert data; no decoder is imported or constructed under the audit hook. Failure receipts retain line numbers through `tb_lineno`/`tb_next`, without audited frame reads.

The tracked outputs are this document and its [report](../reports/cc12_crt_dispatch_first_direct_callee_body_metadata.json). The private final delivery seals actual final Git commit/root/selected trees/blobs, proves exactly those two changes against the frozen base, and verifies current working bytes and index mode/stage/blob against stored blobs. Final original/copy receipts and hashes are in the private handoff. Evidence lives under `local/cc12_crt_dispatch_first_direct_callee_body_metadata/`; prior packet archives are not recursively embedded, and the immutable failed core is pinned with its unchanged member preimages retained in the retry proof.

No Source, build, test, probe, GPR mutation, annotation/ledger edit, Original/game execution, ABI compatibility, OS/audio or runtime credit is claimed. `C05BA1`, prototypes, decompilation, providers, encoded cell contents and other callees remain unqueried.
