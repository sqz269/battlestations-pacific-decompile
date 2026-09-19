# The escort screen: the placement slot, and why the station was never wrong

Packet `cc8_ship_screen`, branch `agent/cc8-ship-screen`, third and last worker on the ship
formation-follow chain (after `cc8-ship-follow` and `cc8-ship-station`). This doc records what
`008193A0` is, and it **retracts the defect the previous two packets were chasing**.

## Read this first: `err_max` is not a defect, and three of us read it as one

`err_max` is the **first follow step's error**, not evidence of divergence.
`src/game_hosts_ship_ai.cpp:4179` seeds `follow_station_error_max` at `0.0f` and takes a running
maximum from the first step onward, so its value is dominated by the initial gap and can only be
compared with the *starting* distance, never read as growth. Rebuilding the station from the
logged join line (base `(5999.7, 7999.8)`, `across 3152.01`, `along 0`, left normal
`(-d.z, d.x) = (0.61569, -0.78799)`) puts it at `(7940.4, 5516.0)`; `Dunlap` starts at
`(6000, -2700)`, **8442.1 m** from it, against a logged `err_max` of **8431.22**, and
`SaltLakeCity` starts **9418.1 m** away against **9375.16**. Both match to a few tens of metres.

So the escorts **converge** - 8431 -> 3826 and 9375 -> 5152 over 150 s - rather than diverge, and
"`err_max` 8431 with `err_final` 3826" describes a follower that closed 4.6 km, not one steaming
for the world origin. Two predecessor packets and the integrator all read that column as a defect.
It is not one, and the station it converges on is the image's own (section 3).

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

## 6. The landing window

All runs on `agent/cc8-ship-screen` with `main@3b6277359` merged, parameters
`--frames 3200 --press-start-frame 30 --menu-select <mission> --mission-frames 3000
--mission-frame-seconds 0.05`, each ending in `native renderer final COM release: device=0 api=0`.

### 6.1 USN01, `local/screen_usn01.log`

The prediction table in section 5 came back **bit-identical on every row**: `Dunlap` 600 /
3826.31 / 8431.22, `SaltLakeCity` 600 / 5151.99 / 9375.16, `total_path` 10852.37,
`groups=3 joins=13 creates=3 rejoins=0 clamped=3 columns_unmeasurable=0`, `steppers=2`, and all
thirteen `formation column:` lines the same to the last digit. Nothing on the follow path moved
when `main` came in, and nothing in this packet moved it either.

Membership, per formation, with the moving group reported separately:

| group | leader | leader moves? | members | join offset (across, along) | err_final | err_max |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | `Convoy1` | no (`stop`) | Convoy2..6 | (200, 450) (-300, 650) (350, 750) (0, 950) (-300, 850) | n/a, no follow step | n/a |
| 1 | `Northampton` | no (`stop`) | - (emptied by the merge) | - | - | - |
| 2 | **`Enterprise`** | **yes** | Ralph (68.92, 0) McCall (-561.47, 0) Blue (561.47, 0) Northampton (3216.82, 0) SaltLakeCity (3152.01, 0) Dunlap (3152.01, 0) | as listed | Dunlap 3826.31, SaltLakeCity 5151.99 | 8431.22 / 9375.16 |

Only `Dunlap` and `SaltLakeCity` run the follow step (`steppers=2`); the rest of group 2 is held by
other states. `err_max` is the **first** step's error (section 4), so both escorts close about
4.6 km and 4.2 km of an initial 8.4 km and 9.4 km gap in 150 s.

### 6.2 The join accounting, and why `calls=13` is not `calls=10`

There is **no counting hole**. Three counters count three populations:

* `summary mission script bindings ... formations=10/13` - thirteen script `JoinFormation` calls,
  thirteen `Formation::command_is_available` asks (`0077C8FE`), **ten pass**. The three refusals
  are refused by `same_entity_or_group_00779820`, the arm that stops a follower already in the
  leader's group; the log carries the per-unit evidence as
  `ai diag follow <ship> -> Enterprise available=0 (ship 1/1, kind2=1, party 0/0, alive 1/1)`,
  every other fact passing.
* `Formation::route_join_message calls=10` - the ten that passed.
* `summary unit formation joins=13` - real joins from **three** sources: the 10 routed, **1 from
  the AI path** (`summary mission ai follow ... joins=1`), and **2 brought by `0077F940`'s merge
  arm** (`src/game_hosts_units.cpp:9283`), which never passes through `route_join_message`.
  10 + 1 + 2 = 13.

