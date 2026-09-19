# What ends a ship's running command, and why the USN04 Yorktown lost its path

Addresses: `00836920`, `00836941`, `00836962`, `008369A1`, `00836A8B`, `00836BF0`, `00836DC9`,
`0071D810`, `0071E430`, `00A2C790`, `00A2CBD0`, `00A02020`, `00A10DC0`, `0077C8D0`, `0071EB60`,
`0071BE40`, `00D21530`, `00D09FE8`, `00D7A218`.

Packet `cc8_ship_command`, on top of `docs/SHIP_AI_DRIVE_GATE.md`, `docs/SHIP_AI_PATH_CURSOR.md`
and `docs/AI_COMMAND_TICK.md`. Every descriptive name is a hypothesis, not a recovered symbol.

The question this packet inherited: the Yorktown closed 5850.94 m to 444.20 m of its first
`CarrierPath4` point at 16.7 m/s, then **left `moveonpath` at about director step 6980 and
finished a 450 s run in `cruise`**, with `end_commands=5` and no cursor advance ever. The arrival
test could not be what ended it: it failed on all 6467 tries.

## 1. `00836920`'s stage spine, read whole

`00836920`-`00836EA7`, 409 instructions, read from the listing. The director step is a ladder on
the primary stage `[director+48h]`, and **stage 2 is terminal**: `00836A81 CMP [ESI+48h],2 / JZ
00836DC9` jumps the whole command-class dispatch straight to the idle tail.

Six sites raise the stage with `PUSH 2 / CALL 0071D810`. Only two of them can touch a running
`moveonpath`, because the other four sit inside a class arm for `stop`, `00E08F60` or `00E08F78`:

| site | condition | arm |
| --- | --- | --- |
| `00836981` | stage == 1 **and** (`0071BE60() > 1` **or** `[unit+184h]`) | the pre-pass |
| `00836A7C` | stage == 0 and `0071BE60() > 1` and the queued tail is a weapon command more than `sqrt(00D09FE8)` away | the far-weapon-target arm |
| `00836AD2` | head is `stop` `00E08F88` | the `stop` arm |
| `00836B3B` | head is `00E08F60` | the follow arm |
| `00836BB6`, `00836BE6` | head is `00E08F78` | the `00E08F78` arm |
| `00836D12` | head is `moveonpath` and `007ADD70` answered true | **arrival** |

`00836BF0 CMP EAX,0xE08F80 / JNZ 00836D67` is the `moveonpath` arm, and the only stage raise it
contains is the arrival at `00836D12`.

One further ender sits outside the director step and is ruled out by the follow mode: the ship-AI
`moveonpath` state step `009E59C0` ends the command itself from its `finished` block
(`009E5C70`, `src/ship_ai_state_steps.cpp`), but that block is reached only when
`007ADC60 BSP_EntityCommand_IsOnFinalLeg` answers true, and `007ADC8F`'s two-value test makes
`PATH_FM_PINGPONG` and `PATH_FM_CIRCLE` answer **false always**
(`docs/SHIP_AI_PATH_CURSOR.md` section 3). USN04's carriers are ordered `PATH_FM_CIRCLE`, mode 3,
so it cannot fire for them.

So, short of arriving, a running `moveonpath` on these two units can be ended by exactly two
things, and **both of them require `0071BE60`'s filled-slot count to be above one** unless the
unit is the controlled one:

```
00836962  CMP dword ptr [ESI + 0x48],EBP        ; EBP = 1, the running stage
00836965  JNZ 0x0083698a
00836967  MOV ECX,ESI
00836969  CALL 0x0071be60                       ; the filled-slot count
0083696e  CMP EAX,EBP
00836970  JG  0x00836981                        ; more than one slot -> raise
00836972  MOV EAX,dword ptr [ESI + 0x24c]
00836978  CMP byte ptr [EAX + 0x184],0x0
0083697f  JZ  0x0083698a                        ; not the controlled unit -> leave it
00836981  PUSH 0x2
00836985  CALL 0x0071d810                       ; terminal
```

