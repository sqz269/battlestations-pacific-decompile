# Scene unit skill: 00927A80 over the merged bag, and who it actually moves

Packet `cc9_scene_unit_skill`, cc9-lua16, 2026-09-29. Addresses: `00822C20`, `00927A80`, `006E6210`,
`006D3CF0`, `00748383`, `0074210A`, `007D65AA`, `007F211B`, `00849D71`. Switch `kSceneUnitSkillBound`
(`src/game_hosts_units.cpp`). All names are hypotheses, not recovered symbols. This follows
docs/AA_LETHALITY_AUDIT.md section 7.1, which recorded the gap and did not bind it.

## 1. The read

**`00822C20` (SEntityInit), `0082386B`-`008238CB`.** `[unit+C0h]+4` selects the arm:
- **1, scene-created** (`008238B1`): `EDI = vtable+128h`, `008238C1 CALL 00927A80`, `008238C8 PUSH EAX`,
  `008238CB CALL [EDI]`. So `unit->vtable[128h](bag skill)`, the SetSkillLevel leaf that stores
  unit+390h (docs/PILOT_SKILL_LEVEL.md section 1).
- **2, template-created** (`00823882`): `vtable[128h]([template+B8h])`, then `[unit+738h]->vtable[3Ch]([template+110h])`.
  Not reached by scene placement; not bound here.

**`00927A80` (`BSP_PropertyBag_GetSkill`), read whole, `00927A80`-`00927ACB`.** `ECX` = entity. It
finds key `00CF8838` `Skill` in `[[this+C0h]+8]` through `008F2260`. If the key is found, it
returns the record's `+0Ch` int. Otherwise it finds `00D19264` `Crew`, and if that is found it
tail-jumps with `ECX = +0Ch` into `006E6210`. With neither key it returns 1.

**`006E6210`, read whole, `006E6210`-`006E6265`.** `f = ([00E188A8]+1FE4h != 0)`. The mapping is
crew 0 -> `f`, 1 -> `1 + 2f`, 2 -> `2 + 2f`, 3 -> 5, anything else -> 1. This host is single player
only (`f = 0`): Rookie 0 -> Stun 0, Regular 1 -> 1, Veteran 2 -> 2, Elite 3 -> Elite 5.

**The other creators take the same value into the same slot.** Each of these is a `CALL 00927A80`
then `PUSH EAX` and `CALL vtable[128h]`, read from disk bytes:
- `006D3CF0`, the airfield (`006D3C10`);
- `00748383`, `007482B0`, in the land-fort/vehicle segment;
- `00849D71`, the shipyard;
- `007D65AA`, the plane (docs/PILOT_SKILL_LEVEL.md).

`0074210A`, the land convoy, keeps the value in `EBP` for its member loop over `+394h`, which is
not traced here. `007F211B`, the squadron's order speed, is a reader and not a store.

## 2. The finding that changes the scope: group defaults hold `Skill`

`00927A80` searches the **merged** bag. docs/SCENE_PROPERTY_BAG_MERGE.md shows that the group
defaults are merged into the entity's bag before the authored body. This installation's library
declares `Skill = E SkillLevels : SPNormal` in four groups (file mtimes 2024-07-13):
- `Ship(Common)`, `universe/library/ship.props:5`, and `Sub(Ship)` by inheritance;
- `LandFort(Common, MultiEntity)`, `landfort.props:6`;
- `LandConvoy(Common)`, `landconvoy.props:49`;
- `PlaneSquadronWNavpoint(Common)`, `plane.props:17`.

No group declares `Crew`. So `Crew` is consulted only by an entity whose groups include none of
the four and whose body authors no `Skill`.

`local\l16_skill_census.py <scn>` (cc9-lua16 tree) walks a scene with this rule.

**JM05** (`ijn_05_invasion_of_port_moresby.scn`, mtime 2024-07-13):
- 305 LandFort, 6 LandConvoy and 3 CommandBuilding entities all resolve through the group default
  to **1**.
- The two airfields (`LandingZone` group) author `Skill = SPNormal` in their bodies, so they also
  resolve to 1.
- All 50 `Crew = Rookie` lines sit on these entities, so **none of them reaches `006E6210`**.
- The only entities that move are **USS Lexington and USS Yorktown**, which author
  `Skill = SPVeteran` (2).

So the claim in AA_LETHALITY_AUDIT.md 7.1 ("50 `Crew = Rookie`, so Stun in the image") does not
hold for this installation: the group default beats `Crew`. That half of the finding is retracted
here. The carrier half stands.

**US rows.**

