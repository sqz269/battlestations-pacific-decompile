# The `moveonpath` path cursor: how a ship walks an authored path

Addresses: `009E59C0`, `0071BFF0`, `007ADC30`, `007ADC60`, `007ADCC0`, `007ADD70`, `0071C1B0`,
`0071F600`, `007B1D30`, `007B1C50`, `007B11F0`, `007B2250`, `007B22A0`, `00836BF0`,
`00D05318` and `00D0534C` (the two path-source vtables)

Packet `cc8_ship_moveonpath`, worktree `agent/cc8-ship-moveonpath`. Ghidra was read-only.
Every name is a hypothesis, not a recovered symbol. Read from the **listing**.

## 1. The two constant spaces, and why they are not guesses

This installation's `scripts/global/luamw_init.lua` lines 224-232 declares both:

```
PATH_FM_SIMPLE   = 1   -- runs to the end, then stops
PATH_FM_PINGPONG = 2   -- back and forth until it gets bored (it never does)
PATH_FM_CIRCLE   = 3   -- round and round, for a long time

PATH_SM_JOIN            = 5   -- start at the nearest point
PATH_SM_JOIN_FORWARD    = 5   -- same as the previous
PATH_SM_BEGIN           = 6   -- start at the head of the path
PATH_SM_JOIN_RANDOM_DIR = 7   -- start at the nearest point, either direction
PATH_SM_JOIN_BACKWARDS  = 8   -- start at the nearest point, reversed
```

`007ADC60` and `007ADCC0` branch on exactly 1, 2 and 3 and on nothing else; `007B1C50`
branches on exactly 5, 6, 7 and 8 and on nothing else, and its arms are "index 0" for 6,
"direction 0" for 8 and a random draw for 7. The binding of name to number is therefore
established by the two sides agreeing, not assumed. `docs/LUA_BINDING_MOVE_ON_PATH.md` left
argument 3 as "what the value selects was not read": it is the **start mode**.

USN04's eight `NavigatorMoveOnPath` sites all pass three arguments, so every one of the 98
calls is follow mode `PATH_FM_CIRCLE` = **3** with the `008A3734` default start mode **5**,
`PATH_SM_JOIN`. A circling carrier therefore has no final leg at all - see section 3.

## 2. Where the cursor lives

`0071BFF0`, `__thiscall(director)(int index)`, `RET 4`, body `0071BFF0-0071C00F`, read whole:

```
0071BFF4  CMP EAX,0x9                       ; index bound
0071BFF9  MOV EAX,[ECX + EAX*4 + 0x1a4]     ; the slot array
0071C004  MOV EAX,[ECX + 0x1a4]             ; slot 0 when that one is null
0071C00A  ADD EAX,0x10                      ; + 10h
```

So every routine that takes "the slot" takes the sub-object at **command+10h**, and the
fields the routines below read are:

| offset | through the cursor | what |
| --- | --- | --- |
| command+08h | - | the **follow mode**, `0071C1D2` |
| command+0Ch | - | the **start mode**, `0071C1D8` |
| command+10h | cursor+0h | the cursor's own vtable |
| command+14h | cursor+4h | the path source object |
| command+18h | cursor+8h | the current point index |
| command+1Ch | cursor+0Ch | the follow mode the cursor runs on |
| command+20h | cursor+10h | the direction byte, 1 forward and 0 backwards |

**Correction to `docs/LUA_BINDING_MOVE_ON_PATH.md` section 3.** That doc leaves open which
slot `0071C1B0` picks relative to the queue of step 3, and calls the target "the slot's path
object". The arithmetic settles it: the scan is over `director+54h + i*1Ch` for the first
`i` whose dword is zero, and the store is at `director+1A0h + i*4`, which is
`director+1A4h + (i-1)*4` - one element **below** `0071BFF0`'s own base. So the slot written
is the last occupied one, i.e. the command `vtable[60h]` has just queued, and `i` can only
be 0 if nothing is queued at all. And the object written is the command/slot object, not the
0Ch-byte path source: the source has no `+0Ch`.

