# Native wake append Source: 00810190

The existing wake-dependency translation unit now implements the complete
00810190 operation sequence behind an ordinary borrowed-storage interface.
The **1,183-byte / 303-instruction Original** maps to a private naked
**1,175-byte / 306-instruction Source kernel**. Every Original instruction is
accounted for: three added pointer MOVs, six live-reset operand replacements,
six constant relocations, six call relocations, and mapped branch encodings.
No other original instruction bytes change. This is an explicitly adapted
Source implementation, not an unchanged Original body or binary ABI entry.

## Borrowed interface and true storage

```cpp
void bsp::append_native_unit_wake_00810190(
    void* actual_wake, const float* actual_world_position,
    float heading, float yaw_rate,
    game::GameNativeGeometryGlobals& actual_geometry_globals);
```

The caller supplies a genuine valid 988-byte raw wake: forty 24-byte records
at +8, head at +0x3C8 (0..39 at every read), the full byte flag at +0x3CC and
three mutable residual cells at +0x3D0. Position supplies three readable
float cells. The geometry reference must be the actual process instance
returned by `game_native_geometry_globals()`. Caller-owned lifetimes and
synchronization cover every access. Valid external regions may alias, but
may not overlap active adapter/kernel frames or formal argument slots.
Scalar by-value copies are an explicit public-interface adaptation.

The actual canonical geometry getter remains six bytes and relocates to one
local, writable, initially zero 12-byte object. The parent obtains its array
address through `zero_vector_00f87574().data()`; it defines no reset storage
and never snapshots reset values. The current semantic `ShipAiWakeTrail`
starts its samples at zero and its head at +0x3C0, despite its native-offset
comments. It cannot be reinterpreted as this raw input. No typed trail,
fill function, existing caller, game host or entity producer was changed.

The complete compiled public wrapper is **25 bytes / ten instructions**.
Integer PUSH instructions copy yaw and heading representations; integer MOVs
acquire the actual reset address and wake pointer. Its sole call reaches the
real local kernel. There is no reset-cell read, FP instruction, environment
operation or unexpected helper. The emitted reset accessor and array `data`
accessor both independently decode as `MOV EAX,ECX; RET`.

## Full instruction and stack proof

The private kernel retains native ECX=wake, three native stack words
(position, heading bits, yaw bits), all original local/argument offsets and
`RET 12`. Its Source-only EDX input is the genuine reset-cell address.

| Added operation | Original location and meaning |
| --- | --- |
| `MOV [ESP+0x28],EDX` | Immediately after original `SUB ESP,0x3C`; saves only the address at entry ESP−20. |
| `MOV EDX,[ESP+0x34]` | Before original 00810235, after the distance gate. |
| `MOV EDX,[ESP+0x34]` | Before original 008102A3, after the real math calls and decay branch. |

Only FLD operands at 00810235/247/25A and MOVSS operands at
008102A3/2B3/2C0 change to `[EDX]`, `[EDX+4]`, `[EDX+8]`. Every observation
and interleaved residual write remains in its original order. No reset value
is read on the early-distance-return path. The first original EDX operation,
00810351, defines it afresh; subsequent original EDX use remains intact.

Independent CFG analysis reaches all 303 Original and 306 Source instructions
with identical mapped ESP/x87 states and consistent joins. Direct stack
operands and the outgoing 3D vector at entry ESP−36..−25 and 2D vector at
entry ESP−60..−53 exclude the saved word. All six calls and their cleanup
contracts agree. The append maximum is four local x87 slots; independently
checked raw 3D/2D/copy maxima are four/three/one. CRT private interiors are
outside that accounting. No FP reset or normalization is introduced.

The complete instruction map preserves repeated position/head/global reads,
all ordered and unordered branches, the +0 Y leg, old-segment truncation,
ordered `55 > span` merge, and the ordered `segment > 50 && flag == 0`
advance. The full-byte flag is not narrowed to equality with one.

## Actual bindings and preserved providers

