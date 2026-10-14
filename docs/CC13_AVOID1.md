# CC13 avoid1: avoid-zone navigation leftovers of segment 1 (cc13-avoid1)

Packet A of cc13-avoid1: 27 avoid-zone functions in segment 1 that the cc9 ship lane hosts
around but had no record for. Code: `include/bsp/cc13_avoid1.hpp`, `src/cc13_avoid1.cpp`
(bsp_core). Every name is a hypothesis, not a recovered symbol. Status for every new
function: reconstructed and Win32 build-tested only; no fixture, no original-ABI test, no game
validation, and nothing is wired into the game.

Two models are used, chosen to match the existing siblings:

- Geometry queries take `AvoidZoneBoundaryZoneView` snapshots and the existing
  `AvoidZoneBoundaryHost` (the interface of 0041AEA0 / 0041B840 in `avoid_zone_boundary.hpp`),
  so the ship lane can bind them through its existing boundary adapter.
- Lifetime code (groups, manager, registry, node allocation, scratch destructor) works on
  native-layout structs with `static_assert`ed offsets and the real allocator services of
  `AvoidZoneAllocationAccess`; callees outside this packet are named host hooks.

## Verified existing implementations (ledger only, no second copy)

| Address | Existing code | Result |
|---|---|---|
| 00416270 | `avoid_zone_outline_walk_00416270`, src/ship_ai_free_bearing.cpp | Faithful. Both walk directions, the facing test `(origin - near) . (e.z - s.z, -(e.x - s.x)) < 0`, the 004F3BA0 call (ECX origin, EDX near end, stack radius / far end / points, checked at 004163C2..004163E0), the two-crossing choice (`|p1 - far|^2 <= |p0 - far|^2` keeps p1), one crossing, zero crossings answering the far end, the null-next exit and the non-monotone `previous > far2` exit all match. |
| 0041B4E0 | `GameAvoidZoneRuntime::group_segment_point`, src/game_avoid_zone_runtime.cpp | Equivalent. The thunk calls 004179D0(toward, from, running, &slot2, &slot3) with its own argument slots as the zone/edge outputs (discarded); the host seeds running = from and copies the hit point, which is what 004179D0 does. |
| 00417E60 | the 006F3C5A arm in src/game_hosts_ship_ai.cpp (`zones.group_for_layer` then `zones.offset(group, xz, 10.0f, true)`) | Equivalent. Native: 00412120(layer) then 00417B10(group)(out, point, margin, 1), RET 10h. The host uses 004120D0 for the group, which `avoid_zone_geometry.hpp` documents as agreeing with 00412120 for the ascending order 00417CA0 keeps; its `group != 0` test never fails (the runtime returns index + 1). |

## Reconstructed

