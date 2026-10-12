# JM08: AA truck 05's direct hits on its own Headquarter (cc13-hqdmg1, 2026-10-12)

Read-only investigation for cc9. No game was run. Ghidra was only read. The log excerpts below are
copied from cc9's trees, which can disappear. The one source change is the OFF-switch diff in §5;
it was applied, built and reverted in this tree, and it is **not committed**.

**Verdict: host defect.** The host tests a shell against a land structure using the class box
(Width x Height x Length, centred on the origin). The image tests it against the structure's model
`GeomMesh`. For JM08's Headquarter 01, that box face lies 2.6 m from AA truck 05's muzzle, while the
mesh is 27.5 m away. The 960-985 s bursts in the log fit only two headings, and from each of them a
round enters the box and **misses the mesh**. Friendly fire on a building is still image behaviour
when the mesh is in the line of fire: the image has no side test and no arming time. The 10-06 run
measured about 0 because truck 05 was dead by then, not because a host change since then switched
this on.

Evidence levels: image routines are **read** (disassembly and existing docs). Host behaviour is
**read** from origin/main `fc69eaf16`. The geometry is **computed** by a probe over this
installation's scene and model files. The diff is **build-tested** with the switch OFF. Nothing here
is game-validated.

## 1. The observation (copied from the logs)

Main's diagnostic run is `J:\PROG\battlestations-pacific-decompile-cc9-lua51\local\l51_D0b3_j8.log`.
cc9-lua48's `l48_hqdmg.py` output for that log:

```
396 hits on Headquarter 01, health 12000.0 -> 0.0, t 37.50..1295.94
Japanese AA truck 05 cat2 b19            hits   73  damage   3116.8  t   37.50..1257.00
Japanese AA truck 04 cat2 b19            hits    4  damage    279.4  t 1075.02..1085.53
Japanese AA truck 03 cat2 b19            hits    3  damage    196.4  t 1065.16..1091.34
```

Every truck-05 shot from 960 s on is one direct hit and one burst at distance 0, at the same time.
The first five bursts also reach US Barracks 08, at a constant distance:

```
l48diag hit t=960.19 shooter=Japanese AA truck 05 row=8 plat=1 cat=2 bullet=19 priced=19 victim=Headquarter 01 side=1/1 health=12000.0 direct
l48diag hit t=960.19 shooter=Japanese AA truck 05 row=8 plat=1 cat=2 bullet=19 priced=19 victim=Headquarter 01 side=1/1 health=11951.9 blast
  impact blast bullet=19 on Headquarter 01 dist=0.0 entries=0 nearest=0.0 base=50.0 range=35.0 armour=30.0 took=11.1 health=11940.8
  impact blast bullet=19 on US Barracks, Quonset Hut, Brown 08 dist=16.9 entries=0 nearest=16.9 base=50.0 range=35.0 armour=2.0 took=23.8 health=111.9
```

The HQ hit times are 37.50 (burst only), then a hit every 5.0-5.3 s through 960.19-1052.55, then
1103.80, then 1174.02-1257.00. On each shot the health drop splits into about 48 from the direct
hit and 11.1 from the burst.

**The 37.50 s shot is different.** Its burst is at distance 0.0 from Barracks 08, so it hit the
barracks directly, and the HQ took only its splash. The same line is in the 10-06 run.

**Positions.** The host's end-of-run unit table, together with the scene file
`universe/scenes/missions/COTP-IJN/PRCPIJN/prcpijn_08_defend_guadalcanal.scn` (2024-08-09; the
host's coordinates are the scene's minus (19500, 15900)):

