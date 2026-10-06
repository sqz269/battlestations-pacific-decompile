# The air-operations tick: 006C0510, and what closes the launch cycle

Packet `cc8_airops_launch_tick`. The packet asked for "the tick that fills a launched slot's
`squadron` field and returns the slot from state 3 to state 1 when its cooldown completes". Two
halves of that premise are wrong and the correction is the substance of this document, so it comes
first.

## 1. Two corrections to the packet's own premise

**The squadron is not filled by the tick.** It is filled by the launch start, `006C7490`, at
`006C74C6`/`006C74FF`, which `docs/AIROPS_LAUNCH_START.md` already established. The tick never
writes slot+28h.

**There is no cooldown compare, and the slot does not return to state 1.** `006C0510` is the only
writer of state 5 in the whole image, and its gate is not the timer:

```
006c0544: MOV ECX,dword ptr [ESI + 0x28]      ; the squadron
006c0547: CMP ECX,EAX                          ; EAX is 0, zeroed at 006c051d
006c0549: JNZ 0x006c05a3                       ; a squadron is out -> only track it
006c054b: MOV EDX,dword ptr [ESI + 0x2c]
006c054e: CMP EDX,0x3
006c0551: JZ  0x006c0558
006c0553: CMP EDX,0x4
006c0556: JNZ 0x006c059f
...
006c058f: MOV dword ptr [ESI + 0x2c],0x5
```

A slot leaves state 3 **when its squadron pointer is zero**, and it goes to state **5**, not 1.
Nothing in `006C0510` reads slot+30h at all. The timer is an accumulator, not a countdown:

```
006c0510: FLD   float ptr [ESP + 0x4]          ; the step
006c051a: FADD  float ptr [ESI + 0x30]
006c0522: FSTP  float ptr [ESI + 0x30]         ; slot+30h += step
```

So what clears slot+28h is the other end of the cycle, `006C65B0
BSP_AirOps_ReleaseSquadronSlot` — the landing — and `006C0510` is what notices. The two are one
loop, and the packet's "cooldown" is the 5.0 preload of a *state entry*, not a wait.

**Where a compare against the timer does exist**, and this is the only one this packet found, is
`006C64B0`, a different sub-update:

```
006c64b0 ... if (slot+2Ch == 2 && DAT_00D7A24C < slot+30h && block+38h == 0 && 006BED60())
                 006BC8E0(slot index); 0048CD50();
```

`DAT_00D7A24C` is `00 00 80 3F`, **1.0**. So a *queued* slot (state 2) waits one second before the
deck acts on it. The 5.0 second constant is `DAT_00CE3850`, `00 00 A0 40`, and every site that
writes it writes it as an initial value of slot+30h, never as a target.

## 2. What slot+34h actually is

`006C0510`'s second block, with the branch senses from the bytes:

```
006c051f: CMP  byte ptr [ESI + 0x34],AL        ; AL = 0
006c0525: JZ   0x006c0544                      ; clear -> skip
006c0527: MOVSS XMM0,dword ptr [ESI + 0x30]
006c052c: MOVSS XMM1,dword ptr [0x00ce3850]    ; 5.0
006c0534: COMISS XMM0,XMM1
006c0537: JA   0x006c053c                      ; timer > 5.0 -> keep it
006c0539: MOVAPS XMM0,XMM1                     ; else take 5.0
006c053c: MOVSS dword ptr [ESI + 0x30],XMM0
006c0541: MOV  byte ptr [ESI + 0x34],AL        ; and clear the byte
```

That is `timer = max(timer, 5.0)`. Every state-entry site pairs with it the same way — `006C74E0`,
`006CD3FC`, `006CCE3A`, `006CCF9D` all write `slot+30h = 0` and then, if slot+34h is set,
`slot+30h = 5.0` and clear the byte. So the byte means **"this slot does not wait"**: it spends the
delay rather than arming one. That is why `00896750 LuaBinding_LaunchAirBaseSlot` sets it — a
scripted launch is meant to take effect at once — and the name `launch_requested` in the header is
kept but now has a writer's meaning behind it.

## 3. The state machine, with every writer by address

An exhaustive store census of the offset (`python tools/store_census.py 0x2c`, every MOV dword
imm/reg in both disp8 and disp32 over `.text`) returns 32 stores to a `+2Ch` inside the
air-operations segment. Each one named below was then checked to address the slot array, either by
the `block+4Ch` load with the `58h` stride or by taking the slot itself in ECX.

| state | writers | what the writer shows |
| --- | --- | --- |
| 1 | 006C6603 (006C65B0, the landing release, with the 5.0 timer), 006CD3FC, 006CCE3A, 006CC61C (006CC5C0, the state-2 cancel), 006C57A7 (006C56D0), 006BD3A3 (006BD360) | the slot holds planes on the deck: 006C7210 launches from 1 or 5, 006BF230 counts stock reserved by 1 or 5 |
| 2 | 006CA66B (006CA640, the queue arm 006CC690 takes at 006CC72C), 006CA91A (006CA8E0) | queued: 006C64B0 waits 1.0 s on it and then calls 006BC8E0 |
| 3 | 006C74E0 (006C7490, the launch start), 006CD0EC (006CCDA0), 006C57F5 (006C56D0), 006C776E (006C7680) | the squadron is away; only 006C0510 leaves it, and only once slot+28h is zero |
| 4 | 006CCF9D (006CCDA0, order 2 against a state-3 slot), 006CD4A9 (the same code inlined into 006CD350) | recalled: the writer first calls the squadron's vtable+114h, holds fire and issues a command to the owner at block+7Ch. 006C0510 treats 4 exactly as 3 |
| 5 | 006C058F — the tick, and nowhere else | ready: the only state 006CD350 and 006CCDA0 dispatch a launch from |
| 6 | 006CB28B (006CADD0 mode 1, a `FakeAllocated` slot), 006C8326 (006C80C0) | parked. 006C56D0 treats 6 as free alongside 1 |

Four further stores to a `+2Ch` (006BA4E0, 006BC751, 006C4F5C, 006C7C3F) are in the census but were
not shown to sit on a slot, so they are not claimed.

**006CD350's 1/2/3 arms are dead code.** `006CD350 BSP_AirOps_UpdateSlot` tests the state at
`006CD39B` (`CMP dword ptr [EAX + EBP*0x1 + 0x2c],0x5`, `JNZ` at `006CD3AC`) and then re-reads the
same dword at `006CD3CA` and dispatches on 1, 2, 3 and 5. The second read can only be 5, so the
other three arms are unreachable there. They are the inlined copy of `006CCDA0`, where the same
code is live; `006CD350`'s one caller is `00895ED0 BSP_LuaBinding_SetAirBaseSlot`, so it is not a
tick at all.

## 4. The cycle, end to end

```
        006CD350 / 006CCDA0 state-5 arm            006C7490                006C0510
  (5) ------------------------------------> (1) ----------------> (3) ------------> (5)
        count = min(limit-committed,               writes 3,             when slot+28h == 0
        stock, slot+0Ch); state 1                  slot+28h = squadron   count refilled, state 5
                                                          |
                                                          | 006C65B0, the landing
                                                          v
                                                     (1) with timer 5.0, +28h and +8h cleared
```

`006C65B0` walks every slot with no early exit (`006C6612` advances by 58h, `006C661B` counts
block+50h down) and for each whose +28h is that squadron unregisters the observer pair, zeroes +28h
and +8h, writes state 1, sets the timer to 5.0 and clears the +34h byte. Its one caller is
`007F1B70 BSP_Squadron_ReleaseFromAllAirBases`, which calls it twice (`007F1BAD`, `007F1BED`).

`006C56D0` is the arrival at the other end of the same cycle, `__thiscall(block, squadron)`, and
reading it whole sharpens two things this document depends on. Its slot search has a priority the
header's `air_ops_slot_is_free_006c56d0` deliberately does not model: it **breaks** on the slot whose
+28h already is that squadron, and only while no such slot has been found does it remember the first
slot that is state 6 **or** state 1 *and* whose class at +4h equals squadron+35Ch *and* whose count
at +8h equals squadron+3CCh. So "free" is an exact-match test, not a state test, and the state test
alone is the part that is reconstructed. Having chosen a slot it writes state 1 with the timer pair,
calls `006C0F00` to reassign the class and count from squadron+3D0h->+538h and squadron+3C8h, and
ends at state 3 with the squadron in +28h and the observer pair moved.

**It also corroborates entity+3CCh independently.** The tick, `006BD3F0` and `006BF230` all read
+3CCh as the squadron's live plane count, and here it is compared against a slot's own `+8h` count —
a fourth reader, and one that only makes sense if the two hold the same quantity.

