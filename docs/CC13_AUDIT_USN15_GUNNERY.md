# USN15 Surigao gunnery audit (cc13-usn15g1, for cc9)

Packet `cc13_usn15_gunnery`, 2026-10-12. Read-only audit: no game runs, no Ghidra writes, no gameplay
change. The one host change proposed is a diagnostic behind its own OFF switch (section 6). It was
build-tested and then reverted; it is not committed.

**Status words.** *Read* means read from the listing, the logs or the data files. *Exported* means the
Ghidra export. Nothing here is game-validated. The arithmetic predictions in sections 2 and 3 are
checked against log rows where a row exists, and are marked where one does not.

## 0. Verdicts

| # | question | verdict |
| --- | --- | --- |
| 1 | Fuso alone sinks the six US battleships while their death rows read `hits c0..c6=0` | **Faithful (data).** The US battleships do fire and hit Fuso (run a: 826 heavy shots, 341 hits, 19,920 hp dealt). The death row's `hits` are hits *received*, and the row prints categories 0, 1, 5 and 6 only, so heavy artillery (4) never shows. The asymmetry is this installation's modded data: Fuso/Yamashiro HP 90,000 against 10,000, and the Fuso 14in shell 2300-2500 damage against 230-250 for the US 14in. Host defect found only in the diagnostic line (section 6). |
| 2 | Every US 5in/6in impact on Fuso/Yamashiro shows armour 130, took 0 | **Faithful (image law + data).** Those are *blast* rows. The image's blast law `004705C0` is max(0, falloff x base - armour) x scale, and every US blast base (65, 75, 90, 115, 120) is below Armour 130. Direct hits still land, by `00470510` (draw - armour) x scale: Fuso's 322 category-6 hits did 3233 hp, 10.0 per hit, the mean of U(130,150) - 130. |
| 3 | The US Fletchers never launch torpedoes: `mask=0` | **Faithful (data).** The torpedo mask is the director's TorpedoDirector bit (+222h). `universe/library/ship.props` (bulk) defaults it to `false`, the scene sets no override, and this installation's USN15 script calls `TorpedoEnable` for the IJN groups only. The bulk IJN-side `07_surigao.lua` does enable the US destroyers. The image's only other writer, the CloseAttack command `00A11AF0`, never runs for the player's own side. |
| 4 | Friendly kills: Portland<-Pennsylvania, Fletcher-09<-Minneapolis, Mogami<-Fuso | **Faithful (image rule).** The image has no side exemption on damage (GUNNERY 78) and no line-of-fire test for weapon kinds 2, 3 and 4 (GUNNERY 63, 164.2). The host matches. Two of the "killers" are attribution artefacts: Portland died of flooding, and its `killer` (last hit of any size) and `killer_gun` (last damaging hit) name two different US ships. |

## 1. Evidence and provenance

### 1.1 Logs (cc9-ships44, reference environment)

The logs are `s44_a_u15i.log` (idle, run a), `s44_a_u15if.log` (idle, full frame trace) and
`s44_b_u15a.log` (run b). They were copied to this tree's ignored `local/` on 2026-10-12. All three
read `difficulty=3` side block, `MissionStart::set_effective_difficulty game+6ACh=1`, and
`BSP_GUNNERY_RNG_STREAMS=1`. `u15i` and `u15if` share every death row up to t=1751 s; `u15if` runs longer.

Excerpts (run a, `s44_a_u15i.log`):

