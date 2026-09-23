# Pilot skill level: the row USN04's Vals fly, the chain that sets it, and the binding plan

Addresses: 00895250, 0089539A, 006D1EB0, 00D05F20, 00D05ED8, 007CFDAA, 007B8AE0, 0072BC80, 0080DF90, 009565A0, 0095CCCC, 00952930, 00927A80, 007D5D20, 007D65AA, 007D66F0, 007D670C, 008FBC80, 009F9D22, 0058BDF0, 0058BF26, 0058BF58, 0051B7B0, 007FDB20, 008AE030

Packet `cc9_skill_level`, 2026-09-22. It follows `docs/TURNDOWN_HEADING.md` section 2. The report is
`reports/pilot_skill_level.json`. All names are hypotheses, not recovered symbols.

**Status: HANDOFF.** Steps 1-3 are read and settled. Step 4, the binding and the runs, is
**not done**. By the lead's rule, this worker's context was judged past the budget for a code change
plus four builds and runs, so section 5 is a binding plan with its predictions, for the next worker.
No source was changed.

**The answer.** Under the image's rules, as this host launches USN04 from a fresh session, the
difficulty is **1**. Every Val, the 12 launched strikers and the scene-authored movieval flight,
flies **PilotBot row 1, SPNormal**. That is the row this host already uses: aimdive gains
0.018/0.025, power 0.7→0.2, brake 0.0→0.5. **So the row does not reopen the aimdive-tail or hull
switches.** What differs is the host's difficulty, 0 instead of 1. That sends usn_19_coralus.lua down
its difficulty-0 branches, giving a different striker schedule and plane types and Elite US ships.
The next worker should bind that.

## 1. The leaf: SetSkillLevel does reach the pilot bot

`SetSkillLevel` (00895250) calls `entity->vtable[128h](level)` at 0089539A.

* **The plane's vtable.** The plane unit's vtable is 00D05F20. Slot +128h (00D06048) is **006D1EB0**:
  `ECX += 38Ch; JMP [[unit+38Ch]+0]`, a tail jump into the skill sub-object at unit+38Ch. Slot +12Ch
  (00D0604C) is 006D1EC0, `return unit+390h`.
* **The plane's sub-object.** `BSP_PlaneUnitInstance_Construct` installs vtable 00D05ED8 at
  unit+38Ch (007CFDAA). Its slot 0 is **007B8AE0**, `__thiscall(sub = unit+38Ch, level)` with RET 4:
  - If `[sub+A68h]`, which is **unit+DF4h, the pilot bot**, is non-null, it calls
    `vtable[18h](level)`. That slot (00D1F360) is **0072BC80 `BSP_GunBot_SetSkillIndex`**:
    `bot+34h = level`.
  - It then calls **009565A0**. That routine skips when the byte at unit+5Dh is set. Otherwise it
    stores unit+390h = level (009565B9), calls `vtable[18h](level)` on unit+6DCh (the 558h weapon
    director), and calls `vtable[1C4h](level)` on each IsKindOf(20h) child from unit+48h.
  - **There is no clamp anywhere in the chain.**
* **The two base classes.** The game-object base (0095CC90) installs vtable 00D1A578, whose slot 0,
  00952930, only stores sub+4. It seeds unit+390h = 1 at 0095CCCC. The unit-instance class (006FE460)
  installs 00CFC388, whose slot 0 is 0080DF90. That is the ship-side twin, which re-skills unit+740h
  instead of a pilot bot.

## 2. The capture order: the approach sees the launch skill, and the script re-applies the same value

* **At spawn.** `BSP_Plane_ReadPropertyBag` (007D5D20) reads the bag's `Skill` through 00927A80 at
  007D65AA. That routine reads key 00CF8838 `Skill`, then falls back to key 00D19264, then 1. The
  result goes straight to the plane's own `vtable[128h]` at 007D65B5. unit+DF4h is still null, so this
  stores unit+390h only. Then the pilot bot is allocated at 007D66F0, and its attach at 007D670C
  reaches `BSP_GunBot_Attach` (008FBC80), which sets bot+34h from `unit->vtable[12Ch]()`, i.e.
  unit+390h. **So the pilot bot starts at the launch `Skill`.**