## 5. block+38h has a writer after all

`docs/AIROPS_LAUNCH_START.md` listed "what writes block+38h" as an open question: three routines
read it and none was found to write it. **Correction: `006C6540` writes it, at `006C6589`.**

```
006c6543: CMP dword ptr [ESI + 0x38],0x0
006c6547: JNZ 0x006c65a1                     ; only while the field is clear
006c6549: CMP dword ptr [ESI + 0xdc],0x0
006c6550: JZ  0x006c65a1                     ; and only while the list at +D8h is not empty
006c6552: MOV EAX,dword ptr [ESI + 0xd8]
006c6559: MOV EDI,dword ptr [EAX]            ; the head node
006c6564: MOV EDI,dword ptr [EDI + 0x8]      ; its payload: an entity
006c6567: LEA ECX,[ESI + 0xc0]
006c656d: CALL 0x006c4a70                    ; an STL operation on the container at block+C0h
006c6575: ADD ESI,0x24                       ; the observer pair record lives at block+24h
006c6582: CALL 0x006952a0                    ; unregister the old pair
006c6589: MOV dword ptr [ESI + 0x14],EDI     ; block+38h = the entity
006c6592: CALL 0x00694a60                    ; register the new pair
006c6597: PUSH 0x0
006c659b: CALL 0x00922f30                    ; and enable its scene node
```

The last call is `BSP_SceneNode_Enable`, and the pushed 0 is that routine's second argument, which
it forwards to `vtable[68h]`; `00922F4B` sets node+5Ch to 1 regardless. A first draft of this
document read the 0 as "disable" and said the deck holds the entity hidden. It does not: the entity
is made visible. `docs/AIROPS_LAUNCH_START.md`'s reading of the same call at `006C7528` was right.

So block+38h is the one entity the deck has pulled out of a queue and is holding, with an
observer pair at block+24h whose observed slot is `[block+24h]+14h` — the same shape the slot uses,
where the pair is at slot+14h and the observed slot at `slot+14h+14h` = slot+28h (`006C7502
ADD ESI,0x14`, `006C7516 MOV dword ptr [ESI + 0x14],EBX`). The three readers follow directly:
`006BF620` refuses readiness while one is held, `006CC690` queues at `006CC715` instead of starting,
and `006C5050` refuses at `006C5078`. The provisional name `launch_in_progress` survives, but it is
now "a plane is already being spotted on the deck" rather than an interpretation of three readers.
What fills the queue at block+D8h has not been read, and `006CF190
BSP_AirOpsSite_SetReadyPlaneObserver` is the obvious next address for it.

**This matters for the run.** That queue is the deck's own brake. Without it, nothing in this
process ever makes `IsReadyToSendPlanes` answer false, so the mission script launches on every check
it makes.

### What fills that queue, found while a run was queued

`006C6540` drains it; the filler is `006CC760`, and the chain runs from the squadron side.

A store census of block+D8h and +DCh over `.text` is **empty**, and so is an address-of census of
+D8h — 42 `LEA [reg+0D8h]` sites in the image and not one in the air-operations segment. Neither
negative means anything on its own, because the container is not addressed at +D8h: `006C6567` takes
`LEA ECX,[ESI + 0C0h]`, so the object starts at **block+C0h** and +D8h and +DCh are its head pointer
and its count. Four sites in the segment take that address: `006C6540` (the drain), `006CA410`,
`006CAC00` and `006CC760`.

`006CC760` is the push, `__thiscall(block, entity)`:

```
006cc760: LEA ESI,[ECX + 0xc0]        ; the container
          CALL 00694a60               ; register an observer pair on the entity
          MOV  iVar1,[block + 0xd8]   ; the list header
          CALL 006bfbd0               ; splice a node carrying the entity
          MOV  [iVar1 + 4],iVar3      ; link it in
```

Its one caller is `007F1C00`, `__thiscall(squadron, airbase_entity, flag)`, which sits beside
`007F1B70 BSP_Squadron_ReleaseFromAllAirBases` — the caller of the landing release. It holds the
squadron's home air base at squadron+404h behind an observer pair, and when that is set it takes the
block off the entity through `006BCD20 BSP_AirOps_GetBlock` and, when `*(00E188A8 + 1FE4h)` is not
zero, calls `006CC760` and `007ED6E0`; otherwise it calls `006CC7B0`, a second path not read here.
`007F1C00`'s own callers name what puts a squadron on a deck:

| caller | what it is |
| --- | --- |
| `007F4580 BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes` | the squadron's own attach-and-spawn |
| `0089E220` | a Lua binding, immediately before `0089E3C0 LaunchSquadron` |
| `007F1FE0` | not read |

So the loop closes: `006C5050` builds a squadron whose `HomeBase` is the owner, the squadron's
attach path hands that air base to `007F1C00`, which pushes it onto block+C0h; `006C6540` pulls the
head into block+38h and enables its scene node — the aircraft appears on the deck — and `006BF620`
refuses readiness for as long as it is held. **That is the brake**, and it is why the native does
not launch on every check while this process does.

### The block has two queues, and they mean different things

`006CC7B0` turned out to be the same push onto a different list, block+74h, and `007F1C00` chooses
between the two on `*(00E188A8 + 1FE4h)`. `006C58A0`, sub-update 1 of `006CDC70` and the only one
that runs **before** the game-state gate, is what drains that second list: for each node it hands
the payload to `006C56D0`, the arrival that puts a squadron straight into a slot and ends at state
3, then unregisters the observer pair and unlinks the node.

So the two queues are two ways onto a deck:

| queue | pushed by | drained by | what happens to the squadron |
| --- | --- | --- | --- |
| block+74h, count block+78h | 006CC7B0 | 006C58A0, before the gate | goes **straight into a slot** at state 3, no spotting |
| block+C0h, head block+D8h, count block+DCh | 006CC760 | 006C6540, behind the gate | is **spotted on deck** one at a time into block+38h with its scene node enabled, and readiness is refused while it is held |

`*(00E188A8 + 1FE4h)` is the same game-state word `006CDC70` gates its sub-updates on against 2, so
the split reads as "a mission is running, bring the aircraft up on deck" against "assign it to a
slot without ceremony", which is what a deck built at scene load needs.

### Correction: a campaign takes the other queue, and has no brake at all

The paragraph that stood here said a squadron created mid-mission takes the spotting queue, so
implementing that queue would give this process the deck's brake and retire the 24-squadron ceiling.
**That is wrong, and the branch bytes say so.** `007F1C00` chooses like this:

```
007f1c45: MOV ECX,dword ptr [0x00e188a8]
007f1c4b: CMP dword ptr [ECX + 0x1fe4],0x0
007f1c55: JZ  0x007f1c69          ; ZERO -> 007f1c69, which CALLs 006CC7B0
007f1c57: CALL 0x006cc760         ; non-zero falls through to the spotting queue
```

The **zero** arm — a campaign session — goes to `006CC7B0` and the block+74h queue, which `006C58A0`
drains straight into a slot. The spotting queue at block+C0h, the one whose drain `006C6540` writes
block+38h and so makes `006BF620` refuse readiness, is the **non-campaign** arm.

This process asserts a campaign session in three places now (`game_non_campaign_flag()` returning 0
for `GetDifficulty`, `non_campaign_session()` returning false, and `inputs.non_campaign_session =
false`), so it can only take the zero arm. ~~In a campaign the deck has no readiness brake: nothing
writes block+38h, `006BF620` never refuses on it, and `006CC690` never takes its queue arm.~~
**CORRECTED (docs/SQUADRON_LAND_TASK.md 5ax and 5bj, 2026-09-30).** `006C6540` is not the only
writer. `006C5050` itself stores the squadron it has just made at block+38h whenever its flag argument
is 0 (`006C5314`-`006C5348`). Every launch from `006CC690` passes 0 (`006CC72E`), so every campaign
launch sets the brake. `006C5B70` releases it: `006CDC70` runs it after `006CD240`, and it clears
block+38h once `007ED740` answers, that is, when every member is in state 7, 6, or 4 with ground
contact. While the brake is held, `006BF620` answers not ready. The host binds both behind
`kBaseLaunchBrakeBound` (ON since 5bj).

What paces a campaign launch is the mission script. `usn_19_coralus.lua` gates each American carrier
on `stloPlaneNum < 2` and each Japanese one on `stloPlaneNum < 4`, counting through
`luaGetSlotsAndSquads`, which counts slots whose `squadron` is not nil — so the gate works only
because the tick keeps slot+28h filled. That is the pacing, and it is already correct on this branch:
the run that went from 82 launches to 4 is the gate doing its job.

