# Participant AI-held byte and active record ownership

Packet `orch6_participant_ai_k` supplies `SessionParticipantPools`, a narrow
owner for the two native participant pools and their active slot identities.
It implements the relevant reset, claim and pointer stores, and the actual
`00927F10` byte read. The represented fields are `+8h` claimed, `+9h` AI-held,
and `+0Eh` device-bound. Other fields of the `118h` records are explicitly
outside this projection. No MissionFrame, Units, ShipAI or Lua host is changed.

## Producer and lifetime

There are eight player records at `game+748h` and eight mission records at
`game+1008h`, both with stride `118h`. The eight pointers at
`game+18CCh..18E8h` select current records. Pool identity and slot index are
separate: installing player record 3 into active slot 5 does not turn it into
mission record 5, and a slot's Party is not its participant-record identity.

The actual zero producer is the application allocation sequence:
`0073E155` allocates `71A0h` bytes; `0073E163` calls `_memset` with that
pointer, EDI=0 and size `71A0h`. `ADD ESP,10h` at `0073E168` cleans the three
memset arguments plus the allocation size left on the stack. The complete
`0073D410` listing has only `XOR EDI,EDI` at `0073D433` and the final restoring
POP as writes to EDI. Thus both pools' `+8/+9/+0E` start at zero and the active
pointers start null.

The game constructor invokes the element constructor `004D6BA0` through
`00BF7CD1` at `004DDD6C` and `004DDD8E`, with count 8 and stride `118h`, once
for each pool. The element constructor does not overwrite `+8/+9/+0E`.
The new owner's explicit `ZeroedGameAllocation` initialization projects only
these produced bytes and null identities. Its default initialization instead
marks bytes unavailable. It does not claim to execute the full game or
participant constructors.

## `004BB160` reset

The complete inspected native body is `004BB160..004BB3DB`, ECX=game, no
stack arguments, `RET` at `004BB337`. It has no calls. Its first action
publishes all eight mission-pool pointers (`004BB169..004BB1C3`).

| Condition / record range | `+8h` | `+9h` | Evidence |
| --- | --- | --- | --- |
| Scene exists, records `[0, signed scene+988h)` | 1 | 1 | `004BB21C/004BB220` |
| Scene exists, trailing records through index 7 | 0 | retained | `004BB2D0` writes only `+8h` |
| Scene absent, all eight mission records | 1 | 1 | `004BB36F/004BB373` |
| Every player-pool record, both arms | 0 | 0 | `004BB30B/004BB30E` |

All `+0Eh` bytes retain. The loops use an interior cursor (`record+0Dh`);
the AI stores address `[cursor-4]`. Searching only stores to `[record+9]`
misses them. With a scene, a signed count <=0 populates no mission records.
The native count is not capped to eight; the typed owner rejects counts >8
without mutation because its two pools have exactly eight represented records.
When no scene is present, the count input is ignored.

Other reset stores were inspected but are not implemented here. Populated
mission records set `+0Ah=1`, `+0Ch=0`, `+0Dh=1`, `+10h=FFFDh`, `+18h=0`,
`+1Bh=0`, `+1Ch/+20h=index`, `+34h=0`, two names, and `+24h/+28h` from the
scene blocks (or zero on the no-scene arm). The player loop resets the same
control fields, `+0Ah=0`, `+20h=-1`, `+2Ch=3`, and the routine resets
game `+18C8h/+18ECh` to -1. None of those writes is simulated by placeholder
record fields in this core.

All four direct reset call sites were checked: `004DFD18` in the local scene
load; `004E350F` in `004E27E0`; and `0076FCB1/0076FD50` in session mode change
`0076FAD0`. The reset API carries actual scene presence/count rather than
assuming every caller is a loaded local mission.

## Claim, selection and restoration

`004BB440..004BB541` has ECX=game and nine stack words (`RET 24h` at
`004BB467/004BB53F`). Its first-free scan examines player-pool `+8h` in index
order. It returns null after eight claimed entries, otherwise writes `+8h=1`
at `004BB47D`. It does not change `+9h` or `+0Eh`. The new claim method covers
only this selection and claimed-byte store. Names, pointer fields, sign-in
data and the user callback are outside coverage.

The local load calls reset at `004DFD18`, calls claim at `004DFD57`, and only
then stores the result at `game+18CCh` at `004DFD5C`. Since reset just cleared
all eight player claimed bytes, the first claim selects player record 0.
`reset_and_claim_local_flags` preserves this relevant order. It does not
invent any of the claim's nine arguments or the following Party/Race stores.

`004BB630` publishes a supplied record pointer at `004BB638`, then writes
record `+20/+28/+24`; ECX=game, record and slot stack arguments, `RET 8` at
`004BB65C`, inclusive end `004BB65E`. Its partial typed method performs only
the active identity store. `004BB660` first dereferences the old record to
write `+20=-1`, then restores the same-index mission pointer at `004BB681`;
ECX=game, one slot argument, `RET 4` at `004BB688`, inclusive end `004BB68A`.
Its typed method requires an existing identity and restores the mission
identity, leaving the unrepresented `+20` side effect explicit.

