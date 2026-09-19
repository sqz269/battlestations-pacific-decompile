# The fly-over's last two flags: `flyabove+1Bh` and `flyabove+1Ch`

Packet `cc8_dive_flyover`, branch `agent/cc8-dive-flyover`. This file takes the two addresses
`docs/HANDOFF_DIVE_BOMB_HEADING.md` section (b) named as unread, plus the dead-band half-width `T`
that the same handoff left with two untraced producers.

The fly-over tick is `009C62B0`-`009C7085`; `ESI` is the state and `EDI` is `&state->approach`.
Every listing quote below is from `tools/flyabove_trace.ps1`, the predecessor's frame walk (frame
base `0x98`, zero join conflicts over all 949 instructions); this packet re-ran it and did not
re-seed it.

## 1. `flyabove+1Bh` is a second read of the old-style-bombing switch

**Proved from the listing.** The byte has exactly four writers in the whole image, and three of
them are a constant zero:

| address | instruction | value | enclosing |
| --- | --- | --- | --- |
| `009C61A6` | `MOV [EAX+1Bh],CL` | `CL = 0` (`009C6189 XOR ECX,ECX`) | the state constructor at `009C6180`, vtable `00D20D04`, `RET 4` |
| `009C75D1` | `MOV [EBP+1Bh],BL` | `BL = 0` | the task constructor `009C73A0`, the same constructor inlined |
| `009C628F` | `MOV [ESI+1Bh],AL` | `AL = 0` (`009C6281 XOR EAX,EAX`) | the fly-over's **enter**, `009C6270` |
| `009C6813` | `MOV [ESI+1Bh],DL` | `DL` | the fly-over's tick |

The census is `python tools/store_census.py 0x1b`, which covers `disp8` and `disp32` for twelve
store forms: 19 hits image-wide, four of them above, the other fifteen in unrelated classes
(`BSP_ParticipantRecord_SetGateByte9`, `BSP_StlVectorInt_Insert`, `BSP_NativeTraceline_*`, ...).
A `disp8` search is what this needs: `+1Bh` on a small state object is never a `disp32` store, so a
`disp32`-only scan would have returned nothing and proved nothing.

**`DL` at `009C6813` is `base[ESP+27h]` on every path into it.** Traced backward from the store,
never forward:

```
009c6532  CMP [ESI+1Bh],0
009c6544  JNE 009c67b6                ; the clamp skip
009c654a  MOV DL,[ESP+27h]            ; <- DL, arm A
009c654e  TEST DL,DL
009c6550  JZ  009c655a
009c6552  TEST BL,BL
009c6554  JNZ 009c67ff                ; old-style && BL
...
009c67b6  (the 009C6544 skip lands here)
009c67bf  TEST BL,BL
009c67c1  JZ  009c6a37                ; BL == 0 leaves; nothing reaches 009C6813
009c67cb  MOV DL,[ESP+27h]            ; <- DL, arm B
009c67cf  XOR EBP,EBP
009c67d1  TEST DL,DL                  ; <- the only other predecessor, 009C6806, jumps HERE
...
009c67ff  XORPS/FSTP/FSTP             ; arm C, entered from 009C6554 with DL still arm A's value
009c6806  JMP 009c67d1
...
009c680a  CMP [ESI+1Ah],0
009c680e  MOV [ESI+18h],AL
009c6811  JZ  009c6816                ; +1Ah == 0 skips the store
009c6813  MOV [ESI+1Bh],DL
009c6822  MOV [ESI+1Ah],0             ; and +1Ah is cleared straight after
```

Arms A, B and C are the only ways in, and each carries `[ESP+27h]`. Nothing writes `DL` between
`009C654A` and `009C6813` on arm A, or between `009C67CB` and `009C6813` on arm B.

`base[ESP+27h]` is `squadron+3A8h`, the **old-style-bombing** flag: `009C64D1 MOV AL,[EAX+3A8h]`
with `EAX = approach+0Ch`, zeroed at `009C64DD` when there is no controller. The integrator
established that byte in full (`0089EB39` from `luaMW_SquadronSetOldStyleBombing`, constructor
default 0 via `007F2BD0`/`007F2C4E`, no script in this installation calls the native), and that is
also the byte `009C6554`'s skip tests.

