# Avoid-zone distance, direction, and nearest-boundary selection

Addresses: 0041aea0, 0041b840

`0041AEA0` computes a polygon's distance/direction pair and `0041B840` selects the
nearest polygon in a group. Both bodies are reconstructed completely through required
callee bindings. The C++ interfaces are semantic projections, not native ABI replacements.
Names are hypotheses, not recovered symbols. Ghidra was read only in this packet.

| Routine | Native ABI and body | Coverage | C++ entry |
| --- | --- | --- | --- |
| `0041AEA0` | ECX=zone; stack point, slack, direction output; ST0 distance; `RET 0Ch` at `0041AFF3`, `0041B004`, `0041B112`; body `0041AEA0-0041B114` | complete through required bindings | `avoid_zone_distance_normal_0041aea0` |
| `0041B840` | ECX=group; stack output, point, slack, push; EAX=output; `RET 10h` at `0041B93E` and `0041B957`; body `0041B840-0041B959` | complete through required bindings | `avoid_zone_nearest_boundary_point_0041b840` |

No function definition is needed. The previous planner doc's end `0041B957` is
the address of the final three-byte return instruction; the inclusive final byte is
`0041B959`.

## Distance and direction

`0041AEA0` clears the direction to `(0,0)` before any rejection. Its initial distance
is `FLT_MAX`: live bytes `FF FF 7F 7F` at `00D7A248`. It expands each minimum by
subtracting slack and each maximum by adding slack. The gate is
`minX-slack <= x < maxX+slack` and `minZ-slack <= z < maxZ+slack`.
The upper edges are excluded. Unordered comparisons reject. Slack is an AABB
extension, not a radial distance ceiling; negative slack is not clamped.

If `00416B50` reports inside, the routine calls `00416F30` with push `1.0f`, then
sets direction to `normalize(returned_point - query_point)` and returns exactly zero.
The returned point is an offset boundary point, not necessarily the unshifted closest
point. The normalize helper yields zero on exactly zero length.

Otherwise the routine walks edges in native vertex-pointer order, starting with the
closing edge `(last, first)`. It calls `00419AB0` for distance; only a strictly smaller
distance replaces the best. On replacement it calls `004F4B50` for the clamped closest
point, then stores `normalize(query_point - closest_point)`. This is the direction from
the boundary toward the outside query, with a possible zero vector. Ties, NaNs, and
distances equal to `FLT_MAX` do not replace an existing result.

The valid-input contract requires stable, nonempty polygons with valid vertex pointers
when their bounds pass. The native code reads the last pointer at `0041B012` before
the empty-range comparison; no safe behavior for malformed empty polygons is invented.
The semantic snapshot and callback outputs must not alias the input point.
Neither the zone collection nor the borrowed vertex vectors may change during host
calls. The native loops reload their counts; the C++ snapshot loops require stability.

## Wrapper polarity and selection

For each zone, `0041B840` calls the kernel with the original query and slack. For a
new minimum `d > 0`, it returns the candidate `query - direction * (d + push)`.
`0041B8BA-0041B8C4` adds and spills `d+push`; `0041B8D2-0041B8DE` multiplies and
spills each component; `0041B8E4` and `0041B8EF` subtract those components from the query.
A new minimum `d <= 0` returns the original query immediately, overriding any earlier
candidate. No qualifying zone, including an empty group, also returns the query.
Strict comparison preserves the first zone on equal distance.

Positive push crosses the selected boundary toward the polygon interior. It is not
an outward clearance margin. For a square `[0,10] x [0,10]`, query `(12,5)`, slack `5`,
and push `1`, the formula returns `(9,5)`. This describes the local direction; a large
push is not guaranteed to leave the final point inside an arbitrary polygon.

Both live callers were inspected. `009E38A2` passes slack `100000`, push `1`.
`007ABCA8` passes slack `800` and push `80 + 3 * (unsigned_word_at_owner_174h % 23)`.
The latter formula follows `007ABC41-007ABC99`, with constants read live at
`00CE3950`, `00D7A2B0`, and `00CF1440`. Four stack arguments agree with `RET 10h`.
Both kernel callers (`0041B5D0`, `0041B893`) pass point, slack, and output direction,
matching `RET 0Ch`; this packet does not reconstruct the separate `0041B580` consumer.

## Callee bindings and data producers

All five direct callee bodies were read in full from pseudocode and assembly before
declaring these bindings. Their own transitive helper behavior remains external.

