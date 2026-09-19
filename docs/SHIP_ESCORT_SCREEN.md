# The escort screen: the placement slot, and why the station was never wrong

Packet `cc8_ship_screen`, branch `agent/cc8-ship-screen`, third and last worker on the ship
formation-follow chain (after `cc8-ship-follow` and `cc8-ship-station`). This doc records what
`008193A0` is, and it **retracts the defect the previous two packets were chasing**.

## The residual, stated first

There is **no station defect left to fix**, and the two numbers that were read as one are
misread columns. Everything below is either read from the listing or measured from a named log.
What remains genuinely open is listed in "Still open" at the bottom; the largest item is the
absolute sign of `record+10h`, which does not affect column 0 and therefore does not affect any
formation USN01 or USN04 authors.

## 1. `008193A0` is `SetWorldPosition`, and it is vtable slot `+118h`

`__thiscall void FUN_008193a0(Unit* this /*ECX*/, const float pos[3])`, `RET 4`, body
`008193A0-008196AA`, 209 instructions, 28 basic blocks, cyclomatic 17. Read whole, as pseudocode
and as the full listing.

**The slot is `+118h` in all nine vtables.** This needed care, because RTTI is stripped in this
image: the dword before a vtable's slot 0 is padding, not a complete-object-locator, so adjacent
vtables run together and a naive walk-back to "the first non-`.text` dword" merges them and
reports three different slots (`0x118`, `0x164`, `0x590`). The reliable base is the one the
constructor stores, so each candidate base was accepted only if its own address occurs as a
literal dword somewhere in `.text`. All nine then agree:

| vtable entry | base (constructor-referenced) | slot |
| --- | --- | --- |
| `00CF91C8` | `00CF90B0` | `+118h` |
| `00CFA890` | `00CFA778` | `+118h` |
| `00CFB850` | `00CFB738` | `+118h` |
| `00CFC4E8` | `00CFC3D0` | `+118h` |
| `00CFFB48` | `00CFFA30` | `+118h` |
| `00D01748` | `00D01630` | `+118h` |
| `00D09790` | `00D09678` | `+118h` |
| `00D0C098` | `00D0BF80` | `+118h` |
| `00D0C760` | `00D0C648` | `+118h` |

Independent confirmation from inside the function: at `00819674` it dispatches `[EDX+118h]` on its
own group-mates, i.e. it calls *itself* through the slot. `008193A0` has **zero `rel32` call
sites** (`tools/callsite_census.py`), so the slot is its only entry.

### The three arms

All three are gated on `notState2 = ([00E188A8]+1FE4h != 2)`, set once at `008193E7`/`008193F0`
into `BL`.

1. **`00819430`, the refusal.** If `notState2 && 007788B0 BSP_Unit_IsFormationFollower(this) &&
   |pos|^2 > 25.0`, the function returns having done nothing at all. The threshold at `00CE3880`
   is a **double** (`FLD qword ptr`, `0081941F`) and reads `25.0`, so the test is "further than
   5.0 m from the world origin". A formation follower cannot be placed at an arbitrary point
   while the global is not 2.
2. **`0081945D`, the station override.** Otherwise `this+0BCCh` is set to the float `1.25`
   (`00CF29A8`), and if the unit is still a formation follower and `notState2`, the requested
   point is *replaced* by the unit's station:

   ```
   col  = [[this+284h]+500h]                       ; the group's shape/column index
   rec  = 0070D080 BSP_UnitGroup_FindMemberRecord(group, this)
   A    = rec[10h + col*4]                         ; across, signed
   B    = rec[20h + col*4]                         ; along, back down the wake
   00810630 BSP_UnitWake_SampleAtDistance(leader+0BD0h, B, &p, &d)
   station = ( p.x - A*d.z , 0.0 , p.z + A*d.x )   ; 008194E3..0081950A
   ```

   `y` is forced to `0.0` at `008194C0` (`XORPS XMM0,XMM0` into the working copy), so a station is
   always at sea level. Note the listing corrects the decompiler here: both `rec+10h+col*4` and
   `rec+20h+col*4` are `FLD float ptr` loads (`0081946F`, `0081947D`); Ghidra types the second as
   a pointer and it is not one.
3. **`0081963E`, the group snap.** If the unit is *not* a follower (or the override was skipped)
   and `00778890 BSP_UnitInstance_IsOccupantOwner(this) && notState2`, it walks `[group+4F8h]`
   members through `0070D060 BSP_UnitGroup_MemberAt` and, for every member but itself, calls
   `member->vtable[118h](&DAT_00F87574)` followed by `member->vtable[0D8h]()`. `00F87574` lies in
   `.data` past the section's raw size, so it is loader zero-fill: the zero vector. Every member
   therefore takes arm 2 and is snapped onto its station. This is the "move the ship the player
   occupies and its screen comes with it" path.

Between the arms the common tail runs `009583C0` (the position store), `00414DB0
BSP_EntityPose_RefreshWorld` when `this+0C8h` is clear, `0092D620` on `[this+1018h]` with
`this+0CCh`, **`00818EA0`** - the same wake-ring refill `00822C20 BSP_UnitInstance_SEntityInit`
uses, which is why a placed unit's trail is never stale - and `vtable[0D8h]`. For a follower it
then sets the heading through `vtable[011Ch]`:

