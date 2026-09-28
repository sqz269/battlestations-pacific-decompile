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
  extra damage on the helm route, compared with the idle run, is a torpedo attack (section 4).
- **Exeter (211.76 s).** She was sunk by a **Nachi torpedo** at 6005 m (`killer_cat=7`,
  `killer_gun=403`), in the director pair's ON run (cc9-gunnery3 `local\WD_ON_USN02.log`), and
  only on that pair's base. On main, with AddDamage also ON, Exeter survives (reference g).

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
  No other Japanese ship moves by more than 153. Tokitsukaze is sunk at 173.16 s on both runs, so
  the extra damage is from her torpedoes already running. Her class is a destroyer with torpedo
  platforms (host categories 6, 7, 8).
- That fits docs/SCRIPTED_HELM.md 9.3. After `takehelm`, the forced cruise keeps `009DA1D0` shut
  and Houston stops evading torpedoes (129 overrides against 320). The route also turns her
  across the phase-2 torpedo attack. That rule is the image's.
- **Labelled:** the Tokitsukaze attribution is from the per-ship totals, not from a per-hit trace.
  The run that would have traced every Japanese shooter crashed on the device loss (section 2).
  Rerun `local\g4_trace_runs.ps1 -Which helm` with `G4_TRACE` set to every Japanese ship to
  confirm it hit by hit.

## 5. Consequences

- No gunnery switch is needed for Haguro. The packet stops here, as its contract says for an
  image-owned result.
- What decides primary 2 on the helm route is the torpedo exposure of a helmed Houston. That is
  the scripted helm's route (docs/SCRIPTED_HELM.md 9.2 labels it) and the torpedo response rule,
  not gunfire.
- GUNNERY_OPEN_ITEMS 15's "Exeter is sunk at 211.76 s" was a Nachi torpedo on a base without
  AddDamage. Reference g (docs/GAME_EXECUTABLE.md, 2026-09-28 g) records that main does not fail.
