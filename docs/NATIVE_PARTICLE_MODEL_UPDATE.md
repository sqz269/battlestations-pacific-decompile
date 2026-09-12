# Native particle model update

Addresses: `00AF6DD0`, `00B6E0A0`, `00B6DB60`, `00AF3F50`, `00AFD9F0`,
`00AFFA70`, `00AFFAE0`.

`native_particle_model_update.hpp/.cpp` reconstruct the complete instruction
paths of the model update and six leaves for MSVC Win32. The main routine uses
the actual `2DCh` model payload, whose prefix is the canonical `NativeNodeStorage`.
It requires the application's existing options, random, particle services and
CRT allocation/math bindings. These interfaces do not construct substitute
particle owners. Descriptive names are hypotheses, not recovered symbols.

## Coverage and ABI

| Original range, inclusive | C++ symbol suffix | Original ABI | Coverage |
| --- | --- | --- | --- |
| `00AF6DD0..00AF7391` | `update_native_particle_model_00af6dd0` | ECX model; stack float delta, low mode byte; RET8 | complete, 375 instructions; required services below |
| `00B6E0A0..00B6E0C0` | `copy_native_node_local_position_00b6e0a0` | ECX node; stack destination; EAX destination; RET4 | complete |
| `00B6DB60..00B6DB66` | `get_native_node_local_matrix_00b6db60` | ECX node; EAX node+B0h; RET | complete |
| `00AF3F50..00AF3F7C` | `copy_native_particle_definition_bounds_00af3f50` | ECX definition; stack destination; EAX destination; RET4 | complete |
| `00AFD9F0` | `destroy_native_particle_temporary_00afd9f0` | ECX temporary; RET | complete, literal RET |
| `00AFFA70..00AFFAD2` | `evaluate_native_particle_linear_curve_00affa70` | ECX curve; stack float time; ST0 float; RET4 | complete |
| `00AFFAE0..00AFFB56` | `evaluate_native_particle_cubic_curve_00affae0` | ECX curve; stack float time; ST0 float; RET4 | complete |

The main C++ entry adds a borrowed access pointer in EDX. It saves this pointer
in four extra stack bytes, shifts only incoming delta/mode references, and
retains the original local offsets and call cleanup. Call-site access loads use
MOV instructions, preserving live flags and x87 values. The definition bridge
also receives the vtable target captured by the original instruction sequence.
This is a new integration interface, not a drop-in binary or C++ exception ABI.

The canonical node declaration already establishes local matrix `+B0h`, local
translation `+E0h`, world matrix `+F0h`, world translation `+120h`, inverse cache
`+60h` and flags `+5Ch`; no second node layout is introduced. Derived model and
definition fields remain native address-relative accesses rather than invented
semantic structs. The required extent exceeds `sizeof(NativeNodeStorage)`.

## Behavior and evidence

When byte `model+1B0h` is set, world/inverse matrices are copied to `+218h/+258h`
before the actual options getter and its current byte `+04h` suppression check.
Initialization sets `model+1A5h` before reading the signed definition count at
`[model+18Ch]+30h`. Every iteration reloads the current top-level definition,
captures the selected entry's virtual `+08h`, and uses its actual temporary.
Random stream zero, the entry's curve, local/world position copies, matrix
copies, owner append, temporary destructor and CRT free retain native order.

The live exported function omits `00AF704F..00AF7071` after the free call because
of an earlier incorrect no-return annotation. The installed PE and live bytes
agree for the entire `1474`-byte routine, including this `35`-byte continuation:

```text
00AF704F MOV EAX,[ESP+1Ch]       00AF7053 MOV ECX,[ESI+18Ch]
00AF7059 ADD EAX,1              00AF705C ADD ESP,4
00AF705F ADD EBX,4              00AF7062 CMP EAX,[ECX+30h]
00AF7065 MOV [ESP+18h],EAX       00AF7069 JL 00AF6E50
00AF706F XOR EBX,EBX            00AF7071 POP EBP
```

`00BF65AC` tail-jumps to the returning CRT free body `00BF9DC8`. Its normal
heap-release and null-input paths return. Initialization therefore continues
into this invocation's regular update; the source neither returns early nor
retains the initializer's nonzero EBX in the later counters. Ghidra was read
only for this packet; function-body repair, annotation and export refresh are
integration work.

Positive delta updates velocity `+200h..+208h` and previous-position words
`+20Ch..+214h` using native local/world selection. The update accumulates time
`+188h`, divides by step `+1A0h`, spills the quotient to float32, widens it to
double and calls the real CRT floor. It retains native fractional remainder,
emitter array/count reloads, fixed-step calls, mode-byte comparison, time
`+184h` accumulation, bounds copy and `+1D4h` stores. x87 lifetimes, float32
rounding points, MOVSS copies, comparisons and unchecked/NaN behavior follow
the instruction listing; no clamping or global clock adjustment is added.

Linear/cubic curves walk rows of `14h/1Ch` bytes through `curve+04h`, comparing
against each row's end time. Their equal-time fast path, unbounded walk, float32
spills and native polynomial order are retained. Bounds copy preserves three
forward x87 component stores and the native fourth result `definition+8Ch +
definition+88h`. Temporary destruction remains the original single RET.

## Required services and reused kernels

All direct/indirect call sites are enumerated with their containing function in
`reports/native_particle_model_update.json`.

