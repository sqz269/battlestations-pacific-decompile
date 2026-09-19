# After the drop: the five deaths are the five bombers, and the torpedoes hit nothing

Packet `cc8_torpedo_after_the_drop`, owner `agent/cc8-plane-squadron`. Parts (1) and (2); part (3),
the goaway tick `009D0F10`, is handed on at the end.

Run: `local/aimclass_after_usn01.log`, the same run `docs/TORPEDO_RELEASE_TIMER.md` validates.

## 1. Who died, and it is not a torpedo kill

**`deaths=5 kill_credits=5` are the five Mavs, shot down by the American ships.** The run's own
per-unit table says so by name:

| unit | side | taken | health | `sunk_at` | `killed_by` |
| --- | --- | --- | --- | --- | --- |
| Mav1 | 1 | 450 | 0 | 133.50 s | **Dunlap** |
| Mav2 | 1 | 450 | 0 | 136.95 s | **Northampton** |
| Mav3 | 1 | 450 | 0 | 109.15 s | **Northampton** |
| Mav4 | 1 | 450 | 0 | 134.25 s | **SaltLakeCity** |
| Mav5 | 1 | 450 | 0 | 109.80 s | **SaltLakeCity** |

**No ship sank.** Every other row in that table carries `sunk_at -1.00`, including the three ships
that did the killing and the `Enterprise`.

The damage total decomposes exactly, which is what makes this a reading rather than an inference:

```
five Mavs        5 x 450 = 2250.0
SaltLakeCity              347.0
                        --------
                         2597.0   against summary total_damage=2596.8
```

and `SaltLakeCity`'s 347 is in turn exactly the sum of what the side-1 **guns** dealt: `CB2` 127 +
`Coastal Gun 03` 79 + `Coastal Gun 02` 53 + `Coastal Gun 01` 88 = 347. **Not one point of the
mission's damage is attributable to a torpedo.** `hit_records=111` and
`projectiles ... entity_impacts=111` agree, and 111 is the gun-hit count.

So the correct sentence about this run is: *the five bombers reached their release point, dropped,
and were then shot down by the cruisers and the destroyer they had attacked.* Anyone quoting
`deaths=5 kill_credits=5` as a result of the torpedo work is quoting the AA gunners.

## 2. What the five torpedoes did

All five entered the water and swam:

```
gunnery: torpedo drop 1 by Mav3 at 12 m, speed 73.5 m/s, bullet 69, swim 30.9 m/s
gunnery: torpedo drop 2 by Mav2 at 12 m, speed 73.6 m/s, bullet 69, swim 30.9 m/s
gunnery: torpedo drop 3 by Mav5 at 12 m, speed 73.7 m/s, bullet 69, swim 30.9 m/s
gunnery: torpedo drop 4 by Mav4 at 12 m, speed 73.7 m/s, bullet 69, swim 30.9 m/s
summary mission gunnery torpedo_drop drops=5 refusals=0 water_entry_breakups=0
summary mission gunnery ... swims_started=5 snaps=0
```

| quantity | value | source |
| --- | --- | --- |
| release altitude | 12.0 to 12.1 m | `approach+78h` `TorpReleaseAlt`, `SPNormal` |
| release speed | 72.9 to 73.7 m/s | against `MaxWaterHitVel` 100, hence 0 breakups |
| swim speed | 30.9 m/s | the round's `+470h`, `WaterTravelSpeed * 0.5999994277954102` at `0085786D` |
| release time | about 78 to 88 s | drops 1 and 2 bracket `gunnery step 1600 t=80.00` |
| release range | about 700 m | the aim census reads 1084 m at aim tick 151 and the timer fired at aim tick 253, 102 ticks and about 372 m later |
| aircraft death | 109 to 137 s | 21 to 49 s **after** their own drop |