| Address | C++ | Original ABI | Behaviour and notes |
|---|---|---|---|
| 004166E0 | `cc13_avoid_draw_selected_segments_004166e0` | thiscall(ECX = &head; rgba*), RET 4 | One 0085FB30 line per selected segment, (start.x, 0, start.z) to (end.x, 0, end.z), colour copied from the argument; next, or next_run when closes_run is set or next is null. |
| 00416840 | `cc13_avoid_draw_zone_screen_00416840` | thiscall(ECX zone; matrix*), RET 4 | Each vertex (x, 0, z) through 004142E0; screen point (t.x / 666.0, -t.z / 500.0) (doubles 00CE3848 / 00CE3840, x87 FDIV then float store); 00860030 white line from the previous point, the ring starting at the last vertex. The last vertex is read before the empty test. |
| 004169A0 | `cc13_avoid_draw_zone_arrows_004169a0` | thiscall(ECX zone; rgba*), RET 4 | 0085FB70 arrow at y = 3.0 (00CE3854) from the previous vertex to each vertex, both colours the argument; after the first arrow only, 00860BE0 at the first vertex with EDX = yellow (1,1,0,1), radius 5.0 (00CE3850), stack colour red (1,0,0,1), detailed 1. The image builds both colours as guarded function-local statics (00E1763C / 00E17650); the C++ uses constant statics. |
| 004178B0 | `cc13_avoid_draw_group_arrows_004178b0` | thiscall(ECX group; rgba*), RET 4 | 004169A0 on every zone of the group. Vtable-only, no direct caller. |
| 00417990 | `cc13_avoid_draw_group_screen_00417990` | thiscall(ECX group; matrix*), RET 4 | 00416840 on every zone of the group. Vtable-only. |
| 0041AC60 | `cc13_avoid_zone_arc_hit_0041ac60` | thiscall(ECX zone; centre*, radius, start, sweep), RET 10h, AL | Bounds grown by radius (half-open, unordered rejects), count >= 1, then per edge (previous, current) 004F3BA0 crossings; each crossing's bearing is 00414EB0's inline formula (atan2, 00CE3830 - a, + 00CE3828 below 0) called here as `heading_angle_00414eb0`; d = 00438B10(bearing, start); hit when 0 < d < sweep or sweep < d < 0. |
| 0041B510 | `cc13_avoid_group_arc_hit_0041b510` | thiscall(ECX group; same four), RET 10h | 0041AC60 per zone, true on the first hit. No direct caller found (vtable or unread). |
| 0041B120 | `cc13_avoid_zone_distance_0041b120` | thiscall(ECX zone; point*, slack), RET 8, ST0 | FLT_MAX outside the grown bounds; 0 when 00416B50 says inside; else min of 00419AB0(previous, current, point) over the ring (strict `best > d`). |
| 0041B680 | `cc13_avoid_group_distance_0041b680` | thiscall(ECX group; point*, slack), RET 8, ST0 | Minimum of 0041B120 over the zones. Caller 00941D30 (spawn placement); the lua_spawn_cc13 note says its class arm is not taken from 00942CB0. |
| 0041B290 | `cc13_avoid_zone_tangent_0041b290` | thiscall(ECX zone; out*, point*, slack), RET 0Ch | Outside: valid = 0 only. Else 00416B50, then 00416F30(point, push 0.0); n = (point - closest) / len with len = sqrt (00BF7030) above 1e-10 squared, else 0 (no zero-division guard); inside negates n and len; len > slack gives valid = 0; otherwise valid = 1, z = -0.0f - nx * d, x = d * nz, d = slack - len. The 00416F30 endpoint byte lands in the dead point-argument slot. Caller 0041B7E0. |
| 0041B700 | `cc13_avoid_merge_tangent_0041b700` | fastcall(ECX acc, EDX addend), RET | Invalid accumulator copies a valid addend; both valid: the addend is negated (times double -1.0, 00D7A250) when the dot product is <= 0, then added, valid = 1. The NEG/SBB/TEST 00E17628 pairs are a safe-bool test. Caller 0041B7E0. |
| 0041C0F0 | `cc13_avoid_hull_list_buy_node_0041c0f0` | stdcall(next, previous, const handle&), RET 0Ch, EAX node | operator new(10h) (00BF681B), next/previous, 00C40F50 copy of the hull handle at +8h; the catch-all at 0041C16B frees the node (00BF65AC) and rethrows. The image's placement null tests on node+4 / node+8 are true for any node; the C++ keeps only the node test for `next`. |
| 0041D0C0 | `cc13_avoid_group_clear_0041d0c0` | fastcall(ECX group) | Each non-null zone: free each record (00BF65AC), count = 0, free the record array (00BF6989) and null it, free the zone (00BF65AC), null the slot; then group count = 0. |
| 0041D150 | `cc13_avoid_group_remove_owner_0041d150` | thiscall(ECX group; owner), RET 4 | owner 0: 0041D0C0. Else each zone with +10h == owner (read before the null test) is released through 004167C0 plus the array and zone frees, and its slot takes the last slot's value with count - 1 (the same slot is re-examined). Caller 00423640. |
| 0041DE70 | `cc13_avoid_group_destroy_0041de70` | fastcall(ECX group) | vtable 00CE3858, 0041D0C0, free the slot array (00BF6989) if non-null and null it. |
| 0041E000 | `cc13_avoid_registry_end_load_0041e000` | fastcall(ECX registry) | +10h = 10.0 (00CE38B8); with a non-empty list, over the FIRST layer: for i, j < n (n read once) the 0041BAE0 cell (grid[n * clamp(j) + clamp(i)] * layer+10h, n re-read for the clamps, checked index) is folded in with `current > h ? current : h`. Called at the end of 004248A0. |
| 0041EA30 | `cc13_avoid_simplify_ring_0041ea30` | stdcall(path*, params*), RET 8 | See the header. Index array (n + 1) words from 00BF55BE, freed with 00BF6989; tolerance 00419010(10, 3, 30, params+4, n - removed) per pass; perpendicular (u, v) built by the image's three-store add/sub sequence (s = dz + dx, u = dx - s, v = s + u, checked against the bytes DE C2 / DE EA at 0041EC69..0041EC87); deviation `ez * nz + ex * nx`, negative side times -1.2000000476837158 (00CE38C0); back-off by one candidate; passes repeat while something was removed and more than ten points remain; final checked erases. Caller 00423640. |
| 0041F0D0 | `cc13_avoid_registry_destroy_0041f0d0` | fastcall(ECX registry) | vtable 00CE38E4, 0041DED0 (delete every layer), 0041D710 on +4h, [00E17620] = 0, base vtable 00CE3818. Called from the scalar deleting destructor 0041F690. |
| 0041F980 | `cc13_avoid_manager_destroy_0041f980` | fastcall(ECX manager) | vtable 00CE38F4, 0041F600, 0041CC80(&+4h), 0041E200(+6Ch), free(+70h head, unconditional) and null it, free(+60h) when non-null, null +60h/+64h/+68h, [00E17624] = 0, base vtable 00CE3818. |
| 00421500 | `cc13_avoid_manager_construct_00421500` | fastcall(ECX manager), EAX | vtable 00CE38F4, null +60h..+68h, +70h = 0041BF50 head, +74h = 0, +58h = 0, twenty slots null, +4h = 00BD1860. |
| 00423520 | `cc13_avoid_corner_offset_point_00423520` | thiscall(ECX zone; out*, index, distance), RET 0Ch | record = records[index % count] (signed IDIV, 00417610 inlined), 00423190(zone, record), s = cache > distance ? distance : cache, out = position + s * offset direction (float products and sums). Caller 009D9390 BSP_ShipAiPathNode_ResolveTurnPoint (not hosted). |
| 004235A0 | `cc13_avoid_draft_scratch_destroy_004235a0` | fastcall(ECX scratch) | Releases the vectors whose first word is at +58h, +44h-object (elements are 10h-byte vectors released by 0041F7B0), +30h, +20h, +10h, nulling each triple. 00423C50's scratch destructor (also its unwind 00C5E5DB). |
| 00424730 | `cc13_avoid_registry_construct_00424730` | fastcall(ECX registry), EAX | vtable 00CE38E4, head from 0041BFD0, size 0, then THREE layers: operator new(28h) + 00424680 with 10, 20 and 70 degrees (floats 3E32B8C3h, 3EB2B8C3h, 3F9C61ABh at 00CE3990 / 00CE398C / 00CE3988), each 0041C1F0 + 00420140 + tail link. A throwing 00424680 frees the block (unwinds 00C5E673 / 7E / 89). |