## 3. The four routines that read the cursor

`007ADC30 BSP_EntityCommand_SlotHasNoLegs`, body `007ADC30-007ADC50`: no path object, or a
point count that is not positive. Already projected in `src/ship_ai_goal_vector.cpp`.
`docs/SHIP_AI_STATE_STEPS.md` called it `contract: unread`; it is read.

`007ADC60 BSP_EntityCommand_IsOnFinalLeg`, body `007ADC60-007ADCBD`, read whole:

```
007ADC63  no path                                   -> true
007ADC7A  path->vtable[0Ch]() <= 0                  -> true
007ADC87  mode 1: forward and index == count-1      -> true      (007ADCA9)
                  backwards and index == 0          -> true      (007ADCB4)
007ADC8F  mode 2 or 3                               -> false, always
          otherwise                                 -> false
```

The `(mode-2) <= 1 unsigned` at `007ADC8F` is the compiler's two-value test, and both its
arms fall on `XOR AL,AL`. **A `PATH_FM_PINGPONG` or `PATH_FM_CIRCLE` command never reports a
final leg**, which is exactly why the state step's `finished` arm is unreachable for USN04's
carriers and why they are meant to circle for the whole mission.

`007ADCC0`, `int __thiscall(cursor)()`, body `007ADCC0-007ADD4A`, read whole. Answers the
index to move to and, for `PATH_FM_PINGPONG` only, flips the direction byte in place. It
does **not** store the index.

| mode | at the far end | in the middle |
| --- | --- | --- |
| 1 SIMPLE | the caller's guard stops it first | `007ADD43` index+1, `007ADD06` index-1 |
| 2 PINGPONG | index 0 -> direction 1 and answer 1 (`007ADD17`); index count-1 -> direction 0 and answer index-1 (`007ADD34`) | the SIMPLE step |
| 3 CIRCLE | forward: count-1 wraps to 0 (`007ADCEF`); backwards: 0 wraps to count-1 (`007ADCFA`) | the SIMPLE step |
| other | - | the index unchanged (`007ADCD7`) |

`007ADD70`, `__thiscall(cursor)(const float* pos, float radius, int, float, float, int)`,
`RET 18h` (`007ADFFB`), body `007ADD70-007AE0xx`, 252 instructions. `SUB ESP,0x38` plus four
pushes puts the sixth argument at `[ESP+54h]`, which is how the argument count is fixed. It
squares `[ESP+50h]` at `007ADDE2`, takes the cursor's current point through
`cursor->vtable[4h](&buf, cursor->index)` at `007ADE0D` and squares the planar delta at
`007ADE4D-007ADE55`, so the arrival test is a squared comparison with no root.

Its advance loop, `007ADF90-007ADFF0`, read whole:

```
007ADFAC  mode != 1                       -> advance
007ADFC5  mode 1 forward, index == count-1 -> 007ADFFE, no advance
007ADFD0  mode 1 backwards, index == 0     -> 007ADFFE, no advance
007ADFD6  EDI = [ESI] + 8                  ; &cursor->vtable[2]
007ADFDF  CALL 007ADCC0                    ; next
007ADFE9  CALL [EDI]                       ; cursor->vtable[8h](next), the store
```

`BL` starts at 1 and `007ADFDA` clears it before every advance, and the exit at `007ADFF2`
answers 0. **So `007ADD70` answers true only when no advance was possible**, which is only
the terminal point of a `PATH_FM_SIMPLE` path. `docs/COMMAND_EXECUTION.md` line 177 reads
"a true answer from `007ADD70` on the path cursor means arrival"; that is right, and this is
the mechanism.

The lookahead that decides *how many* legs to skip (`EBP`, set from the remaining-leg
arithmetic at `007ADD88-007ADDD3` and refined by the `atan2` block `007ADE5D-007ADF80`) was
not transcribed: `contract: partial`.

