# Where a ship's gun sits, and the AA line-of-fire test (packet cc9_ship_platform_attachment)

Addresses: 0095F500 (slot pass), 00961B69, 0072DD20, 0072E6D0 (0072E98A-0072E9AA), 0071AD50,
00803330, 00729560, 00729670, 0072F6E0, 0072CDD0, 0098B130. Ghidra was read only. New ledger
names: `BSP_Gun_PlatformMountFrame` (0072DD20), `BSP_SpatialIndex_NearestUnitOnSegment`
(0098B130), `BSP_MultipleBombPlatform_SetupFromDescriptor` (00803330). The record for 0072CDD0
is corrected (section 3).

## 1. The attachment, from the listing

1. **The ship model carries the platform frames.** A ship model's `Resource` section holds
   `Aux` point groups named `slot`. `models/ships/us/yorktown.mmod` has 55 of them: slots 1-19
   at deck level (y 11.2 to 17.4) and slots 20-55 at y -52 to -63. The class has 19 gun platforms,
   so slots 1-19 are theirs. The hierarchy has no platform nodes; the `Item` names are hull,
   bow, stern, propeller and collision parts only.
2. **`0095F500` builds one frame per platform** (BSP_VehicleClass_BindModelData, the slot pass
   0095FA33-0095FEA9).
   - The platform vector at class+94h is indexed by the Lua `Platforms` key (00961B69), and the
     pass looks up `("slot", key)` through 00718000.
   - It skips a group with fewer than three points. Otherwise:
     - translation = p0 (0095FBF3-0095FC13);
     - row 2 = p2 - p0 (0095FC67-0095FC98);
     - row 1 = (p2 - p0) x (p1 - p0) (0095FD4E-0095FDD8).
   - Then it orthonormalises with 0085DC80 and copies the 16 words to platform+4Ch.
   - Yorktown's slots all have p1 - p0 = (+3.4, 0, 0) and p2 - p0 = (0, 0, +3.5). So row 2 is
     +z (the bow), row 1 is +y (up), and the frame is a pure translation in the ship model.
3. **The gun takes that frame at setup.** `BSP_Gun_SetupFromDescriptor` calls `0072DD20`
   (BSP_Gun_PlatformMountFrame) at 0072E99C.
   - 0072DD20 finds the platform as `[[[gun+3F0h]+538h]+94h][gun+38Ch]` and copies platform+4Ch
     with REP MOVSD (0072DD59-0072DD6A).
   - It re-expresses the frame through the parent's world only when the gun's parent answers
     kind 46h and its grandparent kind 44h. That chain is not a ship's.
   - The setup hands the frame to `[gun+3BCh]->vtable[38h]` (0072E9A1-0072E9AA). gun+3BCh is
     `[[gun+360h]+160h]+0Ch`, the device model's root node (0072E98A).
   - The bomb-rack class (00803330, BSP_MultipleBombPlatform_SetupFromDescriptor) copies the
     same frame into the gun entity's own local matrix gun+74h (0080336B-00803374). It then
     re-aims row 2 at the point (0, 0, ForwardAim).
4. **The mount node.** gun+3CCh is the first of the device model's `barrel` node, its `base`
   node, and the root (0072EE65-0072EE8F, through 0071AD50). Its world matrix is node+F0h,
   composed under the root frame above.
5. **gun+FCh.** The line-of-fire test and `CanFire`'s "muzzle below y = 1.0" rule read gun+FCh.
   That is the gun entity's world translation (00427EB0 refreshes it through 00414DB0). So it is
   the mount's world position, not the ship's centre.
   - **Not read:** the store that puts a turning gun's entity at its platform. The setup writes
     no gun+74h for turning guns. The value is taken as the platform frame's origin carried by
     the ship pose, on the evidence of the bomb-rack class and the device model root.

**What the host now builds** (`kShipPlatformAttachmentBound`):
- **Source.** `VehicleClass[type].Mesh` (new Lua-host accessor `read_vehicle_class_string`),
  read once per class through the mounted VFS and parsed by `read_mmod_aux_point_items_0071b3e0`.
  The frame comes from `gun_platform_slot_frame_0095f500`.
- **Each tick.** `mount_world = ship_origin + right * p0.x + up * p0.y + forward * p0.z`. This
  replaces "unit origin raised by the class Height" wherever the gun's firing point is used:
  the aim, the ballistic arc, the intercept, the shot spawn and the AA acceptance's firing
  direction.
- **Labelled:**
  - **Axes.** Model +x is starboard, +y up, +z the bow. The firing arcs support it: platforms
    11, 13 and 18 at x of about -12 to -15 have port arcs (+10 to +170), and 10, 12 and 19 at
    x = +12 to +15 have starboard arcs (-170 to -10). The island group 14-17 (x of about +13,
    arcs 0 to 180) does not fit and is unexplained.
  - **Not applied:** the device model's base and barrel node chain, the turret's yaw and
    elevation, and the per-barrel muzzle offsets.
  - **Planes** are not covered: plane classes also get slot frames from 0095F500, but only
    ship guns are placed.

