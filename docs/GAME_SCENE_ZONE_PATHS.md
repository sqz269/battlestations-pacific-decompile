# Authored scene path retention

Addresses: 004EA650, 007B34F0. Integration binding in
`game_hosts_scene_contents.hpp/.cpp`; native Ghidra analysis is read only.

`GameSceneEntityRecord::path_points_local` retains the fresh Path's kind-1
property-bag point sequence. `path_points_retained` means this data projection
completed, **not** that a native Path entity was created. `created`, the class
creator tally and the existing unresolved 004EA650 diagnostic are unchanged.

| Routine | Original ABI / body | Coverage |
| --- | --- | --- |
| 004EA650 | EDX name; stack hierarchy parent, frame pointer and two unread arguments; RET 10h; 004EA650..004EA753 | Analysis only; no creator reconstruction. Allocation, name registration, native entity construction and placement remain unresolved in this host. |
| 007B34F0 | ECX path; stack source holder; RET 4; 007B34F0..007B3727 | Partial: fresh kind-1 PathPoints lookup and local XYZ retention (007B354A..007B3604). Group at 007B352E..007B3549, kinds 2/3 at 007B3609..007B36FF, derived-data refresh at 007B3700..007B370C, native point allocation and lifetime are excluded. |

## Producer and order

004EA650 allocates/zeros 210h bytes and calls 0047B660 at 004EA6BC. The
constructor builds the entity, constructs its path component at entity+1E4h
through 007B28C0 at 0047B69B, and publishes path vtable 00CE63E8. The component
constructor initializes its begin/end/capacity and stores the real entity in
component+14h. 004EA6FB invokes entity vtable+98h with the actual hierarchy
argument, world+19CCh and copied frame. It does not read PathPoints itself.

007B38D0's setup passes entity+C0h to 007B34F0 with ECX=entity+1E4h at
007B390C. The other direct callers (007B3920, 007B3940 and 007B3730) pass their
respective actual holder; this retention handles only the scene's fresh kind-1
bag, not those routines' other entity/holder contracts.

007B3552 finds `PathPoints`; 007B3557 loads its nested bag. 00415870 returns
zero for absent begin or `(end-begin)/4`. Its result supplies `Point%002i` at
007B356C and 007B35EC, each with `ADD ESP,0Ch` immediately afterward. Lookup
starts at the current point count (zero for a fresh path), continues after each
append and stops at the first missing or non-block key (native type 6).
It is numeric lookup order, not file order or lexical sorting. Unrelated
blocks and points after a gap do not enter the native sequence.

007B35A3 finds `Pos`. MOVSS loads +0Ch,+10h,+14h into a local triple, then
007B35D8 calls path slot+8h. Path vtable+8h is 00480890, which clears the derived
data flag and enters 007B34B0. The latter copies XYZ unchanged to the allocated
point's first three floats, then appends the pointer. Thus retained triples are
local. The already reconstructed 007AF800 subsequently uses the actual
entity pose and homogeneous divide to produce world XYZ; this binding does not
pretransform or deduplicate them.

Malformed/missing V3 data is reported in `path_points_error`; the retained array
is cleared and `path_points_retained` stays false. This is an explicit unsupported
input boundary instead of emulating native invalid reads or inventing points.

## Hierarchy and map inputs

The existing reader visits the parent before its children and already supplies
the composed world matrix. The binding captures each child's parent from that
actual `SceneEntity::children` relationship while the parser object lives, then
consumes the capture at the child's callback. `scene_id` is a one-based visitation
ID including unregistered entities; `parent_scene_id=0` means no authored parent.
`parent_name`, `local`, `world`, and (for nonzero parent ID) `parent_world` retain
the corresponding actual data. IDs are scene record identities, not native
entity pointers; a parent without a created native object remains unresolved.
The existing gate's parent-pose policy is not expanded by this packet.

`GameSceneContentsHost::root_properties()` returns the actual header
`ScenePropertyBlock`, copied at `publish_scene_root_properties`. It contains
`Map.BorderSizeX/Y` and `Map.MultiPlayMapSizes`. No bounds are inferred from
point extents or substituted with a large rectangle. The existing
`GameUnitsHost::world_bounds_box_00e188a8` still returns false. Bounds producer
004E6C00 reads the map values and calls 004D5BD0, whose stores target
game+711Ch..7130h; the primary integrator owns that separate projection.

## Evidence and verification

The configured wrapper verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe` before each live batch. Ghidra exports were refreshed;
no native annotations were applied. 007B34F0 has a listing gap at
007B3619..007B361F: bytes `8D A4 24 00 00 00 00` encode `LEA ESP,[ESP]`, an
alignment instruction outside the retained kind-1 branch. Both reported body
ends include all bytes of their final RET instruction.

Register provenance was checked across the full 007B34F0 listing: ESI is the
original ECX path, EBX stays zero in kind 1, EBP becomes type 6, and EDI changes
from holder to PathPoints nested bag at 007B3557. `verify_report_calls.py` checks
the report's call-site containment and direct targets.

Installed source for the bounded check is read-only
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/universe/scenes/missions/USN/usn_1_marshall.scn`.
Build and runtime results are recorded in `reports/game_scene_zone_paths.json`.
MSVC Win32 Release and the existing reconstructed_math test passed. The
manifested fixture compiled the exact retention function extracted from the
source with the existing parser: zero parser errors, 21 AvoidZone records and
2,193 local points. Reversing authored point-block order preserved the output.
The first zone's first local point is (370.919,-24.9985,-120.069), with path
frame translation (283.066,0,388.777); its actual parent is `Landscape 03`,
whose authored translation is (3000,0,-4000).

The rebuilt executable also exited 0 after loading USN01 with one mission frame.
`local/scene_zone_paths_runtime.log:865..885` reports all 21 zones retained,
with 2,193 points, `valid=1`, `creator_concrete=0` and actual parent visitation
IDs 1,93,99,105. In particular line 865 has path ID 87, parent ID 1 and 84
points. Line 900 reports 147 scene entities visited, 147 generated and 77
created. The 21 Path records remain outside that created count. These are
scene-retention runtime checks, not validation of polygon construction.
This packet makes no ABI replacement, native differential, polygon-query or
original-game validation claim.
