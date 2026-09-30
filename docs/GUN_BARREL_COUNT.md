# Gun barrel count from the device model (packet cc9_gun_barrel_count)

Addresses: 007325A0, 00718870 (00717F20, 00718000), 0071B5C0, 0071B3E0, 0071ABB0, 0072AB80,
0072E71A. Ghidra was read only. Host files: `include/bsp/gun_fire_points.hpp`,
`src/gun_fire_points.cpp`, the `kGunBarrelCountBound` block in `src/game_hosts_gunnery.cpp`, and
two Lua-host accessors (`read_device_class_string`, `read_resource_file`).

## 1. The chain in the image

- **Model.** The weapon class's `+50h` is the model named by the device row's `Mesh`
  (`scripts/datatables/classtables/arcade/deviceclasses.lua`, `Platform("models/devices/...")`).
  docs/FLAK_PROXIMITY_BURST.md section 6.
- **Aux items.** The model's `Resource` section holds `Aux` entries. The Aux factory `0071B5C0`
  allocates a 54h-byte item through `0071ABB0` (name at +8, index +24h seeded `FFFFFFFFh`,
  category +28h, point vector +44h..+4Ch) and reads it with `0071B3E0`:
  - `Identifier`: counted string into +8 (71B451), U32 into +24h (71B458);
  - `Category`: counted string into +28h;
  - `Points`: three float reads per point, pushed by `004215D0` at 71B4F0 while the node has
    payload left;
  - any other child is skipped (`00BE9C40`).
- **Lookup.** `00718870` calls `00717F20` (any match) then `00718000`, which returns the LAST item
  whose counted name equals the key byte for byte and whose +24h equals the index
  (docs/NATIVE_GAME_RESOURCE_NAMED_GROUPS_BF.md).
- **The list, `007325A0`.** `("fire", 0)` found: its whole point list is copied into class+98h
  (00732689-007326BF). Otherwise it walks `("fire", 1)`, `("fire", 2)`, ... while the lookup
  answers, pushing each item's FIRST point (007326DA-00732788). An item with an empty point list
  reaches the CRT invalid-parameter call before `004215D0`.
- **Count.** `0072AB80` is the list size, or 1 when the begin pointer is null; `0072E71A` stores it
  into gun+448h. The fire path takes the next barrel modulo it, and each barrel has its own reload
  timer, so the count multiplies the rate a gun can sustain.

## 2. The host's reader and switch

- **Path.** `DeviceClass[dev].Mesh` from the live mission Lua state, then the mounted VFS read the
  scripts use (mode 2), then `read_mmod_aux_point_items_0071b3e0` over the recovered
  `StructuredReader`: root `MMOD`, the version control word, the root children, and inside
  `Resource` every `Aux` entry read as `0071B3E0` reads it. Other resource entries are skipped,
  not parsed; `00718870` consults only the Aux items.
- **Cache.** One load per device class (`fire_points_by_device`), at gun-row construction. Nothing
  runs per tick.
- **Switch.** `kGunBarrelCountBound`, ON: gun+448h = `gun_muzzle_count_0072ab80` of the list. OFF:
  the number of `Bullet` records in the device row, which is what the host used before.
- **Fallback.** A device with no `Mesh` string, or a model that does not open or parse, keeps the
  record count. The summary counts these guns. In USN04 nine devices have no `Mesh`, all with one
  record; section 6 lists them and why the image's count for them is not established.
- **Muzzle offsets (packet step 2).** The same read gives class+98h, the per-barrel offsets in the
  mount node's local space. The host has no slot for them: every gun on a unit fires from one
  shared origin (the unit origin raised by the class `Height`, a labelled placeholder in
  `run_gun_aim_and_fire`). To use them the projectile spawn needs:
  1. the mount node, gun+3CCh, the device model's `barrel` or `base` hierarchy node
     (`gun_mount_node_pick_0072ee65`);
  2. that node's world matrix, which needs the ship model's platform attachment transform and the
     turret's current yaw and pitch applied to the device hierarchy;
  3. then `TransformAffinePoint(offsets[gun+44Ch], node_world)` as at 007307A0/007307D3.
  The offsets are kept in the cache (`DeviceFirePoints::list`) and are not applied.