## 2. The mount check (Yorktown, from the model)

| platform | device | host OFF (ship frame) | host ON = slot p0 (ship frame) |
| --- | --- | --- | --- |
| 9, AA 1 | 41, 28 mm quad | (0, 16, 0) | (-0.9, 11.2, 122.0), bow centreline |
| 10 / 11, AA 1 / 2 | 42, twin Bofors | (0, 16, 0) | (12.2, 12.8, 114.1) / (-12.4, 12.8, 114.0) |
| 12 / 13 | 42 | (0, 16, 0) | (13.9, 12.8, 101.1) / (-13.9, 12.8, 101.0) |
| 14 / 15 | 42 | (0, 16, 0) | (13.1, 15.5, 45.9) / (13.5, 17.4, 40.4), island |
| 16 / 17 | 42 | (0, 16, 0) | (13.5, 15.4, -21.4) / (14.1, 13.1, -43.8), island |
| 18 / 19 | 42 | (0, 16, 0) | (-15.0, 13.7, -108.4) / (14.9, 12.9, -114.5) |
| 1-8 | 20, Flak Gun US | (0, 16, 0) | x ±12.8 to ±13.8, y 12.3-12.5, z 79.0-87.4 (bow pairs) and -93.6 to -101.5 (stern pairs) |

The OFF point is `Height` = 16 above the unit origin for every gun. ON, the battery spreads over
236 m of the ship's 250 m `Length` and 30 m of beam, at the model's deck heights. The host logs
the first unit of each class (`gunnery: mount ...`) so a run checks these numbers.

## 3. The line-of-fire test, corrected, and its binding

**Correction** to docs/E2_ZERO_ORDNANCE.md section 4 and to the 0072CDD0 record: the predicate
does **not** test the firer's own hull. 0098B130 (BSP_SpatialIndex_NearestUnitOnSegment):
- walks a 150 x 150 spatial grid for kind-5 objects;
- **excludes** both the firer's and the target's collision objects;
- tests each remaining AABB, then 0085CDB0, and returns the nearest hit.

0072CDD0 answers "blocked" when that nearest unit's +54h, its **side**, equals the firer's, or
when the static-geometry query (flags 44h) hits. So an AA gun does not fire through a friendly
unit: a ship of its own fleet, or a friendly aircraft. 0072F6E0 caches the answer per target
for the gun's life. Its third field, U(0.8, 1.2) × [GlobalConfig+8Ch], has no reader found.

**Binding** (`kAaLineOfFireBound`):
- For weapon kinds 1, 5 and 6, after the fire window, the gun's firing point raised 5 m is joined
  to the target raised 5 m (at least y = 5).
