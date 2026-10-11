# CC12 locale exit target entry operand

The four bytes at BF6FF5..BF6FF8 are **FF 74 24 04**, decoded as `PUSH dword ptr [ESP + 0x4]`. This packet establishes that one instruction's conditional transfer. Its source stackword and every incoming register value remain unknown; no callee return, callback registration or initializer completion is established.

## Fresh gate and exact capture

Before operand access, a fresh typed gate verified saved `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86:LE:32:default, image base 00400000. Exact and containing function entries were both BF6FF5, currently named `_atexit`, with no_return=false, is_thunk=false and no direct thunk target. The complete body range metadata remained BF6FF5..BF7006, 18 addresses; no decoding of the remaining body was admitted.

All four exact/containing instruction/code-unit records were the same InstructionDB at BF6FF5 with length 4. Default/effective flow was FALL_THROUGH to BF6FF9, override NONE, no fallthrough override and empty targets. The gate passed before any new operand bytes were read. Modification 98 and count 64730 were observed afresh and remained stable; equality to an inherited epoch was not assumed.

The bounded sequence was typed-before, count-before, code-before, context 0-text-before, code-after, context 0-text-after, count-after, typed-after. Three count/code/text pairs are byte-equal, and the typed raw responses plus all identity bookends match. Both text captures return only BF6FF5 with empty neighbors. Native access is closed. Exactly four unique code bytes and zero data bytes were read. BF6FF9 bytes/text and target-site metadata were not queried; neither were callers, another callee/target, IAT/data, operand pointers, prototypes or function decompilation. No Ghidra mutation, flow/no-return/listing/body repair occurred.

The predecessor's proto command had supplied unsolicited body metrics, a basic-block hash and a callee label. Its whole capture remains explicitly **not strictly entry-only**. Those raw historical fields stay physically retained and opaque, with no semantic credit or new-target admission. This packet made no proto query and did not use that label to expand scope.

## Conditional instruction contract

Let **W** be the unknown ESP on entry to BF6FF5, **M** the unknown entry memory and **K** the DWORD in M at SS:((W+4) modulo 2^32). Conditional on control reaching this instruction in ordinary decoded 32-bit operand/address mode with a normal 32-bit stack, and successful instruction completion:

| Effect | Result |
|---|---|
| Source effective address | SS:(W+4), using entry ESP **before** decrement |
| Source value | K, unobserved |
| ESP | (W-4) modulo 2^32 |
| Explicit stack store | DWORD[SS:((W-4) modulo 2^32)] = K |
| Next EIP | BF6FF9 |
| Other GPRs | Preserved relative to unknown entry |
| EFLAGS | Unmodified by this instruction |

The conditional architectural account contains one four-byte source read and one four-byte stack write; no stack data were fetched by the analysis. FF selects group 5, ModR/M 74 selects /6 PUSH with disp8 and SIB, SIB 24 selects ESP with no index, and disp8=04 gives +4. No prefix changes the assumed decoded 32-bit operand/address mode. This is not a successful runtime observation or a fault-path contract.

BF6FF5 is an independently admitted, unknown callee-entry state. Earlier EAX=E1768C is **not** imported here. Preserving EAX means preserving its unknown entry value. K is not identified as CD9410, CC8D44, a valid callback or any other concrete value. W/K are not equated with the prior caller's stack symbols, POP-entry U or RET-entry V.

The accepted prior CALL still only encodes BF6FF5 and return address CC8D44. The prior POP's ECX=CD9410 hypothesis still requires **both** returnedESP=S-4 and unchanged DWORD[SS:(S-4)]=CD9410. The accepted RET's EIP=DWORD[SS:V], ESP=V+4 remain conditional on its own successful decoded execution and unknown entry. This PUSH proves no post-callee ESP/GPR/EFLAGS/stackword state, actual return, registration or initializer completion. `_atexit`, its inherited declaration and no_return=false remain database claims, not CRT identity, ABI or cleanup-policy proof.

## Frozen Source and portable evidence

Source is frozen at `febb7a6be871e15092832d58ad68a5717e246f48`: 4217 closed src/include/cmake/CMakeLists paths plus 77 named dependencies, 4294 actual Git blob occurrences and 11 frozen commit/tree objects. All selected physical files equal their frozen Git blobs. Complete query specifications/matches and 23 bounded Source excerpts are retained and replayed; matches include comments/other sites and carry no full-callee semantic credit.

Relative to predecessor 9316daef07de49f6f0637442c435e990d15e83d0, 4215 Source paths match. Only `include/bsp/native_particle_model_construction.hpp` and `src/native_particle_model_construction.cpp` differ, through independently accepted 8984d54eb218a44fb6b1ef551d9833ecc5bc24d3. Two additional actual commit/root-tree objects establish that 8984 and frozen febb have identical src/include/cmake trees and CMakeLists blob, covering all 4217 Source paths. Earlier evidence is not rebased and its build/runtime credit is not transferred.

The new Root raw-key Source3356 manifest is retained at 1271280 bytes, SHA256 e89f7488238a451562d922dba4ecba2e05dba3bb6dc0093fdb5e5a98eb4c8c6a. The overlap is 3346 raw-equal files plus 5 retained CRLF-only variants, 3351 LF-equal Source paths. Only CRLF pairs become LF; all differing raw physical bytes and original pins remain. Nine named/artifact pins outside this Source census stay opaque and unopened. The older d171 Application manifest remains in the predecessor at its original epoch. No sound Source delta, historical build or runtime transfer is claimed.

Accepted predecessor commit f281daa651557f08bfe342f61a0ece6b3d58248a, its 259858593-byte core ZIP, 646114-byte actual-output delivery ZIP and all 22 helper originals are physically retained. Their entire predecessor ZIP/helper graph is recursively replayed. New original and distinct physical-copy guard runs PASS. All 24 current helper files must remain in `Tools/` beside the ZIP. Physical helper hashes are checked before imports execute; the audit is active before helper execution, and only verified in-memory helper code runs. Replay reads only its supplied ZIP, writes only the result and denies external files/processes/network/native loads. Retained producers never run during replay.

Core archive: 284915822 bytes, SHA256 `2c5388ba9651c57307d03417d63fc5c6ed3d56cac376014882a6f66b32f91174`, 8599 entries. Run `python Tools/guarded_replay.py evidence.zip --output result.json`. The exact-two-output Git delivery archive is captured after the immutable commit; it verifies current/staged/committed bytes and has a separately reported pin to avoid circular hashing. No C++/CMake change, build, test, probe, game, OS or audio execution is claimed.

## Separately gated continuation

The next candidate is **metadata only at BF6FF9**, after separate Root admission, a fresh lease and identity checks. Nothing there is admitted or read now. Later bytes, remaining body, callers, targets, callee labels, pointers, IAT/data and prototypes remain excluded. Full-callee effects and actual return/registration/completion remain unknown.
