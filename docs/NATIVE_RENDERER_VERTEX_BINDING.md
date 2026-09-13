# Actual renderer vertex binding and binding reset

Addresses: `00B24840`, `00B23710`, `00B24BF0`, `00B48CF0`.

These are complete reconstructions over actual Win32 storage and observed native
profiles. Descriptive names are hypotheses. The C++ interfaces require explicit
service contexts and are not original binary ABI replacements. Whole renderer
recreation `B29670` remains outside this packet.

| Routine | Native ABI | Coverage |
|---|---|---|
| `B24840..B24A37`, 504 bytes | ECX renderer; stack stream index, logical owner; both exits `RET 8`; no stable EAX contract | complete |
| `B23710..B2374A`, 59 bytes | ECX destination pointer cell; EDX source pointer cell; EAX same destination; `RET` | complete |
| `B24BF0..B24DB4`, 453 bytes | ECX renderer; no stack arguments; `RET` | complete |
| `B48CF0..B48CF9`, 10 bytes | ECX logical; current physical becomes ECX; tail `JMP EDX`; EAX borrowed COM buffer | complete actual-profile adapter; existing callable-table naked entry retained |

`B24840` enters the optional guard before reading the current header at
`renderer + DWORD(index*16 + 1774h)`. The guard cleanup is armed after the first
owner read and identity comparison. Identity returns without a device call. If
both owners are nonnull, it calls the captured old owner's current buffer getter,
then the incoming owner's current buffer getter. It reads cached stride before
the incoming declaration getter and declaration `+CC`; it reads cached offset
before the incoming offset getter. All getters run before the comparisons.

Matching buffer, cached stride and cached offset invokes `B23710` and returns
without COM work. This still changes the logical identity and its references.
The helper captures source before old, publishes incoming before incrementing
its actual `+04`, then decrements captured old. Canonical owner lookup occurs
only at zero and borrows that same atomic. The existing full logical companion
dispatches current `D61D6C / BD30E0 / B4BF10` through destruction and pool return.
Its base `+4C` retained identity is released through the same canonical domain;
the runtime producer/type of that field is still unproved.

The device path repeats that assignment sequence with a fresh old capture,
then recomputes the original incoming owner's buffer, declaration stride and
offset. Null input produces three zero values. Offset is written to header `+8`
before stride at `+4`. The current device at renderer `+1A10` and its current
table are loaded afterward. `SetStreamSource(+190)` receives index, buffer,
offset and stride. HRESULT is ignored; current counter `+1BB4` increments modulo
2^32 only after return. No bounds repair, rollback or alternative-profile
fallback is introduced.

The `B48CF0` adapter reads current logical `+58`, current physical profile, and
that profile's `+1C` slot. Both private `D61E34` and pooled `D61E7C` select the
existing complete `B4B9F0` leaf, which reads the actual COM pointer at `+28`.
Logical `D61D6C` has `+24=B48CE0` (declaration `+68`), `+28=B48D10` (offset `+5C`),
and `+2C=B48CF0`. Original tokens select established source implementations;
they are never called as host addresses. Logical constructor `B4BC00`, base
constructor `B61E20`, and the existing physical/declaration owner APIs establish
these layouts; this packet adds no conflicting record schema.

The full reset preserves this order:

1. Twenty current renderer virtual `+130` calls with `(sampler, null)`, sampler 0..19.
2. Four current virtual `+134` calls with **`(0, null)` each time**. EDI is the
   countdown only: both arguments are literal zero. Other streams survive.
3. Current virtual `+138` with `(null, 0)`. EDI has reached zero; two pushes and
   the established setter's `RET 8` establish both arguments.
4. Optional guard entry; read pixel cache `+176C`, arm state 0, clear cache
   unconditionally, and call current device `SetPixelShader(+1AC, null)` only
   when the captured cache was nonzero. Increment `+1BBC` only after return.
5. Read current mode, disarm state 0, and conditionally leave. **Only inside
   that exit branch**, recheck mode and optionally enter the second guard.
   Read vertex cache `+1770`, arm state 1, clear it unconditionally, optionally
   call current `SetVertexShader(+170, null)`, then increment `+1BC0` on return.
