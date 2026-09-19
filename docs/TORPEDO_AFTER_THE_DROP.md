# After the drop: the five deaths are the five bombers, and the torpedoes hit nothing

Packet `cc8_torpedo_after_the_drop`, owner `agent/cc8-plane-squadron`. All three parts. Sections 3.1
and 3.2 are appended after Coverage and Uncertainty, which were written before the listing read; the
Coverage table below is amended by 3.1.

Run: `local/aimclass_after_usn01.log`, the same run `docs/TORPEDO_RELEASE_TIMER.md` validates.

## 1. Who died, and it is not a torpedo kill

**`deaths=5 kill_credits=5` are the five Mavs, shot down by the American ships.** The run's own
per-unit table says so by name:

| unit | side | taken | health | `sunk_at` | `killed_by` |
| --- | --- | --- | --- | --- | --- |
| Mav1 | 1 | 450 | 0 | 133.50 s | **Dunlap** |
| Mav2 | 1 | 450 | 0 | 136.95 s | **Northampton** |
| Mav3 | 1 | 450 | 0 | 109.15 s | **Northampton** |
| Mav4 | 1 | 450 | 0 | 134.25 s | **SaltLakeCity** |
| Mav5 | 1 | 450 | 0 | 109.80 s | **SaltLakeCity** |

**No ship sank.** Every other row in that table carries `sunk_at -1.00`, including the three ships
that did the killing and the `Enterprise`.

The damage total decomposes exactly, which is what makes this a reading rather than an inference:

```
five Mavs        5 x 450 = 2250.0
SaltLakeCity              347.0
                        --------
                         2597.0   against summary total_damage=2596.8
```

and `SaltLakeCity`'s 347 is in turn exactly the sum of what the side-1 **guns** dealt: `CB2` 127 +
`Coastal Gun 03` 79 + `Coastal Gun 02` 53 + `Coastal Gun 01` 88 = 347. **Not one point of the
mission's damage is attributable to a torpedo.** `hit_records=111` and
`projectiles ... entity_impacts=111` agree, and 111 is the gun-hit count.

So the correct sentence about this run is: *the five bombers reached their release point, dropped,
and were then shot down by the cruisers and the destroyer they had attacked.* Anyone quoting
`deaths=5 kill_credits=5` as a result of the torpedo work is quoting the AA gunners.

## 2. What the five torpedoes did

All five entered the water and swam:

```
gunnery: torpedo drop 1 by Mav3 at 12 m, speed 73.5 m/s, bullet 69, swim 30.9 m/s
gunnery: torpedo drop 2 by Mav2 at 12 m, speed 73.6 m/s, bullet 69, swim 30.9 m/s
gunnery: torpedo drop 3 by Mav5 at 12 m, speed 73.7 m/s, bullet 69, swim 30.9 m/s
gunnery: torpedo drop 4 by Mav4 at 12 m, speed 73.7 m/s, bullet 69, swim 30.9 m/s
summary mission gunnery torpedo_drop drops=5 refusals=0 water_entry_breakups=0
summary mission gunnery ... swims_started=5 snaps=0
```

| quantity | value | source |
| --- | --- | --- |
| release altitude | 12.0 to 12.1 m | `approach+78h` `TorpReleaseAlt`, `SPNormal` |
| release speed | 72.9 to 73.7 m/s | against `MaxWaterHitVel` 100, hence 0 breakups |
| swim speed | 30.9 m/s | the round's `+470h`, `WaterTravelSpeed * 0.5999994277954102` at `0085786D` |
| release time | about 78 to 88 s | drops 1 and 2 bracket `gunnery step 1600 t=80.00` |
| release range | about 700 m | the aim census reads 1084 m at aim tick 151 and the timer fired at aim tick 253, 102 ticks and about 372 m later |
| aircraft death | 109 to 137 s | 21 to 49 s **after** their own drop |

The host's swim is not a stub: `src/game_hosts_gunnery.cpp` levels the round onto the surface plane
at the swim speed, keeps the launch heading, disables gravity through the flight state's own
`classDesc+20h` flag rather than by stepping the round outside `projectile_flight_step`, and lets it
continue through the same per-step entity sweep as any other round. A swimming torpedo in this host
therefore **can** hit. These five did not: `entity_impacts=111` are all gun rounds, and no damage
record attributes to a torpedo.

### Why they missed is not yet established, and here is the bound on it

What is measured: released about 700 m out, swimming 30.9 m/s, so about **23 seconds** of run. What
is not read: whether anything in the chain **leads** a moving target over those 23 seconds.