So the ceiling is not replaced by a brake. It is re-justified as a safety net, raised to 64 so it
sits above the two dozen the script's own gates admit over six decks rather than at it, and it has
never fired. `docs/USN04_STRIKE_CLASS.md`.

What **is** implemented from this section is the campaign arm itself:
`air_ops_push_assign_queue_006cc7b0`, `air_ops_arrive_squadron_006c56d0` and
`air_ops_drain_assign_queue_006c58a0`, with the drain first in the deck update where `006CDC70` has
it. A launched squadron registers with its home base the way `007F4580` registers it, and the drain
finds that same slot by its +28h and rewrites the state it already holds, so the mechanism is the
native's and the behaviour is unchanged. The spotting arm is deliberately **not** written: it cannot
execute here, and an unreachable reconstruction is worth less than the note saying why.

## 6. 006CDC70's nine sub-updates, and which of them this packet reconstructs

`006CDC70 BSP_AirOps_Update`, `__thiscall(block, step)`, is the block's own update, reached from the
owning unit's motion pass: `00758270 BSP_MotherShipUnit_UpdateMotion` for a carrier and `006D2510
BSP_AirField_TickAdvance` for an airfield. It runs, in order:

| callee | what it is | reconstructed here |
| --- | --- | --- |
| 006C58A0 | drains the **other** queue, block+74h with its count at block+78h: each node's payload goes to 006C56D0, which puts the squadron straight into a slot at state 3. The only sub-update that runs before the game-state gate | no |
| 006C0DA0 | the slot walk: every slot of block+4Ch by index, calling 006C0510, routing 006BD520's message for each slot that returned true | **yes** |
| 006C77E0 | a compaction of the 14h-stride list at block+A8h | no |
| 006C64B0 | the state-2 wait described in section 1 | no |
| 006C6540 | the ready-plane pull of section 5 | no |
| 006CD240 | stock regeneration: a 14h-stride array at block+98h, `timer -= step`, and on expiry a reload from 006CC9F0 | no |
| 006C5B70 | behind the two failure bytes and the owner's +5Dh: while block+38h is set and 007ED740 answers, re-publishes through 006BF150 every slot whose +28h is that same entity. A third reader of block+38h, and it treats it as the spotted plane | no |
| 006CD810 | the AI's own launch: walks the slots counting states 3 and 4 whose class answers 13h at vtable+1Ch, against block+E8h | no |
| the virtual at block+3Ch | unread | no |

The gate above all but the first three is `*(00E188A8 + 1FE4h) != 2`.

## 7. What this packet implemented

### The tick itself
`bsp::air_ops_slot_tick_006c0510` is 006C0510 instruction for instruction, including the ordering
trap: `006C055C` zeroes slot+8h **before** `006BD3F0` is called at `006C0562`, so the slot's own
count is not part of the committed total it is then measured against. That is why the reconstruction
takes the deck rather than a pre-computed `committed_planes`.

`bsp::air_ops_stock_available_006bf230` is new and was needed by the tick: the pair
`{available, total}`, where `available` subtracts, for every slot whose class matches **and** whose
state is 1 or 5, the squadron's +3CCh when slot+28h is set and slot+8h when it is not.

`bsp::air_ops_release_squadron_slot_006c65b0` is new and is the far end of the cycle.

`bsp::air_ops_deck_update_006c0da0` is the walk, and `bsp::air_ops_update_decks_006cdc70` runs it
over the table this process keeps its decks in.

### The creation seam
`006C5050`'s bag keys are in `docs/AIROPS_LAUNCH_START.md` section 3 and unchanged. What this packet
added is the seam that makes the squadron a real unit:

* `GameMissionLuaHost` now implements `bsp::AirOpsSquadronFactory`, installed in
  `attach_script_orders` along with the squadron plane-count reader.
* `GameScriptOrdersHost::create_air_ops_squadron_006c5050` makes the unit, because that host owns
  the units host.
* `GameMissionLuaHost::attach_created_entity_00928a00` adds the `thisTable` slot the new unit needs,
  with `ID` as the key **text**, the same four fields `00928A00` seeds at load.

**LABELLED SUBSTITUTION, and the largest in this packet.** The native's `004F0AD0` makes a squadron
container of 1089 bytes that then holds `WingCount` planes. This process's
`create_plane_squadron_004f0ad0` allocates an instance and places it, and nothing in it spawns or
flies a wing. So what is created here is **one unit of the slot's own class** — a plane — standing
for the squadron, placed 150 m above its carrier because the plane rows `create_units` seeds
(+900h = 7, +908h = 3600) describe a plane in free flight. The deck reports the authored `WingCount`
as that squadron's live plane count so the arithmetic of `006BD3F0` and `006BF230` stays the
authored one. The wingmen are not created.

A second guard is equally not native: at most 24 squadrons are created in a process, because the
brake of section 5 is missing.

### A correction to this thread's own Lua code
`push_air_ops_slot_entry` pushed `slot+28h` as an **integer**. `006C6895` pushes the entity. It
matters: the mission script does `PilotSetTarget(slot.squadron, target)` and `00888AA0`'s stand-in
requires a table with an `ID` field, so a number fails the first `lua_type == LUA_TTABLE` test and
the order is dropped without a message. The key now carries the entity's own `thisTable` slot.

### A deviation of position
`006CDC70` is driven from `GameScriptOrdersHost::run_script_timers`, not from
`GameUnitsHost::motion_step_00825f20` where the executable has it. `src/game_hosts_units.cpp` was
leased to another worker (`cc8-dive-bomb:cc8_dive_bomb_servo_run`) for the length of this packet and
the claim was refused, so the walk runs from this host's per-frame pass instead, on the same fixed
step. Nothing in the tick reads anything the motion pass writes, so the difference is one of
position within the frame, not of result. It should be moved when that file is free.

## Measured: the USN04 run of 2026-09-18

`local/usn04_tick.log`, exit 0, 3199 frames presented, 3000 mission frames at 0.05 s = 150 s of
mission time. Compared against `local/usn04_gates.log`, the previous packet's run of the same
mission, which is the row marked "before".

| measurement | before | after |
| --- | --- | --- |
| decks built | 6 | 6 |
| slots per deck at the end | grew without bound, ~40 on each of two carriers | **4, the authored count, on all six** |
| `air_ops` slot ticks | none, there was no tick | 72000 = 3000 frames x 24 slots |
| slots that went 3/4 -> 5 | n/a | 0 |
| 006C65B0 releases | n/a | 0 |
| `IsReadyToSendPlanes` calls / true | 82 / 82 | **4 / 4** |
| `LaunchSquadron` calls / started | 82 / 82 | **4 / 4** |
| squadrons created | 0 | **4** |
| `GetProperty` calls / served | 164 / 164 | 86 / 86 |
| units with guns | 53 | 57 |
| units with torpedo ordnance (2Bh) | 34 | **34** |
| units with general-bomb ordnance | 1 | **5** |
| plane free-flight steps | 6000 | 15716 |
| ordered aircraft | 2 | **6** |
| `range_first_mean` | 0.0 m | 0.0 m |
| torpedo drops / breakups / swims | 0 / 0 / 0 | 0 / 0 / 0 |
| kind Eh torpedo task | none | none |
| `script call Think failed` | 0 | 0 |

### The tick holds the deck at its authored size

That was the defect `docs/AIROPS_LAUNCH_START.md` found and could not fix. `slot_ticks=72000` is
exactly 3000 x 6 x 4, so every deck stayed at four slots for the whole run. `refills_3_4_to_5=0` and
`releases=0` are the correct readings and not a dead tick: all four squadrons were still alive at
the end (`active=1` on every one), so no slot's +28h ever went to zero and none was due to refill.

### The strike is launched, and it is twelve planes

```
air ops squadron: Lexington-class01_sqn01 class=101 wing=3 ... -> unit 53 id 54 at (-12914.8 150.0 -12946.7)
air ops squadron: Yorktown-class01_sqn02  class=101 wing=3 ... -> unit 54 id 55 at ( 12841.1 149.7 -12961.8)
air ops squadron: Lexington-class01_sqn03 class=101 wing=3 ... -> unit 55 id 56
air ops squadron: Yorktown-class01_sqn04  class=101 wing=3 ... -> unit 56 id 57
```

Four squadrons of three, two from each of the two carriers: the twelve-plane strike the stream is
after. **The launch count fell from 82 to 4 because the seam works**, not because a gate closed. The
script launched while `slot.squadron` was nil and stopped when it was not, which is the whole point
of the key; the 82 launches of the previous run were the script retrying against a key that never
filled.

Every created unit reached every pass this process has: `plane spawn` seeded it at 75 m/s on its own
forward axis, `unit motion dispatch` gave it creator 007DDAE0 / entry 007CE040, `unit world
registration` put it in six world lists, and the gun chain gave it ordnance (+4 units with guns).

### Two findings that change the stream's own premise

**The launch that was measured carries general bombs, not torpedoes — but it is not the strike this
stream is after.** The arithmetic is exact: the four new units took "units with guns" from 53 to 57
and "general_bomb" from 1 to 5, while "torpedo" stayed at 34. The class is 101 with a wing of three,
and class 101 in this installation is not a torpedo carrier. `0099A170` builds a kind Eh task only
for an ordered aircraft carrying ordnance kind 2Bh, and the run says so in as many words:

```
summary mission torpedo task: no ordered aircraft carries torpedo ordnance (kind 2Bh),
so 0099A170 builds no kind Eh task
```

### Correction to the paragraph above, same day, before this document left the branch

A first version of this section concluded that "USN04's carrier launch will not exercise the
torpedo path whatever else is fixed". **That is wrong, and the deck list is what disproves it.**
USN04 has six decks:

```
Lexington-class01   Yorktown-class01   Zuiho-class01
Zuikaku-class01     Shokaku-class01    dummylex
```

All four launches came from `Lexington-class01` and `Yorktown-class01` — the **American** carriers,
two squadrons each. `docs/TORPEDO_MISSION_SURVEY.md` section 3 names the strike this stream is
after as `launchedStriker`, twelve aircraft, "launched from `Mission.Zuikaku` and `Mission.Shokaku`
slots and ordered at launch (`usn_19_coralus.lua:1409`-`1446`)". **Zuikaku and Shokaku did not launch
in this window at all.**

So what was measured is the American carriers' own launch, and the class-101 finding is a fact about
*that* launch, not about `launchedStriker`.

### The three classes, read from the installed tables

The classes are now resolved, and without a game run. `bsp_mission_script_probe` runs the same
autoload folder `00886900` does, so the `VehicleClass` global is in its state exactly as the game
leaves it; the new `--vehicle-class <index>` option prints a row from it. All three indices that
matter:

```
VehicleClass[101] : Name="globals.unitclass_wildcat" Type="Fighter"
VehicleClass[158] : Name="globals.unitclass_val"     Type="DiveBomber"
VehicleClass[162] : Name="globals.unitclass_kate"    Type="TorpedoBomber"
```

**This answers `docs/TORPEDO_MISSION_SURVEY.md`'s standing open item.** That document's line 231 says
whether `launchedStriker` contains torpedo-armed aircraft "is still open and now unanswerable from a
run", and its line 233 names the script's two plane types, 158 and 162, as unresolved. They resolve
to the Val and the **Kate**, and the Kate's class `Type` is `TorpedoBomber`. Coral Sea's Japanese
strike is a Val-and-Kate strike, which is historically what it was.

**And it retracts a sentence written two paragraphs above, earlier the same day.** That sentence
called the American launch "twelve dive bombers, which is what a Lexington and a Yorktown carried at
Coral Sea". Class 101 is the **Wildcat**, `Type="Fighter"`: the American carriers launched a
twelve-fighter patrol, not a bomber strike. The ordnance census still says those four units carry
general-bomb and not torpedo ordnance, which is a fact about the gun list this process built for
them; the class *type* is the authored one above and the two should not have been conflated.

Two limits on the reading, stated because they are load-bearing. `Type` is the literal
`00964790`'s string chain compares, so `TorpedoBomber` is an authored class type and **not** proof
that the class's guns carry ordnance kind 2Bh; only a run that creates one settles that. And the
slot's class travels from the scene's numeric `Type` token through `LaunchSquadron`'s argument to
`read_vehicle_class_row` as an index into `VehicleClass`, unchanged — the same id-as-index
convention `attach_scene_entities_00928a00` already uses, and the rows were found rather than
missing, so it is the process's existing convention rather than a new assumption.

### What this leaves

The launch path is no longer the blocker for the stream's goal. With the tick and the creation seam
on this branch, a `launchedStriker` launch of class 162 becomes a real Kate unit with a `thisTable`
slot, and `PilotSetTarget` on it is exactly the call that installs the torpedo task. The only thing
between here and that is **mission time**: the `luaDoTimeTable` entry that carries the order still
had 34.95 s to run when the 150 s window closed.

**The squadrons are ordered by the party AI, not by the mission script.** All four appear in the
pilot-attack tally (`ordered` 2 -> 6), and each one's line is

```
ordered Lexington-class01_sqn01 range 0.0 -> 0.0 m closed 0.0 m heading error 1.571 -> 0.013 rad
player command issued to "Lexington-class01_sqn01": token="artillery" resolved="attackmove"
Lexington-class01_sqn01 moveto ai_command_tick ...
```

`PilotSetTarget` was called once in the whole run, on `movieval`, in `luaStageInit`. The range is
0.0 because the AI's attackmove carries no target position, which is why the four hold station at
their spawn points. The mission's own order has not arrived: the timetable entity that would issue
it still had 34.95 s of its delay to run when the window closed.

```
script entity 100006 created_for=luaDoTimeTable think=luaTimetable armed=1 delay=34.95 thinks=0 dead=0
```

That line is **identical in the before log**, so it is not a regression from this packet; the 150 s
window is simply shorter than the mission's own schedule.

The longer run that would settle it was **not taken**. `cc8-dive-bomb` held
`%USERPROFILE%\.bsp\bsp_game.lock` for the whole of the remaining turn (its two processes started at
17:59:43, the second this packet's own run released the lock), and a doubled run queued behind it
would not have fitted in the foreground call. Orphaning a run that holds the lock would have cost
the peer more than the measurement is worth. The command for whoever takes it:

```
./tools/run_game.ps1 -Log local\usn04_tick_long.log -WaitSeconds 2400 -- --frames 6400 \
  --press-start-frame 30 --menu-select USN04 --mission-frames 6000 --mission-frame-seconds 0.05