## Packet B (segment 61)

Code: `include/bsp/cc13_avoid1_seg61.hpp`, `src/cc13_avoid1_seg61.cpp`, namespace
`bsp::cc13_avoid1`.

| Address | C++ | Original ABI | Behaviour and notes |
|---|---|---|---|
| 00947A30 | `cc13_spawn_request_complete_00947a30` | fastcall(ECX request), RET | Called at 009487A1 from 009483D0 after the member loop and before +C0h = 1. Empty callback (+84h size 0): nothing. Name starting '[' (data read without a null test): "[supportmanager]" (00CF6B78, 00425850 case-insensitive) copies the +CCh member vector into the by-value slot with 00947810; then 00947A30 calls 0065D070 with ECX = [[00E198C4]+BCh]; any other bracketed name goes to 00A16EF0(ECX = &name, EDX = &members). Otherwise operator new(0Ch) zeroed LuaVariant vector, per member (checked index) 00884A90 entity variant, 006EDF00 push_back (the temporary is destroyed on unwind), 00884AF0; then 00887E50(ECX = [[00E188A8]+1A08h]; 0, &name, args, 0, -1), then 006B88E0(args, 0), 00BF6989(args->first), 00BF65AC(args). |

Not attempted (size; each needs its own packet): 0093D2D0 BSP_MapScene_SetupOceanMaterials
(773 listing lines), 0093FF10 BSP_SoundOverlay_Draw (924 lines, no live reference), 00945450
(the SpawnLight Lua native, 7B0h-byte frame), 00947BC0 (session message 42h constructor with
checked vector copies), 0094B600 (its 731-line handler) and 0094BFF0 (263 lines, property-bag
spawn for BSP_PowerUpManager_UseItem).

