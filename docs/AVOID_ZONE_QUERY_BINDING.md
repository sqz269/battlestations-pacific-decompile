# Concrete avoid-zone boundary query binding

Addresses: no new native body. C++ integration of 00416B50, 00416F30,
00419AB0, 004F4B50 and 00419260 into 0041AEA0 / 0041B840.

`AvoidZoneBoundaryQueries` makes the boundary routines from
`AVOID_ZONE_BOUNDARY.md` callable using the storage produced by the recovered
0041CCD0 constructor. Its five host methods delegate to existing reconstructed
functions; none returns a placeholder distance, containment result or point.
The actual CRT access remains an explicit borrowed service.

Construction copies each supplied zone in order. It preserves the native corner
pointer order, all nine corner words and the stored bounds. Those bounds can
differ from the clipped vertices: 0041A540 does not recompute them. The adapter
does not invoke `avoid_zone_rebuild_corner_data` or `avoid_zone_from_path_points`,
whose semantic producer is approximate and omits the world clipping path.
`avoid_zone_polygon_snapshot` also exposes the complete value conversion for
consumers of `AvoidZonePolygon`. It copies native `clearance_scale` at corner+20h
into that older projection's `spare` member without changing its bits.

The owned record copies feed 00416F30. The value polygon feeds the existing
00416B50 containment implementation from `AVOID_ZONE_GEOMETRY.md`. A separate
position vector feeds the boundary loop. All vectors are finalized before views
are published; copying and moving the query object is disabled. The original
zone allocation can be released after snapshot construction. Views contain
snapshot-local indices, not game addresses; the original associated entity and
layer grouping are not reconstructed by this adapter. The caller selects the
group and supplies its zones in native order.

| Native call | Concrete binding | Evidence owner |
| --- | --- | --- |
| 0041AF62 -> 00416B50 | `avoid_zone_contains_point_00416b50` | AVOID_ZONE_GEOMETRY |
| 0041AF87 -> 00416F30 | `avoid_zone_closest_offset_point_00416f30` | AVOID_ZONE_OFFSET |
| 0041B069 -> 00419AB0 | `avoid_zone_segment_distance_00419ab0` | AVOID_ZONE_SEGMENT_MATH |
| 0041B096 -> 004F4B50 | `avoid_zone_segment_closest_point_004f4b50` | AVOID_ZONE_SEGMENT_MATH |
| 0041AFBD / 0041B0BC -> 00419260 | `native_vector2_reciprocal_length_00419260` | native_vector2_math |

No native names, signatures or Ghidra functions are added for this adapter.
Original ABI and call-site evidence remain in those individual reports. The
source storage must have a valid nonnegative count and readable pointer array
and records at construction. The CRT access must outlive the queries. Empty
polygons are not silently dropped: just as for the native boundary kernel, an
empty polygon must not enter the outside arm after passing the bounds gate.
A contained valid polygon must let 00416F30 select a candidate. Unknown public
indices/host tokens raise `std::out_of_range` in this new C++ interface.

Validation is recorded in `reports/avoid_zone_query_binding.json`. Component
native-byte differential results do not establish whole-query parity. The
boundary and containment code still use their documented semantic C++
interfaces. Executable ship-AI wiring belongs to another active lease; this
adapter does not claim game integration, original ABI replacement or gameplay
validation.

Follow-up: attach the snapshot to the executable's actual layer/zone lifetime,
then validate a mission path that crosses a loaded avoid zone. Constructor
scene-path, allocator and parent-vtable bindings must reflect the actual loaded
scene. No fallback scene or synthetic zone should become runtime behavior.

## Integration verification

Win32 Release build input `29486c49` passed `scripts/build.ps1`, including both
existing CTest checks and construction of `bsp_game.exe`. One ignored manifested
integration probe passed 12 checks across transformed scene points, native zone
construction, derived corners, snapshot ownership, source release, and the five
query bindings. The inside normalization check uses tolerance2e-7 because the
native float reciprocal need not produce an exactly unit component.

The integrator reran 00416F30's 96,000 bit comparisons and the segment routines'
3,027 differential checks successfully against the merged sources. The segment
fixture shares the reconstructed CRT boundary. The separate clipping fixture
passed its diamond-to-octagon, retained-bounds, touching-reject and complete
allocation-release checks. No tracked test cases were added. These results
cover the stated fixtures; they do not establish whole-query or gameplay parity.

Nine reviewed Ghidra annotations were read back with prior comments preserved.
Eleven false call-return gaps were repaired in the owner, clipping, clear and
supporting unwind routines; no call gaps remain in those four functions. Two
10-byte compiler SEH dispatchers were defined from verified existing bytes.
All 12 affected exports were refreshed. Mutation records and before/after
annotations are linked from `reports/avoid_zone_query_binding.json`.
