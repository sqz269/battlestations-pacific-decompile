# Audit of cc9-lua51's listener flips (cc13-audit11)

Addresses: 009813A0 009816D5 00981869 009818ED 0098191F 00981953 009819AB 00981AA4 00981AC2 00981AE9 00981C31
0096ACE0 0096AD0D 0096AD3B 0096AD6C 00979140 0077CE60 0077CC80 0077CF84 0077CFA3 0077CFC5 0077D0E8 0077D11A 0077D122
00988510 00988530 0098859B 00988706 00988D17 00988D87 00988E2C 00988F5D 0098901A 0098903B 00989060 009892E4 0098952A
007B97E0 00979670 008D0EA0 00884A90 00884AC0 00884A00 00884FC0 00887E50 00E08E58
0046C550 0046CB43 0046CC01 0046CC29 0046CC37 0046C681 0046D3E0 0046D426 0046D60F 0046D655 0046B730 0046B7CA
0046F160 0046F26C 0046F2E1

Packet `cc13-audit11`, branch `agent/cc13-audit11`, 2026-10-12. Requested by the cc13 lead for cc9. **Read only:**
no Ghidra writes, no ledger records, no edits to cc9's files on this branch. Base: `origin/main` at `8ad515e93`.
The three diffs below were applied in this tree, built, and reverted; only this doc and
`reports/cc13_audit_lua51_flips.json` are committed.

**Method.** I read each switch's ON path on `origin/main` (`src/game_hosts_lua.cpp`, `src/game_hosts_gunnery.cpp`,
`src/game_hosts_scene_contents.cpp`, `src/scene_entity_factory.cpp`). I compared it with the decompile and the
listing of the image (`bsp.py ghidra decompile|disasm`, `disasm-raw` for disk bytes). The mission scripts and
scenes were read from this installation (`I:\SteamLibrary\steamapps\common\Battlestations Pacific`). That
installation is modded, so every script cited carries its mtime.

**Checklist.**
- Rule 2 (boundary before attribution): `tools/verify_report_calls.py reports/cc13_audit_lua51_flips.json`
  checked 22 call rows, 0 failed. Each cited call is inside the function it is attributed to.
- Register inputs and stack slots: the kill boxes, the matcher's three results, the hit target and the scene gate's
  last argument were read from the listing, not the decompile. The decompile drops the `IsKindOf` arguments, and
  in `0096ACE0` it drops the `+1Ch` result into `unaff_SI`.
- Positive control for the census script `local/a11_scn_flags.py`: it lists JM06's three `TypeB_Jake` subs and
  JM05's nine actors, the same entities cc9's `l50_G1` logs drop.

**Status vocabulary.** Every switch audited is reconstructed and build-tested on main. cc9 game-validated it with
pairs (5fr, 5fs, 5fu). This audit is a static second reading and adds no runs. The three proposed diffs are
build-tested only.

## Verdicts

| # | switch | verdict | one line |
| --- | --- | --- | --- |
| 1 | `kLuaListenerCallbackArgsBound` / `kLuaKillAttackerBound` | argument order, types, matcher and second pass FAITHFUL; **two DEFECTS in the boxed values** (1a, 1c) | The kill third argument and `lastAttackerPlayerIndex` match use the side, where the image uses the shot's player slot. A squadron victim's attacker sees only wing-0's hits. The hit target is not the squadron |
| 2 | `kLuaHitSquadronSecondPassBound` | FAITHFUL | `+105h` clear, a live attacker, `IsKindOf(0Fh)`, then `vtable[140h]()`. Only the attacker box changes, and it runs after a first pass that called nothing |
| 3 | `kEngineMovieStopRosterBound` | FAITHFUL on JM05/JM06; **latent DEFECT** (3a); 5fr's account of the gate corrected (3b) | `Hidden` admits and `+158h`-exempt classes survive 0046B730 in the image but are dropped by the host. No reference row has one that a script names |

The defect diffs are all OFF and reverted here. Each was built with `./scripts/build.ps1` (pass) and checked with
`git apply --check` against `origin/main` (clean). They are listed after section 3.

## 1. `kLuaListenerCallbackArgsBound` and `kLuaKillAttackerBound` (SQUADRON_LAND_TASK 5fu)