## 3. Per-device count, host against image (USN04)

From the ON probe (`local/bT_probe.log`, 200 mission frames, 406 guns built at load). "Records" is
the OFF count. Guns built later (aircraft) are not in this table; their devices print in the run's
own summary.

| device | model | guns | host records | image |
| --- | --- | ---: | ---: | ---: |
| 9 | us/northampton_turret | 3 | 1 | 3 |
| 12 | us/atlanta_turret (Atlanta 5in DP) | 6 | 2 | 2 |
| 15 | us/fletcher_turret (FletcherCombiGun) | 17 | 2 | **1** |
| 20 | japan/flak_gun_us (Flak Gun US) | 24 | 1 | 1 |
| 40 | us/oerlikon_aa | 146 | 1 | 1 (index 0, one point) |
| 41 | us/AA_28mm | 3 | 1 | **4** |
| 42 | us/bofors_aa (AA Bofors 40mm) | 24 | 1 | **2** |
| 54 | none (depth-charge launcher) | 18 | 1 | fallback 1 |
| 63 | us/us_triple_torpedo_tube | 4 | 1 | **3** (index 0, three points) |
| 65 | us/us_torpedo_tube_5 | 10 | 1 | **5** |
| 87 | none (500 kg bomb platform) | 3 | 1 | fallback 1 |
| 93 | none | 6 | 1 | fallback 1 |
| 110 | japan/japan_plane_mg | 3 | 1 | 1 |
| 217 | rn/PACK3_Belfast_1962_SecTurretA | 8 | 2 | 2 |
| 218 | rn/perth_turret | 8 | 1 | **2** |
| 258 | us/Quad_Torpedo_launcher | 8 | 1 | **4** |
| 264 | us/bofors_aa_quadruple | 25 | 1 | **4** |
| 266 | us/Browning_aa | 8 | 1 | 1 |
| 269 | us/deck_gun_cv | 44 | 2 | **1** |
| 286 | us/oerlikon_2 | 8 | 1 | **2** |
| 342 | rn/Hood_AATurretA | 10 | 1 | **4** |
| 347 | us/neworleans_turret | 12 | 1 | **3** |
| 348 | us/Farragut_MainTurretA | 8 | 2 | **1** |

184 of the 379 model-read guns change. The offline scan's three rows hold (Atlanta 2, Flak Gun US
1, Bofors 2). The record count was never a barrel count: it is the number of ammunition types, so
every dual-purpose mount with two records (Fletcher, deck gun, Farragut) fired at twice its rate,
and every multi-barrel single-ammunition mount fired at a fraction of it.

## 4. Predictions (written before the runs)

Same tree, `BSP_GUNNERY_RNG_STREAMS=1` on both sides, switch OFF (`local\bC`) against ON
(`local\bT`). Base figures are the last flak-burst treatment (`fT_4500` / `fT_9000`), per device,
through `local/devtable.py`; the OFF side is expected near them but main has moved since.

Mechanism: a gun's shots in an engagement window grow with the barrels ready in it, because each
barrel has its own reload timer and only `BarrelDelayTime` separates them. Windows here are short
against the reload (Fletcher DP about 3 shots per gun over 225 s), so shots should scale close to
image/records, less where `BarrelDelayTime` or target availability binds.

USN04 4500 (OFF to ON):

- **Category 6** (Atlanta flat, Fletcher DP x0.5, deck gun x0.5, Farragut x0.5, Belfast flat):
  shots down about a third, 312 to 190-230. Hits down about a third, 199 to 120-150.
- **Category 5** (Flak Gun US only): flat within 10 %, apart from RNG coupling through the changed
  target deaths.
- **Category 1** (twin Bofors x2, quad Bofors x4, Hood AA x4, 28 mm x4, Oerlikon flat): shots up,
  746 to 1300-2000. Hits up, 26 to 35-60, most from the twin Bofors.
