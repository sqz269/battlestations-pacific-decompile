# E2's two unattributed deaths (packet `cc9_e2_unattributed_deaths`)

**Answer: both are A6M Zeros that fly into the sea and die by the depth kill. The image leaves
them unattributed as well. No host gap, no switch.**

## 1. The two deaths

The same two die in both logs. `local/fgB_e2.log` is E2 9200/9000 on main `27f082d38`;
`local/rtT_usn04.log` is USN04 4500 on the recon tree.

| victim | time (fgB / rtT) | altitude | damage taken | death |
| --- | --- | --- | --- | --- |
| A6M Zero #4.2 | 86.60 s / 88.45 s | -28 m | none (`first_damage=-1`, every `hits`/`dmg` 0) | depth kill |
| A6M Zero #8.2 | 166.41 s / 168.41 s | -28 m | none | depth kill |

The Zero #4.2 sequence in `local/fgB_e2.log`:

```
18350: plane water contact ignored: unit=A6M Zero #4.2 alt=-1.08 ... up_y=-0.6417 MinWaterSpd=22.222; 007BC5B0 false, so 007CB7F0 returns at 007CB9C2 and the aircraft stays in free flight
18439: plane depth kill: unit=A6M Zero #4.2 alt=-32.88 below -30.0; BSP_MissionEntity_Kill(unit, 1) at 007CE3A7
18440: death row: victim=A6M Zero #4.2 t=86.60 alt=-28 first_damage=-1.00 killer=- ... killer_cat=-1
```

The Zero dives through the surface nose-down (`up_y` -0.64). The water-contact law does not take
it, because `007BC5B0` is false. It sinks below 30 m, and the plane's fixed step kills it with
cause 1.

## 2. The image's kill path for that cause

- **`BSP_MissionEntity_Kill` (`00926D90`) names no attacker.** `__thiscall(entity, int cause)`,
  `RET 4`. It sets `+5Fh`, calls `vtable+70h(1)`, remaps cause 7 to 2, recurses over the child
  list and queues the entity on `00F899B4` for the on-killed dispatch (`009273A0` through
  `vtable+80h`). It reads and writes no attacker field.
- **The credit is `+2C4h`, and only damaging hits write it.** `docs/KILL_CREDIT.md` section 1:
  `0077CE60` runs per hit, returns unless the victim is live and the hit's damage is above 0
  (`0077CEB7`), and then writes `+2C4h` from the shot's owner. `0091BDA0` credits `+2C4h`.
  `00929800` publishes `Dead` and a `KillReason` from the cause, with no attacker.
- **No other producer.** A byte scan for `MOV [reg+2C4h], reg` and `MOV [reg+2C4h], imm` finds,
  among the entity classes, only `0077EFC6` in `BSP_UnitOwnerEntity_Construct` (the initial
  store). The rest are GUI, lobby and ship-AI path objects of other classes. No writer sits on
  the plane tick, the depth kill (`007CE3A7`), the water law or a collision path.

A plane that dies with no damaging hit on it therefore keeps `+2C4h` at its constructor value,
and the kill is credited to no one. Its `KillReason` is the depth cause (1). The host's death
table shows exactly that: `killer=-`, `killer_cat=-1`.

## 3. What remains open (not this packet)

Whether an image Zero dives into the sea at that moment at all. The host logs show `007BC5B0`
false at the surface, the water law skipped, and the terrain avoidance reporting
`min_margin=-27.1`. That is flight behaviour (the Zero's maneuver or its pull-out), and it
belongs to the plane-flight packets. The attribution itself is the image's.