`rejoins=0` is consistent: the script's post-merge orders are stopped one level earlier, at
`0077C8F8`, so they never reach `0077F96E`.

### 6.3 USN01's five-torpedo trace, re-recorded as the new reference

Format follows `docs/TORPEDO_AFTER_THE_DROP.md` section 11. **This supersedes that section's
census as the reference for a build whose escorts move.**

Ordered targets (`0071EBF0`) are unchanged: Mav1 -> `Dunlap`, Mav2 and Mav3 -> `Northampton`,
Mav4 and Mav5 -> `SaltLakeCity`. Five drops, five swims, no refusals and no water-entry breakups
(`torpedo_drop drops=5 refusals=0 water_entry_breakups=0`, `swims_started=5`).

| round | shooter | ordered target | exit | closest approach | at | run | target moved | crossing at drop |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 4 | Mav3 | Northampton | **`entity_impact`, hit `Northampton`** at (6308.0, 0.00, -3196.2) | 8.5 m | 11.40 s | 11.40 s | **0.0 m** | 2.0695 rad = **118.6 deg** |
| 5 | Mav2 | Northampton | **`entity_impact`, hit `Northampton`** at (6308.0, 0.00, -3195.6) | 9.1 m | 11.50 s | 11.50 s | **0.0 m** | 2.1178 rad = **121.3 deg** |
| 1 | Mav1 | Dunlap | `expired`, range 1852.0, travelled 1853.5 | 210.3 m | 7.40 s | 60.05 s | 141.3 m | 1.7098 rad = 98.0 deg |
| 2 | Mav4 | SaltLakeCity | `expired`, range 1852.0, travelled 1853.5 | 175.2 m | 7.80 s | 60.05 s | 129.5 m | 1.8678 rad = 107.0 deg |
| 3 | Mav5 | SaltLakeCity | `expired`, range 1852.0, travelled 1853.5 | 182.6 m | 8.00 s | 60.05 s | 132.9 m | 1.8123 rad = 103.8 deg |

Both hits land on `Northampton`, each for 1108.9 / 1109.0 damage against base 1200.0, range 50.0
and armour 90.0, taking it from 6500 to 5391.1 to 4282.1.

**The result is explained entirely by which ships this branch set in motion.** `Northampton` has
no authored `StartSpeed` and its group was emptied by the merge, so it never moves
(`target_moved=0.0 m`) and both rounds aimed at it hit at 8.5 and 9.1 m. `Dunlap` and
`SaltLakeCity` are the two ships that now run the follow step, they move 129 to 141 m during the
torpedo's run, and all three rounds aimed at them miss by 175 to 210 m and expire at 1852 m of
range. The aim carries no lead, so a moving target is missed by roughly its own displacement.

Per-aircraft state ticks (`arm_ticks=1299` for all five):

| aircraft | transitions | prepare | attackrun | aim | goaway | done | releases |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Mav1 | 4 | - | 263 | 181 | 399 | 456 | 1 |
| Mav2 | 5 | 1 | 305 | 194 | 251 | 548 | 1 |
| Mav3 | 5 | 1 | 291 | 195 | 251 | 561 | 1 |
| Mav4 | 5 | 1 | 286 | 179 | 723 | 110 | 1 |
| Mav5 | 5 | 1 | 278 | 180 | 719 | 121 | 1 |

Against the reference the brief quotes for `main` (Mav1 attackrun 424 / aim 254 / goaway 308 /
done 313) every column has moved, and Mav1 alone is `transitions=4` with no `prepare` tick. Both
differences are attributable to `main`'s own dive-bomb and torpedo work merged in at
`3b6277359` as much as to this branch, and are **not** separated here - a clean attribution needs
a `main` build of the same commit, which is the residual named below.

Against `J:\PROG\battlestations-pacific-decompile-cc8-ship-command\local\lead_usn01_control.log`,
the stationary-escort control (5/5 hits, crossings 115-121 deg): this branch keeps **2 of 5**, and
the two it keeps are the two whose crossing angles (118.6, 121.3 deg) sit inside the control's own
115-121 deg band - i.e. the geometry that produced a hit is unchanged, and only the rounds whose
targets now move are lost. That is the intended consequence of the packet, not a regression in the
torpedo path.

### 6.4 USN04, `local/screen_usn04.log`

| row | predecessor's `coord_on_usn04.log` | this run | delta |
| --- | --- | --- | --- |
| `summary ship follow steppers` | 16 | 16 | 0 |
| `summary mission ai follow` | requests=748 available=1 refused=747 joins=1 | requests=731 available=1 refused=730 joins=1 | -17 requests |
| `summary unit formation` | groups=2 joins=25 creates=2 rejoins=0 clamped=9 columns_unmeasurable=0 | identical | 0 |
| `total_path` | 29455.50 | 29455.68 | +0.18 |