- **Category 0** (plane guns): flat if the aircraft gun devices read 1, which the run's device
  lines will show.
- **Torpedo tubes** (63, 65, 258: 3-5 barrels): no effect in USN04, where the ship torpedo gate
  sends nothing (`torpedo_gate ... shots=0`).
- **Deaths** 19 within 17-21. Kate and Val death times move by seconds as the kill mix shifts
  from DP rounds to Bofors rounds, RNG-coupled. Torpedo drops 4 within 3-5. Lexington flat (not
  sunk, no hull damage beyond what the OFF side takes).

E2 9000: the same ratios on the doubled window. Category 6 shots 564 to 350-420, hits 371 to
230-290; category 1 shots up by 1.7-2.5x, hits 48 to 60-100; category 5 flat within 10 %;
deaths 37 within 33-40; drops 8 within 6-10; Lexington flat.

## 5. Pair results

Same tree at the packet commit's parent plus this change, `BSP_GUNNERY_RNG_STREAMS=1` on both
sides. OFF is `local\bC`, ON is `local\bT`. Logs: `local/bC_4500.log`, `local/bT_4500.log`,
`local/bC_9000.log`, `local/bT_9000.log`. Per device through `local/devtable.py`, deaths through
`local/deathdiff.py`.

The OFF side has moved from the `fT` base the predictions quoted: at 4500 it has 22 deaths and one
torpedo drop, not 19 and 4. So the predictions are judged as ratios, OFF to ON.

**USN04 4500, OFF to ON:**

| category | shots | hits | prediction | held? |
| --- | --- | --- | --- | --- |
| 6 | 306 to 214 (-30 %) | 167 to 144 (-14 %) | shots and hits down about a third | shots yes, hits no |
| 5 | 100 to 104 | 57 to 62 | flat within 10 % | yes |
| 1 | 1159 to 2170 (x1.87) | 86 to 94 (+9 %) | shots x1.7-2.7, hits x1.35-2.3 | shots yes, hits no |
| 0 | 1124 to 1124 | 18 to 18 | flat | yes |

- **Deaths** 22 to 21 (predicted within the OFF value +-2: held). One flip: Kate #8.1|.-3 was
  killed by Northampton-class03 at 221.6 s and now survives the window.
- **Death times.** Vals die later in 10 of 11 pairs, mean +2.7 s, range -0.4 to +5.4 s. Kates
  die later in 7 of 8, mean +0.6 s, range 0 to +3.7 s. Predicted "move by seconds": held, and the
  sign is now known: later, which fits the DP mounts that halved, but no kill was traced.
- **Torpedo drops** 1 to 1. **Lexington** takes 0 damage on both sides and is not sunk: held.

**E2 9000, OFF to ON:**

| category | shots | hits | prediction | held? |
| --- | --- | --- | --- | --- |
| 6 | 541 to 372 (-31 %) | 326 to 247 (-24 %) | shots 350-420 of 564, hits 230-290 | yes, both |
| 5 | 162 to 168 | 97 to 93 | flat within 10 % | yes |
| 1 | 1844 to 3247 (x1.76) | 104 to 134 (+29 %) | shots x1.7-2.5, hits x1.25-2.1 | yes, both |
| 0 | 1124 to 1124 | 18 to 18 | flat | yes |

- **Deaths** 37 to 37, no flips. Vals later in 17 of 19, mean +2.8 s; Kates later in 13 of 16,
  mean +2.7 s, one at +22 s. **Drops** 3 to 4. **Lexington** 0 damage on both sides. No mission
  end on either side.
- `total_damage` is 8260.0 on both sides although hits fell 545 to 492. The per-unit damage is
  capped at the health a victim has left, and the same 37 units die with the same health.

**Per device, 4500 (shots OFF to ON, hits OFF to ON):**