**Consequence.** `009C6813` writes `+1Bh := squadron+3A8h` on the tick a break-off is pending, and
in this installation that value is 0. The enter has already written 0. So `+1Bh` is 0 for the whole
life of every fly-over state here, `009C6544`'s `75` JNE is never taken, and:

* the **210 m release clamp at `009C657C` is unconditional in this installation**. Both of its
  skips - `009C6554`'s and `009C6544`'s - are the same scripted switch, read twice. The
  "holds the begin altitude and rolls in" attack the authored comments describe is the old-style
  branch and this installation never gets it;
* `L`, the slew limit on the commanded heading, is **`pi/2` on every path**. The 10-degree `L` of
  `009C6497` survives only when `+1Bh != 0 && BL == 0`.

Recorded as `dive_bomb_flyabove_constant::kOldStyleBombing1b` in
`include/bsp/dive_bomb_task.hpp`. **No behavioural change follows**: the host never modelled a
clamp skip, so binding `+1Bh` confirms the altitude arm rather than moving it. This packet does not
spend a run on it.

Residual uncertainty: the store census is a byte-pattern scan, so a block copy that moved a whole
state object would not appear in it. It would be copying a zero.

## 2. `flyabove+1Ch` is the roll-in latch, and it fires at 120 m

**Proved from the listing**, `009C6857`-`009C6923`. Three quantities go in:

```
Eabs = |E|                        base[ESP+2Ch], folded at 009C642F-009C6453 by (-0.0) - E
E    = SubtractWrappedAngle(bearing_to_lead_point, own_heading)   009C641C -> base[ESP+1Ch]
R    = the three-second lead range                                009C63A6 -> base[ESP+28h]
Rt   = classDesc+268h TurnCircleRadius * 1.4                      009C6853 -> base[ESP+30h]
```

```
009c6861  +1Ch != 0                      -> 009c6929, skip the arm whole (the LATCH)
009c6889  1.5 * sin(Eabs) * R  >  Rt     -> +19h = 0, T = 0 (009C6893's XORPS zero)
009c68a0  otherwise                         approach+CCh = 3, the weapon selector
009c68d4  cos(Eabs) * R - 120.0 <= 0     -> 009c6919 +1Ch = 1
          otherwise                      -> +19h = 0 and 009C6911's T
009c691f  +1Ch == 0                      -> 009c6a37, the dead band and the heading command
          +1Ch != 0                      -> 009c6929, the bank command
009c6dda  +1Ch != 0                      -> skip the heading write at 009C6DE7 entirely
```

`120.0` is the **double** at `00D1F3F8` (`009C68C2 FSUB double ptr`); the `0.5` at `00D7A280` is a
double too, and `009C686F`-`009C6877` folds it into `sin + 0.5 * sin`, i.e. `1.5 * sin`. So the
cross-track test is "does the roll-in fit inside one turn circle" and the along-track test is "am I
within 120 m of the lead point".

**What the latch does** is switch the fly-over from heading control to bank control for the last
120 m: `009C6923`'s `0F84` JZ no longer reaches the dead band, `009C6929`-`009C69F1` writes
`cmd+2C4h = 0` with `cmd+2CCh = 1` and a signed `cmd+284h`/`cmd+288h` pair built from the sign of
`E`, and `009C6DDA` drops the heading write. It is per-state: `009C629E` clears it on the enter and
nothing else writes it.

**`T`, the dead-band half-width, now has all three producers read.** `009C6893` and `009C6911` were
the two the previous packet could not trace; both fall out of the same window.

| site | value | when |
| --- | --- | --- |
| `009C6674` | `InterpolateClamped(0, 30 deg, 200.0, 0, span)` | every path that leaves before the bank arm |
| `009C6893` | `0.0` (the `XORPS XMM0,XMM0` of `009C67BC`/`009C67FF`) | cross-track wider than the turn circle |
| `009C6911` | `InterpolateClamped(0.0, 100 deg, TurnCircleRadius, 10 deg, R)` | the bank arm's common exit |

