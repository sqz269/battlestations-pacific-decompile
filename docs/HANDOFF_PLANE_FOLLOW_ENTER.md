# Handoff: the follow-state ENTRY (packet `cc8_follow_enter`)

Worker `cc8-follow-enter`, 2026-09-19, branch `agent/cc8-follow-enter`. Fourth on the plane follow
law. `docs/PLANE_FOLLOW_ENTER.md` is the real document; this file is only what a cold session needs
to resume.

## State: what is closed

* **`007B8AD0` is the flight-leader test**, not a follow-target test, and the ledger name is
  corrected to `BSP_Unit_IsSquadronFlightLeader` in Ghidra and in `config/names/007b0000.jsonl`.
  `unit+9D8h` is the member's slot in its squadron's `+3D0h` array. Writer census, both store
  forms, and the `007ED610` rotation argument: `docs/PLANE_FOLLOW_ENTER.md` section 1.
* **The moveto/follow choice**, all six sites, dive bomb and torpedo: section 2.
* **How a member leaves follow for the attack**: section 3, including `009D3210` read whole. The
  single best witness in the packet is `009D3241 JZ` - a wing member cannot self-engage.
* **The host feeds it**: one helper `unit_is_flight_leader_007b8ad0` in `src/game_hosts_units.cpp`
  replaced seven hardcoded `true`s. One labelled substitution (no squadron record -> the leader
  answer); section 4 states why.
* Doc withdrawals: `docs/BOT_TASK_STATES.md`'s `unit+9D8h` section, and `docs/PLANE_FORMATION.md`
  section 6's "`009BFEE0`, the station-keeping law".

## The open questions, in dependency order

1. **`009BEE30`'s HOLD arm, `009BEE56`-`009BF9E5`**, still the biggest hole, untouched by this
   packet as by the last one. Recon only: the gate is `009BEE49 CMP byte ptr [ESI+85h],0` /
   `009BEE50 JZ 009BF9EA`, so a member in **good position** takes the hold arm and one out of
   position jumps to the fly-to arm at `009BF9EA`. The arm opens with the lazy pose-matrix refresh
   pattern (`00414DB0` then `00B63D50` guarded by the `+10Ch` dirty byte) applied twice, then
   `0042D0D0` and x87. The function is 997 instructions. Call census in
   `docs/HANDOFF_PLANE_FOLLOW_REGIMES.md`.
2. **The second entry gate, and it is not mine.** `src/game_hosts_units.cpp` hardcodes
   `in.engaged.control_mode_370 = 2` for the dive-bomb transition, so the engaged test of section 3
   is satisfied from the first tick and a member leaves follow before it can fly it. The torpedo
   side does not share the hardcode (`torpedo_attack_mode_370` is maintained). Until that constant
   is fed, the follow state cannot be measured on the dive-bomb path however correct the predicate
   is. Owner: the attack-mode / dive packets, not this one.
3. **Phase A `009C0251`-`009C0EE0`** and the **`009C1552` subtree**, both untouched, as
   `docs/HANDOFF_PLANE_FOLLOW_REGIMES.md` left them.
4. `007B8AD0` for a unit that is **not a plane**. 85 call sites; the non-task ones are HUD markers,
   mission scoring, AI-party resources and gunnery scoring. No writer of `+9D8h` exists outside
   plane/squadron code, so what a ship reads there is whatever its own class puts at that offset.
   Unread, and it does not affect this host, which feeds the predicate only for plane tasks.

## Traps this packet hit, that cost real time

* **A census line a run prints more than once means nothing until you take the LAST one.** I
  predicted "two aircraft enter follow" from the first `plane squadron members:` line in a
  predecessor's log (`1 squadron(s) over 3 member plane(s)`). The last one in my own before-run says
  `39 of 39 wing record(s) resolved to units over 13 squadron(s)`. Same class of error as reading a
  constructor store and calling it the value.
* **`scan-bytes` names the nearest PRECEDING function, which is not the containing one.** The
  `+9D8h` writer at `007ED220` was reported "in `FUN_007ED1D0`"; that function ends at
  `007ED20C RET 4` and Ghidra has no function at `007ED210` at all. `disasm-raw` showed a bare
  append helper.
* **The state-tick census is sampled POST-transition.** `dive_bomb_state_bucket(ctx.current)` and
  the state dispatch both run after the transition rule, so a state entered and left in the same
  arm tick records **zero** ticks and dispatches **no** tick. That is why run A shows no `moveto=`
  entry although every aircraft constructs into moveto, and it is why feeding the predicate alone
  cannot show up in `states[]`.
* **`BotStateFollow::station_keeping` is NOT a witness for the follow state.** It is recorded from
  two sites: the follow tick and the `kDone`/`kPrepare` path. Run A has 795 calls with the follow
  state never entered. The only clean witness is the `follow law` line, printed from
  `run_follow_law_009bfee0_009bee30`, which has exactly one caller - and it prints when
  `db_follow_tick_ticks % 400 == 1`, so a single follow tick still prints one line.
* The `plane formation geometry` line's `pairwise=` column is sampled at the top of
  `place_wing_member_on_station_007f23a0`, **before** the placement gate, so it is genuine drift
  even with placement on.

## Runs

USN04, `--frames 5000 --press-start-frame 30 --menu-select USN04 --mission-frames 4800
--mission-frame-seconds 0.05`, all on this tree.

| run | log | configuration |
| --- | --- | --- |
| A | `local/A_before.log` | before: placement ON, predicate hardcoded `true` |
| B | `local/B_fed_placement_on.log` | predicate fed, placement still ON |
| C | not run | see `docs/PLANE_FOLLOW_ENTER.md` section 6 |

**Landed with `kPlaneFormationPlacementEnabled` ON and the predicate fed.** Eight torpedo wing
members now hold formation (`follow` ~1000 ticks) instead of flying their own attack run, releases
are 35 in both runs, deaths fall 14 -> 10, and every dive-bomber row is identical. `follow law` is
still 0, because the law is wired only into the dive-bomb follow tick and no dive bomber reaches it.

| D | `local/D_mode_fed.log` | B + `control_mode_370` fed (the integrator's granted line) |

**Run D was taken and it regressed; the pin is back**, with D's table in its comment at the line.
`follow law` 0 -> **32** (the law executes for the first time in this chain) and the mutual kills
stay at 0, but releases fall **35 -> 26** and six dive-bomber wing members end the run
`transitions=1 states[follow=1546 flyabove=266] releases=0` - they hold formation correctly and then
run out of mission in **flyabove**, never reaching `done`, so criterion (c) becomes untestable.

**The first thing to read next is therefore NOT the hold arm.** D shows the remaining defect is
downstream of follow: the flyabove arm, or when a member's own in-range latch may set at
`R = approach+B8h = 2080 m`. `control_mode_370` is no longer blocked on the follow entry, and run E
(D + placement OFF) should not be taken until D holds. The hold arm (item 1 above) is the read after
that. Full argument and the criterion (d) baseline: `docs/PLANE_FOLLOW_ENTER.md` sections 6 and 7.
