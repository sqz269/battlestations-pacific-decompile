# The middle of 009F3F80: the obstacle sectors and the throttle ceiling

Addresses: 009F3F80 009F40CA 009F4B98 009D6B40 009D8B90 009EC7C0 009EF230 009EB660 009F4DA0
009F50E0 009EF910 009DA250 00415550 00415690 00419010 0042AC60 00438AA0 00438B10 0092D730
00424C40 009E04E0

Packet `cc_ai_obstacle_tables`, worker `agent/cc-ai-obstacle-tables`, 2026-09-12 UTC. Ghidra was
read-only for this packet: no renames, comments, prototypes, function creation or saves. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; every live query verified both.
Descriptive names are hypotheses, not recovered symbols.

## Answer

**The middle of `009F3F80` is one routine in six parts, and the thing that makes an AI ship back
off an obstacle is a one-second latch at `blk+380h` that narrows the throttle profile's window to
`[-1, 0]`.**

`docs/SHIP_AI_THROTTLE_TO_RING.md` projected the head and the tail of `BSP_ShipAi_DriveOrderRing`
(`009F3F80`, body `009F3F80-009F4D06`, `this` = blk) and left `009F40CA..009F4B98` as one record.
That record is:

| part | range | what it decides |
| --- | --- | --- |
| 1 | `009F40CA..009F4166` | four booleans out of the latched direction `blk+35Ch`, the committed direction `blk+364h`, `blk+36Ch` and the hull's signed speed |
| 2 | `009F4168..009F4392` | the danger level `blk+0A84h`, a ramp of the clearance `blk+37Ch / unit+9CCh` stepped through two dwell timers, and a load latch on the unit |
| 3 | `009F4394..009F44B2` | the throttle: `0.0f` when stopped, else `009EC7C0`'s ceiling limited by `blk+344h`, then snapped by the 65-bin profile at `blk+4h` |
| 4 | `009F44B3..009F4500` | the sector group index, then `009DA250` on the goal heading error when the steering mode is not Rudder |
| 5 | `009F4502..009F487C` | the obstacle sectors: the rudder limit `blk+348h`, the bucket, and the blocked-sector arm |
| 6 | `009F487D..009F4B98` | the escape manoeuvre that rocks a boxed-in hull ahead and astern |

The four questions the packet asked:

**The heading target.** There are two, and both go through `009DA250`. The ordinary one is the
state's own heading target `blk+324h`: the head of `009F3F80` builds the error at `009F40BB` with
`00438B10(blk+324h, heading)` and the middle hands it straight to the rudder law at `009F44F7`,
gated at `009F44E4` on `blk+1C4h != Rudder`. `blk+1DCh`/`+1E0h`, the goal position, is never read
here; `009ED6B0` has already turned it into `blk+324h`. The second target is the obstacle sector's
own avoidance bearing, `sector+20h`: `009F4681` builds `00438B10(sector+20h, reference)` and
`009F4694` overwrites `blk+1D4h` with the rudder law's answer to *that*. The reference bearing is
the hull heading, turned by a further half turn at `009F43BD` when the hull is moving against the
latched direction.

**What fills the tables.** `BSP_ShipAi_ScanObstacleSector` `009EB660`, reached once per sector from
`BSP_ShipAi_RefreshObstacleSectors` `009EF230`, which is the chain slot at `009F51FA` in
`009F50E0` — one slot before `009F4DA0` at `009F5248`, whose tail call runs `009F3F80`. So the
sectors are always one call fresh when `009F3F80` reads them. Not a spatial query of its own:
`009EB660` walks the neighbour list the control block already keeps at `blk+608h` (count
`blk+604h`).

**How an obstacle reverses the throttle.** Three different ways, in the blocked-sector arm:

- with no latched direction (`blk+35Ch == 0`), `00415690` clamps the throttle to the band opposite
  the way the hull is actually moving: `[-1, 0]` when making way ahead (`009F46E0`), `[0, +1]` when
  making way astern (`009F471B`);
- with a latch, the astern latch `blk+380h` is set to `1.0f` at `009F47A7` when the neighbour has
  picked the other passing side or is closing too fast, and while that timer runs the profile's
  window at `009F4878` is `[-1, 0]`: astern only, for one second;
- if every inner sector of the current group is blocked, the escape manoeuvre flips `blk+364h` and
  the hull rocks the other way.

**The throttle ceiling.** `009EC7C0` is five `00419010` stages over the `00424C40` tuning block,
all seven fields of it named in `docs/GAMEPLAY_SETTINGS.md`; and `009F4DA0` writes `blk+344h` and
`blk+348h`, the throttle and rudder magnitude limits this middle consumes. Both are below.

## 1. The four booleans, `009F40CA..009F4166`

`EAX = blk+35Ch` at `009F40C0`: every `mode` test in this range is the **latched direction**
(`ShipAiThrottleDirection`, 0 stopped / 1 ahead / 2 astern), not the steering mode at `blk+1C4h`.
The two speed gates come from the head: `009F403D` sets `[ESP+12h]` from the double `-0.4`
(`00CED6B0`) and `009F4057` sets BL from `+0.4` (`00CE65D0`).