**A second command queued behind the running one terminates the running one, on the very next
director step.** That is the whole rule, and `docs/SHIP_AI_DRIVE_GATE.md` section 4 read only its
`[unit+184h]` half, because that is the half its `+184h` census was looking for. This host already
models both halves, in `weapon_director_step_prepass_00836941` (`src/unit_commanded_speed.cpp`,
the `crowded` local), so the behaviour under test was never a missing projection. **What was
wrong is that something queued a second command into the Yorktown that the image never queues.**

**A gap, named and not fixed.** Of the two, only the pre-pass is wired into this host's step.
`weapon_director_abandon_for_far_weapon_target_008369a1` exists in `src/unit_commanded_speed.cpp`
and is declared in the header, but **no caller anywhere in `src/` runs it**:
`GameCommandsHost::director_step_00836920` calls the pre-pass, the `stop` arm and the idle tail
and skips `00836998..00836A7C` entirely. It is a partial projection by its own ledger record (the
target resolve `00521EA0`, the world position `00427EB0` and the two `vtable[0Ch]` category calls
are all caller-supplied), and wiring it needs the queued tail's target position, which this host's
command rows do not carry. So this host's command lifetime is strictly **more permissive** than
the image's at that one site. It does not affect anything below: that arm also requires the
filled-slot count above one, so it can only ever end a command the pre-pass would already end one
stage later.

Two constants read at the width of their loading instruction (`tools/pe_const_read.py`):
`00D09FE8` is the double 4000000.0, so the far-weapon-target arm's radius is 2000 m; `00D7A218`
is the float 0.0, which is the boundary the `stop` arm and the idle tail compare the commanded
speed against, and `navigator_commanded_speed_active` already treats 0.0 as active.

## 2. The second command was an `attackmove`, and its producer is this host's own stand-in

The 450 s run's `command finished` rows for `Yorktown-class01` are one `cruise`, one `stop` and
**four `attackmove`**, and every `attackmove` arrives in a fleet-wide wave: one
`player command issued to "<name>": token="artillery" resolved="attackmove"` row for every unit
alive at that moment, carriers, cruisers, destroyers and the newly spawned squadrons together.
The run's own order table names the producer: `Yorktown-class01 attackmove **scene** 4`, against
`Yorktown-class01 moveonpath script:NavigatorMoveOnPath 18`.

That producer is `src/game_hosts_ai.cpp`'s `order_attack`, whose tail issued a scene command to
every member of the group:

```cpp
// The substitution for 00A2C790's unread member->vtable[+114h]: each member
// gets the order as a scene command.  ... a ship takes `artillery`.
```

The substitution is wrong in kind, and `00A2C790` is now read.

### `00A2C790` reads the members, it does not order them

`00A2C790 BSP_AiGroup_MemberPass`, body `00A2C790`-`00A2C8C6`, 98 instructions, read whole. Per
member it calls `member->vtable[+114h]` **three times** and uses the answer three ways:

```
00a2c7f0  MOV ESI,[EDI+8]                  ; the member entity
00a2c7f5  CALL [vtable+114h]               ; -> X
00a2c801  JZ  00a2c842                     ; X null -> skip this member
00a2c805  CALL [vtable+114h]               ; X again
00a2c811  CALL 0071eb60                    ; BSP_EntityCommand_ActiveTargetDescriptor(X) -> EDI
00a2c81a  CALL [vtable+114h]               ; X a third time
00a2c826  CALL 0071be40                    ; the current command of X          -> EAX
00a2c82b  MOV ECX,[EBP+564ch]              ; the group's command object
00a2c839  CALL [command->vtable+24h]       ; (EAX, EDI)
```

`0071EB60` and `0071BE40` are both director readers, so `vtable[+114h]` answers **the member's
weapon director** and the pass **collects every member's current command and active target
descriptor and reports the pair upward to the group command**. Nothing in it writes a slot. Its
tail `00A2C859`-`00A2C8BC` is a cooldown: it returns while `[group+5650h]` is still ahead of
`[00F876A4]`, otherwise runs the command's `vtable[+0Ch]` and `vtable[+10h]` and re-arms
`[group+5650h]` with `00BD2F10`'s draw.

