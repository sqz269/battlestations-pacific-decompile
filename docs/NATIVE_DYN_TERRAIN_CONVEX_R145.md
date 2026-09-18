# Complete terrain/convex collision and dispatcher table

## Scope and entry contract

R145 reconstructs the complete `00C53630..00C549C8` normal method: **5,017
bytes / 1,476 instructions**. `NativeDynTerrainConvexRuntime` supplies its
one-slot source table for the stateless four-byte owner at `00E17438`, whose
native table is `00D7A18C`. This closes the remaining collision dispatcher
method among the eight tables listed in `DynDispatchVtables`.

Native ECX owner is unused. The five stack arguments are result, shape A,
matrix A, shape B and matrix B; return cleanup is `14h`. The result begins
with an integer count, followed by at most eight nine-float contacts: two
points and a normal. The count is cleared before scanning vertices. The
normal return constructs EAX zero or one from the count; the source exposes
a C++ Boolean result.

One shape is terrain kind **5** and the other is convex mesh kind **4**.
If terrain is second, the method uses the opposite shape/matrix ordering,
then swaps the output points and negates the normal. It reads convex mesh
pointer `+210h`, mesh vertices/count at `+0/+4`, and 16-byte vertex records.
No shape virtual method is called by this collision method.

## Terrain producer evidence

The complete constructor reference `00C58840..00C58919` is 218 bytes;
its bounds method `00C58690..00C5883C` is 429 bytes. Both are live/PE verified
references, **not new source implementations**. The constructor is named
`DYN_TerrainShape_Construct_00C58840` in Ghidra with its old annotations retained.
It receives body in ECX, then destination shape and descriptor on the stack,
returns the shape pointer, and pops eight bytes. The normal path publishes
the base and terrain tables, writes common fields, copies the local transform,
then calls the bounds method, which calls `00C55FC0` to update body bounds.
Native exception metadata and class construction/lifetime remain separate work.

| Shape offset | Field | Descriptor source |
| --- | --- | --- |
| `04` | body pointer | incoming ECX |
| `08` | kind | `10h` |
| `34..63` | 12-float local transform | `14h..43h` |
| `210` | sample pointer | `5Ch` |
| `214/218` | width/height | `44h/48h` |
| `21C/220` | X/Z spacing | `4Ch/50h` |
| `224` | quantized height multiplier | x87 reciprocal of descriptor `58h` |
| `228` | height bias | `54h` |
| `22C` | sample mode | `60h` |

Mode zero loads float samples. Mode one decodes unsigned 16-bit samples as
`sample * multiplier + bias`, except `FFFFh`, which maps to **-1000.0f** at
`00D7A240`. Other modes use zero. The existing shape bounds method follows
the same decoding rules. Descriptive field names are hypotheses, not symbols
recovered from the original library.

## Preserved sampling and contact behavior

The method composes both shape/body transforms and visits convex vertices in
their original order. It converts each position to terrain grid coordinates,
checks inclusive bounds, obtains four neighboring heights and performs the
native interpolation. A vertex at or below the interpolated height emits a
contact; scanning stops at eight contacts.

Two details are retained exactly. Inclusive checks allow the last grid
coordinate, while sampling still accesses right/down neighbors. This packet
does not claim the external sample producer always provides that padding.
Callers must supply storage covering the actual accesses; the fixture owns an
extra row and element. Also, `C54804` and `C5480F` both multiply the projected
horizontal witness coordinates by **X spacing (`+21Ch`)**, even when Z spacing
differs. No alternate witness calculation or boundary repair is introduced.

The full x87/SSE schedule, spills, sample conversion, interpolation, normal
normalization, point order and count cap are preserved. Constants keep exact
bits: sign mask `80000000h` at `D7A208`, `-1000.0f` at `D7A240`, and double
`-1.0` at `D7A250`. The private float-sqrt adapter consumes the existing
`004011D0` behavior and recovered `00BF7030` CRT service. It retains the
original float store/reload, with explicit borrowed CRT context. The root's
extra final private context is at `EBP+1Ch`, with private `RET 18h`; the
source table retains the original five-stack-argument interface.

## Verification

Project/program are checked before collection: `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. Primary collection matches **5,735 live/PE bytes
in 10 spans**, including the 647 producer-reference bytes and 31-byte float
sqrt reference. The scene union independently matches **25,747 bytes in
42 spans**; these totals overlap. The root listing has no gaps.

The COFF audit checks all **1,476 original instructions, 40 branch targets,
three constant bit patterns and 14 float-sqrt adapter instructions**. Every
other encoded byte matches. Native 16-bit sentinel comparisons are emitted
with their original encodings because MSVC otherwise chooses equivalent
short-immediate forms. Branch/call/constant relocations and context/return
adjustments are checked explicitly.

One primary fixture uses producer-verified consumed terrain fields, the
existing convex shape/body constructors and owned sample/vertex storage.
It checks float, quantized/sentinel and zero-fallback modes, both shape orders,
unequal spacing, inclusive edges with owned padding, empty/multi-vertex meshes,
misses, all contact counts zero through eight, and explicit/table entry paths.
All 12 masked x87 precision/rounding combinations are used, with FP state reset
before inputs and again before calls. **36,864 pairs match 89,948,160 bytes**,
including 26,148 hits. Output, body/shape records, vertices, sample arrays and
FP control/status, tag and MXCSR match; FP instruction pointers are excluded.
The sole original return executes, and owner/output guards remain intact.

A reused narrow-phase fixture supplies the actual terrain/convex and existing
general-convex tables to cells `(4,5)`, `(5,4)` and `(4,4)`. It checks the full
serial task, real pool and critical-section behavior, allocation/free events,
and contact state. **192 pairs match 107,607,400 bytes**, with **22,459 service
events and 384 snapshots**. Two non-FP `MOV` markers in the original image
confirm both terrain/convex shape-order branches execute. Source execution
uses the production table. Existing normalization covers task/convex table
and mesh pointers, OS critical-section bytes and FP instruction pointers.

Strict MSVC Win32 build and all three existing CTests pass. Both comparisons
and the instruction audit repeat after integration; separate immutable archives
preserve provenance. Exact hashes and CALL checks are in
[the report](../reports/native_dyn_terrain_convex_r145.json).

## Remaining work

The fixtures construct terrain records from confirmed consumed fields; they
do not execute or reconstruct the terrain class constructor, bounds update,
destructor or allocator. Existing convex support, general-convex collision and
CRT services retain their earlier evidence rather than receiving new independent
validation here. External sample allocation/padding, arbitrary geometry,
malformed records, unmasked traps, native exception/RTTI metadata and concurrent
execution are not proven.

The eight dispatcher methods now have source tables, but complete shape
production/lifetime, remaining world/ray-query tasks, ordinary application
admission and gameplay remain open. This packet does not establish a runnable
game or native binary compatibility.