| entity | class (row) | scene localframe: right row; translation |
| --- | --- | --- |
| Headquarter 01 | CommandBuilding, type 97: Width 65, Height 26, Length 80, `command_hq.mmod` | (0.4383, 0, -0.8988); (20848.93, 5.00, 11431.91) |
| Japanese AA truck 05 | LandFort, type 450 "Jap Coastal Gun Concrete" (`cannonplatform-big.mmod`), gun device 37 "Jap Deck Gun", bullet 19 (Range 1500, BlastRange 35) | (-0.9724, 0, 0.2335); (20880.44, 5.36, 11408.21) |
| US Barracks, Quonset Hut, Brown 08 | LandFort, type 512: Width 8, Height 7, Length 13 | (0.4384, 0, -0.8988); (20885.96, 5.06, 11391.56) |

Truck 05's land mount is the class slot at local (0, 1.71, 0.01), as the log reports for truck 03:
`gunnery: land mount Japanese AA truck 03 platform 1 cat=2 dev=37 slot local=(-0.00 1.71 0.01)`.

## 2. Question 1: can the image's rounds hit and damage their own building?

**Yes, both directly and by blast, whenever the building's mesh is in the line.**

- **Direct hits exclude only the shooter.** `BSP_Projectile_TraceSegmentAndImpact` (`0084BF00`)
  sweeps with `0098B370(index, segment, owner->vtable[B0h](), record)`, called at `0084C1D3`. That
  reaches `0098ADD0` (at `0098B3EB`) and `0098AC20` (at `0098AF8C`). The excluded entity is
  compared by pointer, at `0098AEE4` and `0098ACE3` (docs/HIT_NARROWPHASE.md). It is only the
  shooter's collision root.
- **The parent exemption does not apply to shells.** The only parent-entity exemption in
  `0084BF00` drops a hit whose entity shares `+9D4h` with `local_94`, when `classDesc+8 == 11h`.
  `local_94` is set only when `[shot+0CCh]` passes `IsKindOf(0Fh)`. For a projectile, `[shot+0CCh]`
  is the projectile itself (class 29h), so the exemption belongs to the other caller, `007C0910`
  (docs/PROJECTILE_IMPACT.md step 6, and the decompile at 0084BF00).
- **No arming distance or flight time.** The per-step sweep in `006E6490` is gated only by
  `[projectile+5Ch]` (`006E64EF`). That byte is the scene-node "active" byte (`kSceneNodeOffActive`,
  `00922F30`), not a timer. Only the flak tick `0070C370` has a fuse time (`classDesc+D8h`,
  `0070C3E6`), and bullet 19 bursts on impact. So the first segment runs from the muzzle.
- **No side test on damage or burst.** `0084BAD0` exempts only the source entity, and
  `008777D0`/`00826F10` apply damage whatever the shooter's side (docs/CC13_AUDIT_USN15_GUNNERY.md
  item 4, GUNNERY 78).
- **No line-of-fire test for this gun.** Truck 05 fires category 2. `00729560` installs the
  line-of-fire predicate `0072CDD0` for kinds 1, 5 and 6 only (GUNNERY 63, 164.2).

**What the image tests against is the building's GeomMesh.** `0087BCC0`
(`BSP_UnitInstance_InitHealthAndParts`, called from `00955420` at `00955424`) builds the unit-part
instance for every unit:

```
0087be08: mov edx, dword ptr [esi]
0087be0a: mov eax, dword ptr [edx + 0x5c]
0087be0d: push 0x1b                ; IsKindOf(1Bh): land structure (MLandFort, MCommandBuilding)
0087be11: call eax
0087be15: push 0x1ac
0087be1a: je 0x87be5f
0087be1c: call 0xbf681b            ; operator new(1ACh), the part instance
0087be34: fld dword ptr [0xced9e0]  ; the land-structure LOD constant
0087be3c: mov eax, dword ptr [edx + 0x190]
...
0087be58: call 0x7135c0            ; 007135C0 -> 00712440: 00CFD768 shapes carrying the GeomMesh
```

