# CC12 CRT dispatch first direct target: saved body

This packet closes stored Native body evidence at `00C05070`, whose entry was independently accepted by Root as the target of the admitted physical call at `00C051C7`. It supplies conditional local instruction semantics; the routing name does not authenticate TLS, locking or provider behavior.

## Admission and provenance

The saved owner is `LIBCRT_unmatched_00c05070`, non-thunk, no-return false, with one complete 50-byte body range `00C05070..00C050A1`. Two complete per-byte listing sweeps were reviewed before operands were admitted. Root independently qualified all 50 bytes as 16 whole InstructionDB units, all at/containing aliases, NONE flow overrides, default/effective flow flags and the two distinct saved EXTERNAL annotations. Its 22,143-byte gate (`c2b7913d442c1c6d927c52416b363b3008923ab1a70d8ad1631317a4171c02af`) and physical producer are sealed.

The admitted paired 50-byte reads agree. A bounded independent opcode/ModRM decoder and actual detailed Capstone 5.0.7 at base `00C05070` agree on all 16 boundaries, bytes, operands and encoding-field offsets/sizes. Saved context-zero text was requested at those 16 verified starts only. A closing complete per-byte sweep matches both earlier sweeps: 150 retained raw responses at epoch 98 / function count 64,730. Eleven successful Native CLI calls comprise three typed sweeps, five count checks, two exact byte reads and one text sweep. No decompilation, prototype, data-cell, provider or callee query was made in this packet.

Code SHA-256: `c56216e301258d74d1f73f4e30645d811c2e7a3cda5bef15b09e94735394ba81`. Automatic target-function objects remain opaque: three envelopes per sweep.

## Conditional local contract

Conditional local 32-bit x86 semantics only: each instruction must still have its captured encoding when it executes normally in an appropriate environment. Faults, changed code and asynchronous effects are not modeled. Addresses retain DS/SS segment roles; no flat-segment equality is presumed. Local operations leave unmentioned registers unchanged; each external call discards all prior returned-state assumptions.

Every operand load and stack pop observes memory at that instruction, after preceding writes on that path. PUSH reads its source before decrementing ESP and writing the new stack slot. A computed CALL reads its dispatch cell before pushing the return address. Do not cache either read of 00E15B00 across calls or pushes.

The computed calls at `00C05077` and `00C05098` physically read dispatch cells `00CE20BC` and `00CE20B4`. Their contents remain unread. Saved EXTERNAL scalars `00000104` and `00000102` are different metadata objects; they do not identify those cells' contents or authenticate providers. The physical direct call at `00C05089` selects `00C04FDE`; its target content and behavior remain unqueried. Data cells `00E15B00` and `0109DE38` are also unread.

Each of the three calls is an unknown state boundary. On a conditional local return, EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and all memory are unknown; neither preservation nor cleanup is inferred. This includes memory underlying future operands, stack slots and code.

If C05077 resumes at C0507D, ESI is set to that returned EAX and tested. A nonzero value takes the local short path to C0509E, where EAX is assigned that ESI before POP ESI and RET. The zero path calls C04FDE at C05089; returned ESP/EBP/EAX/ESI and all other state are unknown. C0508E pops from current returned ESP; C0508F sets ESI from current EAX. C05091 pushes that ESI; C05092 rereads 00E15B00 after that push, preserving possible aliases.

If C05098 resumes at C0509E, MOV EAX,ESI uses returned unknown ESI. It does not promise the value copied or pushed before that call. POP ESI and RET subsequently read current stack memory.