```

What it would answer: whether `luaTimetable` entity 100006 fires at ~185 s, whether the order it
carries is the one that sends the four American squadrons, and — the decisive one — whether
`Zuikaku-class01` and `Shokaku-class01` then launch `launchedStriker` and with what class. If that
class carries ordnance 2Bh, everything this stream has built downstream of the launch becomes
reachable in one run.

## Uncertainty

* ~~What fills the queue at block+D8h that `006C6540` drains.~~ **Answered in section 5**: `006CC760`,
  from `007F1C00` on the squadron side. Implementing the push is what removes the 24-squadron
  ceiling. `006CC7B0` is now read too: it is byte for byte the same push onto a **different** list,
  block+74h, and `007F1C00` chooses between the two on `*(00E188A8 + 1FE4h)` — the same game-state
  word `006CDC70` gates its sub-updates on against 2. What distinguishes the two lists is open, and
  so is `007F1FE0`, one of the three callers.
* ~~`006BF150`, called at the end of `006C7490`, was not read.~~ **Answered.** It is the
  slot-changed notification: `00696350(0)`, then `006BD520` builds the message for that block and
  slot and `0077C7B0` routes it — the same pair the slot walk `006C0DA0` performs inline at
  `006C0DFB`. Every routine that changes a slot publishes it, which is why it appears at the end of
  the launch start and on several arms of `006CD350` and `006CCDA0`. This also closes the
  corresponding item in `docs/AIROPS_LAUNCH_START.md`.
* Of `006CDC70`'s nine sub-updates only the virtual at block+3Ch is now unread; `006C58A0` and
  `006C5B70` are in the table above. None of the eight but `006C0DA0` is reconstructed.
* `006BC8E0`, which `006C64B0` calls when a state-2 slot's second has passed, was not read.
* slot+4Ch has no writer this thread has read. slot+50h now has one: `006CCDA0` stores its fifth
  argument there on two arms (`*(slot + 0x50) = param_5`), which is the `OwnerPlayer` 006C5050
  reads. The argument's own source was not traced and the store addresses were not taken from the
  listing, so this is a decompiler reading and not yet an address-backed claim.
* `006C7680`, which writes state 3 on a slot it takes in ECX, and its one caller `006C8800`.

## Host methods

| method | address | coverage |
| --- | --- | --- |
| `bsp::air_ops_slot_tick_006c0510` | 006C0510 | complete |
| `bsp::air_ops_deck_update_006c0da0` | 006C0DA0 | the walk; 006BD520's message is a count, not a message |
| `bsp::air_ops_update_decks_006cdc70` | 006CDC70 | one of nine sub-updates |
| `bsp::air_ops_stock_available_006bf230` | 006BF230 | complete |
| `bsp::air_ops_release_squadron_slot_006c65b0` | 006C65B0 | complete but for the observer unregister |
| `GameScriptOrdersHost::create_air_ops_squadron_006c5050` | 006C5050 | the bag's effect, not its shape; one plane stands for the squadron |
| `GameMissionLuaHost::attach_created_entity_00928a00` | 00928A00 | one slot, mid-mission |

## no_ghidra_function

None. Every address in this document is a function Ghidra has.

## Validation

Built with `./scripts/build.ps1`; both ctest suites pass. The measured section below is the USN04
run.

## Correction: the one-plane stand-in for a launched squadron is gone

Packet `cc8_plane_squadron_host`.

* **was**: section 7's labelled substitution - "what is created here is ONE unit of the slot's own
  class, a plane, standing for the squadron, and the deck reports the authored `WingCount` as its
  live plane count"; the wingmen were not created.
* **is**: `create_air_ops_squadron_006c5050` now builds the squadron record and one record per extra
  wing as a single batch and hands them to `create_units` in one call, so a launched squadron holds
  `WingCount` real plane units named by `007F4926` / `007F49EF`. `air_ops_squadron_plane_count`
  reports the real `+3CCh` - the number of entries in the squadron's `+3D0h` array whose unit is
  still active - instead of the authored wing.
* **evidence**: `docs/PLANE_SQUADRON_HOST.md`, and the before and after columns of USN04 there.

Two things about section 7 stand unchanged and are worth restating so they are not re-litigated.
The squadron the script sees is still ONE entity with one integer entity id: the `squadron` key in
`push_air_ops_slot_entry` is that id and not a table, because five script readers do
`thisTable[tostring(id)]`. And the launch is still airborne over the home base rather than off the
deck through the taxi and catapult paths, which remain `contract: unread`; every wing takes the
squadron's own frame, because `007F4813` has no per-wing offset - the formation spacing belongs to
the pilot bot.

What this host still fuses is the container with its flight leader: the unit carrying the squadron's
own name is wing 0 rather than a separate `0x414` object. `docs/PLANE_SQUADRON_HOST.md` section 1
states that substitution and why it is the design here.

## The player's launch (packet `cc9_player_air_ops_launch`, cc9-lua41, 2026-10-06)

Switch `bsp::kAirOpsPlayerLaunchBound` (`include/bsp/air_operations.hpp`), committed **OFF**.
Ghidra was read-only.

**There is no launch native: the player launches from the Support Manager screen.** Its update
is `00675C40` (HUD slot 4Eh, vtable `00CF6B44` +20h). Its launch is `0067A565`-`0067A5E6`:
1. **Stock.** `0067A57D` `006BF310`(class): the class's stock (`006BF230`'s second field).
2. **Fill.** `0067A5A7` `006C4780` -> `006C0F00`(slot, class, min(stock, screen+2B4h)).
3. **Message.** `0067A5D1` `00656280` builds message **82h**:
   - +20h the slot;
   - +24h order **3**;
   - +28h the target's +174h (screen+21Ch);
   - +2Ch the player (game+18ECh).
   It is routed at `0067A5E6` (`0077C2A0`).
4. **Delivery.** The deck's message switch `006CD6C0` reaches it from the carrier's `00758EB0`
   and the airfield's `006D2956`. Case 1 is `006CD160`, which calls `006CCDA0`(slot, 3, target,
   player, 1).
5. **The order.** `006CCDA0`, for a slot in state 5 or 1:
   - order 3 with no target answers 0;
   - otherwise `006C4F70` stores the target at slot+4Ch and slot+50h = player;
   - then `006CA640` queues the slot: state 2, timer 0 (or 5.0 with slot+34h), and the index is
     appended to block+14h.
6. **The queue wait.** `006CDC70`'s `006C64B0` serves the head of block+14h:
   - a head no longer in state 2 is dropped;
   - one with timer > 1.0, block+38h zero and `006BED60` true (runway and hangar clear, an owner
     present and not disabled) is popped, and `006BC8E0` routes **89h** with the slot;
   - `006CD6C0` case 8 runs `006C7490`(slot, 0).
7. **The target.** `006C5050` writes slot+4Ch's +174h as the bag key `AutoAttackTarget`. The
   squadron's pass-C init `007F4BA0` reads it at `007F4EC0` and calls `007F15F0`:
   - a plane target is replaced by its squadron (+9D4h);
   - `007EEC50`(target, 1, 1) chooses the class, issued by `0071ECF0`;
   - a null class issues moveto `00E08F68`.
8. **Other senders.**
   - `LaunchAirBaseSlot` sends 83h (`006CA8E0`: immediate `006C7490`(slot, 1), or the same queue).
   - The 81h slot setup goes through `006BD800` (`006C47F0` -> `006C0E70`/`006C0F00`).
   - `006CC690`'s queued arm returns the stock but, in this host, never queues. That is unchanged.

**006C0F00 (fill).**
- The count is clamped to the class's stock total and to `006BD460`. That is the plane limit
  block+58h less what every other slot holds: a launched slot counts its live planes, an idle
  slot nothing, any other its count.
- While the free stock (plus this slot's count, when it holds the class) is short, it takes from
  other idle slots of the class, last first.
- While the plane limit is short, it takes from any other idle slot.
- It writes the class, with class+134h into slot+10h when the class changes, and the count.

**The host.**
- **Pure rules.** `air_ops_fill_slot_006c0f00`, `air_ops_slot_capacity_006bd460`,
  `air_ops_slot_command_006ccda0` (states 1 and 5, orders 1 and 3 only; recall and the other
  states answer -1), `air_ops_queue_slot_006ca640`, `air_ops_deck_free_006bed60` and
  `air_ops_queued_slot_wait_006c64b0`.
- **The harness entry.** `script_orders_player_air_ops_launch(base, slot_number, class, count,
  target)` is for cc9-ships36's `launch` line.
- **The queue wait** runs after the deck update. The launch start passes `AutoAttackTarget`.
- **AutoAttackTarget** is served through the squadron intake `007F1940` once the members exist.

**Substitutions, labelled.**
- The message delivery is immediate, and the player id is 0.
- `006C7D10` and `006C48F0` in `006CA640` are not read.
- A declined `007EEC50` issues nothing, not moveto.
- The AutoAttackTarget order is given at the first step the squadron's members exist, not in
  its own pass-C init.

**Predictions (OFF -> ON).**
- **Rows with no `launch` line:** identical, because nothing queues.
- **USN01 with a `launch Enterprise 0 101 4 <Nell squadron>` line in phase 3:**
  - the slot fills from the 40 Wildcats (class 101);
  - about 1 s later `air ops queued launch` appears, and a squadron of 4 takes off from
    Enterprise;
  - `AutoAttackTarget` issues dogfight (`00E08F58`) on the Nells.

### The player's launch: identity pairs (cc9-lua41, 2026-10-06)

The OFF build is `18f67fa22` (ships36's `launch` line, SCRIPTED_HELM 13, with this packet). The ON
build is its export with `kAirOpsPlayerLaunchBound=true` (`local\l41_on`). The launch form is
reference AB/AC's. Neither run has a `launch` line, so nothing queues.

| row | `pair_diff` | what differs |
| --- | --- | --- |
| USN04 | 1 | only the known noise counter `ship ai free refills` |
| USN01 3000 | 1 | gameplay identical |
| ESMP08 long | 0 | none |
| USNRM01 | 0 | none |
| JM05 long | 1 | gameplay identical |
| IJN11 | 1 | gameplay identical |

The prediction holds: with no `launch` line the switch moves nothing. The USN01 win attempt with a
`launch` line is cc9-ships36's.

### The player's launch: measured, **ON** (cc9-lua41, 2026-10-06)

**USN01 36000** with `local\l41_usn01_launch.txt`: ships35's two phase-2 target picks, then at frame
4300 (215 s, after the Nells' phase-3 generation) `launch Enterprise <1..4> 101 6 Nell<1..4>`.
OFF and ON builds are `05fab57f8` and its flip export. Logs are `local\l41_{off,on}_usn01w.log`
and the diff is `local\l41_diff3_usn01w.txt`. `pair_diff` exits 3. OFF refuses every launch line
("kAirOpsPlayerLaunchBound is off").

**The mechanism matches the read.**
- **Slots 1 and 2 fill with 6 Wildcats each and queue.** Slots 3 and 4 are refused because
  006C0F00 leaves them empty. That is right: Enterprise's MaxInAirPlanes is 12 (`air ops deck`),
  and the first two slots hold all 12 (006BD460).
- **About 1 s later** `air ops queued launch` builds squadrons 112 and 117 (006C64B0 -> 006C7490).
  The creator logs `wing=5` for a slot count of 6. The difference is the squadron creator's own
  and was not traced here.
- **The AutoAttackTarget** chooses dogfight (`00E08F58`) on Nell1 and Nell2. It is served at the
  first step, while one member exists (labelled).
- **What it changed:**
  - deaths go from 70 to 77;
  - six Nells are shot down (Nell1|.-2, Nell2, Nell2|.-2, Nell2|.-3, Nell4, Nell4|.-2);
  - both Wildcat leaders die to the Nells' gunners (328.6 s and 403.3 s);
  - Nell5|.-2 now survives.
- **Enterprise still sinks at 384.43 s,** to Nell5|.-3's bomb, as without the launch. Four Nell
  squadrons (3, 5, 6 and the rest of 1) were never intercepted, so the win needs more fighters
  in the air sooner. That is a scripting choice for the harness line, not a mechanism gap.
- **Rows without a `launch` line** are identical (the identity pairs above).

**Verdict: ON.** The flip only acts on a `launch` line.

**Open:** the creator's `wing=5` for a count of 6; and the AutoAttackTarget order is given to the
members that exist at the first step.

### Queued LaunchSquadron: census, no reach (cc9-lua41, 2026-10-06)

The question was whether any reference row has a script `LaunchSquadron` that 006CC690 queues (its
006CC72C arm, taken while block+38h holds a squadron) and that this host never starts.

**AB's 22 logs** (`summary mission airops gates`, cc9-gunnery28's `local\g28_ab_base_<row>.log`):
- every call started: E2 4/4, IJN11 6/6, JM05 8/8, JM05 long 14/14, USN04 4/4, USN13 9/9 and
  USN13 long 18/18;
- `queued=0` on all 22 rows;
- no row calls `LaunchAirBaseSlot` or `SetAirBaseSlot`.

The scripts gate `LaunchSquadron` on `IsReadyToSendPlanes`, which answers false while block+38h is
set (006BF620), so the queued arm is never taken. **No reach:** no `kAirOpsQueuedLaunchStartBound`
binding is made. The queue machinery this packet added would serve it unchanged if a script
queued one.

### The screen's launch group (packet `cc9_player_launch_group`, cc9-lua42, 2026-10-06)

Switch `bsp::kAirOpsPlayerLaunchGroupBound` (`include/bsp/air_operations.hpp`), committed **OFF**.
Ghidra was read-only.

**The question.** 5ef's `wing=5` for a requested 6.

**The read.**
- 0067A57D..0067A592 (inside 00675C40):
  - `CALL 006BF310` (stock);
  - `CMP EAX,EDI / JLE` (no stock, no launch);
  - `MOV ECX,[ESI+2B4h]`, then `CMP EAX,ECX / CMOVL ECX,EAX`.
  - So 006C0F00 gets min(stock, screen+2B4h).
- screen+2B4h has one writer in the image: `0066EC1B MOV [ESI+2B4h],EBP`, with `0066EC0A MOV EBP,3`.
  It is in `BSP_HudSupportManagerScreen_Register` (body 0066EA60-0066FADD).
  - A whole-image scan for disp32 2B4h (`local\l42_field_writers.py`) found 62 store candidates.
  - Their enclosing functions are plane bot states, plane-class derivation, the main menu, shader
    constants and others. None is a Support Manager method.
  - The only other screen reader is FUN_00673A10 at 00674815. It applies the same min before its
    own 006C4780 call.
- **So a player launch is at most 3 planes per slot.**
  - A 6 never reaches 006C5050's `WingCount` from the screen.
  - In the image, a sixth wing would overrun 007F4B55's five-slot array.
  - The host's five-slot refusal (`plane_squadron_attach_plane_007f4b43`) is what printed `wing=5`.
- **UNCERTAIN:** a block copy into the screen object would not show in a disp32 scan.

**The host.** `player_air_ops_launch` clamps the requested count to `kSupportManagerLaunchGroup` (3)
and logs `count N clamped to the screen's 3`. The stock clamp stays 006C0F00's.

