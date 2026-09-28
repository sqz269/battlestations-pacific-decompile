# The difficulty damage multipliers

Packet `cc9_difficulty_multiplier_read`, worker cc9-ships2, on main `2f4ac5e4c`. Ghidra was
read-only. Every descriptive name here is a hypothesis, not a recovered symbol.

Addresses: `00927F30`, `0095DA00`, `0087D730`, `00826F10` (the site `008270AD`), `0087D7B0` (the
fills at `0087DB08`..`0087DDB3`), `00432650`.

## 1. There are two tables, and the one that matters is party-wide

`00432650 BSP_GlobalConfig_GetSingleton` returns the global config. `0087D7B0` fills four float
vectors in it from `Globals["Difficulty"]` (`scripts/datatables/globals.lua`), one entry per
difficulty index while `HPMultipliers[index]` is not nil. Each list is read from its own local:

| config vector | source list (local) | transform | listing |
| --- | --- | --- | --- |
| `+1Ch` (first `+20h`) | `HPMultipliers` (`[ESP+0B0h]`) | **1 / value** (`0087DB61 FLD1`, `0087DB63 FDIVRP`) | `0087DB6A LEA EBP,[ESI+1Ch]` |
| `+2Ch` | `ScoreMultipliers` (`[ESP+114h]`) | as read | `0087DC08 LEA EBP,[ESI+2Ch]` |
| `+3Ch` | `LockRadiusMultipliers` (`[ESP+13Ch]`) | as read | `0087DCA2 LEA EBP,[ESI+3Ch]` |
| `+4Ch` | `PlayerCheatMultipliers` (`[ESP+128h]`) | **1 / value** (`0087DD33 FLD1`, `0087DD35 FDIVRP`) | the fourth loop |

This installation's `globals.lua` has `HPMultipliers = { 1.8, 1.8, 1.8 }` and
`PlayerCheatMultipliers = { 1, 1, 1 }`. So:
- **`+1Ch[level]` = 1/1.8 = 0.5556 at every level;**
- `+4Ch[level]` = 1.0 at every level.

**The level therefore does not matter here.** In single player (`[00E188A8]+1FE4h == 0`) it is
`[00E188A8]+6ACh`; otherwise it is the literal 2 (`0095DA44`, `0087D75B`, `0082707E`).

### Who is multiplied

- **`0095DA00`** (unit vtable `+1ACh`, AddDamage): only in single player, and only when
  `00927F30(unit, 0)`, **the local player holds role 0 of this unit**. The damage is then times
  `+4Ch[level]`. `00927F30` is `[game+18ECh] <= 7 && unit[1ACh + role*4] == [game+18ECh]`
  (`00927F35`..`00927F44`).

```
0095DA2B  PUSH 0 / CALL 00927F30        ; role 0 held by the local player
0095DA4B  MOV EDI,[EAX+6ACh]            ; level
0095DA51  CALL 00432650
0095DA58  MOV ECX,[ESI+50h] / ADD ESI,4Ch
0095DA76  FLD dword [ECX+EDI*4]         ; +4Ch vector [level]
0095DA7A  FMUL dword [ESP+0Ch]          ; x damage
0095DA7F  FSTP dword [ESP+8]
0095DA8D  CALL 0087D730
```

- **`0087D730`**: in single player, when **the unit's party (`+54h`) equals the local player
  record's `+28h`**. The record is `[game + 18CCh + [game+18ECh]*4]`, and `+28h` is its party, as
  the reconlevel writer `0077B0C0` compares it with a party argument. The damage is then times
  `+1Ch[level]`, then `00879070`.

```
0087D743  MOV EDX,[EAX+18ECh]
0087D749  MOV EDX,[EAX+EDX*4+18CCh]     ; local player record
0087D750  MOV ESI,[EBX+54h]             ; unit's party
0087D753  CMP ESI,[EDX+28h]             ; the player's party
0087D762  MOV EDI,[EAX+6ACh]            ; level
0087D76F  MOV ECX,[ESI+20h] / ADD ESI,1Ch
0087D78D  FLD dword [EAX+EDI*4]         ; +1Ch vector [level] = 1/HPMultiplier
0087D791  FMUL dword [ESP+0Ch]
0087D7A3  CALL 00879070
```

- **The ship hit record `00826F10`**, after the hull formula `00470510` (`0082704D`), applies the
  same party test and the same `+1Ch` vector to the hull damage:

```
00827065  MOV EDX,[EAX+18ECh]
0082706B  MOV ECX,[EDI+54h]             ; victim's party
0082706E  MOV EDX,[EAX+EDX*4+18CCh]
00827075  CMP ECX,[EDX+28h] / JNZ 008270BB
00827085  MOV EBX,[EAX+6ACh]
008270AD  FLD dword [EAX+EBX*4]         ; +1Ch (first +20h) [level]
008270B3  FMUL dword [ESP+14h]
008270B7  FSTP dword [ESP+14h]
```