| device | count | shots | hits |
| --- | --- | --- | --- |
| 15 Fletcher DP | 2 to 1 | 55 to 33 | 25 to 16 |
| 269 deck gun CV | 2 to 1 | 139 to 74 | 71 to 59 |
| 348 Farragut | 2 to 1 | 32 to 16 | 22 to 21 |
| 12 Atlanta DP | 2 to 2 | 57 to 70 | 32 to 33 |
| 42 twin Bofors | 1 to 2 | 511 to 843 | 12 to 13 |
| 264 quad Bofors | 1 to 4 | 333 to 782 | 9 to 10 |
| 342 Hood AA | 1 to 4 | 8 to 43 | 2 to 2 |
| 40 Oerlikon | 1 to 1 | 188 to 326 | 5 to 8 |

- **Where the predictions failed.** The halved DP mounts lost about half their shots but far
  fewer hits, so their hits per round rose (deck gun 0.51 to 0.80, Fletcher 0.45 to 0.48). The
  doubled and quadrupled Bofors fired 1.6 to 2.3 times as often and hit almost no more (about
  1-3 % of rounds on both sides). Neither cause is isolated in this packet; the prediction assumed
  hits scale with rounds, and for these mounts they do not. The Oerlikon's count did not change,
  yet it fired 73 % more, consistent with targets living longer.
- **The AI weight.** `00A08460`'s barrel damage takes the same count as its shots argument
  (00A09501), and the host's weapon-facts publisher reads `barrel_num`. It moves with this switch,
  as it does in the image.

## 6. Decision and what remains

- **`kGunBarrelCountBound` lands ON.** The count is the image's for every model-read device. The
  measured consequence is a quieter DP battery, a busier light-AA battery and aircraft that die
  2-3 s later on average. Deaths move 22 to 21 and 37 to 37, drops 1 to 1 and 3 to 4, and the
  Lexington takes no damage on either side.