```
speed_above_astern_floor = speed > -0.4          ; 009F4043, 009F4052
speed_below_ahead_floor  = speed < +0.4          ; 009F4066
hull_with_latch      = latch==0 || (latch==1 && speed_above) || (latch==2 && speed_below)  ; 009F40CA
committed_with_latch = latch==0 || (latch==1 && blk+364h)   || (latch==2 && !blk+364h)     ; 009F40EF
moving_or_stopped    = latch==0 || (blk+364h ? speed_above : speed_below)                  ; 009F411B
direction_mismatch   = (latch==1 && blk+36Ch != blk+364h) || (latch==2 && blk+36Ch == blk+364h) ; 009F413C
```

`blk+364h` is the committed ahead/astern direction: **clear means astern, set means ahead**. Three
independent sites say so and none disagrees. `009EC9CD` negates the whole throttle ceiling when the
byte is clear; `009F4441` caps the throttle at `+blk+344h` when it is set and floors it at
`-blk+344h` when it is clear; `009F44C3` picks sector group 1, the astern group, when it is clear
and the hull is stopped. `include/bsp/ship_ai_states.hpp` calls the same byte
`yaw_rate_subtracts_364` after how `009ED6B0` uses it; that is the same bit, described from the
other end.

## 2. The danger level `blk+0A84h`, `009F4168..009F4392`

```
ratio  = blk+37Ch / [blk+3FCh]+9CCh                      ; 009F416E FLD, 009F4174 FDIV
target = 00419010(1.0, 1.0, 4.0, 0.0, ratio)             ; 009F4189..009F41A2
```

`00419010` is `float __stdcall(x0, y0, x1, y1, x)`, `RET 14h`, so the five pushes read in slot
order: one half-width of clearance is full danger and four is none. `unit+9CCh` is the half width
`docs/SHIP_AI_ORDER_CONSUMER.md` reads at `009D8FDE`. `blk+37Ch` is written only inside `009EF910`,
which `009F4D10` calls one slot earlier; that routine was not read.

The step is hysteretic, and the decompiler's argument order for the `0042AC60` calls is wrong. From
the listing (`009F41C3..009F4332`, `ECX = EBP = &blk+0A84h` at `009F41BD`, `00D7A270` = 0.05,
`00D7A308` = 2.0):

```
if (target > v + 0.05) {                                  ; 009F41D7 FCOMI, 009F41E1 JBE
    if (blk+0A88h < 1.0) blk+0A88h += 2*dt;               ; 009F41ED, 009F4206
    if (blk+0A8Ch > 0.0) blk+0A8Ch -= dt;                 ; 009F4214, 009F4229
    if (blk+0A88h >= 1.0) 0042AC60(&v, target, 2*dt);     ; 009F423A JC skips, 009F42C1
} else if (v - 0.05 > target) {                           ; 009F4252 FCOMIP, 009F4254 JBE
    if (blk+0A8Ch < 1.0) blk+0A8Ch += dt;                 ; 009F4260, 009F4279
    if (blk+0A88h > 0.0) blk+0A88h -= dt;                 ; 009F428D, 009F42A2
    if (blk+0A8Ch >= 1.0) 0042AC60(&v, target, dt);       ; 009F42B3 JC skips, 009F42B5
} else {                                                  ; 009F42C8
    0042AC60(&v, target, 2*dt);                           ; 009F42E0
    if (blk+0A8Ch > 0.0) blk+0A8Ch -= dt;
    if (blk+0A88h > 0.0) blk+0A88h -= dt;
}
```

So the danger takes half a second of sustained pressure to start rising and a full second to start
falling, and once armed it moves at 2 per second up and 1 per second down. Outside the 0.05 band
nothing moves at all until the matching timer fills.

`009F4332..009F4392`: any danger above `0.01f` (`00D7A238`) raises `unit+102Ch`, the turn-assist
load latch of `docs/UNIT_COMMAND_PRODUCERS.md`, to `min(2 * danger, 1.5)`. The raise is `009D4FB0`
inlined: a compare at `009F4386` and a store at `009F438C`, no call.

## 3. The throttle, `009F4394..009F44B2`

```
if (blk+35Ch == 0)        blk+1D0h = 0.0f;                ; 009F43CD, 009F43D2
else if (blk+1C8h != 0) {
    cap  = 00415550(&1.0f, &blk+344h)                     ; 009F43EC..009F4406, the larger
    ceil = 009EC7C0(heading_error, blk+0A84h, cap)        ; 009F442A, ECX = blk
    blk+1D0h = ceil                                       ; 009F4439, FST keeps ST0
    if (direction_mismatch) {
        if (blk+364h) { if (blk+344h < ceil) blk+1D0h = blk+344h; }     ; 009F4462
        else          { if (ceil < -blk+344h) blk+1D0h = -blk+344h; }   ; 009F4482
    }
}
009D6B40(blk+4h)(&blk+1D0h, -1.5f, +1.5f)                 ; 009F448E..009F44AE
```

