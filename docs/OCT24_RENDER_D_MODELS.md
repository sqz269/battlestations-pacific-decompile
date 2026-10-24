# Render D: installed unit models and live native node poses

This is host composition in `GameNativeMissionModel`, under the existing
`BSP_NATIVE_MISSION_SHIP` runtime opt-in. The switch remains OFF by default.
It does not reconstruct `GGame::Render` or supply an Original binary ABI.

## Domain and caller contract

One model component borrows the application's existing VFS, strings, node
runtime, actual-owner registry, geometry pools, material process and texture
cache. Its sampler/effect services are constructed once. Additional units and
assets do not construct another Application, effect domain or preload graph.

`load_installed_unit(logical_name, unit_index)` may run in the initial ready
state or after earlier units loaded. The caller keeps its existing native
shader/resource loading bracket around each load; no batch-finish API is
required. It returns true for a new unit and false for an already completed
unit with that exact logical name. An index cannot be rebound to another
asset, and interrupted native work cannot replay.

Exact logical names cache an asset's native meshes and authored hierarchy.
Each unit receives separate actual generated-model nodes and canonical model
companions. B75170 retains the cached geometry for each node. The cache also
keeps each mesh creator until mission retirement, permitting later spawns
without reloading an existing asset or borrowing a freed geometry identity.
Name aliases are not canonicalized into one cache entry.

`unit_loaded`, `unit_count`, `unit_drawable_count` and `drawable_unit_index`
expose loading state and the stable host-unit association. New drawable
indices append; existing indices remain stable. `section_count` counts native
sections across unique cached logical assets. `load_installed_ship` preserves
the diagnostic route as unit zero in an empty domain, and `installed_name`
returns the first logical asset name.

Unit keys are sparse. `unit_hierarchy_count` and `drawable_hierarchy_index`
expose the unchanged source hierarchy indices for authoritative device callers.
`update_hierarchy_transform(key, original_item, composed_world)` synchronously
publishes a borrowed, already composed world matrix through B6DB10 to every
actual visual or palette node associated with that item. It stores no angles,
pose or override. A valid metadata-only item has no actual node and returns
false; invalid original item indices fail before native publication.

## Pose, visibility and lifetime

The existing source hierarchy adapter computes `authored_local * parent` with
00413920 and caches the resulting asset-relative matrix. It preserves the
authored hierarchy values intrinsically to the asset; it does not read a unit
pose into that cache or create a second simulation hierarchy. Native visual
nodes remain flattened as in the earlier Fletcher diagnostic.

`update_unit_transform(unit_index, current_matrix)` composes each drawable's
immutable asset-relative matrix with the synchronously borrowed current unit
matrix, then calls the complete actual-storage B6DB10 setter. Its temporary
matrix exists only on the stack. The native setter copies local+B0, and when
flags5C has bits A set it clears bounds bits138/30 and flags5C, notifies the
current +A0/current virtual3C, then invalidates native descendants. It never
publishes a cached world matrix directly. The primary scene owner retains
responsibility for the genuine root attachment at +A4.

`set_unit_visible(unit_index, visible)` calls B6DA70 with factor 1 or 0 and
recurse=0 for every flattened drawable, preserving configured immutable plane
part defaults described below. The caller rereads authoritative unit
liveness/visibility each frame; this domain stores no duplicate flag or pose.
Unknown indices return false from both update APIs. Dead units can remain
resident and hidden until whole-mission retirement. There is no per-unit
unload or host-index reuse contract.

The primary first retires commands and scene associations. `retire_drawables`
then releases model creators through their current native terminal and releases
each cached mesh creator exactly once. Healthy empty domains can also retire.
Shared renderer drain precedes `after_native_renderer_drain`. An interrupted
acquisition, pose publication or retirement retains the domain for the process;
there is no speculative rollback or destructor retry.

Per-node light collection remains the earlier null-lighting diagnostic.
Original per-unit light membership, animation production and damage model
switching are not supplied by this asset/affine adapter. Authoritative device
articulation can use the original-item publication interface. Missing VFS data,
absent authored matrices and unsupported visual associations fail explicitly.