* `009D1360` writes `approach+A0h`, the fall lead `fallTime * v0` - the distance the torpedo carries
  forward while it falls - and `approach+98h`, the total run time to impact. `009D3D12` reads `+98h`
  as the bias of the engagement estimate at `+F8h`. Whether either reaches the commanded **heading**
  is unread.
* The aim tick's commanded heading is `approach+94h`, the bearing to the plan target at
  `approach+ACh`/`+B0h`, plus the sector turn offset `approach+5Ch`. Whether `+ACh`/`+B0h` is a lead
  point or the target's current position is the question, and it is `009D39xx`'s (the replan) rather
  than this packet's.
* The three ships were under way: `Northampton` shows `nearest 18` and `SaltLakeCity` `nearest 17`,
  so the geometry closed to metres. A straight-running torpedo aimed at where a ship **was** 23
  seconds earlier misses by roughly the ship's speed times 23 s.

**Do not read the miss as a defect in the release chain.** The release chain did what the listing
says: correct altitude, correct speed, no breakup, a swim at the authored speed. The next question
is an aiming question, one state earlier.

## 3. The climb-away `009D0F10`, read whole

The five aircraft still touch the water, but 21 to 49 s after dropping, at 69.6 m/s and level
(`alt` -0.02 to -0.10), and the table says they were **already dead** by then or dying: `sunk_at`
109.15 to 136.95 s against water contacts in the same window. So the water contact after the drop is
now entangled with being shot down, and separating the two needs the climb-away bound first.

`009D0F10 BSP_BotStateTorpedoGoAway_Tick` is the state that flies it: three **direct** `009FB800`
calls at `009D109C`, `009D10FF` and `009D1194` - not through `009FBA50` - each with a literal `1.0`
as the second argument (`009D107A FLD1` / `009D1084 SUB ESP,8` / `009D1087 FSTP [ESP+4]` covers the
first two, `009D1158 FLD1` the third), capping the climb-out at `class+1ECh * 1.0` = 0.1854 rad for
the Mav. This host runs no goaway tick at all.

It is worth doing next because it exercises the **climb** arm of `009FB800`, which nothing in this
stream has driven: every measurement so far has been of the dive arm. `class+1ECh` is derived rather
than authored (`007C4C08`-`007C4C14`, 0.6 times `desc+1E4h`), so the climb arm also tests a field
this lineage has only ever read.

The listing is transcribed in section 3.1 below.

## Coverage

| question | coverage |
| --- | --- |
| who died and what killed them | complete, from the run's per-unit table and an exact damage decomposition |
| what the torpedoes did to water entry and the start of the swim | complete |
| what ended each torpedo | **partial**: established that none hit and that the host's swim can hit; which of range expiry or geometry ended them is not separated |
| why they missed | not established; bounded above to an aiming question in the replan or `009D1360`'s lead, not the release chain |
| `009D0F10` | **amended by section 3.1 below: transcribed complete**, both returns, every constant width-checked. The reconstruction and the host binding are not written; 3.2 says what a binder still needs |

## Uncertainty

* The release range of about 700 m is interpolated from the aim census at tick 151 and the timer's
  first fire at tick 253, not read from a census printed at the release instant. The release census
  prints altitude and speed but not range; adding range to that line is the cheapest way to make
  this exact.
* `projectiles ... expired=885` is a whole-mission figure over 1119 rounds; the five torpedoes are
  not separated out of it, so "they expired" is a plausible end rather than a measured one.

### 3.1 `009D0F10` transcribed, with both of its returns

**`docs/TORPEDO_GOAWAY_RELEASE.md` gives the body as `009D0F10`-`009D1153`. That is wrong, and it
is wrong in the way that matters: `009D1153` is the **first** `RET 4`, not the end.** Ghidra's live
body is `009D0F10`-`009D1210`, `009D1032 JBE` jumps over that return to a second arm at `009D1156`
with its own two returns at `009D11ED` and `009D120E`, and one of the three `009FB800` call sites -
`009D1194` - lives in it. Checked with `bsp.py ghidra proto` on each call site rather than by
scrolling: all four answer body `009d0f10 - 009d1210`.

`void __thiscall(BotStateTorpedoGoAway* this, float dt)`, `RET 4`. `approach = this+4h`,
`unit = approach+4h`, `cmd = approach+18h`, `class = approach+8h`, `row = approach+14h` (the
`PilotBotParameters` difficulty row).