**The kill boxes (`009813A0`), from the listing.**
- `009816CA` `LEA ESI,[EDI+2B0h]` and `009816D5` `CALL 006E6670` copy the victim's handle into a local at
  `[ESP+74h]`. Its pointer is at `+14h` (`[ESP+88h]`). `009816DA` `MOV AX,[ESI+18h]` copies the id word from
  victim `+2C8h`.
- The three boxes are built in this order:
  - the victim's id `+174h` (vtable `00D1AF24`, a 16-bit id);
  - that copied id (`00D1AF24`);
  - the int at victim `+2D8h` (`00D1AF2C`).
- The constructor defaults are `+2C8h = 0` and `+2D8h = -1`. cc9's ESMP03 run found and fixed the
  `-1` default (`0077EFEF`, `0077F012`).
- The host's `kill_last_attacker` and `kill_credited_slot` stand in for `+2C8h` and `+2D8h`.

**The matcher `0096ACE0`, from the listing.**
- It tests `[0]` against `+0Ch` (`0096AD0D`), `[1]` against `+1Ch` (`0096AD3B`, result kept at `[ESP+10h]`) and
  `[2]` against `+2Ch` (`0096AD6C`).
- It returns 1 only when all three hold (`0096AD6E`..`0096AD7B`).
- An empty set answers 1 (`00979140`).
- The host's `evaluate` lambda applies the same three tests, AND-ed. An empty `lastAttacker` or
  `lastAttackerPlayerIndex` set means "any". **FAITHFUL.**

**Arguments.**
- Kill (`009818ED`, `0098191F`, `00981953`, then `00887E50` at `009819AB`): `00884A90(victim)` and `00884AC0(handle)`
  are both entity ids (variant type 3; `00884A90` reads `+174h`, `00884AC0` reads `handle+18h`). `00884A00` is an
  int.
- The host pushes:
  - the victim entity;
  - the attacker entity, or nil for id 0;
  - the slot as a number.
- Every matched callback gets the same three arguments, in the same order. **FAITHFUL**
  for order, types and nil.
- Residual (not settled): when the attacker id is non-zero but the entity no longer has a `thisTable`
  entry, the host pushes nil. I did not read how `00887E50` pushes a type-3 id that no longer resolves.
- Hit (`00988D87`..`00988F2C`, call `00988F5D`): eight boxes in this order:
  1. target;
  2. targetDevice;
  3. attacker (the `00D1AF24` id of the handle `00988706` assigns from the shot source `+98h`);
  4. attackType (`00884FC0`, a string from `00E08E58[kind]`);
  5. attackerPlayerIndex (`00D1AF2C`, `[src+1Ch]`, or -1 with no source);
  6. damage (`00D1AF3C` floats, record `+48h`, the applied amount);
  7. fire (`+4Ch`);
  8. leak (`+50h`).
- The host pushes the same eight, in the same order and with the same types. The name table matches the PE.
  `hit.damage` is the applied amount (`HitRecordFill::applied_damage`, `+48h`). **FAITHFUL**, except for the
  target (1c) and the substitutions cc9 already labels: targetDevice is nil, and the kamikaze's attackType `11h`
  is not modelled.

**The kill second pass (`00981AA4`..`00981C31`).**
- The gate checks, in order:
  - `00981A5B`: `+104h` clear. Only `luaMW_OverrideMessage` `008D0EA0` -> `00979670` sets it, at `009796BF`.
  - `00981AA4`: `+105h` clear. It is set at `009819DB` only after a non-empty callback list was called.
  - `00981ABA`: the handle's pointer is non-null.
  - `00981AC7`: `IsKindOf(0Fh)`.
- `00981AE9` re-points the handle at `vtable[140h]()`. `00981C31` evaluates again with the new id. The victim and
  `+2D8h` boxes are rebuilt from the same values, and `00981D6C` calls each match.
- The host's second pass:
  - runs when the first pass collected nothing;
  - replaces the attacker with its registry squadron (`007B97E0` is `plane+9D4h`);
  - keeps the victim and the slot;
  - pushes the squadron as argument 2.
- **FAITHFUL.** Two residuals:
  - The host does not test that the attacker object still exists. The image's weak reference is null once the
    attacker is deleted (`0077CC80` re-registers through `006952A0`/`00694A60`).
  - `+104h` cannot be set when the first pass called nothing, so leaving it out changes nothing.

### 1a. DEFECT: the kill's `+2D8h` (argument 3 and the `lastAttackerPlayerIndex` match) is the side, not the shot's player slot