**Predictions (OFF -> ON).**
- Rows with no `launch` line: identical (the clamp is only on the harness launch path).
- USN01 with launch lines asking 6: each filled slot holds 3 Wildcats, not 6.
  - Enterprise's plane limit of 12 (006BD460) then admits four slots instead of two.
  - The creator logs `wing=3`.

### AutoAttackTarget for every member (packet `cc9_player_launch_group`, cc9-lua42, 2026-10-06)

Switch `bsp::kAutoAttackAllMembersBound` (`include/bsp/air_operations.hpp`), committed **OFF**.

**The question.** 5ef's second open item: the order reached only the members that existed at the
first step.

**The evidence.** cc9-lua41's ON log (`l41_on_usn01w.log`, lines 35781-35800) shows the order in the
same step as 006C5050. It reads `AutoAttackTarget: squadron 112 members=1`, and only after it come the
`plane spawn` lines for `Enterprise_sqn01|.-2..|.-5`. The wings are made one step later by the
squadron's pass A (`kWingConstructionInPassABound`). In the image, the key is read in pass-C init
007F4BA0 (007F4EC0 -> 007F15F0), after 007F4580 has made every wing.

**The host.** `run_air_ops_player_launch_queue` holds the pending order while any registered slot
of the squadron (`member_units`) is still `kPlaneSquadronNoUnit`. It keeps the 600-step cap and then
serves the members that exist. The order still goes through `squadron_intake_007f1940`, which issues
it to every member it is given.