`blk+1C8h` is the throttle hold `009DBF90` clears when a state sets an explicit desired throttle,
so **the ceiling only runs when the state did not ask for a throttle of its own**. The final
`009D6B40` window is wider than the throttle's own range, so nothing there clamps: only the profile
can move the value.

## 4. `009D6B40`, the 65-bin throttle profile at `blk+4h`

`float* __thiscall(blk+4h)(float* value, float low, float high)`, `RET 0Ch`, body
`009D6B40-009D6D57`, complete. The arguments are the pushes at `009F449B..009F44AD`: `&blk+1D0h`
last, so first. Both ship-AI call sites use `this = blk+4h` (`009F44A7`, `009F4875` `LEA
ECX,[ESI+4]`) and so does the third, `009E10E2` in `009E04E0` (`009E10D9 ADD ESI,4`), on a stack
local with the window `[-1, +1]`.

The object is a cost profile over the throttle axis. `009D6B87` scales by the double `16.0`
(`00CED9F8`), `009D6B8F` adds `0.5` (`00D7A280`) and `009D6B99` truncates through `00BF7420`, which
is `CVTTSD2SI` at `00BF7435`; `009D6CE0` inverts it with `0.0625` (`00CEF290`) against the `2.0` at
`00D7A308`. So **bin `i` is the throttle `i * 0.0625 - 2.0`, and the table is 65 bytes,
`blk+4h..blk+44h`**, with a bypass byte at `blk+45h` (`009D6B4F CMP byte ptr [EBX+41h],0`). The
`0x40` at `009D6CCD` is the top index, which is what fixes the count at 65.

```
if (this+41h)   -> clamp into [low, high] and return            ; 009D6B5D -> 009D6CF2
if (!(low<high))-> *value = (low+high)*0.5, return              ; 009D6B63 JC, 009D6B67..009D6B7A
mid = clamp(index(*value), index(low), index(high))             ; 009D6BC4..009D6BD4
if (table[mid] == 0) return with *value untouched               ; 009D6BDF -> 009D6D48
scan down to index(low) and up to index(high) for the cheapest byte,
  each side stopping early at a zero, each remembering the nearest bin
  whose byte is strictly worse than table[mid] (the "wall")      ; 009D6BE5..009D6C77
pick: the cheaper side; on a tie the nearer bin, tie going up;   ; 009D6C79..009D6CAF
      if neither side improved, the wall with more room          ; 009D6CB1..009D6CDC
*value = bin * 0.0625 - 2.0, then clamp into [low, high]         ; 009D6CE0, 009D6CF2
```

Nothing this packet read writes those 65 bytes; both readers only consume them. That is the first
follow-up.

## 5. `009EC7C0`, the throttle ceiling

`float10 __thiscall(blk)(float heading_error, float danger, float cap)`, `RET 0Ch`, body
`009EC7C0-009ECA1A`, complete. Five `00419010` stages over `00424C40`, and
`docs/GAMEPLAY_SETTINGS.md` already carries the Lua names:

| offset | Lua key | default |
| --- | --- | --- |
| `+6CCh` | `Navigator.AutoThrust.HdgDiffValueMin_Slow` | DEG(25) |
| `+6D0h` | `Navigator.AutoThrust.HdgDiffValueMax_Slow` | DEG(75) |
| `+6D4h` | `Navigator.AutoThrust.ThrustMin_Slow` | 0.5 |
| `+6E0h` | `Navigator.AutoThrust.HdgDiffValueMin_Fast` | DEG(45) |
| `+6E4h` | `Navigator.AutoThrust.HdgDiffValueMax_Fast` | DEG(90) |
| `+6E8h` | `Navigator.AutoThrust.ThrustMin_Fast` | 0.75 |
| `+6ECh` | `Navigator.AutoThrust.HdgDiffDangerMul` | 6.0 |

```
error = heading_error * 00419010(0, 1, 1, HdgDiffDangerMul, danger)   ; 009EC7ED, 009EC7F2
m     = |error|                                                        ; 009EC7FA
slow  = 00419010(Min_Slow, 1, Max_Slow, 0, m)                          ; 009EC859
fast  = 00419010(Min_Fast, 1, Max_Fast, 0, m)                          ; 009EC8C1
a     = 00419010(1, cap, 0, ThrustMin_Slow, slow)                      ; 009EC8F7
b     = 00419010(1, cap, 0, ThrustMin_Fast, fast)                      ; 009EC92D
live  = |unit+980h|                                                    ; 009EC93C, 009EC94D
v     = (a > live) ? a : ((b < live) ? b : live)                       ; 009EC962..009EC98A
if ([unit+73Ch]+28h >= 0) v = min(v, [unit+73Ch]+24h / blk+3C4h)       ; 009EC98A..009EC9C7
if (!blk+364h) return max(-v, -0.625f)                                 ; 009EC9CD..009EC9F9
return v
```

