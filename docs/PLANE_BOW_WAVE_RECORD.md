# Plane BowWaves record
Addresses: 007cd2f0, 007d1b50, 007d1d30, 007d3e60, 007d4812, 007d5890, 00718000, 007c3700, 00868420.

The three trailing dwords are **model-space x, y and z floats**. They are filled
from the first point of a model marker named exactly `wave`, whose numeric id is
the BowWaves element's one-based index. They are subsequently copied into the
plane's live bow-wave effect records and transformed by the plane transform.
This resolves the dead-field uncertainty in `PLANE_CLASS_FIELDS.md`: the Lua
BowWaves block does not initialize coordinates, but model setup writes them.

## Evidence and layout

All live reads verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, through `bsp.py`'s verified client. Worker Ghidra
access was read-only. Names below are hypotheses, not recovered symbols.

| Native location | Meaning | Evidence |
| --- | --- | --- |
| Descriptor `+550h` | Container base, with begin/end/capacity at `+554h/+558h/+55Ch` | `007d1d36..007d1d58`, caller `007d3677` |
| Definition element `+0h` | Refcounted effect-definition pointer | `007d3629..007d363c`, `007cd31f..007cd331` |
| Definition element `+4h/+8h/+Ch` | Model-marker point x/y/z, floats | x87 copy `007cd337..007cd346`; model-point writer `007d48d7..007d4946` |
| Instance `+538h` | Descriptor pointer | `007d58ad` |
| Instance `+A3Ch` | Runtime bow-wave vector, begin/end/capacity at `+A40h/+A44h/+A48h` | `007d5a56..007d5a5c`, `007c37ac..007c37be` |
| Runtime element `+0h` | Refcounted created effect instance | `007d59c8..007d59dd` |
| Runtime element `+4h/+8h/+Ch` | Same model-space point | `007d5a1a..007d5a41`; affine transform at `007c3803..007c380c` |

The Lua segment `007d3608..007d367d` clears and resolves only the first dword of
the stack record at `ESP+68h`, then appends it. This packet does not claim to
recover its earlier stack contents. Its point must be considered unset at that
stage; assigning a zero-point default would add behavior.

`007cd2f0` is a repeated fill-copy helper: ECX is destination, EDX is count, and
the first stack argument is a source record that does not advance. It increments
the pointer's reference count at pointee `+4h` and performs three x87 float
loads/stores. It consumes four stack dwords (`RET 10h`), of which the last three
are unused in its normal body. This is not a raw 12-byte integer copy: x87 NaN
and exception behavior can differ. No generic container implementation was added.

`007d1d30` takes ECX=vector and one stack source pointer (`RET 4`). Spare capacity
fills one slot and advances end by 10h. Its slow path uses `007d1b50`, which takes
ECX=vector, stack=(output iterator, position owner, position pointer, source),
calls `007cf4b0` with count 1, and returns the iterator to the inserted element
(`RET 10h`). Allocation, iterator validation and container internals remain host
boundaries, rather than reconstructed game behavior.

## Model binding

The fragment `007d4812..007d4953` inside `007d3e60` walks the descriptor records.
At `007d4843..007d487e` it passes the four-byte string at `00d06560` (`wave`) and
index+1 to `00718000`, with ECX loaded from descriptor `+50h`. That helper compares
the marker name exactly and compares its separate numeric id at `+24h`; this is
not a textual `wave1` lookup. The last matching marker wins if duplicates exist.
The fragment requires a present marker and nonempty 0Ch-stride point vector at
marker `+48h/+4Ch`; it copies the first point into the definition record.

The parent restored 17 missing call-fallthrough instructions after `_free` in
`007d3e60`, saved Ghidra, and refreshed the export. Two intentional jump gaps were
left alone; see `reports/plane_bow_wave_flow_repair.json` supplied by the integrator.
Before repair, the decompiler incorrectly returned after string cleanup. Raw
`007d48a1` is `add esp,4` (three bytes), followed by the coordinate-writing path.
The repaired pseudocode was checked against the assembly before reconstruction.

## Creating live effects

`007d5890..007d5ab7` (ECX=plane instance, no stack arguments, `RET`) traverses the
descriptor vector and, for each definition:

1. Builds a 4x4 identity matrix; `00d7a24c` is 1.0f.
2. Calls `00868420` with ECX=return-handle slot, EDX=the manager from
   `(*(00e188a8)+19ECh)`, and stack=(retained definition, matrix pointer, 0, 0).
3. Takes an owned reference to the result and releases the returned temporary.
4. Copies the three point floats, then writes byte `+9h` of a nonnull effect to 0.
5. Appends the runtime record through `007d1dc0` to instance `+A3Ch`, **including
   a null effect result**, then releases its local effect reference.

The routine does not clear the output vector. `007d5ac0` calls it. The further
consumer `007c3700` passes record `+4h` and instance matrix `+CCh` to the existing
affine-point transform at `007c380c`, then samples ocean height. This establishes
that the stored point is local to the model; that update routine was not ported.

## Reconstruction and validation

`include/bsp/plane_bow_wave.hpp` and `src/plane_bow_wave.cpp` implement the model
binding fragment and complete normal-path effect-creation loop. Both record
images retain the 10h shape with explicit 32-bit opaque tokens. Standard array
types represent the three floats and matrix, avoiding camera/world-only types.
Injected hosts perform marker lookup, effect creation, byte writes, vector append
and reference release. They have no default implementations. Native temporary
reference churn is folded into a single owned result, and C++ scope cleanup
releases it if a host throws. Full MSVC SEH/iterator failure behavior is omitted.

The interface requires initialized model points and stable valid records/count
during callbacks. It does not emulate the Lua reader's unset stack data, x87 copy
edge cases, live pointer storage, vector growth or the entire `007d3e60` function.
It is not ABI compatible, fixture-tested against native calls or game-validated.
The Win32 Release build passed, and the existing `reconstructed_math` CTest
passed 1/1. This packet adds no tests; that target does not behaviorally exercise
the new host interfaces. Details are recorded in the paired JSON report.

## Remaining questions

* The general meaning of effect byte `+9h` remains unnamed; only the write is known.
* The full model-loading call chain and behavior for missing markers need a
  separate packet. The native path has no graceful missing-marker fallback here.
* `00868420` internals and effect enable/stop/update behavior remain outside scope.
* Incidental raw entry `007cc580` has no Ghidra function and begins teardown of
  runtime vectors `+A3Ch/+A4Ch`. It was observed as a follow-up only; its complete
  extent was not established, and no function creation/name/reconstruction is
  proposed for it in this packet.