**Predictions (OFF -> ON).**
- Rows with no `launch` line are identical (the queue is filled only by 006CCDA0 order 3).
- USN01 with launch lines:
  - each `air ops AutoAttackTarget` line moves one step later and reads `members=3`;
  - the wingmen get the dogfight task (`00E08F58`) as well as the leader.

### The screen's launch group and AutoAttackTarget for every member: measured (cc9-lua42, 2026-10-06)

**Measurement configuration only.** Every run below has `kBombDropScatterBound` forced **false** on
both sides (`pair_export --flip kBombDropScatterBound=false`).
- Without that flip, USN01 never reaches phase 3 on this base. The scout's bombs miss Convoy1, so no
  Nell is generated and every `launch` line is refused.
- With it, phase 2 is the pre-scatter one, so the Nells exist.
- The flip is not a proposal to turn the scatter off.

**The runs.**
- Base `7c883e271`, USN01 20000 frames, reference AB/AC launch form.
- Order files are cc9-ships36's:
  - `w5`: four Wildcats per slot (`local\l42_usn01_w5.txt`);
  - `w7`: three per slot, four slots (`local\l42_usn01_w7.txt`).
- Exports:
  - `local\l42_s`: scatter off;
  - `local\l42_sg`: plus `kAirOpsPlayerLaunchGroupBound`;
  - `local\l42_sga`: plus `kAutoAttackAllMembersBound`.
- Logs are `local\l42_{s,sg,sga}_u1w{5,7}.log`. No run completed the mission (the Nell6 group
  survives on every side), so no guard line was expected.

**`kAirOpsPlayerLaunchGroupBound`** (`s` -> `sg`):

| row | `pair_diff` | what moved |
| --- | --- | --- |
| w7 (3 per slot) | 1 | gameplay identical: no count above 3, so nothing is clamped |
| w5 (4 per slot) | 3 | each slot is clamped (`count 4 clamped to the screen's 3`, `count=4->3`, `wing=3`); deaths 90 -> 89, hit records 4074 -> 3928 |

- In w5, slots 1-3 still fill and the Nell5/6 retries are still refused for plane room.
- With three fewer Wildcats in the air, Nell4 dies earlier (332.14 -> 318.25 s) and Nell3's leader
  later (364.83 -> 637.37 s). The prediction holds.
- **Verdict: ON.**
- Order files now get at most 3 planes per slot, which is what a player can do.

**`kAutoAttackAllMembersBound`** (`sg` -> `sga`): **mechanism failure, stays OFF.**

| row | `pair_diff` | `air ops AutoAttackTarget` lines (ON) |
| --- | --- | --- |
| w7 | 3 | sqn01 `members=1`, sqn02 `members=1`, sqn03 `members=0 class=0`, sqn04 `members=0 class=0` |
| w5 | 3 | the same shape |

