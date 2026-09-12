# Ship AI lateral records: the avoid-zone corner producer

Addresses: 00417610 0041A200 0041CCD0 00423190 009D58F0

The record at path node `+10h` is a nine-float avoid-zone corner record. It is
not produced by a routine at `009D5920`: that address is the `MOV [ESI+10h],EAX`
inside the already reconstructed `009D58F0`. `00417610` selects a record; the
allocation/position producer is `0041CCD0`, the derived-geometry producer is
`0041A200`, and the lazy `+20h` producer is `00423190`.

New reconstruction: `include/bsp/ship_ai_lateral_record.hpp` and
`src/ship_ai_lateral_record.cpp`. Existing `ship_ai_path_search.cpp` retains
the attach routine. `ShipAiPathLateralAnchor` in `ship_ai_path_follower.hpp`
remains the compact four-float consumer projection; the new explicit copy
function connects the native record to it. No reinterpret cast is valid.

Names are hypotheses. Ghidra was read only; proposed names and prior values
are recorded in the report and name ledger for the primary integrator.

| Routine | Original ABI / native body | Coverage |
| --- | --- | --- |
| `00417610` | ECX zone; signed stack index; EAX record; `RET 4` at `00417621`; body `00417610-00417623` | complete reconstruction |
| `0041A200` | ECX zone; byte flag in one stack word; AL boolean; `RET 4` at `0041A4CB`/`0041A4D6`; body `0041A200-0041A4D8` | complete geometry and control flow through existing math/CRT boundaries |
| `009D58F0` | ECX node; no stack args; `RET` at `009D5929`; body `009D58F0-009D5929` | complete read; existing reconstruction reused |
| `0041CCD0` | ECX zone storage; stack scene-path handle and layer; EAX zone; `RET 8` at `0041D0AF`; body `0041CCD0-0041D0B1` | partial producer evidence: initialization `0041CCD0-0041CE02`, source-point/store loop `0041CE62-0041CF47`, derive call `0041D058-0041D063`; allocation growth, clipping and association branches are not reconstructed |
| `00423190` | ECX zone; stack record; `RET 4` at `004234E6`; body `00423190-004234F8` | complete top-level static read, no reconstruction; zone/layer selection and temporary-list lifetime contracts remain bounded dependencies |

## Producer layout

The zone is a distinct `24h`-byte object, with a pointer array at `+00h`, count
at `+04h`, capacity at `+08h`, layer at `+0Ch`, associated entity at `+10h`,
and four bounds floats at `+14h..+20h`. The new list view declares only its
first eight bytes. Each array entry points at another `24h`-byte allocation.

`0041CD4D/0041CD7D` and `0041CEEF/0041CEF1` allocate 24h bytes; the matching
`ADD ESP,4` instructions are `0041CD85` and `0041CEF6`. The first point comes
from `007AF800` at `0041CD32`, later points from the call at `0041CE7E`.
`007AF800` refreshes the owner's world pose when necessary, transforms a path
point by its world matrix, divides by homogeneous w, and writes x/y/z.
`0041CCD0` keeps x and z, discarding y. The first point is always retained;
later points pass a squared xz-distance test strictly greater than 25 against
the last retained point (`0041CEDB-0041CEE9`). The subsequent world-bounds
clip `0041A540` is outside this reconstruction.

| Record offset | Meaning | Producer evidence |
| --- | --- | --- |
| `+00h/+04h` | scene-path corner x/z | `0041CD92/0041CD9C`; repeat `0041CF03/0041CF0D` |
| `+08h/+0Ch` | outgoing unit vector `(next-current)/length` | initialized zero at `0041CDAB/0041CDB3`; derived `0041A36A/0041A371` |
| `+10h/+14h` | normalized right normal of the sum of incoming and outgoing unit vectors | initialized zero at `0041CDA1/0041CDA6`; raw sum normal `0041A3FC-0041A423`; normalization `0041A42D-0041A45A` |
| `+18h` | outgoing edge length | zero `0041CDB6`; derived `0041A33E` |
| `+1Ch` | signed wrapped outgoing-minus-incoming heading | zero `0041CDBB`; derived `0041A3F0/0041A3F7` |
| `+20h` | cached clearance scale limiting an outward corner offset | initialized -1 at `0041CDC0/0041CDC8` (float at `00D7A260`); cached at `004234BC` |

The x/z source is proved by the producer, not inferred from the follower.
Likewise `+10h/+14h` is a corner bisector normal, not an outgoing-edge normal.
`+20h` is neither the outgoing length nor a measured polygon breadth.

## `0041A200`: derived geometry and winding

For each pointer, the previous/next pointer wraps around the array. The
incoming vector is `current-previous`, normalized by `00419260` at
`0041A2C1`. The outgoing vector is `next-current`; its squared length is
computed in x87 and spilled once at `0041A30C`. Only a value greater than
the double `1e-10` at `00CE3820` takes the sqrt call at `0041A320`.
Otherwise length is zero, including an unordered comparison. The outgoing
components are divided by that length anyway (`0041A354-0041A362`).

The headings use the same instruction sequence as the existing `00414EB0`
reconstruction: `_CIatan2` with ST0=x/ST1=z gives atan2(z,x); float-spill it,
subtract from the double promoted float pi/2, spill, and add the promoted
float 2*pi once if negative. `0041A3F0` passes outgoing heading then incoming
heading to `00438B10`; its `RET 8` establishes the two stack-float arguments.
The result is stored at `+1Ch`.

For incoming unit `(ix,iz)` and outgoing unit `(ox,oz)`, the unnormalized
offset direction is `(iz+oz, -(ix+ox))`. `00419260` at `0041A42D` supplies its
reciprocal length. This is the right normal of the tangent bisector. The
native helper returns zero for a zero vector; the new routine reuses that
helper, including its exceptional-input behavior.

