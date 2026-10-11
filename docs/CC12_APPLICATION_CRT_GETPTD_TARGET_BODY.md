# CC12 application CRT dispatch target: two-range Native body

Saved entry `00C051B7` is the previously admitted target of the physical call at `00C0522F`. This packet closes only its stored Native body evidence. It supplies no implementation, original ABI, startup or gameplay credit.

## Admission and provenance

| Range | Bytes | Instructions | Code SHA-256 |
|---|---:|---:|---|
| `00C051B7..00C0521E` | 104 | 37 | `129e55689a2a866bc6d492e76bd3b4e0624a57273f2cfc2210c99f0d665c0aa2` |
| `00C05222..00C0522D` | 12 | 6 | `0696a0a5b3e4feb323050302fcad557e9861683d8826b5425aa4a463d8341005` |

The three-byte gap `00C0521F..00C05221` was neither queried nor decoded. Each range was decoded separately at its actual base by an independent bounded x86 decoder and by detailed Capstone 5.0.7; all 43 boundaries, bytes and normalized operands agree with both decoders, and all boundaries/direct destinations agree with saved listing metadata.

Four complete 116-address typed sweeps retain 464 individually hashed raw responses. All four are equal at epoch 98, function count 64,730. The 25 successful Native CLI calls comprise 8 typed requests, 12 counts, 4 paired range-byte reads and one context-zero text sweep at exactly 43 starts. No Native queries were made during this offline closure.

The independent Root whole-unit gate retains both its 56,270-byte JSON receipt (`dc0e29bf81bdb6dfbed5e1fab2af74b8cd278678f948d765c3cae0968a5da913`) and its physical producer script. Both are sealed with the capture producer, original helper preimages, schedules and raw outputs. Automatic provider/target-function objects stay opaque. The saved owner is `LIBCRT_unmatched_00c051b7`, non-thunk, no-return false, with two ranges totaling 116 bytes; the 119-byte min/max envelope is not an admitted body.

## Conditional local contract

32-bit local instruction semantics only; each instruction must execute normally in an appropriate x86 environment; faults, asynchronous interference and changed execution environments are not modeled. Memory addresses retain DS/SS segment roles; overlapping linear locations alias, including all partial overlaps. No flat-segment equality is assumed.

Every read uses memory at that instruction, after all preceding writes on that path. Do not cache data-cell or stack loads. External calls reset all state facts; their return continuations are conditional.

Every one of the ten calls is an unknown external-state boundary: if control resumes locally, returned EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags, memory, segment environment and cleanup remain unknown. Callee names do not prove TLS, allocation, error-state, free or termination behavior. Direct physical call destinations are addresses only.

The computed memory calls read dispatch cells `00CE2258`, `00CE223C`, and `00CE22BC`; their cell contents were not read. Saved EXTERNAL-space scalars `0000016b`, `00000164`, and `00000184` are separate annotations, not physical cell contents or authenticated providers. `CALL EAX` uses the EAX value present at that call. The repeated `00E15AFC` loads and the `0109DE3C` load are read at their own execution times, after preceding pushes/calls and any aliasing writes.

`00C0521A` is a physical E8 call to `00BF9DC8`. Saved CALL_RETURN/effective CALL_TERMINATOR metadata and null fallthrough do not establish actual non-return. If it returns to the arithmetic next address `00C0521F`, this contract stops at the excluded gap.

00C05222 pushes current EDI; 00C05223 resolves M32[DS:00CE22BC] and calls it. If it returns to 00C05229, POP EDI reads returned ESP/memory; 00C0522A MOV EAX,ESI copies returned unknown ESI, not any earlier pointer. POP ESI and RET use subsequent current stack loads.

The table specifies instruction order and current-state operations. Register writes leave other registers unchanged locally; calls replace all state facts. Arithmetic and effective offsets are 32-bit. Memory faults remain possible.