- **The wait works, but the member filter does not.** The wings exist the step after 006C5050
  (`plane spawn: unit=Enterprise_sqn01|.-2` at log line 34286). But the deck launches the members
  one at a time: sqn01 at 210.71, 219.41 and 228.11 s, and sqn03 at 262.91, 271.61 and 280.31 s
  (`base launch ground state ... 2 -> 4`).
- The intake counts only members that are alive and visible (`unit_alive_and_visible`). A plane
  still Inside the deck is not counted.
- **So the order still reaches only the one member already flying.** sqn03 and sqn04 reach the
  600-step cap with no member flying and get no order at all. That is worse than OFF, which gives
  each leader the order. In w7, Enterprise_sqn03 never engages and its Nell3/Nell4 kills move to the
  ships (Nell4 331.04 -> 613.57 s, killer Northampton).
- **What the image does instead.** It issues the order once, to the squadron, in pass-C init
  007F4BA0, whatever the members' deck state. A member that takes off later carries the squadron's
  command. This host has no squadron-level command to inherit.
- **The next step** is to deliver the order to each member as it leaves the deck (state 2 -> 4), not
  to wait for every member slot. That needs the takeoff seam in the units host. It is not done here.

### The squadron's order for members that launch later (packet `cc9_auto_attack_member_launch`, cc9-lua42, 2026-10-06)

Switch `bsp::kAutoAttackMemberOnLaunchBound` (`include/bsp/air_operations.hpp`), committed **OFF**.
It replaces the approach of `kAutoAttackAllMembersBound`, which stays OFF.

