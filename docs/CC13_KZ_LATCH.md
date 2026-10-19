# The kamikaze spent latch and `007F16D0`'s PilotFires input (packet `cc13-kzlatch`)

Worker cc13-reltimer, branch `agent/cc13-kzlatch`, 2026-10-19. The pair comes from
`docs/CC13_SQRTB.md` section 4, and cc9 approved it. Both switches are committed OFF in
`include/bsp/cc13_kz_latch.hpp`. The change is host code in `src/game_hosts_units.cpp`. Nothing is
built, recorded or leased in `007EE000-007F2FFF`.

## 1. The two deviations

1. **The spent latch.** `009F8160`'s ordnance arm, `009F81C8`-`009F8205`, takes
   `AL = 007ED790(squadron, 0)` at `009F81CD`. `007ED790` walks the `+3D0h` members, and at `007ED7B3`
   it calls `007B9140(member, 0)`; the first true answer returns 1. `007B9140` asks `vtable[5Ch](17h)`
   at `007B914A` and answers 1 for any `MPlaneKamikaze` (`007B9150 MOV AL,1`) before it walks a rack.
   So `009F81E9 JNZ 009F820E` never latches `+3Ch` for a squadron with a live kamikaze.
   - The host's four loops in the spent-ordnance pass stand for that same `AL`. They are the ordnance
     arm, the fighter arm, the census diagnostic and the dead-target retest. Each one asks
     `plane_holds_rack_round_007b9140` alone, and that function walks only the racks.
   - The other host callers add the 17h arm themselves: `game_hosts_ai.cpp` `record_loadout_00a04560`
     (`007B914A`), `game_hosts_script_orders.cpp` `0089ECBD`, and `game_hosts_lua.cpp`'s `007ED7B3`
     loop.
2. **The `007F16D0` head test.** `007F1701`-`007F1720`: when `+3D0h` answers `vtable[5Ch](17h)` and its
   `+C24h` is 0, the answer is null (`007F18EB`). `record_return_to_base_007f16d0` passes
   `in.head_c24 = false`, although the host carries the byte as `plane_pilot_fires_c24` (`007CD930`,
   `kPilotFiresBound` ON).

## 2. The switches

- `kKamikazeRackRoundLatchCc13Bound` covers three of the loops. Each counts a member that is
  `unit_is_kind_of(m, 0x17)` as holding a round, ahead of the rack walk.
  - The ordnance-arm latch is the behavioural one. It changes no task, because the null answer it
    removes installed nothing.
  - The fighter-end loop is included for consistency. Its leader must be `IsKindOf(13h)`, so it is
    inert for a kamikaze squadron.
  - The census diagnostic only writes log lines.
  - With the switch OFF, the `&&` short-circuits before any call, so the OFF build is unchanged. This
    switch can be paired on its own.
- `kKamikazeDeadTargetRetestCc13Bound` covers the fourth loop, the dead-target retest
  (`kSquadronDeadTargetRetestBound`, `009F820E`-`009F8412`). It is the same `AL`, but it changes
  tasks: a kamikaze squadron whose target dies has its command ended and re-issued.
  `static_assert(!kKamikazeDeadTargetRetestCc13Bound || kKamikazeRackRoundLatchCc13Bound)`.
- `kRtbHeadC24PilotFiresCc13Bound`: `in.head_c24 = slots[head]->plane_pilot_fires_c24`.
  `static_assert(!kRtbHeadC24PilotFiresCc13Bound || kKamikazeRackRoundLatchCc13Bound)` keeps it from
  being flipped alone. With the switch ON, the `returntobase 007F16D0:` line's note also carries
  ` zone-hit` or ` zone-miss`, the result of the retreat arm's `004C7730`.

## 3. Reach: every kind-17h plane in the census logs

`local/sqrtb_kz.py` reads the `unit hull input ... kind=23` lines, the
`spent ordnance ... latched` lines and the `returntobase 007F16D0:` lines from the eleven census
logs (`docs/CC13_STUB_CENSUS.md` section 1).

| mission | kind-17h units | squadrons | classes | latches on them | resolutions on them |
|---|---|---|---|---|---|
| USN21 | 75 | 15 (Kamikaze1-15) | 45, 46, 100, 103 | 15, all under command `00E08F50` at 2767.57 s | 15, all null |
| JM08, USN04, USN01, BSM09, BSM01, IJN08, USN15, USNOS, LOMP06, JM06 | 0 | 0 | - | 0 | 0 |