So the AI's throttle falls off with heading error on two curves at once, the danger level widens
the error by up to six, the live ring throttle `unit+980h` (`ring+148h`,
`docs/CRUISE_COMMAND.md`) decides which curve binds, the cruise setting is a hard cap, and
**astern power is limited to 62.5%** (`00D21A78` and the double `00D21A80`). `blk+3C4h` is the
cached reference speed: the same division `009E12BD` does with `0080FC30(unit)`.

## 6. `009F4DA0` writes `blk+344h` and `blk+348h`

`009F4DA0`'s `this` is the brain and `blk = brain+8h` (`009F4DAD MOV ESI,ECX`, `009F50C0 LEA
ECX,[ESI+8]`). So the two fields the earlier packets recorded as "brain+34Ch and brain+350h, a
throttle ceiling no packet has reconstructed" are:

| 009F4DA0 writes | is | who reads it |
| --- | --- | --- |
| `brain+34Ch` | `blk+344h` | `009F4406`, the cap handed to `009EC7C0`; `009F4448..009F4482`, the throttle magnitude limit |
| `brain+350h` | `blk+348h` | `009F4525..009F4544`, the rudder magnitude limit |

`009F4DBC` stores `1.0f` into `blk+348h` unconditionally, before the `blk+B30h` early out at
`009F4DC1`, so a skipped ceiling step still leaves full rudder authority. The confirmation that the
decompiler's `param_1` offsets are brain-relative is in the same body: it tests `param_1+364h`
against `1` and writes `param_1+374h` as a byte, and those are `blk+35Ch`, the int latch, and
`blk+36Ch`, the byte, exactly as `009F3F80` uses them.

The rudder limit is applied only under a gate:

```
if (hull_with_latch && committed_with_latch && blk+33Ch < 0.0f)        ; 009F4506..009F451B
    00415690(&blk+1D4h, &(0.0f - blk+348h), &blk+348h)                 ; 009F451D..009F4544
```

`blk+33Ch` is one of the fields `009F4D10` publishes (`009F4D2B`); its producer was not read.

## 7. The obstacle sectors

### The array

**One array, not two tables.** Twelve `2Ch`-byte sectors based at `blk+808h`:

| record offset | blk offset for sector 0 | what | evidence |
| --- | --- | --- | --- |
| `+0h` | `blk+808h` | kind byte; three branches turn on it | `009EB6B4`, `009EB9B3`, `009EBF20` |
| `+4h` | `blk+80Ch` | half width of the probe | `009EB69F`, `009EB979`, `009EBE6B` |
| `+8h` | `blk+810h` | braking distance, written by `009EF230` | `009EF32F` writes, `009EB783` reads |
| `+0Ch` | `blk+814h` | reach | `009EB7B1`, `009EB862` |
| `+10h` | `blk+818h` | lateral offset | `009EB6B7`, `009EB9BC` |
| `+14h` | `blk+81Ch` | **blocked** | `009EB6A3` clears, `009EBECC` and `009EBFEF` set, `009F45B3` reads |
| `+18h`, `+1Ch` | `blk+820h`, `+824h` | the hit point | `009EBF6E`, `009EBF7A` |
| `+20h` | `blk+828h` | the avoidance bearing | `009EC0DE` and `009EC1A3` write, `009F4653` reads |
| `+24h` | `blk+82Ch` | the blocking neighbour, or null | `009EB6A7` clears, `009EBEDB` writes, `009F4662` reads |
| `+28h` | `blk+830h` | the passing side, 1 or 2 | `009EBF83` writes, `009F4669` reads |

`blk+848h` is therefore `sector[i+1]+14h`, the same byte of the **next** sector, one stride on
(`0x848 - 0x81C = 0x2C`). That is what makes the nudge at `009F4632..009F464E` meaningful: the arm
is entered when either of two adjacent sectors is blocked and then picks which of the two to steer
around. Two independent twelve-entry arrays at `blk+81Ch` and `blk+848h` would overlap each other
almost completely, so they cannot be two arrays.

### The producer

`009EF230` (`BSP_ShipAi_RefreshObstacleSectors`, body `009EF230-009EF347`, chain slot at
`009F51FA`) writes each sector's braking distance and calls `009EB660` on it:

```
v = max(0092D730(unit), [[unit+538h]+500h] * 0.1)                    ; 009EF247..009EF283
sector+8h = ((v+3.0)/[[unit+538h]+508h]) * (v+3.0) * 0.55
          + unit+9C8h * 0.6                                          ; 009EF289..009EF2C9