* **The launch bag.** For a carrier launch, `Skill = owner->vtable[12Ch]()`, the carrier's own skill
  (`BSP_MCatapult_Fire` 006EC98D, `docs/AIR_OPERATIONS.md`).
* **The carriers.** usn_19_coralus.lua puts Zuikaku and Shokaku in `Mission.IJNMainFleet`
  (lines 1710-1730) and calls `SetSkillLevel(unit, Mission.SkillLevel)` on that fleet (line 1760),
  before any launch. Those are ships, so the call goes through 0080DF90 and 009565A0, and
  carrier+390h becomes `Mission.SkillLevel`.
* **The strikers.** After each launch the script calls `PilotSetTarget` and then
  `SetSkillLevel(launchedStriker, Mission.SkillLevel)` (lines 1412-1595). For a squadron that fans
  out to every member's `vtable[128h]` (007ECF80), and 007B8AE0 re-skills the pilot bot.
* **The capture.** The approach captures its row once, at 009F9D22, from bot+34h. **Whether that
  happens before or after the script's call does not matter**, because both values are
  `Mission.SkillLevel`.
* **movieval** is authored in the scene (`docs/DIVE_BOMB_FLYOVER_FLAGS.md` §8; log line 891). Its
  skill is the scene's `Skill` property if authored, else the fallback key, else 1. The host does not
  log a scene `Skill` for it. **This one row is unverified.** The default is SPNormal.

## 3. The difficulty

* **Where it comes from.** `GetDifficulty` (008AE030) returns game+6ACh, the effective difficulty
  (`docs/USN04_STRIKE_CLASS.md`).
* **Who writes it.** `BSP_MainMenu_StartSelectedMission` (0058BDF0) writes game+6ACh at 0058BF26:
  - `record+0B4h` directly (0058BF37), unless that is 3.
  - When it is 3, **the player's choice** game+6B0h, stored at 0058BF58, provided the main-menu
    byte at `[[00E198AC]+58h]+5Ch` is clear.
  - **This host's log records USN04's record with `difficulty=3`** (log line 554).
* **The player's choice.** game+6B0h is profile+60h (`SelectedDifficulty`), and game+6ACh is
  profile+5Ch (`Difficulty`), with the profile at game+650h. The briefing's play action 0051B7B0
  writes 0, 1 or 2 into both from the three difficulty widgets.
* **With no profile.** The reset 007FDB20 (`docs/GAME_PROFILE_RESET.md`: 007FDFE2 and 007FE01B)
  sets **both to 1**.
* **This installation's profile.** It has one at
  `C:\Users\sqz269\Documents\Battlestations-Pacific\save\0\player` (2182 bytes, 2026-09-18).
  - It is encoded, so its fields cannot be read as text.
  - This host does not load it. `TitleInit::reset_player_profile` 007FDB20 is UNIMPLEMENTED (log
    line 224), and `chosen_difficulty()` returns 0.
  - The fresh-session value the image applies is therefore **1**. With a signed-in profile it would
    be that profile's `SelectedDifficulty`, which is unreadable here.
* **The script's mapping** (lines 68-82):