| Call site(s) | Original target | Required contract or reused implementation |
| --- | --- | --- |
| `AF6E0F` | `0051F6B0` | Existing `RegisteredModelEffectCallees` actual singleton getter; byte04 read after the call, no cached suppression value |
| `AF6E6C` | selected definition vtable+08 | ECX actual entry; stack actual model, time184; RET8; EAX actual temporary supporting +00..+9F. Dynamic implementation is unread; address/slot contract only |
| `AF6E9C` | `00BD2FC0` | Existing application `RandomThreads::next_00bd2fc0`, stream zero; native signed FILD of returned bits is retained |
| `AF703D` | `00AFD410` | Actual model190 owner; RET4. Signed current count14/capacity08 gate; byte index04, 108h stride, base0C; AFCF50 then current count14 increment |
| `AF704A` | `00BF65AC` | Real allocation domain paired with virtual08; cdecl free returns and caller ADD ESP,4 runs |
| `AF7233` | `00BF85B0` | Real cdecl floor(double), ST0. Preserve current0109EEA0, MXCSR/x87 dispatch, exceptional-input handlers and control-word effects; caller ADD ESP,8 |
| `AF728E` | `00AFF640` | Actual emitter, delta, RET4; B05110 simulation; current empty-container virtual04(1), clear emitter10; EAX active count |
| `AF7302` | `00AFD7A0` | Actual model190 owner, time, low mode-equal byte, RET8; AFE290 over 108h indexed records, swap index bytes and reload current count14 |
| `AF6DFE` | `00B6E0D0` | Reuse canonical `get_native_camera_view_00b6fcb0`; entire world/inverse/copy/flags kernel matches instruction for instruction after address relocation |
| `AF737D` | `00B74390` | Reuse `set_native_generated_model_bounds_00b74390`; cdecl adapter to the native four-word x87 stores/flags update |
| See report | `00B6DB70`, `004134F0` | Existing native world refresh and exact forward x87 matrix copy |

The services must preserve mutations to the actual payloads and valid native
call boundaries. Allocation failure, object construction, destruction ownership,
nonempty simulation and exception dispatch remain the application's contracts.

## Verification

On 2026-09-12, `bsp.py` verified the `bsp` project and
`/battlestationspacific.exe` before the read batch. Seven complete live byte
ranges match the installed executable; per-routine SHA-256, instruction counts,
last instructions and original call-site bytes are recorded in the report.

The new source compiles and links with `/std:c++17 /permissive- /EHsc /W4 /WX
/fp:strict /O2` under MSVC Win32 against the integrator's existing native kernels.
The local executable has `/MANIFEST:EMBED`. One focused original-byte fixture
passed **696 pairs**, including **480 complete AF6DD0 invocations**, under all
three x87 precision modes and four rounding modes. Comparisons include complete
model/definition/owner/emitter backing bytes, x87 sticky status/TOP/control word,
and MXCSR. Curves cover zero, signed zero, segment boundaries, positive infinity,
quiet NaN and signaling NaN. A live x87 sentinel detects stack imbalance.

The fixture runs original singleton, SSE2 floor, AFF640 and AFD7A0 instructions;
uncopied code is INT3. It binds an existing options object, empty top-level
definition lists and empty native particle containers. Actual model updates
cover initialized/uninitialized, local/world, suppressed/enabled and zero,
positive, negative and multiple-step delta cases. Construction services abort if
reached. No successful fake constructor, particle simulation or no-op service is
used. The initializer loop with entries, random branch, allocation/free,
nonempty simulation, CRT fallback/error paths and game behavior are unvalidated.

Local reproduction artifacts are ignored under `local/`: `make_model_update_probe.py`,
`model_update_probe.cpp`, `run_model_update_probe.cmd`, the image/relocation data,
and build logs. Their hashes are recorded in the report; they are not a new
committed test framework. The packet's source awaits integrator CMake registration.
The standard parallel build encountered MSB3491 access failures creating Lua/zlib
`.tlog` temporary files; serial build and existing CTest results are recorded
separately in the report. Neither fixture nor compilation establishes original
object/EH ABI compatibility or game validation.

## AK saved-analysis and combined-build integration

The seven-module AK batch is registered in bsp_core. Strict MSVC Win32
compilation and both seeded CTests passed with explicit `--parallel 1`; the
standard parallel script hit environment MSB3491 before compiling C++.
The report records saved Ghidra name/signature preimages, prior-comment
preservation and readback, original-byte fixture coverage and exact call checks.
Reported returning-free continuations and missing definitions are now repaired
and saved; worker-era pending-integration notes above describe the earlier snapshot.
New C++ interfaces and required real runtime bindings remain as documented.
Successful full construction, native EH compatibility and gameplay are not implied.

## AK final merged validation

After merging current main at `aab1373abde21d9a8d03ad113f8a53317cfd8ff5`, the repository standard
`./scripts/build.ps1` completed successfully and both existing seeded CTests
passed. The focused original-byte fixture was rebuilt with `/fp:strict` and
replayed against that combined library; it passed. The report pins its log and
library hash. Earlier parallel MSBuild failures and worker-pending notes above
are historical; the final build required no global configuration change.
All stated constructor, simulation, current-slot, native EH and gameplay limits
remain in force.

The parallel MSBuild invocation remains intermittent: a later documentation-only
rerun again hit MSB3491 before C++ compilation. The final serial full build and
both existing CTests passed again on unchanged source. This environment issue
was recorded rather than changing global permissions or build configuration.