| Address | Decoded instruction | Conditional effect |
|---|---|---|
| `00C05070` | `push ESI` | v := ESI evaluated before changing ESP; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any later data/dispatch-cell read or earlier stack slot, including partial overlaps. Flags unchanged. |
| `00C05071` | `push M32[DS:0x00E15B00]` | v := M32[DS:0x00E15B00] evaluated before changing ESP; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any later data/dispatch-cell read or earlier stack slot, including partial overlaps. Flags unchanged. |
| `00C05077` | `call M32[DS:0x00CE20BC]` | target := M32[DS:0x00CE20BC] before the return-address push; ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C0507D; transfer to target. If it later resumes at 0x00C0507D, all returned EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No preservation, provider identity or result meaning is assumed. |
| `00C0507D` | `mov ESI, EAX` | ESI := EAX in current state; flags unchanged. |
| `00C0507F` | `test ESI, ESI` | t := ESI & ESI; ZF/SF/PF := predicates of t; CF=OF=0; AF undefined. No register or memory writes. |
| `00C05081` | `jne 0x00C0509E` | If ZF=0, go to 0x00C0509E; otherwise go to 0x00C05083; state unchanged. |
| `00C05083` | `push M32[DS:0x0109DE38]` | v := M32[DS:0x0109DE38] evaluated before changing ESP; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any later data/dispatch-cell read or earlier stack slot, including partial overlaps. Flags unchanged. |
| `00C05089` | `call 0x00C04FDE` | target := 0x00C04FDE before the return-address push; ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C0508E; transfer to target. If it later resumes at 0x00C0508E, all returned EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No preservation, provider identity or result meaning is assumed. |
| `00C0508E` | `pop ECX` | ECX := M32[SS:ESP] at this instruction; ESP := ESP+4 mod 2^32; flags unchanged. No earlier pushed value is presumed. |
| `00C0508F` | `mov ESI, EAX` | ESI := EAX in current state; flags unchanged. |
| `00C05091` | `push ESI` | v := ESI evaluated before changing ESP; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any later data/dispatch-cell read or earlier stack slot, including partial overlaps. Flags unchanged. |
| `00C05092` | `push M32[DS:0x00E15B00]` | v := M32[DS:0x00E15B00] evaluated before changing ESP; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any later data/dispatch-cell read or earlier stack slot, including partial overlaps. Flags unchanged. |
| `00C05098` | `call M32[DS:0x00CE20B4]` | target := M32[DS:0x00CE20B4] before the return-address push; ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C0509E; transfer to target. If it later resumes at 0x00C0509E, all returned EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No preservation, provider identity or result meaning is assumed. |
| `00C0509E` | `mov EAX, ESI` | EAX := ESI in current state; flags unchanged. |
| `00C050A0` | `pop ESI` | ESI := M32[SS:ESP] at this instruction; ESP := ESP+4 mod 2^32; flags unchanged. No earlier pushed value is presumed. |
| `00C050A1` | `ret` | target := M32[SS:ESP] now; ESP := ESP+4 mod 2^32; transfer to target; flags unchanged. Entry-stack restoration, caller identity and a valid return destination are unproved. |

No TLS, locking, caching, allocation, decoding-provider, argument or return-type meaning is authenticated. Saved function names, no-return annotations and EXTERNAL scalars do not establish original ABI or runtime behavior.

## Replay and exact delivery

The supplied archive retains all Native captures/schedules, four physical helper bodies and their preimages, Root's listing gate and producer, actual detailed Capstone output, and twelve immediate accepted-entry/PRIMARY and CLI-producer inputs connected through eight actual Git object payloads to the frozen worker commit. Referenced predecessor archives were not opened; Source subtree objects carry revision identity only.

Original and physically distinct copied ZIPs have equal hashes and produce equal guarded replay results. Each process checks every member hash before exactly one pinned pure-parser execution and performs exactly one supplied-ZIP read. Post-preload imports, non-ZIP filesystem access and external effects are forbidden. Replay recomputes the manual decoder, metadata and contract and checks retained actual Capstone output; native Capstone is not imported inside the guard.

Archive: `local/cc12_application_crt_getptd_first_direct_target_body/evidence.zip` (547224 bytes), SHA-256 `b3502946501de284814b07c492c15e577e0e1cd312a2dab83b066fd6693f7a98`.

Frozen worker HEAD: `f8b434bd2ac7a44057deba47667206fe15aebecf`. Worker base/delivery subtree identities: `src=af8efebaa30365fa0f7f2f591c45861a55b4b1b2`, `include=09add3be594e63edcce6a16ec041ed965441e898`. No Source census or equality to current Root Source is asserted.

Only this document and its companion JSON are tracked outputs. Separate staged/final receipts pin exact working/index/commit bytes and blobs, twelve unchanged immediate input objects and unchanged worker Source IDs. They remain outside the archive to avoid circular hashes. No Source, CMake, ledger or GPR mutation; no build, new test, original ABI, startup, runtime or gameplay credit. The wider reconstruction goal remains open.