**Image (`0077CE60`, listing).**
- `0077CFA3` loads `attacker+180h` into EBX. `0077CFB7` `CMP EBX,8` / `JBE` keeps it when it is 8 or below.
  Otherwise `0077CFC2` takes `[src+1Ch]`, and `0077CFC5` stores the result to victim `+2D8h`.
- `unit+180h` is 9 for every unit whose scene does not author `OwnerPlayer`
  (`docs/AI_BRAIN_PLAYER_EXEMPTION.md`). So `+2D8h` is in practice the shot's team stamp `[src+1Ch]`.
- `[src+1Ch]` is the gun's seat `gun+1ACh` (`0072C0FB`; `shot_team_id_0072c0f2`): the player's slot (0, `PLAYER_1`)
  on a gun the player's seat holds, 8 (`PLAYER_AI`) on an AI gun.

**Host.**
- `kill_credited_slot` returns `attribution.credited_slot`. `apply_hit` (`src/game_hosts_gunnery.cpp`) fills that
  with `attacker.owning_slot = units.unit_side_0054(shooter)`: 0 for a US unit, 1 for a Japanese unit.
- The hit's own `attacker_player_index` already carries `team_id` (`[src+1Ch]`), but the kill box does not use it.

**Effect.** Every kill subscription with `lastAttackerPlayerIndex = {PLAYER_1}` matches kills by any US unit and
never matches kills by a Japanese unit. In the image it matches only kills made by guns the player's seat holds.
- **JM01** (menu row, scene `IJN/JM/ijn_01_attack_on_pearl_harbor.scn`; `scripts/missions/ijn/JM/jm01.lua`, mtime
  2024-07-13 08:26:24):
  - `PlaneSquadronLost` (`AddListener` at 2909, `{PLAYER_1}` at 2913) calls
    `luaJM1PlaneSquadronLost(entity, lastAttacker, lastAttackerPlayerIndex)` (2918). That callback counts allied
    fighter squadrons into `Mission.ShootedDownPlanes`, and `== 5` completes primary objective 4 (1479).
  - The player is Japanese, so on the host the count rises only on an allied fighter killed by a US unit (side
    0, friendly fire). In practice **primary 4 cannot complete**.
  - `DonaldListener` (2875, `{PLAYER_1}` 2879) sets `DonaldKilledByPlayer` (2885), which with `HenryKilledByPlayer`
    gates `SetAchievements("SA_PM")` (1324). It is likewise dead on the host.
- **LOMP05** (`scripts/missions/usn/LOMP/05_santa_cruz.lua`, mtime 2024-07-13 08:26:30): `Listener_JPPlaneShotdown`
  (`{PLAYER_1}`, 1246) counts Japanese planes in `luaJPPlaneShotDown` (1171). At 50 it completes hidden 1 and adds
  a power-up. The player is US, so on the host every US gun's kill counts; in the image only the player's own do.
  The host over-fires.
- **JM06** (`COTP-IJN/PRCPIJN/jm06.lua`, mtime 2024-07-13 08:26:30): `CVKill` (1761, `{PLAYER_1}` 1765) only removes
  itself (1769), so there is no gameplay effect.

**Second part: the squadron victim.**
- `0077D0E8`..`0077D128`: when `victim->vtable[140h]()` is a different unit that answers `IsKindOf(2)` (a plane's
  squadron), `0077D11A` `CALL 0077CC80` re-points the parent's `+2B0h` handle at the same shot source. `0077D122`
  stores `[src+1Ch]` into the parent's `+2D8h`.
- So a squadron's kill box names whoever last hit **any** of its planes.
- The host's squadron slot is fused with wing 0, and only hits on wing 0 write it. When the last plane to die
  is not wing 0, the squadron's kill carries a stale attacker, or none.
- JM01's `PlaneSquadronLost` keys on squadron victims (`entity.Name` with `.|`), so the fix needs both parts.

**Proposed fix**: diff A, `kLuaKillBoxFromHitBound` (OFF), in `src/game_hosts_gunnery.cpp`.
- It adds a Lua-only pair `lua_kill_attacker` / `lua_kill_slot` per unit, which `apply_hit` writes as `0077CE60`
  does. The slot is the shot's `team_id`. The squadron (registry `find_by_member_unit`) is written too.
- Both accessors read the pair when the switch is ON.
- Scoring keeps `attribution.credited_slot` (LABELLED: the image uses one field for both; scoring's side stand-in
  is a separate question).