## Original evidence and ABI

The batch checked `config/target.json` (`C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`) and the live Ghidra program. Existing exported
pseudocode/listings and recovered native implementations remain the evidence;
no Ghidra names, comments or body-count credit changed.

| Address | Original interface and use |
| --- | --- |
| 00413920 | ECX left; stack destination/right; RET8. Row-major product with original x87 ordering. Used for authored and live affine composition. |
| 00B6DB10 | ECX actual node; stack source; RET4. Listing B6DB10..B6DB56 confirms copy, conditional cache clearing, +A0 notification and B6DA30 descendant invalidation. |
| 00B6DA70 | ECX actual node; stack float bits/recurse DWORD; RET8. Writes +AC and optionally traverses +34/+3C. Existing raw setter is used unchanged. |
| 00B75030 | ECX model slot; stack actual name header; EAX same slot; RET4. Existing actual model construction and companions remain canonical. |
| 00B75170 | ECX model; stack unused DWORD, geometry, float178, float17C; RET10h. Publish/retain incoming geometry before releasing old geometry. The zero DWORD here is not a LOD index. |
| 00BEA150 / 00B94710 | Structured reader construction and actual mesh loading remain the earlier native-reader path. BEA150: ECX reader, RET; B94710: ECX pair, EDX unused, stack parent handle, RET4. |

These host APIs have new C++ interfaces. Evidence for the unchanged native
bodies is also recorded in `NATIVE_TRACELINE_RENDER.md`,
`GENERATED_INSTANCE_GEOMETRY.md` and `STRUCTURED_HIERARCHY_VALUES.md`.

## LOD selection and validation limits

No LOD-zero override or substitute threshold is introduced. Original
B748E0..B74B5A (ECX model; context, float LOD, visibility, flags; RET10h)
preserves supplied LOD when notification context+A0 is nonnull. When null,
B74A0F..B74AB2 computes the existing clamped screen-coverage value from the
camera view-projection, transformed sphere and context+C camera scalar+4C.
The x87 listing was inspected rather than relying on decompiler float aliases.

B72F80..B7325A (ECX mesh; context, model, LOD, visibility, flags; RET14h)
uses mesh+50 phase count and the first serialized matching inclusive min/max
range at mesh+10. Its signed section bounds are also inclusive. With zero
phases it submits all sections. The unchanged actual native render body retains
these choices, including the no-matching-phase exit. Unsupported derived
virtual targets still fail through the existing explicit service boundaries.
Recovering arbitrary original notification-context producers or derived LOD
policies requires their actual caller/owner contracts; this component invents
neither.

Validation in this worker: bounded original/source evidence inspection and
`git diff --check`. No tests were added. The primary integrator owns compilation
and runtime checks; this worker ran no build or game. Multi-asset GPU rendering,
spawn handling, OFF identity, visual parity, Original ABI and game validation
remain separate integration gates.

## Plane creator visibility defaults

`configure_plane_default_visibility(unit_key)` is an explicit creator adapter;
the primary calls it only for a true host plane, after model loading and before
the first live visibility publication. It returns false for an unknown key and
is idempotent for a configured unit. Asset names do not classify units.

The installed Note payload is read through the existing 718F50 text adapter,
which truncates at the first NUL. The adapter retains only immutable cockpit
Note/resource roles and source-item default metadata. B891A0 publishes item/node
pairs in hierarchy/resource order; 71AD50/71AAE0 return the first whole-length,
case-sensitive Note match. Thus cockpit roles use attached Note resources, not
hierarchy labels. Native visibility writes retain this original order:

| Original creator site | Selection | B6DA70 factor/recurse |
| --- | --- | --- |
| 7D73BE..7D741E | Note `belsocockpit` | 0 / 0 |
| 7D73E4..7D745D | Note `kulsocockpit` | 1 / 0 |
| 7D7661..7D76D3 | B6F9A0 descendants matching `szarny letorik` | 0 / 0 |

