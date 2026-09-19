# The ship unit group's follow chain: who orders the leader, and what moves the escorts

Packet `cc8_ship_follow`, branch `agent/cc8-ship-follow`, on top of the held `agent/cc8-ship-command`.
This document is the **map** asked for before any binding. Every address below was read from the
listing in this packet unless it is marked *(cited)*, in which case it comes from the named earlier
document, which read it from the listing itself. Names are hypotheses, not recovered symbols.

The question the packet exists to answer: the predecessor proved this host's AI coordinator gave
every ship in a group its own scene `attackmove`, and the image gives it none. Removing that
stand-in leaves every escort in every mission stationary (USN04 fleet `total_path`
108653.79 -> 9811.70). **In the image the escorts steam because they are followers in a unit
formation.** This is the machine that does it.

## 1. The chain, end to end

```
AI command tick  (00A124E0 MoveTo, 00A12A90 MoveToAttack, 00A12430 Idle,
                  00A152B0 CautiousMove, 00A15500 DefendPosition, +4 unnamed)
   |
   +-- first list member only  --> 00A02020 IssueMoveToMember      the LEADER's order
   |
   +-- 00A10DC0 FollowerPass, members 2..n
          ship  (vt[5Ch](6))   --> 0077C8D0 RequestJoinFormation(member, leader)
          kind 24 (vt[5Ch](18h)) --> 00A02020 to 00A10C20 LeaderPoint
   |
0077C8D0 --> vt[16Ch]("follow", leader) = 008162B0 availability
         --> session message type 76h, payload = leader id [leader+174h]
         --> 0077C2A0 RouteMessage
   |
0077FAD0 receiver, record kind 2 --> 00521E30 id->entity
         --> 0077F940 JoinOrMerge --> 0070DB20 create / 0070EF30 join
                                  --> 0070ED30 fills the member's four columns
         --> 0070D080 member record, then the columns from the payload
   |
the follower's OWN director, every idle step:
0083 6DC9..00836E3D --> 007788B0(unit) ? issue "follow" (00E08F60) at 007788D0(unit)
                                       : the cruise / stop pair
   |
ship AI 009E1610 follow step --> 009DF2D0 --> 0070D290 station point
                                          --> 00811150 -> 00810630 the leader's WAKE
                             --> 009DE050 navigation goal, 009DA3B0 station frame
```

## 2. Who orders the leader (item 1a), read this packet

`00A124E0 BSP_AiCommand_MoveTo_Tick`, `__thiscall(cmd)`, `RET 0`, body `00A124E0-00A1253F`,
33 instructions, read whole:

```
EDI = [[cmd+4]+5640h]              ; the member list's first node          00A124E6, 00A124F2
ECX = [EDI+8]                      ; the first member's entity = the leader 00A124FD
if (!009FE080 IsGroupableCombatant(leader)) return                         00A12500, 00A12507
ECX = leader ; EDX = LEA [cmd+8]   ; the command's own destination, inline  00A12522, 00A12525
00A02020 IssueMoveToMember                                                 00A12528
00A10DC0 FollowerPass                                                      00A1252F
JMP 00A11070                                                               00A12538
```

**The leader's movement is `00A02020`, with the command's own point, from the command tick.** The
same pair appears in every tick that calls the follower pass: `00A12A90 MoveToAttack_Tick`'s callee
list carries `00A02020`, `00A10DC0`, `00A10C20 LeaderPoint`, `00A2BD00 SetCommand` and
`00A10710 AiCommandAttack_ConstructBase`; `FUN_00A11FF0` and `FUN_00A126C0` carry `00A02020` and
`00A10DC0` too. So **an attack order reaches a ship leader the same way a move order does**, through
`00A02020`, and reaches a ship follower not at all.

`00A02020`'s 80 m issue gate (`FLD double [00D21530]` = 6400.0) is the predecessor's correction in
`docs/AI_COMMAND_TICK.md`; it applies to this leader order.

This settles the predecessor's open question in the affirmative and **its fix is safe for the
leader**: what it removes is the scene command (`issue_to_member`, the `artillery` token), not the
class tick's `moveto`. The image's leader order lives in `ai_command_tick`'s `tick_orders`, which
this host already runs.

## 3. The follower side (item 1b)

### `00A10DC0 BSP_AiCommand_FollowerPass`, `__thiscall(cmd)`, `RET 0`, body `00A10DC0-00A10EB6`, read whole

* `if ([[cmd+4]+5644h] <= 1) return` (`00A10DC9`): the pass runs only with more than one member.
* The walk **starts at the second element**: `ECX = [[cmd+4]+5640h]` is the list head, `ESI = [ECX]`
  the first element, and `ESI = [ESI]` at `00A10DF8` steps past it before the loop. **The leader is
  skipped by the iteration, not by a test** - there is no kind test and no `+184h` test guarding it.
