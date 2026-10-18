# oct18-tA long-tail reconstruction

Scope: the 82 rows of `codex_tail_A.txt` supplied by cc16-progress on 2026-10-18.
Exact address leases only. No game-host wiring or game launches. New C++ interfaces
under `bsp::oct18_tail_a` are build-tested source reconstructions, not original
register ABI replacements. Object offsets assume MSVC Win32 and valid image-shaped
objects. Descriptive names remain hypotheses.

## Batch 1: game virtual leaves (17 functions)

Evidence is the saved per-address Ghidra decompiler exports; original register
accesses at 006D0D20, 006D0D30 and 006D1F40 were also checked in the assembly.
The existing reviewed BSP names and prior plate comments are retained.

| Address | Ordinary behavior | Original ABI |
|---|---|---|
| 006CA5B0 | return 4 | EAX, RET, no read inputs |
| 006CEBE0 | no-op | RET 4; ECX/stack word ignored |
| 006CEC10 | no-op | RET 0Ch; ECX/three words ignored |
| 006CF3F0 | return 1 | AL, RET; upper EAX unspecified |
| 006CFE30 | no-op | RET 4; ECX/word ignored |
| 006D0D20 | return interior pointer self+15Ch | ECX -> EAX, RET |
| 006D0D30 | load pointer at self+8C8h | ECX -> EAX, RET |
| 006D1640 | no-op | RET, inputs not read |
| 006D1D20 | return category 16 | EAX, RET |
| 006D1D40 | return 1 | AL, RET; upper EAX unspecified |
| 006D1D60 | ignore sync message | RET 4; ECX/word ignored |
| 006D1D70 | return 20 | EAX, RET |
| 006D1DE0 | return 6 | EAX, RET |
| 006D1EE0 | return 0 | EAX, RET |
| 006D1F40 | load byte 00E17BF2 | AL, RET; explicit borrowed global in C++ |
| 006D1F90 | no-op | RET 4; ECX/word ignored |
| 006F7F30 | return `undefined command name` | EAX -> literal 00CFB38C, RET |

No-op parameter meanings remain unknown beyond existing vtable evidence. The
command string has matching contents but a new source address. There is no
binary ABI, original allocation-address, runtime or gameplay claim.

Validation: `scripts/build.ps1` passed MSVC Win32 Release and both existing
CTest checks (reconstructed_math, tool_tests). Per-address naming evidence was
appended through the ledger CLI, applied with the serialized Ghidra annotator
(which verifies project/program, records old annotations and saves), and all 17
affected exports were refreshed. Ignored build/annotation receipts are under
`local/oct18_tA_batch1_build.log` and `local/tail_a_batch1_annotations.log`.

## Held rows

Skipped while cc15-rB owns 00735F30-00758EB0: 00737A50, 00737F30, 007420B0,
00742180, 007422A0, 00748A40, 0074CCA0, 0074E180, 00758140, 00758390.

