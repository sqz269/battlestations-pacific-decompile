# The fighters' gunFire consumer (packet `cc9_fighter_gunfire_consumer`)

The packet asked why the US fighters' rounds stay 0 in this host, citing
`docs/E2_RELEASE_BISECT.md` line 50 ("the gunFire consumer is not established"). On the current
tree they do not stay 0. The consumer was established and bound before this packet. Nothing is
bound here.

## 1. The consumer is established and hooked

`docs/PLANE_GUN_PASS.md` section 2 reads it. The plane's fixed step `007CE040` latches
`unit+BC9h = unit+9FAh` (`gunFire`) at `007CE96F`. It then walks the unit's parts, and for each
`MRFSGun` (class `23h`) calls `gun->vtable[1E8h](gunFire)` (`BSP_Gun_SetTriggerHeld`, `0072D2C0`)
at `007CE9F4`, when that part's weapon group is enabled. From there the round comes out of the
same path as any ship gun: `0072D130`, `0072D860`, `00727E30` `FireIfReady`, `0085A830`
`CanFire` and `00730160` `BSP_Gun_Fire`.

The host binds this under `kPlaneGunfireHooked` (ON): a category-0 gun on a plane takes
`units.plane_gun_trigger_bc9(owner)` as its `want_fire` (`src/game_hosts_gunnery.cpp`). The
weapon-group enable byte is taken as set.

## 2. Measured on the current tree

E2 (USN04 9200/9000, idle player), main at `27f082d38`, `BSP_GUNNERY_RNG_STREAMS=1`,
`BSP_DEATH_TABLE=1`, `local/fgB_e2.log` (module directory `local\fgB`):

| row | value |
| --- | --- |
| plane-gun trigger ticks / rounds | 1908 / 940 |
| fighter (category 0) rounds / hits / damage | 940 / 86 / 1184.8 |
| deaths by killing category | 0 (fighter guns): 5, 1: 13, 5: 2, 6: 15, none: 2 |
| fighter kills | D3A Val #1.1\|.-2 (99.30 s), #1.1 (99.40 s), #1.1\|.-3 (103.45 s), movieval (109.85 s), movieval\|.-3 (113.70 s) |

The dogfight worker's `THR1_9000.log` agrees: 1946 plane-gun rounds, all from category-0 guns, and
96 hits. So both parts of the premise are out of date. The consumer is established, and the
rounds are not 0. A dated correction is appended to `docs/E2_RELEASE_BISECT.md`.

## 3. What is actually true about the Kates

- No fighter damages a Kate. Every B5N death row reads `hits c0=0`. The fighters' bursts all go
  to D3A Vals (6) and to the movieval flight (3).
- The Kates die to ship AA: categories 1, 5 and 6.
- The AI planner does order fighters onto a Kate group:
  `ai diag order_attack group_members=12 group_leader=Lexington-class01_sqn01 target_members=6
  target_leader=B5N Kate #6.1`. No burst ever follows it.

So the open question is the engagement after that order, not the consumer. It could be the
dogfight's range or latch, the finder `007E2090` (still unread), or the order's timing against
the Kates' run-in. That is dogfight and planner territory, and it is not started here.