* Per member `ESI = [EBP+8]`:
  * `ESI->vtable[5Ch](6)` true (`00A10E3E`, the ship base kind, `kUnitGunneryKindShipBase = 6`):
    re-read the first element, `EDX = [first+8]` = the leader, `ECX = member`,
    `CALL 0077C8D0` (`00A10E67`), then straight to the next member. **No command is pushed.**
  * else `ESI->vtable[5Ch](18h)` true (`00A10E77`) and `009FFEB0(member)` false (`00A10E88`):
    `00A10C20 LeaderPoint` then `00A02020 IssueMoveToMember` (`00A10E8F`, `00A10E98`).
    Kind 24 is **not** `kUnitGunneryKindPlaneBase` (0Fh); what class it selects is unidentified here.

### `0077C8D0 BSP_Entity_RequestJoinFormation`, `__thiscall(entity)(void* other)`, `RET 4`, body `0077C8D0-0077C97B`, read whole

```
if (!entity->vtable[16Ch](0CFB52C "follow", other)) return          0077C8F8, 0077C902
if ([entity+1ACh] <= 7) 00905300([[00E188A8]+21A0h], [entity+1ACh]) 0077C90A..0077C91B
M = message(76h) via 0075B430                                       0077C920, 0077C926
M+18h = 0 (word) ; M+1Ah = 0 ; M+1Ch = 0 ; M+20h = [other+174h]     0077C934..0077C951
0077C2A0 RouteMessage(this = entity, M, 7, 0)                       0077C964
```

`M` is the local at `S+8` where `S` is `ESP` before the `PUSH 76h`, confirmed by the second
`LEA EDX,[ESP+0x10]` at `0077C95D` resolving to the same address. **The only payload the request
carries is the leader's 16-bit id at `[other+174h]`.** The routine pushes no command and touches no
formation state directly.

### `0077FAD0`, the receiver, `__thiscall(entity)`, body `0077FAD0-0077FE7E`, 245 instructions; the kind-2 arm read whole

`EAX = [entity+0C0h]`, `[EAX+4]` is the record kind, `EDI = [EAX+8]` the payload. For kind 2:

```
if (![EDI+0C5h]) skip                                               0077FB0C
if ([EDI+0C6h]) {                       ; the RESHAPE arm
    if (![entity+284h]) 0070DB20(entity)                            0077FB22..0077FB2D
    [entity+284h]+4FCh = [EDI+0CAh]                                 0077FB32..0077FB3F
    0070EFD0([entity+284h], [EDI+0CBh])                             0077FB45..0077FB53
} else {                                ; the JOIN arm
    other = 00521E30([EDI+0C8h])  ; 16-bit id -> entity             0077FB5D, 0077FB64
    if (!other) skip                                                0077FB6B
    0077F940(entity, other)                                         0077FB74
    rec = 0070D080([entity+284h], entity)                           0077FB80
    if ([entity+284h]+4FCh == 18h)                                  0077FB8B
        rec+4h,+8h,+0Ch = [EDI+0CCh,0D0h,0D4h]      ; relative position
    else
        rec+10h,+14h,+18h,+1Ch = [EDI+0D8h,0DCh,0E0h,0E4h]   ; the four ACROSS columns
        rec+20h,+24h,+28h,+2Ch = [EDI+0ECh,0F0h,0F4h,0F8h]   ; the four ALONG columns
}
```

The record offsets are exactly the ones `0070ED30` writes *(cited, `docs/SHIP_AI_FORMATION.md`)* and
`0070D290` reads, so **the message can overwrite the join-time columns**. `group+4FCh` is the field
`0070DB20` sets to 6 for a ship leader *(cited)*; the `== 18h` test selects the three-float form.
The payload offsets `0CCh` and beyond are **not written by `0077C8D0`**, so which sender fills them
is unresolved - see section 7.

`0077F940 BSP_UnitGroup_JoinOrMerge` and `0070EF30` / `0070ED30` / `0070EFD0` / `0070D290` /
`009DF2D0` are read whole in `docs/SHIP_AI_FORMATION.md` and reconstructed in
`src/ship_ai_formation.cpp`; `009E1610 follow` is read whole in `docs/SHIP_AI_FOLLOW_LAND.md` and
reconstructed in `src/ship_ai_follow_land.cpp`. This packet re-read none of them.

## 4. The producer of the `follow` state, and it is not the coordinator

**Credit, and a retraction of this section's first draft.** I wrote that this was in no document. It
is: `docs/COMMAND_COMPLETION.md` line 30 records that `00836920`'s tail "re-issues a standing
`cruise`, `stop` or `follow` the moment the queue empties", and the ledger evidence packet
`cc2_command_completion` left on `007788D0` already names `00836E28` as the site and `00E08F60` as
what it issues. What this packet adds is the two predicates read whole, the fact that the arm is
**missing from this host's model** of the re-issue, and the command's lifetime arm in section 4b.

