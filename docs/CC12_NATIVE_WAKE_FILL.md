# Native wake fill, 00810020

`fill_native_unit_wake_00810020(actual_wake, position, heading)` now provides
an ordinary C++ interface to the complete native fill leaf. A private Win32
assembly kernel preserves all 314 bytes / 85 instructions after four actual
constant-address relocations. It has no calls, mutable globals, typed-model
adapter, or synthetic written counter. The previous typed implementation is
unchanged.

## Addressed storage and execution contract

The caller supplies writable actual wake storage containing 40 records at
`+8`, stride `0x18`, a head word at `+0x3C8`, and a flag byte at `+0x3CC`.
The routine writes head 39, flag zero, and each record's XYZ, raw heading, and
segment word in slots `39..0`. Segment is positive zero at slot 39 and
binary32 50 in the remaining slots. Every yaw word, residual word, prefix,
padding byte, and other byte outside this write set is preserved.

`position` addresses three readable float words and may overlap writable wake
storage. The flag/head stores precede the position captures. The caller owns
the preserved preimage, initialization required by its later consumers,
lifetime, and synchronization. This leaf neither creates nor admits a game
entity or owner. If the caller already supplies the wake at `entity+0xBD0`,
the written records/head match the native decomposition leaf's
`entity+0xBD8/+0xF98` storage contract.

The private kernel retains the original ECX/stack/RET8 entry sequence; the
public function is a separate ordinary C++ interface, not a drop-in original
ABI claim. Four x87 slots must be available. The native instruction sequence
preserves FP settings and retains status/exception effects, FCOS/FSIN,
binary32 spills, raw heading writes, interleaved reads/stores, final unused
coordinate subtractions, and stack cleanup. In particular, `0081008C` remains
`DC C9`, **FMUL ST(1), ST(0)**. The angle correction adds two-pi once on the
native ordered-negative branch; it is not replaced with modulo or CRT trig.

All four immutable payloads are the actual Original bytes:

| Original address | Little-endian bytes | Meaning |
| --- | --- | --- |
| `00CE3830` | `00000060fb21f93f` | Binary64 promotion of the original binary32 half-pi |
| `00CE3828` | `00000060fb211940` | Binary64 promotion of the original binary32 two-pi |
| `00CE3938` | `0000000000004940` | Binary64 50 |
| `00D09290` | `00004842` | Binary32 50 |

The complete read/write and x87 schedule is documented in
[the accepted readiness audit](CC12_NATIVE_WAKE_FILL_READINESS.md).

## Verification before the first call

Fresh live Ghidra bytes for the complete leaf and all four constants matched
the current installed PE. The whole production COFF kernel matched Original
after exactly four DIR32 operands. Each referenced constant's actual COFF
section payload and read-only flags were checked. The complete fresh archive
and its exact physical member were retained.

The focused executable links that production archive. Its ordinary wrapper
is 29 bytes / 11 instructions, and its private kernel is 314 bytes / 85
instructions. Both whole linked bodies matched the relocated COFF bytes,
each occurred once physically, and all map aliases and four resolved
read-only data targets were recorded.

The complete 4,096-byte Original image was generated and physically frozen at
fixed address `0x30000000` **before execution**, with only the four data
operands relocated. It contains the full Original code and genuine constant
payloads, with no helper or CRT calls. The probe requires that exact mapping,
checks its copied bytes, changes it to execute/read protection, and retains
the actual mapping before any Original call.

Before that first run, 209 physical input pins were sealed and rehashed:
Source/header and full preprocessed translation units; probe source, runner,
verifier and recipes; Original ranges, complete map and seeded preimages;
whole object/archive/member/executable/map; actual selected MSVC compiler
passes and link-searched libraries; loaded Python verification inputs; and
current Win32 OS/CRT files used as probe-backend evidence. The leaf's
arithmetic backend is the CPU's x87 instructions. OS/kernel/CPU behavior is
not reconstructed, and retained OS files are not a runtime DLL-binding proof.

## Fresh bounded comparisons

The first and only native run passed one family of three fresh
Original/Source pairs:

| Case | Concrete coverage |
| --- | --- |
| Nonzero heading with negative angle correction | All 40 XYZ/heading/segment records and native wrapped-angle sequence |
| Position overlaps head/flag metadata | Head position reads `00000027, 3f800000, 40e00000` after metadata writes; 53-bit x87 precision |
| Position overlaps the head record, heading is negative zero | Captured position survives its later stores; every raw heading word stays `80000000` |

Each pair compares all 1,056 bytes of its seeded actual memory and independently
checks all 40 record metadata words, the head position after metadata writes,
and all 251 bytes outside the native write set. This covers every yaw and
residual word plus prefix, padding, and guards. Saved before/after images and
register/FP snapshots were independently rechecked. ESP, EBX/ESI/EDI/EBP,
x87 control word/empty tags, and MXCSR were preserved. Final x87 status matched
between arms, including `0x0022` in the metadata-overlap case.

Executed cases used masked exceptions, round-to-nearest, empty x87 entry,
64-bit or 53-bit x87 precision, and MXCSR `0x1F80`. Full instruction equality
supports the native sequence in other admitted environments; those were not
separately executed. Post-run live Original bytes/constants and every sealed
input still matched. There were no failing native pairs or historical fixture
replays.

`scripts/build.ps1` completed the MSVC Win32 Release build and all three
existing tests (`reconstructed_math`, `native_math_differential`, `tool_tests`).
The existing unrelated duplicate `spawn_request_id_matches` warning remains.
No permanent test target was added.

This establishes Source, complete provider-byte, build/test, and bounded raw
memory execution evidence. It does not establish game entity construction,
an Original class/owner, append behavior, parent integration, startup, or
gameplay. [The packet report](../reports/cc12_native_wake_fill.json) records the
full receipts. Reproducible inputs and snapshots are retained under
`local/cc12_native_wake_fill_Source_evidence` and its sibling ZIP.

Primary review independently rehashed every saved artifact and ZIP entry, all 209 physical pre-execution inputs, the full Original map, all saved pair memory/register/FP snapshots, current full COFF and the exact unique current archive member. The combined current Win32 build and all three existing checks passed. Git converted Source/header line endings from CRLF to LF; complete text and whole emitted bodies still match. Receipts are under `local/cc12_wake_fill_publication_primary_review`. No new primary execution or owner/game qualification was added.
