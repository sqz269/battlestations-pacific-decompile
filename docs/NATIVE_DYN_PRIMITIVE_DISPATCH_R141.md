# Native sphere and box dispatchers

Addresses: 00C518D0, 00C48330, 00C50740, 00C50DD0; consumed math 004011D0,
00401170; read-only shape producers 00C585B0 and 00C57B90.

R141 supplies four complete normal collision/ray bodies and four independent
one-slot tables in `NativeDynPrimitiveDispatchRuntime`. They use the existing
recovered CRT sqrt service through the native float-rounding boundary. This
closes four of the seven dispatcher gaps remaining after R140. Shape creation,
other dispatchers, application admission and gameplay remain open.

## Native contracts

| Entry | Inclusive end | Bytes | Table | Original stack arguments |
| --- | --- | ---: | --- | --- |
| C518D0 sphere/sphere | C51C16 | 839 | D7A1CC | result, A, matrix A, B, matrix B; RET 14h |
| C48330 box/sphere | C48BD4 | 2213 | D7A1D4 | result, A, matrix A, B, matrix B; RET 14h |
| C50740 sphere/ray | C50DCF | 1680 | D7A204 | result, shape, start, end; RET 10h |
| C50DD0 box/ray | C51473 | 1700 | D7A1FC | result, shape, start, end; RET 10h |

The four bodies total **6,432 bytes and 2,155 instructions**. All use AL as
the hit result; upper EAX bits are not meaningful. Owner ECX is not read by
these original bodies. The source table thunks receive actual
`DynStaticDispatchObjectStorage` in ECX and preserve the listed argument
cleanup. Each table has one callable entry; adjacent source metadata carries
the borrowed CRT context. Keep the noncopyable runtime and its CRT pointees
alive at stable addresses while owners/scenes use the tables.

The existing shape producers establish body pointer +4, kind +8, transform
+34..+60, radius +210 for a 214h sphere record, and half-extents +210/+214/+218
for a 21Ch box record. The sphere constructor produces an identity shape basis
and descriptor center; the box constructor copies twelve transform floats.
Their 394 bytes were rechecked against the live program and PE as read-only
layout evidence. R141 does not reconstruct or invoke those constructors.

## Preserved behavior

* Sphere/sphere composes both centers, tests squared center distance against
  the squared radius sum, and rejects exact tangency. A hit writes count=1,
  two body-local surface witnesses and the A-to-B normal. Coincident centers
  retain the original zero-length normalization behavior.
* Box/sphere selects the box using kind=1 and accepts either argument order.
  It transforms the sphere center to box coordinates and computes distance
  outside the extents. Exact tangency can pass. The inside case retains the
  native nearest-face selection, tie rules and abs/division schedule. It
  computes the contact pair using the native half-depth arithmetic, then swaps
  witnesses and reverses the normal when the input order requires it.
* Sphere/ray transforms both segment endpoints to shape space, evaluates the
  native quadratic, and checks the first root against [0,1]. For a start inside
  the sphere it returns the original start point and normalized segment
  direction. An entering hit returns the interpolated world point and radial
  normal. Degenerate arithmetic and ordered/unordered branches are preserved.
* Box/ray clips the transformed segment against all three box slabs, retaining
  the native interval comparisons and selected entry axis. It transforms the
  resulting point and normal back to world space. Starting inside or using a
  zero-length segment retains the original axis/normal degeneracies.

Collision output is a count followed by nine floats. Ray output is six floats
(world point and normal). Misses and all unwritten result bytes retain their
incoming values. The implementations add no geometry fixes or fallback normals.

## Arithmetic and source boundary

Private kernels retain all original non-relocated instruction bytes, including
the x87/SSE schedule, F32 spills and predicates. Five exact constant patterns
are rebound to source storage. An extra final CRT argument occupies EBP+1Ch
for collision kernels and EBP+18h for ray kernels; adjusted cleanup is RET 18h
and RET 14h respectively. Original locals and argument offsets remain intact.