| Call site in `0041AEA0` | Native callee | Required contract |
| --- | --- | --- |
| `0041AF62` | `00416B50` | AL containment result: unexpanded half-open AABB followed by odd horizontal-ray crossings; `RET 4` |
| `0041AF87` | `00416F30` | Point input, point output, 16-byte edge output, endpoint-byte output, float push; `RET 14h`. Select the closest clamped projection, then add interpolated vertex offsets times push. C++ keeps the address name; this packet does not replace its producer policy |
| `0041AFBD`, `0041B0BC` | `00419260` | ECX=delta pair, ST0=zero for zero length or reciprocal length; `RET`. Existing implementation: `include/bsp/native_vector2_math.hpp`, with explicit CRT access |
| `0041B069` | `00419AB0` | ECX=start, EDX=end, stack query, ST0=segment distance; `RET 4`. Retain its segment-parameter and short-length cutoff behavior |
| `0041B096` | `004F4B50` | ECX=two consecutive endpoint pairs; stack output, query; EAX=output; `RET 8`. Clamp the parameter supplied by `004F3630` to `[0,1]`, then interpolate |

The view uses existing `std::array<float,2>` values and a borrowed vector, with no
new native record overlay. Native offsets were checked against producers:
`0041CCD0` initializes both bounds from the first X/Z point at `0041CD43-0041CD78`.
`0041CFB6` supplies `zone+14h` to `00415010`; that existing helper updates
`minX,minZ,maxX,maxZ`. The constructor appends 24h vertex-record pointers through
the native array/count at `+0h/+4h`.

`0041A200` writes outgoing unit direction at vertex `+8h/+Ch`, outgoing length at
`+18h`, and a normalized right normal of the summed incoming/outgoing unit vectors
at `+10h/+14h`. `00416F30` uses those fields to project and offset. The concurrent
lateral-record packet owns the producer reconstruction and its winding policy;
no producer names, ledgers, or code were changed here. Degenerate and self-intersecting
polygons are not proven to have a globally outward normal. The outside branch's
`query-closest` polarity does not depend on the stored vertex normal.

## Verification and limits

The complete owned listings were read and filtered for ECX/EBX/ESI/EDI/EBP.
In the kernel EDI retains the zone, EBX the query, ESI the direction output, and
EBP walks the vertex pointer array only in the outside branch. ECX still holds the
zone at the containment call, is restored from EDI at `0041AF85`, and is explicitly
changed to direction/start/segment storage before the later calls. The wrapper
keeps query in EBX, group in EDI, and its zone-pointer iterator in ESI. Stack
slots reused as float scratch were resolved from these frames, not decompiler types.

`python tools/verify_report_calls.py reports/avoid_zone_boundary.json` checks every
owned call site and both sets of callers against the live Ghidra bodies. The standard
`./scripts/build.ps1` is the Win32 `/W4 /WX /fp:strict` build plus existing CTest suite.
Its final result is recorded in the report. No shared tests were added.

One ignored fixture is `local/avoid_zone_boundary_probe.cpp`, run with
`./local/run_avoid_zone_boundary_probe.ps1` in this worktree. It compiles a Win32 probe
with `/link /MANIFEST:EMBED`. It verifies `(12,5)->(9,5)` and that the expanded upper
edge `x=15` rejects before any host call, clears direction, and leaves the wrapper's
point unchanged. Its square-geometry host is synthetic: this is fixture evidence,
not differential execution of original bytes. First-on-tie is assembly-reviewed.

These routines are not wired into the runnable game host: its
`zone_group_for_layer_004120d0` returns zero and its
`nearest_zone_boundary_0041b840` logs then returns the input point
(`src/game_hosts_ship_ai.cpp:1684-1692`). No frame-causation or game-validation claim
is made. The projection keeps visible binary32 stages but does not establish bitwise
x87 parity, native exception ABI, native object aliasing, or installed-game behavior.

## Ready follow-up

| Packet | Remaining work |
| --- | --- |
| `avoid_zone_boundary_bindings` | Bind the five real callee implementations and adapt native zone/group storage; reuse the existing reciprocal-length implementation. Differential-check the owned routines including cutoff/degenerate cases |
| existing avoid-zone polygon producer packet | Own `00416F30`, `00419AB0`, `004F4B50`, and their transitive primitives only after lease checks. Preserve the producer's winding/offset policy rather than substituting generic geometry |

## Correction from docs/AVOID_ZONE_QUERY_BINDING.md

All five boundary host calls now have concrete implementations connected by
`AvoidZoneBoundaryQueries`. Its inputs are zones from the real 0041CCD0 storage
producer in `AVOID_ZONE_OWNER.md`. It copies native corner order, derived values
and stored bounds into owned snapshots, so the original allocations can be
released afterward. Layer/group selection remains the caller's responsibility.
The bridge deliberately avoids the approximate semantic polygon producer.

00416F30 is complete in `AVOID_ZONE_OFFSET.md`; 00419AB0 and 004F4B50 plus their
004F3630 solver are covered in `AVOID_ZONE_SEGMENT_MATH.md`. This closes the
implementation dependency portion of the follow-up table. Component differential
checks and a constructor-to-query fixture are recorded in the individual
reports. The two boundary routines themselves still have semantic C++
interfaces; whole-query native parity, executable wiring and gameplay remain
unverified.
