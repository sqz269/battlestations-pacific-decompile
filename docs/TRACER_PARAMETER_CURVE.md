# Tracer parameter curve

Addresses: `00BA9DA0`, `00BACAA0`. These descriptive names are hypotheses.

The implementation operates on the actual 1Ch-byte owner and its 0Ch-byte
records. It supplies neither record allocation nor a successful Lua loader.
It is unrelated to the application's six-setting rudder curve.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| `00BA9DA0..00BA9E4A` | `__thiscall`, ECX owner, one binary32 stack argument, ST0 result; final `00BA9E48 RET 4` is three bytes | complete instruction path, including cache writes and x87 status/rounding behavior |
| `00BACAA0..00BACB05` | `__thiscall`, ECX fresh storage, EAX same storage; `00BACB05 RET` | complete normal-return initialized state; native SEH registration/unwind ABI is not reproduced |

`BA9DA0` lacked a live Ghidra function definition during this packet. Its
171 disk bytes matched the live Ghidra bytes (SHA-256
`d7ad8f9a4e072bc1e1646ccbb260f17f7fde0691a1103dff976a2e858ae3964a`).
The inclusive end is `BA9E4A`, not the final instruction's start `BA9E48`.
Every live query used `bsp.py ghidra`, which verifies the `bsp` project,
`/battlestationspacific.exe`, x86 language and image base before reading.
The configured project is `C:/Users/sqz269/bsp.gpr`; no Ghidra mutations were made.

## Producer and storage

`00869E40` allocates 1Ch bytes and calls `BACAA0` at `00869E7E` when its output
owner is null. The constructor writes refcount 1 and table `D63FFC`; the
DWORD at table+08 is `BA9DA0`. `00869E40` reads each Lua row's first number,
multiplies it by its third argument and spills binary32 at `00869F08`; it
spills the second number at `00869F2D`, then passes both to `BAC670` at
`00869F52`. That producer returns with `RET 8`, confirming two stack floats.
The loader itself ends in `00869FC9 RET 0C` and is not reconstructed here.

| Owner offset | Field | Writer evidence |
| --- | --- | --- |
| +00 | original-image vtable address | `BACAD1` writes `D63FFC`, after transient base table `CEB130` |
| +04 | reference count | `BACAC9` writes 1 |
| +08 | record pointer | `BACADB` zeros it; grow helper `BAC057` publishes the new allocation |
| +0C | storage size | `BACADD` zeros it; `BAC71B` increments after appending |
| +10 | storage capacity | `BACAE0` zeros it; `BAC059` publishes the grow request |
| +14 | evaluation point count | `BACAF1` zeros it; `BAC71E` increments it |
| +18 | mutable current segment | `BACAF4` zeros it; evaluator `BA9DFB` resets / `BA9E21` increments |

`BAC670` writes the new record coordinate at `BAC70F`, value at `BAC715`, and
zero slope at `BAC718`. For a previous record, it spills the coordinate delta
to binary32 at `BAC6B1`. It compares with binary32 `D7A23C` (bits `3A83126F`,
approximately 0.001), writes zero slope when smaller, otherwise computes the
value delta divided by that spilled coordinate delta and spills slope at
`BAC6D5`. The new record remains the last point with zero slope until another
point is appended. These writes establish the record layout independently
of the evaluator; no sorting or duplicate-point repair is inferred.

`BACAA0` calls `BAC5E0` at `BACAE8` with ECX=owner+08 and stack size 0.
The complete `BAC5E0` branch on freshly zeroed size/capacity does not allocate,
construct records or shrink anything: it writes size 0 and returns `RET 4`.
The C++ initializer folds that no-op and the unobserved temporary base table.
Its vtable field is image metadata, not a reconstructed callable C++ table.
It is for fresh storage; applying it to a live owner would lose old records.

The grow helper `BABFE0` is evidence only, not reconstructed. Its live listing
omits `BAC053..BAC05C` after the `_free` call at `BAC04E`; disk disassembly
shows stack cleanup, the pointer/capacity stores, and register restoration.
No Ghidra repair is included in this packet.

## Evaluation and integration

Counts <=0 return positive zero without touching records; count 1 returns
that record's value. For multiple points, an argument <= first coordinate
returns the first value; an argument >= last coordinate returns the last
value. Those endpoint paths do not update the cache.

For an interior argument, a cached coordinate greater than the argument
resets the cache to zero. The scan advances while the argument is strictly
greater than the next coordinate. Equality does not advance it, so the
chosen interval at a knot may depend on the incoming cache. The final
expression is `(argument - coordinate) * slope + value` using consecutive
x87 FSUB/FMUL/FADD under the current control word, followed by explicit
`FSTP32; FLD32`. The native comparisons use FCOMI/FCOMIP and CF/ZF branches;
unordered values follow those exact branches, not ordinary C++ comparisons.
No double approximation or FMA is used.

The API is `TracerParameterCurveStorage::sample_00ba9da0(float) noexcept`.
It is a naked MSVC Win32 member method with the original ECX/RET4/ST0 shape.
Native valid-storage/cache preconditions remain: no bounds guards, repaired
ordering, null fallback for positive counts, or implicit ownership are added.

The type-4 event `872790` captures the actual curve owner at `8729F4`, calls
the current slot +08 at `872A05`, reloads that same owner's current table and
slot, and calls again at `872A1A`. It spills the returns to binary32 at
`872A0F` and `872A20` before `BA9A90`. A runtime binding for the actual
`BA9DA0` slot can call this member on that actual owner. Any other current
slot still needs its real binding. The event's required callback must not
silently snapshot dispatch or create/attach an empty curve.

## Verification and limits

The ignored `local/tracer_curve_probe/` fixture extracts the original bytes,
verifies them against live Ghidra, and calls original/rebuilt code over the
same concrete storage. Its focused differential run passed 66,816 calls
under 12 x87 precision/rounding control words. It compared complete owner
state, 80-bit ST0, caller-style FSTP32 result, x87 status/control, an existing
x87 stack sentinel, EBX/EDI preservation and RET4 stack balance. Inputs cover
empty/negative/singleton counts, endpoints, knots, backward and forward cache
movement, duplicate coordinates, signed zeros, subnormals, infinities and NaNs.
The constructor's final state and returned pointer were checked separately.

The source and fixture compile with MSVC Win32 `/W4 /WX /O2`; the probe uses
`/link /MANIFEST:EMBED`. No fixture is added to the repository test suite.
`scripts/build.ps1` passed Release Win32 and the one configured existing test
(`reconstructed_math`). CMake was unchanged, so that baseline build does not
include the new source; the strict fixture compilation above compiled it.
Native exception/unwind registration is not ABI-replaced, and allocation,
Lua loading, refcount destruction, live dispatch wiring and game validation
remain outside this packet. The evaluator is native-byte fixture tested;
this is not a claim that the tracer effect has run in the game.

## AJ combined integration verification

The source is registered in bsp_core. The combined strict MSVC Win32 build and
both existing CTests passed. The report records the focused fixture replay, exact
call/tail checks, saved Ghidra name/signature preimages and comment readback.
Required external runtime bindings, original exception ABI and gameplay remain
limited as described above; this integration does not extend the fixture coverage.

## Correction from docs/TRACER_PARAMETER_CURVE_LOADING.md

AK reconstructs reserve/resize/append, real Lua loading and destructor/scalar lifetime over existing1Ch storage. Returning-free Ghidra gaps are repaired. Strict combined-library fixtures pass3072 curve pairs and84 actual Lua comparisons. Current family-slot composition, original exception ABI and gameplay remain unvalidated.