```
246415:  Tennessee                    1    4   3000      160     68      471.41  Fuso
  (gun table: unit, plat, cat, range, assigns, shots, first_shot, target)
246549:  gunrow  116 Pennsylvania  plat 1 cat 4 assigns 108 ... shots 45 ... hits 27 dealt 1348.0 dev 208
death row: victim=Mississippi t=611.52 first_damage=576.13 killer=Fuso killer_gun=849 killer_cat=4 killer_blast=0 killer_range=2174 ... hits c0=0 c1=0 c5=0 c6=0
death row: victim=Fuso t=853.57 first_damage=570.53 killer=Minneapolis killer_gun=280 killer_cat=3 ... hits c0=0 c1=0 c5=0 c6=322 dmg ... c6=3233
death row: victim=Portland t=730.25 first_damage=513.40 killer=Pennsylvania killer_gun=489 killer_cat=6 killer_blast=0 killer_range=2316 nearest_ship=Fuso nearest_horizontal=246 ... c6=3 dmg ... c6=87
115171:gunnery: damage control death Portland t=730.25 water_total=1135 fire_total=0 repaired=0
gunnery: component failure Explosion on Fuso t=570.53 health=90000/90000
  impact blast bullet=1 on Fuso dist=0.0 entries=7 nearest=0.0 base=115.0 range=150.0 armour=130.0 took=0.0 health=89899.0
  impact blast bullet=25 on Mississippi dist=1.2 entries=6 nearest=0.0 base=105.0 range=150.0 armour=130.0 took=0.0 health=8687.7
  impact blast bullet=25 on Mississippi dist=4.1 entries=6 nearest=0.0 base=105.0 range=150.0 armour=130.0 took=0.0 health=7179.0
246246:summary mission gunnery ship director torpedo writes lua_enable=17 lua_disable=0 close_attack_sends=0 changed=17 bound=1
246239:summary mission gunnery ship director enables torpedo_disabled_pushes=28837 bound=1
246383:summary unit gunnery torpedo candidates Fletcher-class11 scored=484 masked=22 accepted=0 mask=0 (00863990)
246384:summary unit gunnery torpedo candidates Fletcher-class22 scored=700 masked=700 accepted=0 mask=0 (00863990)
246385:summary unit gunnery torpedo candidates Nachi scored=62 masked=0 accepted=62 mask=3 (00863990)
247681:summary mission ai coordinator game_mode=8 compose=0 seeds=0 groups_created=0 ... tick_orders=0
```

Heavy-gun totals per unit, summed over the final `gunrow` table (`local/u15g_gunrows.py`):

| unit | class (type_id) | run a shots / hits / dealt | run b shots / hits / dealt |
| --- | --- | --- | --- |
| Mississippi | Tennessee 1941 (260) | 124 / 37 / 2881 | 117 / 40 / 2618 |
| Maryland | Colorado 1945 (310) | 65 / 24 / 2086 | 70 / 27 / 2383 |
| West Virginia | Colorado 1945 (310) | 61 / 20 / 2077 | 78 / 31 / 2004 |
| Tennessee | Tennessee 1941 (260) | 255 / 119 / 6025 | 235 / 70 / 4056 |
| California | Tennessee 1941 (260) | 149 / 47 / 2402 | 161 / 37 / 1606 |
| Pennsylvania | Pennsylvania 1941 (261) | 172 / 94 / 4449 | 142 / 63 / 3227 |
| **Fuso** | Fuso 1943 (61) | 279 / 132 / **65,127** | 289 / 109 / **57,332** |
| Yamashiro | Fuso 1943 (61) | 91 / 155 / 20,062 | 240 / 76 / 37,357 |

