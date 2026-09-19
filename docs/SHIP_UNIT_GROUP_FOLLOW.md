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
0077FE80 dispatch, type 76h -> 0077FEC8 --> 00521E30 id->entity
         --> 0077F940 JoinOrMerge --> 0070DB20 create / 0070EF30 join
                                  --> 0070ED30 fills the member's four columns
     (0077FAD0, the AUTHORED placement record at entity+0C0h, is the other
      caller of 0077F940 and the only path that overwrites those columns)
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

### The receiver, corrected

> **Correction to this document's first draft.** I read `0077FAD0` as the receiver on the strength
> of `docs/SHIP_AI_FORMATION.md`'s line "its one call site is `0077FB74` in `0077FAD0`, the session
> message handler". `0077F940` has **two** call sites, and `0077FAD0` is the other one: its callers
> are `00742C10 BSP_LandConvoy_CacheParentAndPlacementLaw` and `FUN_0087BF80`, and it reads an
> authored record at `entity+0C0h`, not a wire message. The runtime receiver is `0077FE80`, as the
> packet brief said. Both readings are kept below, because both paths reach `0070EF30` and they do
> different things.

#### `0077FE80 BSP_Session_DispatchEntityKindMessage`, the runtime path

`__thiscall(entity /*ECX*/)(msg, bool* out)`, `RET 8`, body `0077FE80-00780041`, 156 instructions.
`msg->vtable[10h]()` gates it; then `[msg+10h] - 53h` indexes the byte table at `0078005C` and the
six-entry jump table at `00780044`. Read from the image bytes: index `76h - 53h = 23h` holds `02`,
and jump entry 2 is `0077FEC8`. **The whole type-76h arm is four instructions of work:**

```
0077FEC8  CX = [msg+20h]            ; the 16-bit id
0077FECC  00521E30(id) -> entity
0077FED8  0077F940(this = the receiving entity, other = that entity)
0077FEDE  return true
```

`[msg+20h]` is exactly the field `0077C8D0` writes (`M+20h` at `0077C951`), so sender and receiver
agree field for field. **The runtime join writes no columns at all**: they stay as `0070ED30` left
them. Two neighbouring arms are worth naming because they are what a column sync looks like:
type 77h (`0077FEE4`) resolves the same id and calls `0077BD70`, and type **78h** (`0077FF03`) calls
`0070EFD0` with `[msg+20h]` and then loops over `[group+4F8h]` members, writing each record's
`+10h`/`+20h` and `+14h`/`+24h` from float arrays at `[msg+28h]` and `[msg+2Ch]` through
`0070D070`. That is the message that carries columns, and it is not the one a join sends.

#### The census: who can send a type-76h message with columns in it

Asked for explicitly, and the answer is **nobody, and the class is too small to hold them**.

* **Every site in `.text` that pushes the literal `76h`** (18, the whole section, uncapped): two are
  message builds - `0077C920` in `0077C8D0` and `0077C941`... precisely, `00779941`, which the scan
  attributes to `BSP_EntityOrderMessage_Construct` but which lies **past that function's `RET 0Ch`
  at `00779939` and four `INT3`**. Ghidra has no function there; raw disassembly shows a separate
  out-of-line constructor at `00779940`. The other sixteen are `STL_xlen_throw`, a scene-database
  clear, an animation reader, `FUN_00A6C2A0`'s two `strchr`-style `00BF86F0` calls ('k' and 'v'),
  and ten `Unwind@` stubs.
* **`00779940`**, read from the disk bytes, is the same message as `0077C8D0` builds inline:
  `PUSH 76h` into `0075B430`, `[msg] = 00D02D30`, `[msg+4] = 1`, `[msg+18h] = [msg+1Ah] =
  [msg+1Ch] = 0`, and `[msg+20h] = [entity+174h]`, `RET 4`. **The id and nothing else.**
* **Every site that stamps the class vtable `00D02D30`** (three, uncapped): `0075A2E0` the default
  constructor, `00779963` that out-of-line one, `0077C94D` the inline one. `0075A2E0` zeroes the
  object and its **highest write is `+1Ch`**; the two senders' highest is `+20h`. The class is
  about `24h` bytes, so `+0CCh`..`+0F8h` is not in it at all.