## 4. The path build, `0071F600`

`0071F600`'s `moveonpath` arm, `0071F6A5-0071F885`, read whole, has two sides:

| | target kind != 0, `0071F6BD` | target kind == 0, `0071F7FB` |
| --- | --- | --- |
| the path interface | `00521EA0` then `007AC9D0` on the resolved target | `slot->vtable[8h]()` |
| the source wrapper | `007B22A0`, vtable **00D0534C** | `007B2250`, vtable **00D05318** |
| the follow pair | `slot0+8h`, what `0071C1B0` wrote | a local `{1, 5}` (`0071F80C` writes the 5) |

Both converge on `007B1D30` at `0071F86F`, which forwards to `007B1C50` with the source, the
unit's world position at `unit+0FCh`, the float `unit->vtable[50h](pair)` and the pair's two
integers unpacked. `007B2250` and `007B22A0` are the same 0Ch-byte allocation - vtable,
refcount 1 at `+4h`, the path interface at `+8h` - differing only in the vtable.

`007B1C50`, `RET 14h`, body `007B1C50-007B1D2E`, read whole:

```
007B1C97  cursor+0Ch = follow mode
007B1C9A  cursor+10h = 1                       ; forward
007B1C9E  start mode 6 -> cursor+8h = 0 and return
007B1CBB  start mode 8 -> cursor+10h = 0
007B1CD1  start mode 7 -> 00BD2F10(0.0f, 1.0f) against 0.5f at 00CE3800
007B1D06  start mode 5 -> cursor+10h = 1
007B1D24  cursor+8h = path->vtable[10h](pos, f, direction)
```

`path->vtable[10h]` is `007B11F0` on both sources. Its head is read - `007B11FE` the count,
`007B120E` `path->vtable[14h](pos)` for the nearest point, `007B121B-007B123A` that point's
neighbour in the travel direction as a second candidate - and the projection test at
`007B1263` onwards that chooses between the two is **not** read. `path->vtable[14h]`
(`007B1100` on the authored-path source) is not read either.

### The two path-source vtables, as bytes

| slot | `00D05318` (kind 0) | `00D0534C` (authored path) | what the call sites show |
| --- | --- | --- | --- |
| +00h | `00BD30E0` | `00BD30E0` | deleting dtor |
| +08h | `007B0DD0` | `007B10A0` | `getPoint(out, index)`, `007B1242`/`007B1263` |
| +0Ch | `007B0E30` | `007B1030` | the point count, `007ADC78` |
| +10h | `007B11F0` | `007B11F0` | the start index, `007B1D24` |
| +14h | `007ADA60` | `007B1100` | the nearest index, `007B120E` |

## 5. The director step's arm, `00836BF0`

`00836BF0-00836D66`, read to the `007ADD70` call:

```
00836BF0  CMP EAX,0xe08f80                     ; only while the command is moveonpath
00836C01  FLD [unit+9C8h] ; FMUL qword 00CE3DE0 ; * 2.5
00836C17  CALL 0082E850 BSP_ShipClass_GetTurnRadius ; FMUL qword 00CEC160 ; * 1.2
00836C2E  CALL 00415510 BSP_Math_MinFloatByRef  ; r = min of the two
00836C7F  0071BFF0(director, 0)
00836C92  cursor->vtable[4h](&out, -1)
00836D05  CALL 007ADD70
00836D0A  a true answer -> 0071D810(2) and the `finished` message
```

The two doubles read at their own width (`tools/pe_const_read.py`): `00CE3DE0` = 2.5,
`00CEC160` = 1.2000000476837158, `00CE3800` = 0.5f, `00D7A24C` = 1.0f.

Whether `007ADD70`'s first argument is the unit's planar position was **not** established -
the stack slot it comes from was not traced back to a write. The unit's position is the only
2D position in scope at that point in the arm, and this is the reading the reconstruction
uses; it is a hypothesis, not a proof.

