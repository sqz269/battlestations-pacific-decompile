# CC13 VER 11: the USN01 ConSBD3 aimglide abort and the scope of ctl+394h (cc13-ver11)

Packet cc13-ver11 was a read-only verification done for cc9. It follows `docs/CC13_VER_6.md` and
`docs/CC13_VER_9.md` and asked two questions:

1. Why does ConSBD3's dive flight in USN01 switch from aimglide to moveto/follow at tick 502? Does the image
   do the same?
2. Is unit+9D4h, the block whose +394h is the cruise cap, per squadron or per unit?

Method:

- The bytes were read from disk (`bsp.py disasm-raw`) and compared with the host source.
- No x87 operand order is load-bearing here, so the x87 scanners were not needed. Every decision below is an
  integer or byte compare, or a COMISS.
- Existing logs only; no runs. The log is `J:\PROG\battlestations-pacific-decompile-cc9-physics1\local\p6u1_gj.log`.

Status:

- No new reconstruction. The only code change is a scope comment in `include/bsp/strike_approach_ver_cc13.hpp`.
- No host change. The image matches the host at this edge.
- Nothing here is game-validated.

## Q1: the tick-502 abort is a target loss, and the image aborts too

### What the log shows

| line | event |
| --- | --- |
| `:31025`-`:31041` | ConTBD3\|.-2's torpedo hits Convoy5 (ConSBD3's target, #60), although ConTBD3 was ordered at Convoy6 |
| `:31040` | `death row: victim=Convoy5 t=183.71 ... killer=ConTBD3\|.-2 killer_cat=10` |
| `:31049` | `entity dead: unit=59 "Convoy5" died=183.71 s` |
| `:31056` | `squadron retest: "ConSBD3" target Convoy5 dead ..., command 00E08F20 ended (queue_stage_by_mode, stage 2)` |
| `:31137`-`:31141` | every aimglide member exits with `engaged=0 latch_4c8=0 mode_370=1 tgt_440=0`. The leader goes to moveto, the wingmen to follow |
| `:31386` | the helm script retargets the flight to Convoy2 |
| `:32818`, `:32827` | Convoy2 dies at 191.91 s (ConTBD2\|.-3, gunfire), and the command ends again |

The hand-over summaries (`:156075`-`:156110`, 0.1 s per tick) give the race margins at tick 502:

| aircraft | aimglide entry | at tick 502 | short of release |
| --- | --- | --- | --- |
| ConSBD3 | @485, 468 m | 414 m. All 17 calls blocked at the ceiling (ceiling+50 is about 284-297 m) | about 4 s |
| \|.-2 | @489, 453 m | 415 m, all 13 calls blocked at the ceiling | about 4 s |
| \|.-3 | still in flyabove | 487 m | more than 4 s |
| \|.-4 | @458, 474 m | 288 m. It cleared the ceiling and is held by lead_hi (lead 53.5 m, window -25..-5 m) | about 1-2 s |

After the retarget, the flight re-enters flyabove at 512. It goes to goaway at 513-550, at 557-871 m, which is
too close to dive. Convoy2's death at tick 584 ends that attempt, and the flight keeps its 2 rounds for the rest
of the mission.

### The image's path, from the bytes

1. `009273A0` (FlushPending) sets target+5Dh, +5Eh and +5Fh to 1 at `009274CE`-`009274D6` when the kill is
   processed.
2. `009FADA0` is the first call in `009C7A80`, each tick, on approach+30h.
   - `009FADA6`-`009FADB9` tests three things: sub+18h (the target) is set, sub+14h is set (the target when it
     is IsKindOf 5, stored by `009FB190`), and `[[sub+14h]+5Dh]` is set.
   - Then it freezes the aim point and nulls sub+14h and sub+18h (`009FAE0A`, `009FAE11`).
   - sub+18h is approach+48h, which is task+440h.
   - The observer callback `009FAF90` does the same on destruction.
3. `009C7B03` `CMP [ESI+48h],0 / JE 009C7E91` -> `009C7E91` `MOV byte [ESI+0D0h],0`. With no target, the range
   latch task+4C8h is cleared unconditionally.
