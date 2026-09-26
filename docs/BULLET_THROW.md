# The bullet throw (packet `cc9_bullet_throw`)

`docs/GUN_DISPERSION.md` sections 2 to 6 read `00730160`'s dispersion cone: the magnitude chain
`0073031D`..`00730498`, the draw `00730540`..`0073058F`, the scales `007305D6` and `0073062C`,
and the application `00730654`..`0073075E`. The pure rules are in
`include/bsp/gun_dispersion.hpp`. The host never applied any of it:
`gun_throw_magnitude_0073031d` had no caller, and the fire record's `Throw` was loaded into
the Lua flat table and never read. This packet completes the read in three places and binds
the cone under `kBulletThrowBound`. Ghidra was read-only; every name is a hypothesis.

## 1. What the read adds to docs/GUN_DISPERSION.md

**A seventh arm, the plane's forward guns** (`0073033B`..`0073038F`, missing from the older
table). When the owner answers `IsKindOf(0Fh)` and the weapon Function `gunclass+80h` is 0, the
chain requires role 1 AI-held (`00521E70` at `00730367`). It then multiplies by
`[unit+0DF4h]->vtable[24h]()`. `[unit+0DF4h]` is the plane's bot, allocated at `007D66F0` and
constructed by `0099A880` with vtable `00D1F348`. Its slot `+24h` is `00999610`
(`BSP_PilotBot_GetAimBulletThrowMul`, no Ghidra function), which returns
`[[00F8A30C] + bot+34h * 248h + 250h]`, the PilotBot row's `AimBulletThrowMul`.

**Every seat getter**, one per bot class (`vtable+24h`):

