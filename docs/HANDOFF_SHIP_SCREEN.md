# Handoff: the station was never wrong, and what is genuinely left

Written at the close of packet `cc8_ship_screen` (branch `agent/cc8-ship-screen`, worktree
`J:\PROG\battlestations-pacific-decompile-cc8-ship-screen`, on top of `agent/cc8-ship-station@0f6c4258f`
with `main@3b6277359` merged). `docs/SHIP_ESCORT_SCREEN.md` is the packet; this is only what must
not be re-derived and what is left.

## The headline, and it is a retraction

**There is no station defect.** The previous two packets of this chain were chasing one, and it
does not exist. Three things settle it, all in `docs/SHIP_ESCORT_SCREEN.md`:

1. `00811180`'s tail, unread until this packet, computes `along` as the arc length back to the
   nearest sample with **no extrapolation off the end of the trail**, and `across` as
   `sign * |(point - base) x dir|`, a signed perpendicular distance. The host's
   `ship_ai_wake_decompose_00811180` already computes exactly that. **Nothing needed fixing, and
   nothing was changed** - no constant, no formula.
2. `err_max` is the **first** step's error, not evidence of divergence
   (`src/game_hosts_ship_ai.cpp:4179` takes a running maximum from step one). Rebuilding the
   station from the logged join line puts `Dunlap` 8442.1 m from it at the start against a logged
   `err_max` of 8431.22, and `SaltLakeCity` 9418.1 against 9375.16. The escorts **converge**.
3. The `along=0.00` joins are correct. `Ralph` and `McCall` project 68.9 m *forward* of
   `Enterprise`'s head, and the wake frame has no ahead.

A 3152 m abeam station for a ship the script merges in from 10.7 km is what the image's own
`FollowerMaxDist` 4000 clamp plus its own decomposition produce. Do not "fix" it.

## What this packet settled

* **`008193A0` is `SetWorldPosition`, vtable slot `+118h` in all nine vtables**, with three arms:
  a formation follower is **not placed at all** at any point further than 5.0 m from the origin
  while `[00E188A8]+1FE4h != 2`; one asked for the origin is placed at its **station** instead;
  and an occupant owner re-places every other member of its group at the zero vector
  `&DAT_00F87574` through the same slot. Read whole. Ledger name
  `BSP_UnitInstance_PlaceAtWorldPosition` (provisional), Ghidra renamed, prior value recorded in
  `local/ghidra-annotations-20260919T194044Z.json`.
  * **RTTI is stripped in this image.** The dword before a vtable's slot 0 is padding, not a
    complete-object-locator, so adjacent vtables run together and a walk-back to "the first
    non-`.text` dword" merges them and gives wrong slots. Resolve a base by requiring the base
    address itself to occur as a literal dword in `.text` - the constructor that stores it. This
    is worth a tool the next packet can reuse; it was done ad hoc here.
  * **The integrator's premise for the packet is refuted**: the join does **not** invoke the slot.
    `0077F940`, `0077FAD0`, `0088FFD0`, `0070ED30` and `0070EFD0` are all absent from a census of
    `MOV reg,[base+118h]` over all 24 modrm encodings with the vtable pointer in EAX/ECX/EDX.
    The dispatchers include `00944680 BSP_LuaBinding_Spawn` and
    `00780120 BSP_Session_DispatchEntityMessage`.
* **The join accounting is complete and there is no counting hole.** 13 script `JoinFormation`
  calls, 13 availability asks, 10 pass (`formations=10/13`); the 3 refusals come from
  `same_entity_or_group_00779820`. The units host's `joins=13` is 10 routed + **1 from the AI
  path** + **2 brought by the merge arm** (`src/game_hosts_units.cpp:9283`).
* **`0070ED30` and `0070D290` were already documented whole** in `docs/SHIP_AI_FORMATION.md`
  before this packet, including the three canned column tables and the fact that `group+500h` has
  exactly two writers. Grep that file before touching the member record. USN01 never calls
  `SetFormationShape`, so column 0 is the live column throughout.

## Runs that exist in this worktree

All at `--frames 3200 --press-start-frame 30 --menu-select <mission> --mission-frames 3000
--mission-frame-seconds 0.05`, all with a clean `native renderer final COM release`.

* `local/screen_usn01.log` - the landing run. Bit-identical to the predecessor's
  `follow_merge_usn01.log` on every follow row, as predicted before it was launched.
* `local/screen_usn04.log` - the landing run and the coordinator-fix **ON** half.
* `local/coord_off_usn04.log` - the coordinator-fix **OFF** half, the `if (host.ai == nullptr)`
  guard in `create_units` removed and then **restored**. The pair differs only by the guard and
  both halves are on the same build, which the predecessor's half-pair was not.

## Open, and explicitly NOT done

* **The two-way call-table diff against a `main` build.** A worker cannot create a worktree, so
  there is no `main@3b6277359` build to diff against; sections 6.3 and 6.4 compare against the
  previous packet's logs instead and **do not separate** this branch's effect from `main`'s
  dive-bomb work. This is the packet's one unmet deliverable and the integrator has the tree for
  it.
* **`008193A0` is bound but not wired** into the host's placement path. Nothing in USN01 or USN04
  dispatches slot `+118h` at a unit that is already a formation follower, so wiring it would move
  no measured row. A mission that teleports or re-spawns a ship already in a formation would
  exercise it.
* **The absolute sign of `record+10h`.** `00811180`'s sign is `sign(ST1 - ST0)` at `00811731` with
  five live x87 values (`tools/x87trace.py` puts the depth at 5 there); naming them needs a
  backward walk through the 40-times-unrolled search. It **cancels for column 0** and only matters
  for the canned LINE / COLUMN / DIAMOND columns, which nothing in USN01 or USN04 selects. This is
  the same item the previous two handoffs carried; it is still not urgent.
* **`[00E188A8]+1FE4h`'s state values.** `2` disables every formation behaviour in `008193A0` and
  `1` routes the placement through a session message. What the states *are* is unread.
* **`this+0BCCh = 1.25f`**, written on every placement just below the wake object at `+0BD0h`. No
  reader identified.
* Unchanged from the previous handoff: `0077BD70`'s body, the `vtable[114h]` / `vtable[58h]`
  follow-up at `0077FA8D`, and the sample yaw rate at `+14h` that nothing writes.

## The one thing to be careful about

The USN01 torpedo result changed, and it changed **for the right reason**. Two of five rounds hit
(both on `Northampton`, the one target that still does not move); the three aimed at `Dunlap` and
`SaltLakeCity` now miss by 175 to 210 m because those two ships move 129 to 141 m during the
torpedo's run and the aim carries no lead. The stationary-escort control
(`cc8-ship-command/local/lead_usn01_control.log`) scored 5/5, and the two hits this branch keeps
sit inside that control's own 115-121 deg crossing band. Section 6.3 of the packet doc is the new
reference trace. Do not read the three misses as a torpedo regression.