4. `009C83E0` tests attacking && !(task+4C8h || (ctl+370h == 2 && task+440h)) at `009C83F8`-`009C8417`.
   - ctl+370h is 1, because `0099B740` assigns 1 on every leader think.
   - With both the latch and the target gone, `009C8419` `007B8AD0` sends the leader to moveto+4F0h and each
     wingman to follow+52Ch.

The host reaches the same states on the same tick by a different route:

- The cc9-gunnery38 retest (`game_hosts_units.cpp:15307`-`15332`) clears `command_target_plus_one`.
- Then `:4291`-`4293` (`009C7B0A`) clears the latch, and the transition follows.

### Divergence found (not on this path)

| where | image | host | effect |
| --- | --- | --- | --- |
| `009FADA6`-`009FAE18`, `009C7B03` | every approach drops a dead target, unconditionally | a dead target is cleared only inside the retest (`game_hosts_units.cpp:15307`-`15315`), which needs an ordnance command class and a held rack round | a spent dive or torpedo task, or a non-ordnance approach, keeps aiming at a dead hull in the host. No effect on ConSBD3, which held bombs |

`game_hosts_units.cpp` is leased (lua56), so this is an adoption note.

### Prediction

- Nothing needs fixing at the abort, so ConSBD3 still releases 0 on this sequence.
- A release needs the dive to finish about 1-4 s earlier, or Convoy5 to survive longer. Both depend on upstream
  timing:
  - `CC13_VER_6` finding 4 and `CC13_VER_9` findings 1 and 3 lower the aimglide entry.
  - `CC13_VER_9` finding 1 also changes when the TBD flights arrive: the torpedo moveto cruises at 1450 m
    instead of 500 m.
- Static analysis cannot decide the race. The most likely gain is \|.-4, +1.

## Q2: unit+9D4h is the squadron

- `007B97E0` BSP_Plane_GetSquadron is `MOV EAX,[ECX+9D4h] / RET`, at plane vtable slot +140h.
- `009F9CE0` copies [unit+9D4h] to approach+0Ch (`009F9CF6`-`009F9CFC`).
  - The dive approach is at task+3F8h, so task+404h is the squadron.
  - `009C83E0` reads ctl+370h through the same task+404h, and the host already treats +370h as the squadron's
    (`game_hosts_units.cpp:19848`).
- `007F2BD0` is called from the squadron constructor `007F2C60` with ECX = squadron+37Ch.
  - It sets +394h, +398h and +39Ch to tuning+53Ch (1000).
  - It sets the countdowns +37Ch..+388h to -1.0 (`00D7A260` = `00 00 80 BF`).
- Both profile writers run for the flight leader only, and both write into the shared block:
  - `009C8920`: `009C8931` `007B8AD0`, then `009C8938` JE `009C8A1B`. With +38Dh clear, +380h < 0 and +3A9h
    clear, it sets +394h = tuning+4C0h (`009C8964`/`009C896E`) and +3ADh = 1.
  - `009D4A70`: `009D4A82` `007B8AD0`, then `009D4A8B` JE. The same gate block follows at `009D4AC8`.

So a wingman's `009FBA9B` reads the leader's value: 1300 for a dive, 500 for a torpedo. It reads 1000 only before
the leader's first profile tick.

- `CC13_VER_9` finding 1's adoption note stands. Feed every member from the squadron slot (`squadron_slot_of`,
  which already carries `sq_alt_394` for the Lua writer `0089F550`), with the flight leader's task kind.
- The "a wingman reads 1000" caveat in `CC13_VER_9`'s uncertainty section no longer applies.

## Handoff

- **For cc9/lua56.**
  - No change at the tick-502 abort.
  - Optional: drop a dead target for every approach, not only inside the retest (`009FADA0` dead path).
  - Wire the ctl+394h cap per squadron.
- **Open.**
  - Whether ConSBD3 wins the race against Convoy5's death, once ver6 #4 and ver9 #1/#3 are adopted. That needs
    a run.