## 6. `009E59C0`, read whole

200 instructions, body `009E59C0-009E5C90`, `__thiscall(state)(float)`, `RET 4`; the float
is never read. `docs/SHIP_AI_STATE_STEPS.md` had the head and marked
`009E5B17-009E5C8D` "not transcribed instruction by instruction"; it is transcribed now, in
`src/ship_ai_state_steps.cpp`, and it is byte for byte the `movetopos` target-range arm and
`finished` block with `00E08F80` in place of `00E08F68`.

The head, `009E59DE-009E5A3C`, is the one shape `movetopos` does not have:

```
if (state+8h != 0 || 007ADC30(slot))          ; 009E59E2 JNZ, 009E59FF JZ
    if (007ADC60(slot)) { 009E00A0(); return }  ; 009E5A1C
    state+8h = 0                                ; 009E5A39
```

`state+8h` is raised at `009E5BC5`, inside the `finished` block, so the latch means "this
command already announced that it finished". `009E5AC0-009E5ACA` stores the float32 1.0f at
`brain+308h` on every step whose leg is **not** the final one; that field has no reader
anywhere in this process, so the reconstruction carries the store and reports it.

`cursor->vtable[4h](&out, -1)` at `009E5A59` and `00836C89` passes -1 where `007ADD70` passes
the cursor's own index. **The cursor's vtable was not located**, so "-1 means the current
index" is a hypothesis resting on those three call sites; both readings give the same point
for the current leg, which is the only value either caller uses.

## 7. What the reconstruction does

`src/ship_ai_path_cursor.cpp` projects `007ADC60`, `007ADCC0`, `007ADFAC`'s guard and
`007B1C50`. `src/ship_ai_state_steps.cpp` projects `009E59C0`.
`src/game_hosts_commands.cpp` carries slot 0's cursor on `GameDirector`, takes `0071C1B0`'s
pair, runs the `0071F600` build and moves the cursor from the director step;
`src/game_hosts_scene_contents.cpp` carries the authored `Path` entities' points into world
space with the same `00B62D10` that `007AF800` applies to the avoidance side.

Named holes, each a hole and not a proof:

1. **One leg per director step.** `007ADD70`'s loop can take several, and neither the
   remaining-leg count nor the `atan2` lookahead that sizes it is projected.
2. **The start index is the nearest point.** `007B11F0`'s refinement and `007B1100` itself
   are unread.
3. **`PATH_SM_JOIN_RANDOM_DIR` keeps forward.** The `00BD2F10` draw is not reproduced. No
   USN04 call asks for start mode 7.
4. **`brain+308h` has no reader**, so the 1.0f is stored and reported, not consumed.
5. **The route to the slot** is still this process's `0077D600`/`00816E30` seam rather than
   the director's own `vtable[60h]` `008358D0`, which
   `docs/LUA_BINDING_MOVE_ON_PATH.md` section 5 already records.

## 8. What the run measures

Three USN04 runs on the same binary apart from the change under test, `--mission-frames 3000`
at 0.05 s, `local/mop_before_usn04.log`, `local/mop_after2_usn04.log` and
`local/mop_after3_usn04.log`. The middle one also carried a `GetSelectedUnit` binding and is
kept only for the finding in section 9.

Rows that moved, before -> after (`local/calldiff.txt` has all 216):

```
ShipAiState::moveonpath_step        009e59c0  UNIMPLEMENTED 179 -> concrete 179
Navigator::path_object_set_follow_mode 0071c1b0 UNIMPLEMENTED 98 -> concrete 98
ShipAiState::set_navigation_goal    009de050  concrete 8400 -> concrete 8579  (+179)
WeaponDirector::path_build_0071f600           new, concrete 98
WeaponDirector::path_follow_00836bf0          new, concrete 900
ShipAiMoveOnPath::* (13 rows)                 new, 174..537 each
```

