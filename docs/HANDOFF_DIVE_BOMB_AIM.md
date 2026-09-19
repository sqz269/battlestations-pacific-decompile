# Handoff: the aim point has a producer, and neither half of the dive-bomb solution leads

Packet `cc8_dive_aim`, branch `agent/cc8-dive-aim`. Successor of `cc8_dive_goaway`. Read
`docs/DIVE_BOMB_AIM_POINT.md` whole first (about 180 lines) - it is this packet's evidence and it is
written to be read in order. This file is only what that document does not say.

## (a) Settled, and not to be re-derived

* **`009FADA0` is the producer of the dive-bomb aim point**, named
  `BSP_BotApproachTargetRef_Update` in the ledger. It is the per-tick update of a target-reference
  sub-object at `approach+30h` (vtable `00D21CB4`), and `009C7A80` calls it as its first act every
  tick at `009C7A9F`. The aim point `approach+4Ch/+50h/+54h` is that sub-object's `+1Ch/+20h/+24h`,
  which is why five separate censuses of the `approach+4Ch` and `task+444h` displacements found
  nothing - including mine, before I found the sub-object.
* **Neither half of the solution leads the target.** The aim point is
  `target_world_matrix x body_frame_offset` (`009FAEDF`, `004142E0` with `ECX` = the SOURCE point)
  and the predicted impact point is `own position + own velocity x tf` (`009C7D71`, `007BCC80` fed
  only a height). This **retracts** the premise the packet was framed on, that the dive-bomb miss
  against a moving ship is a lost lead. Most of the 6.1-57.0 m is the image's own behaviour.
* **The target chooses the aim spot.** `009FA260` calls `target->vtable[+100h]` and stores the
  returned body-frame point; it is re-picked every 1.5-2.5 s (`00CE3958` = 2.0, re-armed from
  `BSP_Random_UniformFloatRange(-0.5, +0.5)` at `00CE69D0`/`00CE3800`), or when
  `target->vtable[+104h]` refuses the current one, or on the dirty byte `sub+41h`.
* **The host's one real divergence** is that it aims at the commanded target's ORIGIN (the labelled
  substitution at `src/game_hosts_units.cpp` around line 1415) where the image aims at that
  body-frame hull point. The absence of a lead is faithful and must not be "fixed".
* **`009C51D3` is not an unidentified virtual.** It is the approach's vtable SLOT 0, the aim point
  getter: `009C51B1 MOV EBP,[ESI+4]` makes `state+4h` the approach, `009C51C7`/`009C51CA` take its
  vtable and slot 0. So aimglide pull-out arm B's length 1 is `|aimPoint - aircraft|` planar, and
  `[ESP+24h]` is `|aimPoint - aircraft| / |aimPoint - impactPoint|`. One of that arm's two remaining
  slots is therefore closed. **EBP is reassigned at `009C51ED` and `009C524C`**, so any later
  `[EBP+N]` in that body is a different object - that is the trap the slot map must carry.
* **Item 4's cause is the gunnery host being rebuilt per spawn batch**, not a windowed summary and
  not a diverged run. `GameUnitsHost::create_units` ends with
  `host.gunnery = std::make_unique<GameGunneryHost>(...)` (`src/game_hosts_units.cpp` ~3704)
  unconditionally, so each batch discards the summary and every in-flight round. Four diagnoses are
  withdrawn in `docs/DIVE_BOMB_GOAWAY.md` 4b and `docs/HANDOFF_DIVE_BOMB_GOAWAY.md` (a), including
  **my own "the run diverged"**. The fix belongs to packet `cc8-gunnery-host`; I did not start it.
  `docs/HANDOFF_DIVE_BOMB_GOAWAY.md` (a)'s SECOND ATTACK RUN stands - those are dive-bomb task
  counters on the units host, which a batch does not reset.
