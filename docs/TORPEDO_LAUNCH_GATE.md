# The ship torpedo bot's launch gate (packet `cc9_torpedo_launch_gate`)

Question from `docs/KILL_CREDIT.md` section 4: does the image's ship torpedo bot hold a launch
when a friendly ship lies in the torpedo's path? **Yes.** `008FFF20`
(`BSP_GunBot_HeadingAimTick`, the `TorpedoBot` tick, weapon sub-type 7, slot `+39Ch`) walks the
own-party ship list before every launch and aborts the shot through `gun->vtable[1E8h](0)` when a
friendly would be near the crossing of its course with the torpedo's run. The host had no such
gate. It is now reconstructed and bound under `kTorpedoFriendlyCrossingBound`.

`008FBB00` is **not** the torpedo bot tick, as `docs/KILL_CREDIT.md` says. It is
`BSP_Math_SolveInterceptPoint`, which `008FFF20` calls at `0090025E`. The tick is reached only
through the vtable slot at `00D182BC`, so `callers` shows none.

Evidence: the Ghidra listing of `008FFF20` (body `008FFF20..0090099B`), the raw bytes of
`00900440..00900583`, the constants below, and `docs/RECON_SLOT_LISTS.md` for the list.
Ghidra was read-only.

## 1. Every gate on a launch, in order

| # | address | image rule | native (host) |
| --- | --- | --- | --- |
| 1 | `008FFFCE` | walk the bot's parent chain to the first `IsKindOf(5)` unit; return while `unit+184h` is set | not modelled |
| 2 | `008FFFE9`, `00900018` | a target that answers `IsKindOf(8)` is dropped unless the owner does too or `00852820` accepts | not modelled |
| 3 | `0090003B`..`00900100` | think timer `bot+5Ch -= dt`; runs every `0.2 s` (`00CE54A0`) | host runs every tick |
| 4 | `00900110` | a submarine target must be shallow: `008527E0`, `[t+1204h] - 3.0 < [t+100h]` | `submarine_is_shallow_008527e0`, not wired here |
| 5 | `009001A8`..`009001F3` | range to the target: `distance <= min(owner+44Ch * 1.5 (00CE3D78), bot+68h + 100 (00D7A220))`, else abort at `009003A6` | the host's shared target range |
| 6 | `0090025E` | `008FBB00` must solve the intercept at WaterTravelSpeed (`+0E4h`), else abort | `torpedo_intercept_point_008fbb00`, bound |
| 7 | `009002EB`..`0090031E` | range to the lead point: `distance <= min(owner+44Ch, bot+68h)`, else abort | not separately modelled |
| 8 | `00900380`, `009003C6` | `0085AB50` snaps the heading up to pi/4 onto a window edge; `FLT_MAX` aborts | `gun_snap_heading_to_fire_window_007f6190`, bound |
| 9 | `009003DD` | `0085ABA0(heading, 0.0f)` | `want_vert = 0` |
| 10 | `009003F8`..`0090041C` | `\|bot+60h - gun+480h\| < 1 degree` (`00CE3984`, float `0.0174533`), else abort | host settle band 0.1 degree plus `007F60A0` window |
| 11 | `0090042F` | `gun->vtable[1D0h](1)` must accept, else abort | the host's `0085A830` can-fire gate after the trigger |
| 12 | `0090058A`..`009007F6` | **the friendly-crossing scan**, section 2 | `torpedo_friendly_crossing_008fff20`, `kTorpedoFriendlyCrossingBound` |
| 13 | `00900830`, `0090083E` | aim jitter `+/- 00BD2F10(level+0Ch, level+10h)` degrees (AngleErrMin/Max) | not modelled |
| 14 | `00900964` region | bit 2 of `[gun+3F0h]+634h` aborts | inhibit is always clear in the host |
| 15 | `009008F3`..`00900951` | the launch command object; `00951FC0` flips the `+6D4h` spread offset | not modelled |

Robot parameters (`008FD640`, `robots.lua` `TorpedoBot`): AngleErrMin `+0Ch`, AngleErrMax `+10h`,
FireTargetAccuracy `+14h`, AnyTargetAccuracy `+18h`, BulletThrowMul `+1Ch`. None of them enters the
friendly scan.

## 2. The friendly-crossing scan

| address | rule |
| --- | --- |
| `0090011D`..`00900142` | `gun = 00427EB0([bot+58h])`, the gun's `+FCh` position, kept at `[ESP+3Ch..44h]` |
| `00900236` | `speed = [[gun+3F8h]+34h]+0E4h`, WaterTravelSpeed, kept at `[ESP+74h]` |
| `009002AB`..`009002BF` | `lead = 008FBB00 result + unit+6D4h * owner axis (+20h, +28h)`, `(x, z)` at `[ESP+70h]`, `[ESP+60h]` |
| `0090043D`..`0090048B` | `snap = \|00438B10(bot+60h, raw heading)\|` in degrees (`FDIV pi`, `FMUL 180`) |
| `0090049D`..`009004DA` | `p = gun + 2.0 (00D7A308) * snap * [gun+3CCh]+110h`, row 2 of the gun node's world matrix |
| `009004DE`..`00900506` | `dir = normalise(lead - p)` in `(x, z)` (`0042B260`) |
| `00900520`..`00900583` | run line `a0 = gun`, `a1 = gun + 1000 (00CE47A0) * dir` |
| `0090058A`, `0090058F` | list head `[008053C0([owner+54h]) + 0DDCh]`, the recon slot's own-party triple |
| `009005AD`, `009005BA` | the node's entity must answer `IsKindOf(6)` (a ship) and must not be the owner (`[gun+3Ch]`) |
| `00900607`..`00900618` | `\|gun - entity\|^2` in 3D must not exceed `4000000` (`00D09FE8`), so 2000 m |
| `00900630`..`009006E7` | the friendly's line `b0 = pos - 1000 * (+94h, +9Ch)`, `b1 = pos + 1000 * (+94h, +9Ch)` |
| `009006EE` | `004F3730(a0, a1, b0, b1, &cross)` must accept (both parameters in `[0, 1]`) |
| `009006FB`..`00900735` | `run = 00414C60(cross - a0)` |
| `00900742`..`00900750` | `time = run / speed` |
| `00900754`..`009007C7` | `miss = 00414C60(pos + vtable[34h]() * time - cross)`, `(x, z)` only |
| `009007D0`..`009007E6` | `threshold = run * 300 (00CE3CA8) / 1000 + 200 (00CE4D70)` |
| `009007F2`, `009007F6` | `threshold > miss` jumps to `0090096D`, which calls `gun->vtable[1E8h](0)` and returns |