| Original call | Actual Source target |
| --- | --- |
| 00810273, 00810416 | `native_unit_wake_length_0042b2f0`: complete 77-byte / 31-instruction raw body. |
| 00810280 | Genuine SDK `_CIsqrt`, ST0 input/output. |
| 0081043F | Genuine SDK `_CIatan2`, ST0=delta-X and ST1=delta-Z. |
| 008104E2 | `raw_length_2d_00414c60`: actual five-byte tail JMP to its existing local 66-byte / 26-instruction kernel. |
| 00810566 | `copy_native_unit_wake_sample_00810160`: complete 43-byte / 15-instruction raw body, RET4. |

The last call retains the already-pushed source and the existing EDX source
value; no dummy-parameter initialization is inserted. Both raw lengths
return final float bits in ECX from their overwritten scratch slot, rather
than restoring the incoming pointer. Their ST0 and stack contracts remain.

All twelve existing whole functions across the four selected objects retain
their complete bytes and normalized ordered relocations. This includes both
raw dependencies, the existing 12-byte 2D cdecl bridge, its kernel and facade,
the canonical owner getter and the selected camera provider. Six parent
constant operands resolve to real read-only cells with exact Original
payloads. The helper cutoff is likewise exact. The actual normal-link SDK
`ucrt.lib` contains unique complete 60-byte `__CIsqrt` and 61-byte `__CIatan2`
import members, mapping to `_CIsqrt`/`_CIatan2` in
`api-ms-win-crt-math-l1-1-0.dll`; their whole payloads and library are retained.

The required canonical `<array>` include also emits the SDK `wchar.h`
four-byte `__Avx2WmemEnabledWeakValue` fallback. Its exact external symbol,
zero-initialized `.bss`, `IMAGE_COMDAT_SELECT_ANY` auxiliary record and
`/alternatename` directive match the pre-existing vector/owner/camera
definitions. No parent function references it. This standard-header
selectable definition is accounted for separately from reset storage; the
parent object is not claimed to have no writable section.

## Build, provenance and limits

The fresh worktree's first `./scripts/build.ps1` passed its two configured
checks. Its seed header was initially absent; standard read-only
`verify-seeds` verified the Original seeds and generated that header. A second
normal build enabled the existing differential check. All three pass:
`reconstructed_math` (0.17 s), `native_math_differential` (0.04 s) and
`tool_tests` (8.37 s). No new test or ad-hoc link/probe was added. The existing
LNK4006 duplicate `spawn_request_id_matches` warning remains.

Source, headers, selected recipes, configured tools, SDK libraries and
external main baseline artifacts were physically captured before editing
and before the first build. Actual consumed dependency sets of 77, 75, 73
and 228 files were subsequently matched to those physical pre-build copies;
all remain unchanged. This is a selected input-closure claim, not every
project input or a loaded-tool-module observation. The fresh worktree's
generated recipes/command records had no pre-configure preimage and are
explicitly retained post-only relative to the first build. They were also
captured before the second build. Early snapshots honestly record two absent,
misspelled optional typed-trail paths; actual typed Source is only later
context and was not consumed by the selected translation units.

All six physically retained whole archive snapshots were independently
parsed; each has 1,904 members and exactly one member equal to each selected
whole object. The four selected objects and entire core archive are byte
identical across seed/check enablement. Evidence and hashes are indexed in
[the report](../reports/cc12_native_wake_append_source.json), with full
instruction maps, CFG states, COFF sections, archive members and provenance
under its sealed ignored evidence family.

Admission is limited to complete Source, normal build/checks and actual
object/archive bindings. The new Parent API and Original Parent were never
executed. No application COMDAT, map/PDB/full-body linkage, entity producer,
application consumption, startup or gameplay validation is claimed. Original
CRT interior/dispatch/diagnostic/exception-policy equivalence, hardware-fault
resumption and full FIP/FDP environment identity remain outside this scope.
The ordinary public interface also makes no Original volatile-register or
EFLAGS ABI promise. Root owns shared annotation, registration and integration.