The two moved rows are both frame-timing sensitive and both are explained by `main`'s dive-bomb
work changing when aircraft reach their requesting states; the formation counters, which are what
this chain owns, are identical.

### 6.5 The coordinator pair, completed

The predecessor left this half-done: `coord_on_usn04.log` existed and the without-fix half did
not, so the fix was observed but never measured. Both halves now exist **on one build differing
only by the `if (host.ai == nullptr)` guard in `create_units`**, with the guard removed for the
OFF run and restored afterwards, same mission and same parameters.

| row | OFF, `local/coord_off_usn04.log` | ON, `local/screen_usn04.log` | delta |
| --- | --- | --- | --- |
| `summary mission ai follow` | requests=**238** available=**0** refused=238 joins=**0** | requests=**731** available=**1** refused=730 joins=**1** | +493 / +1 / +1 |
| `summary ship follow steppers` | 16 | 16 | 0 |
| `summary unit formation` | groups=2 joins=25 creates=2 rejoins=0 clamped=9 columns_unmeasurable=0 | identical | **0** |
| `total_path` | 26985.10 | 29455.68 | **+2470.58** |

This reproduces the artefact the guard was added for, exactly as it was described. Without the
guard the coordinator is rebuilt on every spawn batch, so its counters are the **last batch's
alone** - 238 requests where the mission really makes 731, and no join at all because the batch
that could have made one had its coordinator thrown away. The units host survives the rebuild, so
its `joins=25 clamped=9` is **identical in both halves**, and that is precisely what made the
artefact look harmless to the earlier packets: the counter that moved was not the one being read.

The one behavioural consequence is the join: with a coordinator that lives for the mission, the AI
path finds its one available follow and makes it, and `total_path` rises 2470.58 m as the joined
ship steams to its station. Every other row is unmoved.

### 6.6 USN01 state census

Taking each unit's last logged `ship ai step` state over the 918 census lines of
`local/screen_usn01.log` (21 units are logged; the mission has 62 entities, most of which are not
ships and never appear):

| state | units |
| --- | --- |
| `cruise` | 9 |
| `not_ship` | 7 |
| `follow` | **2** (`Dunlap`, `SaltLakeCity`) |
| `stop` | 2 |
| `movetopos` | 1 |

The two in `follow` are exactly the two `steppers` the follow summary reports. Note that this is
a *last-state-per-unit* census; the "ships in `stop` = 49" figure the previous packet's prediction
table used counts a different population and the two are not comparable.

### 6.7 The two-way call-table diff against a `main` build

`J:\PROG\battlestations-pacific-decompile-cc8\local\main_usn01_3b6277359.log`, taken by the
integrator on `main@3b6277359` with this packet's exact parameters, against
`local/screen_usn01.log`. Main has 1227 call-table rows, this branch 1166.

| section | rows | what they are |
| --- | --- | --- |
| only on `main` | **84** | almost entirely the `ShipAiApproach::*` and `ShipAiApproachPoint::*` family - the `order_attack` stand-in this branch replaces. `score_slot_009E6640` and `score_slot_009E6870` alone are 210240 calls each, `scan_scale_1284_target_0370` 277984, and the family's frame routines sit at 3504 = one per tick per standing-in ship |
| only on this branch | **23** | the `ShipAiFollow::*` family - `station_point [0070D290] calls=1200`, `update_formation_point [009DF2D0] 1200`, `leader_body_speed [0092D730] 4800`, `turn_radius [0082E850] 3598`, `push_out_of_zones [00417B10] 2400` - plus `Formation::route_join_message [0077C964] calls=10` and `Formation::entity_route_slot [0077C904] calls=10` |
| status moved | **2** | `AiCommand::request_join_formation` and `Formation::command_is_available`, both `UNIMPLEMENTED` on `main` and **`concrete`** here. These two rows are the packet chain's core contribution |
| calls moved | 197 | led by `WeaponDirector` (28 rows), `Gun` (13), `ShipAiGoal` (11), `Projectile` (10), `ShipAiPlanner` and `ShipAiMoveTo` (8 each) - the downstream consequence of two ships holding station instead of steaming, and of `main`'s own dive-bomb work |

**Which ships move, and the reverse.** Final state per unit:

| build | `attackmove` | `cruise` | `follow` | `stop` | `movetopos` | `not_ship` | `total_path` |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `main@3b6277359` | **6** | 6 | 0 | 1 | 1 | 7 | **17579.17** |
| this branch | 0 | 9 | **2** | 2 | 1 | 7 | **10852.37** |

On `main`, `Dunlap`, `SaltLakeCity` **and `Northampton`** are all in `attackmove`, steaming under
the stand-in; on this branch `Dunlap` and `SaltLakeCity` are in `follow` and `Northampton` is in
`stop`. No ship moves on this branch that does not move on `main`. The 6726.80 m of `total_path`
the branch does **not** travel is the stand-in's six `attackmove` ships being replaced by two
station-keepers and four ships that correctly hold `cruise` or `stop` - which is also why
`Northampton` is stationary here and takes both torpedo hits (section 6.3).

### 6.8 What `00811180` answers off both ends of the trail

The integrator asked for this explicitly, so it is stated with addresses. The search seeded at
`00D7A248` (`FLT_MAX`) walks back from the head through **all forty slots**, keeping the nearest
by full 3D squared distance and its step count in `EDI`; the answer is therefore always **one of
the forty stored samples**, never a point interpolated along a leg. The arc length is then the sum
of those samples' own stored legs, `EDI` terms of `FLD float ptr [ESI+EDX*8]` accumulated into
`[ESP+44h]` by the loop at `00811700-00811721`, which `008117FD` returns from `[ESP+38h]` after
three intervening `POP`s.

* **Forward of the head.** The nearest sample is the head, `EDI = 0`, the accumulator loop does
  not execute at all, and `along` is exactly `0.0`. `across` is then
  `sign * |(point - head) x dir_head|` (`00811768-008117C0`, magnitude at `008117D6`, sign applied
  at `00811803`), i.e. the perpendicular distance from the head's direction line. **The
  along-track component pointing ahead is discarded**, because the wake frame has no ahead. This
  is what `Ralph` and `McCall` get at 68.9 m forward, and what `SaltLakeCity` and `Dunlap` get at
  2462 m forward after the `FollowerMaxDist` clamp.
* **Beyond the tail.** The nearest sample is the oldest live slot, `along` is the sum of every
  stored leg, i.e. the whole trail length, and `across` is again the perpendicular distance from
  that sample's direction line. **There is no extrapolation along the last leg** - the excess
  beyond the tail is discarded exactly as the excess ahead of the head is.

Both ends are clamped structurally rather than by a test: because the result is always one of the
forty samples, `along` is confined to `[0, trail length]` by construction. `src/ship_ai_wake_trail.cpp`'s
`ship_ai_wake_decompose_00811180` has the same two properties - a nearest-**sample** search over
`kShipAiWakeSampleCount` and an `along` summed from `samples[...].segment` - so **the host is
faithful at both ends**, and that is the instruction-level answer to whether the 3152 m abeam
station is ours or the image's. It is the image's.

## Still open

* **The absolute sign of `record+10h`.** `00811180`'s sign comes from `sign(ST1 - ST0)` at
  `00811731` with five live x87 values; naming them needs a backward walk through the unrolled
  search. It cancels for column 0 and only becomes load-bearing for the canned LINE / COLUMN /
  DIAMOND columns 1-3, which nothing in USN01 or USN04 selects.
* **`008193A0`'s `[00E188A8]+1FE4h` values.** `2` disables all three arms' formation behaviour and
  `1` routes the placement through a session message; what the states *are* is not read here.
* **`this+0BCCh = 1.25f`.** Written on every placement, immediately below the wake object at
  `+0BD0h`. Its reader is not identified.
* **The USN01 call-table diff is DONE** (section 6.7), on a `main@3b6277359` build the integrator
  took at this packet's parameters. **A `main`-build USN04 was not taken**, so section 6.4's two
  moved rows (`requests` -17, `total_path` +0.18) are still attributed jointly to this branch and
  to `main`'s dive-bomb work rather than separated. Sections 6.3's per-aircraft torpedo ticks are
  likewise compared against the brief's quoted `main` reference, not against a `main` build of
  `3b6277359`; the USN01 diff now bounds how much of that is this branch (the `ShipAiApproach`
  family disappearing and `ShipAiFollow` appearing) and how much is not.
* **`008193A0` is bound but not wired.** The host's placement path does not apply the formation
  override, the placement refusal or the occupant-owner group snap. Nothing in USN01 or USN04
  dispatches slot `+118h` at a unit that is already a formation follower, so wiring it would have
  moved no measured row; it is left for a packet that has a mission which exercises it.