`009C6911`'s five arguments come out of the window the `SUB ESP,14h` at `009C68DA` opens, with the
frame base at `0xAC`: `arg5 = R` (the `FXCH` at `009C68D8` brings it to `ST0` before the store at
`009C68DD`), `arg4 = 0.1745329` at `00CE3990`, `arg3 = classDesc+268h` **unmultiplied**,
`arg2 = 1.7453293` at `00CEDD00`, and `arg1 = 0.0` - the `FLDZ` of `009C68CC`, which survives the
`FCOMIP` pop at `009C68D2` and is still `ST0` at `009C6906`. Each constant read at the width of the
instruction that loads it (`tools/pe_const_read.py`).

So the dead band runs from **100 degrees at range 0 down to 10 degrees at one turn circle**,
clamped. It is wide, not narrow, and that is what makes the substitution it replaces wrong: with
`T` at 10 degrees the aircraft ignores a bearing error under 10 degrees entirely, and inside the
turn circle it very nearly stops steering.

## 3. `BL`, the predicate that admits the bank arm

Read for the same packet, `009C64EE`-`009C6530`, because everything in section 2 sits behind it:

```
009c64ec  AL = vtable[5Ch](0x14) on approach+4h, the UNIT  UNBOUND
009c64fc  AL != 0                          -> BL = 0
009c6510  approach+D4h  >  C               -> BL = 0    (FCOMI / `77` JA)
009c651a  approach+B4h <=  R               -> BL = 1    (FCOMPI ST(4) / `76` JBE)
009c6522  B < approach+D4h                 -> BL = 0    (FCOMI ST2 / `72` JB), else BL = 1
```

`B` is the height above the aim point (`base[ESP+38h]`), `R` the lead range, and **`C` is the
UNCLAMPED commanded altitude** of `009C64C9`. That ordering matters: the 210 m clamp at
`009C6580`-`009C6589` is inside the `009C654A` branch and runs *after* this test. Comparing
`approach+D4h` (675 m in this installation) against the post-clamp 210 m would veto the bank arm on
every tick of every mission, which is the trap this binding avoids and the reason `unclamped_c` is
computed separately at the call site.

`vtable[5Ch](0x14)` is the one input no reconstruction supplies, and it is better understood than
"unknown". `ECX` is the approach - `009C64B2` and `009C64B8` read `[ECX+ACh]` and `[ECX+50h]` off
it - and `009C64E2` takes `[ECX+4]`, which `include/bsp/dive_bomb_task.hpp:116` records as **the
aircraft unit** (`009F9CEA`). And `include/bsp/attack_commands.hpp:29` already establishes
`entity->vtable[5Ch]` as **`IsKindOf(int)`**, the class-id test, with a table of the kinds other
call sites ask for: `0x0F`/`0x18` aircraft, `0x10` level-bomber-self, `0x16`
dogfight-excluded-self, `0x17` kamikaze-capable-self, `0x1C` structure. A byte scan for the same
call shape (`8B 42 5C 6A ?? FF D0`) finds 267 sites across the image with small enum literals, which
is what a generic class test looks like.

So `009C64EC` is **`aircraft->IsKindOf(0x14)`**, and `0x14` is a kind the repository has not named.
Two consequences:

* it is a **class** test, so for a given aircraft type it is constant for the whole mission - not a
  per-tick state. For the D3A Val it is either always true or always false;
* it is a self-test in the same family as `0x10` and `0x17`, and `009C64FC`'s JNE reads "if this
  aircraft IS kind 0x14, do not run the bank arm". The bank arm is the dive-bomb fly-over's
  roll-in machinery, so a dive bomber being the excluded kind would make it dead code for the very
  aircraft it exists for. That is the argument for taking it `false` - an argument, not evidence.

It is a **labelled substitution**, taken as `false`.

**This single substitution decides the whole behavioural change, and that has to be said plainly.**
If the query returns true, `BL` is 0 on every tick, the bank arm never runs, `T` is whatever
`009C6674` left, and `009C6674`'s value is 0 for every span at or above 200 m - which is most of
the fly-over. With `T <= 0` the dead band at `009C6A46` is skipped and the commanded heading *is*
the bearing, i.e. exactly what this host did before this packet. So the two readings are:

