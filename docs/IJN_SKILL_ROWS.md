# Which skill row the Japanese aircraft fly (packet cc9_ijn_skill_rows)

Addresses: 009420A0, 00927A80, 006E6210, 0043D8F0 (partial, as before), 007B8AE0, 009565A0.
Ghidra was read only. No host code changed, and no pair was run, because the rows match.

## 1. Answer

**Every Japanese aircraft in the E2 window flies row 1, SPNormal, in the image and in the host.**
The Kates' release distances 450 m (after 15 s of run) and 650 m are that row's. The attackers'
skill is the image's.

## 2. The image's selection

1. **The bag's Skill.** Every E2 Kate, Val and Zero is a `SpawnNew` member (usn_19_coralus.lua
   2590-2880). `BSP_SpawnMember_SeedPropertyBag` (009420A0) seeds the member's property bag
   before the Lua member table is copied over it:
   - `Skill` = `1 + 2 * (game+1FE4h != 0)`, stored at 009420D3-009420E9 with key 00CF8838
     "Skill". That is 1 in single player and 3 in network play.
   - No member table in this script authors `Skill`, so the seeded 1 stands.
2. **`Crew` is never consulted.** These members author `Crew` = 1, 2 or 3, which escalates wave
   by wave. But the only reader of the `Crew` key (00D19264) is `BSP_PropertyBag_GetSkill`
   (00927A80, the reference at 00927AAF). It reads `Crew` only when the bag has **no** `Skill`,
   and then maps it through 006E6210. A SpawnNew bag always has one, so `Crew` does not reach
   the skill of these planes.
3. **No script call changes it in the window.** `SetSkillLevel` reaches, by the host log of
   `local/rT_9000.log`:
   - the 18 US ships (`Mission.SkillLevelOwn`, SPVeteran at difficulty 1);
   - units 81-85, the IJN ships (`Mission.SkillLevel`, SPNormal at difficulty 1);
   - unit 89 (level 2).

   The `LexKillers` (two D4Y and two B5N, `SetSkillLevel(..., SKILL_ELITE)` at 1178-1181) spawn
   from `luaSpawnLexKillers` after a later cinematic (line 1036). They do not appear in the E2
   run. The phase-3 strikers (1395-1595) take `Mission.SkillLevel`, which is SPNormal, and are
   not reached either.
4. **Where the row is read.** The skill lands in unit+390h (the constructor default 1 at 0095CCCC;
   SetSkillLevel through 007B8AE0 and 009565A0), which is the pilot bot's +34h. The torpedo task
   reads its release pair from the row `[[unit+DF4h]+34h]` selects. The dive-bomb task captures
   it at 009F9D22.

## 3. Row table, image against host (E2)

| aircraft | image row | host row | release distances near / far | release alt | aspect scale |
| --- | --- | --- | --- | --- | --- |
| B5N Kate (16) | 1 SPNormal | 1: pilot_skill_index default 1; the torpedo pair is named SPNormal | 450 / 650 m | 12 m | 0.7 |
| D3A Val (16) and movieval (3) | 1 SPNormal | 1: dive row captured from the slot's skill index | dive row SPNormal: release alts 350 / 450 m | - | - |
| A6M Zero (16) | 1 SPNormal | 1 | - | - | - |
| rear gunners (category 1 on B5N, D3A) | 1 | 1: `units.skill_level(owner)` | AAGunner row SPNormal | - | - |

Torpedo rows in this installation (docs/TORPEDO_RELEASE_GATE.md table):

| row | release alt | near | far | aspect |
| --- | --- | --- | --- | --- |
| SPNormal | 12 | 450 | 650 | 0.7 |
| SPVeteran | 5 | 800 | 1200 | 0.5 |
| MPNormal | 10 | 800 | 1200 | 0.7 |
| MPVeteran | 10 | 800 | 1200 | 0.6 |
| Elite | 5 | 800 | 1200 | 0.5 |
| Stun | 12 | 350 | 600 | 0.7 |

**Why this matters.** A Veteran or Elite Kate would release at 800-1200 m, and run A's Kates die
448-1074 m from the nearest ship. So a higher row would have let some of them drop. The image
does not give them one in single player at difficulty 1: only `GetDifficulty() == 2` gives the
IJN fleet SPVeteran, and even then the phase-1 planes keep their seeded 1.

## 4. The host's one standing substitution

The host's torpedo pair is **named** SPNormal (`kTorpReleaseDistNearSPNormal` and the others)
rather than selected from the slot's skill index. For every E2 aircraft the two agree. They
would diverge only for a Kate with a non-1 index: a LexKiller (Elite) or a network-play spawn.
That selection is a change request for the owner of the torpedo-attack hunk
(cc9-dogfight-engaged): read `pilot_skill_index` into the row as the dive task does. It does not
move E2.

## 5. Decision

Nothing to bind: the Japanese aircraft's skill row is the image's. The E2 zero-ordnance outcome
stands with the attackers' SPNormal row, the defenders' SPVeteran row and every AA term as read.