- `unit+180h` is taken as 9 (LABELLED). An authored `OwnerPlayer` of 8 or below is not consulted, because the
  gunnery host cannot reach the AI host's table.
- Torpedo and depth-charge stamps stay -1 (cc9's existing label).

**Predicted pairs** (not run):
- JM01: `ShootedDownPlanes` rises once the player's own plane shoots down allied fighters, and primary 4
  can complete. Expect exit 3.
- LOMP05: the hidden-1 count rises only on the player unit's kills, so it rises more slowly than OFF. Expect exit
  3 if it reached 50 OFF, else 0/1.
- JM06, BSM01, BSM04, ESMP03, ESMP08: identical. None keys a kill on `PLAYER_1` with a gameplay effect, and the
  attacker sets compare ids, which are unchanged except on squadron victims.

### 1b. The hit's `attackerPlayerIndex`: FAITHFUL

The hit pushes `hit.attacker_player_index`. That is the same `[src+1Ch]` stamp (`team_id`) the image boxes at
`uVar14` (`[src+1Ch]`, or -1 with no source). No change is needed.

### 1c. DEFECT: the hit target is the hit plane, not its squadron

- **Image.** `00988530` calls `hitEntity->vtable[140h]()`. `0098857F` `PUSH 2` / `0098858A` `CALL` (IsKindOf) and
  `0098858C`..`00988592` (`NEG`/`SBB`/`AND`) keep it only when it answers `IsKindOf(2)`. `0098859B` stores it at
  `[ESP+38h]`, and `00988D87` boxes that slot (`[ESP+34h]`, one push later) as argument 1. The same box is what
  `0097B8C0` matches against `target`. For a plane, the target is its squadron.
- **Host.** `victim = hit.victim + 1` is used for both the match and the push. cc9 labels this ("the hit target is
  the hit unit").
- **Effect.**
  - A `target = {squadron}` subscription matches only hits on wing 0 (the fused slot). Hits on the other
    planes are missed.
  - `bsm/bsm_04_vengance_at_luzon.lua` (mtime 2024-07-13 08:26:36): `FortressHitListener` (1598, `target =
    {Mission.FortressRed}`, `FindEntity("B-17")` at 395) plays `UNDERATTACK` on the first hit (1644). On the
    host that dialog waits until wing 0 is hit.
- **Proposed fix**: diff B, `kLuaHitTargetSquadronBound` (OFF), in `src/game_hosts_lua.cpp`. It maps a member's
  hit to its registry squadron for the match and the push. The rate limiter's key is left on the hit unit (not
  re-read).
- **Predicted**: BSM04 moves only when the B-17 squadron has more than one plane and a non-wing-0 plane is hit
  first. Rows whose hit subscriptions key on ships are identical.

## 2. `kLuaHitSquadronSecondPassBound` (SQUADRON_LAND_TASK 5fs)

**Image (`00988510`, listing).**
- `0098901A` checks `+105h` clear (set at `00988F88` only after a non-empty callback list was called). It is
  preceded by the `+104h` test at `00989012`.
- `[ESP+B0h]` is the attacker handle's pointer. It must be non-null, and `0098903B` `PUSH 0Fh` / `CALL` checks
  IsKindOf(0Fh).
- `00989047`..`00989060` call `vtable[140h]()` and re-point the handle. `009892E4` evaluates again.
- The eight boxes are rebuilt from the same target, type, slot and floats, so only the attacker id changes.
  `0098952A` calls each match.

**Who is matched and what is pushed.**
- The attacker set is matched against the squadron's id. The target, type, player index and ranges are as in
  the first pass.
- The pushed argument 3 is the squadron. The host (`boxed_attacker`) does the same.

**Ordering.**
- In the image, the first pass's callbacks (none, since `+105h` is clear) run before the second evaluation. The
  host collects the first pass, evaluates the second only when it collected nothing, then calls.
- The observable order is therefore the same.
- Skipping the wing-0 plane (`r.squadron_unit == hit.shooter`) is equivalent: its `vtable[140h]()` id is its own
  fused slot, so a re-evaluation could not match anything new.

**Verdict: FAITHFUL.** It has the same weak-pointer residual as the kill (a deleted shooter skips the image's
pass). A shell in flight after its plane is deleted is the only case that reaches it.

**Scripts** (5fs census):
- `PRCPIJN/jm13.lua` and `prcpjm13.lua` (`luaDonaldHit`, attacker `Mission.Fighters`);
- `bsm_04` (`Mission.FortressRed`);
- `bsm_08` (`Mission.Fortress`);
- `usn_1_pearl`;
- `jm06` (`Mission.PlayerUnit`, a submarine, which never reaches the pass).

JM06's `cvhit` subscription (`PRCPIJN/jm06.lua` 1506..1516, `target = {Mission.Lex}`,
`attacker = {Mission.PlayerUnit}`) needs `arg[4] == "TORPEDO"` (1524). It works only under
`kLuaListenerCallbackArgsBound`. On the host the torpedo's `ordnance_kind` is `bullet_sub_type 0x0A` (the
`== 0x0A` torpedo test at `src/game_hosts_gunnery.cpp` near the throw counter), so `00E08E58[10]` = `"TORPEDO"`.
`luaJM6FleetAmbushed` (2051..2056) stores `arg[1]`, the victim.