* And the receiving end agrees: `0077FE80`'s type-76h arm reads `[msg+20h]` and nothing else.

So the columns at `+0CCh`..`+0F8h` belong to `0077FAD0`'s **authored record at `entity+0C0h`**, a
different structure reached from the scene, not to any wire message. **A runtime join leaves
`0070ED30`'s join geometry in place**, and the join arm does not "run on zeros" - the arm that
writes columns is not on the runtime path at all. The station binding can proceed.

The one bound on this census: it enumerates sites that push the literal type byte or stamp the
class vtable. A type held in a register and a message built by copying another object would both
escape it. The vtable scan is the stronger of the two, because every instance of this class must
get that pointer from somewhere.

#### `0077FAD0`, the authored-placement path, `__thiscall(entity)`, body `0077FAD0-0077FE7E`, 245 instructions; its kind-2 arm read whole

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
`0070D290` reads. `group+4FCh` is the field `0070DB20` sets to 6 for a ship leader *(cited)*; the
`== 18h` test selects the three-float form. Since `entity+0C0h` is an authored record and not a wire
message, this is the path by which a **scene** can give a formation its columns outright - which is
what the convoy caller's name suggests - and it is not on the runtime join's path at all.

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
| `00810190` | `BSP_UnitWake_AppendSample` | 303 | **read whole, this packet** (section 5b) |
| `00810630` | `BSP_UnitWake_SampleAtDistance` | 198 | **read whole, this packet** (section 5c) |
| `00811180` | `BSP_Unit_DecomposeAgainstWake` | 523 | **its search read** (section 5d); the tail unread |
| `00811150` | `BSP_Unit_WakePointAtDistance` | 12 | trivial forwarder, `+0BD0h` |

The ring is 40 samples of `18h` bytes at `wake+8h` (`entity+0BD8h`) with the head index at
`wake+3C8h` (`entity+0F98h`) *(cited, `docs/SHIP_AI_FORMATION.md`)*, confirmed by this packet's read
of `00810190`'s indexing: `EAX = [this+3C8h]`, `LEA EAX,[EAX+EAX*2]`, `LEA EAX,[this+EAX*8+8]`.

### 5b. `00810190 BSP_UnitWake_AppendSample`, read whole this packet

`__thiscall(wake /*ECX = entity+0BD0h*/)(const float world_pos[3], float heading, float yaw_rate)`,
`RET 0Ch` at `0081062C`, body `00810190-0081062E`, 303 instructions. Ring: 40 samples of `18h`
bytes at `wake+8h`, head index `wake+3C8h`, a byte flag at `wake+3CCh`, a residual accumulator at
`wake+3D0h`. The trail is **not** one sample per tick; it is a decimated polyline.

```
new = world_pos + acc                               ; acc = wake+3D0h        00810198..008101EB
h   = wake+3C8h ; head = wake+8h + 18h*h
d2  = (new.x-head.x)^2 + 0^2 + (new.z-head.z)^2                              0081020F..0081021D
if (16.0f > d2) return                              ; 00CE6454 float, 4 m    00810225..0081022F
q = 0.25 * sqrt(d2)                                 ; 00D7A348 DOUBLE        00810280..00810293
acc = (q < |acc|) ? acc * (|acc|-q)/|acc| : (0,0,0)  ; decay, or the 00F8757x triple
new = world_pos + acc                               ; recomputed             008102F3..00810326

sample[h]   = new.xyz ; +10h = 0 ; +0Ch = heading ; +14h = yaw_rate          0081034B..008103AB
sample[h-1]+0Ch = wrap_2pi(pi/2 - atan2(new.z-p2.z, new.x-p2.x))             0081043F..0081047B
       ; p2 = sample[h-2]; 00CE3830 = pi/2, 00CE3828 = 2pi, both doubles
len = |new - sample[h-1]|                           ; 0042B2F0               00810416
if (sample[h-1]+10h > len) { wake+3CCh = 1 ; sample[h-1]+10h = len }          00810484..00810493
       ; the FCOMIP at 00810480 has the STORED length in ST0 and the new one in
       ; ST1 and the JBE leaves for "stored <= new", so this is the SHRINKING leg
else if (!wake+3CCh)        { sample[h-1]+10h = len }                        0081049D..008104B1
else if (|new - p2|_2d < 55.0) {                    ; 00D09438 DOUBLE, merge 008104E2..008104F9
    sample[h-1] = the new sample ; p2+10h = that distance ; 00810160 copies
    sample[h] -> sample[h+1] ; wake+3C8h = h-1       ; the head moves BACK    008104FB..0081056B
} else { sample[h-1]+10h = len ; wake+3CCh = 0 }                             00810573..0081057E

if (sample[h-1]+10h <= 50.0) return                 ; 00CE3938 DOUBLE        00810585..00810593
if (wake+3CCh) return                               ; a shrinking leg never advances  00810599
h = (h >= 27h) ? 0 : h+1 ; wake+3C8h = h            ; ADVANCE, wrap at 40     008105A6..008105BE
sample[h] = new.xyz ; +10h = 0 ; +0Ch = heading ; +14h = yaw_rate             008105C7..00810620
```

