# Muzzle offsets applied (packet `cc9_muzzle_offsets`)

`docs/GUN_MOUNT_POSITIONS.md` established where a shot starts:
`TransformAffinePoint(class->muzzleOffsets[gun+44Ch], [gun+3CCh]->worldMatrix)` at
`00730799`..`007307D3` inside `00730160 BSP_Gun_Fire`. `docs/GUN_BARREL_COUNT.md` loaded the offsets
and cached them. This packet reads how the turret's yaw and elevation reach the node matrix, and
binds the shot origin under `kMuzzleOffsetsBound`.

Every name is a hypothesis, not a recovered symbol. Ghidra was read-only.

## 1. The node chain at fire time

| step | address | rule |
| --- | --- | --- |
| 1 | `0072E98A` | `gun+3BCh` is the device model's first node, `[[gun+360h]+160h]+0Ch` |
| 2 | `0072E9A1`..`0072E9AA` | setup installs the platform frame (`0072DD20`, platform+4Ch, a translation in the ship model) into the root through `vtable[38h]` |
| 3 | `0072EE65`..`0072EE8F` | `gun+3CCh` is the first of `"barrel"` (`gun+3C8h`), `"base"` (`gun+3C4h`) and the root |
| 4 | `0085A3B9` | every turning-gun update (`0085A270`, vtable slot `0DCh`) ends in `00859550` |
| 5 | `00859559`..`0085965A` | with a barrel node: root local = RotY(-horz) keeping the root's own row 3 |
| 6 | `0085966F`..`0085974C` | barrel local = RotX(-vert) keeping the barrel's own row 3 |
| 7 | `00859766`..`00859826` | with no barrel node: root local = RotX(-vert) * RotY(-horz), row-3 xyz kept |
| 8 | `007301C3`..`007301E0` | at fire, `00B6DB70` refreshes `[gun+3CCh]`'s world (local times parent world) and the 4x4 at `node+0F0h` is copied |
| 9 | `00730799`..`007307D3` | `004181A0(class+98h, gun+44Ch)`, then `004142E0` transforms it by that matrix |
| 10 | `00730899`..`00730913` | an empty list fires from the root's translation (`node+120h`) |

`00B646E0` builds RotY as rows `(c, 0, -s)`, `(0, 1, 0)`, `(s, 0, c)`; the inline barrel branch
builds RotX as `(1, 0, 0)`, `(0, c, s)`, `(0, -s, c)`. Both take `-0.0f - angle` (`00D7A208`).
With row vectors, RotY(-horz)'s row 2 is `(-sin horz, 0, cos horz)`: a positive image horizontal
angle turns the barrel towards model `-x`.

## 2. A convention finding: the host's horizontal angle is mirrored

The image's gun horizontal angle is `-0.0 - atan2(x, z)` of the local direction: `00521370`
loads `[row]` then `[row+8]` before `_CIatan2` (`005213A7`..`005213AC`), which is `atan2(x, z)`,
and `008FDAF0` subtracts it from `-0.0f`. `00859550` agrees: positive horz turns towards `-x`.

The host computes `want_horz = atan2(dot(dir, right), dot(dir, forward))` and fires along
`forward * cos(h) + right * sin(h)`, with `right` the unit's world row 0 (model `+x`). So the
host's angle has the opposite sign, and the authored `Windows` arcs, which the host loads
unnegated, are mirrored for it: a port sponson authored `+10..+170` degrees can only train to
starboard in the host. This packet does not change it. The muzzle pose uses `-horz` so the barrel
points where the host actually fires.

## 3. The node names: `0071AD50` matches Notes, not node names