`00A2CBD0 BSP_AiGroup_OrderAttackOnGroup` does not reach a member either: its seven callees are
the two attack-command constructors, `00A10C20 BSP_AiGroup_LeaderPoint`, `00A2C9F0`, the random
draw, `operator new` and one libcrt helper. **Neither `00A02020` nor `0077D600` is among them.**

### The image's split is by kind, and a ship follower gets no command at all

`00A02020` is the one bridge from an AI command to a scene command
(`docs/AI_COMMAND_TICK.md`), and the follower pass `00A10DC0` decides who reaches it.
`00A10DC0`'s per-follower body, read from the listing:

```
00a10e36  MOV ESI,[EBP+8]                  ; the follower
00a10e3e  PUSH 0x6 / CALL [vtable+5ch]     ; a ship base?
00a10e46  JZ  00a10e72                     ; no -> the squadron arm
00a10e61  MOV EDX,[EDI+8]                  ; the group's first member, the leader
00a10e67  CALL 0077c8d0                    ; RequestJoinFormation(follower)(leader)
00a10e70  JMP 00a10e9d                     ; next member - nothing else happens
00a10e72  PUSH 0x18 / CALL [vtable+5ch]    ; a plane squadron?
00a10e83  CALL 009ffeb0                    ; the second squadron exclusion
00a10e8c  ... 00a02020(follower, leader point)
```

**A ship follower receives `0077C8D0` and nothing else.** `0077C8D0` itself, body
`0077C8D0`-`0077C97B` read whole, asks `entity->vtable[+16Ch]` with the literal `00CFB52C` and
then routes a session message through `0077C2A0 BSP_Session_RouteMessage` carrying a record whose
class pointer is `00D02D30`. It never calls `0077D600` and never names an `00E08Fxx` command
object, so it is not the scene-command push that fills a director slot.

So in the image an attack order reaches a ship follower as a formation request, and the only
scene command any AI command class issues anywhere is `00E08F68 moveto`, to the leader and to
squadron followers. **A ship follower's director queue is never touched, so its running
`moveonpath` never meets `00836962`'s `count > 1`.**

### What the run measures, and a correction to this packet's own first reading

**The `attackmove` does not queue behind the `moveonpath` and terminate it through `00836962`.
It replaces the whole queue.** The trace row that ends the path order is

```
349.29s  Yorktown-class01  issue scene/attackmove pushed  stage 0 -> 0
                           slots 3 -> 1   head moveonpath -> attackmove
```

Three filled slots become one, the head is the new command, and the stage never moves. So the
`moveonpath` is **discarded**, not terminated: this host's scene-command path carries the same
replace semantics `0077D600` has in the image. This packet's first reading was that the extra slot
made `0071BE60`'s count 2 and the pre-pass raised the running command to the terminal stage; the
run refutes that for this event, and it is retracted.

`00836962`'s count arm is nonetheless real and observed - it just ends a different command. At
3.10 s and 111.10 s the Yorktown's trace reads

```
  3.10s  prepass 00836941  stage 1 -> 2  slots 2 -> 1  head cruise -> moveonpath   RAISE
111.10s  prepass 00836941  stage 1 -> 2  slots 2 -> 1  head stop   -> moveonpath   RAISE
```