- **Aircraft-mounted devices are not established** (closed in section 7.1: the image's count is 1). Nine devices have no `Mesh` in their row:
  54 (the Fletcher depth-charge launcher) and eight built only on aircraft: 80 and 95 (the US
  squadrons), 85 (B5N), 87 (D3A 500 kg bomb platform), 89, 98 and 101 (A6M), 93 (B5N and D3A).
  They build 315 guns in USN04, all on one record, and the host keeps 1. `007325A0` is reached
  only through seven vtable slots (00CE4554, 00CE4580, 00CE45BC, 00CE4600, 00CE4634, 00CE4668,
  00CE469C), and its first call is the base binder `00879AD0`. With
  a null `[class+50h]`, `00718870` would read `[0+64h]`, so the image must either skip the binder
  for these classes or give them a model some other way. That path is unread. It does not move
  USN04, where the aircraft gun category matched exactly on both sides.
- **Muzzle offsets.** Read and cached, not applied: section 2 lists what the projectile spawn needs.
- **Torpedo tubes** read 3 to 5 barrels (devices 63, 65, 258). No ship torpedo fires in USN04, so
  the first mission with a ship torpedo attack is where this count shows.

## 7. Read-only follow-up (packet `cc9_rebaseline_3`, 2026-09-23)

### 7.1 The nine Mesh-less devices: the image's count is 1, which the host already uses

- **The seven slots are all vtable offset +20h.** `007325A0` sits 20h into the gun-family vtables
  that start at these addresses:
  - `00CE4534`, written by `BSP_GunDescriptor_ConstructCommon` at `00442BB3` and by `00732F6F`;
  - `00CE4560`, written by `BSP_ConstructBombPlatformClass` at `00442C58`;
  - `00CE459C`, written by `BSP_ConstructMultibombPlatformClass` at `00442CE8`;
  - `00CE4648`, written by `BSP_ConstructSingleTurningGunClass` at `00442EC8`;
  - `00CE467C`, written by `BSP_ConstructDepthChargeLauncherClass` at `00442F58`;
  - two more, near `00CE45E0` and `00CE4614`. The only reference found for these is the table read
    at `0044333D` in `BSP_DeviceClass_ResolveFromLua`, so their constructors are not named here.
  - So bomb platforms, multibomb platforms and the depth-charge launcher run the same bind as guns.
- **The slot runs only when a model loaded.** `00879AA0` (BSP_DamageableClass_ActivateModelResource)
  calls `00879590` and then tests `[class+50h]` at `00879AAD`. It calls `vtable[20h]` at
  `00879AB5`-`00879ABA` only when that pointer is non-null.
- **No `Mesh` means no model.** `0087CA80` stores the row's `Mesh` string into class+38h
  (`0087CB49`-`0087CB8F`). `00879590` returns at once when class+38h is null (`008795AB`-`008795B4`)
  or class+50h is already set (`008795BA`). So a row without `Mesh` never loads a model, and
  `007325A0` never runs for it.
- **The count.** class+98h stays empty, so `0072AB80` takes its null-begin branch (`0072ABA5`) and
  answers 1. All nine devices get 1 in the image, and the host's fallback of 1 matches for every
  one of them. Section 6's open item is closed.
- **The `_enemy` model variant.** `00879590` also tries a variant with `_enemy` (`00D0DF08`)
  inserted at the first dot, when its stacked flag is set and the name resolves. No
  `models/devices` file in this installation has `_enemy` in its name, so the plain `Mesh` path the
  host reads is the model the image loads.

### 7.2 Applying the cached muzzle offsets: what the projectile spawn needs

What the host already has, per gun:
- the gun row: unit, `platform_key`, `platform_name`, the device class, the firing arcs, the rest
  angles, the aim angles, `barrel_num` and `next_fire_barrel` (gun+44Ch);
- the unit pose: right, up, forward and origin (`unit_pose`);
- the offsets: `DeviceFirePoints::list.offsets`, class+98h, one per barrel in the mount node's
  local space;
- the rule: `gun_muzzle_world_position_00730762` and `gun_advance_barrel_index_007309ed` in
  `include/bsp/gun_mount_positions.hpp`.

What is missing, in dependency order:
1. **The device model's node table.** The same `.mmod` holds a `Hierarchy` of `Item` nodes
   (Parent, Name, Matrix). `parse_hierarchy_item_00b7eb90` already reads one item. The gun reader
   would keep the items beside the offsets, in the same one-load-per-device cache.
2. **The mount node pick.** Pick `"barrel"`, then `"base"`, then the model's first node, as
   `0072E9E2`-`0072EE8F` do through `0071AD50`. `0071AD50` is unread, and a name match on the
   parsed items is its stand-in.
3. **Where the device model sits on the ship.** This is `[gun+360h]+160h`, the device instance's
   model in the ship's scene. The ship-side attachment is **not in the Lua row**: a `Platforms[k]`
   entry has `Name`, `Gun`, `Windows`, `RestAngles` and `MainPlatform`, and no position. It must
   come from the ship model's hierarchy, keyed by a platform node, when the device instance is
   created and parented. That creation and parenting is unread. It is the one hard dependency,
   and it needs its own read of the unit-instance device attachment.
4. **The turret's current pose.** The node's world matrix is ship world x platform attachment x
   the device's local node chain, with the yaw and elevation drives applied to `base` and
   `barrel`. Whether `barrel` carries the elevation is open (docs/GUN_MOUNT_POSITIONS.md section
   10). The `[gun+3Ch]` slot 8Ch/94h override (recoil or animation) is optional.
5. **The spawn.** `origin = TransformAffinePoint(offsets[gun+44Ch], node_world)`, then advance
   gun+44Ch modulo gun+448h. In `run_gun_aim_and_fire` this replaces
   `muzzle = origin + hull_height`, and the same point must feed the ballistic arc's
   `h = aim.y - muzzle.y`.

Until item 3 is read, the offsets can only be applied relative to the unit origin, which would
be a placeholder of its own. So nothing is wired.

## 8. The fallbacks by cause (packet `cc9_gun_fallbacks`, cc9-gunnery14)

Ranking #10 and #11 (lua19's refreshed ranking).

### 8.1 Barrel count

**Which devices take the fallback.** In reference p's USN04, JM05, USN13 and USNOS logs, every
device that takes it logs `barrel count not read from the model: no Mesh string`. Those devices
are listed below by `local\g14_devices.py`, from this installation's arcade `deviceclasses.lua`
(mtime 2026-05-09, modded; `gamemode.lua` selects the arcade table):
- depth-charge launchers 54, 55, 57 and 154;
- torpedo catapults 61 and 66;
- bomb platforms 75, 78..80, 85..89, 92 and 122;
- plane guns 93, 95, 96, 98 and 101.

**The image's count is 1** (7.1): no `Mesh` means no model, so 007325A0 never runs and 0072AB80
answers 1. The host kept the platform's `barrels`, which is also 1 wherever section 6 looked.

**`kGunBarrelMeshlessOneBound` (committed OFF).** ON sets 1 for a `Mesh`-less device. The new summary
line `summary mission gunnery fallbacks ...` counts, on both sides:
- the `Mesh`-less guns;
- the ones whose platform `barrels` is not 1, by device.

**Prediction:** the switch moves a row only where `meshless_changed > 0`.

### 8.2 Muzzle offsets

The same line splits `muzzle offsets fallbacks` by cause, with shots by device:
- **`no_mount`:** no platform attachment. The shot starts at the unit origin raised by `Height`,
  the labelled placeholder.
- **`meshless`:** the image's own empty-list fallback 00730899 (the root translation, the mount
  point). The host matches it already.