Hits per shot are alike: Fuso 0.47 in run a, Tennessee 0.47, Pennsylvania 0.55. This is not a strict hit
rate, because the gunrow `hits` also count blast records on other units (Yamashiro's exceed its shots).
What differs is the damage per hit: Fuso averages about 490 per hit, the US battleships about 50.

### 1.2 Data files (this installation, `I:/SteamLibrary/steamapps/common/Battlestations Pacific`)

`gamemode.lua` (2024-07-13 11:27, a BSPRM note) sets `GameMode = 0`, so `autoload/bulletclasses.lua` loads
the **arcade** table. The blast ranges in the log (bullet 1: 150, the arcade value; realistic: 25) confirm it.

| file | mtime | status |
| --- | --- | --- |
| `scripts/datatables/autoload/vehicleclasses.lua` | 2026-05-09 21:52 | **modded** |
| `scripts/datatables/classtables/arcade/bulletclasses.lua` | 2026-05-09 | **modded**, loaded |
| `scripts/datatables/classtables/arcade/deviceclasses.lua` | 2026-05-09 | **modded**, loaded |
| `scripts/datatables/classtables/realistic/bulletclasses.lua` | 2025-06-02 | modded, not loaded |
| `scripts/missions/COTP-USN/PRCPUS/usn_15_battle_of_surigao_strait.lua` | 2024-08-09 06:00 | **modded** |
| `universe/scenes/missions/COTP-USN/PRCPUS/usn_15_battle_of_surigao_strait.scn` | 2024-08-09 06:00 | **modded** |
| `universe/library/ship.props` | 2024-07-13 08:27 | bulk |
| `scripts/missions/ijn/ESMP/07_surigao.lua` | 2024-07-13 08:26 | bulk |
| `scripts/datatables/globals.lua` | 2024-07-13 08:26 | bulk |

**Bulk comparison.** This installation keeps `scripts/` and `universe/` as loose files. No archive in the
install holds them: a search for `*.pak/*.dat/*.bin/*.arc/*.zip/*.big` over 1 MB at depth 2 finds only
`iowa_mmod/iowa.bin`, `out_bearcat/F8F_Bearcat.bin` and `shaderfx/shaders.bin`. The only pristine copies
are `robots__.lua.bak` and `global/commandhelpers.lua.bak`, and neither is involved here. So "modded" rests
on mtimes only: 2024-07-13 is the bulk install stamp, and anything later was rewritten. Content cannot be
compared against a retail original from this machine.

## 2. Item 1: do the US battleships fire and hit?

**Yes.** The trace through the gates, from the log:

- **Target choice.** All six target Fuso (`gun` table, `target` column; the ship-AI rows read `fire=Fuso`).
  Tennessee's first shot is at 471.41 s.
- **Gates.** The category table reads `HEAVYARTILLERY guns=36 assigns=4086 shots=1196 no_window=14452
  arc_blocked=6858`, both sides together. Tennessee and Pennsylvania show `refusals 0`; Mississippi 683,
  Maryland 734, West Virginia 556. No US heavy gun is starved.
- **Range and elevation.** The US 14in (bullet 1, arcade `bulletclasses.lua:26`) and 16in (bullet 101,
  `:2527`) both have `Range = 3000`. So does the Fuso 14in (bullet 25, `:791`). The engagement at 2.0-2.8 km
  is inside both. The gun table prints `range 3000` for both sides.
- **Fire control.** USN15 sets `SetSkillLevel(unit, Mission.SkillLevelOwn)` for the US groups
  (`usn_15...lua:270,282,294,306`) and `Mission.SkillLevel` for the IJN groups (`:396,415,434,453`). At
  difficulty 1 both are `SKILL_SPNORMAL` (`:71`, `:78`). Hits per shot match (section 1.1).
- **Hits on Fuso.** Run a has 94 bullet-1 and 13 bullet-101 impact rows on Fuso. Each direct hit also prints
  its blast row, as the 322 bullet-15 rows show against the death row's `c6=322`. The first 14in hit took
  Fuso from 90000 to 89899, which is 101 hp.

**Why Fuso wins: the class and ammo rows (all modded files).**

| row | file:line | value |
| --- | --- | --- |
| Fuso class 1943, `VehicleClass[61]` | `vehicleclasses.lua:31298` / `:31113` / `:32013` | HP **90000**, Armour 130, UnderwaterArmour 250 |
| Tennessee class 1941 `[260]` (Mississippi, Tennessee, California) | `:94917` / `:94794` | HP 10000, Armour 130 |
| Pennsylvania class 1941 `[261]` | `:95724` / `:95592` | HP 10000, Armour 120 |
| Colorado class 1945 `[310]` (Maryland, West Virginia) | `:133513` / `:133381` | HP 10000, Armour 130 |
| Fuso/Kongo 14in shell, arcade `[25]` | `bulletclasses.lua:778` / `:777` | DamageMin **2300**, DamageMax **2500**, blast 105 |
| US 14in (KGV row) `[1]`, devices 207/208 | `:13` / `:12` | DamageMin 230, DamageMax 250, blast 115 |
| US 16in (Iowa row) `[101]`, device 205 | `:2514` / `:2513` | DamageMin 280, DamageMax 300, blast 120 |
| devices: Fuso turrets `[25]`/`[26]` -> bullet 25; New Mexico `[207]`, Pennsylvania `[208]` -> bullet 1; Colorado `[205]` -> bullet 101 | `deviceclasses.lua:1440`, `:1509`, `:8895`, `:8957`, `:8773` | |

The arcade Fuso shell is the largest gun row in the table, above the 460 mm Yamato row (`[23]`, DamageMax
2020, `:713`). Its realistic twin is 230-450 (`realistic/bulletclasses.lua:743`), and its arcade `Mass` is
225.03 against 25.03 for the KGV row. The extra leading digit looks like a mod edit, deliberate or not. It
is data either way.

With the image's direct law `00470510`, (draw x ownerMod - armour) x scale, read in section 3:

- a US 14in on Fuso: 230..250 - 130 = **100..120** (the log shows 101);
- a US 16in on Fuso: 280..300 - 130 = 150..170;
- a Fuso 14in on a US battleship: 2300..2500 - 130 = 2170..2370 per hit, before the hull-segment and
  part handling of `00826F10`. The log shows Mississippi losing about 1500 per Fuso hit
  (8687.7 -> 7179.0 -> 5598.9). The gap to 2170 was not traced; it does not change the verdict.

Fuso needs about 800 US 14in hits. Six US battleships with 10,000 hp each fall to a few Fuso hits apiece.
Fuso's own HP went from 90000 to about 58000 at 570.53 s through a magazine `Explosion` component
failure (`ExplosionDamagePercentage=0.350`, `90000 x 0.35 = 31500`). Cruiser 8in fire then sank it
(`killer=Minneapolis killer_cat=3` in both runs).

**The battleships' stop at z ~ 1000** belongs to the ship-AI lane, not gunnery. Mississippi is the
player's controlled unit (`hud root observer ... controlled unit "Mississippi"`). It leaves its formation
and Tennessee becomes the column leader (`formation leave: unit=Mississippi ... leader Mississippi ->
Tennessee`). The followers then step `state=follow dir=stopped`. Not examined further here.

## 3. Item 2: armour 130, took 0

The image's two damage terms, both x87, read from the listing:

- **Direct, `00470510` (BSP_HitRecord_HullDamage).** `004705A4 FLD [ESP+4]` (record+14h x the owner
  modifier, `00470598..004705A0`), `004705A9 FSUB [ESP+10h]` (armour argument), `004705AD FMUL [ESP+8]`
  (weapon scale, `vtable[58h]` or 1.0 from `00D7A24C`). The result is not floored here. `008777D0` skips
  AddDamage at or below zero (GUNNERY 98.3). Called from `00826FA3` and `0082704D` in
  BSP_ShipEntity_ApplyHitRecord and from `008778A4` in BSP_UnitInstance_ApplyHitRecord.
- **Blast, `004705C0` (BSP_HitRecord_PartDamage).** falloff = 1 - entry distance / record+24h
  (`00470661..0047066F`), clamped to 1 under record+2Ch. Then `00470690 FLD; 00470694 FMUL [ESP]`
  (base x modifier) and `00470697 FSUB [ESP+10h]` (armour). `0047069F..004706B1` keeps the value only when
  it is above 0 (FLDZ / FCOMIP / JBE). Called from `008275B5` (ships) and `008779D3` (units).

Against Armour 130 (Fuso and Yamashiro are both class 61), every US blast base is below the armour:
5in 65 (`bulletclasses.lua:479`), 6in 75 (`:420`), 8in 90 (`:288`), 14in 115 (`:7`), 16in 120 (`:2508`).
Run b also shows bullet 30 (5.5in, 75) on Yamashiro. So **every blast row reads `took=0.0`**, in both runs,
for every bullet class. This is the image's law (the same verdict as GUNNERY 98.3).

The direct hits are not zero. Each direct hit prints a blast row too. Fuso's `c6=322` hits for `3233` hp
is 10.04 per hit, the mean of U(130, 150) - 130 = 10. That matches the direct law with ownerMod = scale = 1
(`game_hosts_gunnery.cpp`, `hit.owner_modifier = 1.0f`, `hit.weapon_scale = 1.0f`). Per direct hit on
class 61: 5in 0..20, 6in 15..25, 8in 70..90, 14in 100..120, 16in 150..170. A hit that the host routes to
the underwater segment meets 250, and everything but a torpedo does nothing there.

**Host check.** `apply_impact_blast` builds the blast record with `part_damage_base` = the class's blast
draw, `weapon_scale` 1, `owner_modifier` 1 and the class armour. The 1.0 owner modifier is a labelled
host substitution: `008E6430` over an empty modifier list, gated by `00E0C978` and `[00F88C30]+94h`
(`0047056B..0047057F`). It is the same for both sides, so it cannot make the asymmetry. Not a defect for
this question.

## 4. Item 3: why the Fletchers' torpedo mask is 0

The chain, read:

1. **The mask.** `00863990` refuses a candidate through `008633D0` when `mask[7]` lacks bit 1 (a ship
   target). `008624C0` (BSP_UnitGunneryDirectorBridge_Apply) stores the torpedo group's mask word from the
   director's `+222h` on every call (`00862581 CALL 00861D70`; host `apply_director_stance_008624c0`,
   `src/unit_gunnery_pass.cpp:128`). The log's `torpedo_disabled_pushes=28837` counts those pushes with
   `+222h = 0`.