* **This packet's own run is clear of that defect**: all eight `SpawnNew` batches are at
  `local\aim_before.log` lines 19567-19570 and the first bomb drop at line 27436, so no census row
  was lost to a boundary. The 23 drops against 20 impacts is three rounds still in flight at
  mission end, reported on their own line.

## (b) The one thing to do next, and why it is the whole prize

**Read `target->vtable[+100h]`** - the virtual by which a ship tells a bomber where on its hull to
aim. It is the only unread term left in the image's dive-bomb aim geometry, and it is the exact size
of the gap this packet's measurement leaves: the prediction error is 2.3-21.4 m, the miss against
the target is 6.1-57.0 m, and on a 180-270 m hull a body-frame offset is the right order to cover
the difference that ship movement does not.

### The packet, scoped

**`cc8_hull_aim_point`.** Questions, in order: which classes implement slot `+100h` and slot `+104h`
(start from the ship entity vtables the dive-bomb targets use, and expect the fighter/ship split to
matter); what the four floats at `sub+64h`..`+70h` are, given `009FB200` seeds them `-1.0, 1.0, 1.0,
1.0` (`00D7A260`, `00D7A24C` x3) - a selector plus a per-axis extent is the obvious reading and is
**untested**; what geometry `+100h` samples (a hull box, a named node, a damage section); and what
`+104h` tests, since it is what refuses an offset and forces a re-pick. Deliverable is whether the
chosen point is far enough from the origin to matter at the 10-60 m scale the census measures.

**The torpedo side gets it for free, and this is worth telling that stream.** `009FB200`'s caller
list includes `BSP_BotApproachTorpedo_Reset` (`009D0380`), so the torpedo approach embeds the SAME
target-reference sub-object, and a worker has already observed `009FADA0` running before `009D3517`
"writing only `+1Ch`/`+20h`/`+24h` on its own `this`". That is almost certainly the torpedo's
long-unfound `approach+D0h` writer, sitting at `approach+B4h` (`B4h + 1Ch = D0h`). **I did not
verify it** - it needs one `LEA ECX,[reg+B4h]` in the torpedo reset or update, the exact counterpart
of `009C3EDF LEA ECX,[ESI+30h]` on the dive-bomb side. One call settles it; I ran out of context
before I could and am naming it rather than claiming it.

Scope it as its own packet, because it needs what this host does not have:

* the virtual's slot is `+100h` on the TARGET entity's vtable, called from `009FA260` with an out
  pointer, `&sub+48h`, `&sub+54h` and four floats staged from `sub+64h`..`+70h`. Those six inputs
  are a spread or extent block and none of them is read.
* `009FB200` seeds the offset from `00F87574`/`+78h`/`+7Ch`, which are `.data` **past raw size**, so
  they are loader zero-fill and the seed is `(0,0,0)`. Per the standing rule, that is a statement
  about the image on disk and NOT proof that nothing writes them at runtime - check for a Lua or
  class-table writer before concluding the offset starts at the origin.
* `sub+44h` (= `approach+74h`) gates the tail block at `009FAF0A` against `00D7A218` = 0.0 and is
  initialised to 0.0 at `009FB25F`. Something must set it for that block to run; unread.

## (c) Left deliberately undone, with the reason

* **The body-frame offset is NOT bound in the host.** Binding it needs target hull geometry this
  host does not model, and a guess at the offset would be tuning a constant to move a miss
  distance - the one thing the packet rules forbid. The census this packet adds is what will
  measure it once `vtable[+100h]` is read.
* **The evasive turn in the goaway** (`+24h`/`+28h`/`+2Ch` timers, the bank arm
  `009C4DAF`-`009C4DF6`, the heading arm `009C4E05`-`009C4E1D` via `009C47D0`) is untouched. It is
  still the largest absent piece of `kGoAway` and `docs/HANDOFF_DIVE_BOMB_GOAWAY.md` (b) scopes it.