```
high      = unitY > state+20h                                   009D0F2F .. 009D0F42  (JBE -> 0)
009D0C10(this)                                                  009D0F46, the heading update
if (approach+90h < state+24h - 100.0)   state+28h -= dt         009D0F56 [00D7A220], 009D0F64 JBE
state+30h += dt                                                 009D0F6E, 009D0F81 FST
if (approach+134h < 1.0 && state+30h > state+34h + 6.0)
        state+28h = -1.0                                        009D0F84 COMISS, 009D0F9C, 009D0FA6
if (state+28h < 0.0 && high) {                                  009D0FB2 COMISS, 009D0FBA JZ
    state+34h = jitter(3.0, 6.0)                                009D0FD7 -> 009D0FDE
    state+28h = jitter(row+10h, row+14h) + state+34h            009D1000 -> 009D101A
    state+30h = -(state+34h * 0.5)                              009D101D, 009D1023 FCHS, 009D1025
}
if (state+30h < state+34h) {            the manoeuvre window    009D1032 JBE -> the else arm
    cmd+278h = 1.0; cmd+27Ch = 1; cmd+2A8h = 0.0; cmd+2ACh = 1; cmd+2D8h = 0
    if (unitY > 20.0) {                                         009D108B COMISS [00CE3930]
        009FB800(state+1Ch, 1.0)                                009D109C, arg1 from 009D107A FLD1
        cmd+2C4h = clamp(2 * state+2Ch * state+30h, -1.2, 1.2)  009D10A7 FADD ST0,ST0; 009D10B1
        cmd+2CCh = 1                                            009D10EA
    } else {
        009FB800(1000.0, 1.0)                                   009D10FF, altitude [00CE3804]
        cmd+2C0h = state+18h;  cmd+2CCh = 2                     009D110C, 009D1112
    }
    (approach+1Ch)+40h = tuning+674h                            009D111C .. 009D112C
    009FABE0(approach+1Ch, state+18h, class+1ECh)               009D114A
    RET                                                         009D1153
} else {                                the window is over      009D1156
    cmd+278h = 1.0; cmd+27Ch = 1; cmd+2A8h = 0.0; cmd+2ACh = 1; cmd+2D8h = 0
    009FB800(state+1Ch, 1.0)                                    009D1194, arg1 from 009D1158 FLD1
    if (high) {
        cmd+2C0h = state+18h;  cmd+2CCh = 2                     009D11A5, 009D11AB
        (approach+1Ch)+40h = tuning+670h                        009D11B5 .. 009D11C5
        009FABE0(approach+1Ch, state+18h, 0099B630(cmd))        009D11D0, 009D11E4
        RET                                                     009D11ED
    } else {
        cmd+2C4h = 0.0;  cmd+2CCh = 1                           009D11FB, 009D1203
        RET                                                     009D120E
    }
}
```

| constant | address | width at its load | value |
| --- | --- | --- | --- |
| the break-off slack | `00D7A220` | `FSUB double ptr`, `009D0F56` | 100.0 |
| the re-seed guard | `00CE6628` | `FADD double ptr`, `009D0F90` | 6.0 |
| the window jitter high | `00CE6630` | `FLD float ptr`, `009D0FBC` | 6.0 |
| the window jitter low | `00CE3854` | `FLD float ptr`, `009D0FCE` | 3.0 |
| the half | `00D7A280` | `FMUL double ptr`, `009D101D` | 0.5 |
| the low-altitude split | `00CE3930` | `COMISS`, `009D108B` | **20.0 m** |
| the climb-out altitude | `00CE3804` | `FLD float ptr`, `009D10F6` | **1000.0 m** |
| the roll clamp | `00D05EA4` / `00CE3814` | `FLD float ptr` / `MOVSS` | -1.2 / +1.2 |
| `009FB800`'s reference, all three sites | `FLD1` | - | **1.0** |

Every one checked at both widths against the width of the instruction that loads it, the habit that
found `kPitchClampLo`.

**So the climb-away is: below 20 metres, command altitude 1000 m and fly the heading `state+18h` in
heading mode 2; above 20 metres, command `state+1Ch` and roll.** The aircraft that ditch at 69.6 m/s
after their drop are below 20 m with nothing running this arm, so nothing ever tells them to climb
to 1000. `009FB800`'s reference is the literal `1.0` at all three sites, which caps the climb at
`class+1ECh * 1.0` = 0.1854 rad for the Mav - the fourth independent confirmation that the second
argument is a dimensionless scale.

### 3.2 What is still needed, and it is code rather than reading

The listing is complete above. What is not done is the reconstruction and the host binding:
`src/torpedo_goaway_tick.cpp` and its header, then `run_goaway_tick_009d0f10` beside
`run_torpedo_aim_tick_009d15f0` in `src/game_hosts_units.cpp`'s state switch, which currently
handles only `kAim`, `kMoveTo`/`kFollow` and `kAttackRun`.

Two things a binder must not take on trust:

* `009D0C10` at `009D0F46` is called before everything and is **unread** here; it is the goaway's
  own heading update and it is what fills `state+18h`, the heading both arms command. Binding the
  tick without it commands a heading that nothing writes.
* `state+1Ch`, `state+2Ch`, `state+20h` and `state+24h` come from the goaway **enter** `009D0D90`,
  which `docs/TORPEDO_GOAWAY_RELEASE.md` section (2) reads for `+24h` and `+28h` only. `+1Ch` is the
  altitude both `009D109C` and `009D1194` command and it has no producer in this document.

### 3.3 Correction to 3.2, made before anyone acted on it: `state+1Ch` does have a producer

Section 3.2 says "`+1Ch` is the altitude both `009D109C` and `009D1194` command and it has no
producer in this document". That is **wrong**, and the mistake was mine: I read
`docs/TORPEDO_GOAWAY_RELEASE.md`'s section (2) summary line, which says `009D0D90` is read "for its
use of `+24h` and `+28h`", and did not read that document's own correction from packet
`cc8_torpedo_goaway_release`, which extends the enter past `009D0E3A` to `009D0F04` and reads the
rest of it.

* **was**: `state+1Ch` has no producer; a binder cannot command the altitude the tick asks for.
* **is**: `009D0D90 BSP_BotStateTorpedoGoAway_Enter` (body `009D0D90`-`009D0F04`) writes every field
  the tick reads except `+24h`'s companion `+2Ch`, which it also writes at its head:

| field | producer | value |
| --- | --- | --- |
| `state+2Ch` | `009D0D90` head | `+1.0` or `-1.0` (`00D7A24C` / `00D7A260`) on a parity of the global at `00F876B0`, so it is the **side** the break-off turns to |
| `state+24h` | `009D0E37` | `max(tuning+438h SafeDist, the target's extent from 007B5BE0)` times `UniformFloatRange(1.0, 1.15)` (`00D20CE4`). The only writer in the band |
| `state+1Ch` | `009D0E88` | `UniformFloatRange(50.0, 100.0)` (`00CEB4D4`, `00CE3D08`) **plus** `approach+78h + approach+74h` when `approach+132h` is set, else `ctl+394h` |
| `state+28h` | `009D0EAB` | `UniformFloatRange((approach+14h)->+10h, ->+14h)` - the same row pair the tick re-seeds from at `009D1000` |
| `state+30h`, `state+34h` | `009D0EB1`, `009D0EB6` | 0 |
| `state+20h` | `009D0EEB` | the altitude `high` is tested against |
* **evidence**: the plate comment on `009D0D90` in the live project, which carries that packet's
  extent correction verbatim, and `bsp.py ghidra proto` for the body.

**What this changes for a binder.** `state+1Ch` is the release altitude plus 50 to 100 metres - so
the climb-away above 20 m asks for a little above where the aircraft dropped, and the climb-away
below 20 m asks for 1000 m. Only **one** unread dependency remains, and it is the heading:

* `009D0C10 BSP_BotStateTorpedoGoAway_UpdateGeometry`, body `009D0C10`-`009D0D87`, called first at
  `009D0F46`. It is what fills `state+18h`, which both arms write to `cmd+2C0h`. Nothing else in the
  band writes it. **That is the one routine that has to be read before `009D0F10` can be bound
  faithfully**, and it is 375 bytes.

A binder that skipped it would command heading 0 on every goaway, which on this placement points the
aircraft north rather than away from the ships that are shooting at them - the kind of substitution
that reads as a behaviour bug forever. `009D0F10`'s own transcription in 3.1 needs no revision.

### 3.4 `009D0C10` read for its output, and the completion predicate cleared

Two checks stood between section 3.1's transcription and a binding. One is done and one is
half-done; both are recorded here so the binding packet does not repeat them.

**The completion predicate is clear, and the trap it could have been does not bite.**
`agent/cc8-dive-bomb` lost an 8800-frame run to a goaway whose completion rule was modelled from its
first condition only: the state got one tick, the binding was correct and looked broken. The torpedo
equivalent is `009D3150`, and `docs/TORPEDO_GOAWAY_RELEASE.md` section (3) reads it whole already -
`approach+90h > state+24h`, with an optional `* 0.4` arm that needs both `ctl+369h` and the global
at `00E17BF2` and that this host's `read_control_block` reports off. No altitude condition, unlike
the dive bomb's `009C7F00`. The run agrees: `enters=1 break_off_24h=700.0 range_peak_in_goaway=382.2
done_last=0` on every aircraft, so the state is entered once, never completes, and would tick for
the rest of the mission if anything ticked it. A binding here will not be cut off after one tick.

**`009D0C10 BSP_BotStateTorpedoGoAway_UpdateGeometry` produces `state+18h`, and its shape is:**

```
bearing   = 009FD570(vtable[0](&scratch, approach+4h, state+24h, state+2Ch, approach+90h, 0.8))
hdgErr    = SubtractWrappedAngle(bearing, unitHeading)          unitHeading via vtable[50h]
turn      = clamp(hdgErr, -0.5235988, +0.5235988)               00CEC728 / 00CEC724, -/+ DEG(30)
probe     = -p[0] * p[1] * p[2]        from 007F0280's triple, seeded 00CEB4B0, 00CEB4D4, 00D1A918
if (turn > 0 && probe < 0) || (turn < 0 && probe > 0)
          turn += probe * 0.6981317401                          00D20CC0, a double, DEG(40)