So: **nothing happens until the ship has moved 4 m** from the head sample (`d2 > 16.0f`); the head
sample is then rewritten in place and the previous sample's heading is back-filled from the
direction to it; and **the head only advances to a new slot once the current leg exceeds 50 m**.
Forty slots of up to 50 m is a trail of about 2 km, which is the length a follower's station can be
measured back along. Three of the four thresholds are **doubles** (`0.25`, `55.0`, `50.0`) and all
three read `0.0` at float width - the same trap `docs/AI_COMMAND_TICK.md` records at `00D21530`.

The `[ESP+n]` slots above were traced across the two `PUSH EBP` / `POP EBP` shifts at `00810332` and
`0081058B`, and `[ESP+0x4C]` is the incoming `delta` argument's own slot reused as scratch after
`EBX` has it. `RET 0Ch` gives the three arguments.

Its one caller is `00825F20 BSP_UnitInstance_UpdateShipMotion`, at `00826CEE`, with the tick's yaw
rate as the third argument (*cited*, `src/game_hosts_ship_ai.cpp:5156`, from
`docs/SHIP_AI_RUDDER_HOP.md`). **The host already ticks that motion virtual**
(`motion_step_00825f20`), so the append has a hook point that exists; what is unimplemented there is
the motion tail.

`00810160`, the 24-byte sample copy used by the merge arm, is unread; so is what writes the residual
accumulator `wake+3D0h`, which this routine only ever decays - a producer must exist elsewhere,
and until it is found the accumulator should be modelled as zero and said to be a hole, not a proof.

**Two offset bases, one layout.** The earlier ledger evidence on `00810190` (packet
`cc_ai_rudder_hop`) names the sample fields `+0/+4/+8`, `+14h` heading, `+18h` length, `+1Ch` yaw,
because it measures from `wake + 18h*h`, the `ESI + EDX*8` form the instructions use. This document
and `docs/SHIP_AI_FORMATION.md` measure from the sample itself, `wake + 8h + 18h*h`, giving `+0Ch`,
`+10h`, `+14h`. The two differ by the ring's own `+8h` base and agree field for field; neither is a
correction of the other. It also read the first argument as the world position, which is why this
document calls it `world_pos` and not a delta: the ring stores `new` directly as a sample position,
so an argument that were a per-tick delta would store a near-zero vector.

### 5c. `00810630 BSP_UnitWake_SampleAtDistance`, read whole this packet

`__thiscall(wake /*ECX*/)(float along, float out_pos[3], float out_dir[3], float* out_yaw)`,
`RET 10h` at `008108F2`, body `00810630-008108F4`, 198 instructions. This is the routine
`00811150` forwards to (`+0BD0h`), and therefore the one `0070D290` runs for every follower every
tick. `COMISS`/`JC` at `0081063C` splits it in two.

**`along <= 0`** (`00810645-00810718`), the head-sample branch:

