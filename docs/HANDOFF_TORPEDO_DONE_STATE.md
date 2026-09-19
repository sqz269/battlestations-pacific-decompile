# Handoff: what a bot task does when it reaches `done`

Written by packet `cc8_torpedo_retire` at the end of its context, for the next worker. Nothing here
is a conclusion about the image; it is the question, what is already known, and the entry points, so
the next worker starts from the listing rather than from this file.

## The symptom this opens from

`docs/TORPEDO_AFTER_THE_DROP.md` section 14 closed the spent-bomber chain: a torpedo bomber now
drops, climbs away on the goaway, and the task reaches `kDone` when the range opens past 700 m.
Measured on USN01 at 3000 mission frames, all five reach it and then sit in it for 313 to 385 arm
ticks - a quarter of the mission.

**In `kDone` this host commands nothing at all.** `run_torpedo_task_arm_009d4850` keeps calling the
arm, and the per-state tick chain in `src/game_hosts_units.cpp` has branches for `kAim`, `kGoAway`,
`kMoveTo`/`kFollow` and `kAttackRun` and **no branch for `kDone`**, so the aircraft holds whatever
the goaway last commanded: its break-off heading and a 500 m climb, for the rest of the mission.
That is almost certainly not what the image does, and nobody has read what it does instead.

`009D4030` itself is not the answer: `009D4097` returns `kNone` for a task already in `kDone`, ahead
of the break-off test, so the state machine deliberately parks. Whatever happens next is **outside**
the task's own transition rule.

## The question, in three parts

1. What does the `done` state object tick, if anything?
2. Who notices that the task is finished, and what replaces it?
3. What is the next behaviour for an aircraft with no ordnance - does it return to the carrier, and
   is that a landing task?

## Entry points, all already in hand

**The state objects are at fixed task offsets, and `bsp::TorpedoState`'s enum values ARE those
offsets** (`include/bsp/torpedo_task_arm.hpp`): `kMoveTo = 0x544`, `kFollow = 0x580`,
`kDone = 0x618`, `kAttackRun = 0x6B4`, `kGoAway = 0x6D8`, `kAim = 0x710`, `kPrepare = 0x740`. So the
done state is the subobject at `task+618h`, and the arm's current-state pointer is `task+310h` -
`009D30CC` `MOV [ESI+310h],ECX` writes it in the constructor, with `ECX` = `task+544h` (moveto) or
`task+580h` (follow) chosen by `007B8AD0` at `009D30B7`. Read the vtable that `task+618h` gets and
what its tick and enter slots do; the sibling `009D0D90` (the goaway's `+4h` enter) shows the shape.

**Where a completion would be consumed.** The arm is not called from the task; `009998A0`'s slow
path runs it, and the sequence is recorded in `src/game_hosts_units.cpp` next to the host's call
site: `0099B740`, `task->vtable[64h](dt)`, `009FC7C0`, `009FD0E0`, `009A17D0`, then `0099D300`.
`009998A0` is therefore the first place to look for "the task is finished, do something else".

**The likely channel, and it is already reconstructed.** Task vtable slot `+50h` is the three-value
step result - `009A5D80` for the depth charge, `bot_task_step_result` in `src/bot_tasks.cpp` -
returning approaching / attacking / attacking-flagged. If the bot polls that to decide when a task
is spent, the `done` state's contribution to it is the thing to read. `docs/BOT_TASKS.md` owns the
vtable map.

**Returning and landing.** `Pilot/Landing` has its own reference speed row at tuning `+52Ch`
(KMH(140)), read by `009AFE70` and `009AFFF0` (`docs/BOT_TASKS.md`), so a landing task class exists
and has a constructor pair like every other class. Start from `docs/AIROPS_*.md` for what is already
reconstructed about returning and landing before reading those two.

## Rules this packet learned the hard way

* **Read before binding.** Both defects section 14 fixed were host bindings that re-stated an image
  condition slightly wrong - a per-tick test of an install-time condition, and an input fed `false`
  because its NAME said "squadron" while the listing said `approach+0Ch` (`= unit+9D4h`, the pilot
  control block). The ledger had the second one right all along.
* **Judge by the real state field.** `done_last` in the goaway census is `009D3150`'s predicate on
  the last tick, not the `kDone` state; reading it as "the task retired" cost a run.
* **A census beats a suspect.** Item 1 of this packet was handed over with a leading suspect that the
  latch refuted in one run.
* USN01 runs land in about five minutes tonight, outlast the foreground cap, and are deterministic:
  the same binary gives identical log line numbers, so an additive-logging build can be checked
  against the run it is meant to reproduce.

## Files and leases

No lease is held for this work. The torpedo hunks of `src/game_hosts_units.cpp` are under the
integrator's hunk arbitration of 2026-09-19; `docs/BOT_TASKS.md` and
`docs/TORPEDO_AFTER_THE_DROP.md` are the documents this would extend.
