# USN02's phases after phase 1: a pre-read for the first phase-2 run

Packet `cc9_usn02_phase2_preread`, worker cc9-ships2, on main `9780c63a9`. It is docs only, with
no runs and no Ghidra writes. It sits in its own file because `docs/LUA_BINDING_MISSION.md` is
leased to cc9-lua2 (`cc9_lua_natives_ranking`).

- **Script:** this installation's `scripts/missions/usn/usn_2_java.lua`, 975 lines, read in pages.
- **Status source:** the phase-2 native status comes from the last worktree-cc9-ships2 log that
  reached phase 2, `local\hp_off_usn02.log` (`MissionPhase=2`). It is checked against the current
  base's `local\p2_off_usn02.log` and the source.

## 1. The transitions

`luaCheckObjectives` (`:493`) runs every `MissionObjectiveCheckerDelay`. In every phase above 0,
`Houston.Dead or Exeter.Dead` calls `luaMissionFailed()` (`:521`).

| from | to | trigger | line |
| --- | --- | --- | --- |
| 0 | 1 | `luaIn` after the intro movie | `:651..655` |
| 1 | 2 | primary 1 active and `GetHpPercentage(DeRuyter) < 0.15` **or** all eight `EnemyDestroya` dead (Yamakaze, Minegumo, Asagumo, Yukikaze, Kawakaze, Tokitsukaze, Amatsukaze, Hatsukaze). Then `luaObj_Completed("primary",1)`, `luaPh1FadeOut` = `Blackout(true, "luaMoveToPh2", 1)`, and the fade's callback runs `luaMoveToPh2` | `:529..545`, `:611`, `:677` |
| 2 | end (complete) | the first live `CATable` ship (Houston, then Exeter) within 500 m of `EscapePoint` (0, -7500), then `luaMissionComplete` | `:547..600` |
| 2 | end (failed) | Houston or Exeter dead | `:521` |

DeRuyter's group carries `SetInvincible(unit, 0.1)` (`:230`). In the image that is a 10% health
floor, so the `< 0.15` test is met by damage without a death. The six DRKillers (Haguro, Jintsu,
Yudachi, Samidare, Murasame, Harusame) carry `SetInvincible(unit, 0.5)` (`:310`).

## 2. Phase 2's natives, in order of first call

| native | address | host status (phase-2 log, current base) | what phase 2 depends on |
| --- | --- | --- | --- |
| `Blackout` (callback `luaMoveToPh2`) | `008D1340` | record; the callback runs | the phase switch itself |
| `NavigatorAttackMove` | `008A30D0` | concrete | DRKillers and FinalShips orders |
| `GenerateObject` (Nachi, Sazanami, Naka, Ushio) | `00944FD0` | concrete (4 calls) | **the four spawns** |
| `SetSkillLevel`, `NavigatorSetTorpedoEvasion`, `NavigatorSetAvoidLandCollision`, `TorpedoEnable`, `RepairEnable` | `00895250`, `008A3CD0`, `008A3B10`, `0089C8F0`, `008AD330` | concrete | the FinalShips' setup |
| `FindEntity` | `00898E30` | concrete | HiddenTrgs |
| `StartDialog` (`luaStartDialog("DRDEAD")`) | `008B0540` | record | dialogue only |
| `luaIngameMovie` (then `luaPh2MovieEnd`) | movie natives | the callback runs (`AddDamage` is reached) | the rest of phase 2 |
| `Music_Control_SetLevel` | `008C4D10` | record | music only |
| **`SetInvincible`** (DRKillers to 0) | `00897A50` | **UNIMPLEMENTED** (20 calls) | lifts the DRKillers' 50% floor. Queued as cc9-gunnery3's `cc9_set_invincible_floor` |
| **`AddDamage(unit, 100000000)`** over DRGrp | `0088E000` | **UNIMPLEMENTED** (4 calls) | **scuttles DeRuyter, Java, Kortenaer and Electra** |
| `SetSelectedUnit(Houston)` | `00647300` path | concrete | the controlled unit |
| `Objectives_Add` (via `luaObj_Add`, primary 2 and hidden 1) | - | concrete | objective display and state |
| `GetPosition`, `GetMeasure` | `008A7B00`, `0088D8E0` | concrete | the escort distance |
| `DisplayScores` (via `luaDisplayScore`) | `008C20D0` | UNIMPLEMENTED (1) | display only |
| `HideUnitHP` | `008C1F50` | UNIMPLEMENTED (1) | display only |
| `Objectives_Completed` / `Objectives_Failed` | `008BD340` / `008BD900` | UNIMPLEMENTED | the native objective record and score. The script's own `luaObj_IsActive` state lives in Lua |
| `MissionNarrative` | `008B0C10` | UNIMPLEMENTED | display only |

## 3. The gaps ranked, and two packet contracts

1. **`AddDamage` (`0088E000`, body `0088E000-0088E1A3`), a missing kill.** In the image
   `luaPh2MovieEnd` scuttles all four DRGrp ships the moment phase 2's movie ends. The host
   leaves them alive and fighting, which changes every phase-2 death and hit row.

   *Contract:*
   - Bind `AddDamage(entity, amount)` behind one switch. The native reads the entity (`00888AA0`)
     and one number, then calls `entity->vtable[1ACh](amount)` at `0088E15B`
     (docs/UNIT_DAMAGE_AND_DEATH.md).
   - Route it through the gunnery host's unit damage, `0095DA00` then `0087D730` then `00879070`,
     including the invincibility floor and the party multiplier of docs/DIFFICULTY_MULTIPLIERS.md.
   - Predictions on USN02: at phase 2's movie end, DeRuyter, Java, Kortenaer and Electra die in the
     same tick, unless already dead. The deaths rise by those not yet sunk. USN01, USN02 phase 1 and
     USN04 are identical.
2. **`SetInvincible` (`00897A50`, body `00897A50-00897CAE`).** This is already
   `cc9_set_invincible_floor` with cc9-gunnery3. On phase 2 it lifts the DRKillers' 50% floor and
   the DRGrp's 10% floor before the scuttle. That binding should carry a phase-2 row.
   - If a second new packet is wanted, the next is **`Objectives_Completed` / `Objectives_Failed`**
     (`008BD340` / `008BD900`, read in docs/AI_WORLD_SETS.md and docs/CONTROLLED_UNIT.md). They are
     scoring and display, and do not stop phase 2.

## 4. What a phase-2 run changes in the current predictions

- **Units:** 28 becomes **32**. GenerateObject adds Nachi, Sazanami, Naka and Ushio.
- **Deaths:**
  - The phase-1-only numbers of reference f and the multiplier predictions (19 / 17 deaths, 566 to
    610 hits) stop being comparable once phase 2 runs.
  - The four FinalShips attack Houston (Nachi, Sazanami) and Exeter (Naka, Ushio).
  - With `AddDamage` bound, DeRuyter, Java, Kortenaer and Electra die at the movie's end. Without
    it they survive.
- **Objectives:**
  - Primary 1 completes.
  - Primary 2, escort to EscapePoint, cannot complete with an idle player: Houston is the selected
    unit and nothing orders it south.
  - Hidden 1 (Nachi and Naka dead) is possible.
  - Secondary 1 fails if Perth dies.
  - The mission then ends only by Houston's or Exeter's death, or runs out the frames.