| row | scene | entities whose bag gives other than 1 | re-skilled by the script afterwards |
| --- | --- | --- | --- |
| USN04 | `usn_19_coralus.scn` | none | - |
| USN01 | `usn_1_marshall.scn` | Northampton, Dunlap, SaltLakeCity, Ralph, McCall, Blue: MPNormal 3 | all six, to 2 (`SetSkillLevel` log lines) |
| USN13 | `usn_13_truk.scn` | 45 US ships at MPNormal 3 | 37 of them, to 2. **Iowa, NJ, Mnp, NO and DD_22..DD_25 are not re-skilled** in 3000 frames |

## 3. The binding

- `GameSceneEntityRecord::bag_skill` / `bag_skill_source` are filled by the instantiate pass of
  `src/game_hosts_scene_contents.cpp`:
  - an `I` value is its own integer, and an enum symbol resolves through the library;
  - `Crew` goes through the single-player `006E6210` mapping.
- `create_units` copies the value into `pilot_skill_index` (unit+390h) when `kSceneUnitSkillBound`
  is set. A later script `SetSkillLevel` overwrites it, as `vtable[128h]` does in the image.
- Both builds print `scene skill: unit=... level=... applied=...` for every unit whose bag gives
  other than 1.

**Not bound:**
- The template arm (`00823882`).
- The convoy member loop.
- The launch inheritance: the carrier launch bag's `Skill = owner->vtable[12Ch]()`
  (`include/bsp/air_operations.hpp`, where `owner_skill` is never set and `request.skill` is never
  applied). With this switch on, the JM05 carriers are at 2 in the image, and so are the planes
  they launch. The host's launched planes stay at 1. That is the next gap in this chain.

## 4. Predictions, written before the runs

Same-binary OFF/ON pairs, reference environment.
1. **USN04 4700/4500:** no `scene skill` lines. pair_diff 0 or 1.
2. **USN01 3200/3000:** six lines at level 3. The script re-skills all six to 2 at mission start,
   before any shot, so the pair is gameplay-identical (0 or 1). A move would mean some gun tick
   reads the skill before the script call.
3. **USN13 3200/3000:** 45 lines. Where Iowa, NJ, Mnp, NO and DD_22..25 fire within the row,
   their artillery and AA rows move from 1 to MPNormal 3, and only their shots move. If they do
   not engage in 3000 frames, the pair is identical apart from coupling.
4. **JM05 9200/9000:** two lines (Lexington, Yorktown, level 2). The carriers' own guns take row 2:
   the artillery aim error, the section points, the AA gun throw and the torpedo scan row. The IJN
   aircraft deaths credited to the two carriers may move. Nothing else changes, apart from
   coupling: no fort, convoy or airfield changes skill.
5. **Mechanism failure:** a `scene skill` line on a fort, convoy or airfield in JM05, or a level
   that differs from the census.

## 5. Measured, and the verdict

The pair is one commit, `ff4ac4806`, exported twice by `tools/pair_export.py`: OFF `E28973BC78BF`,
and ON `B384DA1A95EF` with `kSceneUnitSkillBound=true`. Both ran in the reference environment. A
300-frame JM05 smoke of the ON binary came first and printed exactly the two carrier lines.
The logs are `local\l16_s{off,on}_{usn04,usn01,usn13,jm05l}.log` in the cc9-lua16 tree.

| row | `scene skill` lines | pair_diff | deaths |
| --- | --- | --- | --- |
| USN04 4700/4500 | 0 | 1, gameplay identical | 45, identical rows |
| USN01 3200/3000 | 6, all MPNormal 3 | 1 | 5, identical |
| USN13 3200/3000 | 33, all MPNormal 3 | 1 | 32, identical |
| JM05 9200/9000 | 2: USS Lexington and USS Yorktown, `Skill` 2 | 1 | 29, identical |

Each prediction against the result:
1. **USN04: held.** The run printed no `scene skill` lines.
2. **USN01: held.** The script re-skills all six ships to 2 before any gunnery tick reads the value.
3. **USN13: held, trivially.** Only 33 of the 45 MPNormal entities are created in 3000 frames.
   Iowa, NJ, Mnp, NO and DD_22..25, which the script would not re-skill, are among the 12 that
   are not created. Every created one is re-skilled.
4. **JM05: held for the mechanism, and the effect is null.** The two carriers start at 2. Neither
   carrier appears in any death row or moved line, because no attacker reaches them in 9000
   frames with an idle player.
5. **Mechanism: held.** No fort, convoy or airfield printed a line. The levels match the census
   exactly.

**Verdict: `kSceneUnitSkillBound` flips ON.** The mechanism matches the image, and the
switch changes no gameplay in any reference row today. It matters once:
- a carrier fights, as in the JM05 carrier strike or a player-driven run;
- or MPNormal ships are created without a script re-skill;
- or the launch inheritance of section 3 is bound. The JM05 carriers' planes would then fly the
  SPVeteran pilot rows.