```

with the doubles `00D7A3A0` = 0.1, `00D7A2B0` = 3.0, `00CEC8F0` = 0.55, `00CEFF98` = 0.6 and
`class+508h` the deceleration `docs/SHIP_AI_STATES.md` already divides by at `009ED8EC`. The
sector pointer is `009EF31C LEA EDI,[EDX + EBX + 808h]` and the store is `009EF32F MOVSS
[EDI+8h],XMM0`. The schedule is a round robin on `blk+0A18h`, a 0..3 counter (`009EF2BD`, cycled
at `009EF2E9..009EF2FD`): the loop starts at sector `(counter & 1) + (counter >= 2 ? 6 : 0)`
(`009EF2CE SETGE`, `009EF308`) and steps `0x58`, two strides (`009EF339`), three times
(`009EF312..009EF323`), so **the twelve sectors refresh over four frames, three a frame**.

`009EB660` (`BSP_ShipAi_ScanObstacleSector`, body `009EB660-009EC277`, read **partially**) takes
the sector in ECX and blk on the stack (`009EB66C`, `009EB693`). It clears `+14h` and `+24h`
(`009EB6A3`, `009EB6A7`), walks the neighbour list at `blk+608h` (count `blk+604h`), and on a unit
hit stores the point (`009EBF6E`, `009EBF7A`), the neighbour pointer (`009EBEDB`) and the agreed
passing side (`009EBF83`); a terrain hit writes the bearing directly (`009EC0DE`) and leaves
`+24h` null. The tail re-tests `+14h` at `009EC1A8`, measures the distance to the hit point and
biases the bearing away from the obstacle by an interpolated angle that shrinks with distance
(`009EC20B..009EC237`, the constants `00CE38B8`, `00CE3808`, `00D20A18`), storing it back at
`009EC1A3`. The swept-arc geometry between the entry and `009EBEC0` was not read.

### The bucket and the arm

```
bucket = 0 if rudder > 0.65 else 1 if > 0.25 else 2 if > -0.25 else 3 if > -0.65 else 4
                                                        ; 00D07FC4, 00CE3868, 00CF00A8, 00D21B34
if (!speed_above_astern_floor) bucket = 4 - bucket       ; 009F4594
group  = speed_above_astern_floor ? (speed_below_ahead_floor ? (blk+364h ? 0 : 1) : 0) : 1
                                                        ; 009F44B3..009F44DC