The device models name their nodes with a namespace (`150mm dual:base`,
`Hood(Complete):AATurretB(Barrels)`), so no node is literally `barrel` or `base`. `0071AD50` does
not look at node names. It walks the 8-byte pairs of the instance's fifth vector (`+7Ch` proxy,
`+80h`/`+84h`) with `00719FA0`, and the predicate `00718C70` copies `record+8` through `00711C30`
and compares it whole with the key (`004BEB60` at `00718CBF`). It returns the pair's second word.
`record+8` is the string of a `Note` resource: `00719000` allocates a 28h-byte item and reads the
entry's counted string into `item+8` through `00718F50`. Every device model in USN02 carries
`Note` entries `base` and `barrel`, and the Hierarchy item whose `resources` list holds the Note's
index is the named node. For `deruyter_turret.mmod` that is `150mm dual:base` (item 1, under the
root) and `150mm dual:barrels` (item 2, under base, local translation `(0, 1.44, 1.07)`).

The pairing of a Note with the node that lists it is inferred from the data, not read from the
producer of the `+7Ch` vector.

## 4. The host binding

`kMuzzleOffsetsBound` in `src/game_hosts_gunnery.cpp`:
- `read_mmod_hierarchy_items` and `read_mmod_resource_notes` (`src/gun_fire_points.cpp`) load the
  device's Hierarchy and Notes once per device class.
- `turning_gun_apply_angles_00859550` (`src/gun_mount_positions.cpp`) builds the root and barrel
  locals.
- `gun_barrel_muzzle_world_00730762` walks picked node to root, multiplying `local * parent world`
  with the root's parent the ship pose, and applies `gun_muzzle_world_position_00730762` to the
  fired barrel's offset.
- Only the projectile's spawn point moves. The aim, the arc, the intercept, the torpedo gate and
  `CanFire`'s muzzle height keep the mount point, as the image's bots read `gun+FCh`.

