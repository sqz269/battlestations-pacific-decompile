# IsReadyToSendPlanes and LaunchSquadron, 00895D20 and 0089E3C0

Addresses: 00895D20, 006BF620, 006BCD20, 0089E3C0, 006CC690, 006C7210, 006C7490, 006CA640,
00964790, 00CADD0 mode 3.

Evidence: the listings of 00895D20 and the decompilations of 006BF620, 0089E3C0, 006CC690 and
006C7210. The two failure flags are named from 006CADD0 mode 3's own descriptors. Every offset
below is a native one.

## 1. IsReadyToSendPlanes, 00895D20

The listing settles the register ABI, which the pseudocode leaves implicit:

```
00895E39  MOV ECX,ESI / CALL 006BCD20   the block, entity in ECX and DL zero, result in EDI
00895E44  MOV EAX,[EDX+0x5c]
00895E47  PUSH 0x45 / CALL EAX          the class test, 45h
00895E4F  JZ  00895E5D                  not that class -> the block test
00895E51  CMP byte [ESI+0x720],0        the entity's own byte
00895E58  JZ  00895E5D                  clear -> the block test
00895E5A  PUSH EBP                      set -> push false
00895E5F  MOV ECX,EDI / CALL 006BF620   the block test
```

So the false arm is taken **only when both** the class test against 45h passes and the byte at
entity+720h is set. A mother ship can never take it, because its class test fails. Everything else
is `006BF620` over the block.

`006BF620` is five tests, and all five must hold:

| test | field |
| --- | --- |
| byte is clear | block+1Ch |
| byte is clear | block+1Dh |
| pointer is non-null | block+7Ch |
| byte is clear | that object's +5Dh |
| int is zero | block+38h |

**block+1Ch is `runwayFailure` and block+1Dh is `hangarFailure`.** 006CADD0 mode 3 builds a
descriptor pair for each and terminates the group with the name: `LEA EDX,[EBP+1Ch]` at 006CBE20 is
followed by the terminator carrying `runwayFailure` at 006CBE35, and `LEA ECX,[EBP+1Dh]` at
006CBE62 by `hangarFailure` at 006CBE77. Neither is a scene key, so both start clear.

**block+38h is provisionally the launch already in progress.** Three sites read it: 006BF620
requires it zero for readiness, 006CC690 branches on it at 006CC715 to choose between starting a
launch and queueing one, and 006C5050 refuses at 006C5078 when it is set. All three treat it as
"something is already pending". **None of them writes it**, and no writer has been found, so the
name is an interpretation of three readers rather than a recovered meaning. This paragraph
originally called it named on the strength of two agreeing sites; agreeing readers are weaker
evidence than that wording implied. See `docs/AIROPS_LAUNCH_START.md`.

block+7Ch is the **owning entity**, named since this document was written by `006C5050`, which
reads its virtual at +12Ch for `Skill`, its +54h for `Party`, its +188h for `OwnerPlayer` and
passes the pointer itself as `HomeBase`. Its +5Dh byte is still unnamed. This process has no such
entity object.

## 2. LaunchSquadron, 0089E3C0

```
argument 0  the entity -> 00888AA0 -> 006BCD20 for the block
argument 1  an integer -> 00964790 BSP_VehicleClass_GetOrCreate -> the class
            the arm starts as class+134h
            when the argument count is 4 (the 00B663F0 test), argument 3 replaces it
            006BF620 is called again
argument 2  the count
            006CC690(block, class, count, arm)
            BSP_SEntity_InitAll
            push the 006CC690 result PLUS ONE
```

That `+1` is what closes the loop with the mission script. `LaunchSquadron` returns a **1-based
slot index**, and `GetProperty(carrier, "slots")` publishes a 1-based array, so
`slots[slotIndex]` in `usn_19_coralus.lua` is the slot this call just wrote.

The arm default is class+134h, which is the same field the Lua reader publishes as `equipment` and
the scene authors as `Arm`. This is the third place that field appears and the first that shows
where its default comes from.

## 3. 006CC690, what the launch actually does

```
006CC6D6  CALL 006C7210                 pick a slot, index in ESI
006CC6E3  IMUL ESI,ESI,0x58
006CC6E6  ADD  ESI,[EDI+0x4c]           slot = index * 58h + block[4Ch]
006CC6E9  CALL 007B8A80                 resolve the class
006CC6EE  CMP  [ESI+4],EAX / JZ 006CC707    already this class: skip the class write
006CC6F5  MOV  [ESI+4],EAX
006CC6FA  MOV  ECX,[EAX+0x134]          the class's own arm, or zero at 006CC702
006CC704  MOV  [ESI+0x10],ECX
006CC70F  MOV  [ESI+8],ECX              count
006CC712  MOV  [ESI+0x10],EDX           arm, so the argument always wins
006CC715  CMP  [EDI+0x38],0 / JZ 006CC72E
006CC71F  CALL 006CA770                 non-zero: the stock goes back
006CC727  CALL 006CA640                 and the request is queued
006CC733  CALL 006C7490                 zero: the launch starts, block in ECX,
                                        the slot index and 0 pushed
```

