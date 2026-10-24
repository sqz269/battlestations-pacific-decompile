# Native mission device visual seam, 2026-10-24

Packet `oct24-renderD-device-visual`. This packet adds a pure gunnery-host read
seam and a metadata/live-pose adapter. It loads no native model, allocates no
replacement simulation object, stores no pose, and starts no device frame.
Primary renderD owns asset/scene integration and runtime validation. Construct
the adapter only under the existing default OFF `BSP_NATIVE_MISSION_SHIP` master.

## Canonical identity and metadata

`GameGunneryHost::guns()` returns `Impl::guns` itself. Those `GameGunRow` records
contain the current angles used by firing; they are not rebuilt diagnostic
snapshots. The new pure methods read that same private vector and its existing
`DeviceFirePoints` cache:

```cpp
bool device_visual_metadata(size_t gun_index, GameDeviceVisualMetadata& out) const noexcept;
bool device_visual_node_world(size_t gun_index, size_t hierarchy_item, CameraMatrix& out) const;
```

Metadata exposes unit index, original platform key, selected device class,
borrowed cached `Mesh` string, original hierarchy item count, source-load state
and mount validity. The cache obtains Mesh through the existing
`read_device_class_string(device, "Mesh")`; its hierarchy and base/barrel
associations come from the already-loaded device asset. An empty nonnull Mesh
means an authored meshless device. A null metadata pointer means the provider
is absent, not permission to draw a placeholder.

Physical identity is `(unit_index, platform_key)`. The existing flatten chunk
walks each `Platforms[k]` once, selects `Gun[1]`, and records equipment overrides
in that same platform row. The creation loop emits one canonical row per
flattened platform. Neither `barrel_num` nor `platform_gun_count` creates another
physical visual instance here. The adapter deduplicates that pair, keeps the
first stable gun index, and requires duplicate rows to agree on selected class,
Mesh and hierarchy count. Conflicting selections fail explicitly; device class
is not appended to the key to invent another physical platform.

`GameNativeMissionDevices` assigns `0x80000000 | first_gun_index` as a sparse
render key. Ordinary unit keys must remain below that range. The native model
facade supports sparse keys through its existing map. Each binding stores only
identity/path/hierarchy-count metadata. No gun row, current angle, mount pose or
world matrix is retained in the adapter.

## Shared current-pose calculation

The original source path is `Impl::gun_barrel_muzzle_world_00730762` in
`game_hosts_gunnery.cpp`, with evidence in `MUZZLE_OFFSETS.md`,
`GUN_HORZ_SIGN.md` and `SHIP_PLATFORM_ATTACHMENT.md`. Its admitted pose comes
from the same gun's current angles, known platform mount, cached authored
Hierarchy and current `GameUnitsHost::unit_pose`.

The matrix block is factored into private pure
`Impl::device_hierarchy_node_world`. Firing and rendering call that one kernel:

1. Construct the existing translation-only platform root from `mount_local`.
2. Call the same `turning_gun_apply_angles_00859550` with exactly the existing
   `-kGunHorzSign * gun.angles.horz`, current vertical angle, and cached barrel
   association. That routine drives root yaw and barrel pitch, or the combined
   root rotation when no barrel node exists.
3. Multiply root by the supplied current unit world matrix with the existing
   `multiply_camera_matrices_00413920`, then walk the requested item's original
   parent chain, using the driven barrel local only for that actual node and
   authored local matrices for the other nodes.

The new public accessor obtains fresh unit pose arrays and returns false when
the unit/attachment/cache/item chain is unavailable. It does not use
`last_muzzle_node_world`; that is a last-shot diagnostic. Its bounds checks are
confined to the new read seam. The existing firing caller preserves its own
preconditions, barrel/base/first-node selection, muzzle-offset application,
wrapped barrel selection, output writes, and final diagnostic stores in the
same order. No firing counter, RNG draw, angle update, timer or fallback changes.
The kernel's angle and matrix operations keep their original source order;
runtime OFF identity still requires the primary's focused pair verification.

These remain the host's documented substitutions: item 0 is the model root;
authored Item matrices are interpreted as parent-relative; the platform frame
used by this firing path is translation-only; native kind-46h/44h parent
re-expression, recoil and the gun slot-94h override are not supplied here.
Rendering matches that existing admitted host pose convention. This packet
does not claim original whole-device animation or binary ABI equivalence.

Original address anchors are `0095F500` (slot association), `0072DD20` and
`0072E9A1..0072EA15` (device root and named node binding), `00859550` (angle
drive), `00B6DB70` (local/parent composition), `00413920` (matrix multiply) and
`00730762` (firing-point use). The new C++ read methods have no original binary
entry or recovered symbol. No Ghidra export, rename or mutation was performed.

## Primary integration and lifetime contract

Construct `GameNativeMissionDevices(log, units)` after the actual gunnery host
has populated its rows/cache. Its bindings include meshless platforms, which
must be skipped for model loading. Nonempty Mesh with missing cached hierarchy
or unknown attachment fails preparation with a precise provider message.
The adapter logs one census of gun rows, physical platforms, visible instances,
meshless platforms, duplicate rows and distinct Mesh strings. No count is
hard-coded or claimed measured by this source-only packet.

For each nonempty binding, primary loads that exact installed Mesh with
`load_installed_unit(mesh, render_key)`, verifies the model facade's original
hierarchy count matches the cached source, and attaches its genuine drawables.
No lookup by guessed instance, base or barrel label occurs in the adapter.

Before an actual camera/pose update, call `validate_source()`. For every original
hierarchy item that the native model/palette owns, call
`current_node_world(render_key, hierarchy_item, out)` and synchronously publish
the returned matrix through the model's genuine node setter. The model worker's
`update_hierarchy_transform(render_key, original_item, composed_world)` publishes
`00B6DB10` to matching actual visual/palette nodes; using original indices also
keeps child subtrees and matrix palettes coherent. No angle or simulation state
is copied into the model facade. A false result must remain an explicit missing
pose, never an identity or origin fallback.

The adapter borrows the same units/gunnery hosts until native visual retirement.
Source queries and simulation run on their existing single application thread.
A replaced host, changed gun census, changed row identity, equipment/Mesh change
or changed authored hierarchy requires explicit asset/owner rebinding and fails
validation. Dynamic asset replacement/spawn integration is not supplied here.
The adapter owns no native references and must be retired before its source
host disappears. The native model/scene/command graph retains its existing
owner/queue rules and sole BeginScene/Present bracket.

## Verification boundary

Only source review and `git diff --check` were run for this packet. No new tests,
full build, standalone probe or game run was performed. The exact shared-kernel
delta above is provided for primary review and focused default-OFF identity,
ON census/pose/draw and visual checks. Source implementation is not a claim of
build-tested, fixture-tested, OFF-identical, ABI-compatible or game-validated.