| Address | Physical instruction | Conditional effect |
|---|---|---|
| `00C051B7` | `push ESI` | v := ESI evaluated in pre-instruction state; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any admitted data address or earlier stack slot; flags unchanged. |
| `00C051B8` | `push EDI` | v := EDI evaluated in pre-instruction state; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any admitted data address or earlier stack slot; flags unchanged. |
| `00C051B9` | `call M32[0x00CE2258]` | target := M32[0x00CE2258] in pre-call state (computed memory operand uses DS); ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C051BF; transfer to target. The target is resolved before the return-address push, even if the write aliases its dispatch cell. If control later resumes at the arithmetic return address: all EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No callee preservation, success, termination or identity is inferred. |
| `00C051BF` | `push M32[0x00E15AFC]` | v := M32[0x00E15AFC] evaluated in pre-instruction state; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any admitted data address or earlier stack slot; flags unchanged. |
| `00C051C5` | `mov EDI, EAX` | EDI := EAX evaluated now; flags unchanged. |
| `00C051C7` | `call 0x00C05070` | target := 0x00C05070 in pre-call state (computed memory operand uses DS); ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C051CC; transfer to target. The target is resolved before the return-address push, even if the write aliases its dispatch cell. If control later resumes at the arithmetic return address: all EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No callee preservation, success, termination or identity is inferred. |
| `00C051CC` | `call EAX` | target := EAX in pre-call state (computed memory operand uses DS); ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C051CE; transfer to target. The target is resolved before the return-address push, even if the write aliases its dispatch cell. If control later resumes at the arithmetic return address: all EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No callee preservation, success, termination or identity is inferred. |
| `00C051CE` | `mov ESI, EAX` | ESI := EAX evaluated now; flags unchanged. |
| `00C051D0` | `test ESI, ESI` | t := ESI & ESI; ZF/SF/PF := predicates of t; CF=OF=0; AF undefined. No register/memory write. Flags survive intervening POP instructions. |
| `00C051D2` | `jne 0x00C05222` | If ZF is 0, go to 0x00C05222; otherwise go to 0x00C051D4. State unchanged. |
| `00C051D4` | `push 0x00000214` | v := 0x00000214 evaluated in pre-instruction state; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any admitted data address or earlier stack slot; flags unchanged. |
| `00C051D9` | `push 0x00000001` | v := 0x00000001 evaluated in pre-instruction state; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any admitted data address or earlier stack slot; flags unchanged. |
| `00C051DB` | `call 0x00C0485A` | target := 0x00C0485A in pre-call state (computed memory operand uses DS); ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C051E0; transfer to target. The target is resolved before the return-address push, even if the write aliases its dispatch cell. If control later resumes at the arithmetic return address: all EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No callee preservation, success, termination or identity is inferred. |
| `00C051E0` | `mov ESI, EAX` | ESI := EAX evaluated now; flags unchanged. |
| `00C051E2` | `test ESI, ESI` | t := ESI & ESI; ZF/SF/PF := predicates of t; CF=OF=0; AF undefined. No register/memory write. Flags survive intervening POP instructions. |
| `00C051E4` | `pop ECX` | ECX := M32[SS:ESP] now; ESP := ESP+4 mod 2^32; flags unchanged. No earlier pushed value is presumed. |
| `00C051E5` | `pop ECX` | ECX := M32[SS:ESP] now; ESP := ESP+4 mod 2^32; flags unchanged. No earlier pushed value is presumed. |
| `00C051E6` | `je 0x00C05222` | If ZF is 1, go to 0x00C05222; otherwise go to 0x00C051E8. State unchanged. |
| `00C051E8` | `push ESI` | v := ESI evaluated in pre-instruction state; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any admitted data address or earlier stack slot; flags unchanged. |
| `00C051E9` | `push M32[0x00E15AFC]` | v := M32[0x00E15AFC] evaluated in pre-instruction state; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any admitted data address or earlier stack slot; flags unchanged. |
| `00C051EF` | `push M32[0x0109DE3C]` | v := M32[0x0109DE3C] evaluated in pre-instruction state; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any admitted data address or earlier stack slot; flags unchanged. |
| `00C051F5` | `call 0x00C04FDE` | target := 0x00C04FDE in pre-call state (computed memory operand uses DS); ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C051FA; transfer to target. The target is resolved before the return-address push, even if the write aliases its dispatch cell. If control later resumes at the arithmetic return address: all EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No callee preservation, success, termination or identity is inferred. |
| `00C051FA` | `pop ECX` | ECX := M32[SS:ESP] now; ESP := ESP+4 mod 2^32; flags unchanged. No earlier pushed value is presumed. |
| `00C051FB` | `call EAX` | target := EAX in pre-call state (computed memory operand uses DS); ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C051FD; transfer to target. The target is resolved before the return-address push, even if the write aliases its dispatch cell. If control later resumes at the arithmetic return address: all EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No callee preservation, success, termination or identity is inferred. |
| `00C051FD` | `test EAX, EAX` | t := EAX & EAX; ZF/SF/PF := predicates of t; CF=OF=0; AF undefined. No register/memory write. Flags survive intervening POP instructions. |
| `00C051FF` | `je 0x00C05219` | If ZF is 1, go to 0x00C05219; otherwise go to 0x00C05201. State unchanged. |
| `00C05201` | `push 0x00000000` | v := 0x00000000 evaluated in pre-instruction state; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any admitted data address or earlier stack slot; flags unchanged. |
| `00C05203` | `push ESI` | v := ESI evaluated in pre-instruction state; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any admitted data address or earlier stack slot; flags unchanged. |
| `00C05204` | `call 0x00C050F8` | target := 0x00C050F8 in pre-call state (computed memory operand uses DS); ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C05209; transfer to target. The target is resolved before the return-address push, even if the write aliases its dispatch cell. If control later resumes at the arithmetic return address: all EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No callee preservation, success, termination or identity is inferred. |
| `00C05209` | `pop ECX` | ECX := M32[SS:ESP] now; ESP := ESP+4 mod 2^32; flags unchanged. No earlier pushed value is presumed. |
| `00C0520A` | `pop ECX` | ECX := M32[SS:ESP] now; ESP := ESP+4 mod 2^32; flags unchanged. No earlier pushed value is presumed. |
| `00C0520B` | `call M32[0x00CE223C]` | target := M32[0x00CE223C] in pre-call state (computed memory operand uses DS); ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C05211; transfer to target. The target is resolved before the return-address push, even if the write aliases its dispatch cell. If control later resumes at the arithmetic return address: all EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No callee preservation, success, termination or identity is inferred. |
| `00C05211` | `or M32[ESI+0x00000004], 0xFFFFFFFF` | M32[ESI+0x00000004] is read now, ORed with 0xFFFFFFFF, then written as 0xFFFFFFFF; CF=OF=ZF=0, SF=PF=1, AF undefined. The read can fault; this is not a write-only store. |
| `00C05215` | `mov M32[ESI+0x00000000], EAX` | M32[ESI+0x00000000] := EAX evaluated now; flags unchanged. |
| `00C05217` | `jmp 0x00C05222` | Go to 0x00C05222; state unchanged. |
| `00C05219` | `push ESI` | v := ESI evaluated in pre-instruction state; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any admitted data address or earlier stack slot; flags unchanged. |
| `00C0521A` | `call 0x00BF9DC8` | target := 0x00BF9DC8 in pre-call state (computed memory operand uses DS); ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C0521F; transfer to target. The target is resolved before the return-address push, even if the write aliases its dispatch cell. If control later resumes at the arithmetic return address: all EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No callee preservation, success, termination or identity is inferred. STOP the possible continuation at 00C0521F: it is outside the admitted ranges. |
| `00C05222` | `push EDI` | v := EDI evaluated in pre-instruction state; ESP := ESP-4 mod 2^32; M32[SS:ESP] := v. The write may alias any admitted data address or earlier stack slot; flags unchanged. |
| `00C05223` | `call M32[0x00CE22BC]` | target := M32[0x00CE22BC] in pre-call state (computed memory operand uses DS); ESP := ESP-4 mod 2^32; M32[SS:ESP] := 0x00C05229; transfer to target. The target is resolved before the return-address push, even if the write aliases its dispatch cell. If control later resumes at the arithmetic return address: all EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, flags and memory are fresh unknowns; segment environment and cleanup are unproved. No callee preservation, success, termination or identity is inferred. |
| `00C05229` | `pop EDI` | EDI := M32[SS:ESP] now; ESP := ESP+4 mod 2^32; flags unchanged. No earlier pushed value is presumed. |
| `00C0522A` | `mov EAX, ESI` | EAX := ESI evaluated now; flags unchanged. |
| `00C0522C` | `pop ESI` | ESI := M32[SS:ESP] now; ESP := ESP+4 mod 2^32; flags unchanged. No earlier pushed value is presumed. |
| `00C0522D` | `ret` | target := M32[SS:ESP] now; ESP := ESP+4 mod 2^32; transfer to target; flags unchanged. Neither caller identity nor restoration of entry stack/register values is proved. |