The host's swim is not a stub: `src/game_hosts_gunnery.cpp` levels the round onto the surface plane
at the swim speed, keeps the launch heading, disables gravity through the flight state's own
`classDesc+20h` flag rather than by stepping the round outside `projectile_flight_step`, and lets it
continue through the same per-step entity sweep as any other round. A swimming torpedo in this host
therefore **can** hit. These five did not: `entity_impacts=111` are all gun rounds, and no damage
record attributes to a torpedo.

### Why they missed is not yet established, and here is the bound on it

What is measured: released about 700 m out, swimming 30.9 m/s, so about **23 seconds** of run. What
is not read: whether anything in the chain **leads** a moving target over those 23 seconds.

* `009D1360` writes `approach+A0h`, the fall lead `fallTime * v0` - the distance the torpedo carries
  forward while it falls - and `approach+98h`, the total run time to impact. `009D3D12` reads `+98h`
  as the bias of the engagement estimate at `+F8h`. Whether either reaches the commanded **heading**
  is unread.
* The aim tick's commanded heading is `approach+94h`, the bearing to the plan target at
  `approach+ACh`/`+B0h`, plus the sector turn offset `approach+5Ch`. Whether `+ACh`/`+B0h` is a lead
  point or the target's current position is the question, and it is `009D39xx`'s (the replan) rather
  than this packet's.
* The three ships were under way: `Northampton` shows `nearest 18` and `SaltLakeCity` `nearest 17`,
  so the geometry closed to metres. A straight-running torpedo aimed at where a ship **was** 23
  seconds earlier misses by roughly the ship's speed times 23 s.

**Do not read the miss as a defect in the release chain.** The release chain did what the listing
says: correct altitude, correct speed, no breakup, a swim at the authored speed. The next question
is an aiming question, one state earlier.

## 3. Handed on: the climb-away, `009D0F10`

The five aircraft still touch the water, but 21 to 49 s after dropping, at 69.6 m/s and level
(`alt` -0.02 to -0.10), and the table says they were **already dead** by then or dying: `sunk_at`
109.15 to 136.95 s against water contacts in the same window. So the water contact after the drop is
now entangled with being shot down, and separating the two needs the climb-away bound first.

`009D0F10 BSP_BotStateTorpedoGoAway_Tick` is the state that flies it: three **direct** `009FB800`
calls at `009D109C`, `009D10FF` and `009D1194` - not through `009FBA50` - each with a literal `1.0`
as the second argument (`009D107A FLD1` / `009D1084 SUB ESP,8` / `009D1087 FSTP [ESP+4]` covers the
first two, `009D1158 FLD1` the third), capping the climb-out at `class+1ECh * 1.0` = 0.1854 rad for
the Mav. This host runs no goaway tick at all.

It is worth doing next because it exercises the **climb** arm of `009FB800`, which nothing in this
stream has driven: every measurement so far has been of the dive arm. `class+1ECh` is derived rather
than authored (`007C4C08`-`007C4C14`, 0.6 times `desc+1E4h`), so the climb arm also tests a field
this lineage has only ever read.

## Coverage

| question | coverage |
| --- | --- |
| who died and what killed them | complete, from the run's per-unit table and an exact damage decomposition |
| what the torpedoes did to water entry and the start of the swim | complete |
| what ended each torpedo | **partial**: established that none hit and that the host's swim can hit; which of range expiry or geometry ended them is not separated |
| why they missed | not established; bounded above to an aiming question in the replan or `009D1360`'s lead, not the release chain |
| `009D0F10` | not started, handed on |

## Uncertainty

* The release range of about 700 m is interpolated from the aim census at tick 151 and the timer's
  first fire at tick 253, not read from a census printed at the release instant. The release census
  prints altitude and speed but not range; adding range to that line is the cheapest way to make
  this exact.
* `projectiles ... expired=885` is a whole-mission figure over 1119 rounds; the five torpedoes are
  not separated out of it, so "they expired" is a plausible end rather than a measured one.