The consumed 31-byte 4011D0 boundary loads a stack float, calls BF7030 with ST0,
stores the result to float, reloads it, and returns with RET4. The explicit
source context adapter keeps this rounding and stack alignment, with ECX
save/load/restore and RET8 for context plus value. The existing 28-byte abs
boundary is consumed with its exact schedule. These are service adapters,
not new CRT implementations or changes to the older semantic math interfaces.

The COFF audit checks **2,166 original instructions** across the four bodies
and abs reference, **42 branch destinations**, call/data relocations, context
offsets, cleanup and all five constant bit patterns. It separately checks all
14 instructions of the float-sqrt adapter. No native exception metadata,
private-stack aliasing or binary replacement claim is made.

## Validation

Strict MSVC Win32 build and **all three existing CTests pass**. No permanent
tests were added. Ignored probes use /MD, /fp:strict and embedded manifests.

**59,232 pairs match 99,983,616 exact bytes.** Each dispatcher is compared
through its explicit source API and production source table over 617 scenarios
and all 12 masked x87 precision/rounding modes with default MXCSR. Cases cover
translated/rotated bodies and boxes, varying radii/extents, reversed box/sphere
order, overlap/miss/tangency, adjacent float boundaries, inside starts, slab
directions and zero-length/zero-radius/zero-extent degeneracies. The same mode
is set before constructing each side's inputs.

The comparison includes AL hit, the entire poisoned result buffer, body and
shape input records, endpoints and CW/SW/tag/MXCSR. No data/pointer normalization
is applied. All **11 native dispatcher return sites** are reached (2/2/3/4).
An integer MOV before each copied native RET records coverage without changing
registers, flags or FP state. Full return coverage does not establish every
predicate combination or arbitrary geometry coverage.

**192 scene pairs match 107,208,048 comparison bytes**, recording **22,164
service events and 384 snapshots** (counts per side across paired cases).
The existing complete narrow-phase task consumes sphere/sphere and both
box/sphere scene cells through these production tables, compared with the
copied native task and native dispatchers. It exercises shape chains, filters,
separated/overlapping bodies, event flags and task ranges in all 12 FP modes.
Real pool critical sections and the event spinlock execute; final unlock is
checked. The box/box cell remains empty because that dispatcher is still open.
Ray tables are exercised by the direct probe, not by this scene task.

Scene traces normalize only the task table pointer and exclude the pool's
24 OS-managed critical-section bytes and FP instruction pointers. Actual
allocation/service sites, recursion depth, raw records, manifolds and events
are compared. The inherited controlled-dispatch/replacement counters are unused.
This is serial scene evidence, not concurrency testing.

Both fixtures explicitly initialize the geometry fields established by the
native producers. They do not run full sphere/box construction, bounds refresh,
support methods or lifetime slots. Primary shape vtable words retain poison;
scene shape vtable words are zero and never called. This narrows the fixture
claim to these geometry consumers and task paths.

Primary native collection verifies 6,539 bytes; the scene union verifies
**10,205 bytes in 25 spans**. Those totals overlap. The additional 394 producer
bytes are disjoint. Both comparison sides share the recovered CRT numerical
service and existing body initialization; these dependencies are not
independently re-proven by the new fixture.

## Remaining work

Box/box, terrain/convex and convex/ray dispatchers remain unreconstructed.
Primitive shape construction/support/lifetime, remaining world tasks and
application context admission still need source wiring and validation. Valid
borrowed records and CRT lifetime are required. Arbitrary geometry/nonfinite
inputs, unmasked FP traps, malformed pointers, private-stack aliases, native
exception ABI, concurrent lifetime and gameplay remain unproved.

See `reports/native_dyn_primitive_dispatch_r141.json` for native bytes, call
rows, instruction audit, coverage, annotation receipts and sealed provenance.