which is the arm doing its job: a queued `moveonpath` waiting behind a running `cruise` or `stop`
ends that command and is promoted. **It cannot reach the `moveonpath` itself in this host, because
the pre-pass requires stage 1 and a `moveonpath` head never leaves stage 0 here** - only the
`cruise` and `stop` begins raise the stage (`finish_issue`'s two arms). Whether `00835C70`
raises the stage for a `moveonpath` in the image was not read, so "a `moveonpath` at stage 0
forever" is a **named hole**, and it is the reason the count arm is harmless here rather than a
proof that it would be harmless in the executable.

The Lexington's trace answers the coordinator question in the same run:

```
  0.05s  prepass 00836941  stage 1 -> 2  slots 1 -> 0  head cruise -> (none)   RAISE
  7.00s  issue ai_command_tick/moveto pushed  slots 1 -> 1  head attackmove -> moveto
  9.05s  issue script:Navigator/moveonpath    slots 1 -> 2  head moveto -> moveto
```

The two raises in the first 0.10 s happen with **one** filled slot, so they are the `[unit+184h]`
arm, not the count arm: the controlled unit's running command is terminated every step, as
`docs/SHIP_AI_DRIVE_GATE.md` section 4 says. After that the head is `moveto` from the class tick
for the rest of the run, the script's `moveonpath` sits behind it at slot 1 and never becomes the
head, and the stage stays 0. **There is no gate excluding the player's unit from `00A02020`** -
the `+184h` census does not list it and the listing has no such test - so the image would issue
into it too, and a group leader receiving `moveto` is what the image does. The Lexington standing
still in `cruise` with no player input is faithful and is left alone.

### What this host does now

`order_attack` issues to the group **leader** and to **plane members** only; a ship member that is
not the leader is skipped and counted as `ship_followers_not_ordered` on the coordinator summary.
The plane tokens are unchanged and remain the labelled substitution they always were: a squadron
follower is genuinely reached by `00A02020`, and the 26-row registry has no single "attack" token
(`docs/ENTITY_LUA_ORDER_PATH.md`).

**Named hole.** What `0077C8D0`'s session message does at the far end is not read, so "a ship
follower joins a formation" is the shape of the call, not a recovered behaviour. This host has no
formation ring and issues nothing in its place. That is a hole, not a proof.

## 3. `00D21530` is 6400.0, and the old reading took half of it

`docs/AI_COMMAND_TICK.md` recorded `00A02020`'s distance gate as inert: "the dword at `00D21530`
reads `00 00 00 00`, so `d2 >= 0.0f` always holds and the distance test never blocks an order",
and `include/bsp/ai_command_tick.hpp` carried `kAiOrderIssueDistanceSquared = 0.0f`.

The loading instruction is `00A02098 FLD double ptr [0x00D21530]` - **eight bytes**. 6400.0 as an
IEEE double is `00 00 00 00 00 00 B9 40`, whose low dword is `00 00 00 00`: exactly the bytes the
old note read, and exactly half the constant.

```
tools/pe_const_read.py f:00d21530  ->     0.0
tools/pe_const_read.py d:00d21530  ->  6400.0
```

So the gate is a real **80 m radius**: `00A02020` issues nothing to a member already within 80 m
of the point it would be ordered to. The constant is corrected here and in the header, and the
doc carries a correction block rather than a rewrite. This is the class
`tools/const_width_sweep.py --all --load-sites` exists to catch.

## 4. What is measured

Two USN04 runs, `--frames 9200 --mission-frames 9000` (449.96 s), same binary apart from the
change under test: `local/cmdlife_before_usn04.log` and `local/cmdlife_after_usn04.log`.

**Both runs were taken on `agent/cc8-ship-command` based at main `3a691e884`, which is BEFORE
main `f14732dc4`.** That commit gave every weapon class with a Blast table an impact burst
(`0084BC60` step 7), and it moved the gunnery totals on every mission; `docs/GAME_EXECUTABLE.md`
carries the new reference lines. The pair below is internally valid, because the two runs differ
only by the change under test, but **neither number here may be compared against a run taken at
`f14732dc4` or later**, and re-taking this pair on merged main would move the gunnery columns. The
rows this packet rests on - the path cursor, the command tables and `end_commands` - are ship
geometry and command bookkeeping rather than gunnery, so the conclusions stand; `total_path` is
the one row where a later re-take could drift, since a ship that sinks earlier stops moving.
The before run reproduces `docs/HANDOFF_SHIP_DRIVE_NEXT.md`'s 450 s numbers exactly
(`end_commands=5`, `moved=100.51`, `total_path=108653.79`), which is what says the trace is
additive.

| row | before | after |
| --- | --- | --- |
| `path cursor Yorktown-class01 CarrierPath4` | `at 2  advances 0  travelled 3957.45` | **`at 3  advances 1  travelled 7351.86  legs 2>3`** |
| `summary mission director completion` | `end_commands=5 stage_raises=10 queue_advances=10` | `end_commands=0 stage_raises=4 queue_advances=4` |
| `summary mission commands` | `resolved=160 issued=1190 pushed=924 steps=2102` | `resolved=61 issued=1088 pushed=815 steps=31` |
| `summary mission world` | `moved=100.51 total_path=108653.79` | `moved=100.51 total_path=9811.70` |
| `ai coordinator` | `tick_orders=217 tick_followers=796 formation_requests=600` | identical, plus `ship_members_not_ordered=27` |
| Yorktown `cmdlife` rows | 27 | 5 |

**The measure the packet was set is met.** The Yorktown reaches its first `CarrierPath4` point and
the cursor advances for the first time in any run: `advances 0 -> 1`, and `legs 2>3` says which
leg it moved onto. Its whole command history after the fix is five trace rows, the last at 6.05 s:

```
3.10s  prepass 00836941  stage 1->2  slots 2->1  cruise -> moveonpath   RAISE
6.05s  issue script:Navigator/moveonpath  slots 2->3  moveonpath -> moveonpath
```

and the head stays `moveonpath` for the remaining 443 s. `end_commands` falls to **0**: nothing
ends any command on either carrier for the whole mission.

**The coordinator line is identical except for the new counter**, so the attribution is clean:
`tick_orders`, `tick_followers` and `formation_requests` are unchanged at 217, 796 and 600. The
class tick still orders exactly what it ordered; only `order_attack`'s per-member scene command
stopped, 27 times.

### Three things this does NOT show, said plainly

1. **Successive points in order, and the circle wrap, are still unobserved.** One advance is all
   450 s buys: the first point was 5850 m away and the hull makes 16.7 m/s, so arrival lands near
   350 s and only about 100 s of the next leg is left. Reaching all eight points and wrapping needs
   a much longer run, and nobody has seen `007ADD70`'s multi-leg advance yet either, so
   `docs/SHIP_AI_PATH_CURSOR.md`'s hole 1 stands exactly where it was.
2. **`total_path` falls from 108653.79 to 9811.70, and that is the honest cost of this change.**
   The Yorktown accounts for 7351.86 of the 9811.70; the escorts have stopped moving. They were
   moving because `order_attack` handed every one of them an `attackmove`, which the image does
   not do. What the image does instead is `0077C8D0 RequestJoinFormation`, and this host has no
   formation ring - `formation_requests=600` are recorded and do nothing. So the change converts a
   stand-in that produced motion into an **honest hole that produces none**. It is not a claim
   that the escorts should be still; it is a claim that nothing recovered so far says they should
   move, and the one routine that would move them is unimplemented.
3. **Whether a `moveonpath` head reaches stage 1 in the image was not read.** In this host it
   never leaves stage 0, which is the only reason `00836962`'s count arm cannot reach it even at
   three filled slots. If `00835C70` does raise the stage for a path command, the image would end
   a `moveonpath` whenever a second slot is filled and this host would not. Named, not fixed.

### A diagnostic defect found on the way, not fixed

The script-orders table prints the carriers' path target as `CarrierPath4` for the first eight
orders and then as `B5N Kate #2.1` (`D3A Val #1.1` for the Lexington), 141 rows of it, although
`usn_19_coralus.lua` lines 515 and 520 always pass `FindEntity("CarrierPath1")` /
`FindEntity("CarrierPath4")`. The column is `name_of(entity_from_argument(1))`, the resolved path
entity. It is **most likely a naming artefact rather than a resolution drift**: a real drift would
make each repeat a non-repeat, push a slot every 3 s and end the command dozens of times, and the
before run ends commands 5 times while the cursor still reports `CarrierPath4 pts 8`. That
inference is not a proof, the row will mislead the next reader, and it belongs to the
navigator-path area rather than this packet.

## 5. Coverage

| Address | Coverage |
| --- | --- |
| `00836920` | complete as rules, `00836920`-`00836EA7` read whole from the listing |
| `00A2C790` | complete, `00A2C790`-`00A2C8C6` read whole |
| `00A2CBD0` | callee census only; the body was read in full by `docs/AI_COMMAND_LIFETIME.md` |
| `00A10DC0` | the per-follower body read whole; the list walk read as a walk |
| `0077C8D0` | `0077C8D0`-`0077C97B` read whole; its message's far end **unread** |
| `00A02020` | complete, and its distance constant corrected |
