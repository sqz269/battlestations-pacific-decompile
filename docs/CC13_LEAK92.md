# CC13 leak92: the wreck's leak rates and message 92h (STUB census row 22)

Packet cc13-leak92. The switch is `kLeakMessageRouterDeliveryCc13Bound` in
`include/bsp/cc13_leak92.hpp`. It is committed OFF. Not game-validated.

## Verdict

The census row reads: "The leak rates, including their stream-1 draws, are applied at once
instead of through the session router. This is timing only." It is half right.

- The **draws are not delayed in the image.** `0074EC50` makes them before it routes the
  message (`0074ECA5`, before `0074ED7B`). Only the rates the draws produce wait for the
  delivery. The switch moves no draw, ON or OFF.
- The **rates are one leak tick late in the image.** The delivery is a fixed delay: posted at
  row 15 of step N, delivered at row 9 of step N+1. In between, one leak tick runs with the old
  rates.
- **The delivery overwrites the rates.** `0074E860` copies the message's array over the
  manager's array. Any leak a hit adds to the wreck between the post and the delivery
  (`0074F440`) is lost in the image. The host keeps it today.
- The host makes the draws on one of the two generators it splits image stream 1 into. That is
  a separate substitution, which this packet does not change (see "RNG" and
  docs/CC13_SINKRNG.md).

So the delay does not move a draw directly. It can move a mission only chaotically, through the
wreck's pose: shells that hit wrecks draw from the gunnery stream. Rated **minor**, with a cheap
binding that is now available OFF.

## 1. The image path, post to delivery

| order | row | site | what happens |
| --- | --- | --- | --- |
| 1 | 15 | `009273A0` -> `00926390` -> `vt[7Ch]` | the destroy-list flush reaches the ship's wreck handler `00824B60` (docs/UNIT_DEATH_MESSAGE_AND_SINK.md) |
| 2 | 15 | `00824FE5` | its sink block calls `0074EC50(&unit+10D4h)` |
| 3 | 15 | `0074EC70` | `[00E188A8]+1FE4h == 2` (lobby mode 2) skips everything |
| 4 | 15 | `0074ECA5` | `00BD2F10` with `ECX = 1`, `U(0, 1)`, once per leak (`+14h` leaks) |
| 5 | 15 | `0074ECBD`, `0074ECC5` | `sum += r`; `d[i] = r + water[i]` (`+1Ch`) |
| 6 | 15 | `0074ECD6`..`0074ED18` | `rate'[i] = d[i] * 2cap / (+28h + sum) + rate[i]` into a local array; the manager is not written |
| 7 | 15 | `0074ED21` | `0075B430(92h)`, MT_SHIP_SINK; count and rates copied into the message (`+1Ch`, `+20h`) |
| 8 | 15 | `0074ED7B` | `0077C2A0(msg, 7, 0)`, `ECX = [manager+10h]`, the unit; the local session copies it to `session+24Ch` (`0076E520`), stamped with the current tick |
| 9 | 9 of N+1 | `00875E91` -> `00778450` -> `0076C600` -> `00780670` | the drain; the id resolve `00521E30` finds the wreck (it lives until kill depth) and the generic path reaches the unit's handler |
| 10 | 9 of N+1 | `00821E80`, table `00822394` index `0Dh` (`00822447`) | the arm `0082220D`: `0074E860(&unit+10D4h, msg+1Ch, msg+20h)` (call `0082221B`), then `AL = 1` |
| 11 | 9 of N+1 | `0074E860` | the rate array (`+18h`) is overwritten with the message's copy |

The delay is fixed. docs/SESSION_MESSAGE_DISPATCH.md ("Same-step answer") shows that a message
routed after row 9 is delivered at the next pump. Row 15 is after row 9, so the 92h always waits
one step. The other pump is the extra fixed step `00874D00` (`00874D68`). When one runs between
the two steps, it delivers earlier. The deferred hit drain `00926700` (rows 6 and 14) plays no
part.

The leak tick is `0074F930`: `00933A01` in `009329C0` calls `0074F930`, and `00937622` in the
ship's force callback `00937440` calls `009329C0`. It reads the rates once per step. In the image, the tick that follows the post still
uses the old rates. The next one uses the new rates.

## 2. The host today (OFF)

`ship_wreck_sink_block_00824fe5` (`src/game_hosts_units.cpp`) runs from the host's row 15 flush
(`GameStepSubsystemsHost::flush_pending_entity_queues_009273a0`). It makes the draws and then
writes the rates straight into `s.leaks[i].rate`. The host's motion step follows its fan-out, so
the host's next leak tick already uses the new rates. That is one tick earlier than the image.
A hit on the wreck before the image's delivery adds its leak on top of the new rates, and the
host keeps that leak.

## 3. The binding (ON)