2. **The producer of +222h.** It has three writers:
   - scene pass B reads `TorpedoDirector` from the unit's merged property bag (`008238FD..008239A8`;
     host `game_hosts_scene_contents.cpp:1764`);
   - Lua `TorpedoEnable` (`0089CA52 CALL 0071E0D0`, BSP_LuaBinding_TorpedoEnable);
   - the AI CloseAttack command (`00A11B5C CALL 0071E0D0`, BSP_AiCommand_CloseAttackEnableShipTorpedoes).
3. **Scene and library.** The USN15 scene's Fletchers are `DestroyerGen` entities with
   `properties (Common, GameUnit, Ship, Command)` and template `USA\SEA\Fletcher-class`
   (`usn_15...scn:8145` and its neighbours). The scene file contains no `TorpedoDirector` key (count 0).
   The `Ship` group default in `universe/library/ship.props:7` (bulk) is `TorpedoDirector = B false`.
   `ArtilleryDirector`, `AADirector` and `DCDirector` are `true` (`:6`, `:8`, `:9`). A Fletcher is
   destroyer kind, not kind 8, so the `00823941` class-8 override does not apply. No `ship director
   class8 override` row is in the log.
4. **Script.** USN15 (modded) calls `TorpedoEnable` only for the four IJN groups. At difficulty 1 the
   calls are `TorpedoEnable(unit, true)` (`usn_15...lua:405,424,443,462`). The log's 17 `TorpedoEnable:`
   rows all name IJN units, and `lua_enable=17`. The four US group loops (`:268-310`) set skill,
   avoidance, torpedo evasion and repair, but no torpedo enable. In contrast, the bulk IJN-side mission
   `07_surigao.lua` enables the US DesRon 56 and DesRon 24 destroyers at difficulty 1 and 2 (`:368`,
   `:383`), and its generated PTs (`:594`). This is why Nachi reads `mask=3` and every Fletcher reads `mask=0`.