## Skipped

- **00421CE0** (caller 00423640): the grid contour tracer. It walks an eight-neighbour boundary
  over the byte grid with two jump tables (00422274, 00422294), marks visited cells, converts
  cells through 0087FC30, inlines the entity pose refresh (00414DB0, 00413920, the 16-float copy
  into +CCh, 00B63D50 into +110h), transforms through that matrix with x87 reg-reg multiplies
  (00422001..0042204D, DC-form candidates), appends through 004215D0, and then runs a scanline
  fill pass. A faithful reconstruction needs a byte-level x87 audit and an owner layout this
  packet does not have. Recommended for an x87-capable worker.

## Adoption notes for the ship lane

1. **Registry default layers (LIVE, no RNG).** 00424730 appends three default layers (10, 20, 70
   degrees; 0042476E..00424886). `GameAvoidZoneRegistry::GameAvoidZoneRegistry`
   (src/game_hosts_avoid_zones.cpp) appends one (10 degrees). And 004248A0 calls 0041DED0 at
   004248C1, which deletes every layer, defaults included, before the file's layers are
   appended; `load_scene_layers` instead clears and pushes one default layer ahead of the file
   layers, so every host index is one higher than the image's. `squadron_layer_34c` is read by
   src/game_hosts_units.cpp (three sites). Because indices are only used against the same list
   and 0041DF40 picks the largest positive slope limit below tan(1.5), the chosen layer differs
   only when no file layer's limit lies between tan(10 degrees) and tan(1.5) (the host then
   picks its 10-degree default; the image picks its best file layer, or file layer 0), and for
   missions without a `.nav` (image: the 70-degree default, index 2; host: the 10-degree
   default). game_hosts_avoid_zones.hpp records the default grid as all zero, so sampled
   heights should agree in that last case. Does not touch 00BD2F10.
2. **Registry +10h (0041E000) is not produced by the host.** `NavStream::end_load` only logs. The
   reader of registry+10h was not identified in this packet; if one is hosted it reads nothing.
   Not RNG.
3. 00416270, 0041B4E0 and 00417E60: no mismatch (table above).
4. **Spawn completion callback (LIVE).** `GameMissionLuaHost::complete_spawn_request_0094c777`
   (src/game_hosts_lua.cpp) says the route from the record+84h callback to Lua is open; it is
   00947A30 at 009487A1, inside 009483D0 right after the member loop and before 009487AD sets
   +C0h, not at 0094C777. Differences: (a) bracketed names are never Lua calls:
   "[supportmanager]" issues 0065D070 member orders and any other "[...]" creates an AI group
   through 00A16EF0, while the host would look up a global with that bracketed name and log it
   missing; (b) the arguments are 00884A90 entity variants built from the +CCh member pointers,
   pushed in member order (the host pushes resolved entity tables, nil for unresolved ones);
   (c) the call goes through 00887E50 with (0, name, args, 0, -1), not a direct lua_pcall. No
   00BD2F10 use inside 00947A30 itself; whatever the called script or 0065D070 / 00A16EF0 draw is
   outside this function.

## Uncertainty

- x87 divides and products are written with double intermediates and one float rounding, the
  image's precision under a 53-bit control word; a 24-bit control word (D3D default) rounds once
  in single precision, which differs only in rare double-rounding cases.
- The image's CRT invalid-parameter reports (00BF6713) are host hooks that must not return; the
  C++ continues with the native read after the hook, as the image would if the handler returned.
- Exception unwinding is reproduced only where stated (0041C0F0, 00424730's layer block); the
  constructors' member unwinds (00421500's list/vector, 00424730's list 0041E290 and base
  004152D0) are not.
- Native-layout structs name only the words these bodies touch; their other words are unread
  here, and the 00423C50 scratch layout beyond the five vectors is unknown.