* query false (taken here): the bank arm runs, `T` is 10-35 degrees, the fly-over holds heading
  inside the dead band. This is what the run below measures.
* query true: the fly-over commands the bearing, and the host was already right.

Nothing in this packet distinguishes them. Naming kind `0x14` and deciding whether the D3A Val
answers it is the single highest-value follow-up to this reading, and it should be settled before
the behavioural change is taken as final - it is one class-id lookup, not a trace, now that
`vtable[5Ch]` is known to be `IsKindOf`. The measurement below stands either way: it is what the
host does with the query taken `false`.

## 4. What is bound, and the prediction written before the run

Bound this packet, in `include/bsp/dive_bomb_task.hpp` / `src/dive_bomb_task.cpp`:
`dive_bomb_flyabove_span_dead_band_009c6674`, `dive_bomb_flyabove_bank_arm_009c6530`,
`dive_bomb_flyabove_bank_009c6857`, `dive_bomb_flyabove_dead_band_009c6a37`,
`dive_bomb_flyabove_slew_009c6d6f`. Wired in `src/game_hosts_units.cpp`'s
`run_dive_bomb_flyabove_tick_009c62b0`, which now runs the altitude arm first (it is what produces
`C`) and computes the heading as

```
delta   = deadband(E, T)                     009C6A37-009C6A7F, identity when T <= 0
heading = AddWrappedAngle(C_heading, clamp(delta, +/- pi/2))      009C6D6F-009C6DC8
```

in place of the previous `heading = the bearing to the lead point`. The `suppress_heading_1c`
contract ("this host keeps no flyabove `+1Ch`, so it never suppresses") is **withdrawn**.

Still unbound and labelled at the call site: the `009C64EC` query, and the avoidance increment
`009C6D59` adds to `A` out of the unbound `007F0280`. `009C688F`/`009C68E1`'s clear of `+19h` is
read and **not applied**, because this host recomputes `+19h` from `009C67B0`'s rule alone in the
state feed each tick, ahead of the transition rule, so a clear written during the tick could never
be read.

### Prediction, per squadron, written before the window is read

From `local\heading_after.log`'s hand-over table the fly-over runs from about `rng 2078` to about
`rng 364` (`movieval`, `D3A Val #1.1`, `#5.1`) or `rng 150-272` (`#3.1`, `#7.1`), with the begin
altitude about 1395 m and `approach+D4h` 675 m.

1. **`+1Bh` changes nothing.** Every aircraft keeps the 210 m clamp it already had; the hand-over
   altitudes and the `f18`/`f19` columns should reproduce `heading_after.log` exactly wherever the
   heading has not yet moved the aircraft.
2. **`BL` is 1 for most of the fly-over.** `D4h = 675 <= C = 1395` passes, and `B4h <= R` holds
   while the aircraft is still outside the attack distance. Expect `bl=` close to the tick count.
3. **The latch fires late or not at all.** It needs `cos(Eabs) * R <= 120 m`. The three squadrons
   that leave the fly-over at `rng ~364` never get there, so I predict `latch=0` for `movieval`,
   `#1.1` and `#5.1`. `#3.1` and `#7.1` reach `rng 150-272`; if their bearing error stays small
   they still stop short of 120 m, so I predict `latch=0` for them too, with `along=` bottoming out
   between 150 and 270 m. A `latch=1` anywhere would mean the fly-over is being flown much closer
   than the before table shows.
4. **The dead band is the change that bites.** With `T` at 10 degrees for all of `R > TurnCircleRadius`,
   the fly-over will stop correcting bearing errors under 10 degrees. I expect the hand-over
   bearings to be *looser* than `heading_after.log`'s, the fly-over to last the same number of
   ticks or slightly more, and the release count to be unchanged or lower. If `T` reaches its
   100-degree end the aircraft will barely steer at all; that would show as a large `T=` in the new
   `flyabove bank` census row and a hand-over bearing far off.
5. **Damage should not rise.** `heading_after.log` is `queued_hits=68 hull=45 deaths=8
   total_damage=11427.2`. A faithful dead band is a *less* aggressive steering law than the raw
   bearing this replaces, so if anything moves it should move down. This packet is not a tuning
   exercise: the reading stands on the listing, and a worse number is a result, not a defect.

