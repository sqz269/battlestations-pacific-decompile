# Avoid-zone manager queries and point offset

Addresses: 00417580 00417E40 00417E90 00417EF0 004120D0

Packet `orch6_avoid_zone_manager_queries`, worker `agent/orch6-zone-wrappers`.
Live Ghidra target `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, checked by the repository query/export client
before each live batch. Analysis remained read only. Descriptive names are
hypotheses. Source is `src/avoid_zone_manager_queries.cpp`; the report is
`reports/avoid_zone_manager_queries.json`.

These wrappers connect the planner's existing interfaces to concrete geometry.
They do not own or load a scene. The caller keeps native storage and a matching
`AvoidZoneTable` snapshot, retaining group and zone order while any plan holds
an identity. `AvoidZoneManagerZone` is the pair of group/zone indices in that
table, with `{-1,-1}` for no containing zone. It has no relationship to a game
pointer. Native storage continues to provide the actual corner records and
their lazy clearance cache; semantic corner copies must not substitute for
that mutable cache in the follower.

| Routine | Inclusive body | Original ABI | Coverage |
| --- | --- | --- | --- |
| 00417580 | 00417580-00417600 | ECX zone; stack out,point,float margin,byte test in word; EAX out; RET10h at004175E8/004175FE | complete wrapper |
| 00417E40 | 00417E40-00417E58 | ECX manager; stack point,signed layer; EAX zone/null; RET8 at00417E56 | complete wrapper |
| 00417E90 | 00417E90-00417EED | ECX manager; stack layer,toward,from,out_zone,out_edge; AL bool; RET14h at00417EEB | complete wrapper |
| 00417EF0 | 00417EF0-00417F5B | ECX manager; stack layer,toward,from,out_point; AL bool; RET10h at00417F59 | complete wrapper |
| 004120D0 | 004120D0-00412111 | ECX manager; stack signed layer; EAX group; RET4 at00412104/0041210F | complete; existing implementation reused |

All five functions are defined and have zero flow gaps under `bsp.py ghidra
flow`. Earlier docs used the starts of final three-byte RET instructions as
inclusive ends for 00417580 and 00417EF0; this report includes all RET bytes.
The existing geometry header's 00412119 end is also corrected here to00412111.
There is no `no_ghidra_function` span or unread wrapper branch.

## Point offset and direction

00417580 first tests the low byte of its fourth argument. Nonzero invokes
00414F50 on the native stored bounds at zone+14h, then00416B50 on the polygon.
An outside point copies through unchanged. Zero bypasses both gates and always
projects. The projected branch passes the original margin through an x87
load/store to00416F30, then copies its point output. It never negates, clamps
or normalizes the margin or the selected direction.

00416F30 adds `margin * ((1-t)*a.offset_dir + t*b.offset_dir)` to the closest
unshifted clamped edge projection. The producer0041A200 writes the right-hand
normal of the sum of incoming/outgoing unit directions at0041A418/0041A423,
normalizes at0041A454/0041A45A, and reverses/recomputes positive winding sums.
Consequently a positive margin offsets outward for a valid simple produced
ring. This is a producer-backed statement; arbitrary, degenerate or
self-intersecting records carry no global outwardness guarantee. Interpolation
is not normalized, so the margin need not equal perpendicular distance.

Both known call sites in009E3780 were checked. EBX becomes1 at009E380E;
whole-listing EBX filtering shows no intervening write before009E3840 and
009E38FE push it. The calls at009E3851 and009E390B therefore request containment
gating and pass positive margins10 and5 respectively. Goal/pose output is read
through returned EAX. No other xrefs to00417580 or00417E40 exist in this live
snapshot.

The API accepts native storage and its matching semantic polygon. Reuse
`avoid_zone_polygon_snapshot`; do not rerun the older approximate polygon
producer. Stored bounds may intentionally differ from clipped vertices.
00416F30 must select a candidate on the projection path. The native otherwise
copies uninitialized stack locals; this C++ boundary detects its unchanged
endpoint sentinel and throws `std::domain_error`. It never publishes a made-up
origin. NaN margins are still forwarded when a candidate exists.

## Manager selection and outputs

00417E40 calls00412120, then004178F0, preserving native first-containing-zone
order. 00417E90 and00417EF0 call004120D0, then004179D0. The existing004120D0
implementation is already correct: return the first exact key, otherwise the
last slot with a lower key, otherwise slot0. It scans the entire array, so it
must not be replaced with an early-break sorted lookup. 00412120 agrees on the
producer's sorted table. The existing group/zone record declarations are
reused; no new native overlay is introduced.

004179D0 initializes a running point from the third manager argument and
passes that same point through all zones. Hits move it toward the second
argument. The manager's public endpoint order is therefore `(toward, from)`,
and the crossing nearest `toward` wins. 00417E90 passes the caller's edge output
directly into this search and copies the temporary zone only when AL is true.
Both zone and edge remain unchanged on a miss. 00417EF0 discards zone/edge and
only copies its local point on a hit. AL survives each wrapper's epilogue even
though Ghidra's current signatures display a void/undefined result.

A valid group with zero zones answers no hit. A manager with zero groups is
different: native lookup reads slot0 unconditionally, and the wrapper then
dereferences it. The C++ wrappers throw `std::invalid_argument` for that
native-invalid precondition. The existing standalone layer lookup retains its
documented empty-table `-1` projection. Runtime callers should preserve the
real rebuild's layer0 creation and zone lifetimes.

| Site | Containing function | Native callee | Concrete binding |
| --- | --- | --- | --- |
| 00417596 | 00417580 | 00414F50 | contains_point_00414f50 |
| 004175A2 | 00417580 | 00416B50 | avoid_zone_contains_point_00416b50 |
| 004175C5 | 00417580 | 00416F30 | avoid_zone_closest_offset_point_00416f30 |
| 00417E4A | 00417E40 | 00412120 | avoid_zone_group_for_layer_00412120 |
| 00417E51 | 00417E40 | 004178F0 | avoid_zone_first_containing_004178f0 |
| 00417ECE | 00417E90 | 004120D0 | avoid_zone_group_for_layer_004120d0 |
| 00417ED5 | 00417E90 | 004179D0 | avoid_zone_group_segment_hit_004179d0 |
| 00417F2E | 00417EF0 | 004120D0 | avoid_zone_group_for_layer_004120d0 |
| 00417F35 | 00417EF0 | 004179D0 | avoid_zone_group_segment_hit_004179d0 |

All dependency bodies above were read. Call-site argument setups were also
checked for009E3075,00422BAB,0042315C and009E4216. 004120D0 has21 live xrefs;
the wrapper/planner ones were checked here, and the existing pure lookup body
supplies its contract independently of unrelated callers.

## Verification limits

`reports/avoid_zone_manager_queries.json` records the standalone Win32 build,
the existing CTest results, live call-row checks, and one ignored manifested
integration fixture. No tracked test cases were added. The fixture uses one
fixed convex native-record ring and a translated semantic copy to check layer
selection, zone identities, nearest-crossing orientation, miss preservation,
positive/negative margins, and the two native-invalid boundary cases.

The fixture passed all12 checks, compiled with `/W4 /WX /fp:strict` and
`/link /MANIFEST:EMBED`. The live verifier passed all18 call rows with zero
failures. These are integration-fixture and static native-call results, not
whole-manager differential or gameplay evidence.

`scripts/build.ps1` passed MSVC Win32 Release and built `bsp_game.exe`.
After `verify-seeds` confirmed installed/Ghidra byte equality, both existing
CTest checks passed: `reconstructed_math` and `native_math_differential`.

The wrappers are complete over their documented inputs, but their containment,
SAT and edge-crossing dependencies retain the semantic geometry implementation's
existing numeric limits. This packet does not claim whole-manager native
differential parity. Exact leaf-kernel integration is coordinated through the
primary worker. This packet does not change executable wiring, original ABI,
the game installation, or gameplay state.
