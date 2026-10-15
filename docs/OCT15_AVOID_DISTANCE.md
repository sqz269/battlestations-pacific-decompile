# Avoid-zone segment boundary distance and expanded-box query

Addresses: `004171F0`, `004174A0`, `004F49F0`. Packet `oct15_avoid_distance`.

The source covers all three complete caller bodies. `004171F0` uses the new
`segment_pair_distance_004f49f0` wrapper over the already complete public
`004F3730` and `00419AB0` dependencies and borrowed actual CRT access.
`004174A0` reuses the existing
`00416B50` semantic containment and `00416F30` instruction-backed projection.
Names below are provisional descriptions, not recovered symbols.

| Address / proposed name | Body inclusive | Original ABI | Coverage |
| --- | --- | --- | --- |
| `004171F0` / `BSP_AvoidZone_MinSegmentBoundaryDistance` | `004171F0..004172DE` | ECX zone; stack start,end; ST0 distance; `RET 8` at `004172DC` | complete caller body; actual public dependencies |
| `004174A0` / `BSP_AvoidZone_QueryExpandedBoundsBoundaryPoint` | `004174A0..0041757B` | ECX zone; stack point,margin,out; AL admission; `RET 0Ch` at `00417564` / `00417579` | complete caller body; existing geometry APIs |
| `004F49F0` / existing `BSP_Segment2f_SegmentDistance` | `004F49F0..004F4AD0` | ECX a0; EDX a1; stack b0,b1; ST0 distance; `RET 8` at `004F4A1C`, `004F4ABA`, `004F4ACE` | complete outer body; actual public dependencies |

The configured project is `C:/Users/sqz269/bsp.gpr`. `bsp.py` verified live
project name `bsp`, program `/battlestationspacific.exe`, x86 language and image
base before each live analysis/export command. Both initially missing bodies were freshly exported,
then all three were read as pseudocode and assembly. No existing implementation
of the first two addresses was found in `src` / `include`. Live caller queries
for those two returned no callers. No Ghidra
mutation was made; old live names are `FUN_004171f0` and `FUN_004174a0`.

`004171F0` copies the two query endpoints before touching polygon records.
`00417240` loads the last record before the empty-loop comparison. Consequently
the API requires a positive count and stable, readable pointer-array/record
storage. It does not invent an empty result. Edges run `(last,first)`, then
`(first,second)`, through `(penultimate,last)`, including the one-record case.
Each dependency result has the native binary32 boundary at `0041729A`.
`004172A6..004172AA` replaces the running `FLT_MAX` only on an ordered strict
improvement; ties, NaN and positive infinity leave it alone. `00D7A248` was read
live as `FF FF 7F 7F`. There is no early return at zero. A wholly interior query
segment can have positive boundary distance: no containment test occurs here.

| Containing function | Call site | Native callee | Contract / binding |
| --- | --- | --- | --- |
| `004171F0` | `00417295` | `004F49F0` | Body read. ECX edge-start, EDX edge-end, stack query-start,query-end; `RET 8`. New concrete wrapper; return rounded at the caller's FSTP. |
| `004174A0` | `0041753C` | `00416B50` | Body read. Original unexpanded half-open AABB and crossing-number containment; existing `avoid_zone_contains_point_00416b50`. |
| `004174A0` | `00417556` | `00416F30` | Body read. Closest clamped edge projection, closing edge first; strict rank; caller sends positive zero push. Existing `avoid_zone_closest_offset_point_00416f30`. |
| `004F49F0` | `004F4A0A` | `004F3730` | Body read. Existing `native_segment_crossing_004f3730`; on success outer wrapper returns positive zero. |
| `004F49F0` | `004F4A24`, `004F4A32`, `004F4A40`, `004F4A4E` | `00419AB0` | Body read. Existing `avoid_zone_segment_distance_00419ab0` with unchanged required CRT access. Query order b0,b1,a0,a1; `RET 4` each. |

The `004F49F0` ledger's `reconstructed_build_tested` entry points to the private
double helper in `game_hosts_ship_ai.cpp`. That helper explicitly labels itself
`SUBSTITUTION` and says its child bodies were unread. It uses textbook crossing
and zero-length repair, so its build status does not establish native behavior.
This packet does not call or expose that substitution. The integrator is
handling the old named record separately; the new exact outer wrapper receives
its own reconstruction record and preserves the established Ghidra name.

`004F49F0` first calls the exact existing crossing helper. On a miss it makes
all four native endpoint-distance calls, preserving their order and each
binary32 FSTP at `004F4A29`, `004F4A37`, `004F4A45`, `004F4A53`. Comparisons
retain the first operand only when it is ordered strictly smaller, so equality
and unordered comparisons select the second operand, including the final
group selection. The source uses the native FCOMIP schedule for each choice.
This deliberately differs from the outer polygon walk, which rejects a NaN
candidate. No `std::min`, textbook crossing or zero-length policy is inserted.
The complete `00419AB0` API retains ignored solver failure and query-pointer
bits; addresses of borrowed point objects therefore remain observable for
degenerate inputs. The C++ interface does not promise native stack-address
identity or an original-ABI replacement.

`004174A0` copies the zone bounds through x87 binary32 stores, subtracts margin
from both minima and adds it to both maxima, spilling each result. Its exact
FCOMI/FCOMIP order rejects unordered comparisons and admits minimum equality
while excluding maximum equality. Negative margins shrink the box; NaN margins,
coordinates or compared expanded bounds reject. There is no repair for an
inverted box and no Euclidean distance test. The source retains the native
shared margin operand and all four bound stores in inline x87 assembly.

The output layout is established by this routine's own stores and callee
argument offsets: closest point at `+00h`, four-float edge at `+08h`, unexpanded
containment byte at `+18h`, admission byte at `+19h`, endpoint byte at `+1Ah`.
No existing result declaration was found. The C++ type asserts these offsets;
its trailing alignment does not claim a recovered native object size.

On a miss only `+19h` becomes zero. On a hit it becomes one before the geometry
calls; `+18h` receives containment; the closest-point call gets `push=+0.0`.
No candidate (including empty geometry) preserves the old point, edge and
endpoint fields while admission remains one. Native storage and the caller's
`avoid_zone_polygon_snapshot` must describe the same bounds, ordering and
positions, have nonnegative count/readable records, remain stable, and not alias
query/output storage. The wrapper does
not allocate, synthesize a candidate, normalize an edge or apply the margin to
the returned point.

Validation: the integrator's complete build at `0db8158e2` passed all three
existing suites (`reconstructed_math`, `native_math_differential`, `tool_tests`)
and the source packet was integrated into main. The report-call checker passed
all eight rows; `git diff --check` passed. The worker's complete
`./scripts/build.ps1` also passed both suites enabled in its fresh build tree
(`reconstructed_math`, `tool_tests`), 2/2, in 7.12 test seconds. Its full log is
`local/oct15_avoid_distance_build.log`. The native seed suite was enabled in the
integrator tree, where it passed; this packet adds no native fixture.
The worker object timestamp `2026-10-15T16:39:15Z` is later than the final source
and header, so the compiled file includes the concrete wrapper and required no
incremental rebuild. No new test cases or native differential fixture were
added. Source coverage and compilation will not establish original
register/exception ABI, semantic containment's exact
x87 parity, complete floating-point exception equivalence, or gameplay. The
routines have no current executable integration or runtime claim.