The signed turns are accumulated with one float store per addition. A
nonpositive or unordered sum returns AL=1. A positive sum returns AL=0. If
the supplied flag is nonzero, it first reverses the pointer array and
recursively recomputes with flag zero (`0041A4BA-0041A4BE`), then **still
returns zero** at `0041A4C5`, ignoring the recursive result. The constructor
calls with flag one at `0041D063`; the other xref is the flag-zero self-call.

For an ordinary simple polygon this makes the final xz winding counterclockwise,
so right normals point outside. The exact native rule is the signed-turn sum;
there is no separate area test or repair of self-intersections/degeneracy.
Empty input returns true. One- and two-record inputs are not rejected, and
zero outgoing length still divides by zero. The C++ retains those rules and
requires valid nonnegative counts and readable native pointers. It does not
reset the cached clearance when deriving or reversing records.

The complete listing was filtered for ESI/EDI/EBX/EBP: ESI remains the zone;
EDI is the array cursor until the reversal swap; EBX first denotes the current
record and later its `+10h` vector; EBP preserves that current record across
the reciprocal-length call. No register-input guess supplies the layout.

## `00423190`: what the cached scale means

Both callers were read: `009D5923` supplies the just-selected corner record,
and `00423534` supplies the indexed record before an offset-point query.
The guard is `COMISS record[20h],0; JA done` at `004231B8/004231C4`.
Therefore positive caches return immediately; zero, negative and unordered
caches compute again. In particular, Ghidra's printed `<=0` is incomplete for
NaN. The computation enters the manager's critical section when nonnull and
leaves it after the store.

Let `p=(x,z)`, `d=offset_dir`, and `base=p+d`. It starts with `r=800`, a
candidate at `base+800*d`, and a square centered there with half extent 800.
`call_004120d0` selects from the zone's `+0Ch` layer, and `call_00417a40` takes
that square to obtain a temporary segment list. Their complete selection
contracts were not read, so this packet does not claim which scene geometry
the list includes. `004158E0` intersects `base -> base+1600*d` against that
list, shortening its current endpoint after each hit. If a hit exists,
`r=0.5*length(base-hit)` and the candidate becomes `base+r*d`.

`00419FC0` walks the list and takes the minimum `00419AB0` point-to-segment
distance, call it `q`. While `q < r-1`, the next `r` is the smaller of
`r-50` and `(r+q)/2`. If that falls below 25 it substitutes 20 and exits;
otherwise the candidate is recomputed as `base+r*d` and the distance repeats.
The stored `r` is a clearance-like radius/offset limit for that selected
geometry. The initial one-unit shift `base=p+d`, max initial 800, reductions
of at least 50, 25 threshold and 20 fallback are all native, not tuning choices.
Constants were read at `00CE3930..00CE3953`, `00CE3880`, `00D7A210` and
`00D7A280`. None of this algorithm is implemented by the new module.

`00423520` confirms the contract from the other side: it ensures the cache,
chooses the smaller of the requested scalar and `record+20h`, then writes
`p + scalar*d` (`00423539-0042358A`, `RET 0Ch` at `00423590`). It does not
enforce a positive requested scalar. This is supporting consumer evidence,
not a new reconstruction.

## Calls, corrections and verification

The report carries every new routine's direct native call sites, and separate
supporting call rows with their real containing functions. Reused heading
code is an equivalent sequence, not a fabricated native call to `00414EB0`.
The only new nonlibrary routine dependencies are the already reconstructed
`00419260` and `00438B10`; sqrt uses the existing explicit native CRT access.
Heading retains `00414EB0`'s existing current-host-CRT boundary. No library
was ported and no unresolved call was replaced by a stub.

| Was | Is | Evidence |
| --- | --- | --- |
| `009D5920` listed as a producer function | interior field store inside `009D58F0` | live proto/body and `MOV [ESI+10h],EAX` |
| `00417610` implicitly a record producer | signed remainder pointer-array accessor | `IDIV ESI` at `00417619`; load at `0041761E` |
| `IDIV` cited at `0041761B` | instruction is at `00417619`; `0041761B` loads the array | whole 8-instruction listing |
| negative index "wraps toward zero" | quotient truncates toward zero; remainder keeps dividend's sign and may address before array | `CDQ; IDIV ESI`, EDX indexing; no modulo fixup |
| attach helper comment says metric refresh precedes storage | node handle is stored before refresh | `009D5920` precedes call `009D5923` |
| follower layout left producer unread | nine-float layout established by `0041CCD0` and `0041A200` | producer store table above |

No new module is connected to `bsp_game.exe` in this packet. Existing game-host
tables record the accessor and cache updater with zero calls, and the new
derivation routine is not wired. There is no frame-behavior claim or gameplay
validation. Build/CTest and call-row verification results are recorded in
`reports/ship_ai_lateral_record.json`; native differential testing of the new
whole routines has not been performed. No new test cases were added.

## no_ghidra_function

No missing routine definition was needed. `009D5920` is explicitly an interior
instruction, not a missing function. Three listing holes were checked as raw
bytes without changing saved analysis: `0041A49D` is a 3-byte padding
`LEA ECX,[ECX]` ending at `0041A49F`; `0041CE2C` and `0041CFA0` are 3-byte
`ADD ESP,4` instructions ending at `0041CE2E` and `0041CFA2`. The latter explain
the decompiler's spurious early returns after `_free`. These do not affect
the new accessor or derivation control flow. Last actual instructions and
inclusive body ends are separate fields in the report.