```
h = wake+3C8h
a = wrap_2pi(pi/2 - sample[h]+0Ch)          ; 00CE3830 pi/2, 00CE3828 2pi
out_dir = (cos a, 0, sin a)                                          008106A0..008106B3
out_pos = sample[h].xyz - along * (cos a, 0.0, sin a)                008106DD..00810712
                                            ; the middle term via double 00D7A258 = 0.0
out_yaw is NOT written
```

**`along > 0`** (`0081071B-008108F2`), the walk:

```
i = h ; n = 0
while (along > sample[i]+10h && n < 40):    ; the leg to the next sample     00810740..00810782
    along -= sample[i]+10h ; i -= 1 ; if (i < 0) i = 27h ; n += 1
t = (n == 40) ? 1.0f (00D7A24C) : along / sample[i]+10h                00810786..008107A0
j = (i == 27h) ? 0 : i+1                    ; the sample one step toward the head
out_pos = t * sample[i].xyz + (1-t) * sample[j].xyz                    008107D7..00810842
d = 00438B10(sample[i]+0Ch, sample[j]+0Ch)  ; the wrapped angle difference   00810854
a = wrap_2pi(pi/2 - 00438AA0(sample[j]+0Ch, d * (1-t)))                0081087B..0081089A
out_dir = (cos a, 0, sin a)                                            008108A2..008108D2
out_yaw = sample[i]+14h * t + sample[j]+14h * (1-t)                    008108D7..008108ED
```

`00438B10 subtract_wrapped_angle` is the routine `include/bsp/ship_ai_follow_land.hpp` already
declares for the `land` step, so the host has it.

Two consequences for the binding:

* **`out_yaw` is genuinely left unwritten on the `along <= 0` branch.** This packet confirms
  `docs/SHIP_AI_FORMATION.md`'s note independently: `[ESP+4Ch]`, the `out_yaw` argument slot, is
  reused as the angle scratch at `00810658`. `009DF2D0` reads that yaw at `009DF3F9`, so a follower
  whose station is abreast of or ahead of the leader reads a stale stack value in the image. The
  reconstruction must carry a validity flag and substitute 0, which is what `src/ship_ai_formation.cpp`
  already does - **not** reproduce the uninitialised read.
* **The walk terminates after at most 40 samples and clamps `t` to 1.0.** A station further back
  than the whole trail therefore lands on the oldest sample, not off the end. With 50 m legs that
  is about 2 km of usable station depth.

### 5d. `00811180 BSP_Unit_DecomposeAgainstWake`, its search read

`__thiscall(entity /*ECX*/)(const float point[3], ...)`, body `00811180-00811812`, 523
instructions. This is the one `0070ED30` runs once per join to turn a member's current position
into column 0. Unlike the other two it takes the **entity** as `this`, so its offsets are `0BD8h`
for the ring and `0F98h` for the head - `0BD0h` higher than `00810630`'s.

Its first 72 instructions, read this packet, settle the one thing a reconstruction has to get
right, which is what "nearest" means:

```
best = 3.4028235e38                                 ; 00D7A248, FLT_MAX at float width  00811187
i = [entity+0F98h] + 27h ; nearest = -1                                          008111BB..008111D5
per sample, walking BACK from the head through all 40 slots (IDIV by 28h):
    d = point - sample.xyz                          ; all three components       00811208..00811222
    d2 = d.x*d.x + d.y*d.y + d.z*d.z                                             00811226..00811242
    if (d2 < best) { best = d2 ; nearest = this slot }                           0081124E..00811260
```

**The search is over sample POINTS, by full 3D squared distance, not over the segments between
them**, and the loop is unrolled, which is most of why the body is 523 instructions.

Its last 68 instructions were read too, which pins the ABI and the two answers:

```
00811731..00811760  sign = (v > 0) ? -1 : (v > w ? +1 : 0)      ; an integer in [ESP+4]
00811768..008117C2  a cross product, then its squared length     ; [ESP+18h]
008117C6..008117EB  dist = (sq > 1e-10) ? sqrt(sq) : 0        ; 00CE3820, a DOUBLE; as a float
                                                              ; it reads -7.59e15, a fifth of these
008117F1  FILD [ESP+4]                                          ; the sign
008117F5  EAX = arg1 ; ECX = arg2 ; XMM0 = [ESP+38h]            ; arg0's slot, reused as scratch
00811803  *arg1 = sign * dist                                   ; the SIGNED perpendicular distance
00811809  *arg2 = XMM0                                          ; the accumulated arc length
00811810  RET 0Ch                                               ; three arguments
```