index  = group * 6 + bucket                              ; 009F45A8..009F45B0
```

and the arm runs when `sector[index].blocked || sector[index+1].blocked` (`009F45B3`, `009F45C0`).
Because the bucket is 0..4 and the group is six wide, `index+1` never leaves the array.

Inside the arm, in order: `blk+354h` is raised to `3.0f` (`009F45DE`), `unit+1034h` and
`unit+102Ch` are raised to `1.5f` (`009F4606`, `009F462A`), the index is nudged by one when the
*other* of the two adjacent sectors is the blocked one (`009F4632..009F464E`), and the rudder
becomes `009DA250(00438B10(sector+20h, reference))` (`009F4681`, `009F4694`).

Then the throttle, and this is the whole back-off:

```
if (blk+35Ch == 0) {                                                   ; 009F4699
    if (making way ahead)  00415690(&blk+1D0h, -1.0f, 0.0f);            ; 009F46E0
    if (making way astern) 00415690(&blk+1D0h,  0.0f, 1.0f);            ; 009F471B
} else {
    if (hull_with_latch && committed_with_latch && sector+24h && ![sector+24h]+14h == 0
        && ![[sector+24h]+14h]+5Eh) {
        if ([sector+24h]+88h != sector+28h)            -> back off       ; 009F475B
        else if ([[sector+24h]+14h]+184h) {                              ; 009F4768
            closing = 009D8B90(sector+24h)(&blk+184h, sector+28h)        ; 009F477E
            if (closing > blk+3C4h * 0.75) -> back off                   ; 009F4791..009F479D
        }
    }
    back off:                                                            ; 009F479F
        blk+380h = 1.0f
        other = |0092D730([[sector+24h]+14h]+1018h)|                      ; 009F47B8
        if (other < 2.0f) blk+384h += 2.0f - other                        ; 009F47DE..009F47EE
    if (blk+380h > 0.0f) 009D6B40(blk+4h)(&blk+1D0h, -1.0f, 0.0f);        ; 009F47FC, 009F4878
    else {
        ceil = 009EC7C0(sector bearing error, 1.0f, 1.0f)                 ; 009F4836
        blk+1D0h = ceil
        009D6B40(blk+4h)(&blk+1D0h, ceil >= 0 ? -0.5f : ceil,
                                    ceil >= 0 ? ceil  : +0.5f)            ; 009F4855..009F4878
    }
}
```

The first test is the interesting one. `[sector+24h]+88h` is the passing side the **other** ship
picked and `sector+28h` is the one this ship picked, both written by the same `009D84E0` side flag
in `009EB660`. **When the two ships have not agreed on a side, this one backs off immediately**,
with no speed test at all. Only when they agree does the closing-speed test at `009D8B90` decide.

`009D8B90` (`BSP_ShipAi_NeighbourClosingSpeed`, `float10 __thiscall(neighbour)(const float* xz,
int side)`, `RET 8`, body `009D8B90-009D8C51`, complete) returns the neighbour's own forward speed,
its sign flipped when the dot product of the offset with the neighbour's forward vector disagrees
with the side code:

```
if (!this+14h || [this+14h]+5Eh || this+68h) return 0.0f;               ; 009D8B96..009D8BB5
v   = 0092D730([[this+14h]+1018h])                                      ; 009D8BC1
dot = (x - this+44h) * this+54h + (z - this+48h) * this+58h             ; 009D8BDC..009D8C01
if (dot > 0)  return (side == 1) ? -v : v;                              ; 009D8C0F, 009D8C11
else          return (side == 2) ? -v : v;                              ; 009D8C23
```

## 8. The escape manoeuvre, `009F487D..009F4B98`

Entered when `blk+35Ch != 0` and either the astern latch is not armed (`blk+380h < 0.0f`) or the
stall accumulator has passed `10.0f` (`009F4880..009F48A9`). It first asks whether all four inner
sectors of each group are blocked, by walking `blk+848h` and `blk+950h` four times with stride
`0x2C` — sectors 1..4 and 7..10 (`009F48B5..009F48D8`).

If the group the hull is moving in is fully blocked it zeroes the throttle on that side and, once
the hull has stopped, raises request 1. Three more requests come from `009F4977..009F4A57`
(2: the committed direction disagrees with the latch and the other group is clear; 3: the hull is
stopped and the stall has passed ten; 4: `blk+370h` is 1 or 3, or the stall has passed ten).
**Only whether the request is non-zero is ever read again.**

The flip itself:

```
raise unit+1034h to 0.5f                                                ; 009F4AB4
if (blk+364h) {                          ; committed ahead
    if (blk+1D0h > 0) blk+1D0h = 0;                                      ; 009F4AD2
    if (speed < 0.4) { if (blk+368h < 0) { blk+384h = 0; blk+364h = 0;
                                           blk+374h = -1.0f; blk+368h = 1.0f; }
                       blk+378h = 1; }                                   ; 009F4ADA..009F4B26
} else {                                 ; committed astern
    if (blk+1D0h < 0) blk+1D0h = 0;                                      ; 009F4B3B
    if (speed > -0.4) { if (blk+368h < 0) { ... blk+364h = 1; ... }
                        blk+378h = 2; }                                  ; 009F4B43..009F4B8B
}
```

So the manoeuvre kills the throttle in the direction it is abandoning, waits for the hull to
actually stop moving that way, and then, if the dwell timer `blk+368h` has run out, commits to the
other direction and records which flip it made in `blk+378h`. `blk+378h` is what stops it flipping
twice for the same block: with no request, the gate at `009F4A7D` runs the flip only when the latch
does not already record the current direction.

## Host methods the executable must implement, in call order

| # | method | native call site | callee | note |
| --- | --- | --- | --- | --- |
| 1 | `raise_turn_assist_load_102c` | `009F438C` | inlined `009D4FB0` | `min(2*danger, 1.5)`, only above `0.01` |
| 2 | `rudder_law_009da250` | `009F44F7` | `009DA250` | the goal heading error; gated on `blk+1C4h != Rudder` |
| 3 | `raise_secondary_load_1034` | `009F4606` | inlined `009D4FE0` | `1.5f`, blocked sector |
| 4 | `raise_turn_assist_load_102c` | `009F462A` | inlined `009D4FB0` | `1.5f`, blocked sector |
| 5 | `rudder_law_009da250` | `009F4694` | `009DA250` | the sector's bearing error |
| 6 | `neighbour_body_axis_speed_0092d730` | `009D8BC1` | `0092D730` | inside `009D8B90`, from the `009F477E` site |
| 7 | `neighbour_body_axis_speed_0092d730` | `009F47B8` | `0092D730` | the same value again, for `blk+384h` |
| 8 | `raise_secondary_load_1034` | `009F4912` | inlined `009D4FE0` | `1.5f`, all sectors blocked |
| 9 | `raise_turn_assist_load_102c` | `009F4A51` | inlined `009D4FB0` | `0.5f`, escape request 4 |
| 10 | `raise_secondary_load_1034` | `009F4AB4` | inlined `009D4FE0` | `0.5f`, the flip |

The two load latches have no `CALL`: `009D4FB0` and `009D4FE0` are inlined here, so each row is a
compare and a store and the host method has to carry the raise
(`if (value > latch) latch = value;`). Everything else the middle calls is already reconstructed
and is used directly: `00419010`, `0042AC60`, `00438AA0`, `00438B10`, `00415690`, and `00415550`
as a plain maximum.

## Coverage

| routine | coverage |
| --- | --- |
| `009F3F80` | partial: `009F40CA..009F4B98` projected operation for operation. `009F3F80-009F40C6` and `009F4B99-009F4D04` belong to packet `cc_ai_throttle_ring` and are untouched. No ledger record: the address is leased to `agent/cc-exe-2o` |
| `009D6B40` | complete |
| `009D8B90` | complete |
| `009EC7C0` | complete |
| `009F4DA0` | partial: what it writes and who reads it. `009F4DC7..009F50BA`, the arms that compute the two limits, are not projected. Leased to `agent/cc-exe-2o`: no ledger record |
| `009EF230` | read, not projected: the schedule and the braking distance are documented |
| `009EB660` | partial read, not projected: everything between `009EB6B7` and `009EBECC`, the swept-arc geometry, was not read |
| `009EF910` | not read |

`src/ship_ai_obstacle_tables.cpp` builds clean under `/W4 /WX` for Win32 and the existing
`reconstructed_math` test still passes. No new test case was added: every routine here is a
projection whose evidence is the listing, and the repository already covers the five math helpers
it calls.

## Run-time evidence

**None, and none is available yet.** `docs/GAME_EXECUTABLE.md` milestone 2o records
`009F40CA..009F44E3` and `009F4502..009F4B98` as one host record in `bsp_game.exe`, and the
executable never runs `009EF230`, so no sector is ever blocked there and the obstacle arms cannot
be reached. Closing that needs both this projection and the sector producer wired, which is a
milestone, not this packet. `src/ship_motion_probe.cpp` is owned by another packet this turn and
was not touched.

## Evidence for 009F3F80 (apply later)

`009F3F80` is leased to `agent/cc-exe-2o`, so nothing was written to its ledger record. For whoever
holds it next:

- Body `009F3F80-009F4D06`. The middle, `009F40CA..009F4B98`, is projected by
  `bsp::ship_ai_drive_order_ring_middle_009f40ca` in `src/ship_ai_obstacle_tables.cpp`, status
  `semantic_reconstruction_strict_win32_build_passed`, `abi_compatible: false`, coverage
  `partial: the head and the tail belong to packet cc_ai_throttle_ring`, doc
  `docs/SHIP_AI_OBSTACLE_TABLES.md`.
- The name `BSP_ShipAi_DriveOrderRing` still fits: this middle only decides the pair the tail hops
  into the ring.
- The six remaining writers of `blk+1D0h` the earlier doc listed are now read: `009F4462` and
  `009F4482` are the `blk+344h` limit, `009F492B` and `009F4951` are the all-sectors-blocked arm,
  and `009F4AD2` and `009F4B3B` are the escape flip. All six only ever clamp toward zero or toward
  `blk+344h`; none of them produces a new value.
- `blk+384h` is a float, not an int (`009F47EE FSTP`, `009F489E COMISS` against `10.0f`).

## Corrections

| what said it | what is true | evidence |
| --- | --- | --- |
| `docs/SHIP_AI_THROTTLE_TO_RING.md` uncertainty 3: "the two `2Ch`-stride tables at `blk+81Ch` and `blk+848h`, indexed by `((direction * 6) + bucket) * 2Ch`" | One array of twelve `2Ch`-byte sectors based at `blk+808h`. `blk+81Ch` is `sector[i]+14h` and `blk+848h` is `sector[i+1]+14h`, the same byte of the next sector | `009F45B3` with `EAX = index*2Ch` against `009F45C0` with `EDI = EAX+ESI`; `009EF230` at `009EF30E` computes `index*2Ch + 808h` and writes `+8h` of it; `009EB660` clears `this+14h` and `this+24h` of the same record |
| the same uncertainty: "their producer was not read" | `009EB660`, once per sector from `009EF230`, the chain slot at `009F51FA`; three sectors a frame on `blk+0A18h` | `009EF334`, `009F51FA`, `009EF2BD`, the `0x58` step at `009EF33C` |
| the same doc's follow-up `ship_ai_throttle_ceiling`: "what `009F4DA0` computes into `brain+34Ch` / `+350h`" | `brain+34Ch` is `blk+344h`, the throttle magnitude limit and the cap handed to `009EC7C0`; `brain+350h` is `blk+348h`, the rudder magnitude limit. Both are read in the middle of `009F3F80` | `009F4DAD` with `009F50C0 LEA ECX,[ESI+8]`; `009F4DB6`, `009F4DBC`; the consumers at `009F4406`, `009F4448`, `009F4525` |
| the same doc's uncertainty 4: "`blk+0A84h` ... whose `BSP_Math_InterpolateClamped` argument order the decompiler scrambles. It was not resolved from the listing" | Resolved. The one `00419010` call in that range is `InterpolateClamped(1.0, 1.0, 4.0, 0.0, blk+37Ch / unit+9CCh)`; the `0042AC60` calls take `(target, step)` with `this = &blk+0A84h`, step `2*dt` rising and `dt` falling | the five stores at `009F4181..009F419F`, `009F41BD`, `009F42B8`, `009F42BE` |
| the same doc's uncertainty 2: "`009D6B40`'s contract is unread past its first two arms ... the decompilation of that loop is not trustworthy" | Read from the listing. It is a nearest-cheapest-bin search over a 65-byte profile at `blk+4h`, with the value mapped to a bin by `(v + 2.0) * 16.0 + 0.5` truncated | `009D6B87`, `009D6B8F`, `009D6B99` with `00BF7435 CVTTSD2SI`; `009D6CE0`; the walk `009D6BE5..009D6CDC` |
| `include/bsp/ship_ai_states.hpp` `ShipAiControlBlock`: `int direction_counter_384` | `blk+384h` is a **float**, a stall accumulator compared against `10.0f`. This packet does not edit that header; `ShipAiObstacleState::stall_time_384` carries the right type and the duplicate is flagged in both | `009F3FE3 MOVSS`, `009F47EE FSTP`, `009F489E COMISS` against `00CE38B8`, `009F4AFF MOVSS` |
| the same header: `bool yaw_rate_subtracts_364; // +364h, picks add or subtract` | Refined, not contradicted. `blk+364h` is the committed direction: clear is astern, set is ahead | `009EC9CD` negates the ceiling when clear; `009F4441` caps at `+blk+344h` when set and floors at `-blk+344h` when clear; `009F44C3` picks the astern sector group when clear |

