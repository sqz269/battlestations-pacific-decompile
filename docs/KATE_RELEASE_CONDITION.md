# Why Kate squadrons #2.1 and #6.1 never release

Packet `cc9_kate_release_condition`. Addresses: 009D15F0 (`BSP_BotStateTorpedoAim_Tick`),
009D1F78-009D2287 (the aspect envelope and the five-flag release chain), 00419010, 009FA3A0,
[00CF3F20]. Every name is a hypothesis, not a recovered symbol. Ghidra was not written. No host
code changed, and no pair was run, because no divergence was found.

## 1. Answer

**The conjunct that holds #2.1 and #6.1 closed is the lead flag.** At 009D2034-009D2052 the chain
needs `envelope > range`, where the envelope is the release distance times the aspect scale.

- The release distance is `approach+7Ch` = 450 m once the run has lasted 15 s, and
  `approach+80h` = 650 m before that. The switch is [00CF3F20] = 15.0 s (009D1F78-009D1F99). The
  values come from robots.lua's SPNormal row (`docs/TORPEDO_RELEASE_GATE.md` table, 450 / 650 /
  0.7).
- The aspect scale is `InterpolateClamped(0.5, 1.0, 1.0, approach+84h = 0.7, |cos aspect|)`
  (009D1FED).

**Every one of the eight Lexington Kates is shot down before its range falls below that envelope.**
- They enter the aim state at 2192-2199 m, like every Kate.
- They die 15.7-21.1 s later. Six are killed by Lexington-class01's own guns and two by
  Fletcher-class04.
- Their closest range over the whole run is 536.5-846.8 m. That already includes the dead
  aircraft's glide: power-lost and delayed-explosion aircraft keep flying the aim task at
  throttle 0 until removal, which is why the approach census reports `min=last`.

**The eight Yorktown Kates show what reaching the envelope takes.**
- The lead flag is the LAST of the four flags to open for every one of them. Altitude and bank
  open 42-58 aim ticks earlier, and the cone is open from tick 1 (`timer arm 009D2287 ...
  opened_at`).
- It opens at 442.7-448.2 m, which is 450 m times an aspect scale of 0.984-0.996.
- That happens 19.5-26.1 s after aim entry. The Lexington Kates die 1-10 s short of that.

**The image would hold them closed the same way.** The chain, its constants and the switch are the
image's (`src/torpedo_aim_tick.cpp`, read in `docs/TORPEDO_RELEASE_GATE.md` and
`docs/TORPEDO_FIRST_RELEASE.md`), and here they are met exactly as those documents describe. What
differs between the two targets is survival:
- The Lexington lies stopped at (-12915, -12947), with its own AA and a Fletcher close aboard.
- Yorktown is under way at 16.66 m/s (`target_speed` at the arm), which shortens the run.
- Six of the eight #4.1 and #8.1 Kates were later killed by Northampton-class03 and -class05,
  all after their releases. Two survive.

Whether the ship AA that kills the Lexington Kates is faithful is a gunnery question, outside this
packet.

**The squadron split is not a script difference.** `luaBombersSpawnedLex` and
`luaBombersSpawnedTown` in this installation's `scripts/missions/usn/usn_19_coralus.lua` issue the
same `EntityTurnToEntity` / `PilotSetTarget` pair. Only the target differs. The `issues=1` /
`requests=1` counters on the Yorktown Kates are consequences of their releases, not causes.

## 2. Per-Kate table (V1b, `local\V1b_9000.log`: E2 9000, `BSP_GUNNERY_RNG_STREAMS=1`, the leave-at-death tree)

Column notes:
- "aim entry" is the first aim census tick, at the mission frame time.
- "closest range" is the approach census minimum, which includes any post-death glide.
- "last census alive" is the last velocity census printed before the death.
- "release arm" is the 009D2287 timer arm with its range and time.