- **`other`:** a loaded model without offsets or nodes.

Nothing is bound for #11 until the census shows a `no_mount` or `other` share.

## 9. The `no_mount` muzzle fallback is the land classes' mount (packet `cc9_muzzle_no_mount`, cc9-gunnery15)

Ranking #11. In reference q, `no_mount` carries shots on four rows:

| row | shots | device classes (arcade `deviceclasses.lua`) |
| --- | ---: | --- |
| JM08 | 1000 | 52 (7.7 mm machine gun) |
| JM05 | 98 | 37 (Jap deck gun) 40, 52 (7.7 mm machine gun) 10, 84 (ghost casemate gun) 48 |
| USN12 | 83 | 37 |
| USN01 | 5 | 117 |

- The other twelve rows have 0.
- JM05 long has 1311.

### 9.1 Who has no mount

**The census.** An env-gated census line (`BSP_MOUNT_CENSUS=1`, `gunnery: mount census ...`)
shows that every JM05 gun without a mount belongs to a unit that is neither a ship nor a plane:
- coastal guns (types 89, 460, 645);
- concrete bunkers (403);
- fortress elements and fortresses (79);
- heavy and light AA emplacements (461, 462, 629);
- command buildings (6, 408).

**Why they have no mount.** The host attached a platform frame only for kinds 06h (ship) and 0Fh
(plane).

### 9.2 The image

- **Every vehicle class runs the slot pass.** `BSP_VehicleClass_BindModelData` 0095F500 is the
  class vtable's slot `+20h` entry in six vtables, found by scanning absolute dwords for
  `00 F5 95 00`:
  - `00CFF7B0`: LandFort, vtable `00CFF790`;
  - `00D1A558`: CommandBuilding, `00D1A538`;
  - `00D1A9C0`: AirField, `00D1A9A0`;
  - `00D1A9FC`: Shipyard, `00D1A9DC`;
  - `00D1AA78`: DummyTarget, `00D1AA58`;
  - `00CFF7EC`: the base vtable.
  - LandVehicle's slot `+20h` (`00D1AA38`) is `0074DA10`, which calls 0095F500 at `0074DA2E`.
  - Ships (`0082FE51`) and planes (`007D3E81`) call it from their own binders.
  - So a fort's gun platforms get their `("slot", key)` frames exactly as a ship's do.
- **A platform without a slot group keeps the identity frame.** The platform constructor 007F7110
  (called from `BSP_VehicleClass_ReadLuaFields` 00960230):
  - builds a 4x4 with 1.0 (`00D7A24C`) on the diagonal;
  - copies it to `platform+4Ch` (`lea ecx,[esi+4Ch]` at 007F7193, `call 004134F0` at 007F7201).