## Offline replay and delivery

The supplied ZIP is replayed under a read-only audit guard after every manifest member hash has been checked, including all helpers, producers, capture bodies and preimages. The pure parser uses only the supplied ZIP after its standard-library preload boundary; subsequent imports, subprocesses, network access and non-ZIP filesystem access are denied. Original and copied ZIPs have different filesystem identities and equal SHA-256; both independent guarded processes produce equal results. The replay recomputes the manual decoder and contract, and checks retained actual Capstone output without importing native Capstone under the guard.

Archive: `local/cc12_application_crt_getptd_target_body/evidence.zip` (645339 bytes), SHA-256 `0e03ef428bfc130cd420f7cd5edba6e2af96c4fa228373318efaa6a9f1d7e19f`.

Frozen worker HEAD: `5a47b51a151b70259880597d519486d7889c8bb9`. Twelve immediate positive paths are connected through eight actual Git object payloads to that commit. The frozen input documents/reports are retained as immediate evidence only; their referenced archives are not opened.

Source identity qualifiers: `src=af8efebaa30365fa0f7f2f591c45861a55b4b1b2`, `include=09add3be594e63edcce6a16ec041ed965441e898`. These are worker base/delivery subtree identities only; no Source content census and no equality to current Root Source is asserted.

Only this document and its companion JSON report are delivered as tracked changes. Post-seal staged/final Git receipts pin their exact bytes and blobs, the twelve unchanged immediate input objects and the unchanged worker Source subtree IDs; those delivery receipts remain outside the ZIP to avoid circular hashes. No source, CMake, ledger or saved GPR mutation; no build, fixture, ABI, game or OS/audio validation. The wider startup/gameplay goal remains open.