## 5. Measured: `local\flyover_after.log`

4800-frame USN04 on `65c997deb`, against `cc8-dive-heading`'s `heading_after.log` on the merged
`d6275ef49`. The two differ only by this packet's commit.

### The four predictions

| prediction | outcome |
| --- | --- |
| 1. `+1Bh` changes nothing | held. Every aircraft keeps `cmd=210` at the fly-over hand-overs and `f18`/`f19` are unchanged |
| 2. `BL` is 1 for most of the fly-over | held, and stronger: `bl=` **equals the tick count on all fifteen aircraft** (96, 84, 78, ...). The bank arm runs on every fly-over tick |
| 3. the latch fires late or not at all | held exactly: `latch=0@-1 sup=0` everywhere. `along=` bottoms out at 914-974 m against a 120 m gate |
| 4. the dead band is the change that bites | held, and it is the reason 2 and 3 came out as they did |

### What the census row says, and the number that was guessed wrong

```
divebomb movieval     flyabove bank: bl=96 latch=0@-1 sup=0 T=0.6108 rad along=936.0 m cross=111.3 m turn_circle=1300.0 m
divebomb D3A Val #3.1 flyabove bank: bl=84 latch=0@-1 sup=0 T=0.5672 rad along=936.1 m cross=409.4 m turn_circle=1300.0 m
divebomb D3A Val #7.1 flyabove bank: bl=78 latch=0@-1 sup=0 T=0.5391 rad along=973.7 m cross=329.8 m turn_circle=1300.0 m
```

The D3A Val's `classDesc+268h` **TurnCircleRadius is 1300 m**, not the few hundred metres the
prediction assumed. So the dead band never sits at its 10-degree end: `R` runs from 2077 m at the
hand-over (above the turn circle, `T` clamped to 10 degrees) down to about 930 m at the hand-out,
where `T = 100 - 90 * 930/1300 = 35.6` degrees. The measured `T=0.6149 rad` is 35.2 degrees, and
the interpolation predicts 0.6213 rad at the printed `along=932.6 m`; the residual is the sample
being the last bank tick rather than the last tick of the state.

That 1300 is authored, not a host default: `scripts\datatables\autoload\vehicleclasses.lua`
line 54263 carries `["TurnCircleRadius"] = 1300` for the row whose `Comment` at 54032 is
`"D3A Val"`. **In this installation** - that file is locally modified, mtime 2026-05-09 21:52, and
the repository's notes already record this install as modded, so the number is this install's and
not necessarily retail's.

The same number is why the latch cannot fire here. `along = cos(|E|) * R` is 914-974 m at the
hand-out, and the gate is 120 m: the fly-over gives the aircraft to the turndown at seven times the
distance the roll-in latch wants. The `+1Ch` arm is therefore **read, bound and inert on USN04** -
which is a result, not a gap, and it is what makes `009C6929`'s bank command and `009C6DDA`'s
suppression unreachable in this mission. `cross=` separates the two groups cleanly: 110-111 m for
the six aircraft that fly a clean dive, 330-449 m for the nine that do not.

**Coverage of the new binding, honestly.** `dive_bomb_flyabove_bank_009c6857` has four exits and
USN04 exercises **one**. `bank_arm_bl == false` never happens (`bl=` is the tick count);
`cross_track > turn_circle * 1.4` never happens (the widest measured cross-track is 449 m against
1820 m); the latch never fires. Every live tick takes the `009C6911` exit. So `009C6893`'s `T = 0`,
the latch itself and the `009C67C1` early-out are proved from the listing and **not** exercised by
any run in this repository. `dive_bomb_flyabove_dead_band_009c6a37` and
`dive_bomb_flyabove_slew_009c6d6f` are exercised on every fly-over tick, and the release numbers
below are their evidence.

### The behavioural change, and what moved