So a launch is held when another own-side ship within 2000 m of the gun has its heading line
crossing the first 1000 m of the run, and will be within `0.3 * run + 200` metres of that crossing
when the torpedo arrives. A friendly more than 2000 m away, or one whose line crosses the run
beyond 1000 m, never holds a launch.

## 3. The host binding

`src/gun_bot_remainder.cpp` has the two pure rules: `torpedo_run_end_008fff20` and
`torpedo_friendly_crossing_008fff20`. The second uses the exact `004F3730` kernel
`native_segment_crossing_004f3730`. `src/game_hosts_gunnery.cpp` walks the units in
`torpedo_friendly_hold_008fff20` when the torpedo gun's `want_fire` is true, and clears it on a
hold. The summary line `torpedo_friendly scans= holds=` counts it, and the first 60 holds are
logged with shooter, friendly, run, miss and threshold. A diagnostic `torpedo launch` line at
every launch names the own-side ship closest to holding it; it changes no state.

Substitutions, all labelled in the code:
- The lead point lacks the `+6D4h` spread offset, which the host does not model.
- The node axis is the hull forward turned by the gun's current horizontal angle, at zero
  elevation. The host builds no gun node.
- The own-party list is every live unit with the owner's `+54h` side. A sunk ship leaves the
  host's list; whether a sinking wreck leaves the image's own triple was not read.
- The gate runs where the host's `want_fire` is true. The image runs it after its one-degree
  test, and the host's settle band is 0.1 degree, so the gate sees a subset of the image's ticks.

## 4. Predictions, written before the pair

Pair: USN02 9200/9000, `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1` on both sides, OFF
`local\tgO` against ON `local\tgT`, one tree, only the switch differs.

- `torpedo_friendly scans` is equal on both sides' count basis, and `holds` is 0 OFF by
  construction. ON: holds between 5 and 300.
- Torpedo shots (`torpedo_gate shots`) fall on the ON side, by less than half.
- The Jupiter kill by Encounter (22 m) and the Amatsukaze kill by Hatsukaze (320 m) vanish.
  Both shooters were close to the victim, well inside 2000 m.
- The Jintsu kill by Tokitsukaze (4895 m) may survive. A friendly whose line crosses the run beyond
  1000 m, or who is over 2000 m from the gun at launch, is not tested. If it survives it is the
  image's behaviour.
- Houston and Exeter survival rows hold. Gun rows move only where a torpedo mount changed its
  launches, and through RNG-stream coupling in the targets they no longer hit.

## 5. The pair, and the decision

`local/tgO_usn02.log` against `local/tgT_usn02.log`. A third run, `local/tgD_usn02.log`, is the ON
binary plus the per-launch diagnostic line; it matches `tgT` in every summary line, death and gun
row.

| row | OFF | ON |
| --- | --- | --- |
| friendly scans / holds | 0 / 0 | 146655 / 8008 |
| torpedo shots | 346 | 343 |
| deaths | 20 | 19 |
| queued hits | 763 | 759 |
| total damage | 75904.7 | 73299.9 |
| gun rows that differ | - | 47 of 464 |

Verdict per prediction:
- **Holds between 5 and 300: missed high.** The gate is tested every tick of a ready torpedo gun,
  and 8008 of 146655 such ticks held. Launches fall by only 3, because the reload, not the
  gate, limits them.
- **Torpedo shots fall by less than half: held.**
- **Jupiter by Encounter vanishes: held.** Jupiter survives the mission. Hatsukaze, which Jupiter
  had sunk at 240.51 s, now falls to Exeter at 240.36 s.
- **Amatsukaze by Hatsukaze vanishes: failed.** The diagnostic shows Hatsukaze's launches at
  123.15..124.70 s, aimed at Encounter, with two own-side ships within 2000 m and **no** friendly
  line crossing the first 1000 m of the run. Amatsukaze met the torpedo later and further out,
  where the image does not look. This is the image's behaviour under the host's positions.
- **Jintsu may survive: it does not, as allowed.** Tokitsukaze's launches at 128.45..130.00 s
  have one friendly within 2000 m and no crossing; Jintsu dies 4895 m away.
- **Houston and Exeter survive: held.** Neither dies on either side.
- The other moved rows (Haguro, Harusame by 0.35 s, kill ranges by 2..11 m, 47 gun rows) follow
  the three missing launches and the surviving Jupiter.

**Decision: ON.** It is the image's rule. The remaining same-side kills are torpedoes that met a
friendly outside the 1000 m and 2000 m windows the image checks.