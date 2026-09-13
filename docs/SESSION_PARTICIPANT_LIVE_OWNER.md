# Live participant owner for mission entry and ship AI

Packet `orch6_participant_live_l` binds the existing partial
`SessionParticipantPools` core to one persistent `GameMissionHost::Impl` member.
It removes the frame host's separate slot arrays and its unconditional
`device_bound=true` assignments. This is process wiring, not a new full game,
participant constructor, network-session implementation, or native ABI replacement.

## Ownership and inputs

The owner is constructed explicitly with `ZeroedGameAllocation`, based on the
native `0073E163` zeroing of the `71A0h` game allocation. The two subsequent
`004D6BA0` constructor arrays retain the represented bytes. The prerequisite
proof, original-store fixture, pool layout, other reset/claim callers and
availability contract are in [SESSION_PARTICIPANT_AI_FLAG.md](SESSION_PARTICIPANT_AI_FLAG.md).

`GameMissionHost` owns the pools before its Lua/frame members. `GameMissionFrameHost`
borrows a required reference; its `GameShipAiHost` receives that same reference
once immediately after construction and before registration. Reverse member
destruction releases frame/AI before the pools. Creating another frame for a
scene load does not recreate the pools or erase retained bytes. Multiple scene
loads in one process are supported by this ownership arrangement but are not
claimed as runtime-tested here.

`finish_scene_load` now passes the actual selected `SceneRecord*` and a separately
produced, available `record+988h` scalar to `run_scene_load_004dfb70`. The old
semantic `side_blocks` vector is never filled by this executable's header pass;
its empty size cannot establish native zero. Resizing it would also invent the
still-unbound Party/Race values. It remains untouched.

After an actual successful scene read, the existing parsed header must contain
its `properties` bag. Only then does `read_scene_record_slot_table_004f1d70`
produce MaxPlayerNum. `GameMissionHost` retains that scalar under the actual
scene path, and looks it up using the selected record's path at load. Starting
a new read erases that path's prior value, so failed input cannot reuse stale
availability. A present scene without an available count stops explicitly before
the participant reset. A count above eight is also rejected; signed nonpositive
counts retain the existing core's native zero-iteration behavior. No-record
behavior remains the explicit all-eight branch and does not require a count.

The native header calls the property parser at `00469D09`, then calls the applier
at `00469D45` with ECX=bag, EDX=record and one stack argument. The applier retains
bag in EBX and record in EBP through the MaxPlayerNum lookup at `004F2143`
(literal `00CEA450`). A present property stores its dword at `004F214F`; a
successfully parsed absent property stores eight at `004F2157`. This corrects
the older side-block doc's approximate default-store address `004F2145`.
The applier's two exits use `RET 4`; no new parser or ABI replacement is added.

The header first merges its named default bag with keep-existing at `00469D23`,
then applies an assign-existing override at `00469D37`. The tested authored
MaxPlayerNum survives the former. The latter receives the weather bag built by
`0046DF00` at `0046E09F` and passed at `0046E73B`. In the inspected installed
`Scripts/datatables/Weather.lua`, the sole table payload is inside a Lua long
comment, leaving `Weathers = {}`. That input supplies no override assignment.
This is static installed-data evidence, not execution of the original weather
loader. Arbitrary modified weather bags and inherited/default property registry
behavior are outside the new host binding; the supported runtime proof uses
the authored scalar and that empty weather table.

## Exact represented schedule

| Native site | Represented effect |
| --- | --- |
| `004DFD18 -> 004BB160` | Publish all eight active pointers to mission records; reset the two pools' proven `+8/+9` fields using actual scene presence/count. |
| `004DFD57 -> 004BB440` | Claim the first free player record, setting its `+8` byte only within this projection. |
| `004DFD5C` | Publish the returned identity as active slot zero. |
| `004DFD77` | Native local index is zero; the existing local host uses that same index. |

`004BB440` receives nine stack words and ends with `RET 24h`; names, callback
arguments and the other participant fields are intentionally outside the core.
The host does not claim to execute those missing effects. Native stores after
the selected pointer (`+20/+24/+28`) likewise remain outside these three bytes.
The reset and claim never write `+0E`; trailing mission `+9` and all device bytes
retain their previous values and availability. In a fresh zeroed game those
retained device bytes are known zero.

