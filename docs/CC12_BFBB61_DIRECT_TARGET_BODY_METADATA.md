# CC12 BFBB61 direct-target body evidence

The accepted 60-byte saved owner `00BFBB61–00BFBB9C` contains 25 whole instructions. Two independent metadata walks, paired exact body bytes, paired context-zero text and a closing metadata pass agree. The independent full-field manual record was written before the first fresh detailed Capstone 5.0.7 import and construction; all 25 instructions match on the first comparison.

The body contains two direct calls to opaque numeric targets `00BFFB8B` and `00BF66EF`, two conditional branches within the admitted owner, explicit DS memory references and two near returns. The saved name `__get_winmajor` is not authenticated as a library identity or behavior.

## Admission and provenance

Actual Main is frozen at `10e917580e9c279586ae1de13daa66cea4094c20`. The exact eight positive inputs are the accepted BFBB61 ENTRY_METADATA and ENTRY_METADATA_PRIMARY document/report pairs, `config/target.json` and the three CLI source files. Fourteen actual Git commit/tree/blob objects, exact working bytes after LF normalization and stage-zero index blobs were captured before Native. Source subtrees are retained as identities only.

Phase 1 independently walked from `00BFBB61` twice, deriving every next cursor only from each current original InstructionDB length. Both walks cover exactly 60 bytes and stop at `00BFBB9D` without querying it. The four listing aliases, full containing owner, functions, default/effective flow, target arrays and automatic call companions remain in every original typed payload. Phase 1 used 54 CLI calls and 262 HTTP responses. Root reviewed all 50 complete originals before admitting body bytes.

Before phase 2, all 763 initial Root artifacts were physically copied and readback-verified: 752 metadata preimages equal to their worker originals, ten listed top files and the admission control. A separate completed UTC chronology precedes phase 2, whose producer reverified every local copy before the first Native command. Phase 2 captured paired exact 60-byte reads, paired context-zero text at only the 25 admitted starts, a closing typed pass and count bookends: seven CLI calls and 47 HTTP responses. All 25 closing original replies equal both initial originals.

Across all 61 Native calls, each of the three CLI files was physically read before and after every call, with 122 separate observations and retained content-addressed raw payloads. Project/program remained `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`, language `x86:LE:32:default`, modification number 98 and function count 64,730. The paired 60-byte body SHA-256 is `ae8c7e0db8242c255db52267491487b5e32f5f934b9bc1eaf0b832463dc6a3a0`.

## Admitted physical instructions

| Address | Bytes | Saved text |
| --- | --- | --- |
| `00BFBB61` | `8B442404` | `MOV EAX,dword ptr [ESP + 0x4]` |
| `00BFBB65` | `56` | `PUSH ESI` |
| `00BFBB66` | `33F6` | `XOR ESI,ESI` |
| `00BFBB68` | `3BC6` | `CMP EAX,ESI` |
| `00BFBB6A` | `751D` | `JNZ 0x00bfbb89` |
| `00BFBB6C` | `E81A400000` | `CALL 0x00bffb8b` |
| `00BFBB71` | `56` | `PUSH ESI` |
| `00BFBB72` | `56` | `PUSH ESI` |
| `00BFBB73` | `56` | `PUSH ESI` |
| `00BFBB74` | `56` | `PUSH ESI` |
| `00BFBB75` | `56` | `PUSH ESI` |
| `00BFBB76` | `C70016000000` | `MOV dword ptr [EAX],0x16` |
| `00BFBB7C` | `E86EABFFFF` | `CALL 0x00bf66ef` |
| `00BFBB81` | `83C414` | `ADD ESP,0x14` |
| `00BFBB84` | `6A16` | `PUSH 0x16` |
| `00BFBB86` | `58` | `POP EAX` |
| `00BFBB87` | `5E` | `POP ESI` |
| `00BFBB88` | `C3` | `RET` |
| `00BFBB89` | `393584DD0901` | `CMP dword ptr [0x0109dd84],ESI` |
| `00BFBB8F` | `74DB` | `JZ 0x00bfbb6c` |
| `00BFBB91` | `8B0D90DD0901` | `MOV ECX,dword ptr [0x0109dd90]` |
| `00BFBB97` | `8908` | `MOV dword ptr [EAX],ECX` |
| `00BFBB99` | `33C0` | `XOR EAX,EAX` |
| `00BFBB9B` | `5E` | `POP ESI` |
| `00BFBB9C` | `C3` | `RET` |