The land structure has its own branch here. Its segment test is therefore `00724510 -> 00723E90 ->
00723D60` over the model's triangles (docs/NARROWPHASE_UNIT_PART_SHAPE.md §0). The only class that
gets the transformed box at `entity+1A4h` (`00929B80`) is the breakup piece (`0092AAE0`, from
`00935D30`), as `kPlaneMeshHitTestBound`'s comment already records for planes.
`LandFort`'s shape reload at `00747244` (docs/COLLISION_SHAPES.md row D) appends the same list-node
shapes into `[unit+360h]`.

**The host tests the class box instead.** `SegmentBinding::shape_trace_segment` uses
`hit_mesh_of`, which is ship (`kShellHullHitTestBound`) or plane (`kPlaneMeshHitTestBound`) only.
Any other unit falls to the `0085CDB0` slab against `{Width/2, Height/2, Length/2}` about the origin.
The blast distance (`apply_impact_blast`) uses the same box for non-ships.

### The geometry (probe `local/hq1_probe.cpp`, this tree)

`command_hq.mmod` (2024-07-13) has 2 GeomMeshes, of 47 and 84 triangles, with bounds x -29.22..7.60,
y -0.13..15.55, z -36.00..35.00. The host box is x ±32.5, y ±13, z ±40. Truck 05's muzzle, posed in
the HQ frame:

```
muzzle in HQ frame (35.11 2.07 17.93); host box half (32.5 13.0 40.0)
mesh 0: 47 tris, muzzle-to-mesh distance 27.51 m
mesh 1: 84 tris, muzzle-to-mesh distance 27.51 m
muzzle-to-host-box distance 2.61 m
el=-10.0  directions=180  host_box_hits=85 (nearest 2.7 m)  mesh_hits=0 (nearest -1.0 m)  box_only=85
el= -5.0  directions=180  host_box_hits=85 (nearest 2.6 m)  mesh_hits=0 (nearest -1.0 m)  box_only=85
el= -2.0  directions=180  host_box_hits=85 (nearest 2.6 m)  mesh_hits=47 (nearest 27.5 m)  box_only=38
el=  0.0  directions=180  host_box_hits=85 (nearest 2.6 m)  mesh_hits=47 (nearest 27.5 m)  box_only=38
el=  5.0  directions=180  host_box_hits=85 (nearest 2.6 m)  mesh_hits=47 (nearest 27.6 m)  box_only=38
el= 10.0  directions=180  host_box_hits=85 (nearest 2.7 m)  mesh_hits=43 (nearest 27.9 m)  box_only=42
el= 20.0  directions=180  host_box_hits=84 (nearest 2.8 m)  mesh_hits=23 (nearest 29.3 m)  box_only=61
el=-1.0 per-azimuth (world heading, 0=+z scene, 90=+x): B=box only, M=mesh only, X=both, .=neither
  XXBBBBBBBBBB......................................BBBBBXXXXXXXXXXXXXXXXX
```

- At grazing elevations, the box catches 85 of 180 headings and the mesh catches 47.
- Below -3 degrees the mesh catches none, because the box reaches 13 m below the HQ origin.
- Headings 10-55 and 250-275 degrees are box only.
- Shots at the barracks (161.7 degrees) and at the two beached LandingShips (179.6 and 223.4
  degrees) cross neither volume. This is why the 37.50 s shot reached the barracks.

**The 960-985 s bursts are box-only shots.** The probe looked for headings where the burst point
on the host box (backed off 0.05 m, `kBlastCentreBackOff`) lies 16.8-17.0 m from Barracks 08's
class box. For every elevation from -6 to +30 degrees there are exactly two: **48.5-49.0 degrees**
(burst 9.8-10.2 m out) and **254.5 degrees** (14.4 m out). Both are in the box-only sectors. In the
image, those five rounds pass the HQ and burst elsewhere. After Barracks 08 dies (985.88 s), no
other unit is within 35 m of a burst, so the log does not constrain the headings of the later
shots. Shots aimed at 275-10 degrees within 10 degrees of level would hit the HQ in the image too.

## 3. Question 2: which host change since 10-06 enables it? None

