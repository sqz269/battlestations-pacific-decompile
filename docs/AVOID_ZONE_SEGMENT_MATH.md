# Avoid-zone segment parameters, distance, and closest point

Addresses: `00419AB0`, `004F4B50`, `004F3630`, `00414C60` (consumed-callee CRT adaptation).
Names are hypotheses, not recovered symbols. This packet supplies concrete Win32
math for `AvoidZoneBoundaryHost`; it does not change the zone producer, registry,
intersection policy, or executable host. Ghidra access was read-only against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, port 8089. The wrapper
verifies the project and program before each live query/export batch.

| Routine | Original body and ABI | Coverage | C++ implementation |
| --- | --- | --- | --- |
| `004F3630` | `004F3630-004F372E`, 255 bytes; ECX=a0, EDX=da; stack b0, db, t, u; AL success; `RET 10h` at `004F36B3`/`004F372C` | complete | `native_segment_parameters_004f3630` |
| `00419AB0` | `00419AB0-00419BED`, 318 bytes; ECX=start, EDX=end; stack query; ST0 distance; `RET 4` at `00419B83`/`00419B98`/`00419BC5`/`00419BEB` | complete | `avoid_zone_segment_distance_00419ab0` |
| `004F4B50` | `004F4B50-004F4C50`, 257 bytes; ECX=two adjacent endpoint pairs; stack output, query; EAX=output; `RET 8` at `004F4C4E` | complete | `avoid_zone_segment_closest_point_004f4b50` |
| `00414C60` | `00414C60-00414CA1`, 66 bytes; ECX=delta pair; ST0 length; `RET` at `00414C92`/`00414CA1` | complete private binding adaptation of existing reconstruction | `segment_cutoff_length` |

No unread branches or missing Ghidra functions exist within those bodies. The
public distance/closest interfaces are new typed interfaces with required borrowed
`CameraAxesCrtAccess`; they are not drop-in binary replacements. The raw parameter
solver keeps the native pointer ABI. External CRT dispatch state and `__87except`
effects remain supplied through the existing concrete CRT boundary.

## Parameter solver

`004F3630` solves `a0 + t*da = b0 + u*db`. The second and fourth vectors are
directions, not endpoints. It loads and caches all four input pairs, computes
`denominator = db.x*da.y - db.y*da.x` in x87, then spills the denominator to float.
`004F36A5` calls the existing concrete `circle_relative_equal_004f3560` with that
float and zero; this callee returns with `RET 8`.

The tolerance is the native strict relative comparison: the float-rounded
absolute difference must be less than `max(1,abs(a),abs(b))*float(0.0001)`.
The original x87/SSE instruction schedule, including its unordered branches, is
reused. A near-zero denominator returns false without writing either output.
Otherwise the original expanded products and cancellation order compute t and u;
the implementation does not rewrite these as cross-products of rounded deltas.
t is stored first and u last. Because all inputs are cached before either output
store, outputs may alias input words; if t and u alias, u wins.

## Native short-segment and exceptional behavior

Both consumers first spill `end-start` to two floats, call
`native_vector2_reciprocal_length_00419260`, spill its result, and compute a
float-rounded unit direction. The perpendicular is `(-unit.y, unit.x)`.
The solver receives start, the unnormalized segment delta, query, perpendicular,
the reused query stack slot as t output, and the unit-x local as u output.

**Neither consumer checks the solver's returned success flag.** If the denominator
is near zero, t retains the 32 bits of the query pointer interpreted as a float;
u retains normalized dx. This is native, address-dependent behavior. The kernels
retain the caller's actual query address in the same logical scratch slot. They
do not substitute t=0, retry with a dot product, or select a repaired endpoint.
Zero length produces reciprocal zero; the resulting solver rejection is included
in this behavior. Reconstructed and original objects at different addresses can
therefore differ on this native failure path even with identical coordinates.

Distance compares `0` with t using `COMISS/JBE`: only an ordered-negative t takes
the start-endpoint branch. It then compares t with one; only an ordered-above-one
t takes the end-endpoint branch. Interior and unordered t return the absolute
bits of u (`AND 7FFFFFFFh`), not a fresh Euclidean calculation.