- **Post** (`ship_wreck_sink_block_00824fe5`). The draws and the arithmetic are unchanged. The
  rates go into a pending 92h record (the slot and its six rates) instead of into the slot.
- **Deliver** (`bsp::game::units_deliver_ship_sink_92h_0082220d`). It is called from
  `GameFixedStepHost::pump_session_00778450` after the script-order drain and before the commands
  host finishes its drain. It walks the records in post order and overwrites each slot's six
  rates (`0082220D` -> `0074E860`). The pump is the host's row 9 and the extra step's `00874D68`,
  so both image delivery points are covered.
- **Order with the host's leak tick.** Post at row 15 of N, one tick (in the motion step of N)
  with the old rates, delivery at row 9 of N+1, new rates from the motion step of N+1. That is
  one old-rate tick after the post, as in the image.
- **Lifetime of the ON-only queue.**
  - *Who fills it:* only the sink block. It runs once per ship (`leak_redistributed`), so a ship
    has at most one record.
  - *Who drains it:* only the pump. The drain swaps the queue out first, so a record is
    delivered once.
  - *The slot pointer:* slots are owned by `impl_->slots` and are never erased in the host, so it
    stays valid.
  - *Unit death:* the record is posted after the death, by design. A ship that reaches kill depth
    before the next pump would still be delivered. In the image, the id resolve holds a message
    to a missing entity for ten ticks and then drops it (`007807C4`). Every sink to kill depth in
    the frozen log set below takes 60 s or more, so the case does not arise.
  - *Mission end:* the queue belongs to the units host and dies with it. In the image, the
    loopback array dies with the session. A record left at teardown is not delivered in either.
  - *When the world is inactive,* the host skips its pump as the image skips rows 9-13. The record
    waits, as the image's does.
- **OFF:** none of this runs. The OFF path keeps the direct write and its
  `LeakManager::route_ship_sink_92h` record line.
- **ON log:** `LeakManager::route_ship_sink_92h [0077c2a0]` becomes concrete, and
  `ShipMessage::sink_92h_0082220d [0082220d]` counts the deliveries.

**SUBSTITUTION (labelled).** In the image, the 92h is one entry in the loopback FIFO. The host
delivers all pending 92h records together, after the script orders. No other message writes the
leak array (the 91h cheat has no host), so this order cannot change a rate.

## 4. Reach

The log set is frozen in `reports/cc13_sink_logs.txt`: the 471 bsp_game logs over 100 KB that
were written in `cc9-physics1\local` and `cc9-gunnery52\local` in the 36 hours before
2026-10-19 12:30 UTC, covering 33 missions. These are current main and its pairs. The list was
made by `python tools/cc13_sink_log_scan.py freeze`, and the table below is the output of `scan`
over it. (The first version of this table used a moving 36-hour window, 464 logs; it is replaced
by this frozen set. packet cc13-sinkrng, rev4 N3.) Columns:
- *92h posts*: `LeakManager::route_ship_sink_92h` calls, one per sinking ship;
- *wreck hits*: `applied_wreck`, the 0074F440 leaks added to a wreck;
- *sinkTime*: the range of `sunk ship kill depth ... sinkTime=` over the mission's logs.

| mission | logs | 92h posts min/median/max | logs with wreck hits | max wreck hits | sinkTime (s) |
| --- | --- | --- | --- | --- | --- |
| BSM01 | 28 | 4 / 7 / 12 | 28 | 51 | 74.8-254.6 |
| BSM09 | 61 | 1 / 2 / 10 | 2 | 26 | 86.8-161.3 |
| JM03 | 6 | 0 / 16 / 26 | 5 | 66 | 76.2-130.2 |
| JM05 | 12 | 0 / 1 / 10 | 1 | 21 | 94.2-104.2 |
| JM06 | 12 | 17 / 19 / 20 | 12 | 42 | 65.9-149.3 |
| JM08 | 33 | 1 / 25 / 39 | 26 | 277 | 66.7-161.2 |
| JM16 | 2 | 51 / 51 / 51 | 2 | 136 | 65.0-139.9 |
| PRCP01 | 5 | 9 / 9 / 20 | 5 | 26 | 80.0-115.2 |
| PRCP05 | 6 | 1 / 3 / 5 | 0 | 0 | 96.3-206.9 |
| PRCP07 | 1 | 2 / 2 / 2 | 1 | 10 | 141.8-153.7 |
| PRCP08 | 5 | 1 / 1 / 1 | 5 | 4 | 99.9 |
| USN01 | 88 | 0 / 4 / 7 | 78 | 8 | 68.8-172.9 |
| USN10 | 2 | 24 / 24 / 24 | 2 | 25 | 75.2-161.6 |
| USN11 | 3 | 3 / 3 / 3 | 0 | 0 | 156.4-165.8 |
| USN12 | 2 | 21 / 21 / 21 | 2 | 37 | 80.0-133.8 |
| USN13 | 4 | 2 / 5 / 5 | 0 | 0 | 91.5-162.6 |
| USN15 | 7 | 15 / 23 / 54 | 7 | 100 | 78.5-162.7 |
| USN21 | 65 | 0 / 15 / 17 | 0 | 0 | 60.2-168.9 |
| USNOS | 20 | 19 / 24 / 37 | 20 | 63 | 68.4-169.8 |
| USNRM01 | 2 | 10 / 11 / 11 | 2 | 11 | 99.0-167.1 |