Unbound, labelled: the recoil slide of `0085A270`'s barrel loop, the `[gun+3Ch]` slot-8Ch/94h pose
override, planes, and the devices whose model has no `Mesh` or no Hierarchy (they keep the mount,
which is the image's empty-list fallback). The shot direction stays the host's angle direction,
not the node's row 2.

## 5. Predictions, written before the pair

USN02 9200/9000, `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`, OFF `local\mzO` against ON
`local\mzT`, one tree differing only in `kMuzzleOffsetsBound`.
- `muzzle offsets shots` covers most ship shots; mean shift 3 to 10 m, max under 25 m.
- Shots, trigger rises and target assignments are identical: the aim does not read the muzzle.
- Hits and damage move by under 10 percent overall, through the spawn point and RNG coupling.
- Deaths stay within 2 of 19 and Houston and Exeter survive on both sides. Any kill flip is a
  small-origin effect on a marginal hit.

USN04 4700/4500, same binaries and environment, written after the USN02 result and before the
USN04 runs: AA rounds now start at the barrel tips, a few metres from the mount. Predicted: AA
shots within 10 percent, AA hits and plane kills within 15 percent, and the Lexington row judged
only as a knife-edge (it flips with unrelated changes on this mission).

## 6. The pair, and the decision

USN02: `local/mzO_usn02.log` against `local/mzT_usn02.log`. USN04: `local/mzO_usn04.log` against
`local/mzT_usn04.log`. Both trees include the landed torpedo gate.

| row | USN02 OFF | USN02 ON | USN04 OFF | USN04 ON |
| --- | --- | --- | --- | --- |
| muzzle-offset shots / fallbacks | 0 / 0 | 1412 / 0 | 0 / 0 | 2066 / 2051 |
| mean / max shift (m) | - | 5.63 / 10.09 | - | 3.19 / 6.63 |
| shots | 1208 | 1412 | 4113 | 4117 |
| trigger rises | 587 | 874 | 781 | 724 |
| queued hits | 759 | 952 | 473 | 465 |
| deaths | 19 | 18 | 29 | 29 |
| gun rows that differ | - | 255 of 464 | - | 269 of 726 |

The USN04 fallbacks are the plane guns (category 0, 1988 shots), which are not placed.

Verdict per prediction:
- **Mean shift 3..10 m, max under 25 m: held** on both missions.
- **USN02 shots identical: failed.** Shots rise 17 percent. The aim does not read the muzzle, but
  the kills move and the engagement follows them. The largest single cause: Samidare's
  platform-12 torpedo salvo at 184..358 m sank Java and DeRuyter at 142.75 and 144.70 s OFF and
  lands no hit ON. DeRuyter then lives to fire 58, 32 and 30 rounds from platforms 1, 4 and 3
  instead of 26, 3 and 4, and sinks Samidare itself at 146.95 s. Earlier gun kills (Kawakaze
  81.65 to 72.15 s, Yudachi 131.85 to 115.70 s) are the first moves in the death table.
- **USN02 hits within 10 percent: failed** (+25 percent), for the same cascade.
- **USN02 deaths within 2 of 19, Houston and Exeter survive: held** (18; neither dies). 15 of 20
  death rows change time or killer; Perth and DeRuyter survive, Encounter dies.
- **USN04 AA shots within 10 percent, AA hits and plane kills within 15 percent: held.**
  Category-1 shots 1799 to 1820, hits 62 to 67; deaths 29 on both sides; one kill credit changes
  owner at 152 s.

Both runs of each pair are the same tree with RNG streams on, so every moved number belongs to the
origin. USN02 is a close-range surface mission whose kill table is sensitive to any change in the
first torpedo exchange; USN04 barely moves. The USN02 ON run ended with exit code 1 because two
frames failed their Present (9193 presented + 5 skipped of 9200); every mission step ran
(`fixed steps=9000`), so the gameplay rows are complete.

**Decision: ON**, as the image's shot origin, with the USN02 churn stated. It is a one-line revert
if the integrator prefers to wait for a per-hit trace of the USN02 cascade.

## 7. Notes for the next re-baseline

**The two failed Presents on the USN02 ON run.** `local/mzT_usn02.log` has two frames whose
Present returned `D3DERR_DEVICELOST`, each followed by two skipped Presents:

```
51583: native frame complete: frame=5416 ... present=1 hr=88760868 ...
51625: native frame complete: frame=5419 ... present=1 hr=88760868 ...
```

The run reports `frames_presented=9193 presents_skipped=5 ... exit_code=1`. The exit code is 1
because presented plus skipped frames (9198) fall short of `--frames 9200`. The OFF run had one
skipped Present (frame 1, the start-up inhibit) and exit code 0. A lost device is a
session-level event, not a gameplay one. All 9000 mission steps ran on both sides
(`fixed steps=9000`), so the gameplay rows compare. A re-baseline that gates on exit code alone
would reject this run wrongly.

**Samidare's salvo, traced.** A diagnostic, `BSP_MUZZLE_TRACE=<unit prefix>`, prints every
placed shot of that unit. Each line gives the mount, the muzzle, the picked node's name, its
facing (row 2), the shot and target bearings, and the shift split into along the facing, across
it and up. Pending: the run needs a connected session (session 1 was disconnected from 16:01 on
2026-09-25).

What the static data already settles for that salvo: Samidare's platform 12 is device 70,
`jap_quadruple_torpedo_turret.mmod`. It carries a `base` Note (resource 7, listed by item 1
`Quad launcher:base`, which also lists the four `fire` Aux entries) and **no** `barrel` Note.
So the picked node is `base`, the same node that owns the fire points. The no-barrel branch of
`00859550` gives the root RotX(-vert) * RotY(-horz), and `base` inherits it. The four offsets are
`(+1.28, 0.69, 1.41)`, `(+0.42, 0.71, 1.41)`, `(-0.48, 0.69, 1.41)` and `(-1.30, 0.69, 1.41)` in
the base's frame. The tube-mouth shift is therefore 1.41 m along the mount's facing and 0.7 m
up, plus up to 1.3 m across it. The trace will confirm that split on the live salvo.
Every torpedo mount in USN02 has this shape (a `base` Note and no `barrel`); gun turrets carry
both, and `barrel` resolves to the elevating child (for example `150mm dual:barrels`).