Neither pointer-changing routine changes the selected record's `+9` byte.
Their native callers include join, leave and reconnect paths; the typed
methods accept an explicit pool/index rather than inferring one from Party.

## Writer coverage and the query

The identified writers of the represented AI byte are:

| Writer | Effect / coverage |
| --- | --- |
| `0073E163` whole-game zeroing | Initial zero in both pools; allocation fragment inspected, not reconstructed as a full constructor. |
| `004BB220`, `004BB373` | Mission-record AI byte 1, for the two reset branches. |
| `004BB30E` | Player-record AI byte 0 on reset. |
| `004B5680..004B5689` | Out-of-line `MOV [ECX+9],AL` setter, byte stack argument, `RET 4`; no current xrefs or absolute address occurrences in the image. It is not newly called here. |

`004D6BA0`, `004BB440`, by-index fill `004BB550`, bind `004BB630`, and restore
`004BB660` were inspected and do not write `+9`. This is closure over the
identified record lifecycle, not a proof against arbitrary aliasing writes
through every instruction in the image. Earlier claims that every active
record's byte is permanently zero are false: the pointer table selects both
pools and reset explicitly sets mission flags to one.

`00927F10..00927F26` is five instructions: load the one stack slot index,
load game through `00E188A8`, load `[game+18CCh+slot*4]`, return byte `+9` in
AL, `RET 4`. It has **no** range, null, sentinel8 or claimed-byte check. ECX
is overwritten internally, so it is not a receiver argument. The owner API
preserves the raw byte; it reports unavailable for missing represented data
or indices outside 0..7. It never invents a sentinel8 result. The existing
`gun_bot_side_enabled_00927f10` merely combines sentinel8 with a supplied
predicate; it does not own or read this native participant table.

## Canonical API and host integration

`SessionParticipantPools` owns two arrays of narrow `ParticipantRecordBytes`
and eight `ParticipantRecordId` selections. Each represented byte carries its
own availability. Reset preserves availability wherever native bytes retain;
claim cannot skip an earlier record with unknown `+8`. A failed read leaves
the caller's output unchanged.

The main APIs are `reset_flags_004bb160`, `claim_player_flags_004bb440`,
`publish_active_record`, `install_record_004bb630`,
`restore_mission_record_004bb660`, `reset_and_claim_local_flags`, and
`try_ai_held_00927f10(slot, uint8_t&)`. Explicit record adoption accepts actual
restored/observed bytes. `write_device_bound` accepts an actual `+0E` writer;
it does not assert that a device-ready delivery has occurred.

`try_entry_slots` derives the existing `MissionEntryPlayerSlot` view from the
same selected records: `excluded` is raw `+9 !=0`, and `device_bound` is raw
`+0E !=0`. It returns unavailable without changing output unless all eight
selected pairs are represented. The current MissionFrame `entry_slots` array
contains independent defaults, and its reset step sets every `device_bound`
true. That is not an implementation of the native two pools/pointer table.
Host integration must derive these views from this owner and preserve actual
device-write scheduling; the new core does not silently make them all ready.

Actual reset inputs already exist upstream: `GameMissionHost::Impl::load`
owns `SceneRecord` and its parsed `side_blocks`. `finish_scene_load` has the
record, but `GameMissionFrameHost::run_scene_load_004dfb70` currently accepts
only path, script, locale and mission id. Its API must carry actual record
presence/count (or the existing record), retaining this participant owner for
the game/session lifetime. The projection can live in MissionFrame while
that is the session lifetime owner; mode transitions or reconnects must use
the same owner if those paths become live. Units/ShipAI should borrow its
availability read, not copy per-slot booleans or infer slots from scene Party.

## Verification

One ignored focused fixture executes original bytes with explicit initial
record bytes. It compares mission/player/reset stores including retained
inactive AI bytes, the full first-free scan, current-pointer stores, and the
complete unmodified `00927F10`. The scan's successful continuation is a
harness return of the selected pointer; the original `+8=1` instruction is
then executed separately. The complete native claim routine and complete
reset control flow are not exercised. The reset loop schedule belongs to the
fixture, while the memory stores are original. No fabricated complete
participant vtable/record or user service is called.

The fixture also checks typed unknown-state behavior, invalid indices/count,
pool identity changes, local reset/claim order and shared entry views. Its
fixed inputs seed claimed bytes with `index%2`, AI bytes with
`80h+pool*10h+index`, and device bytes with `index%3`; it runs scene-count2
and no-scene store cases. It checks 18 native first-free outcomes and 20
native query results. The native getter uses its original `00E188A8` global
through a fixture-owned mapped page. It does not test a live mission/session,
FP state, or the complete game/participant constructors.

Build, existing-test, fixture, direct-call check results and exact source/
artifact hashes are recorded in `reports/session_participant_ai_flag.json`.
No tracked test or host-file change is added. Ghidra remained read-only.