**All three are damage taken.** Nothing on the dealing side reads these vectors.

**The correction.** docs/UNIT_DAMAGE_AND_DEATH.md calls `0087D730`'s test "the current player's
own unit". The listing compares the party, so it covers **every unit on the player's side**. The
host's `src/unit_damage.cpp` (`is_current_player_unit`) and the gunnery ShipHit binding
(`unit_is_local_players() == false`, `difficulty_multiplier` = 1.0, `008270AD` recorded) inherit
that wording. **In this installation every player-side unit takes 1/1.8 of its damage.** None of
this host's paths applies it.

**Correction to docs/GAME_SHIP_NAVIGATION_BINDING.md (Exeter).** That section says difficulty
scaling does not touch Exeter. It does: Exeter is party 0, the player's side. In the image the
torpedo's 410 + 6076.4 becomes about 3604 of her 6500 HP. Whether the flooding still sinks her is
the measurement below.

## 2. What the host has to reuse

- **Role 0.** `GameUnitsHost::unit_current_role_slot(index, 0, holder) && holder == 0` is
  `00927F30(unit, 0)`, as the hud host already calls it (`src/game_hosts_hud.cpp`, the movie
  screen inputs). The units host's own `holds` lambda (`role_screen_update_0067bb50`) is private
  to the units host.
- **Party.** `GameUnitsHost::unit_side_0054(index)` against the local player's party. On every
  reference run that is party 0, the controlled unit's party. JM06's controlled unit is
  Fletcher-class 08, party 0.
- **Values.** `Globals["Difficulty"]` is already read by `read_lock_radius_multipliers_0087dc85`
  (`src/game_hosts_lua.cpp`). The same walk yields `HPMultipliers` and `PlayerCheatMultipliers`.

## 3. The binding, for the source step

The switch is `kDifficultyMultipliersBound`, in `include/bsp/game_hosts_gunnery.hpp`, OFF. In
`src/game_hosts_gunnery.cpp`, which cc9-gunnery3 holds under four leases on 2026-09-28, so the
edit is queued:
1. ShipHit's `unit_is_local_players()` answers `unit_side_0054(victim) == local party`. Its
   `difficulty_multiplier(level)` answers `1 / HPMultipliers[level]`. That is `008270AD`.
2. Every AddDamage the host routes for a unit (the blast part pass's `00877A37`, and plane and
   other unit damage) takes `1 / PlayerCheatMultipliers[level]` when role 0 is the local
   player's, then `1 / HPMultipliers[level]` when the party matches: `0095DA00` then `0087D730`.
   `src/unit_damage.cpp`'s `is_current_player_unit` should be renamed to the party test.
3. The level is the campaign difficulty `game+6ACh`. It is labelled, because all three levels
   carry the same values here.

## 4. Predictions, from the OFF logs of `4df48ee59` (worktree cc9-ships2)

Party-0 damage taken, from each log's unit table (`local\p2_off_*.log`):

| mission | party-0 damage taken | party-0 sunk | expected with the switch |
| --- | --- | --- | --- |
| USN02 9200/9000 | 20579 (Exeter 6486, Java 2927, DeRuyter 2831, John2 2500, John3 2230, Electra 1126, Houston 1109, Kortenaer 1016, John1 354) | 7 | Every party-0 figure falls to about 0.556 of its value. Exeter takes about 3604 from the 35.65 s torpedo and is **expected to survive it**, so the 39.65 s failure moves later or goes away and phase 2 becomes reachable. Party-0 deaths fall; party-1 deaths and hit records move with the longer battle. Exit 3 |
| USN04 4700/4500 | 1149 (Fletcher-class05) | 0 | Fletcher-class05 takes about 638. No death flips. Exit 3 on its row, deaths 44 unchanged |
| USN01 3200/3000 | 440 (the two ScoutDauntless planes, 220 each) | 2 | About 122 each, **if** the plane damage path reaches `0087D730`. Whether they survive depends on the plane HP (not read here). One or both deaths may flip. Exit 3 |
| JM06 3200/3000 | 0 | 0 | Identical (exit 0 or 1) |

## 5. Uncertainty

- The ScoutDauntless flip depends on the plane class HP and on the plane damage path reaching
  `0095DA00`/`0087D730`. Neither was read.
- Exeter's survival depends on the flooding. The segments destroyed by `0092D1F0` and the leaks
  come from the unscaled part damage (`0082703E` precedes the multiply), so her water ingress may
  still sink her later.
- `unit[1ACh]` role 0 on idle runs: the hud host reads the role slot the units host keeps. Every
  role-0 unit is the controlled one, and it is on the player's side, so `0095DA00`'s 1.0 changes
  nothing here.