## 3. `kEngineMovieStopRosterBound` (SQUADRON_LAND_TASK 5fr; cc9's lane, read only)

### 3b. Correction to 5fr's account of the gate (no host change)

`0046C550`'s mode-9 arm, from the listing (`0046CB43`..`0046CC58`, `RET 5Ch`, the flag is the last argument at
`[ESP+D0h]`):
- `GenerateInGame` true, flag **clear**:
  - it builds the 5Ch record (`0046CB9C`..`0046CBDD`), appends it to the vector at `this+24h` (`0046CC01`) and
    registers the stock (`0046CC10`);
  - then `0046CC29` tests the flag again and **falls through** to `0046CC37`, returning
    `GenerateInEngineMovie`.
- `GenerateInGame` true, flag **set**: no record (`0046CB9A` `JNE 0046CC06`). It registers the stock and returns 1
  through `0046C681`.
- `GenerateInGame` false: it returns `GenerateInEngineMovie`.
- The callers: `0046D426` passes 0 (`PUSH 0` at `0046D3E0`, an entity that is not `Hidden`), and `0046D655` passes 1
  (`PUSH 1` at `0046D60F`, reached when `Hidden` (`00CE5708`) is set).

So during the movie the image creates only the `GenerateInEngineMovie` entities. `0046B730` then kills every
non-exempt root and re-creates the `GenerateInGame` ones from their records.
- 5fr's "flag clear ... returns 1" describes the Hidden pass.
- The roster after the stop is the same as the host's (it creates the `GenerateInGame` entities at load and, under
  the switch, drops the rest). The difference is confined to the movie window, which the host skips.
- **FAITHFUL** for the post-stop roster on JM05/JM06. `local/a11_scn_flags.py` over the two scenes lists exactly
  the 9 + 3 entities the `l50_G1` logs drop, all non-hidden roots of non-exempt classes.

### 3a. Latent DEFECT: two admits the stop does not remove

1. **`Hidden` entities.**
   - A hidden entity reaches the gate with flag 1 (`0046D655`). With `GenerateInGame` false the gate returns
     `GenerateInEngineMovie` (default `B true`, `universe/library/global.enums` 1774, `properties MultiEntity`).
   - The entity is then registered for `GenerateObject` and never created. `0046B730` has nothing to kill, and the
     script can spawn it after the movie.
   - The host's drop runs before its Hidden test (`src/game_hosts_scene_contents.cpp`, the `return` before
     `scene_spawn_pool().add`), so such an entity never reaches the spawn pool and `GenerateObject` fails.
   - Census of the 33 `.ema` scenes that have a `.scn` (`local/a11_scn_flags.py`):
     - `COTP-IJN/PRCPIJN/ijn_14_invasion_of_havaii.scn` (JM16) `Yorktown-class 01` (4218);
     - `COTP-USN/PRCPUS/usn_04_Defend_Guadalcanal.scn` six `US Jeep`/`US Truck` children (4751..10871).
   - No mission script names them (`PRCPIJN/jm14.lua` mentions `Yorktown-class 01` only in comments, 515 and
     558), so this is inert today.
2. **The `+158h` exemption.**
   - `0046F160` fills the set with 44h, 5Eh, 1Dh, 47h and 3Dh (`0046F26C`..`0046F2E1`, `EDI` = `this+158h` from
     `0046F205`).
   - `0046B730` (`0046B7CA`) queues only roots outside the set, and of an exempt root only its non-exempt children.
   - 47h and 3Dh are also always generated (`+164h`), so the reachable cases are `LandingPoint` (1Dh) and
     `Landscape` (44h). Neither appears as a fallback admit in the census.