state+18h = AddWrappedAngle(unitHeading, turn)                  009D0C10's only store
```

So the break-off heading is the aircraft's **current** heading plus a turn of at most 30 degrees
toward the break-off bearing, pushed further by up to 40 degrees when the terrain probe disagrees
with the turn's sign. `state+2Ch`, the +/-1 side the enter draws, is an argument to the bearing
call, which is what makes the break-off turn left or right.

**What is still unread, and it is why this packet stops here rather than binding:**

* `009FD570` and the `vtable[0]` call before it, which together produce the bearing. Without them
  the turn is `clamp(-unitHeading, ...)` rather than a break-off.
* `007F0280`'s output triple at this call site. The same routine is called from the aim tick's
  sector probe and from both attack-run ticks, so its contract is shared and worth reading once for
  all of them rather than three times.

A binding that substituted either would command a heading that is not the image's, and the run would
report a climb-away that flies somewhere the native never goes. Given that this stream has spent the
night correcting exactly that class of error in its own and others' work - `pitch_scale_188 = 1.0f`,
`rangeLow = rangeHigh = 0`, `kPitchClampLo` - binding on two unread inputs is the wrong trade. The
listing above is the durable part; the code is one focused packet with `009FD570` and `007F0280`
read first.

**A blind confirmation of `kPitchClampLo`, recorded because it is independent of this stream.**
`agent/cc8-dive-bomb` built `tools/const_width_sweep.py` (on main as `616382183`) and added a
`--load-sites` mode that finds every instruction with an absolute `[disp32]` naming a declared
constant and reports the operand width. Run blind over all 697 constants in `include/bsp`, in a tree
that still carried `0.05625f`, it produced exactly **one** wrong-width finding: `kPitchClampLo`. It
was told nothing about this packet.

**A correction accepted, on `task+41Ch`.** This document's predecessor advised that
`in.speed_ratio_41c = 1.0f` "needs `task+41Ch`'s producer" and was worth a packet of reading. That
was wrong: `009F9D37 FLD [EAX+188h]` is `desc+188h MaxSpd` - the same field this stream bound as
`plane_max_spd` - divided by the reference `009C3EC4` loads from `[EAX+4D8h]`, taken `max(., 1.0)`
and stored to `approach+24h` at `009F9D61`. It is a stand-in with a known formula, so it is a
binding in its own window rather than a packet of reading.

### 3.5 `009D0C10` read whole from the listing, and what the binding actually costs

Body `009D0C10`-`009D0D87`, `void __fastcall(this)`, no stack argument, `SUB ESP,0x3C` + `PUSH ESI`,
balanced by `POP ESI` / `ADD ESP,0x3C` / `RET` at `009D0D83`-`009D0D87`. Section 3.4's shape is
confirmed and the two call setups are now exact rather than decompiler-shaped.

**The bearing call, from the pushes in stack order** (`009D0C26`-`009D0C4F`):

```
009d0c16  MOV ECX,[ESI+4]                  ; ECX = approach, the receiver
009d0c1f  MOV EAX,[ECX+4]                  ; EAX = unit
009d0c26  SUB ESP,8
009d0c29  FLD  [0x00ce74f8] -> [ESP+4]     ; 0.8
009d0c36  FLD  [ESP+0x14]   -> [ESP]       ; approach+90h, saved at 009D0C22
009d0c3d  PUSH EDX          (EDX = ESI+2Ch); &state+2Ch, the +/-1 SIDE, BY POINTER
009d0c3e  FLD  [ESI+0x24]   -> PUSH        ; state+24h, the break-off distance
009d0c45  PUSH EAX                          ; the unit
009d0c4e  PUSH EDX          (EDX = &scratch)
009d0c46  EAX = [[ECX]]                     ; approach->vtable[0]
009d0c4f  CALL EAX
009d0c51  PUSH EAX                          ; the returned point
009d0c52  MOV ECX,ESI                       ; this = the goaway state
009d0c54  CALL 009FD570                     ; -> the bearing, in ST0
```

so it is `009FD570(state, approach->vtable[0](&scratch, unit, state+24h, &state+2Ch, approach+90h,
0.8))`. The **side is passed by pointer**, which means `vtable[0]` may write it back - a detail a
by-value reconstruction would lose.

**The tail** (`009D0D62`-`009D0D87`) is `state+18h = AddWrappedAngle(unitVtable50(), turn)`.
`0074E260` is `FLD [ECX+0C6Ch] / RET`, so `vtable[50h]` takes nothing and cleans nothing; the two
`PUSH ECX` at `009D0D71` and `009D0D77` are building `AddWrappedAngle`'s two-argument frame around
it, which is why only one `SUB ESP,8` appears for two calls. The stack balances against the
prologue, which is the check that the reading is right.

**`007F0280` can be substituted safely, and this is the one piece of good news.** `009D0C96`-
`009D0CAA` zero the three out-slots with `XORPS`/`MOVSS` immediately before the call, and the probe
term is `-p[0] * p[1] * p[2]`. A no-hit answer leaves all three zero, so `probe` is `0`, and the
nudge at `009D0D50` needs `probe` strictly positive or negative on one side of a sign test. **An
inert probe therefore changes nothing**, which is exactly how the host already models the same
routine for the aim tick (`sector_probe_009d1a94` returning a default `TorpedoAimSectorProbe{}`).
On an open-sea placement that is also the right answer physically.

**`009FD570` cannot, and it is the whole cost of this packet.** Body `009FD570`-`009FDEDE`, 2414
bytes, **six callers** - `007B5AF0`, `009A3CF0`, `009AD480`, `009B5760`, `009C47D0` and this one -
and fourteen callees including `00413920 BSP_Matrix_Multiply4x4`, `004134F0 BSP_Matrix_Copy4x4X87`,
`00414DB0`, `00419010`, `00438AA0`, `00438B10`, `009FA510` and `009FCFD0`. It is a shared
fly-to-a-point solver, not a bearing helper, and the dive bomb (`009C47D0`) and depth charge
(`009A3CF0`) reach it too.

So the honest shape of the remaining work is **not** "bind `009D0F10`". It is:

1. read `009FD570` (2.4 KB, six callers) and the `vtable[0]` break-off-point routine it is fed;
2. then `009D0F10` and `009D0D90` reconstruct and bind in one short window, because everything else
   they need is already read: the tick in 3.1, the enter's draws in 3.3, the predicate in 3.4, and
   `007F0280` inert by the argument above.

Step 1 pays for six call sites rather than one, which is why it deserves its own packet rather than
being done hurriedly inside this one. Binding `009D0F10` with a substituted bearing would command a
heading that is not the image's, and the run would report a climb-away flying somewhere the native
never goes - the same failure `agent/cc8-dive-bomb` hit from a modelled-from-one-condition predicate,
reached by a different road.

## 4. The goaway is bound, and three of the five bombers now live

Packet `cc8_flyto_solver_and_goaway`. `009FD570` is read whole in `docs/TORPEDO_FLY_TO_SOLVER.md`,
which also retracts section 3.5's reading of `approach->vtable[0]`'s arguments. With the solver in
hand, `009D0C10`, the enter tail `009D0E3A`-`009D0F04` and `009D0F10` are reconstructed
(`include/bsp/torpedo_goaway_tick.hpp`, `src/torpedo_goaway_tick.cpp`) and bound in
`src/game_hosts_units.cpp`'s state switch, which until now fell through `kGoAway` entirely.

**The mode words the planner gates need.** `009FB800`'s chain writes `cmd+2BCh` and `cmd+2D0h = 2`,
and `cmd+2D0h` non-zero is what lets `0099E3BF` run the pitch arm at all; the binding publishes both.
`cmd+2CCh` is 2 on the two arms that command a heading and 1 on the two that do not, and mode 1 is
what lets the task's own `cmd+2C4h` roll through `0099E2xx`. The tick writes `cmd+278h = 1.0`,
`cmd+27Ch = 1`, `cmd+2A8h = 0.0`, `cmd+2ACh = 1` and `cmd+2D8h = 0` on every arm.

**One reading of the enter tail is worth stating on its own**: `009D0EBB`-`009D0EFB` computes
`state+20h` as **`min(approach+78h + approach+74h + 30, 50)`**, a MINIMUM. The `JBE` at `009D0EE1`
takes the computed value and falls through to the `50.0` at `00CEB4D4`, so the `high` test the tick
runs is never above 50 m. On USN01 the band is 12.0 m, so `state+20h = 42.0`.

### Run: `local/goaway_bound_usn01.log` against `local/aimclass_after_usn01.log`

Same binary, same command (`--frames 3200 --press-start-frame 30 --menu-select USN01
--mission-frames 3000 --mission-frame-seconds 0.05`), `query session` Active on the console.

| aircraft | goaway ticks | arms: win/low, win/high, post/high, post/low | heading ticks | `+1Ch` | `+20h` | world y range in goaway | fate |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Mav1 | 465 | 0, 106, 0, 359 | 0 | 62.0 | 42.0 | 6.9 to 50.2 | killed 134.9 s by Dunlap |
| Mav2 | 546 | 0, 170, 17, 359 | 17 | 62.0 | 42.0 | 8.1 to 56.3 | **alive, 450/450** |
| Mav3 | 551 | 0, 170, 23, 358 | 23 | 62.0 | 42.0 | 8.1 to 57.7 | **alive, 450/450** |
| Mav4 | 507 | 0, 150, 0, 357 | 0 | 62.0 | 42.0 | 9.4 to 49.7 | **alive, 175/450** |
| Mav5 | 517 | 0, 162, 0, 355 | 0 | 62.0 | 42.0 | 9.2 to 47.7 | killed 129.7 s by SaltLakeCity |

| quantity | before | after |
| --- | --- | --- |
| `deaths` / `kill_credits` | 5 / 5 | **2 / 2** |
| hits taken by the flight | 111 | 59 |
| damage taken | 2596.8 | 1501.9 |
| post-drop world y | to the water at 69.6 m/s | bottoms at 6.9 to 9.4 m, climbs to 47.7 to 57.7 |
| `goaway` enters | 1 | 82 |
| `range_peak_in_goaway` | 382.2 | 701.0 |
| `transitions` | 3 | 165 |
| `run_time_009D1360` | 0 | 112 |

So: **yes, the five aircraft climb away instead of touching the water, and three of the five survive
the AA.** The climb is the post-window arm's `009FB800(state+1Ch, 1.0)` with `state+1Ch = 62.0` m -
the 12 m release band plus the 50 m low end of `009D0E71`'s draw - and the aircraft level their
wings (`cmd+2C4h = 0`, `cmd+2CCh = 1`) until they pass `state+20h = 42` m, at which point the
manoeuvre window opens and they roll.

**What the task does next, and it is not settled.** `releases=1` each, so every aircraft is out of
ordnance after its drop. The goaway now opens the range past its 700 m break-off (`range_peak`
701.0 against 382.2), so `009D3150` finally answers true and the arm cycles back through
`attackrun` and `aim` - 82 goaway entries and 165 transitions where the unbound host had 1 and 3.
`009D4030 BSP_BotTaskTorpedo_TransitionRule` decides that next state and this host's binding of it
is **not** established to be the image's for a torpedo-less aircraft. Treat the churn as a finding
about the transition rule, not as evidence about `009D0F10`: the tick's four arms are exercised
465 to 551 times each and every one of them behaves as the listing says.

### The two holes, and how far each one reached

| hole | reach in this run |
| --- | --- |
| `PilotBotParameters` `TorpFlikFlakTime` (`row+10h`/`+14h`), not loaded by this host, so `state+28h`'s draw starts at 0 | the manoeuvre window opens on the first tick above `state+20h` instead of after the authored delay. `win_high` = 106 to 170 of 465 to 551 ticks |
| `unit->vtable[34h]` = `007BBB70` copies out `unit+AC8h..AD0h`, whose identity is unproved (no literal-address writer in `.text`; the only two disp32 references, `007BBB74` and `007C1B85`, are reads). The host feeds its own world velocity | it moves the break-off BEARING only, and the bearing is published on **0 to 23 ticks of 465 to 551**. Three of the five aircraft never publish it at all |

The second is why the binding is defensible at all: the substituted input reaches the commanded
heading on 40 of 2586 goaway ticks across the flight, and the climb - the thing the run is testing -
does not depend on it.

## 5. The miss: the image does not lead, and the lead it computes is a dead field

The question left open in section 2 was whether anything in the chain leads a moving target over the
torpedo's 23-second run. Answer: **no**, and the proof is a census rather than a reading.

**`009D1360`'s two outputs.** Full byte census over `.text`, `--limit 4000` on every scan so no list
is truncated (the default cap is 20 matches, which is how this packet's first pass produced a false
negative and had to redo every scan):

| field | encoding scanned | every access in the `009C`/`009D` band |
| --- | --- | --- |
| `approach+A0h`, the fall lead | `D9 ?? A0 00 00 00`, `D8 ?? ..`, `F3 0F 10/11 ?? ..`, `8B ?? ..` | `009D04E9` store (Reset), `009D13B9` store (`009D1360`), `009D12D0` read - a one-instruction virtual getter `FLD [ECX+0A0h]; RET` - and `009D2044` read, which is the aim tick's release gate `range + 80 > +A0h` |
| `approach+98h`, the run time | the same four forms | `009D14E7` store (`009D1360`), `009D3D12` read (the engagement estimate) |
| `approach+F8h`, the engagement estimate | the same four forms, plus the `task+4F0h` alias `?? ?? F0 04 00 00` and the SIB forms | **three stores** at `009D3D2F`, `009D3D52`, `009D3D65`, all inside `009D3420`, and **no read anywhere in the image** |

Positive controls for every negative: `D9 ?? F8 00 00 00` occurs 20 times in `.text`,
`F3 0F 10 ?? F8 00 00 00` 18 times, `D9 ?? F0 04 00 00` 9 times, `F3 0F 10 ?? F0 04 00 00` once,
`D8 ?? F0 04 00 00` twice, `D9 ?? ?? F8 00 00 00` 23 times and `F3 0F 10 ?? ?? F8 00 00 00` 4 times
- none of them in the bot-task band. The encodings the compiler would use exist; they are simply not
used on this field.

**So `approach+F8h` is a write-only field.** The run time `+98h` reaches it and stops there. The fall
lead `+A0h` reaches the release **gate** and a getter, never a heading. Neither term can move
`approach+94h`, the commanded bearing.

**And the host's commanded heading is the bare bearing, measured.** From
`local/goaway_bound_usn01.log`'s aim census, at every tick and for every aircraft,
`cmd_2C0 == bearing_94 - 2pi` exactly, with `scan 009D37AE: turn_5c=0.0000 rad`:

| aircraft | aim tick | `cmd_2C0` | `bearing_94` | `bearing_94 - 2pi` | `yaw_C6C` | `range_90` |
| --- | --- | --- | --- | --- | --- | --- |
| Mav3 | 201 (the last census before its drop) | -1.8507 | 4.4325 | -1.85069 | -1.8683 | 698.6 |
| Mav2 | 201 | -1.8819 | 4.4012 | -1.88199 | -1.9010 | 695.3 |
| Mav3 | 251 | -1.8170 | 4.4662 | -1.81699 | -1.8424 | 398.2 |
| Mav1 | 251 | -1.8063 | 4.4769 | -1.80629 | -1.8254 | 458.8 |

`bearing_94` is the bearing to `approach+ACh`/`+B0h`, and `src/torpedo_approach_update.cpp` sets
that pair from `approach->vtable[0]()`, for which this host substitutes the ordered target's
**present** world position. So the host aims exactly at where the ship is, with zero lead and zero
sector offset.

**How big the resulting miss has to be.** The three escorts are under way for the whole mission:
`ship ai step 3000` has Northampton at `throttle 0.841`, Dunlap `0.811` and SaltLakeCity `0.795`
against a `reference_speed` of 16.72 m/s, and Northampton's own distance-to-waypoint closes from
12046.18 m at step 90 to 10059.83 m at step 3000, which is 1986 m in 145.5 s = **13.7 m/s**. A
torpedo released at about 700 m and swimming 30.9 m/s runs for **22.7 s**, in which the target moves
**about 311 m** - against a 180.0 m hull (`Northampton`'s class-row Length). Aimed at the present
position it cannot hit unless the approach is nearly bow-on or stern-on.

**The one door left open.** Everything above says the image does not lead *through `009D1360`*. It
does not say the image never leads: `approach->vtable[0]()` could itself return a lead point rather
than the target's position, and its body is unread - it is reached only indirectly, through
`MOV EDX,[EAX] / CALL EDX` at `009D3517`, `009D36E4` and `009D3DC8`, so `BotApproachTorpedo`'s
vtable has to be found from its constructor before the slot can be disassembled. **That is the next
packet**, and it is the last place a lead could hide.

### Not measured, and why

Per-torpedo closest approach to the target's hull and its time is **not** in this report. The swim
is stepped in `src/game_hosts_gunnery.cpp`, which is leased to `agent/cc8-ai-squadron` for packet
`cc8_ai_target_choice_classes`; instrumenting it needs that lease and its own mission-length run.
The log records the drops (`drop 1 by Mav3 at 12 m, speed 73.5 m/s, swim 30.9 m/s`, five of five,
`water_entry_breakups=0`) and the damage ledger (`attributions=59`, all of them gun rounds: side
1's own `dealt` column sums to 326 against SaltLakeCity's 327 `taken`), so **zero torpedo damage**
stands, but the closest-approach number itself is still owed.
