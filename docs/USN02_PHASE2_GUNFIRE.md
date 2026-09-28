# USN02 phase 2: is Haguro's gunfire the image's?

Packet `cc9_usn02_phase2_gunfire`, worker cc9-gunnery4, on main `b605b9eb9` (reference g's binary
`local\rb7`, SHA-256 prefix `E5FD8535DB95`). Ghidra was only read. The installation was only read.

## 1. Answer

**The host does not over-kill with Haguro's main battery.** Every term of her per-hit damage and
her rate of fire equals the image's rule applied to this installation's tables. Her accuracy chain
is the image's in every bound term. **No switch was added.**

**The premise does not hold on main.** Neither loss the packet names is a gunfire kill:
- **Houston (helm route, 208.26 s).** Haguro is credited only as the last hitter. Her 9 direct
  hits between 166 and 213 s applied 474 of the 4462 that Houston took from all shooters. The
  rest, 3988, is **one Tokitsukaze Long Lance at 168.41 s** that was fired at Exeter at about
  126 s (section 4).
- **Exeter (211.76 s).** She was sunk by a **Nachi torpedo** at 6005 m (`killer_cat=7`,
  `killer_gun=403`), in the director pair's ON run (cc9-gunnery3 `local\WD_ON_USN02.log`), and
  only on that pair's base. On main, with AddDamage also ON, Exeter survives (reference g).

**On the synced head** (main `cf152453e`, with `kGeneratedShipAiBound`), section 6 repeats the
measurements. Haguro's counts are identical to the decimal. The helm-route loss is still the
Tokitsukaze torpedo, and Exeter now falls to an Ushio torpedo.

## 2. The runs

All four runs use `local\rb7\build\win32\Release\bsp_game.exe`, USN02 9200/9000, streams and the
death table on, lockstep 0.05. `BSP_AA_TRACE_UNIT` names the traced shooters. It is observation
only: `pair_diff` of `local\rb7_usn02.log` against `local\g4_tr_idle_usn02.log` exits 1, with
gameplay, the 23 death rows and the 32 unit rows identical.