```
h   = pi/2 - atan2(d.z, d.x)          ; 008195C1..008195DC, pi/2 is a double at 00CE3830
if h < 0: h += 2*pi                   ; 2*pi is a double at 00CE3828
vtable[011Ch]( -0.0f - h )            ; 00D7A208 is a float (MOVSS), -0.0
```

so the follower is also turned to lie along the wake at its station.

### The `state==1` arm

At `0081950E`, if `[00E188A8]+1FE4h == 1` and `[00E188A8]+218Ch == 0`, the resolved position and
`[group+500h]` are packed by `007EDF00` and handed to `0077C7B0`, with a
`PTR_BSP_SessionMessageRoot_ScalarDelete_00CE4974` vtable installed on the frame object. That is
the networked/replay path publishing the placement as a session message rather than applying it
locally; `00780120 BSP_Session_DispatchEntityMessage` is one of the slot's dispatchers, which
closes that loop.

## 2. The join does NOT invoke the slot - the brief's hypothesis is refuted

The integrator's premise for this packet was that a `JoinFormation` at `luaStageInit` re-places
the follower on its canned station, which would make the join distance, the `FollowerMaxDist`
clamp and the decomposition all irrelevant for authored formations. **It does not.**

MSVC compiles a virtual call at this displacement as `MOV reg,[base+118h]` followed by `CALL reg`;
the `CALL dword ptr [base+118h]` form does not occur (a scan of `ff ?? 18 01 00 00` returns three
hits, none of them a call at that slot). Censusing `8b <modrm> 18 01 00 00` over all 24 modrm
encodings with the vtable pointer in `EAX`, `ECX` or `EDX` gives the dispatchers:

`0048A010`, `00496BD0`, `00497BB0`, `004A0D20`, `0053D230`, `0062C2C0`, `006E6750
BSP_Projectile_PlaceStepPose`, `00780120 BSP_Session_DispatchEntityMessage`, `007EC5A0`,
`008193A0` (itself), `0084CB30`, `008A9F90`, `008AA260`, `008AA560`, `00891FB0`, `008F02B0`,
`008F02E0`, `008F0CE0`, **`00944680 BSP_LuaBinding_Spawn`**, `00993220`, `00A49000`, `00A49220`,
`00B1C4A0`.

`0077F940`, `0077FAD0`, `0088FFD0`, `0070ED30` and `0070EFD0` are **all absent**. Joining a
formation does not place the follower; only spawning it, teleporting it, or moving the ship the
player occupies does.

## 3. What `00811180`'s tail actually answers, and why the station is faithful

`__thiscall 00811180 BSP_Unit_DecomposeAgainstWake(Unit* this, const float point[3], float*
out_across, float* out_along)`, `RET 0Ch`. The argument roles are fixed by `0070ED30`'s use of
them: `0070EEAB` copies the third argument's slot into `record+20h` (along) and `0070EEB6` the
second into `record+10h` (across), which is what `docs/SHIP_AI_FORMATION.md` already records.
Section 5d of `docs/SHIP_UNIT_GROUP_FOLLOW.md` read the search and left the tail unread; this is
the tail.

* **`along` is the arc length back to the nearest sample, and there is no extrapolation.** The
  accumulator loop at `00811700-00811721` adds each visited sample's own stored leg
  (`FLD float ptr [ESI+EDX*8]`) into `[ESP+44h]`, `EDI` times. The final `MOVSS XMM0,[ESP+38h]`
  at `008117FD` reads that same slot - three `POP`s (`EDI`, `ESI`, `EBX` at `0081172A`,
  `0081172D`, `00811730`) sit between the two, and `44h - 0Ch = 38h`. **So the answer to the
  integrator's question is: the image does not extrapolate along the last leg.** When the nearest
  sample is the head the loop does not run and `along` is exactly `0`, and when it is the oldest
  sample `along` is the whole trail. The wake frame has no "ahead", by construction.
* **`across` is a signed perpendicular distance.** `00811768-008117A2` builds a three-component
  cross product into `[ESP+28h/2Ch/30h]`, `008117B2-008117C0` takes its squared length, and
  `008117C6` compares that with a double epsilon at `00CE3820`: above it, `00BF7030` (the CRT
  `sqrt`) gives the magnitude; at or below it the magnitude is `0.0`. `008117F1` then loads the
  integer at `[ESP+4h]` - `-1`, `0` or `+1`, set at `00811743`/`00811756`/`00811760` from the
  sign of a single x87 difference at `00811731` - and `00811803` multiplies the two. So
  `across = sign * |(point - base) x dir|`, which for a unit `dir` is the perpendicular distance
  from the trail's line, signed by side.

`src/ship_ai_wake_trail.cpp`'s `ship_ai_wake_decompose_00811180` computes exactly this, expressed
as a signed dot with `0070D290`'s own left normal instead of a cross-product magnitude times a
sign. The two are the same quantity up to (a) the absolute sign convention, which cancels for
column 0 because `00811180` and `0070D290` are used as inverses of each other, and (b) the image
taking a 3D cross where the host takes the XZ component, which differ only by the query point's
height above the base sample. **The host's decomposition is faithful.**

