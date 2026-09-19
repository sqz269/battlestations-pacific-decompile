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
009c64ec  AL = vtable[5Ch](0x14) on (approach+0Ch)->+4     UNBOUND
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

`vtable[5Ch](0x14)` is the one input no reconstruction supplies. It is a **labelled substitution**,
taken as `false` (the arm runs), which is the reading the previous packet's run supports.

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