**Why.** LOMP10 (cc9-ships37's `s37_l10p2.log`, SHIP_AI 181) is the case:
- CB4_AF's four squadrons were not wholly lost. Only the leaders of sqn01/03/04 and sqn04|.-2 died.
- At the end, 8 of the 12 planes are alive. Every wingman sits in `director idle tail`, with
  `releases_006c65b0=0`.
- The slots are correctly held, because a squadron with a live plane is away. The wingmen never
  received the AutoAttackTarget order (`members=1`), so they never attack, spend their ordnance,
  return and land. Nothing frees the slot.

**The image.** 007F15F0 issues the chosen class to the squadron (0071ECF0). The squadron's +128h,
007ECF80, applies it to every plane in +3D0h, whether it is flying or on the deck.

**The host.**
- The order served at the first step is kept as a standing order on the squadron.
- `run_standing_attacks` gives it to each registered member through `squadron_intake_007f1940` on
  the first step that member is alive and visible, which is when it leaves the deck (ground state
  2 -> 4).
- Each such order logs `air ops AutoAttackTarget member at launch`.
- The standing order ends when:
  - every member has been served or has left;
  - the target (its squadron's flight leader, for a plane target) is gone;
  - or 6000 steps have passed.
- **SUBSTITUTIONS, labelled:** the order arrives at launch rather than at the serve, and each
  member's class goes through its own 007EEC50 call (the image makes one call for the squadron).

**Predictions (OFF -> ON).**
- Rows with no `launch` line: identical (the standing order is made only from a served
  AutoAttackTarget).
- USN01 `s37_u1_p5`: each F4F squadron's |.-2/|.-3 gets the dogfight order (`00E08F58`) as it leaves
  Enterprise's deck, about 8.7 s apart. More Nells die earlier, more Wildcats engage, and deaths move.
- LOMP10 `s37_l10_p2`: CB4_AF's wingmen attack their slot's ship instead of idling. Some slots may
  return (squadrons wholly lost or landed), so some of the retries at 8000-11000 may stop being
  refused.

#### Measured: **ON** (cc9-lua42, 2026-10-06)

The OFF build is `local\l42_m`'s base commit, built in the tree. The ON build is its export with
`kAutoAttackMemberOnLaunchBound=true` (`local\l42_m`). Every other switch is as on main; the
scatter is ON. The order files are cc9-ships37's: `s37_l10_p2.txt` (LOMP10 11200) and
`s37_u1_p5.txt` (USN01 14000). Logs are `local\l42_{off,m}_<row>.log` and diffs
`local\l42_diffm_<row>.txt`.

| row | `pair_diff` | what moved |
| --- | --- | --- |
| smoke 300 | 1 | gameplay identical |
| LOMP10 p2 | 3 | 14 member orders at launch; `releases_006c65b0` 0 -> 3; deaths 9 -> 19 |
| USN01 p5 | 3 | 6 member orders at launch; deaths 92 -> 89; the win leaves the 14000-frame window |

**LOMP10.**
- CB4_AF's wingmen now attack:
  - every sqn01-03 |.-2/|.-3 dies in action (12 rows only ON);
  - Kasumi is sunk.
- Three slots come back to state 1 (`releases_006c65b0=3`) and are relaunched:
  - slot 3 at Kashi twice;
  - slot 1 at Asashimo.
  - The new squadrons sqn05 and sqn06 fly; the other retries are still refused while their slots'
    squadrons are away.
- This is the slot return SHIP_AI 181 asked for. Its cause was the idle wingmen, not a missing
  release arm. 006CD350 states 1/2 and 006CC5C0 were not needed for this case and stay unread.

**USN01 p5.**
- The fighters' wingmen engage. The Nell1-4 squadrons are wholly destroyed earlier:
  - Nell2 is `Dead` at 310.85 s instead of 604.68 s;
  - Nell3 at 343.44 s instead of 387.23 s;
  - Nell4 at 311.75 s instead of 401.08 s.
- Nell5|.-3 and Nell6, which OFF's escorts shot down late (Nell6 at 697.50 s), survive to the end
  of the window. So `luaRemoveDeadsFromTable(Mission.Nells)` is not empty and the mission does not
  complete within 14000 frames (OFF: completed at 697.90 s, with the guard line).
- Enterprise survives on both sides.
- The win now depends on the harness lines for the last two Nell groups (cc9-ships37), not on this
  mechanism.

**Verdict: ON.** The mechanism matches: every later member gets the squadron's order as it leaves
the deck. The moves trace to wingmen that now fight.

### Orders to a held slot (packet `cc9_air_ops_held_slot_orders`, cc9-lua44, 2026-10-06)

**Why.** A squadron that is out, or landed and below, keeps its slot in state 3 (SQUADRON_LAND_TASK 5ep). So the
player's `launch` entry, which serves states 1 and 5 only, can never re-send it. The Support Manager re-sends
it through `006CCDA0`'s other arms: the same 82h message with order 1, 2 or 3 against an occupied slot. These
arms were read from the disk listing at `006CCF4A`-`006CD117`.

In every arm the command is issued **on the squadron**, through `0077D600` with `ECX = slot+28h`.

| state | order | the image | answer |
| --- | --- | --- | --- |
| 3 | 2 (recall) | squadron `vtable[114h]` -> `0071BED0` (hold fire); `land` `00E08FA0` at the block's owner `+7Ch` (`00465080`); state **4**, the timer pair, `006BF150` | 1 |
| 3 | 3 (attack) | only when `006BC5E0` is true: `SetTarget` `00E08EF8` at the target (`006CCFF6`-`006CD014`) | 1, else 2 |
| 3 | 1 (moveto) | only when `006BC5E0` is false: `moveto` `00E08F68` at the squadron's **own** position (`00427EB0` on the squadron, then `00468560`), `006BF150` | 1, else 2 |
| 4 | 2 | refused | 2 |
| 4 | 1 / 3 | moveto own position / SetTarget target; then `006BC730`, which sets slot+8h = `007EE5C0` (and state 1 when 0). `006CD0EC` then writes state **3** unconditionally, plus the timer pair | 1 |

The helpers:
- **`006BC5E0`** (`006BC5E0`-`006BC683`, `__fastcall(squadron)`, bool). It is true when the squadron's current
  command (`vtable[114h]` -> `0071BE40`) is moveto `00E08F68` and the squadron is within class+268h x
  `[00CEC160]` of the command's point (`0071EB60` -> `006F7DD0`, `0042B2F0`).
- **`007EE5C0`** (one caller, `006BC730`). It counts the members of `+3D0h` that are **not** landed (`+904h`)
  in state 5 or 2, or in state 1 while disabled (`+5Ch` clear).
- So a busy squadron takes only a moveto (it holds where it is). Once it holds at that point, it takes an
  attack. **The player's re-send is order 1, then order 3**, or order 2, then 1 or 3, for a recalled one.
- No arm releases the slot: `006BC730`'s state 1 is overwritten at `006CD0EC`. A held slot frees only when its
  squadron is gone (5ep).

**The binding.** `bsp::kAirOpsHeldSlotOrdersBound` (`include/bsp/air_operations.hpp`), committed **OFF** and
flipped **ON** by the ESMP08 pair below (cc9-lua46):
- `air_ops_held_slot_order_006ccda0` is the arm table above;
- `GameScriptOrdersHost::player_air_ops_order(base, slot_number, order, target)`, with the free
  `script_orders_player_air_ops_order`, is the player entry;
- the units host exposes `plane_order_view` for `007EE5C0` and `006BC5E0`.

Labelled substitutions:
- delivered at once, with player 0;
- `006BC5E0`'s distance test is not read: holding means the leader's stored class is moveto;
- the commands reach the members the host's way: the attack through the squadron intake `007F1940`, the
  moveto issued on each member at the leader's position with its kind-7 task installed after delivery
  (cc9-lua46; before, the task read the member's old command), the land through `0099A3DD`;
- the hold fire is not carried.

The state-2 arms (retarget, and `006CC5C0`'s cancel) and the state-5 order-2 re-arm are not served by this
entry.

**Predictions, written before any ON run.** These need a harness line (`order <base> <slot> <1|2|3>
[target]`), routed to cc9-ships39.
- **ESMP08 with `s38_e8_p3.txt` plus `order` lines** for the slots whose squadrons are still flying after
  their target died. Each slot answers order 1, then order 3 with a live target. The squadrons re-attack, so
  IJN strikes on the remaining USN ships rise.
- **A squadron landed and stowed** (state 3, all members in state 2): order 1 issues a moveto that the host's
  stowed planes cannot fly (state 2 has no motion arm). The host is expected to show 0 planes moving: a known
  limit, not the image.

#### The ESMP08 pair (cc9-lua46, 2026-10-06): predictions, written before the ON run

- **The order lines.** They are generated from the OFF run (`local\l46_e8gen.py`), with the `s38_e8_p3.txt` launch lines
  kept. For every slot whose squadron is still out when its launch target dies, the file sends `order <base> <slot> 1`
  about 5 s later. On the next frame it sends `order <base> <slot> 3 <target>` for each USN ship in the launch generator's
  rotating priority.
- **A new labelled gate in the entry.** Order 3 refuses a target that fails `0043F080` (dead or hidden), as the launch
  line's harness gate does, because the screen offers only live targets. The first live target is taken; the later
  lines for that slot answer 2, because the leader is no longer holding.
- **OFF.** Every order line is refused with "kAirOpsHeldSlotOrdersBound is off". Gameplay is identical to a run with no
  order lines.
- **ON.**
  - Each order 1 to a busy squadron answers 1 and gives its members a moveto at the leader's position.
  - The order 3 that follows answers 1 and goes through the squadron intake `007F1940`, so the re-sent squadrons fly
    and strike again.
  - Expected: more torpedo and dive releases after the first re-send, and US ship deaths no later than OFF. This
    reaches Cummings, the one US ship the OFF run of ships40 (`s40_on2_e8`) never sank, only if a re-sent squadron
    lives long enough.
  - USN04 (the control, no order lines) is gameplay-identical.
- **Mechanism failure:** an order that answers 1 but whose members never take the target (no `command target` line
  naming it), or a re-sent squadron that never releases.

#### The ESMP08 pair: results (cc9-lua46, 2026-10-06), the switch ON

The pair was built from the same tree (`pair_export.py`, commits `f59c3f4a9` and `be9ff3830`). The runs were ESMP08 at
72000 frames with `l46_e8_orders2.txt`, which is generated from the OFF log so that each order's premise holds there.
The logs are in `local\l46_*` of the cc9-lua46 tree.

| run | US ships sunk | mission end | order 1 / order 3 accepted |
| --- | --- | --- | --- |
| OFF | 14 of 15 (Cummings lives) | none | 0 / 0 (every line: "is off") |
| ON i1 (orders from ships40's log, all order-3 lines in one frame) | 15 | completed at 2277.55 s | 23 / 68 |
| ON i2 (one order-3 line per frame) | 15 | completed at 2777.98 s | 23 / 11 |
| ON i3 (i2, with the moveto issued before the task) | 15 | completed at 3387.43 s | 24 / 24 |

USN04 (the control, no order lines) is gameplay-identical (pair_diff exit 1). Two OFF runs with different order
files are identical (exit 0). Every ON run that completes logs `bsp: refused a mission script's process launch:
sus_prog.exe`.

**Two host corrections, made during the pair.**
1. **i1, holding.** All the order-3 lines of one frame were accepted: the members' attack class is stored after the
   order's delivery, so the leader still read as holding. The image's player clicks one target. The file now
   sends one line per frame, so the next line sees the attack and answers 2.
2. **i2, the moveto.** 8 of the 23 order-1s took no task. The kind-7 install read the member's old command
   (`0071BE40`), for example a torpedo run on a dead target, and refused it. The moveto is now issued on each member
   (`entity_issue_command`, the position descriptor of `00468560`), and `0099A170` runs after its delivery, as the Lua
   moveto does. In i3 every order 1 leaves its squadron holding, and its order 3 is accepted.

**How the win comes about (i3), from the per-entity tables.**
- The re-sent squadrons take their new targets. Each of the 24 accepted order-3s is followed by `command target
  0071EBF0` lines naming the new target, from 24 to 666 lines per squadron.
- Two of them sink their targets earlier:
  - Grayson, by `Chitose_sqn37` (re-sent), at 1642 s against 2263 s;
  - Woodworth, by `Chitose_sqn47` (re-sent), at 2346 s against 3209 s.
- Re-sent squadrons fight, then die or land, and their slots free (stock return, SHIP_AI 200.2). Then the
  launch lines reach Cummings: `Chitose` slot 2 queues class 162 at Cummings, and `Chitose_sqn51|.-3` sinks it at
  3385 s.
- In OFF every slot stays in state 3 to the end. Its squadrons loiter over dead targets, so no launch line is ever
  accepted for Cummings.
- Deaths move 675 -> 685. Before the ON mission end at 3387 s there are 637 OFF deaths and 648 ON deaths. The mission
  then enters EndScene, so the later deaths are not comparable.

**Verdict: ON.** The mechanism matches the image's arms, and it reaches the mission's scripted win. The
`planes` count in the log now counts members that were issued the moveto: the install runs after delivery. The
i3 logs show 0 for order 1 because they predate that change.

Still not served: the state-2 retarget and cancel (`006CC5C0`), and the state-5 order-2 re-arm. A landed and
stowed squadron (state 2 members) still cannot fly the moveto.

#### Orders to a scene squadron's slot, `kAirOpsSceneSquadronOrdersBound` (packet `cc9_lua47_scene_squadron_orders`, cc9-lua47, 2026-10-06)

JM08 as the IJN player (SHIP_AI_OPEN_ITEMS 193/195): `MainAirFieldEntity 01`'s slots are held by the scene squadrons
`Ki-43 Oscar 01` and `Gekko 01`. Their `HomeBase` key queues them on the deck (`007F1C00` -> `006CC7B0`), and
`006C58A0` -> `006C56D0` stores the squadron id at slot `+28h`. A 6000-frame probe (`local\l47pr_R1_j8.log`, ships38's
`s38_j8_p1.txt` plus `order` lines) refused all 156 orders: "the slot holds no squadron this host built". The entry
looked the squadron up only among the squadrons this host launched.
- **`006CCDA0` does not care who built the squadron.** It reads it through the slot's `+28h`. With the switch, the entry
  resolves `+28h` to the squadron unit (id - 1, the numbering `006CC7B0`'s push and the air-ops launch share) when no
  squadron launched by this host holds the slot. Committed OFF.
- **Predictions.** The base row is JM08 24000 with `local\l47_j8_o1.txt` (ships38's p1, plus order 1 / order 3 lines for
  the airfield's slots 1-4 every 2000 frames from 400).
  - **OFF:** identical to a run without the order lines (every order refused).
  - **ON:** the scene squadrons' slots log `holds scene squadron Ki-43 Oscar 01` / `Gekko 01`. Each order 1 answers 1
    (moveto at the leader). The next order 3 at a transport goes through the intake `007F1940`, and the fighters attack
    the transports. A fighter has no bomb rack, so it is expected to strafe at most.
  - Once the scene squadrons die or land, their slots should free (stock return), and ships38's `launch` lines on the
    airfield should then go through.
  - **Mechanism failure:** an order that answers 1 but whose members never take the target.