| Kate | role | aim entry (s) | range at aim entry (m) | closest range (m) | last census alive (speed, alt, throttle) | release arm | death (s, mode) | killed by |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B5N Kate #2.1 | leader | 186.30 | 2196 | 828.9 | 84.45 m/s, 25.2 m, thr 1.00 | none | 202.01 explosion_delayed | Lexington-class01 |
| B5N Kate #2.1\|.-2 | wingman | 214.40 | 2192 | 747.4 | 91.80 m/s, 46.5 m, thr 1.00 | none | 232.11 explosion_delayed | Lexington-class01 |
| B5N Kate #2.1\|.-3 | wingman | 229.10 | 2199 | 543.5 | 84.45 m/s, 26.8 m, thr 1.00 | none | 249.91 powerlost | Lexington-class01 |
| B5N Kate #2.1\|.-4 | wingman | 226.30 | 2199 | 789.0 | 84.50 m/s, 26.9 m, thr 1.00 | none | 247.41 explosion_delayed | Lexington-class01 |
| B5N Kate #4.1 | leader | 142.90 | 2195 | 15.8 | 84.08 m/s, 27.0 m, thr 1.00 | 442.7 m at 162.40 s | 215.21 explosion_delayed | Northampton-class03 |
| B5N Kate #4.1\|.-2 | wingman | 170.40 | 2196 | 9.9 | 83.61 m/s, 26.8 m, thr 1.00 | 448.2 m at 194.80 s | 246.41 explosion_delayed | Northampton-class03 |
| B5N Kate #4.1\|.-3 | wingman | 168.90 | 2199 | 9.9 | 83.66 m/s, 26.8 m, thr 1.00 | 444.8 m at 193.40 s | 242.96 explosion_delayed | Northampton-class03 |
| B5N Kate #4.1\|.-4 | wingman | 172.50 | 2197 | 4.6 | 83.66 m/s, 26.8 m, thr 1.00 | 444.1 m at 197.00 s | 284.05 explosion_delayed | Northampton-class05 |
| B5N Kate #6.1 | leader | 266.20 | 2196 | 588.7 | 84.45 m/s, 25.2 m, thr 1.00 | none | 281.95 powerlost | Lexington-class01 |
| B5N Kate #6.1\|.-2 | wingman | 294.30 | 2197 | 536.5 | 91.83 m/s, 47.4 m, thr 1.00 | none | 312.10 powerlost | Lexington-class01 |
| B5N Kate #6.1\|.-3 | wingman | 309.00 | 2199 | 548.2 | 84.45 m/s, 26.8 m, thr 1.00 | none | 329.74 powerlost | Fletcher-class04 |
| B5N Kate #6.1\|.-4 | wingman | 306.20 | 2199 | 846.8 | 84.50 m/s, 26.9 m, thr 1.00 | none | 327.29 explosion | Fletcher-class04 |
| B5N Kate #8.1 | leader | 232.60 | 2196 | 2.7 | 73.36 m/s, 12.9 m, thr 1.00 | 446.5 m at 253.30 s | 377.03 explosion_delayed | Northampton-class03 |
| B5N Kate #8.1\|.-2 | wingman | 265.90 | 2198 | 1.8 | 73.38 m/s, 13.0 m, thr 1.00 | 444.4 m at 291.70 s | alive | - |
| B5N Kate #8.1\|.-3 | wingman | 264.70 | 2199 | 6.2 | 73.40 m/s, 13.0 m, thr 1.00 | 443.5 m at 290.60 s | 341.14 explosion_delayed | Northampton-class03 |
| B5N Kate #8.1\|.-4 | wingman | 268.10 | 2198 | 5.9 | 73.38 m/s, 13.0 m, thr 1.00 | 445.0 m at 293.90 s | alive | - |

## 3. What would move these rows (not bound here)

- **AA lethality against a stopped carrier** (gunnery). Every Lexington Kate dies in the last
  third of its run.
- **The run's closing speed.** The Kates fly the aim at 73-92 m/s (last census alive), and a
  moving target that closes on them shortens the run.
- **The aspect scale.** At |cos| between 0.5 and 1.0 it shrinks the envelope toward 0.7 x 450 =
  315 m. It stayed near 1.0 for these runs.