| quantity | `heading_after.log` | `flyover_after.log` |
| --- | --- | --- |
| dive-bomb task `releases` | 17 | **19** |
| `bomb_drops` / `bomb_impacts` | 17 / 16 | **19 / 18** |
| `D3A Val #7.1` flight releases | 0, 1, 0 | **1, 1, 1** |
| `movieval` aim error `009C5C9B` | 8.05 m | **2.89 m** |
| `movieval` turndown / aimdive entry bearing | 0.0099 / 0.1833 rad | 0.0605 / 0.2688 rad |
| `#3.1` turndown / aimdive entry bearing | -0.0049 / 0.2491 rad | 0.1865 / 0.5090 rad |
| fly-over ticks (`movieval`) | 96 | 96 |
| `queued_hits` / `hull` / `deaths` | 68 / 45 / 8 | 61 / 41 / 8 |
| `total_damage` | 11427.2 | 9826.4 |

The hand-over bearings are **looser**, by 0.05 to 0.26 rad, exactly as prediction 4 said they would
be: a 10-to-35-degree dead band stops the fly-over correcting anything inside it, so the aircraft
holds heading where it used to chase the lead bearing. That is what the image does.

The release count moved the other way. `D3A Val #7.1`'s whole flight now releases, the leader
included - it had released nothing at all before (`release alt=-1.0 m`), and it now releases at
345.4 m with an aim error of 6.47 m inside the 25 m gate. `movieval`'s aim error more than halved.
So the looser fly-over hands the turndown a better dive, not a worse one.

### RETRACTED: the damage drop is mine, and USN04 is deterministic

The first draft of this section said the damage totals were "one sample" and carried a gunnery
difference that could not be separated from this packet's effect. **That was wrong, and the reason
it was wrong is worth more than the claim.** The brief this packet was given states that "gunnery
totals on USN04 are NOT deterministic run to run". They are.

`local\usn04_rebaseline.log` and `local\usn04_rebaseline2.log` are two runs of the same binary at
the same settings, taken one after the other. Filtering both logs to the census lines
(`divebomb`, `torpedo`, `summary mission`) gives **3340 lines each and zero differences**. The raw
files do differ on 9426 of 46917 lines, and every one of those carries a process address, a handle
or a worker id (`owner=006EF810`, `lua=03D8CED0`, `worker=18560`, `storage=actual1d94h`). The
mission is reproducible; only the allocator is not.

So the before/after pair differs only by this packet's commit, and the damage move is **this
packet's**, not noise:

| unit, `taken` column | `heading_after.log` | `flyover_after.log` |
| --- | --- | --- |
| `Yorktown-class01` | **1445** | **0** |
| `Northampton-class03` | 221 | 0 |
| `Northampton-class02` | 39 | 0 |
| `Fletcher-class05` | 0 | 204 |
| `Northampton-class05` | 14 | 36 |
| `Northampton-class04` | 0 | 16 |
| `Lexington-class01` | 8000 (sunk 123.70 s, `B5N Kate #6.1|.-2`) | 8000 (same, same second) |

The whole of the 1600-point drop is the carrier: the dive bombers' own target took 1445 before and
takes nothing now. Two more bombs are released and two more impact, and the target is hit less.

**The mechanism, from the per-round rows.** The bombs miss by tens of metres in *both* runs:

```
BEFORE  bomb from D3A Val #3.1 impact ... | vs target at release = 30.3 m | died above water (entity sweep)
AFTER   bomb from D3A Val #3.1 impact ... | vs target at release = 41.7 m | died at the sea surface
```

`died above water (entity sweep)` is a hit on a hull; `died at the sea surface` is a miss into the
water. Across both runs the release solution puts bombs 11 to 43 m from the aim point, and a
warship is a few tens of metres wide - so the chain is sitting exactly on its hit/miss boundary,
and an 11 m shift flips individual bombs across it. The predicted impact point of `009C7D71` is
good to 2-12 m in both runs; it is the *release geometry*, not the ballistics, that is loose.

**What that means for this packet.** The binding is faithful to the listing and the dive quality
improved by every measure the dive-bomb census has - two more releases, two more impacts,
`movieval`'s aim error more than halved, `#7.1`'s whole flight bombing for the first time - and the
mission outcome still got worse, because the release accuracy downstream is not fine enough for
those improvements to survive to the hull. This is a result, not a defect in the reading, and it
relocates the lever: the dive-bomb chain's bottleneck is now the **release solution's 25-43 m
error**, not the fly-over's steering. Whether to carry the change is the integrator's call, and the
evidence for both sides is above.