Every one of the four classes authors `PilotFires = true` on every platform
(`scripts/datatables/autoload/vehicleclasses.lua`):
- 45 Kamikaze A6M Zero: lines 23127-23398, 4 platforms.
- 46 Kamikaze D3A Val: lines 23399-23644, 2 platforms.
- 100 Kamikaze Ki-43 Oscar: lines 44535-44756, 2 platforms.
- 103 Kamikaze D4Y Judy: lines 45583-45848, 2 platforms.

So `+C24h` is 1 for every kamikaze head seen. Among the census logs, every switch reaches USN21
only. Other kamikaze users are known but unverified, since they are not in the census logs: JM13
(`jm13.lua:1012-1032`), LOMP11 Onslaught (KamikazeZero1-3) and competitive11.

## 4. Expected movers (USN21)

**First switch alone.**
- At 2767.57 s, 15 fewer `spent ordnance ... latched` lines and 15 fewer B5 lines.
- 15 fewer `returntobase 007F16D0:` resolutions, so the summary moves from `null=16` to `null=1`
  (JakeTmpl stays).
- 15 fewer `land task REFUSED` lines (the line is logged once per squadron), and the squadrons'
  `refused arm` counts drop from 75 to 0. `spent_rtb_latches` and `spent_rtb_issues` both drop by 15.
- No task changes, unless a kamikaze squadron's target is released while the squadron lives. The
  kamikaze tasks (`kamikaze task 009AF720 kind 0Ch installed ...`) run as they do now, because the
  null answer never installed anything.
- The general step tail (`kSquadronGeneralStepTailBound`) no longer skips these squadrons. This
  switch is what keeps `latch_3c` clear, and the tail skips a squadron whose `latch_3c` is set
  (`game_hosts_units.cpp`, the spent_rtb check ahead of `director_slot_command`).
  - While the kamikaze command `00E08F50` heads the queue below stage 2, `squadron_step_tail_plan_0084e4b1`
    answers nothing. With `+369h` = 1 the idle arm does not resolve, and `b5` needs stage 2 with a
    returning command or the latch.
  - When the kamikaze target is released while the squadron lives, `stage_48` goes to 2
    (`commands.cpp` 3963/3763). The squadron then takes `b4`, and with `+369h` set that gives
    `kMoveToSelfIfRoom`: the tail issues a `0084E595` moveto-self, logged as a `squadron step tail:`
    line. That is the image's tail and the one task change this switch can make. I did not observe it,
    because no run was made.
- When ON, a kind-17h member answers before the rack walk, so the rack-walk counters stop counting
  for it.
- The dead-target retest is NOT part of this switch.

**`kKamikazeDeadTargetRetestCc13Bound` (with the latch switch).** The retest now applies to kamikaze
squadrons, as it does at the image's `009F820E`-`009F8412`. If the kamikaze target dies while the
command is live, the command ends (`0071E430`), `squadron retest:` lines appear, and the squadron
is re-issued. Whether Intrepid sinks in a run decides whether anything moves. The census USN21 log
has no line naming Intrepid as sunk, killed or destroyed, but I did not confirm the pattern against
a known sink line, so treat that as weak evidence.

**Both switches.**
- No further movers are expected in the census logs. With the latch fixed, no path resolves
  `returntobase` for a kamikaze squadron:
  - `0084E552` needs B5 (latch or returning command) or the `+369h`-clear idle arm.
  - `009FFF09` needs a head of class `10h`/`11h`/`12h`.
  - The spawn-time order is `00E08F50`.
- If a kamikaze squadron is ever ordered `returntobase`, for example by a script, its resolution now
  follows the home, site and retreat arms instead of null. With no landing site for side 1 and a
  side-1 border zone always present (`004E7142`'s table), that is `retreat` to the zone centre.

## 5. Uncertainties

- The kind test is the host's `unit_is_kind_of(m, 0x17)`, the same stand-in for `vtable[5Ch]` that
  the other callers use.
- The four host loops take the same `AL`, which the image computes once at `009F81CD`. They are split
  across two switches only so the pairs can be judged; the image has no such split.
- "Both switches -> retreat" depends on `004C7730` finding a side-1 zone. Unlike `side < 0`, side 1
  has no any-side fallback (`world_map_bounds.cpp`: the loop returns at once for side 0 or 1). The
  host's zone table gives side 1 six of its twelve zones, but the USN21 log records no zone result for
  these squadrons. The ON-only ` zone-hit`/` zone-miss` note on the `007F16D0` line will show it.
