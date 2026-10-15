# Actual nonzero DWORD range movement

Complete `00468F60..00468F8F` covers 48 bytes / 18 instructions, three blocks and
one physical outgoing transfer. Prior placeholder `STL_inst_00468f60` is preserved
in the naming evidence and annotation prior-value log. New descriptive hypothesis
`STL_DwordRange_MoveNonzero_00468F60` identifies the established behavior; it is
not a recovered symbol or nominal pointee/container type.

Original ABI is stdcall(first,last,destination), stack entry+4/+8/+Ch, RET0Ch.
Incoming ECX/EDX are unused; ESI is preserved. Read last, first, destination in
that order before subtracting the two addresses modulo32. Reproduce SAR2 result
bits explicitly without C++ signed overflow or signed-shift assumptions. Multiply
those bits by4 modulo32, then capture destination+bytes in the saved result before
the CRT can report, return or throw. The new ordinary interface exposes these
pointer values and calling-convention keyword, with no private-frame alias claim.

JZ at `00468F7C` uses ZF from SAR at `00468F6E`: intervening LEA/PUSH preserve
flags. Consequently EVERY nonzero shifted word count reaches the real current
SDK, including negatives. A raw difference of FFFFFFFF yields wordsFFFFFFFF and
bytesFFFFFFFC and reaches the CRT; raw differences0..3 yield words0 and skip it.
No signed-positive, alignment, order, count, null or additional validation guard
is introduced. Accepted `00467D90` has a signed-positive guard and cannot supply
this complete body, so no delegation to it is made.

`00468F82 CALL 00BF67A7` is the only outgoing native transfer. Original pushes
count=bytes, source=first, destination_size=bytes, destination, then clears those
four arguments with ADD ESP,10h. Source directly
invokes actual current SDK `memmove_s(destination,bytes,first,bytes)` and ignores
its status. Both paths return the result captured BEFORE the CRT, without a
caller-slot reload. Returning diagnostics and exceptions retain the concrete SDK
behavior; original static CRT errno/handler/global/exception/SEH/fault identity
is not asserted. The body has no local EH frame, allocation, pool action, cleanup,
ownership or rollback. Reached raw backing must satisfy the actual SDK service.
No callback, stub, host container or implicit Source/native argument-home/private-
frame alias is supplied.

The four parent sites are in `0046AFB0`; each child RET0Ch cleans its own triple.
The parent may put its tree in ECX before some transfers; ECX is not a child input.

| Native transfer | Pushed destination,last,first | Continuation |
| --- | --- | --- |
| `0046B06E CALL 00468F60` | EBP,EAX,ECX | captured result feeds the following child arguments |
| `0046B08D CALL 00468F60` | EAX,current end EAX,ECX | EAX is overwritten by current tree.begin |
| `0046B0F0 CALL 00468F60` | computed EAX,EBP,EDI | EAX is overwritten by current tree.end |
| `0046B138 CALL 00468F60` | EBP,EBP,EBX | result publishes tree.end at `0046B140` |

## Typed flow and verification

The cached lookup listed two pool edges, while live prototype metrics/callees and
the complete listing expose only the real SDK target. Read-only typed metadata
at ALL 18 instruction starts verified a complete contiguous 48-byte body,
modification775, no override, matching default/effective flows, and only the
single transfer above. No flow repair or pool dependency was invented. Receipts:
`local/goal_oct15_dword_copy_nonzero_preflight_all_flow.json` and `_callers.json`.
All 48 live bytes match the installed PE. The owned ledger and report retain this
cached/live distinction; naming annotations do not modify flow.

The five actual native transfer rows are verified separately from SDK Source
composition. Locked annotation preserves old values/comments/evidence, saves the
project and refreshes the affected export with readback recorded in the report.
Unique CMake registration requires an immediate unleased check without a whole-
file lease. Prior-value annotation log:
`local/ghidra-annotations-20261015T221222Z.json`.

Required normal `scripts/build.ps1` at
`32e7e0edf4bd2da8377ac2fb58682d2be3e400a3` passed strict Win32 and all 3 existing
CTests in 8.20s. Log: `local/goal_oct15_dword_move_nonzero_build.log`.
Source/header hashes remain unchanged from that draft. The primary independently
reviewed all 18 native instructions and the exact Source/header, including the
negative guard and captured result, without a mismatch or requested Source edit.

Focused actual-object inspection found a 97-byte stdcall entry. It reads last,
first and destination in that relative order, constructs the explicit SAR2 bits
through OR+18h and captures the wrapped result in ESI+25h before SDK effects.
JE+28h uses the OR's ZF and skips only zero words; no signed-positive guard is
present. The real current SDK is inlined: its byte-count test+2Ah retains all
negative counts, real errno/returning-invalid-parameter imports occur+32h/+3Eh,
and actual memmove is reached+52h after its pointer validation. Both returning
diagnostics and the normal path restore the captured result from ESI+44h/+5Ah,
then RET0Ch. These SDK checks belong to the concrete provider, not an invented
Source filter. Object SHA/offsets/relocations are recorded in the report and
`local/goal_oct15_dword_move_nonzero_object.txt`.
No new tests were added; native differential of this body, original binary ABI
and game behavior remain unproved. Existing math checks do not execute this body.