## Uncertainties

1. `blk+37Ch`, the clearance the danger ramp divides by `unit+9CCh`, is written only inside
   `009EF910` (six sites), which `009F4D10` calls. That routine was not read, so what the clearance
   measures is a hypothesis from its use.
2. `blk+33Ch`, the gate on the rudder limit, is published by `009F4D10` at `009F4D2B` and read at
   `009F4514`. Its producer was not read.
3. `blk+368h`, the escape dwell timer this routine sets to `1.0f`, is counted down somewhere else.
   `include/bsp/ship_ai_states.hpp` attributes the countdown to `009ED6B0`; that was not re-checked.
4. The sign convention of `009D8B90`'s result comes from the listing, not from a run. A result
   above `0.75 * blk+3C4h` arms the astern latch; whether that is a neighbour approaching or one
   pulling away was not confirmed dynamically.
5. `009EB660` was read partially. The sector fields `+0h`, `+4h`, `+0Ch` and `+10h` are named from
   how the unread geometry uses them and are the weakest names in this doc.
6. Nothing read here writes the 65 profile bytes at `blk+4h` or the bypass byte at `blk+45h`.

## Follow-up packets

| packet | addresses and files | what is left |
| --- | --- | --- |
| `ship_ai_obstacle_sector_scan` | `009EB660` between `009EB6B7` and `009EBECC`, `009DD010`, `009D84E0`, `009DC2E0`, `009D80C0`, `009D8160`, `blk+604h`, `blk+608h` | The swept-arc query itself: what shape each of the twelve sectors is, how a neighbour is tested against it, and how `009D84E0` picks the passing side both ships have to agree on |
| `ship_ai_clearance_37c` | `009EF910`, `009F4D10`, `blk+37Ch`, `blk+33Ch` | The producer of the clearance the danger ramp reads and of the gate on the rudder limit |
| `ship_ai_throttle_ceiling_step` | `009F4DA0` `009F4DF7..009F50BB`, `0070D140`, `0070D0F0`, `0082E850`, `0070E3C0`, `00863780`, `0080FC30` | How `blk+344h` and `blk+348h` are actually computed. This packet settled only what they are and who reads them |
| `ship_ai_throttle_profile_producer` | `blk+4h..blk+45h`, `009D6B40`, `009E04E0` | Who fills the 65-bin cost profile. Both readers only consume it, and an all-zero profile makes `009D6B40` a no-op, so nothing in the chain reconstructed so far can bend the throttle |