6. Disarm/conditionally leave; current renderer virtual `+E0(null)` releases the
   cached hardware layout. Its established null path does not unbind the COM
   declaration. Four direct `B23D80(color, null)` calls follow. Color zero reads
   the actual default wrapper at `+197C`; other colors bind null.
7. Optional guard entry; capture current device/table and depth `+9C`, arm
   state 2, then call `SetDepthStencilSurface(null)`. No depth counter is changed.
   Read current mode, disarm, and conditionally leave.

The original `D5F0A8` table maps `+130=B24710`, `+134=B24840`, `+138=B24B00` and
`+E0=B23F20`. Each virtual dispatch rereads renderer profile and slot. The reset
supports this observed profile; the setter's xrefs expose only its `D5F1DC`
table cell, not a comprehensive recovered graph of indirect callers. Reset's
two direct callers (`B296E0` in `B29670`, `B3297E` in `B32920`) pass their captured
renderer in ECX; the latter explicitly publishes `D5F0A8` at `B32942` first.

The full listings establish register provenance: vertex EBP starts as renderer,
temporarily holds the incoming buffer during comparisons, then is restored on
the changed path; EDI remains the scaled header base. Reset ESI is renderer
until the final device capture; EDI's initial/countdown/color-loop writes and
EBP's `FFFFFFFF` cleanup-state value were read across the whole owned listing.

| Native call sites | Callee/body contract |
|---|---|
| `B2486E`; `B24C64`, `B24CD6`, `B24D65` | complete `B33AD0` actual optional entry |
| `B248BF`, `B24A1E`; `B24CC0`, `B24D2F`, `B24DA1` | complete `B33B00`, current-mode leave, stack `RET 4` |
| `B248EE`, `B248F9`, `B249B6` | current logical virtual `+2C`, `B48CF0` |
| `B2490E`, `B249C3` | current logical virtual `+24`, full `B48CE0` |
| `B2492B`, `B249D2` | current logical virtual `+28`, full `B48D10` |
| `B24949` | complete `B23710`; sole recorded direct xref |
| `B23744`, `B249A3` | current zero terminal; canonical same-atomic provider |
| `B249FB` | current COM `SetStreamSource`, stdcall five arguments including device |
| `B24C1D`, `B24C3E`, `B24C51`, `B24D40` | current renderer profile slots above; actual complete texture/vertex/index/layout providers |
| `B24CA0`, `B24D12`, `B24D87` | current COM pixel/vertex/depth setters; stdcall two arguments including device |
| `B24D49` | complete `B23D80`, stack color/null, `RET 8` |

The vertex FH3 handler is raw `CBD018..CBD021` (10 bytes, final JMP length 5),
with `DF5694` FuncInfo and `DF568C` unwind map state 0 to -1 via `CBD010`.
Reset's raw handler `CBD088..CBD091` (10 bytes, final JMP length 5) uses
`DF5728` and `DF5710`: states 0/1/2 all go directly to -1 via `CBD070/78/80`.
The four complete 8-byte unwind thunks load the same guard address `[EBP-14h]`
and tail-jump to `B21110` (final instruction length 5). Both handler starts have
`no_ghidra_function`; the four unwind thunks have complete saved Ghidra bodies.
No flow repairs or Ghidra writes were made by this worker.

Normal leave is disarmed before calling its provider. Exceptional cleanup uses
the complete actual `B21110`, with termination on a second C++ exception. Reset
reuses one eight-byte native guard record. Skipped entries preserve its old or
indeterminate storage; enabling cleanup of a never-initialized record remains
outside the original valid input domain. Existing canonical terminal providers
require nonthrowing destruction. These domain restrictions are not silent
success substitutes for unimplemented lifetime or recreation work.

Validation and reproducible fixture paths are recorded in
`reports/native_renderer_vertex_binding.json`. The installed executable is
read-only. Private mapped native routines, original FH3 metadata, current library
providers and a real D3D9 HAL device are used by the ignored fixture. Its callback
faults are controlled injections at the current COM slot, not game-generated
exceptions. Native reset delegates already reconstructed texture/index/layout/
color callees to the current source library; their independent reconstruction
evidence remains in their existing reports. No full game/frame or recreation
validation is claimed.