`RET 0Ch` makes it `__thiscall(entity)(const float point[3], float* out_across, float* out_along)`,
and the two stores confirm `docs/SHIP_AI_FORMATION.md`'s contract from the other end: a signed
perpendicular distance and an arc length, which `0070ED30` stores as `record+10h` and `record+20h`.
And the middle is now read in outline, which is what a reconstruction needs:

* **The search is fully unrolled**, one ~29-instruction block per sample: `EAX = ECX - k`,
  `CDQ`, `IDIV EBP` with `EBP = 40`, then the three-component difference, the squared length, a
  compare against the running best and `LEA EDI,[EBX - k]` on a win. `ECX` starts at
  `head + 27h` and `EBX` at 2, so `EDI` ends up holding **how many slots back from the head the
  nearest sample lies**.
* **The arc length is a sum of the stored legs, not a recomputed distance.** Two accumulators, at
  `[ESP+48h]` and `[ESP+44h]`, are filled by two loops - `0081167F-008116E5`, unrolled four deep
  with its back-edge at `008116E5`, and `00811700-00811721` - and each term is
  `FADD float ptr [ESI + ((i mod 40) + 7Fh)*18h]`, which is `entity + 0BD8h + 18h*(i mod 40) + 10h`:
  the sample's own `+10h` arc length. The `CMP EAX,EDI / JG` at `008116F1` picks which of the two
  runs, against a second index held at `[ESP+28h]`.
* `[ESP+48h]` is what the tail returns as `*out_along`: after the four `POP`s at `008116F3`,
  `0081172A`, `0081172D` and `00811730` the stack has moved 16 bytes, so the tail's
  `MOVSS XMM0,[ESP+38h]` at `008117FD` reads that same accumulator.

**What is still unread** is the sign convention and the exact interpolation between the nearest
sample and its neighbour that the cross product at `00811768` is taken over. A reconstruction can
be written against the contract without it, but the sign of `record+10h` cannot be claimed until it
is, and column 0's across offset is that sign.

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

## 7. What was measured

The wake is bound; nothing consumes it yet. Two runs on the build at this document's commit,
3000 mission frames each, 0.05 s a frame.

### USN01, `local/wake_usn01_fixed.log`

```
summary unit wake ships=14 appends=1346 advances=122 merges=0
summary mission world units=62 walked=186000 updated=186000 motion_ticks=42000
                      simulated=150.00 s controlled=Airfield2 moved=0.00 total_path=5600.63
```

**The number that checks the transcription is `advances`.** The head advances once per 50 m of
travel, so 5600.63 m of fleet path predicts about 112 advances plus one per ship for the opening
leg out of the zeroed ring. Measured: 122. The first run of this packet, with the `00810480`
comparison inverted, reported **4**, and that is how the inversion was caught - the flag stayed set
for every ship steaming straight and `00810599` refused to advance. `appends=1346` is unchanged
between the two runs, as it must be: the 4 m gate is upstream of the flag.

`merges=0`: no leg shrank on USN01, which is what a fleet on straight courses should give.

### USN04, `local/wake_usn04.log`

```
summary unit wake ships=18 appends=849 advances=83 merges=0 longest_trail=1950.10 m
summary mission world units=57 walked=63000 updated=63000 motion_ticks=54000
                      simulated=150.00 s controlled=Lexington-class01 moved=100.51 total_path=3511.74
```

3511.74 m of fleet path predicts about 70 advances plus one per moving ship; measured 83. The
second check is the **ceiling**: forty slots of legs that are each the first measurement past 50 m
cap a trail at roughly 2 km, and the deepest trail on USN04 is 1950.10 m - the ring has very nearly
lapped itself, which is what the 50 m advance and the 40 slots together predict and what makes a
follower's station depth finite. On USN01 no ship travelled far enough to get near it.

`merges=0` on both missions. The merge arm needs a leg that shrinks, and nothing on either mission
turned back on its own track hard enough. **That arm is transcribed but unexercised**, and it should
not be called measured until something runs it.