- Every live unit other than the two is tested as its oriented hull box (the class `Width`,
  `Height` and `Length`, the same box the host's projectile trace uses).
- A friendly nearest hit refuses the target, and the answer is cached per (gun, target).
- **Labelled:** the host's hull box stands for the image's AABB plus 0085CDB0. The static half is
  not modelled: at sea nothing static stands between ships.
- The test is independent of the attachment. With the attachment off it runs from the unit
  origin raised by `Height`.

## 4. Predictions (written before any run)

Pairs: E2 9000 and USN04 4500, same tree, `BSP_GUNNERY_RNG_STREAMS=1`. OFF is both switches off
(`local\pN`); ON is both on (`local\pT2`).

- **Mount positions:** the logged Yorktown mounts equal section 2's slot points.
- **Refusal share.** The fleet steams in company. Fletchers screen the carriers at 500-1500 m,
  and friendly fighters mix with the Kates. So 5-20 % of AA (gun, target) decisions are
  refused, mostly on carriers and cruisers firing past their escorts at low-flying Kates.
- **E2:**
  - Kate deaths fall by 0-3 of 16, and the Kates that die do so 0-150 m closer to their target.
  - Torpedo drops go from 0 to 0-3 and bomb drops from 0 to 0-2.
  - The Lexington is undamaged unless a Kate now releases on it.
- **USN04 4500:** category 1 hits fall 5-20 %. Category 6 hits fall 0-15 %; the dual-purpose
  and flak guns fire higher and farther, past fewer screens.
- **Attachment alone:** hit counts within ±10 %, because the zero-error aim is computed from the
  same point the round leaves.

## 5. Status, 2026-09-23

**No pair could run.** A 120-frame probe of the ON binary failed at device creation
(`device_hr=0x80004005`), and `query session` shows session 1 as `Disc`. Following the
renderer-init rule, nothing more was launched.

**Both switches are committed OFF.** The binaries for the pairs are built: `local\pN` (both off)
and `local\pT2` (both on). They land by the pairs once the session is active.

## 6. The attachment pair (2026-09-23)

Binaries are rebuilt on the merged tree: `local\qN` has both switches OFF and `local\qA` has
the attachment ON. The option is on for both. Logs: `local/qN_9000.log`, `local/qA_9000.log`,
`local/qN_4500.log`, `local/qA_4500.log`.

- **The mount check held.** The logged Yorktown mounts equal section 2's slot points to the
  centimetre, for example platform 9 at (-0.88, 11.21, 122.00). All 450 ship guns in E2 found
  their slot, and none was missing.

| row | OFF | ON | prediction | held? |
| --- | --- | --- | --- | --- |
| E2 torpedo / bomb drops | 0 / 0 | 0 / 0 | unchanged | yes |
| E2 deaths | 35 | 35 | ±2 | yes |
| E2 category 1 shots / hits | 2789 / 119 | 2640 / 93 | hits ±10 % | **no**: -22 % |
| E2 category 5 shots / hits | 96 / 73 | 125 / 115 | ±10 % | **no**: +58 % |
| E2 category 6 shots / hits | 237 / 213 | 275 / 205 | ±10 % | yes (-4 %) |
| E2 Kate nearest-ship median (min) | 693 m (485) | 687 m (368) | - | - |
| USN04 4500 deaths | 27 | 27 | ±2 | yes |
| USN04 4500 category 1 hits | 91 | 87 | ±10 % | yes (-4 %) |
| USN04 4500 category 5 hits | 55 | 67 | ±10 % | **no**: +22 % |
| USN04 4500 category 6 hits | 165 | 138 | ±10 % | **no**: -16 % |
| category 0 (aircraft guns) | 729 / 64 | 729 / 64 | flat | yes |

- **Why the hits move although the aim uses the same point the round leaves.**
  - The mounts are now up to 125 m fore and aft of the ship's centre. A gun at the far end is up
    to 250 m farther from a target than one at the near end.
  - The range gate still tests from the unit's aim point, so an engaged gun can be at the edge
    of its real reach. Flight time and the flak fuse distance then differ per mount.
  - Deaths, drops and the Kate outcome do not move.
- **Decision: `kShipPlatformAttachmentBound` lands ON.** The mount positions are the image's
  platform frames, and the pair moves no outcome row. The hit moves are the consequence of
  firing from the right place.

## 7. The line-of-fire pair (2026-09-23)

**First run: vacuous.** On the class hull box the test blocked nothing: 0 of 1083 decisions in
E2 and 0 of 866 in USN04 4500 (`local/qT_*`). The box is `Height` tall, centred on the
waterline, so its top stands 5-8 m up. A segment from a deck mount raised 5 m to an aircraft
never comes down that low.

**The fix.** The image tests each unit's AABB, which covers its whole model. The ship models'
own `BoundingBox` chunks run much higher:
- Yorktown -9.1 to 45.3 m;
- Fletcher -4.3 to 29.4 m;
- Northampton -6.6 to 45.9 m;
- Lexington -10.2 to 59.3 m.

The test now uses the unit's model `BoundingBox`, read in the same pass as the slots and
oriented by the unit's pose (`read_mmod_bounding_box`). It falls back to the hull box when a
class model does not read. **Labelled:** an oriented model box stands for the image's world
AABB followed by the unread `0085CDB0`.

Pair on the rebuilt tree: `local\rA` (attachment ON, line of fire OFF) against `local\rT`
(both ON), option on. `rA_9000` equals `qA_9000` in every count, so the rebuild changed nothing
else.

| row | line of fire OFF | ON | prediction | held? |
| --- | --- | --- | --- | --- |
| E2 decisions / blocked | - | 1087 / 61 (5.6 %) | 5-20 % | yes |
| E2 acceptance refusals | 0 | 115 | - | - |
| E2 torpedo / bomb drops | 0 / 0 | 0 / 0 | 0-3 / 0-2 | yes |
| E2 deaths | 35 | 35 | Kate deaths down 0-3 | yes (0) |
| E2 category 1 shots / hits | 2640 / 93 | 2610 / 89 | - | - |
| E2 category 6 shots / hits | 275 / 205 | 254 / 194 | - | - |
| E2 category 5 hits | 115 | 113 | - | - |
| USN04 4500 decisions / blocked | - | 870 / 61 (7.0 %) | 5-20 % | yes |
| USN04 4500 category 1 hits | 87 | 83 (-4.6 %) | -5 to -20 % | at the edge, just short |
| USN04 4500 category 6 hits | 138 | 126 (-8.7 %) | 0 to -15 % | yes |
| USN04 4500 deaths | 27 | 27 | - | - |

- **What it refuses.** About one (gun, target) pair in 16 to 18 is blocked by a friendly unit on
  the line, and the decision holds for the gun's life. Dual-purpose and light-AA fire falls by
  5-9 %.
- **What it does not change.** No outcome moves: deaths, drops and the Kate death ranges are
  unchanged. So this substitution was not what made the fleet's air defence total. The zero
  ordnance stands with the image's line-of-fire rule in place.
- **Decision: `kAaLineOfFireBound` lands ON**, on the model boxes.
`rA_4500` also equals `qA_4500` in every count.