| seat | bot | getter | record | multiplier by level (Stun, SPNormal, SPVeteran, MPNormal, MPVeteran, Elite) |
| --- | --- | --- | --- | --- |
| `gun+390h`, sub-type other, roles 2 and 3 | AAGunnerBot | `008FB4C0` | `[00E19998] + 10h*lv + 18h` | 1.0, 1.0, 0, 0.5, 0.25, 0 |
| same, owner of kind `0Fh` | TailGunnerBot | `008FB4E0` | `[00E199A0] + 24h*lv + 1Ch` | 1.0, 1.0, 1.0, 1.0, 0.5, 0 |
| `gun+394h`, sub-type `10h`, role 3 | AAFlakBot | `008FB500` | `[00E1999C] + 20h*lv + 28h` | 1.0, 0.6, 0, 0, 0, 0 |
| `gun+398h`, sub-types 4..7, role 4 | ArtilleryGunnerBot | `006DEE00` | `[00E19994] + 28h*lv + 30h` (the SubDirector's) | 1.0, 0.9, 0.1, 0.85, 0.5, 0.1 |
| `gun+39Ch`, sub-type `0Ah`, role 5 | TorpedoBot | `008FB550` | `[00E1998C] + 14h*lv + 1Ch` | 1.0, 1.0, 0, 1.0, 0.5, 0 |
| `gun+3A0h`, sub-type `0Bh`, role 7 | DepthChargeBot | `008FB570` | `[00E19988] + 14h*lv + 10h` | 1.0, 1.0, 1.0, 1.0, 0.5, 0 |
| `[unit+0DF4h]`, a plane's Function-0 guns, role 1 | PilotBot | `00999610` | `[00F8A30C] + 248h*lv + 250h` | 1.0 at every level (`AimBulletThrowMul`) |

The multipliers are this installation's `scripts/datatables/robots.lua` (2025-06-01). It keys
the rows by name, in the order SPNormal, SPVeteran, MPNormal, MPVeteran, Elite, Stun. The table
above re-indexes them by the skill index (0 Stun .. 5 Elite). `bot+34h` is the unit's skill
index, set from `unit+390h` by `008FBC80` (`docs/TURNDOWN_HEADING.md`).

**`00521E70`** (`BSP_Unit_RoleIsAiHeld`): `slot = [unit+1ACh + role*4]; return slot == 8 ||
00927F10(slot)`. A role that is not AI-held skips the multiplier, so the authored `Throw` is used
**unscaled**. It is not zeroed.

**`unit+63Ch`**, which scales the radius of sub-types 4..7 at `007305D6`: the constructor
stores 1.0 (`0095CD9E`, from `00D7A24C`). `BSP_UnitGameObject_UpdatePlayerArtilleryThrow`
(`0095DC40`) re-stores 1.0 at `0095DD79` for every unit whose role 4 is not held locally
(`00927F30` at `0095DC4B`). Only a locally held artillery seat gets the steady-aim fraction.

**`TurnOffAAGunThrow`** is `false` in this installation's `shipglobals.lua:74`, so the AA zeroing
at `007302EA`..`00730317` never fires.

## 2. The host binding (`kBulletThrowBound`)

- **Where it applies:** at every shot, after the host builds the shot direction, and for every
  gun kind, planes included.
- **Magnitude:** `bullet_throw_magnitude_0073031d` is the listing's dispatch. It keys on the
  weapon Function, the bullet class's `+8h` sub-type, the owner's kind, the role slots
  (`unit_current_role_slot`) and `skill_level`.
- **Draw:** `theta = U(0, 2pi)`, `radius = tan(m) * U(0, 1)`, from their own RNG purpose
  `bullet_throw`, keyed per gun under `BSP_GUNNERY_RNG_STREAMS`. It is added across the direction
  and not renormalised.
- **Torpedoes:** sub-type `0Ah` takes the fan instead of the cone.

Substitutions, all labelled in the code:
- A role slot other than 8 counts as player-held; `00927F10` is not modelled.
- `unit+63Ch` is 1.0 and `00470440(7)` is 1.0.
- The cone's axes are an orthonormal pair across the shot direction, not the barrel node's
  rows 0 and 1. The draw is uniform in theta, so the distribution does not depend on that choice.
- The torpedo fan rotates about the hull's up axis. `0085C3F0` was read only at its entry.

A diagnostic line prints each gun's terms at its first shot, on both sides. The summary line
`gunnery bullet throw` counts cone, fan and zero shots, the mean magnitude and angle, and the
seat each shot used.

## 3. The terms each mission fires with (short OFF runs, 3000 steps)

From `local/btS_e2.log` and `local/btS_usn02.log`, guns that fired, as `cat/sub throw_deg level
seat -> magnitude_deg`:

| mission | guns | terms |
| --- | --- | --- |
| E2 | 41 | dual-purpose 6/6, 0.573 deg, level 2, artillery seat -> 0.057 |
| E2 | 29 | AA 1/3, 0.400, level 2, AAGunner -> **0** |
| E2 | 10 | AA 1/3, 0.400, level 2, roles not AI-held (the player's Lexington) -> **0.400 unscaled** |
| E2 | 18 | fighters 0/2, 0.573, level 1, PilotBot -> **0.573** |
| E2 | 6 / 4 | flak 5/16 0.573: Lexington unscaled 0.573 / AI level 2 -> 0 |
| E2 | 1 | Val tail gun 1/2, 0.057, TailGunner -> 0.057 |
| USN02 | 43 | torpedo 7/10, authored `Throw` 0 -> 0 (no fan) |
| USN02 | 13 | artillery at level 0 (2, 3 and 6 with sub-type 6) -> 0.573 |
| USN02 | 16 | artillery at level 1 -> 0.516 |
| USN02 | 10 | artillery at level 2 -> 0.057..0.086 |

## 4. Predictions, written before the pairs

Pairs: E2 (USN04 9200/9000) and USN02 9200/9000, `BSP_GUNNERY_RNG_STREAMS=1` and
`BSP_DEATH_TABLE=1`, OFF `local\btO` against ON `local\btT`, one tree, only the switch differs.
The throw draws come from their own stream, so no other draw moves directly. Everything after
the first changed trajectory still cascades: targets die at other times, and fire moves
elsewhere.

- **E2 AA.** The AI escorts' AA and flak run at level 2 with a multiplier of 0, so only Lexington's
  own guns (0.4 deg AA, 0.573 deg flak) and the dual-purpose guns (0.057 deg) spread. AA hit
  records (categories 1, 5 and 6) fall by 0 to 15 percent.
- **E2 fighters.** Fighter hits fall from 86 by 20 to 50 percent. A 0.573 deg cone is 2 to 8 m
  at the 200 to 800 m the fighters fire from, against an 11 m Val. Fighter kills go from 5 to
  between 2 and 5.
- **E2 headline.** Deaths within 4 of the OFF count; the Lexington row is judged only as a
  knife-edge.
- **USN02 surface.** Main and secondary batteries at levels 0 and 1 spread by 0.52 to 0.57 deg,
  which is 45 to 100 m at 5 to 10 km. Surface hit records fall by 25 to 50 percent. Torpedo
  launches and hits move only by cascade.
- **USN02 headline.** Deaths fall, to between 6 and 15 from the OFF count, because fewer shells
  land. Houston and Exeter survive on both sides.

## 5. The pairs, and the decision

E2: `local/btO_e2.log` against `local/btT_e2.log`. USN02: `local/btO_usn02.log` against
`local/btT_usn02.log`. Main at `0a70c39b9` plus this packet. All four runs exited 0.

| row | E2 OFF | E2 ON | USN02 OFF | USN02 ON |
| --- | --- | --- | --- | --- |
| cone / fan / zero-magnitude shots | - | 1692 / 0 / 3018 | - | 879 / 0 / 305 |
| mean magnitude / mean off-axis angle (deg) | - | 0.373 / 0.188 | - | 0.405 / 0.210 |
| queued hits | 569 | 636 | 647 | 603 |
| deaths | 37 | 38 | 16 | 16 |
| fighter shots / hits / kills | 940 / 86 / 5 | 645 / 80 / 5 | - | - |
| AA cat 1 / 5 / 6 hits | 85 / 106 / 281 | 110 / 97 / 319 | - | - |
| surface cat 2 / 3 / 6 hits / shots | - | - | 161/228, 224/193, 254/429 | 155/228, 232/221, 208/430 |

Verdict per prediction:
- **E2 AA hits fall 0..15 percent: failed; they rose** (cat 1 85 to 110, cat 6 281 to 319).
  The escorts' category-1 AA fires with magnitude 0 at level 2, yet its hits went 81 to 104,
  so the rise is cascade: the planes fly other paths once Lexington's guns and the fighters
  spread. Flak and dual-purpose hits are proximity bursts at about 80 percent, and a 0.06 to
  0.57 degree cone does not move them.
- **E2 fighter hits fall 20..50 percent: failed** (86 to 80, -7 percent). The fighters fired
  fewer rounds (940 to 645) at a higher hit rate (9 to 12 percent). **Kills 2..5: held** (5).
- **E2 deaths within 4: held** (37 to 38).
- **USN02 surface hits fall 25..50 percent: failed; they fell 7 percent** (639 to 595 over
  categories 2, 3 and 6; category 3 per shot 1.16 to 1.05). A 0.52 degree cone is 0.21 degrees
  off-axis on average. That is small against the bound artillery aim error
  (`kArtilleryGunnerLevels`, 4 to 7 degrees at the low levels) and against hull lengths of
  100 to 180 m.
- **USN02 deaths fall to 6..15: failed** (16 on both sides, the same victims).
- **Houston and Exeter survive: held.**

The magnitude chain and draw are the image's, read to the instruction. The multipliers are this
installation's authored values. Neither mission's headline moves outside the noise the gun-sign
and muzzle pairs already showed. **Decision: ON.**