Nothing went down and no row was lost. The two `Navigator::avoidance_receiver_*` holes at
`0071C1E0` are untouched: they are the `5Ah` arm, not this one.

The cursor table the run now prints:

```
unit                 path             pts mode start from  at dir final advances travelled
Lexington-class01    CarrierPath1       6    3     5    2   2 fwd    no        0      0.00
Yorktown-class01     CarrierPath4       8    3     5    2   2 fwd    no        0     48.30
```

**The ships still do not drive.** Both cursors are built on the right authored path with the
right mode (3 `PATH_FM_CIRCLE`) and start mode (5 `PATH_SM_JOIN`), both join at point 2, both
correctly report no final leg, and `009E59C0` sets the navigation goal at point 2 on every one
of its 179 steps - and the arrival radius test in `00836BF0` never passes in 900 tries, because
the Lexington moves 0.00 m and the Yorktown 48.30 m while the command is current. `moved` for
the controlled Lexington is 100.51 in all three runs, unchanged to the centimetre; the fleet
`total_path` is 35103.17 -> 36963.45.

Two numbers say where the remaining gate is, and it is **not** in this packet's chain:

1. `009E59C0` runs 179 times out of 54000 AI steps and `00836BF0`'s arm 900 times out of 6000
   director steps for the two carriers. The 98 `moveonpath` commands are issued and then
   displaced: the command table's rows for both carriers read `latch -` and
   `steer 0.000 thrust 0.000`, the same as before this packet.
2. `009DE050` receives the goal and the drive publishes 54000 times, and the hull still does not
   move. That is the same gate the baseline `moveto` commands hit on these two units.

So the packet's own claim is narrow and measured: **the command now carries a real path cursor
that the state step reads and the director step would move.** Making the hull follow it is the
next packet, and it starts at the navigation/throttle chain, not here.

## 9. `GetSelectedUnit` `008AB070` is a hole with a named cause

`local/mop_after2_usn04.log` is the run with the one-line binding - the controlled unit's entity
id, `controlled_bound` standing for the null test at `008AB15C`. USN04's `Think` then failed on
every frame from frame 61:

```
commandhelpers.lua:330: attempt to index field '?' (a nil value)
  commandhelpers.lua:330  in luaGetShipsAround
  commandhelpers.lua:13294 in luaCheckMusic
  usn_19_coralus.lua:490
```

`luaCheckMusic` returns early while `GetSelectedUnit` answers nil. With a unit it reaches
`luaGetShipsAround`, whose line 330 is `pairs(recon[targetUnit.Party][allegiance])` with
`targetUnit = thisTable[target.ID]`. The slot `00928A00` builds in this process carries `ID`,
`Dead`, `Ptr` and `Class` and **no `Party`**, so `recon[nil]` is nil and the index raises. The
whole mission diverges: the AI group, planner, air-operations and auto-target families stop
running, the 98 `NavigatorMoveOnPath` calls never happen and the world ends at 45 units instead
of 57. The binding is therefore reverted and the reason is in `src/game_hosts_lua.cpp`. What it
needs is `Party` (and `Name`) on the `thisTable` slot plus a filled `recon` table, not a native
body: `008AB070` itself is read.

## Address table

| address | coverage |
| --- | --- |
| `0071BFF0` | complete, read whole |
| `007ADC30` | complete, read whole |
| `007ADC60` | complete, read whole |
| `007ADCC0` | complete, read whole |
| `007ADD70` | partial: the head, the arrival square and the advance loop; the `atan2` lookahead is not transcribed |
| `0071C1B0` | complete, read whole |
| `0071F600` | the `moveonpath` arm `0071F6A5-0071F885`, read whole |
| `007B1D30` | complete, read whole |
| `007B1C50` | complete, read whole |
| `007B11F0` | partial: the head only |
| `007B2250`, `007B22A0` | complete, read whole |
| `00836BF0` | partial: `00836BF0-00836D0C`, to the `007ADD70` answer |
| `009E59C0` | complete, read whole |
