# Handoff: the ring is never empty, the merge is bound, and the station is still wrong

Written at the context threshold of packet `cc8_ship_station` (branch `agent/cc8-ship-station`,
worktree `J:\PROG\battlestations-pacific-decompile-cc8-ship-station`, on top of
`agent/cc8-ship-follow@92210a894`). `docs/SHIP_UNIT_GROUP_FOLLOW.md` sections 5e and 5f are the
packet; this is only what is left and what must not be re-derived.

## What this packet settled

* **`00810020 BSP_UnitPoseHistoryRing_Fill` read whole** (section 5e). The wake ring is **never
  empty**: `00822C20 BSP_UnitInstance_SEntityInit` calls `00818EA0` at `00823508`, whose tail fills
  the ring from `&entity+0FCh` and the float vtable slot `+50h` returns, laying forty synthetic
  samples 50 m apart about 1950 m dead astern of the spawn pose. Bound at the spawn point; `calls=62`
  on USN01. **The previous packet's "the ring starts zeroed" is retracted**, and with it both of the
  options the integrator's step 2b offered - there is no guard to find because there is no empty
  ring, and no sample count anywhere in the object for one to test.
* **`0077F940`'s merge arm read whole** (section 5f) and bound: a unit that leads a group brings its
  whole formation when ordered to join another, with the `FormationMaxCount` refusal at `0077F9B4`.
  `formation 1 leader=Northampton` goes from `count=3` to `count=0` on USN01 and
  `formation 1 leader=Yorktown-class01` to `count=0` on USN04.
* **The predecessor's account of the defect is refuted.** `Dunlap` and `SaltLakeCity` never follow
  `Northampton`: the script re-joins them, and `Northampton` itself, to `Enterprise`. Their leader is
  the one ship in USN01 that lays a real wake, so they were never steering for the world origin.

## The defect that is left, and it is the whole of the next packet

`local/follow_merge_usn01.log`: `Dunlap` 600 steps, `err_final` 3826.31, `err_max` 8431.22;
`SaltLakeCity` 600 / 5151.99 / 9375.16; `total_path` 10852.37. Unmoved by either binding.

The two escorts join `Enterprise` from 10700 m away. The join clamps the offset to `FollowerMaxDist`
4000 m, and that clamped point lies far off the end of a 1950 m trail, so `00811180`'s round trip
(section 5d, **lossy off the end of the trail, in the image as much as here**) throws the
along-track component away and answers `along=0.00, across=3152.01`. `0070D290` then rebuilds the
station 3152 m **abeam** of `Enterprise` instead of 4000 m **astern** of it, and the escorts chase
that. The per-join diagnostic line at the column-0 site prints exactly this and is kept in the
source; thirteen lines on USN01.

**Whether the image diverges here is not established.** Its `00811180` picks the nearest sample the
same way, so on the evidence read it would lose the same component. Two things would settle it and
neither was done: the across sign at `00811726-00811760` (four x87 operands), and a reading of what
the image answers for a point beyond the trail's extent. Do not "fix" the station before one of
those is read - the host would then be diverging on purpose.

A second candidate worth one look first: **`008193A0`**, a vtable slot (nine vtables) that takes a
world position and, when `007788B0` says the unit is a formation follower and the global
`[00E188A8]+1FE4h` is not 2, resolves the member record's column through `0070D080` and the leader's
wake through `00810630` and places the unit **at its station** instead of at the requested point. It
is the routine that would snap an authored escort onto station rather than let it swim there. Head
read only (`008193A0-0081948x`); its two other arms and its caller are unread.

## Open, and explicitly NOT done

* **Step 3, the coordinator pair, is HALF DONE.** `local/coord_on_usn04.log` is the *with-fix* run
  (`--frames 3200 --press-start-frame 30 --menu-select USN04 --mission-frames 3000
  --mission-frame-seconds 0.05`, clean `native renderer final COM release`):
  `summary mission ai follow requests=748 available=1 refused=747 joins=1`,
  `summary ship follow steppers=16`, `summary unit formation groups=2 joins=25 creates=2 rejoins=0
  clamped=9 columns_unmeasurable=0`, `total_path=29455.50`. The counters are mission totals, not the
  last batch's. **The *without-fix* half is not run**: it needs the `if (host.ai == nullptr)` guard
  in `create_units` removed, a rebuild, the same USN04 run, and the guard restored. Until that pair
  exists, nothing about the coordinator fix is measured, only observed.
* **Step 4, the landing window, is not started.** No main merge since `8437b2616`, no USN01 torpedo
  trace, no two-way call-table diff.
* **A counting hole in the join path.** `rejoins` was predicted to become 2 after the merge - the
  script's own later orders for `SaltLakeCity` and `Dunlap` should stop at `0077F96E` - and stayed
  0, so those two orders never reach `formation_join_0077f940`. `JoinFormation` is `calls=13` and
  `route_join_message` `calls=10` in every run, before and after, so whatever stops them is not the
  merge. Settle this first: every add is accounted for by a log line, so it is a counting hole, not
  a behavioural one, but it means the join path has an arrival nobody has counted.
* Unchanged from the previous handoff: the across sign, `0077BD70`'s body (so a detach promotes no
  new leader for the group left behind), the `vtable[114h]` / `vtable[58h]` follow-up at `0077FA8D`,
  and the sample yaw rate at `+14h`, which neither `00810020` nor `00815600` writes - a hole in the
  image, not in this host.

## Runs that exist in this worktree

`local/follow_fill_usn01.log` (the fill alone, against the predecessor's `follow_moves_usn01.log`),
`local/follow_diag_usn01.log` (the same build plus the per-join line - this is the one that names
the defect), `local/follow_merge_usn01.log` (the merge), `local/coord_on_usn04.log` (the coordinator
fix on, half a pair). All at `frames=3200 press_start_frame=30 mission_frames=3000
mission_frame_seconds=0.05`, which is what the predecessor used and which the integrator's brief
quoted wrongly as 4800.