Both endpoint branches square and sum the float-rounded query/endpoint delta,
spill that sum to binary32, and compare it with native double `1e-10`. Below,
equal, and unordered return positive zero. Ordered-above invokes the native CRT
sqrt boundary and retains both float store/reload pairs. The double's live bytes
at `00CE3820` are `BB BD D7 D9 DF 7C DB 3D`; one at `00D7A24C` is `00 00 80 3F`.

Closest point clamps only ordered-negative t to zero and ordered-above-one t to
one; NaN t is retained. It spills t*end.x, t*end.y, `1-t`, and both products of
`1-t` with the cached start coordinates before adding the two terms per axis.
The expression is not changed to `start+t*(end-start)`. All endpoint reads precede
the first output store, permitting output/query or output/endpoint aliasing.

## Existing implementations and binding changes

The solver was previously documented by `GUN_BOT_REMAINDER.md`; its body was
exported and rechecked, and no implementation was present in that packet's C++.
`ship_ai_nav_circle_tangent.hpp` supplies complete relative equality;
`native_vector2_math.hpp` supplies reciprocal length and actual CRT access.
`geometry_helpers.hpp` has no matching segment kernel.

The existing `vector_helpers.cpp` cutoff-length routine calls current-host
`_CIsqrt`, which does not accept `CameraAxesCrtAccess`. Its arithmetic schedule is
retained in a private bridge with the actual borrowed CRT instead. No existing
public vector-helper policy or implementation was changed. The library routine
at `00BF7030` retains its existing name; this packet does not fabricate CRT state.

Distance adds a saved EBP holding CRT, moves only native query argument/scratch
references by four bytes, and changes its private `RET 4` to `RET 8` for the added
stack CRT pointer. Closest point passes CRT in EDX, adds a saved EBX, moves only
its two native argument slots by four bytes, and retains `RET 8`. Original local
offsets and x87 instructions remain intact. Full-listing register filtering shows
distance uses ESI=start, EDI=end, EBX=query and no native EBP; closest uses
ESI=endpoints and no native EBX; the solver uses no callee-saved general register.

## Calls and input producers

Every live direct caller was inspected at its argument setup. No native record
layout was introduced: the boundary caller copies two vertex pairs into adjacent
stack pairs at `0041B03C-0041B063`, then passes the same segment to closest point.
The borrowed public storage is `std::array<float,2>` and `std::array<float,4>`.
The solver's own input cache establishes which arguments are directions.

| Native callee | All direct call sites checked | Containing routines |
| --- | --- | --- |
| `00419AB0` | `00419FE6`, `0041B069`, `0041B23B`, `004F4A24`, `004F4A32`, `004F4A40`, `004F4A4E`, `00809B57`, `008FF8A7` | `00419FC0`, `0041AEA0`, `0041B120`, `004F49F0`, `00809A80`, `008FF640` |
| `004F4B50` | `0041B096` | `0041AEA0` |
| `004F3630` | `00419B1F`, `004F3781`, `004F3C5B`, `004F4903`, `004F4BC8` | `00419AB0`, `004F3730`, `004F3BA0`, `004F4880`, `004F4B50` |

Every internal direct call and the caller rows above carry exact `address`,
`native`, and containing `function` fields in `reports/avoid_zone_segment_math.json`.
The report checker verifies those instructions against live Ghidra bodies.

## Verification and limits

Standalone `scripts/build.ps1` passed in MSVC Win32 Release, including the existing
`reconstructed_math` CTest (1/1). The report checker validated 21 call-site rows
with zero failures. The ignored manifested probe passed 3027 bitwise/return checks.
Its build uses `/MD` to match `bsp_core.lib` and `/fp:strict`.

The ignored focused probe relocates
the six complete original routines' installed/live-matching machine bytes; only
call targets and absolute constant addresses are relocated. Original and rebuilt
geometry share the existing CRT boundary and explicit fixture CRT binding. This
provides differential evidence for geometry, comparison, float-spill, and argument
adaptation behavior; it does not independently validate the CRT reconstruction.
The probe includes endpoint branches, cutoff distance, zero/short segments,
NaN/infinity, signed zeros, output aliasing, no-write solver failure, and generated
finite coordinates. Original and rebuilt calls use the same query addresses.

At this packet's checkout these helpers have no executable host caller; the primary
integrator owns that binding. No gameplay/frame behavior or game validation is
claimed. Zone containment, offset boundary selection, owner lifetimes, and the
registry remain outside this packet. No tracked tests or Ghidra mutations were added.