5. **CloseAttack.** `close_attack_sends=0`. In single player the image never plans for the player's own
   side (SHIP_AI_OPEN_ITEMS, AI_BRAIN_PLAYER_EXEMPTION: slot 0 is NONCONTROL). USN15's coordinator line
   reads `groups_created=0 tick_orders=0`, so no AI command runs at all on this row. Whether the IJN side
   should have groups is cc9's coordinator question, not this one.
6. **Launchers.** These are present and not the cause. Each Fletcher carries two category-7 rows on
   device 65 (`US quintuple torpedo tube`, `deviceclasses.lua:3548`, bullet 62: arcade blast 1500, range
   50, `bulletclasses.lua:1666`). Against Fuso's UnderwaterArmour 250, a launch would do real damage. The
   mask keeps them silent.

**Verdict: faithful.** A player who orders torpedo fire, or a script that calls `TorpedoEnable`, is the
only route, and the modded USN15 script has neither for its US ships.

## 5. Item 4: friendly fire

**Image.** Both questions are already read in cc9's docs, and this audit re-checked the call sites:

- **Damage has no side test.** `0084BAD0` exempts only the source entity. `009239A0` notifies the
  victim's party but refuses nothing. `008777D0` / `00826F10` apply (base x mod - armour) x scale whatever
  the shooter's side (GUNNERY 78).