The x87 walk (`tools/x87trace.py trace 00811180 00811812`) puts the depth at `00811726` at **5**,
so five values are live when the sign block runs, and the two multiplications at `00811728` and
`0081172E` write into `ST3` and `ST5` for the cross product that follows rather than into the
sign. Naming those five requires walking back through the 40-times-unrolled search; it is not
done here and it is not load-bearing for column 0 (see "Still open").

## 4. Two retractions the chain must carry forward

* **`err_max` is the first step's error, not evidence of divergence.**
  `src/game_hosts_ship_ai.cpp:4179` seeds `follow_station_error_max` at `0.0f` and takes a
  running maximum from the first follow step onward, so its value is dominated by the initial
  gap. Rebuilding the station from the logged join line for `local/follow_merge_usn01.log`
  (base `(5999.7, 7999.8)`, `across 3152.01`, `along 0`, left normal `(-d.z, d.x) =
  (0.61569, -0.78799)`) puts it at `(7940.4, 5516.0)`. `Dunlap` starts at `(6000, -2700)`, which
  is **8442.1 m** from it, against a logged `err_max` of **8431.22**; `SaltLakeCity` starts
  **9418.1 m** away against a logged **9375.16**. Both match to a few tens of metres. The escorts
  therefore **converge** - 8431 -> 3826 and 9375 -> 5152 over 150 s - rather than diverge.
* **The `along=0.00` joins are correct, not lossy.** Seven of the thirteen joins report it, and
  the three that do so from only ~565 m (`Ralph`, `McCall`, `Blue`) settle the reading: solving
  `across = (p.x-b.x)*(-d.z) + (p.z-b.z)*d.x` from `Blue`'s `+561.47` and `Ralph`'s `+68.92`
  gives `d = (-0.78799, -0.61569)` with `|d| = 1.000001`, and both `Ralph` and `McCall` project
  68.9 m **forward** of `Enterprise`'s head. `along = 0` is the right answer for a point ahead of
  the trail, and `across(McCall) = -561.47` is exactly `-across(Blue)`, so the host's values are
  self-consistent. An earlier note of mine claiming they were not antisymmetric compared
  `Ralph (-400,-400)` with `Blue (+400,-400)`, which are not negatives of each other; that claim
  is withdrawn.

`SaltLakeCity` and `Dunlap` are 10700 m from `Enterprise` when the script merges them into its
formation. `0070ED30` clamps the offset to `FollowerMaxDist` 4000 (`settings+424h`), and that
clamped point lies 2462 m ahead of and 3152 m abeam the carrier, so `along` clamps to 0 and the
station is 3152 m abeam. **On the reading above that is what the image does too.** It is a wide
station, but `FollowerMaxDist` is 4000 and the script asks for a merge from 10.7 km; the game
does not reposition the ships, it gives them a station within the follower radius and lets them
steam to it.

`group+500h` is 0 for every USN01 formation: `docs/SHIP_AI_FORMATION.md` records that its only
two writers are the constructor (zero) and the Lua `shape` field, and **USN01 never calls
`SetFormationShape`** - measured, no such row appears in `local/follow_merge_usn01.log`'s call
table. So column 0, the measured-and-decomposed offset, is the live column throughout.

## 5. Prediction, recorded before the run

The analysis above changes no constant and no formula on the station path, so the USN01 follow
numbers should be **unmoved** from `local/follow_merge_usn01.log` except where this branch's
merge of `main@3b6277359` (the dive-bomb work) moves something unrelated.

| quantity | control (`follow_merge_usn01.log`) | predicted | why |
| --- | --- | --- | --- |
| `Dunlap` steps / `err_final` / `err_max` | 600 / 3826.31 / 8431.22 | identical | nothing on the follow path changed |
| `SaltLakeCity` steps / `err_final` / `err_max` | 600 / 5151.99 / 9375.16 | identical | as above |
| `total_path` | 10852.37 | identical | as above |
| `summary unit formation` | `groups=3 joins=13 creates=3 rejoins=0 clamped=3 columns_unmeasurable=0` | identical | as above |
| `summary ship follow steppers` | 2 | 2 | as above |
| ships in `stop` / `cruise` / `follow` | per control | identical | as above |

A miss on any row is a finding about `main`'s merge, not about this packet.

## Still open

* **The absolute sign of `record+10h`.** `00811180`'s sign comes from `sign(ST1 - ST0)` at
  `00811731` with five live x87 values; naming them needs a backward walk through the unrolled
  search. It cancels for column 0 and only becomes load-bearing for the canned LINE / COLUMN /
  DIAMOND columns 1-3, which nothing in USN01 or USN04 selects.
* **`008193A0`'s `[00E188A8]+1FE4h` values.** `2` disables all three arms' formation behaviour and
  `1` routes the placement through a session message; what the states *are* is not read here.
* **`this+0BCCh = 1.25f`.** Written on every placement, immediately below the wake object at
  `+0BD0h`. Its reader is not identified.