B6F9A0's listing starts from root+34, recurses before testing each child, uses
CRT strstr at B6FA3C, then advances through +3C. It excludes the supplied root.
B6E010 prepends children at B6E049..B6E064, so the metadata traversal visits
siblings in reverse authored creation order and descendants before their parent.
All matched actual visual/palette nodes receive the nonrecursive native setter;
no source item or actual node is added. Subsequent whole-unit visibility uses
`host_visible && immutable_item_default`, preventing hidden parts from being
re-enabled by a frame's liveness gate. No live visibility field is retained.
For a hidden source parent, B6D990 normally gates descendant render traversal.
Because the mission nodes are flattened, the immutable mask also incorporates
hidden authored ancestors, publishing zero to any additional affected actual
descendants after the creator's three ordered selections. This adds no second
native graph and no live visibility cache.

For installed D3A_Val, inner cockpit item14/Note63 is hidden, outer cockpit
item10/Note56 is shown, and broken-wing items17 then16 are hidden. Its root has
Flags1 and all remaining items Flags0. B89474's Flags bit0 selects the group
factory, while bit1 controls group bounds; neither is a serialized visibility
bit. The nine native drawable registrations remain present; three have a hidden
creator default. Notes such as `visp100-visp8` are not interpreted as hide flags.

This bounded adapter supplies the creator's visibility factors. The separate
interior-cockpit scale write, rotor-speed blur blending, damage transitions and
animation are outside it. In particular, 7EC0AF's damaged-rotor initialization
and 7EC3B0's live rotor blend are not replaced with a guessed permanent choice.

## Matrix-indexed plane bodies

`MatrixIndexedMesh` uses the same complete B94710/B944E0 actual mesh, subset,
material, texture and vertex/index-stream loader. Original parser B94900 wraps
that same mesh pair in the distinct D63780 item; it does not decode another
geometry representation. The mission adapter records the resource kind and
creates a genuine B91220/D63590 matrix-indexed model through the separately
recovered native owner/pool component. It never relabels an ordinary 188h model
or fabricates geometry, shaders or an identity palette.

B91220 retains its ECX-model/stacked-name/RET4 contract in the owner component;
see `OCT24_RENDER_D_BONE_OWNER.md` for the 194h pool, type, construction and
physical-disposal evidence. B90600 is ECX actual model, stacked index/node,
RET8; its unused EDX remains explicit. B90620 is ECX actual model, stacked
index, RET4. B90E30's C++ adapter uses the existing B1C770 pointer-array body at
the real model+184 header. These interfaces do not add node credits implicitly.

The native mesh's `WeightMapNames` order selects the model's real pointer array
at +184/+188/+18C. The adapter uses B90E30 to resize it and B90600 to publish each
matched actual node. Name matching follows B87E80's equal-byte-length then CRT
case-insensitive first match. A missing authored node fails explicitly. Already
materialized visual nodes are reused; remaining referenced nodes use the genuine
B866C0 plain-node factory and canonical NativePlainNodeReference. All reference
counts, scene bindings, pools and name services belong to the existing domains.

Each palette node receives its authored asset-relative matrix and the current
borrowed unit affine through B6DB10. This supplies authored rest pose until an
authoritative animation owner is connected. It does not claim full B87E80's
vertex-derived bound recomputation or the distinct SkinedMesh 60h record path;
SkinedMesh assets now fail explicitly rather than silently omitting that visual.

Material builder B42350/B426A8 reads B8FF00's real model+188 count, selects each
bone with B90620 and transposes its refreshed actual world+F0 into the matrix
constant register bank. This is the matrix register+30 branch, separate from
skinned register+32 and its inverse-bind/optimized storage. D63590's +20/+48/+58
are the existing B748E0/B6E8C0/6EF890 paths. The primary scene and render20 adapter
admit the genuine profile without substituting another native table.