- **No line-of-fire test for artillery.** `00729560` installs the predicate `0072CDD0` only for weapon
  kinds 1, 5 and 6 (the install call is `0072E708` in BSP_Gun_SetupFromDescriptor). Kinds 2, 3 and 4
  (light, medium and heavy artillery) answer "clear" through `0072F6E0` (GUNNERY 63, 164.2). The kind-6
  cache is never invalidated (164.1), so a 5in mount keeps a clear answer after a friend steps into the line.

**Host.** `kAaLineOfFireBound = true` covers kinds 1, 5 and 6 only. The log reads `aa line of fire
queries=215 blocked=37`. `apply_impact_blast` skips only the shooter. That matches the image.

**The three cases, read from the death rows:**

| victim | `killer` / `killer_gun` | reading |
| --- | --- | --- |
| Portland (US) t=730.25 | Pennsylvania (US) / gun 489 = **Fletcher-class07** plat 3, cat 6 (US) | Portland sat **246 m from Fuso** (`nearest_horizontal=246`) while Pennsylvania (heavy, no line-of-fire test) fired at Fuso from about 2.3 km. It died of flooding: `damage control death Portland ... water_total=1135`. `killer` is the last attributed hit of any size (`target.last_attacker`, written on every hit, `game_hosts_gunnery.cpp` near `kill_attribution_0077ce60`). `killer_gun` is the last *damaging* hit (death table, `applied > 0`). The two name different US ships. Both are friendly; the image's rule. |
| Fletcher-class09 (US) t=753.64 | Minneapolis (US) gun 278, cat 3, range **142 m** | Medium artillery, no line-of-fire test, a destroyer 142 m off the muzzle line. The image's rule. |
| Mogami (IJN) t=496.75 | Fuso (IJN) gun 853, cat 4, range **51 m** | Mogami follows Fuso in formation. A heavy turret at 51 m, no line-of-fire test, a 2300-2500 shell against Mogami's 6500 hp (`vehicleclasses.lua:32958`) and Armour 90. The image's rule, made lethal by the modded shell. |

**Verdict: faithful.** The host neither adds nor drops a side test.

## 6. Host defect (diagnostic only): the death row hides categories 3, 4 and 7

The `death row` line (`src/game_hosts_gunnery.cpp`, `log.notef("death row: ...")`) prints `hits` and
`dmg` for categories 0, 1, 5 and 6 only. A battleship sunk by heavy artillery therefore reads `hits
c0=0 c1=0 c5=0 c6=0`, which was read as "the US battleships never hit". The counts are also hits
*received* by the victim, not dealt. `killer` and `killer_gun` use two different definitions (any hit /
damaging hit). This is DIAGNOSTIC only; no gameplay term reads it. The proposed fix adds a second line,
behind its own OFF switch, so existing death-row parsers and pair diffs are untouched while it is OFF.

It was applied in `agent/cc13-usn15g1`, built with `./scripts/build.ps1`, and reverted. The diff below was
checked with `git apply --check` against `origin/main` (section 7).