**Proposed fix**: diff C, `kEngineMovieStopRosterExemptBound` (OFF). It keeps `Hidden` entities and exempt-class
roots, or exempt children of exempt roots, out of the drop. LABELLED: a grandchild is judged by its parent's class
only.

**Predicted**: JM05, JM06 and USNOS identical. A JM16 or PRCP USN04 load would keep `Yorktown-class 01` / the
jeeps in the spawn pool, with no script effect.

## Diffs

Diffs A, B and C form one patch, written against `8ad515e93`. `git apply --cached --check` against an index read
from `origin/main` `f4e163a9e` is clean. With all three applied, `./scripts/build.ps1` passed in this tree (build
plus the two ctest suites). They were then reverted. Diff A is the `src/game_hosts_gunnery.cpp` hunks, diff B the
`game_hosts_lua` hunks and diff C the `game_hosts_scene_contents` hunks.

```diff
diff --git a/include/bsp/game_hosts_lua.hpp b/include/bsp/game_hosts_lua.hpp
index 00c3ba0b3..5ef682381 100644
--- a/include/bsp/game_hosts_lua.hpp
+++ b/include/bsp/game_hosts_lua.hpp
@@ -234,6 +234,13 @@ inline constexpr bool kLuaListenerCallbackArgsBound = true;
 // attacker sets name lands a kill in them.
 inline constexpr bool kLuaKillAttackerBound = true;
 
+// docs/CC13_AUDIT_LUA51_FLIPS.md item 1c. 00988510 boxes, and 0097B8C0 matches,
+// the target as hitEntity->vtable[140h]() kept only when it answers IsKindOf(2)
+// (00988530..0098859B): for a plane that is its squadron (007B97E0, plane+9D4h).
+// True: a hit on a squadron member is reported with the squadron as target.
+// False: the hit unit, as before.
+inline constexpr bool kLuaHitTargetSquadronBound = false;
+
 // Packet cc9_lua_aa_enable (docs/LUA_BINDING_MISSION.md, "AAEnable, 0089C740").
 // AAEnable(entity, flag): director = entity->vtable[114h](); when it exists,
 // 0071E050 sends session message 5Ah sub-kind 4 with the flag, which the director
diff --git a/include/bsp/game_hosts_scene_contents.hpp b/include/bsp/game_hosts_scene_contents.hpp
index e94ffc72c..b0612d152 100644
--- a/include/bsp/game_hosts_scene_contents.hpp
+++ b/include/bsp/game_hosts_scene_contents.hpp
@@ -454,6 +454,16 @@ inline constexpr bool kEngineMovieEntryBound = true;
 // three TypeB_Jake subs no script names; USNOS identical.
 inline constexpr bool kEngineMovieStopRosterBound = true;
 
+// docs/CC13_AUDIT_LUA51_FLIPS.md item 3. Two admits the stop does not remove:
+// a `Hidden` entity goes through 0046C550 with the last argument 1 (0046D60F) and
+// is registered for GenerateObject, never created, so 0046B730 has nothing to
+// kill; and 0046B730 queues only roots whose class is outside the scene
+// database's +158h set (44h, 5Eh, 1Dh, 47h, 3Dh; 0046F26C-0046F2E1), and of an
+// exempt root only the children outside it (0046B7CA). True, with
+// kEngineMovieStopRosterBound: those entities are not dropped. False: every
+// GenerateInEngineMovie admit is dropped, as before.
+inline constexpr bool kEngineMovieStopRosterExemptBound = false;
+
 // Packet cc9_shipyard_scene_attach (GUNNERY 136.2, the scene half routed to this lane;
 // docs/SQUADRON_LAND_TASK.md 5ej). True: create_units reads a Shipyard row's bag
 // (`NumSlots`, "Stock 1".."Stock 12", "Hangar 1".."Hangar 12", "Slot 1".."Slot n") into
diff --git a/src/game_hosts_gunnery.cpp b/src/game_hosts_gunnery.cpp
index f66d4a260..90a3f86ef 100644
--- a/src/game_hosts_gunnery.cpp
+++ b/src/game_hosts_gunnery.cpp
@@ -1144,6 +1144,15 @@ constexpr bool kReconAggregatesBound = true;
 //    wrote it on every hit, so a zero-damage hit could take the credit. OFF:
 //    every hit. Packet cc9_kill_credit, docs/KILL_CREDIT.md.
 constexpr bool kKillCreditDamageGateBound = true;
+//  * kLuaKillBoxFromHitBound: the `kill` channel boxes victim+2C8h and +2D8h
+//    (009813A0) as 0077CE60 writes them. +2D8h is attacker+180h when 8 or below,
+//    else the shot's [src+1Ch] (0077CFA3-0077CFC5); when the victim's
+//    vtable[140h]() is another unit answering IsKindOf(2) (a plane's squadron),
+//    that unit gets the same attacker (0077D114) and [src+1Ch] (0077D0E8-0077D128).
+//    LABELLED: unit+180h is taken as 9 (unauthored), so +2D8h is the shot's team
+//    stamp; scoring keeps attribution.credited_slot. OFF: the kill box reads
+//    last_attacker and the side stand-in. docs/CC13_AUDIT_LUA51_FLIPS.md item 1a.
+constexpr bool kLuaKillBoxFromHitBound = false;
 //  * kPlaneNullFireTargetProviderBound: 008636A0 (called once, 00864C18, from
 //    the attach 00864BD0) installs the director-backed fire-target provider
 //    00D0D324 (slot +4h 00863640, director+238h) only when the unit answers
@@ -1449,6 +1458,9 @@ struct GameGunneryHost::Impl {
         // The kill attribution block at victim+2C4h..+2E8h, as 0077CE60 leaves it.
         bsp::KillAttributionFields attribution{};
         std::size_t last_attacker{0};   // victim+2C4h, one based
+        // kLuaKillBoxFromHitBound: +2C8h (one based) and +2D8h as 009813A0 boxes them.
+        std::size_t lua_kill_attacker{0};
+        int lua_kill_slot{-1};
         // The per-step scratch the pass fills.
         std::vector<bsp::GunneryCandidate> candidates;
         std::vector<int> order;
@@ -9807,6 +9819,18 @@ void GameGunneryHost::Impl::apply_hit(std::size_t shooter, std::size_t gun_row,
         target.attribution = bsp::kill_attribution_0077ce60(target.attribution, true, source,
             attacker, kamikaze, true);
         target.last_attacker = shooter + 1;
+        if (kLuaKillBoxFromHitBound) {
+            target.lua_kill_attacker = shooter + 1;
+            target.lua_kill_slot = team_id;   // +180h = 9, so [src+1Ch]
+            if (const bsp::PlaneSquadronHostRecord* sqn =
+                    bsp::plane_squadron_registry().find_by_member_unit(victim)) {
+                if (sqn->squadron_unit != bsp::kPlaneSquadronNoUnit && sqn->squadron_unit != victim
+                    && sqn->squadron_unit < unit_state.size()) {
+                    unit_state[sqn->squadron_unit].lua_kill_attacker = shooter + 1;
+                    unit_state[sqn->squadron_unit].lua_kill_slot = team_id;
+                }
+            }
+        }
     }
     ++summary.attributions;
     {
@@ -12252,11 +12276,13 @@ bool GameGunneryHost::unit_dead(std::size_t unit_index) const noexcept {
 
 std::size_t GameGunneryHost::kill_last_attacker(std::size_t unit_index) const noexcept {
     if (unit_index >= impl_->unit_state.size()) return 0;
+    if (kLuaKillBoxFromHitBound) return impl_->unit_state[unit_index].lua_kill_attacker;
     return impl_->unit_state[unit_index].last_attacker;
 }
 
 int GameGunneryHost::kill_credited_slot(std::size_t unit_index) const noexcept {
     if (unit_index >= impl_->unit_state.size()) return 0;
+    if (kLuaKillBoxFromHitBound) return impl_->unit_state[unit_index].lua_kill_slot;
     return impl_->unit_state[unit_index].attribution.credited_slot;
 }
 
@@ -12302,6 +12328,8 @@ bool GameGunneryHost::revive_unit_garrison_006f3660(std::size_t unit_index) {
     s.health = s.max_health;
     s.row.health = s.health;
     s.last_attacker = 0;
+    s.lua_kill_attacker = 0;
+    s.lua_kill_slot = -1;
     s.fire_target = 0;
     s.command_target = 0;
     s.attribution = {};
diff --git a/src/game_hosts_lua.cpp b/src/game_hosts_lua.cpp
index 7272a6905..adf0016fd 100644
--- a/src/game_hosts_lua.cpp
+++ b/src/game_hosts_lua.cpp
@@ -6193,7 +6193,16 @@ void GameMissionLuaHost::dispatch_hit_listeners_00988510() {
             next = static_cast<float>(clock + (quick ? 9.99999974737875e-05 : 2.0));
             ++summary_.listener_hit_throttle_passed;
         }
-        const int victim = static_cast<int>(hit.victim + 1);
+        int victim = static_cast<int>(hit.victim + 1);
+        if (kLuaHitTargetSquadronBound) {
+            // 00988530..0098859B: target = hitEntity->vtable[140h](), a plane's squadron.
+            if (const bsp::PlaneSquadronHostRecord* sqn =
+                    bsp::plane_squadron_registry().find_by_member_unit(hit.victim)) {
+                if (sqn->squadron_unit != bsp::kPlaneSquadronNoUnit) {
+                    victim = static_cast<int>(sqn->squadron_unit + 1);
+                }
+            }
+        }
         const GameBulletClassRow* bullet = gunnery->bullet_class_row(hit.bullet_class);
         const std::string type = bullet != nullptr ? bullet->type : std::string();
         std::vector<std::string> callbacks;
diff --git a/src/game_hosts_scene_contents.cpp b/src/game_hosts_scene_contents.cpp
index a606ca63b..e0c754f94 100644
--- a/src/game_hosts_scene_contents.cpp
+++ b/src/game_hosts_scene_contents.cpp
@@ -1186,6 +1186,7 @@ private:
     struct AuthoredParent {
         std::size_t scene_id{0};
         std::string name;
+        std::string class_name;
         std::array<float, 16> world{};
     };
     // visit_entity calls a parent before its children while their addresses
@@ -1586,6 +1587,7 @@ void SceneReaderBinding::instantiate_entity(const SceneEntity& entity,
     AuthoredParent child_parent;
     child_parent.scene_id = scene_id;
     child_parent.name = entity.name;
+    child_parent.class_name = entity.class_name;
     std::copy_n(world_frame, child_parent.world.size(), child_parent.world.begin());
     for (const SceneEntity& child : entity.children) {
         authored_parents_[&child] = child_parent;
@@ -1915,7 +1917,22 @@ void SceneReaderBinding::instantiate_entity(const SceneEntity& entity,
     // only from the records, so it is not in the mission. LABELLED: with the
     // skip on the first frame (kEngineMovieEntryBound) it is not created at all
     // instead of created and killed; the registration pass is left as it was.
+    bool survives_stop = false;
+    if (bsp::game::kEngineMovieStopRosterExemptBound) {
+        // 0046F26C-0046F2E1: the +158h set; 0046B7CA tests a root, then an
+        // exempt root's children.
+        auto exempt = [](const std::string& class_name) {
+            const SceneEntityClassRow* row = find_scene_entity_class_by_name(class_name);
+            if (row == nullptr) return false;
+            const int id = static_cast<int>(row->class_id);
+            return id == 0x44 || id == 0x5E || id == 0x1D || id == 0x47 || id == 0x3D;
+        };
+        survives_stop = hidden_property_bool(bag.find(bsp::kSceneHiddenPropertyKey))
+            || (exempt(entity.class_name)
+                && (authored_parent.scene_id == 0 || exempt(authored_parent.class_name)));
+    }
     if (bsp::game::kEngineMovieEntryBound && bsp::game::kEngineMovieStopRosterBound
+            && !survives_stop
             && gate.generate && gate.rule == SceneGateRule::EngineMovieFallback) {
         gate.generate = false;
         record.generated = false;
```

## Summary for cc9

- Item 1's mechanism (argument order and types, nil for a missing attacker, the AND-ed matcher, the second pass)
  is the image's.
- **1a is a gameplay defect on a menu row.** JM01's primary objective 4 (`ShootedDownPlanes == 5`, `{PLAYER_1}`
  kills) cannot complete on the host, and LOMP05's hidden-1 count over-fires. Diff A is the fix. Run JM01 and
  LOMP05 OFF/ON pairs.
- 1c (hit target = squadron) is a dialog-timing defect on BSM04. Diff B.
- Item 2 is faithful.
- Item 3's roster is faithful on JM05/JM06. 3a is latent, with diff C. 3b corrects the 5fr text: with flag 0 the
  `GenerateInGame` arm returns `GenerateInEngineMovie`; the "returns 1" path is the `Hidden` pass.