`00E08F60`, the `follow` command object, is pushed at five sites; the one on the AI path is
`00836E38`, inside `00836920 BSP_WeaponDirector_Step`'s idle re-issue. Read whole at
`00836DC9-00836E3D`:

```
00836E0D  ECX = [director+24Ch]                 ; the unit
00836E13  CALL 007788B0                         ; -> AL
00836E1A  JZ 00836E3F                           ; false: the cruise / stop pair
00836E1F  ECX = [director+24Ch]
00836E28  CALL 007788D0                         ; -> the leader
00836E32  CALL 00465080                         ; the command argument pair, with 0.0f
00836E38  PUSH 0E08F60h                         ; "follow"
00836E3D  JMP 00836E90                          ; issue
```

with the two predicates read whole:

```
007788B0 __thiscall(unit) -> bool      ; body 007788B0-007788C7
    g = [unit+284h]; if (!g) return false; return [g+14h] != unit;
007788D0 __thiscall(unit) -> entity*   ; body 007788D0-007788DE
    g = [unit+284h]; if (!g) return 0; return [g+14h];
```

**A ship that is in a formation and is not its leader re-issues `follow` against its leader from its
own director, every idle step.** A leader, and any ship with no formation, falls through to the
cruise / stop pair that `docs/SHIP_AI_DRIVE_GATE.md`'s correction already describes - and that
correction, which read `00836E45` onward, **missed this arm above it**. It is not wrong about
cruise / stop; it is incomplete, and this section is the correction to it.

So no order has to reach a follower at all. Membership alone is the whole input: join the
formation, and the follower's own director drives it into `follow` for as long as it is a member.

### 4b. What ends a running `follow`, read whole at `00836ADC-00836B40`

The same routine's per-command-kind arm, where `EAX` is the running command's descriptor:

```
00836ADC  if (running != 0E08F60h "follow") -> the next kind (0E08F78h at 00836B45)
00836AE3  ECX = [director+24Ch]                      ; the unit
00836AE9  if (![unit+284h])            -> 00836B37   ; no group
00836AF2  if (!007788D0(unit))         -> 00836B37   ; no leader
00836B03  if (007788D0(unit) == unit)  -> 00836B37   ; the unit is now the leader
00836B0C  target = 00521EA0(director+58h)            ; the command's stored target
00836B1A  if (007788D0(unit) != target)-> 00836B37   ; the leader is no longer the target
00836B27  if (0071BE60(director) <= 1) -> 00836D67   ; nothing queued: keep running
00836B37  0071D810(director, 2)                      ; otherwise raise the stage: END
```

`follow` therefore ends exactly when the membership that produced it goes away - or when a second
command is queued behind it, the same `0071BE60 > 1` arm `docs/SHIP_COMMAND_LIFETIME.md` found
ending the Yorktown's `moveonpath`. Producer and terminator are both in `00836920`, and the host
models neither.

## 5. What this host has, and what it lacks

| piece | image | this host |
| --- | --- | --- |
| leader's order | `00A02020` from the command tick | bound (`ai_command_tick` `tick_orders`) |
| follower pass, ship arm | `0077C8D0` | counted only: `formation_requests`, `game_hosts_ai.cpp:1496` |
| join receiver | `0077FAD0` kind 2 | absent |
| the group object | `0070DB20` / `0070EF30`, `entity+284h`, 508h bytes | absent |
| member columns, station | `0070ED30`, `0070D290`, `009DF2D0` | reconstructed, **unbound**: `src/ship_ai_formation.cpp` |
| `follow` state step | `009E1610` | reconstructed, **unbound**: `src/ship_ai_follow_land.cpp`; `step_concrete=false` at `src/game_hosts_ship_ai.cpp:172` |
| the director's follow arm | `00836DC9` produces, `00836ADC` terminates | **missing both**: `weapon_director_idle_reissue_00836dc9` models cruise / stop only (`src/unit_commanded_speed.cpp:126`) |
| the leader's wake trail | `00810190` / `00810630` / `00811180` | **absent, and not reconstructed anywhere** |

Most of the primitives the five formation host interfaces need are already bound elsewhere in the
game hosts (`009DE050`, `00417B10`, `0082E850`, `0092D730`, `0080FC30`, `00811940`, `00414DB0`,
`004142E0`, `00424C40`); the ones that are not are small (`0042B260`, `00415510`, `0070D080`,
`0070D100`) - except the wake.

### The wake is the missing machine

`0070D290` measures a follower's station **back along the leader's wake**, so the trail is not
optional decoration; it is the frame the station lives in.