## no_ghidra_function

| start | end (inclusive) | evidence |
| --- | --- | --- |
| none | | Every routine this packet read or named has a Ghidra function whose body range the bridge reports: `009F3F80-009F4D06`, `009D6B40-009D6D57`, `009D8B90-009D8C51`, `009EC7C0-009ECA1A`, `009EF230-009EF347`, `009EB660-009EC277`, `009F4DA0-009F50D2`, `009F50E0-009F5253`. `tools/verify_report_calls.py` checked all 23 call rows of `reports/ship_ai_obstacle_tables.json` against the live bodies and the call graph, and caught one wrong attribution before this doc was written (`009F50C6` is inside `009F4DA0`, not `009F50E0`) |

## Correction from docs/SHIP_AI_CLEARANCE_PROFILE.md

Packet `cc_ai_clearance_profile` (read `009EF910` and `009E04E0` whole) closes uncertainties 1, 2
and 6 above and corrects two readings: `blk+37Ch` is the turn **clearance**, not the danger
level. `009EF910`, its sole writer, rewrites it with the 9999.0f sentinel `00CE3D64` on a timer and
only ever lowers it, from the static avoid-zone segment tree and the neighbour footprints, measured
from the inside-of-turn shoulder point `009DE2F0` builds; `009F3F80` forms the danger `blk+0A84h`
from it at `009F416E`, so an open-sea ship reads danger 0.0 (the executable's 1.0 saturation was
its substitute record leaving `blk+37Ch` at zero). `blk+33Ch` is not computed by `009EF910`: it
is the constant -1.0f `009F4D10` stores at `009F4D27`, which `009EF350` can replace with a real
distance at `009EF876`; the rudder clamp at `009F451D` runs only while it is negative, so -1.0f
opens that gate. The 65 throttle cost bins at `blk+4h` are written only by `009D56F0` (through
its cost-1 wrapper `009D67F0`) from `009E04E0`, and the constructor `009E4330` sets the bypass byte
`blk+45h` to 1, so an untouched profile is switched off rather than merely empty.