## Consumer-time views

Mission entry constructs `MissionEntryPlayerSlot[8]` from the current selected
identities every time it runs. `excluded` is the same record's `+9 != 0`;
`device_bound` is `+0E != 0`. A missing view prevents the mode-1 count arm from
consuming invented values. It does not change the existing device-arm policy.

The actual `004DB920` listing selects the device arm for `game+1FE4 == 0` **or**
`session+29C != 0` (`004DB93C..004DB953`). Otherwise only mode 1 reaches the count
at `004DB9E0..004DBA2D`: count records with both `+9 == 0` and `+0E == 0`, without
a `+8` test. Old Ghidra comments asserting the opposite session-byte polarity
are stale; the listing and compiled `mission_entry_uses_player_count_arm` agree.
Mode 0 therefore enters through the already represented input edge even with
nonzero unbound count. The existing executable's injected device edge is still
an explicit harness behavior, not proof of real device delivery.

Frame control separately samples selected pointer presence, `+8` and `+9` for
the existing `004C6E50..004C6EC5` inline count fragment inside `004C6E30`. An
unavailable byte stops that consumer explicitly. The fixed-step gate obtains
its local pointer presence from the pool. Its pre-existing `+10` ready-word
projection is outside the pool's three-byte scope and is not promoted to
canonical device ownership by this change.

## Actual device writer and pending delivery

Live listing and original disk bytes agree at `004E0844..004E085E`:

```text
EAX = game[18EC]; EAX = game[18CC + EAX*4];
byte[EAX+0E] = 1; word[EAX+10] = FFFD; byte[EAX+18] = 0;
```

`EDI` reloads the original game receiver from `[EBP+4C]` at `004E07FE`; that
saved receiver was written at `004DFBB5`. `EBX` is re-zeroed at `004E07AA`
after the preceding tree cleanup. `004E080C` skips these writes for mode 0. Mode 2 dispatches its
event arm and also skips the stores; a nonzero mode other than 2 reaches them.
The supported mode-1 interpretation matches the existing scene-load contract.
No mode-1 session delivery is introduced here. The existing
`write_device_bound(record,value)` remains available for a future actual writer.

The readiness stores resolve to the current Ghidra body of `004DFB70`; they are
not a missing function. A failed instruction-start query at `004E07A0` is the
second byte of the two-byte `JZ` at `004E079F`, not a flow gap or function entry.
`004C6E50` is an inline fragment,
not a separate callable entry. No Ghidra mutations or new address annotations
were made in this packet.

## Validation

Build, live host result, precise input/binary hashes and artifact manifest are
recorded in [session_participant_live_owner.json](../reports/session_participant_live_owner.json).
The corrected Win32 build and both existing CTests passed. The actual vendor-XLive
USN01 run completed 120/120 frames and exited zero. It read authored MaxPlayerNum
eight, selected player record zero plus mission records one through seven, and
entered state `0D` with one unbound record on the mode-0 device arm. The same run
logged 2400 avoidance queries, ten refills, twelve clears and 1080 cruise-owner
reads with zero unavailable results. Those cruise reads use current role eight's
short circuit and do not prove an assigned-role participant lookup.

A prior host run that used the empty vector also exited zero, with eight unbound
records. Its exact binary/inputs/log remain under `local/participant_live_pre_correction_*`;
it is excluded as evidence of native count or correct participant AI-byte production.
The final run uses the corrected header-produced count and source hashes.
No new tracked tests or synthetic participant/device defaults were added.
Native-store differential coverage remains the earlier K fixture; this packet's
runtime evidence is limited to the actual host paths exercised by its mission run.
Assigned unit-role message delivery, mode-1 readiness/network startup, other
`118h` fields and the full native constructors remain outside this binding.

## Root integration lifetime correction

`finish_scene_load` now destroys the old Frame before replacing its borrowed
Lua owner. The member destruction order already preserves this relationship
at shutdown; the explicit reset preserves it during reassignment as well.
Participant pools remain owned by GameMissionHost throughout. This is a process
lifetime correction verified by source order; repeated-load runtime validation
remains separate from the worker's single-load mission evidence above.