The 10-06 base is main `62988a6fa` (5fg's `l48_D0`). It already had the class-box fallback,
`kAabb0085cdb0Bound = true`, `kShellHullHitTestBound = true` and `kPlaneMeshHitTestBound = true`.
A pickaxe (`git log -S`) over `62988a6fa..origin/main` finds no commit that touches
`SegmentBinding`, `box_of(`, `shape_trace_segment`, `hull_height`, `hit_mesh_of` or
`gun_muzzle_point`.

| suspect | what it changes | reaches bullet-19 hits on a building? |
| --- | --- | --- |
| `ecb1c55d1` (with `ec19522f3`) | `kGunneryLiveWaterSurfaceBound` ON: the medium-crossing height is the live sea instead of y = 0 (`0078CF20`/`0084B380` path) | no: the static water trace, not the entity sweep; these rounds hit 2-15 m from a land muzzle |
| `90493a7c3` | `kAaKind6GateBound` ON: kind-6 (AA) slot gate before the line-of-fire query | no: truck 05's gun is category 2 |
| flak FlyTime / full-damage blast (`8160e604f`, OFF) | the flak tick's FlyTime burst | no: OFF on main, and bullet 19 bursts on impact |
| `d13436eb4` | `src/hull_terrain_contact.cpp`: scaled fort frames in the convex support table (crash fix) | no: rigid-body hull-fort contact, not the projectile segment query |
| `kTailGunnerNoArmourTestBound` (`43da2ae09`, `acd45f8c1`) | a plane-owned kind-1 gun skips the armour term (`0072C6A0`, TailGunnerBot) | no: category 2, land-owned |

The other 19 gunnery commits since 10-06 (`git log 62988a6fa..origin/main --
src/game_hosts_gunnery.cpp`) are torpedo, bomb drift, shipyard hangar, roll-arm, kill-attribution
and SetHealth-guard changes. None touches projectile-versus-static collision.

**Why it measured about 0 on 10-06** (cc9-lua48's `local\l48g9_D0_j8.log`):

```
death row: victim=Japanese AA truck 05 t=714.80 alt=9 first_damage=659.91 killer=Nevada killer_gun=492 killer_cat=4 ...
gunnery: garrison revive unit=Japanese AA truck 05 health=300 t=1005.73 (006F3660 re-create, packet cc9_command_building_garrison)
command building garrison: Headquarter 01 re-creates Japanese AA truck 05 with party 2 skill 1 at t=1005.90 (neutralize D4h 006F5362): revived in place
```

- In g9, Nevada kills truck 05 at 714.80 s, before the invasion phase.
- It comes back only at the HQ's neutralize (1005.90 s), with party 2.
- Its only HQ line in g9 is the 37.50 s splash (`hits 1 damage 0.0`).

On main the battle differs. Nevada sinks at 535.99 s (Hosho_sqn08) and Missouri at 671.31 s
(Gyoraitei #Y11, category 7). Truck 05 survives to 1258.90 s and fires from 960 s on.

**The same over-reach was already present on 10-06.** In g9, truck 03 lands 17 direct hits on its
own "Hangar, Small, 04 01", 34 m away. This was not checked geometrically.

## 4. Verdict

- **Faithful:** a Japanese gun's rounds can hit and damage a Japanese building, both directly and
  by blast. The only exemption is the shooter itself. There is no arming time and no line-of-fire
  test for category 2.
- **Host defect:** the hit volume. A land structure's direct-hit shape is its model's GeomMesh in
  the image and the class box in the host. For JM08's HQ, the box makes a nearby friendly gun
  (truck 05, 35 m from the HQ origin) hit the HQ in 85 of 180 grazing headings instead of 47, and
  on every depressed shot. The impact is 2.6 m from the muzzle instead of 27.5 m.
- **Demonstrated** for the five 960-985 s shots (box-only headings).
- **Not resolved** for the other 31 direct hits, because their headings cannot be recovered from
  the log. How much of the 3117 the switch removes needs a run with it ON.
- **Residual, not in the diff:** the blast distance to a land structure is still the class box
  (`apply_impact_blast`, about line 10380). The image's sphere test is `0070F720 -> 00723F80` on the
  same mesh shapes. A ground burst 2-3 m from truck 05 is at distance about 2.6 m from the box but
  about 27 m from the mesh. That would be a second switch, mirroring `kPlaneBlastElementEntriesBound`.
  A land structure's elements are `body` (kind 9), and how the structure damage path takes per-element
  entries is unread.

## 5. The diff (OFF switch, cc9's file)

`kLandStructureMeshHitTestBound` (OFF):
- `hit_mesh_of` also returns the class model for an `IsKindOf(1Bh)` unit.
- With the switch OFF, `ship_model` equals the old `plane_model == nullptr` for every unit that
  reaches the mesh branch.
- With the switch ON, a structure's record keeps `0Ah / -1`, as a plane's does. The broadphase
  `box_of` already takes `mesh_box` from `hit_mesh_of`.

Every `SegmentBinding` user sees the change when it is ON: the projectile sweep, the AA
line-of-fire retry, the HUD pick and the gun seat. All of them go through `0098ADD0` in the image,
so this matches the image.

Build: `./scripts/build.ps1` in this tree with the diff applied passed (§6). Applies to origin/main
`fc69eaf16` and `eb3311a32` (`git apply --cached --check` on the doc's copy; the file is identical in
both).

```diff
diff --git a/src/game_hosts_gunnery.cpp b/src/game_hosts_gunnery.cpp
--- a/src/game_hosts_gunnery.cpp
+++ b/src/game_hosts_gunnery.cpp
@@ -980,6 +980,19 @@ constexpr bool kHullElementSegmentBound = true;
 //    docs/AA_LETHALITY_AUDIT.md section 9.5.
 //    Packet cc9_plane_mesh_hit.
 constexpr bool kPlaneMeshHitTestBound = true;
+//  * kLandStructureMeshHitTestBound: a shell's direct-hit test against a LAND
+//    STRUCTURE (IsKindOf 1Bh: MLandFort, MCommandBuilding) uses the triangles of
+//    its model's GeomMesh, as kPlaneMeshHitTestBound does for a plane. 0087BCC0
+//    builds the unit-part instance for every unit and picks the land-structure
+//    LOD constant [00CED9E0] by vtable[5Ch](1Bh) at 0087BE0D, so a fort's 00CFD768
+//    shapes carry its GeomMesh too. JM08's Headquarter 01 (command_hq.mmod) spans
+//    x -29.2..7.6, y -0.1..15.6, z -36..35 against the host's 65 x 26 x 80 box
+//    centred on the origin: AA truck 05's muzzle is 2.6 m from the box and 27.5 m
+//    from the mesh (docs/CC13_JM08_AA_TRUCK_HQ_DAMAGE.md). Labelled substitutions
+//    as for the plane: every triangle of every mesh in the file, and the record
+//    keeps 0Ah / -1. The blast distance stays the class box. OFF: the class box.
+//    Packet cc13_jm08_aa_truck_hq_damage.
+constexpr bool kLandStructureMeshHitTestBound = false;
 //  * kPlaneBlastElementEntriesBound: a burst's distance to a PLANE is taken per
 //    GeomMesh element, as kBlastElementEntriesBound does for a ship. The image's
 //    sphere test for the plane's unit-part shape is 0070F720, which ends in
@@ -2470,9 +2483,19 @@ struct GameGunneryHost::Impl {
         const ShipModelSlots& model = ship_model_slots(type_id);
         return model.has_mesh ? &model : nullptr;
     }
+    // Packet cc13_jm08_aa_truck_hq_damage. See kLandStructureMeshHitTestBound.
+    const ShipModelSlots* land_structure_mesh_of(std::size_t index) {
+        if (!kLandStructureMeshHitTestBound || index >= unit_state.size()) return nullptr;
+        if (!units.unit_is_kind_of(index, bsp::kUnitKindQueryLandStructure)) return nullptr;
+        const int type_id = unit_state[index].row.type_id;
+        if (type_id < 0) return nullptr;
+        const ShipModelSlots& model = ship_model_slots(type_id);
+        return model.has_mesh ? &model : nullptr;
+    }
     const ShipModelSlots* hit_mesh_of(std::size_t index) {
         if (const ShipModelSlots* ship = ship_mesh_of(index)) return ship;
-        return plane_mesh_of(index);
+        if (const ShipModelSlots* plane = plane_mesh_of(index)) return plane;
+        return land_structure_mesh_of(index);
     }
     // Packet cc9_aa_turn_average: 00901C20's turn-rate average, 00901CC1..00901EEE,
     // on the target velocity `v` in place. See kAaTargetTurnAverageBound.
@@ -8223,8 +8246,6 @@ bool SegmentBinding::shape_trace_segment(const void* entity, int,
     // The hull box in its own frame: a slab test along the three pose rows.
     float right[3], up[3], forward[3], origin[3];
     owner_.unit_pose(index, right, up, forward, origin);
-    const GameGunneryHost::Impl::ShipModelSlots* plane_model = owner_.ship_mesh_of(index) == nullptr
-        ? owner_.plane_mesh_of(index) : nullptr;
     if (const GameGunneryHost::Impl::ShipModelSlots* census = owner_.ship_mesh_of(index) == nullptr
             ? owner_.plane_mesh_of(index, true) : nullptr) {
         // Census only, on both sides of the switch: the class box's and the
@@ -8292,8 +8313,10 @@ bool SegmentBinding::shape_trace_segment(const void* entity, int,
         float best = 2.0f;
         int best_kind = 0x0A;
         int best_index = kDirectHitHullSegment;
+        // A plane's or a land structure's record keeps 0Ah / -1 (see the switches).
+        const bool ship_model = model == owner_.ship_mesh_of(index);
         const bool by_element = kHullElementSegmentBound && !model->element_tris.empty()
-            && plane_model == nullptr;
+            && ship_model;
         const auto& tri = by_element ? model->element_tris : model->tris;
         for (std::size_t n = 0; n + 2 < tri.size(); n += 3) {
             const auto& a = tri[n];
@@ -8338,7 +8361,7 @@ bool SegmentBinding::shape_trace_segment(const void* entity, int,
         bsp::hit_record_set_entity_00470370(record, entity);
         hit_unit = index + 1;
         hit_landscape = 0;
-        if (plane_model == nullptr) ++owner_.shell_mesh_hits;
+        if (ship_model) ++owner_.shell_mesh_hits;
         return true;
     }
     const GameGunneryHost::Impl::UnitState& state = owner_.unit_state[index];
```

**Suggested pair:** JM08 with `kLandStructureMeshHitTestBound` ON against OFF, using cc9-lua51's
orders and l48diag. Expect the 960-985 s truck-05 HQ lines to disappear. Expect the 37.50 s splash
to remain. On ON, also check that the forts' own guns and the AA line-of-fire counts
(`aa line of fire queries/blocked`) move only by the changed building silhouettes.

## 6. Status

- The diff was built with the switch OFF in `J:\PROG\battlestations-pacific-decompile-cc13-hqdmg1`
  (`./scripts/build.ps1`: build passed, `100% tests passed, 0 tests failed out of 2`), then reverted. It is not committed.
- `reports/cc13_jm08_aa_truck_hq_damage_calls.json` holds the five call sites cited above, for
  `tools/verify_report_calls.py`.
- The probe sources (`local/hq1_probe.cpp`, `local/hq1_cl.ps1`, `local/hq1_bursts.py`,
  `local/hq1_cmp.py`) are scratch files in this tree's ignored `local/`.