| address | name | instructions | state |
| --- | --- | --- | --- |
| `00810190` | `BSP_UnitWake_AppendSample` | 303 | unread, x87-dense |
| `00810630` | `BSP_UnitWake_SampleAtDistance` | 198 | unread |
| `00811180` | `BSP_Unit_DecomposeAgainstWake` | 523 | unread |
| `00811150` | `BSP_Unit_WakePointAtDistance` | 12 | trivial forwarder, `+0BD0h` |

The ring is 40 samples of `18h` bytes at `wake+8h` (`entity+0BD8h`) with the head index at
`wake+3C8h` (`entity+0F98h`) *(cited, `docs/SHIP_AI_FORMATION.md`)*, confirmed by this packet's read
of `00810190`'s indexing: `EAX = [this+3C8h]`, `LEA EAX,[EAX+EAX*2]`, `LEA EAX,[this+EAX*8+8]`.

`00810190`'s first 78 instructions, read this packet:

```
new = arg0[0..2] + [this+3D0h][0..2]          ; an accumulator at wake+3D0h   008101 98..008101EB
head = sample at this+8 + 18h*[this+3C8h]
d2 = (new.x-head.x)^2 + 0^2 + (new.z-head.z)^2                                0081020F..0081021D
if (16.0f > d2) return                        ; 00CE6454, float, JA 00810626  00810225..0081022F
if ([this+3D0h] == (00F87574,78,7Ch)) goto 0081032A   ; the three read 0.0 at load
0042B2F0 length(this+3D0h) ; sqrt(d2) ; * 0.25 (double 00D7A348)              00810273..00810293
```

**A sample is appended only after the ship has moved 4 m horizontally** (`d2 > 16.0f`). `00D7A348`
is the double `0.25`; read as a float it is `0.0`, the same trap `docs/AI_COMMAND_TICK.md` records.
The remaining 225 instructions are unread.

Its one caller is `00825F20 BSP_UnitInstance_UpdateShipMotion`, at `00826CEE`, with the tick's yaw
rate as the third argument (*cited*, `src/game_hosts_ship_ai.cpp:5156`, from
`docs/SHIP_AI_RUDDER_HOP.md`). **The host already ticks that motion virtual**
(`motion_step_00825f20`), so the append has a hook point that exists; what is unimplemented there is
the motion tail.

## 6. The cut this packet proposes

The chain does not fit one context at this project's reading fidelity: ~1000 instructions of unread
x87 wake code, ~37 host virtuals to bind, a group object this host does not have, and two
3000-frame runs. Proposed:

* **This packet, `cc8_ship_follow`**: the wake trail (`00810190` append and `00810630`
  sample-at-distance, read from the listing and bound into the ship motion tick the host already
  runs) plus `00811150`. Measurable on its own - samples appended per ship, ring depth, arc length -
  and it changes no ship's motion, so it is a same-binary null on every existing trace.
* **Next packet**: `00811180` decompose, the group object and the join receiver, then bind
  `0070D290` / `009DF2D0` / `009E1610` and add the director's follow arm at `00836DC9`. That is the
  packet where escorts move and the USN01 torpedo trace is re-taken.

The alternative cut - bind the group and the follow state first, with the wake projected - would
put a **hole** where column 0 lives: column 0 is "the relative position you had when you joined,
expressed in the leader's wake frame" *(cited)*, it is the live column for every runtime group
because the constructor leaves the pattern index at 0 *(cited)*, and it cannot be computed without
the trail. Doing the wake first is what keeps the station from being invented.

## 7. Uncertainties, and what is not read

* `00810190`'s last 225 instructions, `00810630` and `00811180` whole. No claim here depends on
  them except the 4 m threshold and the ring geometry, which are quoted from the instructions above.
* `00F87574/78/7Ch` read 0.0 at load because they are past `.data`'s raw size (loader zero-fill).
  Whether anything writes them at runtime is **unchecked**; a literal-address xref negative would
  not settle it (`docs/` records block-copy writers that xref only a base).
* The payload fields `0CCh`..`0F8h` that `0077FAD0`'s join arm copies into the member record are not
  written by `0077C8D0`. Either another sender of type 76h fills them, or they arrive zero and the
  join-time columns are destroyed. **Unresolved, and it decides whether `0070ED30`'s geometry
  survives a join.** It must be settled before the station is bound, not after.
* `00465080` (the command argument pair the follow re-issue builds) and `00905300` (the `[entity+1ACh] <= 7`
  call in `0077C8D0`) are unread.
* `009FE080 IsGroupableCombatant` and `009FFEB0` are unread; they gate the leader order and the
  kind-24 arm respectively.
* Kind `18h` in `00A10DC0` is not the plane base kind (`0Fh`). What class it is was not established.
* `bsp.py ghidra callers` under-reports (it returned 4 of 9 once tonight), so the nine callers of
  `00A10DC0` in section 1 are a floor, not a census. No claim here rests on the count.