| log | player | traced | outcome |
| --- | --- | --- | --- |
| `local\rb7_usn02.log` | idle | none | reference g: 23 deaths, no mission end, Houston 900 taken |
| `local\g4_tr_idle_usn02.log` | idle | Haguro, Nachi, Jintsu, Naka | identical to the row above |
| `local\g4_tr_helm_usn02.log` | `3135 takehelm Houston 1.0 EscapePoint` (cc9-ships2's file) | the same four | Houston sunk at 208.26 s, killer Haguro gun 166 at 1878 m; failed at 212.91 s |
| `local\g4_trall_helm_usn02.log` | the same | all 18 Japanese ships | **crashed** at mission frame 795 after `present failed hr=0x88760868` (device lost); the renderer then failed at init for the rest of the session |
| `local\g4_trall3_helm_usn02.log` | the same | all 18 Japanese ships | the rerun once the renderer recovered: `pair_diff` against `g4_tr_helm_usn02` exits 1 with gameplay, the 24 death rows and the 32 unit rows identical |

## 3. Haguro's main battery against the image

Haguro's main battery is five platforms (host gun rows 166..170), device class 200
(`PACK3 Aoba Main Turret A 2019`) in this installation's
`scripts/datatables/classtables/arcade/deviceclasses.lua`, which is the modded table
(docs/GUN_SHOT_CADENCE.md section 9).

| term | image rule | this installation's value | host | verdict |
| --- | --- | --- | --- | --- |
| shell | device 200 `Bullet[1].Bullet` | 28, `Tone/Takao 8'' Shell` | class 28, V0 300, range 2200 | same |
| hull damage base | `00470350` -> `006E7C60`: uniform draw between `DamageMin` (`+ACh`) and `DamageMax` (`+B0h`) | 180..190 | logged bases 180.1..189.5 | same |
| hull damage | `00470510`: `(base * ownerMod - armour) * scale` (listing below) | Houston's `Armour` 90; ownerMod 1.0 (no modifier); scale 1.0 | applied 50.1..55.3 per direct hit = (180..190 - 90) x 0.5556 | same |
| difficulty | `0087D730`, party multiplier 0.5556 (DIFFICULTY_MULTIPLIERS 6) | 0.5556 | 0.5556 | same |
| blast | `0084BAD0` part record, `max(0, f * b - armour) * scale` | `BlastDamageMin/Max` 90, range 80 | 90 against armour 90: applied 0 on every blast hit on Houston | same |
| reload | `007313E0` reads `ReloadTime` into `+28h`/`+2Ch`; steady rate `M/R` (GUN_SHOT_CADENCE 2) | `ReloadTime` 10, `BarrelDelayTime` 0.4, 2 muzzles | mean interval 4.34..5.01 s per platform = 10 s / 2 barrels | same |
| dispersion | `00730160`: `Throw` x the seat bot's `BulletThrowMul` (GUN_DISPERSION 3-5) | `Throw` 0.010001; ArtillerySubDirectorBot SPNormal 0.9 | `kBulletThrowMul[kThrowArtillery][1]` = 0.9 | same |
| aim error | `006DEFF0`: `U(0,1)^Power * U(0, MaxAngleError)` at a `U(0, 2pi)` roll, rerolled every `U(3, 8)` s (`006DF09D FLD [EDX+ECX*4+0Ch]` reads `MaxAngleError`) | SPNormal: 5 deg, Power 1.0 | `kArtilleryGunnerLevels[1]`, `kGunAimErrorBound` ON | same |
| skill | constructor default `unit+390h = 1` (`0095CCCC`); `usn_2_java.lua` never calls `SetSkillLevel` on the six DRKillers (`:300..315`) | SPNormal | `pilot_skill_index` default 1 | same |
| aim point, section draw, ranging error | `006DF520` steps 4-5, `00816650`, `00862660` | robots.lua rows | `kArtilleryAimPointBound`, `kShipSectionPointsBound`, `kArtilleryRangingErrorBound` ON | bound by earlier packets (their labels stand) |

`00470510`'s tail, the hull formula:

```
0047053c: MOVSS XMM0,dword ptr [ESI + 0x14]     ; base = hit+14h
0047057f: CALL 0x008e6430                       ; ownerMod = ProductForUnit(1, owner), when enabled
0047059c: FMUL float ptr [ESP + 0x4]            ; base * ownerMod
004705a9: FSUB float ptr [ESP + 0x10]           ; - armour (the stack argument)
004705ad: FMUL float ptr [ESP + 0x8]            ; * scale, [hit+4h]->vtable[58h]()
```

**Not established:** the hit fraction itself. Haguro's shells hit Houston directly 9 times in 19
turret shots on the helm route and 10 times in 40 on the idle run. Each term that sets that
fraction is bound as above, but no run of the original executable exists to compare the fraction
against, and this packet did not launch one.

## 4. What sinks Houston on the helm route

Haguro's fire at Houston, 166..212 s, counted from the trace lines:

| run | Haguro turret shots at Houston | direct hits | applied | blast hits (applied) | Houston taken, whole run | Houston |
| --- | --- | --- | --- | --- | --- | --- |
| idle (`g4_tr_idle_usn02`) | 40 | 10 | 530.7 | 10 (0) | 900 | survives, health 6037 |
| helm (`g4_tr_helm_usn02`) | 19 | 9 | 473.9 | 9 (0) | 4462 | sunk 208.26 s |

- Haguro fires at Houston alone in that window on both runs. She applies about the same damage
  in both runs, so she does not explain the 3562 difference.
- The per-ship `dealt` column moves from idle to helm by **+3988 for Tokitsukaze** (5000 -> 8988).
  No other Japanese ship moves by more than 153.
- **Every Japanese hit on Houston, helm route** (`local\g4_trall3_helm_usn02.log`):

| shooter | category | record | hits | applied | time |
| --- | --- | --- | --- | --- | --- |
| Tokitsukaze, gun row 345 | 7 (torpedo) | blast | 1 | 3760.6 | 168.41 s |
| Haguro | 3 | direct | 9 | 473.9 | 166.61..207.26 s |
| Tokitsukaze, gun row 345 | 7 (torpedo) | direct | 1 | 227.8 | 168.41 s |
| Haguro | 3 | blast | 9 | 0.0 | 166.61..207.26 s |
| **total** | | | | **4462.3** | = Houston's `taken` |

- **The torpedo is class 67**, `24. Type 93 Mod1 Long Lance ship torpedo`: `DamageMin`/`Max` 500,
  `Blast` 6000..8000 over 50 m, V0 13. The logged direct base is 500.0, which applies 227.8 =
  (500 - 90) x 0.5556. The blast base is 6862.3, inside the authored range, at distance 0. It
  applies 3760.6 through the blast record, which earlier packets bound
  (docs/PROJECTILE_IMPACT.md, the element blast).
- **Gun row 345 aimed both of its four-torpedo salvos at Exeter:** at 5.15..6.65 s and at
  125.15..126.65 s (2427..2438 m). The torpedo that hit Houston at 168.41 s is from the second
  salvo, after a run of about 42 s. Tokitsukaze herself is sunk at 173.16 s.
- The hit takes Houston from 6305 of 6500 to 2316. It destroys hull segment 0 and starts an
  explosion component failure. Haguro's later hits and the damage-control losses (water, fire,
  explosion) finish her at 208.26 s. Haguro is the last hitter.
- **Why only on the helm route:** docs/SCRIPTED_HELM.md 9.3. After `takehelm` the forced cruise
  keeps `009DA1D0` shut, so Houston stops evading torpedoes (129 overrides against 320). The route
  also turns her across the spread aimed at Exeter. That rule is the image's.