No 92h was posted in BSM04, CHG05, ESMP08, JM01, JM02, JM04, JM13, LOMP01, USN04, USN07, USN09,
USN16 or USN20. Twenty of the 33 missions post at least one 92h.

**Size of the timing effect.** Summed over the leaks, step 6 adds `2cap` in total. For a
destroyer in JM08 (rate_cap 19.38) that is 38.8 water units per second, or 1.9 units over one
0.05 s step, against a final water of about 4300. For a troop transport (rate_cap 411.36) it is
41 units against about 84600. Either way, about 0.05 % of the water, once per ship. The sinks to
kill depth take 60.2 to 254.6 s over the frozen set (the sinkTime column; for example JM08
66.7-161.2 s, USNOS 68.4-169.8 s). The figures for the destroyer and the transport come from
`p24s3_j8_def.log`.

**The overwrite effect.** It needs a wreck hit inside the one-step window after the death, for
example a shell already in flight. The aggregate counts do not show the timing of the hits, so
how often that happens is not measured. Missions with wreck hits are the candidates: BSM01,
JM03, JM06, JM08, JM16, PRCP01, USN01, USN10, USN12, USN15, USNOS and USNRM01. USN21, cc9's
first re-pair mission, has up to 17 posts and no wreck hits, so ON there moves only the one-tick rate
timing.

**Can it move a mission?** Not through a draw (below). Through physics, yes, but only
chaotically: the wreck's water mass, heel torque (`0074F2E0`) and so its pose move by a fraction
of a step's inflow, which can change where later shells hit or miss it. A wreck hit makes
gunnery-stream draws (`Draw::hit_effect`), so once a wreck's pose changes a hit, the gunnery
stream can diverge from there. Kill-depth times can move by a step.

## 5. RNG

- **Image.** The `[manager+14h]` = 6 `00BD2F10` draws (`ECX = 1`) are made at row 15 of the death step, at the
  post. The delivery makes no draw. The window the lead asked about (the draws landing in a
  stream-1 window shared with the kill/aim draws) does not open: the delay moves no draw.
- **Host, ON and OFF.** The draws are made at the same point, the row 15 flush. ON only defers
  the rates, so no draw moves.
- **A separate substitution, flagged and not changed here.** The host does not make these draws
  on its gunnery stream (`GameGunneryHost::Impl::draw` -> `random_range_00bd2f10`). They come
  from the units host's `release_altitude_draw_00bd2f10`. That stream (`db_release_rng`, keyed per
  unit name when `BSP_GUNNERY_RNG_STREAMS=1`) is shared with the plane AI's dive-bomb, torpedo,
  dogfight and take-off draws. In the image, each sinking ship takes `[manager+14h]` draws from
  stream 1 at row 15, one per leak. `0074F490` always sets that count to 6 (`0074F4DD`), so the
  host's fixed six match it. Image stream 1 also carries the gunnery and ship-AI draws, which the
  host keeps on its gunnery generator (H-G). The host splits one image stream into two
  generators, so whichever generator takes the six draws, the other one misses them.
  docs/CC13_SINKRNG.md settles which side the image's thread model favours and adds the OFF
  switch for the other choice (packet cc13-sinkrng, rev4 N1).

## 6. Expected movers with the switch ON

- `LeakManager::route_ship_sink_92h` becomes `concrete`, and a new
  `ShipMessage::sink_92h_0082220d` row appears with the same count, unless a record was still
  pending at teardown.
- Per sinking ship: one step of inflow at the old rates. The `wreck ... water=` and
  `sunk ship kill depth ... t=` lines move by small amounts, and kill depth can move by a step.
- Where a hit lands on a fresh wreck before the next pump, its leak is overwritten:
  `hull_leak_rate_added` is unchanged, but the wreck sinks more slowly.
- Chaotic divergence afterwards, as described in section 4.
- No draw count changes.

## Uncertainty

- The FIFO position of the 92h among the other loopback messages is substituted (section 3); no
  other message writes the leak array.
- `0074E860`'s tail (after `0074E8B0`) is read only for the copy, as its ledger row says.
- Whether host leak ticks run exactly once between the host's row 15 and its next row 9 was
  established from the host's step order, not from a run log.
- `0074EC50` recomputes the total from `+28h`, and the host from the waters (the existing
  0074EC50 ledger note). That is unchanged here.