### The same-binary null

`local/wake_usn01_noappend.log`, the control: the same source with the append call switched off by a
local `constexpr bool`, built, run, and the switch then removed again (a `constexpr true` branch
compiles to the same code as the unconditional block, so the committed build is the measured one).
Against `local/wake_usn01_fixed.log`, `local/wake_null_calldiff.txt` is the whole host-call and
native-call table diffed in both directions:

```
before rows=1115 after rows=1116 changed=1
NEW   UnitWake::append_sample   00810190   concrete calls=42000
```

**One row, in one direction, and nothing else changed.** The control's own wake line reads
`ships=0 appends=0 advances=0 merges=0`, and `units=62 walked=186000 updated=186000
motion_ticks=42000 total_path=5600.63`, `torpedo_drop drops=5 refusals=0 water_entry_breakups=0`,
`swims_started=5`, `torpedo_ranges_derived=35` and `bullet_ranges_derived=234` are identical across
the pair. 42000 is the run's `motion_ticks` exactly, so the append runs once per ship motion tick
and on no other schedule.

One caveat stated rather than hidden: the control binary also carries the `written` counter added
for the trail-length report, which the measured run predates. It is inert with the append off - the
counter is only touched inside the append, and the report skips any ship with `appends == 0` - and
the one-row diff is itself the evidence that nothing else moved.

### The earlier cross-binary diff, and why it is not the null

`local/wake_calldiff.txt`, against the predecessor's own USN01 control:

```
NEW   UnitWake::append_sample   00810190   concrete calls=42000
```

42000 is exactly the run's `motion_ticks`, so the append runs once per ship motion tick and on no
other schedule. **It is the only row attributable to this packet.** The other 34 changed rows and
the two other NEW rows (`Projectile::blast_radial_damage_0084bad0`,
`ShipHit::part_damage_004705c0`) are `main`'s, not this packet's: the control was taken on a base at
main `3a691e884`, this build merges main `b4aa9e241`, and `6ba5797b3 Ungate the impact burst` and
`8108007c4 Torpedo warhead: the damage is the Blast` sit in that range. So this diff is **not** a
same-binary before/after and is not offered as one.

It is kept because it is the comparison against the predecessor's own control, and because the
motion totals match it to the last centimetre anyway - `units=62`, `motion_ticks=42000`,
`total_path=5600.63`, `moved=0.00`, `torpedo_drop drops=5 refusals=0 water_entry_breakups=0`,
`swims_started=5`, and the whole AI coordinator line including `formation_requests=306`. The null
above is what actually settles the question.

## 8. Uncertainties, and what is not read

* `00810630` and `00811180` whole, and `00810160`. `00810190` is now read whole (section 5b); what
  writes its residual accumulator `wake+3D0h` is not, and nothing here may assume it stays zero.
* `00F87574/78/7Ch` read 0.0 at load because they are past `.data`'s raw size (loader zero-fill).
  Whether anything writes them at runtime is **unchecked**; a literal-address xref negative would
  not settle it (`docs/` records block-copy writers that xref only a base).
* ~~The payload fields `0CCh`..`0F8h` that `0077FAD0`'s join arm copies into the member record are
  not written by `0077C8D0`.~~ **Settled, same packet**: `0077FAD0` is not the runtime receiver.
  `0077FE80`'s type-76h arm resolves the id and calls `0077F940`, nothing more, so a runtime join
  keeps `0070ED30`'s columns; the message that carries columns is type 78h. See the correction in
  section 3. The station binding is no longer blocked on this.
* `00465080` (the command argument pair the follow re-issue builds) and `00905300` (the `[entity+1ACh] <= 7`
  call in `0077C8D0`) are unread.
* `009FE080 IsGroupableCombatant` and `009FFEB0` are unread; they gate the leader order and the
  kind-24 arm respectively.
* Kind `18h` in `00A10DC0` is not the plane base kind (`0Fh`). What class it is was not established.
* `bsp.py ghidra callers` under-reports (it returned 4 of 9 once tonight), so the nine callers of
  `00A10DC0` in section 1 are a floor, not a census. No claim here rests on the count.
