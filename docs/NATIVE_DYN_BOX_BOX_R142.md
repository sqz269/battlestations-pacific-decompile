# Native box/box intersection and contact clipping

Addresses: 00C49A30, 00C49230, 00C34BA0, 00C33710, 00C33CA0, 00C34240,
00C347D0; consumed math references 00401170 and 004011D0.

R142 supplies the complete seven-body box/box chain and the one-slot
`NativeDynBoxBoxRuntime` table for the actual static dispatcher owner.
Descriptive helper names are hypotheses, not recovered symbols. Terrain/convex,
convex/ray, primitive class production/lifetime, world tasks and application
admission remain open.

## Complete bodies and original ABI

| Entry | Inclusive end | Bytes | Native contract |
| --- | --- | ---: | --- |
| C49A30 dispatcher | C4B540 | 6929 | five stack arguments; AL hit; RET 14h |
| C49230 face contacts | C49A21 | 2034 | ESI work, stack incident-face axis; RET 4 |
| C34BA0 rectangle clipping | C34C58 | 185 | ECX count, EDX quad, EDI other axis; stack output/extents/first axis; RET 0Ch |
| C33710 upper plane | C33C93 | 1412 | ECX count, ESI output, EBX other axis; stack input/count/bound/plane axis; RET 10h |
| C33CA0 lower plane | C34239 | 1434 | same private register/stack contract; RET 10h |
| C34240 upper plane | C347C3 | 1412 | same private register/stack contract; RET 10h |
| C347D0 initial lower plane | C34B96 | 967 | ECX count, EDX four vertices, EAX other axis; stack output/bound/plane axis; RET 0Ch |

These bodies total **14,373 bytes and 4,213 instructions**. The original
dispatcher ignores incoming owner ECX and consumes stack result/box A/body
matrix A/box B/body matrix B. D7A184 is its verified one-entry native table.
Only AL is the hit result. The source thunk takes an actual
`DynStaticDispatchObjectStorage`; keep its noncopyable runtime and borrowed CRT
pointees alive and stable while scenes use the table.

Box inputs use the existing producer-verified 21Ch layout: transform +34..+60
and half-extents +210/+214/+218. R141 records the C57B90 producer evidence.
This packet does not reconstruct the box constructor or lifetime slots.

## Recovered behavior

C49A30 composes both shape/body transforms and tests the three axes of each
box, followed by nine cross-product axes. It preserves the original strict
separation predicates, axis ordering, ties, small-cross-axis threshold and
approximately 0.001 face-axis bias. The cross-axis length uses the existing
float-rounded sqrt boundary.

A selected face axis builds an incident quad and calls C49230 at C4B531 with
ESI pointing to the actual scratch work. That record contains copied body
matrices at +0/+30, composed transforms at +60/+90, extents at +C0/+CC, result
pointer +D8, reference axis +DC, incident corner +E0, clipped world points
+104..+163, clipped count +164, depth +168, normal +16C..+174, orientation sign
+178 and reference-box selector +17C. The root allocates the native 22Ch local
frame with eight-byte alignment; its scratch remains private to the invocation.

C49230 selects the two axes around the incident face, creates its four
vertices, projects them into reference-box coordinates, and invokes C34BA0 at
C496AC. Rectangle clipping runs the four planes in native order: initial
lower, first upper, second upper, second lower. Each stage stops the chain if
the count becomes zero. The initial lower-plane body is specialized to four
vertices; the others retain their four-vertex unrolling and scalar remainder.
They preserve wraparound from the last vertex, inclusive plane membership,
interpolation order and the count pointer in ECX used by their parent.

The face emitter filters clipped candidates using the original approximately
-0.01 signed-depth tolerance, creates paired body-local witnesses through the
native half-depth arithmetic, and writes their common normal in polygon order.
An accepted dispatcher hit can have **zero contacts**; this native result is
preserved. Valid box inputs require result capacity for a count and up to eight
nine-float contacts.

A selected cross-product axis follows the edge-contact path. It selects signed
edge endpoints, solves the two line parameters with the native approximately
0.0001 denominator guard, and uses zero parameters in the near-parallel case.
It writes one paired body-local contact and the selected world normal. Misses
and all unwritten result bytes retain their incoming values.

## Arithmetic and source boundary

Private kernels retain the original register ABIs and every non-relocated
instruction byte. Internal calls remain calls between these complete source
bodies. The dispatcher alone receives an extra final CRT argument at EBP+1Ch
and uses private RET18h cleanup. The existing float-sqrt boundary retains its
native F32 store/reload through an explicit-context adapter; the abs reference
retains its exact schedule. No new CRT implementation is supplied.

Eleven constants retain exact bit patterns, including negative zero and the
rounded tolerances. The assembler shortened one alignment LEA during the first
audit; explicit emitted bytes now retain that original encoding. The subsequent
strict build, audit and native comparisons all pass.

The COFF audit verifies **4,224 original instructions** (the seven bodies plus
the consumed abs boundary), **181 branch destinations**, call/data relocations,
context/cleanup changes and all eleven constants. It separately checks the
14-instruction float-sqrt adapter. Public C++ interfaces and the source table
do not claim native exception metadata, private-stack aliasing or binary
replacement compatibility.

## Validation

Strict MSVC Win32 build and **all three existing CTests pass**. No permanent
tests were added. Both ignored probes use /MD, /fp:strict and embedded manifests.

**49,152 pairs match 82,968,576 exact comparison bytes:** 2,048 deterministic
configurations through both the explicit source API and production table,
across all 12 masked x87 precision/rounding modes with default MXCSR. Cases
include independent body/shape rotations, translations, swapped input order,
thin boxes, coincident faces, tangency, adjacent float boundaries and a rotated
square producing an eight-point contact polygon. The same mode is established
before constructing each side's inputs.

The trace compares AL hit, the complete poisoned result buffer, body/shape
records and CW/SW/tag/MXCSR without data or pointer normalization. It also
retains 24 unused endpoint bytes inherited from the earlier fixture; no ray
coverage is claimed. The corpus reaches **all nine native return sites across
all seven bodies**, and every hit contact count **0 through 8**. Coverage uses
only integer MOV stores before copied native RETs, preserving registers, flags
and FP state. Return/count coverage is not every predicate combination.

**192 scene pairs match 111,093,264 comparison bytes**, with **27,492 service
events and 384 snapshots** (counts per side across paired cases). The complete
narrow-phase task consumes the actual box/box table in scene cell (1,1),
compared with the copied native task and full native geometry chain. Filters,
shape chains, overlap/separation, event flags and task ranges run in all 12 FP
modes. Real pool critical sections and the event spinlock execute; final unlock
is checked. Only the task-table pointer is normalized; the pool's 24 OS-managed
lock bytes and FP instruction pointers are excluded. The inherited controlled-
dispatch and replacement counters are unused. This is serial scene evidence.

Fixture geometry fields follow the verified box producer, but full primitive
construction, bounds refresh, support and lifetime slots are not invoked.
Shape vtable words remain poison in the direct probe and zero in the scene;
neither path calls them. Both sides share existing body initialization and the
recovered CRT numerical service, which this packet does not independently prove.

The primary collection verifies 14,496 bytes. Its scene union re-verifies
**18,162 bytes in 31 spans**; those totals overlap. Unmasked traps, arbitrary
geometry/nonfinite inputs, malformed pointers, private-stack/exception ABI,
concurrent lifetime, application admission and gameplay remain unproved.

See `reports/native_dyn_box_box_r142.json` for native byte/call evidence,
coverage, instruction audit, Ghidra annotations and sealed artifact provenance.
