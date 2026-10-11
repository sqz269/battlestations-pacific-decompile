# Locale onexit second call: bounded body evidence

The previously accepted physical CALL at `BF6FC5` selects saved function `BFBA68`. Paired per-byte typed metadata covers exactly `BFBA68..BFBA70`: four whole instruction units and five interior addresses. Root accepted that gate before admitting the exact nine code bytes and context-zero text at the four verified starts. Fresh operand-phase metadata and counts match the gate; all 36 raw typed responses remain at epoch 98 / count 64,730 under `bsp.gpr` and `/battlestationspacific.exe`.

Both exact code reads and both text sets agree. Manual decoding and actual detailed Capstone operands agree on:

| Address | Bytes | Saved instruction text |
|---|---|---|
| BFBA68 | 6A 08 | PUSH 0x8 |
| BFBA6A | E8 B2 61 01 00 | CALL 0x00c11c21 |
| BFBA6F | 59 | POP ECX |
| BFBA70 | C3 | RET |

Physical code SHA256 is `6085cbacd1d0f0746985417de59fd6660af45f46b70ec140821178181388a24d`. The E8 signed relative displacement is `+0x000161B2`; `0x00BFBA6F + 0x000161B2 = 0x00C11C21`. Independently observed saved default/effective flows agree on an unconditional call, override `NONE`, target-address `C11C21` and fallthrough `BFBA6F`. The other units have matching fall-through/fall-through/terminator metadata. The physical opcode is established from bytes, without relying on library names or no-return flags.

If entered with ESP=`S` and the first two instructions complete with a 32-bit stack, PUSH writes DWORD8 at `SS:[S-4]`; the near CALL writes `BFBA6F` at `SS:[S-8]` before transferring to `C11C21`. The external call's returned control, registers, ESP and memory are all unknown. Its target object remains wholly opaque and its body was not followed.

If control subsequently reaches `BFBA6F`, let the then-current returned ESP be `U`. A completing POP reads the then-current DWORD through `SS:[U]` into ECX and sets ESP=`U+4`. A completing RET consumes the then-current DWORD through `SS:[U+4]` and leaves ESP=`U+8`. The POPed word is not equated with the earlier pushed8, and the RET word is not equated with an incoming caller return word. No balanced cleanup, provider/locking contract, preserved EAX or return status is assumed.

Saved name `LIBCRT_unmatched_00bfba68`, its saved range, non-thunk and not-no-return flags remain metadata. Actual OS/CRT authenticity, caller-frame continuation, callback ownership, status, ABI compatibility and execution remain unresolved. There were no data/stack reads, repeated prototype, decompile/disasm, target/caller expansion, GPR mutation, Source implementation, build or game/OS/audio execution. Native is closed and no successor is admitted.

The finite standalone proof retains both metadata phases, exact byte/text pairs, all 12 CLI streams, counts, the Root admission, manual/Capstone records, nine immediate positive prerequisite/tool inputs, six actual frozen Git objects, four pure replay helpers and their producers. Original and distinct physical-copy replay both pass with helper pins checked before import and only the selected archive read/result write permitted. Separate delivery checks cover eight actual base/final Git objects and exact current/staged/committed bytes for these two outputs. No prior archives are traversed.

Frozen worker base is `b1591d875a4fe995c60b6def77b7768af306477d`. Source identities are `src=af8efebaa30365fa0f7f2f591c45861a55b4b1b2` and `include=09add3be594e63edcce6a16ec041ed965441e898`; they identify only this worker's frozen base and final state. They provide no Source census, current/future Root equality or implementation/build/runtime credit. The prior entry and prolog proof sets remain immutable.
