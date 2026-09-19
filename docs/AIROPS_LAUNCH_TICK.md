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
006c659b: CALL 0x00922f30                    ; and disable its scene node
```

So block+38h is the one entity the deck has pulled out of a queue and is holding, hidden, with an
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

## 6. 006CDC70's nine sub-updates, and which of them this packet reconstructs

`006CDC70 BSP_AirOps_Update`, `__thiscall(block, step)`, is the block's own update, reached from the
owning unit's motion pass: `00758270 BSP_MotherShipUnit_UpdateMotion` for a carrier and `006D2510
BSP_AirField_TickAdvance` for an airfield. It runs, in order:

| callee | what it is | reconstructed here |
| --- | --- | --- |
| 006C58A0 | unread | no |
| 006C0DA0 | the slot walk: every slot of block+4Ch by index, calling 006C0510, routing 006BD520's message for each slot that returned true | **yes** |
| 006C77E0 | a compaction of the 14h-stride list at block+A8h | no |
| 006C64B0 | the state-2 wait described in section 1 | no |
| 006C6540 | the ready-plane pull of section 5 | no |
| 006CD240 | stock regeneration: a 14h-stride array at block+98h, `timer -= step`, and on expiry a reload from 006CC9F0 | no |
| 006C5B70 | unread, behind the two failure bytes and the owner's +5Dh | no |
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

## Uncertainty

* What fills the queue at block+D8h that `006C6540` drains. Until that is reconstructed this
  process has no brake on the script's launches, and the 24-squadron ceiling stands in for it.
* `006C58A0`, `006C5B70` and the virtual at block+3Ch were not read.
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
