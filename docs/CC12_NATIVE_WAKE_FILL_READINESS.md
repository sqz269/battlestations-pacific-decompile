# Native wake fill readiness, 00810020

The complete `00810020..00810159` leaf is ready for a separate implementation
over actual native storage. It is 314 bytes / 85 instructions, has no calls,
and depends only on four verified immutable constants. Its pure memory
producer contract does not require reconstructing a game entity constructor,
world owner, settings owner, or the append path. No Source implementation,
build, execution, Ghidra annotation, or shared ledger change was made here.

## Actual storage and entry contract

The original entry takes ECX = actual wake storage, stack arguments = a
three-float position pointer and a float heading, and returns with `RET 8`.
It saves ESI; EBX, EDI and EBP are untouched. The largest x87 depth increase is
four entries. It preserves the caller's FP settings and follows the native
x87 status/exception behavior without resetting the environment.

| Address relative to wake | Fill behavior |
| --- | --- |
| `+0x000..+0x007` | Unread and untouched prefix |
| `+0x008 + slot*0x18` | Writes XYZ, heading at sample `+0x0C`, segment length at `+0x10`, for all 40 slots |
| Sample `+0x14` | Every yaw word remains exactly as supplied in the preimage |
| `+0x3C8` | Writes signed head `39` once |
| `+0x3CC` | Writes byte zero once |
| `+0x3CD..+0x3CF` | Untouched padding |
| `+0x3D0..+0x3DB` | Residual words are neither read nor written |

The caller supplies writable addressed storage and a readable position triple,
and owns initialization of preserved bytes, lifetime, and synchronization.
The fill overwrites the old head/flag and all XYZ/heading/segment words; their
old values are not needed. It does **not** initialize yaw or residual. Those
preserved words must not silently become zeros, host counters, or a synthetic
validity/written model.

If a caller already owns an entity with this wake at `entity+0xBD0`, the filled
XYZ/segment records and head are exactly at `entity+0xBD8` and `entity+0xF98`,
the addresses consumed by the complete native decomposition leaf. That
storage relationship closes this bounded producer/consumer contract; it does
not prove the entity's production or application wiring. Three current xrefs
are retained as caller identities only, without expanding those callers.

## Complete numerical and write schedule

1. Load the heading through x87, clear the flag byte, compute the extended
   subtraction of the stored double half-pi and heading, write head 39, and
   spill the angle to binary32. Compare zero against that spilled angle with
   FCOMIP. Add the stored double two-pi exactly once only for an ordered
   negative angle, then spill again. This is not a general modulo operation;
   unordered values follow the original JBE path.
2. Execute native FCOS and FSIN on separate loads of the spilled angle. Spill
   both results to binary32, including the extra cosine load/store. Preserve
   the exact x87 multiplication and spill schedule that forms
   `step = (float(50*cosine), float(+0*50), float(50*sine))`, interleaved
   with the position captures described below.
3. Crucial operand recovery: at `0081008C`, bytes `DC C9` mean
   **`FMUL ST(1), ST(0)`**. Ghidra renders an ambiguous `FMUL ST1`.
   After this instruction and FXCH, the stored X step is `50*cosine`, and
   `50` remains on the x87 stack for the other components. Treating this as
   `FMUL ST(0), ST(1)` would produce a different routine. The full disk decode
   and saved Ghidra bytes agree.
4. Read the position pointer (`00810078`), then position X (`00810082`),
   head into EDX (`00810086`), and the raw heading argument (`0081008E`).
   Store the X step (`00810099`) and captured position X (`0081009D`).
   Load position Y (`008100A5`) and spill it (`008100AC`); load and spill Z
   (`008100B2/008100B7`) before the final Y/Z step spills
   (`008100BF/008100C7`). All position captures use MOVSS after the head/flag
   writes. A position pointer overlapping metadata therefore sees those
   writes. Do not move all input reads to entry.
5. Visit slots `39,38,...,0`. Within each iteration, write heading, select and
   write the segment word (`+0` for slot 39, binary32 `50` for the other 39),
   and write the running XYZ. Each next-coordinate subtraction uses x87 and
   has its own binary32 spill, interleaved with those stores. The final
   iteration still performs its three next-coordinate subtractions. Head
   remains 39; only the register index rotates.
6. Pop the three retained step values and return. No yaw, residual, prefix,
   padding, append counters or written count is changed. Arithmetic flags,
   rounding, exceptional/large-angle FCOS/FSIN behavior, signed zero, and
   partial writes follow the original sequence, not generic C++ math.

The four Original payloads are:

| Address | Width | Little-endian bytes | Value |
| --- | --- | --- | --- |
| `00CE3830` | 8 | `00000060fb21f93f` | Exact promotion of binary32 `1.5707963705062866` |
| `00CE3828` | 8 | `00000060fb211940` | Exact promotion of binary32 `6.2831854820251465` |
| `00CE3938` | 8 | `0000000000004940` | Binary64 `50` |
| `00D09290` | 4 | `00004842` | Binary32 `50` |

## Current typed implementation and provider evidence

The current `ShipAiWakeSample` is exactly six float words / `0x18` bytes with
the expected field order. `ShipAiWakeTrail`, however, begins its samples at
offset zero rather than native `+8`. Its compiled head/flag stores are at
`+0x3C0/+0x3C4`, and it additionally writes host `written=40` at `+0x3E0`.
Its fill leaves existing yaw/residual words alone; default member
initialization elsewhere is a separate host-object behavior.

The whole existing typed fill is 286 bytes / 70 instructions in both current
worker and primary objects. Each exact archive member was retained, together
with the whole archive, current source, project settings, and executable.
Each existing executable contains one complete matching typed body after
seven resolved relocations. Its genuine `cos` and `sin` calls resolve through
actual UCRT import thunks; the three float constant values match their
intended values. This is real current provider evidence, but those CRT calls
and SSE arithmetic are not native FCOS/FSIN or the native storage interface.
Compiled store order also differs: segment is stored after XYZ. Reusing this
typed fill through a view or callback would not satisfy the raw contract.

Pre/post live Ghidra bytes for the complete leaf and all four constants agree
with the current installed PE. Every live CLI query verifies the configured
`bsp` project and `/battlestationspacific.exe` program. The 85 instruction
starts match the complete disk decode. Source inputs and retained physical
providers are hashed in
[the packet report](../reports/cc12_native_wake_fill_readiness.json).

## Smallest next Source packet

Add an ordinary C++ interface such as
`void fill_native_unit_wake_00810020(void* actual_wake, const float* position, float heading)`
in new `include/bsp/native_unit_wake_fill.hpp` and
`src/native_unit_wake_fill.cpp`, with a private Win32 assembly kernel preserving
the complete sequence and four genuine immutable payloads. It needs no
external helper, CRT shim, callback, global owner, or constructor. Suggested
companion files are `docs/CC12_NATIVE_WAKE_FILL.md` and
`reports/cc12_native_wake_fill.json`, plus one CMake registration when Source
work is authorized.

Before any fresh comparison, verify the complete Original/COFF/unique linked
body and all four resolved data operands, retaining consumed inputs. A small
focused comparison should inspect all 40 records and every preserved byte
from a seeded preimage, plus the metadata-overlapping position case that makes
input-read ordering observable. No old fixture replay, current raw Source
implementation, ABI execution, startup or gameplay result is claimed by this
read-only readiness packet. Root acceptance is required before implementation.

Primary review accepted the retained evidence after independent complete-body and physical-artifact checks. Receipt: local/cc12_ready_packet_primary_audits/wake_fill.json. No new Source, build or runtime execution occurred during that review.
