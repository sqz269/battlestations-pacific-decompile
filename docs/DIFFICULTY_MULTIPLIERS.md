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

Revised by packet `cc9_difficulty_multiplier_predictions`, a read with no runs.

### The ship hit, step by step, for the multiplier

In `00826F10`, the breakable-segment step comes **before** the multiply. It passes the raw part
damage to `0092D1F0`:

```
0082700E  FLD  dword [ESP+18h]        ; the raw amount
00827013  FSTP dword [ESP]            ; argument
00827026  MOV  ECX,[EDI+1018h]        ; the controller
0082703E  CALL 0092D1F0               ; segment slot -= raw; at <= 0: -10000, route 99h
0082704D  CALL 00470510               ; hull formula
008270AD  FLD  dword [EAX+EBX*4]      ; +1Ch[level], party-gated (section 1)
008270B3  FMUL dword [ESP+14h]
```

- **The segment slots and their 99h** (`0092CED0` then `00821FF0` then `0080E440`: debris and
  the hull body, docs/BLAST_ELEMENT_PARTS.md section 2) take the **unscaled** amount. So do the
  burst's part pass (`008275A0` then `0092D1F0`) and the effects.
- **Flooding is not scaled either.** The water seconds come from the weapon's `WaterDamage` (10
  for the Type 93), queued at `0082734C`..`00827363` into `0093A4F0`, independent of the damage.
- **The water damage itself is scaled.** It runs through `unit->vtable[1ACh]`, which is
  `0095DA00` then `0087D730`, every step at the class's `dcwater` rate
  (`src/game_hosts_gunnery.cpp`, the 0093C120 water step).
- **The health loss of the burst is scaled.** `008777D0`'s part pass adds its largest result
  through `vtable[1ACh]` (`00877A37`).

**Exeter under the multiplier, from `local\p2_off_usn02.log`:**
- The 35.65 s Type 93 still destroys hull segments 0 and 1 (unscaled), and it still starts about
  10 s of flooding.
- **Health:** 6500 - 0.5556 x (410 + 6076.4) = about 2896 after the hit.
- **Flooding:** the water step drained 16 in its first 0.15 s, about 107 per s. That is about 1070
  over the 10 s unscaled, so about 595 scaled, against about 20 per s of hull repair
  (`repaired=3` in 0.15 s). **She ends the flooding near 2500 and survives.** The failure test at
  `usn_2_java.lua:521` does not fire at 39.65 s.
- **What could kill her next:**
  - In the OFF run no torpedo is aimed at Exeter or Houston after 4.75 s.
  - Tokitsukaze's next salvo is at 123.15 s, and Asagumo and Yukikaze keep firing to about 243 s
    and about 160 s.
  - One more Type 93 hit is about 3604 + 595 scaled, which kills her from about 2500.
  - Houston took 1109 in the whole OFF run, about 616 scaled, so it survives.

### The four missions

| mission | OFF party-0 damage taken | prediction |
| --- | --- | --- |
| USN02 9200/9000 | 20579, 7 party-0 ships sunk | Party-0 damage falls to about 0.556. **Exeter survives the 35.65 s torpedo** (about 2500 left), so there is no failure at 39.65 s. Either a later salvo re-targets and hits Exeter (a failure band of about 150 to 280 s: the salvos at 123 s and later plus about 30 s of run) or **phase 2 is reached**. Party-0 deaths fall; party-1 deaths and hit records rise with the longer fight. Exit 3 |
| USN04 4700/4500 | 1149 (Fletcher-class05) | Fletcher-class05 takes about 638. No death flips, deaths stay 44. Exit 3 |
| USN01 3200/3000 | 440 (the two ScoutDauntless) | See below. Exit 3 |
| JM06 3200/3000 | 0 | Identical (exit 0 or 1) |

**USN01's Dauntlesses.**
- Aircraft damage reaches `0087D730`. The Dauntless primary vtable `00D19D28` and the PBY's
  `00D00308` both hold `0095DA00` at `+1ACh`, and the AA hit's `008777D0` adds through
  `vtable[1ACh]`.
- `VehicleClass[108]` (Dauntless) has `HP 220` and `Armour 5`.
- In the OFF run each died to about 8 or 9 machine-gun hits (`c1`) over 2.6 to 2.9 s
  (127.20 to 129.85 and 133.10 to 135.95). Scaled, a Dauntless needs about 396 raw, roughly 1.8
  times the hits.
- **Prediction: both still die, about 2 to 3 s later** (about 132 s and 138 s). A flip to
  survival would need the AA to stop within that window. That is possible but not the expected
  row. USN01's deaths stay 7, and the two death times move.

## 5. Uncertainty

- Exeter's later fate depends on whether a Japanese destroyer re-targets her after 35 s. That
  selection runs through the ship AI and the gunnery host and was not read. The prediction gives
  the band and the alternative (phase 2).
- The 107 per s water rate is measured from the first 0.15 s of the OFF log (`water_total=16`),
  not from the class `dcwater` value.
- The Dauntless call assumes the same AA exposure as the OFF run. Positions shift once the fight
  changes, so the death times are a band, not a figure.