`006C7210` walks the slots at stride 58h while the index is below block+50h and takes the first
whose state at slot+2Ch is **1 or 5**, which are `kCooldown` and `kReady`. When the walk finds
none it continues into the array-growth path, so the index it returns is one past the last slot.

**006CC690 does not create a squadron.** It writes the slot and asks `006C7490` to start the
launch, with the block in ECX and the slot index and a zero pushed. The entity that ends up at
slot+28h, which is what the Lua `squadron` key publishes, is filled later, by `006C65C5` and
`00896990` per `include/bsp/air_operations.hpp`. `006C7490` was not read and is the next packet.

## 4. The host

`src/game_hosts_lua.cpp` now runs both rows. `bsp::air_ops_is_ready_to_send_planes_00895d20` is
section 1's whole rule over the deck, and `bsp::air_ops_launch_squadron_006cc690` is section 3's
slot pick and writes, including the branch on the in-progress launch. Two summary lines report
them:

```
summary mission getproperty 0088bf80: calls=N served=N unserved=N slots_rows=N decks=N
summary mission airops gates: ready_calls=N ready_true=N launch_calls=N started=N queued=N
```

### Contracts

* **block+7Ch is a substitution.** The object it points at has no counterpart here, so a deck the
  scene loaded reports it present and unblocked. Without that, readiness could never be true and
  the gate would be answered by an absence rather than by the rule.
* **class+134h is not authored under any key** in this installation's `vehicleclasses.lua`, so the
  arm defaults to zero for a three-argument call where the native would use the class's own value.
* **entity+720h is not modelled**, so the airfield arm of section 1 never fires here.
* **006C7490 is unread.** The launch is requested and the slot is written, and nothing fills
  slot+28h, so the `squadron` key stays absent.
* The `006C0F00` count clamp from the previous packet still is not modelled.

## 5. What the next run will show, and the risk in it

This is the first change in the chain that can move the mission script past its launch line, so it
is worth saying plainly what it should and should not do.

`luaGetSlotsAndSquads` now sees real slots. `IsReadyToSendPlanes` can now answer true. The script
will then call `LaunchSquadron` and get a real 1-based index back, and index the `slots` array with
it successfully. The line after that is

```lua
Mission.ZuikakuZero = thisTable[tostring(GetProperty(Mission.Zuikaku, "slots")[slotIndex].squadron)]
```

and `squadron` is absent, because section 3 says what fills it is unread. `tostring(nil)` is
`"nil"`, `thisTable["nil"]` is nil, and the striker is nil. Whether the shipped script survives
that depends on what `PilotSetTarget` and `SetSkillLevel` do with a nil first argument, which is
not established here.

So the expected outcome of the next run is **progress, not a launched strike**: the think should
reach further than `commandhelpers.lua:2496` and may fail somewhere new. That would be a different
and more advanced failure, and naming `006C7490` as its cause in advance is the point of writing
this down before the run.

## Uncertainty

* What block+7Ch points at and what its +5Dh byte means.
* Where class+134h is authored, given no key in `vehicleclasses.lua` carries it.
* What state 6 means, carried from the previous packet.
* Whether the shipped script tolerates a nil striker, as section 5 says.

## Host methods

`bsp::air_ops_is_ready_to_send_planes_00895d20`, `bsp::air_ops_pick_launch_slot_006c7210` and
`bsp::air_ops_launch_squadron_006cc690`, with
`GameMissionLuaHost::run_is_ready_to_send_planes_00895d20` and
`GameMissionLuaHost::run_launch_squadron_0089e3c0` as the bindings.

## Corrections

None, and no retraction of a native-behaviour claim in this packet.

## no_ghidra_function

None.

## Validation

**Blocked.** Session 1, where the agents run, is disconnected, so no window, D3D device or FMOD
output can be created. The build is clean and both ctest suites pass. The run is one call:

```
./tools/run_game.ps1 -Log local\usn04_gates.log -WaitSeconds 2400 -- --frames 3200 `
    --press-start-frame 30 --menu-select USN04 --mission-frames 3000 --mission-frame-seconds 0.05
```

Read `summary mission airops gates` first: `ready_calls` above zero means the script reached the
gate, `ready_true` above zero means a deck answered yes, and `started` above zero means a slot was
written. Then read whether `script call Think failed` has moved off `commandhelpers.lua:2496`.