```diff
diff --git a/src/game_hosts_gunnery.cpp b/src/game_hosts_gunnery.cpp
index c7844a874..7069d80a8 100644
--- a/src/game_hosts_gunnery.cpp
+++ b/src/game_hosts_gunnery.cpp
@@ -2235,6 +2235,13 @@ struct GameGunneryHost::Impl {
         bool last_blast{false};
     };
     std::map<std::size_t, DeathTableRow> death_table;
+    // DIAGNOSTIC, packet cc13_usn15_gunnery: the "death row" line prints
+    // categories 0, 1, 5 and 6 only, so a battleship sunk by HEAVYARTILLERY (4),
+    // MEDIUMARTILLERY (3) or a TORPEDO (7) reads "hits c0..c6=0". ON: a second
+    // line with all twelve categories and the owner of killer_gun, which can
+    // differ from killer (last_attacker is written on every hit, killer_gun only
+    // on a damaging one). No gameplay term reads it.
+    static constexpr bool kDeathRowAllCategoriesBound = false;
     static bool death_table_enabled() {
         static const bool on = [] {
             char* text = nullptr;
@@ -10797,6 +10804,19 @@ void GameGunneryHost::Impl::kill_unit(std::size_t victim) {
             static_cast<double>(nearest_range), dt.hits[0], dt.hits[1], dt.hits[5], dt.hits[6],
             static_cast<double>(dt.damage[0]), static_cast<double>(dt.damage[1]),
             static_cast<double>(dt.damage[5]), static_cast<double>(dt.damage[6]));
+        if constexpr (kDeathRowAllCategoriesBound) {
+            std::string cats;
+            for (int c = 0; c < 12; ++c) {
+                char cell[48];
+                std::snprintf(cell, sizeof cell, " c%d=%d/%.0f", c, dt.hits[c],
+                    static_cast<double>(dt.damage[c]));
+                cats += cell;
+            }
+            const std::string gun_owner = dt.last_category >= 0 && dt.last_gun < guns.size()
+                ? guns[dt.last_gun].unit_name : std::string("-");
+            log.notef("death row categories: victim=%s hits/dmg%s killer_gun_owner=%s",
+                target.row.name.c_str(), cats.c_str(), gun_owner.c_str());
+        }
     }
     done("Death::entity_kill_00926d90", 0x00926d90u);
     if (kUnitInvincibilityFloorBound
```

Prediction when ON: no gameplay counter moves. Each death prints one more line. On USN15 run a,
Mississippi's line shows a non-zero `c4` and `killer_gun_owner=Fuso`, and Portland's shows
`killer_gun_owner=Fletcher-class07`.

## 7. Verification

- `./scripts/build.ps1` in this tree with the diff applied: see the build note below.
- `git apply --check` of the section 6 diff against `origin/main`: see below.
- `reports/cc13_audit_usn15_gunnery.json`, checked with `tools/verify_report_calls.py`.
- No game run (RUN SLOTS rule). No Ghidra write.

Results (2026-10-12): the build passed twice, with the switch OFF and with it flipped ON, both exit 0 with `100% tests passed`. The diff above is the exact `git diff` of the OFF state. After the revert it passed `git apply --check --cached` against `origin/main` `db5ab3c4e`. `verify_report_calls.py` reported 9 call rows checked, 0 failed.

## 8. Open questions for cc9

- The roughly 1500 hp per Fuso hit on Mississippi, against a predicted 2170..2370, was not traced
  through `00826F10`'s segment and part handling. It does not affect the verdicts.
- The US battle line holding at z ~ 1000 after Mississippi (the player's idle unit) leaves the column
  belongs to the ship-AI lane.
- If cc9 wants this row to be a fair US fight, the levers are data or script, not host: the arcade
  `ArcadeTable[25]` damage, `VehicleClass[61].HP`, or a `TorpedoEnable` for the US groups. All are mod
  content in this installation and should be left as they are for a faithful reconstruction.