- **The gun takes that frame.** `0072DD20` copies `platform+4Ch` for every gun, whatever the
  unit's kind.
  - The kind 46h / 44h re-expression applies only to a gun whose parent is a Shipyard.
  - So a slotless land gun is mounted at the unit origin, not raised by `Height`.

### 9.3 The binding (`kLandPlatformAttachmentBound`, committed OFF)

- **Where:** the gun setup in `src/game_hosts_gunnery.cpp`.
- **What:** for a unit that is neither ship nor plane, the class model's `("slot", key)` frame
  origin if it has one, else the identity origin (0, 0, 0).
- **Counted on both sides:** `summary mission gunnery land mounts slot=.. identity=..`.
- **Logged:** one `gunnery: land mount` line per class, with the frame's forward.
- **Simplification (the ships' label):** only the origin is carried. A land slot whose frame is
  rotated has its rotation dropped. The logged forward shows whether any is.
- **OFF:** the placeholder, the unit origin raised by the class `Height`.

**Predictions:**
- ON moves JM05, JM08, USN12 and USN01 (exit 3): the ground guns' shots start from their mounts,
  which changes their flight and hits.
- The other twelve rows are gameplay-identical.
- `no_mount` goes to 0 on every row, and `muzzle offsets` counts the shots instead.
- Whether a mission's ground guns hit more or less is not predicted; that depends on each model's
  slot heights.

### 9.4 The pair: the mechanism held, flip ON

**Setup.**
- Exports of `8c7511688`: `local\g15_lmoff`, no flip, SHA-256 prefix `4404FE995B01`; and
  `local\g15_lmon`, which flips `kLandPlatformAttachmentBound`, prefix `DCA5D507ABB8`.
- q's sixteen rows plus JM05 long, in the reference launch form.
- A 300-frame smoke passed first, at about 23:47 UTC. An earlier attempt at 23:29 UTC failed at
  renderer init with CreateDevice 0x8876086A, after the session changed to rdp-tcp#1.
- The runs ended by about 23:59 UTC (log mtimes).

**pair_diff.**
- Exit 3 (moved): JM05, JM05 long, JM08, USN12 and USN01. This is exactly the predicted set.
- Exit 1 (gameplay-identical): the other twelve rows.

**Mechanism.**
- `no_mount` goes to 0 on every row: JM05 98 -> 0, JM05 long 1311 -> 0, JM08 1000 -> 0,
  USN12 83 -> 0 and USN01 5 -> 0.
- `land mounts` reports `identity=0` on every row: every land gun's class model carries its
  `("slot", key)` group. The slot counts are JM05 67, JM08 17, USN12 4 and USN01 8.
- **Every logged land frame is a pure translation:** forward is (0, 0, +z). So carrying only the
  origin loses nothing on these rows.
- Slot heights range from 0.47 m (concrete bunkers, 3 m aft of the origin) and 1.71 m (coastal
  guns) up to 15.66 m (a command building's upper guns).

**Effects** (death tables, per entity):

| row | effect |
| --- | --- |
| JM05 | `Mogami-class 01` sinks, only ON. Hit records 39 -> 52, damage 2130.8 -> 2587.0. |
| JM05 long | Three more sinkings, only ON: `Fubuki-class 01`, `Kuma-class 01` and `Japan Troop Transport 05`. Deaths 28 -> 31, hull hits 772 -> 951. |
| JM08 | Same victims. Nine rows change time or killer; for example, `Japanese Patrolboat 01` now dies at 27.15 s instead of 81.70 s, to Medium Bunker 03 instead of 05. Shots 2867 -> 2464. |
| USN12 | Hit records 32 -> 36, first hit 8.30 -> 7.85 s. No death row. |
| USN01 | Shots 1263 -> 1277; the death table is identical. |

**Verdict: flip ON.**
- The mechanism matches the image, the moved set is the predicted one, and the controls are
  identical.
- The size of the change is a measured consequence, not a prediction: the US shore batteries on
  JM05 now sink Japanese ships.
- JM05 and JM08 now move against reference q, as this pair records.