| difficulty | Mission.SkillLevel (Japanese) | Mission.SkillLevelOwn (US) |
|---|---|---|
| 0 | Stun | Elite |
| **1 (the image's)** | **SPNormal** | **SPVeteran** |
| 2 | SPVeteran | SPNormal |

  The strike launches are gated on the same value (lines 159, 198, 492, 569, 1382 on). Difficulty 0
  launches its own branch; for example, the 1382 branch launches Zero type 150 escorts first.
* **The host's value.** It returns **0**. So this host has been flying USN04's difficulty-0 mission:
  the difficulty-0 launch branches, and US ships at Elite for their gun and director bots. The Vals'
  PilotBot row is unaffected only because the host ignores `SetSkillLevel` and hard-codes SPNormal.

## 4. Host against image

| term | image | host |
|---|---|---|
| effective difficulty (game+6ACh) | 1 on a fresh session (record 3, profile default 1) | 0: `chosen_difficulty()` returns 0 and the store is a no-op |
| GetDifficulty | 1 | 0 |
| SetSkillLevel on a plane | pilot bot+34h, unit+390h, director, children | recorded in `row_->skill_level`, not applied |
| launch Skill | carrier+390h into the bag, then unit+390h, then bot+34h at attach | not modelled |
| approach row | `[00F8A30C] + bot+34h x 248h + 0Ch` at 009F9D22 | SPNormal constants |
| Vals' row | SPNormal (1) | SPNormal |

## 5. Binding plan for the next worker, with predictions

1. **`src/game_hosts_mission.cpp`, around line 933.**
   - `chosen_difficulty()` returns **1**: the profile-reset default of 007FDB20, since the host
     loads no profile. Label it as that substitution.
   - `set_effective_difficulty(v)` stores v into a store the script host can read. game+6ACh is one
     global; a small accessor declared in `include/bsp/game_hosts_script_orders.hpp` suits both
     hosts.
2. **`src/game_hosts_script_orders.cpp:148`.** `game_effective_difficulty()` returns the stored
   value instead of 0.
3. **`entity_set_skill_level` (line 1382).** Resolve the entity to its units-host slot and set a
   new `skill_level` field (default 1, per 0095CCCC). For planes, apply it immediately; that is
   007B8AE0's effect on the pilot bot's index.
4. **`src/game_hosts_units.cpp`.** A slot field `pilot_skill_index` (default 1). The dive-bomb
   approach captures it at the arm's construction point, the equivalent of 009F9D22. The aimdive
   gains and the tail's power and brake then select the PilotBot row by that index, from
   robots.lua's six blocks. Keep them as authored constants per row, as the host does for SPNormal
   now; `src/robot_config.cpp` has a reader, but it is not wired to the units host. Everything goes
   behind `kSkillLevelBound`, default true, with the old behaviour retained.
5. **Gunnery.** Nothing needed. The gun bots' skill (BSP_GunBot_Attach's host counterpart in
   src/game_hosts_gunnery.cpp) would read the same slot field. If the next worker wants the US
   ships' SPVeteran to reach AA and gunnery, that is a one-line call there: `skill index =
   units_host.slot_skill_level(unit)`. It is the lead's hunk, not this packet's.

**Predictions for the treatment** (USN04, 4800 frames):
- **The strike schedule changes.** GetDifficulty 0→1 selects the difficulty-1 launch branches,
  so the count, the types (lines 1395-1600: types 158/162 against 150) and the timing of Japanese
  launches change. Per-entity dive-bomb rows are therefore not comparable one to one with the
  control. Judge them by squadron, as a term of the difficulty.
- **The Vals' PilotBot row stays SPNormal**, so gains, power and brake are unchanged, and the
  aimdive swing should look the same per dive.
- **US ship skill moves from Elite to SPVeteran** for their gun and director bots, but only if the
  gunnery host reads the field (item 5). Otherwise nothing moves there.
- **Aborts and releases** move only through the changed strike set. There is no per-dive mechanism
  change.
- **The tail and hull switches are not reopened by the row.** The row is the same. They would only
  be reopened by the different set of dives.

## 6. Open

* movieval's scene `Skill` property: read the scene record, `docs/SCENE_*`, to confirm it is not
  authored.
* The profile at `save\0\player` is encoded. If the image were signed into that profile, its
  `SelectedDifficulty` would apply instead of 1. The codec is in `docs/GAME_PROFILE_ARCHIVE.md`.