## 6. Item 3: why `D3A Val #3.1` drops one bomb and never the second

Read from `heading_after.log` and confirmed in `flyover_after.log`. **The brief's premise is wrong
for two of the three squadrons** and is corrected here.

| flight | `heading_after.log` | `flyover_after.log` |
| --- | --- | --- |
| `movieval` x3 | 2, 2, 2 releases, `done` | unchanged |
| `D3A Val #1.1` x3 | 2, 2, 2 releases, `done` | unchanged |
| `D3A Val #3.1` x3 | 1, 1, 1, then 794-869 aimglide ticks | unchanged |
| `D3A Val #5.1` | 1, 0, 0 - **four transitions, no aimglide at all** | unchanged |
| `D3A Val #7.1` | 0, 1, 0 | **1, 1, 1** |

`#5.1` and `#7.1` have `arm_ticks` 1349 and 1339 against 2138-2370 for the others: they spawn late
and the 4800-frame mission ends mid-attack. `#5.1`'s wingmen are still in `aimdive` when the run
stops. They are a truncation, not a blocked release, and they should not be read as `#3.1`'s case.

**The gate that decides it is the aimdive ABORT, not an aimglide gate.** `009C5B43` fires for
`#3.1` (tick 1268, `h14=322.5` against `d4=675.0`, range 242.5, pitch -0.823) and for `#7.1`; it
clears `alive_19`, and `009C8557` hands the state to the aimglide. `movieval` and `#1.1` have
`abort fires=0`, exit the aimdive by `other`, release both bombs in one dive and go to `done`. The
first bomb `#3.1` does drop is the aimdive's, at 342.9 m with an aim error of 9.03 m inside the
25 m gate.

Once in the aimglide, nothing ever passes: 869 calls, `passed=0`,
`blocked[bearing=817 ceiling=38 lateral=10 lead_lo=3 rearm=1]`. The bearing gate takes 94 per cent
of the ticks; on the 52 where it passes the ceiling, lateral and lead gates take the rest, and the
lead gate is reached three times and fails low every time.

**Would the image re-dive, glide-release or retire with a bomb aboard? It would RE-DIVE, and the
869-tick sit is a host artefact.** The transition rule gives the aimglide exactly two exits, both
to `goaway`: `009C8694` on out-of-bombs and `009C86B4` on the state's `+76Ch` pull-out. From
`goaway`, `009C86D9` sends an aircraft that still has bomb ordnance (`has_bomb_ordnance_4c9`) back
to **flyabove** - a second attack run. Neither exit can fire in this host:

* `aimglide_out_of_bombs` is false, because the aircraft has a bomb left;
* `aimglide_pull_out_76c` is **hardcoded `false`** in `dive_bomb_transition_inputs`
  (`src/game_hosts_units.cpp`), and nothing would set it anyway: the host's
  `run_dive_bomb_aimglide_tick_009c5180` binds the heading arm and nothing else, so the aimglide
  state's `+18h`/`+76Ch` is never written.

So in this host the aimglide is terminal for any aircraft holding a bomb, and "retires with a bomb
aboard" describes the reconstruction, not the image. The fix is the aimglide tick's pull-out
producer, which is not this packet's hunk; it is recorded here and reported to the integrator.
Nothing about the abort itself is wrong - `dive_bomb_dive_abort_009c5b43` is a faithful binding of
a real rule, and the aircraft that abort are the ones that reach the dive with a saturated roll and
a bearing error near 0.89 rad while `movieval` reaches it at -0.16 rad.

### Column checks, before quoting any of the above

`throw=` in the aimglide row is `db_impact_throw_14` and `range=` is `db_planar_bc`, both
last-sampled rather than sampled at the gate; `lead last`/`min` are only updated when
`gate_reached >= 4`, so `#3.1`'s -115.91 m is one of its three lead evaluations, not a per-tick
minimum. The `(window -25.0..-5.0 m)` is printed as `4 * travel_20 + 5` and `5.0`, not read from
the image at print time.