* **`[ESP+6Ch]` at `009C57F1`**, the path-dependent scratch, is the last open slot of aimglide arm B.
* **`docs/DIVE_BOMB_APPROACH.md` line 332-333 is now wrong** ("the aim point still has no producer
  read") and I did not edit it - that file is not in my lease. Whoever holds it should point it at
  `docs/DIVE_BOMB_AIM_POINT.md`.
* **`docs/HANDOFF_DIVE_BOMB_GOAWAY.md` section (a)'s second-attack-run demonstration** should be
  re-run with `--instance-tag`/`--affinity-core` before it is quoted again; it rests on a log whose
  gunnery side reports no bomb was ever dropped. I did not edit that claim.

## (d) The census this packet added, and how to read it

Print-only, in `GameProjectileRow` and `GameBombImpactRow`. Every field name says WHEN it was
sampled, because the trap it exists to close is exactly a last-sampled column read as a release one
(`docs/HANDOFF_DIVE_BOMB_FLYOVER.md` (b2) flagged the aimdive `tf=` for this and was right to).

| field | sampled |
| --- | --- |
| `release_fall_time` (`tf@release`) | at the RELEASE tick, `009C7D71`'s own `tf` |
| `target_speed_release`, `target_heading_release` | at the RELEASE tick |
| `target_pos_impact`, `target_heading_impact` | as the round DIES |
| `impact_error`, `miss_along`, `miss_across` | the miss against the target AT IMPACT, resolved in the target's own frame |

`miss_along` is positive ahead of the ship on its course, `miss_across` positive to starboard; the
forward axis is `(sin h, cos h)` in `(x, z)`, the convention `009C7C7C`'s `pi/2 - atan2(dz, dx)`
fixes. That axis convention is a LABELLED choice, not a recovered fact.

## (e) The runs, in this worktree's `local\`

| log | binary | what it is |
| --- | --- | --- |
| `aim_before.log` | `5a92141d0` | the census's first run. **Not half of a valid pair** - see below |

**The one thing I got wrong, and the first thing to redo.** I treated
`J:\PROG\battlestations-pacific-decompile-cc8-dive-goaway\local\goaway_after.log` as this packet's
before. It is not: it was built in another worktree at `b7be4aca1`, and this tree is at main
`4e02a7a78`. Every behavioural column moved (`bomb_drops` 19 -> 23, `deaths` 9 -> 14, `total_damage`
10188.4 -> 14042.2, `first_hit` 17.15 -> 13.35 s) and **none of it is evidence about the census**.
The census is print-only by inspection but that is not measured. **Do this first:** revert the
census hunks in this tree, run the same command line, and pair it against `aim_before.log`.

What survives that mistake, and does not depend on the pair, is in
`docs/DIVE_BOMB_AIM_POINT.md` section 5: `tf@release` is 2.38-3.24 s against measured falls of
2.6-3.25 s, which clears `007BCC80`; and the stationary-target control inside the SAME run
(`D3A Val #1.1`, `speed=-0.0 m/s`) misses by 11.3-25.5 m with an along-course component of +3.8 and
+11.1 m, against -45.9 to -77.0 m for the rounds aimed at a target making 16.7 m/s. A
stationary-vs-moving contrast on one binary and one tick is stronger evidence for "the miss is
target motion, and neither side leads" than the pair would have been.

## (f) Two traps this area set

* **A sub-object defeats a displacement census.** Five encodings, both displacements, an exhaustive
  raw-displacement scan and an encoding-independent decompiler sweep over all 90 functions of the
  task all returned nothing, and all of them were right: the writer addresses the field as
  `sub+1Ch`. Before concluding "nothing writes this field", look for a `LEA ECX,[this+N]` in the
  constructor and re-run the census at the sub-object's base. `009C3EDF` was the whole answer.
* **The vtable a constructor installs is not the one that runs.** `009C3EE2` installs `00D20C48`
  and `009C740B` overwrites it with `00D20E08` from the more-derived constructor. Both happen to
  carry `009C40A0` at slot 0, so the aim point conclusion survived - but a claim about any other
  slot taken from `00D20C48` alone would not.