## 5. Consequences

- No gunnery switch is needed for Haguro. The packet stops here, as its contract says for an
  image-owned result.
- What decides primary 2 on the helm route is the torpedo exposure of a helmed Houston. That is
  the scripted helm's route (docs/SCRIPTED_HELM.md 9.2 labels it) and the torpedo response rule,
  not gunfire.
- GUNNERY_OPEN_ITEMS 15's "Exeter is sunk at 211.76 s" was a Nachi torpedo on a base without
  AddDamage. Reference g (docs/GAME_EXECUTABLE.md, 2026-09-28 g) records that main does not fail.

## 6. On the synced head (main `cf152453e`, with `kGeneratedShipAiBound`)

The lead asked for the measurements on a head that carries ships3's `kGeneratedShipAiBound`
(`f5863a954`). This tree merged main `cf152453e`. Its `build\` adds only the display request of
`cc9_display_required`, which is gameplay-neutral (docs/TOOLING.md 8.1). Both runs trace all 18
Japanese ships, with streams, the death table and lockstep 0.05.

| log | player | Houston | Exeter | mission end |
| --- | --- | --- | --- | --- |
| `local\g4_sy_helm_usn02.log` | `3135 takehelm Houston 1.0 EscapePoint` | sunk 208.26 s, credited to Haguro (gun 166, 1878 m) | sunk 211.56 s by an Ushio torpedo at 4606 m | failed 212.91 s |
| `local\g4_sy_plain_usn02.log` | idle | sunk 295.95 s, credited to Jintsu (category 2, 1040 m) | sunk 210.81 s by an Ushio torpedo at 4602 m | failed 212.91 s |

**Haguro against Houston, from Houston's first damage (166.61 s) to her sinking:**

| run | window | Haguro shots at Houston | direct hits | applied | blast hits (applied) | Haguro sunk |
| --- | --- | --- | --- | --- | --- | --- |
| helm | 166.61..208.26 s | 19 | 9 | 473.9 | 9 (0) | 206.76 s, by Exeter |
| plain | 166.61..295.95 s | 40 | 10 | 530.7 | 10 (0) | 206.81 s, by Exeter |

These equal the rb7 numbers in sections 3 and 4 to the decimal. Every term of section 3 holds on
this head: bases 180.1..189.5, armour 90, 50.1..55.3 applied per direct hit, 0 per blast, a mean
interval of 4.34..5.01 s per platform.

**Everything that damaged Houston, by shooter** (`local\g4_onhouston.py` over the trace lines):

| run | shooter | category | record | hits | applied |
| --- | --- | --- | --- | --- | --- |
| helm | Tokitsukaze | 7 (torpedo) | blast + direct | 2 | 3988.4 |
| helm | Haguro | 3 | direct | 9 | 473.9 |
| helm | Nachi | 7 | direct | 1 | 0.0 (after the sinking) |
| plain | Haguro | 3 | direct | 10 | 530.7 |
| plain | Jintsu | 2 | direct | 12 | 407.9 |
| plain | Murasame | 6 | direct | 8 | 270.2 |
| plain | Harusame | 6 | direct | 2 | 70.5 |
| plain | **John1 (party 0, friendly)** | torpedo, class 62 | blast | 2 | 1565.5 (782.9 + 782.6) |

- **Helm route.** It is the same as on rb7. One Tokitsukaze Long Lance, from the salvo she aimed
  at Exeter, does 3988 of Houston's 4462. Haguro only finishes her.
- **Plain run.** All Japanese gunfire together applies 1279.3 to Houston over 129 s, and Haguro's
  share is 530.7.
  - Two torpedoes from the US destroyer John1 hit Houston at 276.11 and 277.01 s. They were
    launched at 273.71 s at Harusame, with Houston 101 m away along the run (87 m along, 51 m
    across). Each blast applied about 783.
  - John1's launch-gate line reads `friendly_in_2km=4 crossed=0 closest=-`.
  - A component-failure explosion at 274.01 s and the fire and flooding losses take the rest.
- **So no USN02 loss on this head is a gunfire over-kill.** Houston's helm-route loss is a
  torpedo that her idle path would have evaded. Exeter's loss is Ushio's torpedo
  (GENERATED_SHIP_AI 5). Houston's plain-run loss comes mostly from friendly torpedoes and damage
  control.

**Open, for other packets (not gunfire):**
- **John1's friendly launch.** `kTorpedoFriendlyCrossingBound` (008FFF20's gate,
  `0090058A..009007F6`) let a spread go with Houston 51 m off the run line at 101 m. Whether the
  image's gate would hold that launch is not established here.
- **Houston's damage-control losses on the plain run** (about 3650 of 6500) are not broken down
  per source. The log has no per-unit fire or flood line.