All instructions have no prefixes and use 32-bit operands/address calculation. The report includes opcode, ModR/M, SIB, displacement/immediate field offsets and sizes, register and memory operands, operand access, explicit/implicit register access, independent numeric EFLAGS masks and symbolic effects, groups and numeric transfers. The decoder-free physical helper accepts only the admitted range and the observed encoding forms. Saved default/effective flows and target arrays are retained independently and match the physical flow shapes at every unit.

`75 1D` at `00BFBB6A` adds signed +29 to next offset `00BFBB6C`, giving `00BFBB89`. `74 DB` at `00BFBB8F` adds signed -37 to `00BFBB91`, giving `00BFBB6C`. Both branch targets are admitted instruction starts. The first E8 adds +16,410 to encoded return offset `00BFBB71`, giving `00BFFB8B`; the second adds -21,650 to `00BFBB81`, giving `00BF66EF`. Neither callee was queried. Their saved `LIBCRT_unmatched_00bffb8b` and `LIBCRT_unmatched_00bf66ef` labels, no_return=false flags and nonthunk status remain metadata.

## Current-state and ABI limits

The entry MOV reads four bytes through current SS at explicit `(ESP+4) mod 2^32` and writes EAX. That explicit address calculation does not itself require SS.B=1. Normal completion preserves ESP and EFLAGS. Current memory value, readability, linear address and segment equality are unknown.

Renew current SS.B=1 at every PUSH, POP, CALL and RET when describing its implicit stack operation in terms of ESP. Under normal successful completion, each PUSH/CALL writes four bytes through current SS at `(ESP−4) mod 2^32`; each POP/RET reads four bytes through current SS and advances ESP by four. Code/segment state, access permissions, interrupts and faults remain outside these normal-completion statements.

Both E8 operations are opaque boundaries. Encoded return offsets do not establish that a callee returns. After any return, current registers, flags, segment state and memory values are unestablished. In particular, the earlier `XOR ESI,ESI` does not prove that the five later ESI pushes carry zero. The EAX-based store after that call uses the current unknown EAX and can fault. No argument layout, caller/callee preservation or aggregate stack balance is inferred across either call.

`C7 00 16 00 00 00` writes the four-byte constant 22 through current DS at current EAX. The absolute operands `0109DD84` and `0109DD90` also use current DS; neither cell was read. The later EAX-based store uses the value loaded into ECX. Encoded addresses establish no pointer validity, writable mapping, nonaliasing or provider/getter meaning; a store may alias stack memory. Therefore even the no-call path does not prove restoration of the caller's ESI or a safe return target.

`ADD ESP,20` is explicit 32-bit arithmetic, separate from an implicit stack operation. CMP and ADD affect AF/CF/OF/PF/SF/ZF; XOR changes PF/SF/ZF, clears CF/OF and leaves AF undefined. JZ/JNZ test current ZF. The two self-XOR instructions establish a local zero result on normal completion; they do not authenticate the function's overall return contract. Both RET targets come from current SS stack state and remain unknown.

## Histories and validation boundary

Current metadata, Root-artifact copy, body capture, manual decode and actual Capstone comparison all passed their first producer executions. Current worker inspection/display counts are 0/2: the initial positive-history display truncation and a later combined pure-helper display truncation remain separate incidents. The initial publication and helper bytes are retained before this history update; no Native or decoder capture was repeated. Root's current metadata gate passed its first child execution; its two prior read-only schema inspection failures and zero display truncations are retained unchanged. Accepted BFBB61 ENTRY and earlier C04EFB histories retain their original scopes and are not counted as current packet failures.

No Source reconstruction or absence census, Ghidra mutation, build, new test, probe, Original/game execution or OS/audio experiment occurred. `00BFBB9D`, both callees, absolute memory cells, providers and prototypes remain unqueried. This is static physical and provenance evidence, without ABI or game validation.

The private proof binds the final actual commit, its single frozen parent, exactly two public outputs and all ten input/output working-LF/index/blob checks using 20 actual Git objects. The supplied pure helper is fully read and pinned before execution. Distinct original/copy strict runs allow one ZIP read, one exact precompiled helper execution and one exclusive receipt write, denying every other audit event. No archived code, earlier archive or decoder is executed under that hook.