The palette's plain nodes need no separate render submission to refresh world
matrices. Original B426DC tests each selected node's +5C bit2 and B426E4 calls
B6DB70 when stale, before the first +F0 load at B426E9. The existing native
constant builder's ensure_world performs that same per-node gate/call immediately
before its three-row transpose. B6DB10 pose publication therefore suffices;
no parallel palette matrices or eager direct +F0 writes are introduced.

`drawable_reference` now returns the common RenderCommandReference over actual
+04; each companion also implements GeneratedModelNodeLifetime. Render-group
B1D760 cleanup uses the new canonical node lookup and the same B6DFA0 unlink/
logical release, preserving existing ordinary-model providers through a default
adapter. Bone arrays borrow their nodes without retaining them. Reused ordinary
visual nodes therefore receive one real extra actual+04 reference credit per
unit; matrix-model-to-matrix-model palette dependencies fail explicitly to avoid
an unsupported retention cycle. Plain palette nodes keep their creator credits.
The last physical matrix-model disposal callback for a unit releases the extra
visual credits, then the plain creator credits through B6F310/B6F8D0. This runs
during renderer/cache retirement while original pools/runtime/name services
still admit node terminals. Missing obligations set a persistent failed state
and prohibit normal completion. `after_native_renderer_drain` releases no
palette nodes; after the full singleton shutdown it only verifies that these
earlier callbacks consumed every palette obligation.

Installed D3A_Val has 19 authored hierarchy items. Item18 `MeshHolder` references
resource0 MatrixIndexedMesh. Its ten names bind items1,2,3,4,5,7,9,10,6,8 in that
order. The ordinary cockpit visual at item10 is reused; nine referenced plain
nodes are required. Other ordinary visuals are cockpit/propeller/noseart and
broken-wing models, so a Mesh-only load omitted the main plane body.

## Baseline inventory audit

Read-only inspection of `local/render_D/origin_retry_usn04.log` and the installed
autoload VehicleClass table produced the following exact initial inventory.
That log loads `usn_19_coralus.scn`; instance labels are not class identities.

| Unit indices | Class | Current Mesh path | Mesh-only drawables per unit |
| --- | ---: | --- | ---: |
| 0 | 1 | models/ships/US/LexingtonCV.mmod | 26 |
| 9 | 2 | models/ships/us/yorktown.mmod | 49 |
| 1,2,10,11 | 294 | models/ships/us/neworleans.mmod | 29 |
| 3,4,13,14 | 364 | models/ships/us/sims.mmod | 17 |
| 5 | 378 | models/ships/us/phelps.mmod | 20 |
| 6,15,16,17 | 288 | models/ships/us/Farragut.mmod | 26 |
| 7,8 | 263 | models/ships/rn/leander.mmod | 39 |
| 12 | 380 | models/ships/us/Northampton.mmod | 35 |
| 18,19,20 | 158 | models/planes/japan/D3A_Val.MMOD | 8 plus one matrix-indexed body |

There are 21 unit instances, nine logical assets and 249 unique ordinary Mesh
resources. Existing Mesh-only materialization gives 520 drawable instances;
the three genuine matrix bodies raise that to 523. Every ordinary Mesh in these
assets has one hierarchy association. Ship resources also include Aux,
ConvexObject, GeomMesh and Note; Phelps includes GroupParams. None of these is a
separate LOD visual kind: LODPhases are fields within each native mesh. This
baseline contains no submarine row and establishes no submarine coverage.

Weapon visuals remain a separate coverage requirement. DeviceClass.Mesh is
stored in class+38 by 87CA80 and loaded into class+50 by 879590; the gun's model
is reached via [gun+360]+160. Ship model Aux platform attachments position those
device instances. For example, Sims' Turret A/B/D platform rows use DeviceClass15
`models/devices/us/fletcher_turret.mmod`, rather than a Fletcher hull inferred
from the instance label. The audited log carries 406 guns from 23 device rows,
20 with nonempty distinct Mesh paths. Base unit asset loading does not itself
create or animate those device child models. See GUN_BARREL_COUNT.md and
GUN_MOUNT_POSITIONS.md; no device runtime/visual parity is claimed here.
