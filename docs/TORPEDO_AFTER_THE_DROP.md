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

## 6. Four corrections to sections 4 and 5, each made before anyone acted on it

### 6.1 Section 4's before column was confounded, so it was re-run clean

Section 4 compared `local/goaway_bound_usn01.log` against `local/aimclass_after_usn01.log`. That
baseline was taken on `d74aa5b4a`, and **159 lines of the plane, ship and AI path landed between
that tree and the tree the after-run was built from** - `src/game_hosts_ai.cpp` +85,
`src/game_hosts_units.cpp` +47, plus `include/bsp/plane_squadron_host.hpp`,
`include/bsp/ship_ai_attackmove_substates.hpp` and `include/bsp/unit_death_sink.hpp`, arriving
through the `cc8-dive-bomb` and `cc8-plane-squadron` merges. A deaths delta measured across that
span is not attributable to the goaway binding.

So the before column was re-taken at **`81ccfd29d`**, which is the after-tree minus exactly the
binding: `include/bsp/torpedo_goaway_tick.hpp` and `src/torpedo_goaway_tick.cpp` are present and
`src/game_hosts_units.cpp`'s state switch does not call them. Detached build, both ctest suites
passed, same command, `query session` Active, log `local/goaway_before_usn01.log`.

**It reproduces the old baseline to the digit**: `deaths=5 kill_credits=5`, `queued_hits=111`,
`total_damage=2596.8`, `first_hit=58.65 s`, `goaway enters=1`, `range_peak_in_goaway` 382.2 / 368.9
/ 370.6 for Mav1 / Mav3 / Mav5, `transitions` 3 to 4. All five aircraft die, at 109.15 to 136.95 s.

* **was**: the before column is a different tree and the delta is confounded.
* **is**: the confound was a real risk and is **inert in fact** - the intervening 159 lines move
  nothing in this mission. Section 4's table stands, and it now stands on a before column taken one
  commit away from the after column rather than thirty.

The corrected pairing, for anyone quoting it:

| quantity | before `81ccfd29d` (`goaway_before_usn01.log`) | after `eedc1a5d3` (`goaway_bound_usn01.log`) |
| --- | --- | --- |
| deaths / kill_credits | 5 / 5 | **2 / 2** |
| hits taken / damage taken | 111 / 2596.8 | 59 / 1501.9 |
| aircraft alive at the end | none | Mav2 450/450, Mav3 450/450, Mav4 175/450 |
| `goaway` enters / transitions | 1 / 3 to 4 | 82 / 165 |
| `range_peak_in_goaway` | 368.9 to 382.2 | 701.0 |

### 6.2 `approach->vtable[0]` is `009D0670`, it is read, and it does no arithmetic

Section 5 closed with "`approach->vtable[0]()` could itself return a lead point, and its body is
unread ... so `BotApproachTorpedo`'s vtable has to be found from its constructor". Both are done.

`009D3050 BSP_BotTaskTorpedo_Construct` does `LEA EDI,[ESI+3F8h]` at `009D3080` and
`MOV dword ptr [EDI], 0D213C0h` at `009D30A7`, so **`BotApproachTorpedo`'s vtable is `00D213C0`**
and slot 0 is the dword there: **`009D0670`**. (Slot 1 at `00D213C4` is null; `00D213C8` is the
task's own vtable, from `MOV [ESI],0D213C8h` at `009D30A1`.)

```
009d0670: MOV EAX,[ESP+4]            ; the return buffer
009d0674: FLD  [ECX+0D0h]  -> [EAX]
009d067c: FLD  [ECX+0D4h]  -> [EAX+4]
009d0685: FLD  [ECX+0D8h]  -> [EAX+8]
009d068e: RET 4
```

`Vec3 __thiscall(approach*, Vec3* out)`, 31 bytes, `RET 4`, no constants. It copies the stored vec3
at `approach+D0h..D8h` and does **nothing else**: no velocity, no time, no sector offset, no
arithmetic of any kind. It is the same shape as the dive bomb's slot 0 `009C40A0`, which returns
that approach's own `+4Ch/+50h/+54h`.

* **was**: the last place a lead could hide is `approach->vtable[0]`'s body.
* **is**: `vtable[0]` cannot lead. The question moves one step, to `approach+D0h`'s producer.
* it also **confirms the section 2 retraction from the callee's own body**: `RET 4`, one stack
  argument. The stack-balance walk and `009C47D0`'s call setup both said four bytes; the body agrees.

**Ghidra has no function at `009D0670`.** `009D0380 BSP_BotApproachTorpedo_Reset`'s body is
`009D0380`-`009D066F`, one byte short of it, so a byte scan reports hits here as "in `009D0380`".
That is the nearest-preceding-function artefact, not containment: the ledger record is
`no_ghidra_function`.

### 6.3 `approach+D0h`'s producer is not reachable by a literal-address scan, and that is a bound, not a negative

Seven encodings over `.text`, each with a positive control, `--limit 4000` throughout:

| form | image-wide hits | hits in the `009C`/`009D` bot-task band |
| --- | --- | --- |
| `F3 0F 11 ?? D0 00 00 00` MOVSS store | 16 | none |
| `D9 ?? D0 00 00 00` x87 load/store | 109 | only `009D0674`, the read inside `009D0670` itself |
| `89 ?? D0 00 00 00` MOV store | 59 | none |
| `8B ?? D0 00 00 00` MOV load | 145 | none |
| `C7 ?? D0 00 00 00` MOV immediate | 8 | none |
| `8D ?? D0 00 00 00` LEA | 29 | none |
| `0F 10 ?? D0 00 00 00` MOVUPS | 4 | none |

`66 0F D6 ?? D0 00 00 00` (MOVQ store) and `F3 0F 7E ?? D0 00 00 00` (MOVQ load) returned nothing
**image-wide**, so they have no positive control and are recorded as vacuous rather than as
evidence. `approach+CCh`, the target pointer that sits immediately before the vec3, is the same:
read at `009D0BE0`, `009D0DC9`, `009D1701`, `009D362A`, `009D36D7` and `009D3B0E`, written nowhere
in the band (`89 ?? CC 00 00 00`, 60 image-wide, none in band).

**This does not say nothing writes it.** It says nothing writes it through a literal displacement
from the approach base inside the bot band, which is the known signature of a field filled through a
base pointer or by a block copy. A helper handed `&approach+CCh` would address the three floats at
`+4h`, `+8h`, `+0Ch` - **disp8**, invisible to any `D0 00 00 00` pattern - but `8D ?? CC 00 00 00`
(430 image-wide) has no hit in the torpedo band either, so that particular route is excluded. The
producer holds the approach at some other offset, or lives outside the band.

**So section 5's conclusion stands where it was proved and no further.** The image does not lead
through `009D1360`: `approach+F8h` is write-only and `+A0h` reaches only the release gate and a
getter. Whether the target point *itself* is a lead point is open, and it is now one well-posed
question - who writes `approach+D0h..D8h` - rather than a routine to disassemble.

### 6.4 Section 5's escort speed: 13.7 m/s was two endpoints across a turn

Section 5 derived the escorts' speed from Northampton's `d32c` at `ship ai step 90` and `step 3000`
and got 13.7 m/s. That span includes the ship accelerating from rest and a rudder-0.805 turn at step
800, during which `d32c` (distance to waypoint) falls more slowly than the hull travels. Sampled
across the run instead:

| span | steps | seconds | `d32c` closed | rate |
| --- | --- | --- | --- | --- |
| 90 to 800 | 710 | 35.5 | 300.68 m | 8.5 m/s (accelerating, then turning) |
| 800 to 1600 | 800 | 40.0 | 636.41 m | 15.9 m/s |
| 1600 to 2400 | 800 | 40.0 | 612.41 m | 15.3 m/s |
| 2400 to 3000 | 600 | 30.0 | 436.85 m | 14.6 m/s |

* **was**: the escorts make 13.7 m/s, so a 22.7 s run lets the target move about 311 m.
* **is**: the steady-state rate over steps 1600 to 3000 is **15.0 m/s**, and every drop falls inside
  that phase (about t = 78 to 88 s, steps 1560 to 1760). A 22.7 s run lets the target move
  **about 340 m**.
* and it is a **lower bound**, not a measurement: `d32c` is the distance to the waypoint, and
  `heading = -1.0472` against `target = 4.5952` says the hull is not pointing at it, so the distance
  closes more slowly than the hull travels. The authored `reference_speed` is 16.72 m/s and the
  throttle sits at 0.92, which puts the true speed near 15.4 m/s and corroborates the bound.

The conclusion is unchanged and slightly stronger: the displacement over the run is close to twice
the 180.0 m hull length, so a torpedo aimed at the present position cannot hit except bow-on or
stern-on.

#### 6.3.1 The census extended to nine forms, and one scan withdrawn as malformed

Three more encodings, each with a positive control, all empty in the `009C`/`009D` band:
`0F 11 ?? D0 00 00 00` (MOVUPS store, 16 image-wide), `05 CC 00 00 00` (`ADD EAX,0CCh`, 138
image-wide, one hit at `009C964E` in the dive-bomb band) and `81 ?? CC 00 00 00` (the group-1
immediate forms including `ADD reg,0CCh`, 86 image-wide). With the seven in 6.3 that is **nine
valid encodings** and no writer of `approach+D0h..D8h`, and no pointer arithmetic that would reach
it at disp8, anywhere in the bot-task band.

Withdrawn: a scan written as `81 C? CC 00 00 00` returned zero. `C?` is not the pattern syntax -
only a whole-byte `??` is - so that scan proved nothing and is replaced by the `81 ?? CC 00 00 00`
row above. `66 0F 11 ?? D0 00 00 00` and the two MOVQ forms in 6.3 remain vacuous for the other
reason: no image-wide hits, so no positive control.

The bound is now tight enough to name the next move, and it is not another scan: the producer has to
be found from the other end, by reading what constructs or re-targets the approach rather than by
looking for the field.

## 7. The churn section 6.1 flagged: the image retires a spent bomber, and this host re-attacks

Section 4 recorded `goaway` entries going from 1 to 82 and `transitions` from 3 to 165 once the tick
was bound, and said the transition rule was "not established to be the image's". It is now, and the
gap is a single host predicate rather than anything in `009D0F10`.

### 7.1 `009D4030`, the two routes to `done`

`009D4030 BSP_BotTaskTorpedo_TransitionRule`, body `009D4030`-`009D4222`, `__fastcall(task)`, called
once per tick from the arm at `009D48DE`. The state pointer is `task+310h` and the states it
compares against are `task+710h` aim, `task+618h` done, `task+6D8h` goaway, `task+6B4h` attackrun,
`task+740h` prepare, `task+544h` moveto and `task+580h` follow.

Two branches reach `done`:

```
if (current != done) {
    if (IsAttackState(current) && task->vtable[1Ch]())   -> done        009D4C10
    ...
    if (current == goaway && GoAway_IsComplete()) {                     009D3150
        if (task+52Ah == 0)  -> done                                   task+618h
        else                 -> aim                                    task+710h
    }
}
```

So **a bomber whose `task+52Ah` is clear retires when its break-off completes**; only one that still
has the flag goes round again. `task+52Ah` is `approach+132h` - the approach sits at `task+3F8h`, and
`3F8h + 132h = 52Ah` - which `include/bsp/torpedo_task_arm.hpp` already names `kAttackFlag` and the
approach update already carries as `has_ordnance_132`.

### 7.2 Why this host goes round again, named to the line

`src/torpedo_approach_update.cpp:346` fills `has_ordnance_132` from
`host.unit_has_torpedo_ordnance_007b93f0()`, and the binding at
`src/game_hosts_units.cpp:3753`-`3759` says in its own comment:

> the kind 2Bh test `0099A170` already made when it built the task; **the loadout does not shrink in
> this host**, so it holds for the whole run.

So `approach+132h` is **true for the whole mission whatever the aircraft drops**. The consequences
line up exactly with both runs:

| | `local/goaway_before_usn01.log` | `local/goaway_bound_usn01.log` |
| --- | --- | --- |
| `range_peak_in_goaway` | 368.9 to 382.2, under the 700 m break-off | 701.0, over it |
| `009D3150` | never true, so the transition never ran | true, so it ran |
| `task+52Ah` at that moment | (never reached) | true, because the loadout never shrinks |
| result | one goaway, ticking forever | 82 goaway entries, 165 transitions |

**The churn is not a defect in `009D0F10` and it is not evidence against the binding.** It is a
pre-existing host gap that the binding made reachable: before it, the break-off never opened the
range past 700 m, so `009D4030`'s goaway branch was dead code in this host and nothing could notice
that the ordnance byte never clears.

`009D3150` itself refuses outright without the byte (`src/torpedo_task_arm.cpp:384`,
`009D317C XOR AL,AL`), so a host that cleared it on release would see the goaway never complete and
the aircraft hold the break-off - which is the image's behaviour for a bomber that is out of
torpedoes and has already been told to go away.

**The fix, for whoever owns the release spawn**: clear the torpedo bit from the slot's ordnance mask
when the release actually spawns a round, instead of deriving the byte from the static loadout. Not
done here: `slot_.ordnance_mask`'s producer is the release chain, not this packet, and changing it
moves the aim state's gates as well as the transition.

### 7.3 `TorpFlikFlakTime` is not a small gap, and here is why

The manoeuvre-window delay `state+28h` draws `UniformFloatRange(row+10h, row+14h)` at `009D0EA3`,
and those two are `TorpFlikFlakTime` 1 and 2 (`src/robot_config.cpp` lines 109 and 110, config
offsets `1Ch` and `20h`; `docs/TORPEDO_RUN_PROFILE.md` fixes `row+0h` as `TorpReleaseAlt`, so the
fifth and sixth floats are the pair). The loader **is** in our files:
`load_robot_config_00901610` builds one descriptor per name and `src/global_subsystems.cpp:81` calls
it, with `"PilotBot"` mapping to `[00F8A30C]` - the very array `approach+14h` indexes.

But it does not run in `bsp_game`. `src/game_hosts_mission_frame.cpp:1545` handles the
`global_subsystems` load step as a **record**, not a concrete call, with the reason in its own
comment: `004dc6a0` "needs a `GlobalSubsystemContext` this process cannot build". Only the script
folders inside it run. So `PilotBotConfig` has no instance at runtime, and there is nothing for a
units-host binding to read.

Closing it therefore means building a `GlobalSubsystemContext`, and the step that would call it
lives in `src/game_hosts_mission_frame.cpp`, which is Codex-owned. It is a packet of its own with a
different owner, not something to pick up on the way; recorded here so the next reader does not
re-derive it. Meanwhile the hole's reach is bounded and stated in section 4: the window opens on the
first tick above `state+20h` instead of after the authored delay.

## 8. The closest approach, measured — and it contradicts section 5's magnitude

Packet `cc8_torpedo_closest_approach`, the number this stream has owed since section 2.
`src/game_hosts_gunnery.cpp` census (commit `1fdecbb39`), run `local/closest_approach_usn01.log`,
USN01, 3000 mission frames.

**A swimming round carries no target.** `GameProjectileRow` has `owner_unit` and no victim, and the
swim keeps its launch heading, so "closest approach to its target" is not directly measurable. The
census measures against every unit of another side and names the nearest. Distances are **centre to
centre and horizontal** — this host has no oriented hull box for a ship — so they are not miss
distances from the plating and have to be read against the target's own Length.

| torpedo | nearest unit of another side | closest approach | at | run length |
| --- | --- | --- | --- | --- |
| Mav1 | Dunlap | **51.5 m** | 26.45 s | 60.05 s |
| Mav2 | Storage, 04 01 | 26.1 m | 56.15 s | 60.05 s |
| Mav3 | Hangar, Small, 04 01 | 17.3 m | 51.75 s | 51.75 s |
| Mav4 | SaltLakeCity | **76.8 m** | 21.60 s | 60.05 s |
| Mav5 | SaltLakeCity | **67.9 m** | 21.85 s | 60.05 s |

Two facts fall straight out. The rounds **swim for a full minute**, not the 22.7 s the section 5
arithmetic used — that figure was the time to the target, and the round keeps going past it; two of
the five ran on to shore structures. And the closest approach to an escort happens at **21.6 to
26.5 s**, which is exactly the 22.7 s the geometry predicted, so the timing half of section 5 holds.

### 8.1 The magnitude does not hold, and this is a retraction of the prediction, not of the reading

Section 5 predicted that a zero-lead aim against a target making 15.0 m/s over a 22.7 s run would
miss by **about 340 m**. Even taking only the component across the torpedo's track — the escorts
head about `-1.05` rad and the commanded heading at release is about `-1.79` rad, some 42 degrees
apart — that is still roughly 220 m. **The measured closest approach to the ships aimed at is 51.5
to 76.8 m**, three to six times smaller.

* **was**: the miss is of the order of the target's displacement during the run, about 340 m.
* **is**: the miss is 51.5 to 76.8 m centre to centre, at the predicted time. The *timing* model was
  right and the *displacement* model is wrong by a factor of three to six.
* **not concluded**: which of the premises fails. Three survive the evidence here and this document
  does not choose between them:
  1. the escorts' speed over the drop window is well below the 15.0 m/s that `d32c` implies —
     `d32c` is the distance to a waypoint, not a path length, and nothing in this run reports a hull
     speed directly (the single `keel=` line is at load time);
  2. the ships the torpedoes came closest to are not the ships they were aimed at — the census
     reports the nearest unit of another side, and `PilotSetTarget` logs
     `target_object_id=45 target_valid=0 pos=(0.0 0.0 0.0)`, so the ordered target's identity is
     never resolved to a name anywhere in the log;
  3. something upstream does lead after all, which is exactly the open question of section 6.3 —
     what writes `approach+D0h..D8h`.

Premise 3 is the one that matters, and the measurement now bears on it directly: a 51.5 m closest
approach is **much closer than a zero-lead aim against a 15 m/s crosser can produce**. That is
evidence for a lead somewhere upstream, and it is the first evidence in either direction that did
not come from a byte census.

**The next check is small and belongs with this census**: record, per swimming round, the ordered
target's identity and that target's world position at release and at closest approach. That
separates premise 1 from premise 2 from premise 3 in one run, and it is three more fields in the
same structure this packet already added. It is not done here.

Until it is, section 5's *conclusion* — that nothing reaches the commanded heading through
`009D1360`, proved by census — stands, and section 5's *arithmetic about how big the resulting miss
must be* is withdrawn.

#### 6.3.2 Eleven forms, and the Reset does not touch the aim point either

Two more encodings with positive controls, both empty in the `009C`/`009D` band: `05 D0 00 00 00`
(`ADD EAX,0D0h`, 4 image-wide) and `81 ?? D0 00 00 00` (the group-1 immediates, 44 image-wide).
**Eleven valid encodings** now, and no writer of `approach+D0h..D8h` in the bot-task band.

Two structural facts from the other end, which is where the lead pointed:

* **Six routines take `&task+3F8h`** (`8D ?? F8 03 00 00`, 88 image-wide): `009D3080` in
  `BSP_BotTaskTorpedo_Construct`, `009D42BC` in `FUN_009D4230`, `009D485A` in
  `BSP_BotTaskTorpedo_TickArm`, `009D4BA1` in `BSP_BotTaskTorpedo_UpdateCruiseProfile`, `009D4D1B`
  in `FUN_009D4C90` and `009D4DD7` in `FUN_009D4DB0`. `009D42BC`'s is a **read**: `LEA ECX,[EBP+3F8h]`
  then `LEA EDX,[ESP+5Ch]` then `CALL EAX`, which is `approach->vtable[0](&buf)` again.
* **`009D0380 BSP_BotApproachTorpedo_Reset` does not touch it.** Over its whole body
  `009D0380`-`009D066F` there is no store at any displacement in `0C0h`..`0DFh`, and the only address
  it takes of a member is `LEA ECX,[ESI+0B4h]` at `009D052F`. So `approach+CCh` and `+D0h..D8h`
  **survive a reset**, which fits a pair owned by whatever installs the target rather than by the
  approach itself.

The static bound is now as tight as scanning can make it. **The cheaper and more decisive next step
is section 8.1's**, not more of this: record the ordered target's identity and its world position at
release and at closest approach. That separates "the escorts are slower than `d32c` implies" from
"the nearest ship is not the ship aimed at" from "something upstream leads" in a single run, and it
does not depend on finding the producer at all — if the aim point sits ahead of the target, the
producer hunt has its answer from the outside.

### 7.4 `009D4C10` is a target-and-range rule, not the ordnance route

Section 7.1 listed `task->vtable[1Ch]` (`009D4C10`) as the first of two routes to `done` without
saying what it tests. Read from the listing, because acting on 7.1 made the difference matter:

```
009d4c22: EAX = [ESI+4C4h]                ; task+4C4h = approach+CCh, THE TARGET POINTER
009d4c28: if (EAX == 0)            -> true
009d4c2c: if ([EAX+5Dh] != 0)      -> true
009d4c32: EAX = [ESI+404h]                ; approach+0Ch, the control block
009d4c38: if ([EAX+369h] && [00E17BF2]) -> false
009d4c4a: ... a call on the current state [ESI+310h] ...  -> false on one arm
009d4c65: FLD [ESI+488h]                  ; approach+90h, the range
009d4c6f: CALL 0042E740                   ; the tuning singleton
009d4c78: FLD [EAX+438h]                  ; Pilot/Torpedo/SafeDist, 700 here
009d4c7e: FMUL [ESI+41Ch]                 ; approach+24h, the speed ratio
009d4c88: if (SafeDist * ratio > range) -> false
009d4c8a: -> true
```

So it fires when the **target pointer is null**, when the target's `+5Dh` byte is set, or when the
**range has opened past `SafeDist * approach+24h`**. It has nothing to do with the ordnance byte. A
bomber is therefore retired by losing its target or by opening the range, on any attack state - and
in `local/goaway_bound_usn01.log` the range peaks at 701.0 against a SafeDist of 700, right at that
threshold.

**A false alarm recorded so nobody re-raises it.** Reading `src/torpedo_task_arm.cpp:384` -
`if (!in.has_ordnance_132) return false;` - without its enclosing guard suggested `009D3150` refuses
outright whenever the byte is clear, which would make section 7.1's `task+52Ah == 0 -> done` branch
unreachable and the whole diagnosis self-contradictory. It is not: `009D3168` and `009D3171` skip
the whole block unless **both** `ctl+369h` and `[00E17BF2]` are set, and the host's reconstruction
nests the refusal inside exactly that condition. With `ctl+369h` off, as this host reports it,
`009D3150` is the plain range test and the ordnance byte does not gate it. Section 7.1 stands.

### 8.2 The same numbers against each class's own Length

| torpedo | nearest | class | beam (`width`) | Length | closest approach | as a fraction of half-Length |
| --- | --- | --- | --- | --- | --- | --- |
| Mav1 | Dunlap | Destroyer (`type_id` 309) | 10 | not in this run's log | 51.5 m | - |
| Mav4 | SaltLakeCity | HeavyCruiser (`type_id` 297) | 16 | 180.0 m | 76.8 m | 0.85 |
| Mav5 | SaltLakeCity | HeavyCruiser | 16 | 180.0 m | 67.9 m | 0.75 |
| Mav2 | Storage, 04 01 | shore structure | - | - | 26.1 m | - |
| Mav3 | Hangar, Small, 04 01 | shore structure | - | - | 17.3 m | - |

`Length 180.0` is Northampton's class row, quoted by the hydrodynamics line at load; SaltLakeCity
shares `key=HeavyCruiser` and the same `reference_speed`, so it is the same row. The Destroyer
Length is **not** in this run's log and is left blank rather than guessed.

**What the fraction does and does not say.** A centre-to-centre distance of 0.75 to 0.85 of the
half-Length means the round passed within the hull's *along-track* envelope: had it been crossing
near the bow or stern line it would have been a hit, and had it been abeam it cleared the 8 m
half-beam by some sixty metres. A single scalar distance cannot tell those apart, which is the whole
force of the centre-to-centre caveat. Separating them needs the bearing of the closest-approach
point relative to the target's heading, which is one more field in the same census and is part of
section 8.1's next check.

### 8.3 Retraction: `target_valid=0` does not mean the order's target failed to resolve

Section 8.1's premise 2 said the ordered target's identity "is never resolved to a name anywhere in
the log", citing `PilotSetTarget: unit=Mav1 target_object_id=45 target_valid=0 pos=(0.0 0.0 0.0)`,
and I put the stronger form of it - "an order whose target never resolves" - to the lead and to
`agent/cc8-ai-squadron`. **That reading is wrong**, and the producer says so.

`src/game_hosts_script_orders.cpp:1000`-`1009` prints `target_valid` from
**`target.position_valid`** and `pos` from `target.position` - the `SceneCommandTarget`'s *position*
fields, not its object. An order that names an object carries no explicit point, so `0` and
`(0,0,0)` are the **expected** values for an object-targeted order, and line 1013 counts exactly
this case as resolved: `if (target.object_id != 0 || target.position_valid)
++pilot_set_target_target_resolved_`.

* **was**: the order's target never resolves, which puts a fault upstream of everything measured.
* **is**: the order carries object id 45 (Mav1), 44 (Mav2, Mav3) and 46 (Mav4, Mav5). Only the
  *position* is absent, and correctly so. Nothing is broken here.
* **still open**: which object those ids name. That is a real question and this document does not
  answer it.

**And the obvious way to answer it is a trap.** The same run prints
`scene path retained: id=44 ... name=p6de`, `id=45 ... name=p7de`, `id=46 ... name=p8de`, and also
`scene class Landscape id=44` and `scene class AirField id=45`. Three different id spaces carry 44,
45 and 46 in this one log. Matching `target_object_id` against either table on the number alone is
the bare-offset collision in another dress; a first pass here did exactly that and briefly concluded
the Mavs were ordered against paths. **Withdrawn.** `SceneCommandTarget::object_id` is a
`std::uint16_t` in the scene *object* id space, and nothing in this run prints that space's table.

The fix is one line in the same census section 8.1 already needs: print the object-id-to-entity
mapping for the ids the orders actually carry, in the same run. Until then the five Mavs' ordered
target is **unnamed**, and the separate question of what the gunnery path resolves is also unnamed:
`src/game_hosts_gunnery.cpp:1318`-`1322` picks the newest current command row per unit
(`0071EBF0`'s rule, categories 1 and 2 only) and resolves `row.target_token` **by unit name**
through `by_name`, but the token string is never logged. `summary mission gunnery command_targets
units_with=5` says five units got one; which unit it names is not in the log either.

### 8.4 One of section 8.1's three fields cannot answer what it was meant to

Section 8.1 proposed recording, beside the target's identity and position, "the stored aim point
`approach+D0h..D8h` at release", on the reasoning that if the stored point sits ahead of the target
along its course then the producer question is answered from outside, without finding the writer.

**That field is vacuous in this host.** `TorpedoApproachHost::approach_target_point`
(`src/game_hosts_units.cpp:3759`-`3769`) substitutes the ordered target's **current world position**
for `approach->vtable[0]`, and `src/torpedo_approach_update.cpp:443` stores that same value into
`plan_target_x_ac`/`plan_target_z_b0`. So the host's stored aim point **is** the target's position by
construction: logging it and comparing it to the target can only ever return "identical", whatever
the image does. A run spent on it would produce a confirmation of the host's own substitution and
read afterwards as evidence about the image.

* **was**: instrumenting the stored aim point answers the producer question from outside.
* **is**: it cannot, in this host. The producer question needs either the image's writer (the static
  hunt, bounded at eleven encodings in 6.3) or a native trace; nothing the host stores can stand in,
  because the host is where the substitution lives.

**What the same run can still settle, and it is worth one run:**

| field | premise it decides |
| --- | --- |
| the ordered target's identity, and the `target_token` the gunnery rule resolved | 8.1's premise 2 - whether the ship a torpedo came nearest to is the ship it was aimed at |
| that target's world position at release and at closest approach | 8.1's premise 1 - the target's real speed over the run window, instead of the waypoint-closing rate `d32c` gives |
| the bearing of the closest-approach point relative to the target's heading | whether 51.5 m is a near miss abeam or a pass inside the bow or stern line (section 8.2), which a scalar distance cannot separate |
| the object-id to entity mapping for the ids the orders carry | what object 44, 45 and 46 are (section 8.3), which three colliding id spaces make unanswerable from the current log |

Premise 3 - that something upstream leads - is then reached by elimination rather than directly: if
the target is the one aimed at and its speed over the window really is of order 15 m/s, a 51.5 m
closest approach is not what a zero-lead aim produces, and the lead has to be upstream. That is
weaker than a direct measurement and it is the strongest this host can give.

## 9. The ordnance decrement, tried and falsified by its own run

Section 7.2 named the host gap — `approach+132h` never clears because the loadout does not shrink —
and prescribed the fix: clear the owner's kind `2Bh` bit on a drop so a spent bomber retires the way
`009D4030` describes. It was implemented (`1e7c0f2f2`), run, and **the run falsified it**. Reverted
in the same session; `src/game_hosts_gunnery.cpp` is byte-identical to `1fdecbb39` apart from a
comment recording this, so no confirming run is needed.

### 9.1 What the run said

`local/ordnance_clear_usn01.log` against `local/closest_approach_usn01.log`, same binary, same
command:

| quantity | byte stays set (`closest_approach`) | byte clears on the drop (`ordnance_clear`) |
| --- | --- | --- |
| `torpedo_loadout_cleared` | (not instrumented) | 5 — the change did fire |
| `goaway` **enters** | 82 | **0** |
| `009D0F10` ticks | 465 to 551 | **0** |
| `states` | `attackrun`, `goaway`, `aim` | `attackrun`, `aim` only |
| `arm_ticks` (Mav1) | 1299 | 678 |
| deaths / kill_credits | **2 / 2** | **5 / 5** |
| hits taken / damage | 59 / 1501.9 | 178 / 2572.9 |

The goaway state was **never entered**, so nothing ran the climb-away, so every bomber went back into
the water. The churn was the lesser wrong by a wide margin.

### 9.2 Why, and what it proves about the byte

`009D3F60`, the entry chooser (`src/torpedo_task_arm.cpp:54`-`62`, from `009D3FA9`/`009D3FC7`):

```
if (!attack_flag_52a && (!control_flag_369 || !global_e17bf2))  return kDone;
```

With `ctl+369h` off — as this host reports it — a task whose `+52Ah` is clear is sent **straight to
`kDone`** the next time the entry chooser runs. The aim state never hands off to the goaway, because
the task leaves the attack chain before it can.

* **was** (section 7.2): a drop clears `approach+132h`, and `009D4030` then retires the bomber
  through the goaway's completion.
* **is**: a cleared `+132h` removes the whole attack chain, goaway included. The goaway is entered
  **from aim**, and `009D3F60` retires the task before aim can get there.
* **therefore**: `approach+132h` **does not clear on a drop in the image**, or something else keeps
  the task in the attack chain long enough to break off. A state with a 300-byte enter
  (`009D0D90`-`009D0F04`), a 768-byte tick (`009D0F10`-`009D1210`) and its own completion predicate
  (`009D3150`) is not dead code, and a model that makes it unreachable is wrong on that ground alone.

What `007B91C0`'s ordnance-object `vtable[8](0)` consumes on a drop **stays unread**, and section
7.2's confidence that it must consume something is now only half right: it may consume a per-device
round count without the *kind* bit ever clearing, which would leave `+132h` set and every transition
above it intact. That is the shape the evidence now favours and it is not proved either.

### 9.3 What section 7.2 should have said

Section 7.2's reading of `009D4030` is unchanged and still correct: with `+52Ah` clear, a completed
goaway goes to `done`. What was wrong was the inference that clearing the byte is therefore what
retires a bomber — it is upstream of the goaway, not downstream, and the entry chooser sees it
first. The 82 re-entries remain unexplained by anything this packet has established, and the honest
statement is that the host re-attacks because `+132h` stays set **and that may be what the image
does too**.

#### 6.3.3 The hunt closed image-wide, and a gap in 6.3 corrected

Sections 6.3 and 6.3.1 filtered every scan to the `009C`/`009D` bot-task band. **That was too
narrow**: the order path that installs the target lives at `0099xxxx` and the shared approach
machinery at `009Fxxxx`, neither of which those filters covered. Re-run without the band filter:

* `F3 0F 11 ?? D0 00 00 00`, all **16** image-wide hits, by containing function: `004CB420`,
  `006E22D0 BSP_ProjectileShotBase_Construct`, `007868C0`/`00786A80` the peer rows, `00799B00`,
  `0079C910`, `007A0E00`, `007AB230`, `00812D40 BSP_UnitOrderRing_Construct`,
  `009E0270 BSP_ShipAi_HullPreStep`, `00B4D500`, `00BA3BD0`, `00BA40F0`, `00BB0E30`. **Not one is a
  bot-approach class.**
* The `0099`/`009A`/`009B` bands do carry `+D0h` accesses, but they belong to other objects:
  `009973B0 read_PilotBot_parameters_009973B0` writes its own, and `009B5C80`, `009B6670` and the
  thunk at `009B5C50` **read** a `+D0h` vec3 exactly the way `009D0670` does - `009B5760` is one of
  `009FD570`'s six callers, so that is a sibling approach class with the same field.

So the field is a **shared approach-class vec3, read by several of them and written by none of them
through a literal displacement, anywhere in the image**. The remaining mechanisms are a struct
assignment through a pointer the callee received (the `LEA` that would hand it over is excluded,
`8D ?? D0 00 00 00`, 29 image-wide, no bot-class hit) or a block copy over a region whose base is
the object, which is the documented shape of a field with no literal-address writer.

**One route this packet found and could not use.** `009A6FB3` builds `{3, &esi+0D0h}` followed by
`{0, 00D1F878}` = `"inattackrange"` - a Lua property registration, the pattern that names original
struct fields. It is `FUN_009A6D40`'s class, not `BotApproachTorpedo`, so the name does not
transfer; and `8D ?? D0 00 00 00` has no hit in the `009C`/`009D` band, so the torpedo approach
registers no such property for its own `+D0h`. Recorded because a reader who finds
`"inattackrange"` next to a `+D0h` elsewhere should not carry it across.

The static hunt is closed. What remains is a native trace or the elimination argument in 8.4.

## 10. Why the decrement broke the goaway — and it was not the entry chooser

Section 9.2 blamed `009D3F60`, the entry chooser. **That attribution is wrong.** Read properly:

### 10.1 (a) `009D3F60` has exactly one call site, and it cannot run after a drop

`python tools/callsite_census.py 009d3f60` over the whole image: **one** caller, `009D41D3`, inside
`009D4030` itself. A scan for the absolute dword `60 3F 9D 00` returns **0**, so no vtable carries
it either. It is not a per-think entry point.

`009D41D3` is reached two ways, and the host's two calls (`src/torpedo_task_arm.cpp:75` and `:94`)
are those same two paths, not an extra one — the host is faithful here:

* `009D41C4`, when the current state is **not** one of the five attack states (moveto/follow); and
* the `goto LAB_009d41d1` taken when the current state is **prepare** (`task+740h`).

After a drop the state is `aim`, so **the chooser never runs**. Mav1's decrement run confirms it
from the other side: `states[attackrun=424 aim=254]`, no prepare ticks at all.

### 10.2 (b) `task+52Ah` is not a copy of `approach+132h` — it is the same byte

The approach is at `task+3F8h`, proved by `009D3050`'s `LEA EDI,[ESI+3F8h]` at `009D3080` followed
by `MOV [EDI],0D213C0h` at `009D30A7` storing the approach's own vtable. So
`approach+132h` is `task+(3F8h + 132h)` = **`task+52Ah`**, one byte with two names.
`009D34CD MOV [ESI+132h],AL` writes it with `ESI` = the approach; `009D4030` reads it as
`param_1+52Ah` with `param_1` = the task. There is **no copy, therefore no latch and no staleness**,
and the question of "where is it refreshed" has the answer "in `009D3420`, once per tick, because
the arm runs the approach update at `009D486F` every tick".

### 10.3 (c) What actually retired the task: the aim hold, released at the wrong moment

`009D4030` evaluates its tests in this order, and the order is the whole answer:

```
if (current != done) {
    if (IsAttackState(current) && task->vtable[1Ch]())  -> done     <-- FIRST
    ...
    if (current == aim) {
        if (task+52Ah && !AimHold())  return;      // stay in aim
        -> goaway
    }
```

`vtable[1Ch]` is `009D4C10`, which section 7.4 read: true when the target is gone **or the range has
opened past `SafeDist * approach+24h`**. It does not consult the ordnance byte.

So with the byte **set**, the aim state *holds*, and the task stays in aim until `009D31B0` lets it
go — by which time it hands off to the goaway. With the byte **clear**, the hold is released
immediately, and because the break-off test is evaluated **first** in the same call, a task whose
range is already past `SafeDist * ratio` — which after a drop at about 700 m against a SafeDist of
700 it is — goes to `done` **before** the aim branch is ever reached.

* **was** (9.2): `009D3F60` sent the task straight to `kDone`.
* **is**: the chooser never ran. Clearing the byte released the aim hold at a moment when
  `009D4C10` was already true, so `009D4030`'s first test retired the task before aim could hand off
  to the goaway.

### 10.4 What this says about the lead question

The proposed sequence — aim, release, goaway (no chooser), `009D4030` sees the byte clear, done — is
**not** what the image would do on this placement, and the chooser is not the reason. The byte is
read by the aim branch as a *hold*, and releasing it at a range already past the break-off threshold
retires the task on the earlier test. For the sequence to work, the byte would have to clear
**after** the goaway is entered, not at the release.

That is testable and cheap: the release and the aim-to-goaway transition are separate ticks, so a
host that cleared the byte one state later — on entering the goaway rather than on the drop — would
distinguish "the image clears it late" from "the image does not clear it at all". Not done here, and
not worth a run before the section 8.1 census answers what the bombers were aiming at; recorded so
the option is on the table rather than rediscovered.

Section 9's conclusion is unchanged: the decrement as written is wrong and stays reverted. What
changes is the reason, and the reason matters because it says the byte is an **aim hold**, not a
retire signal.

### 8.2.1 Correction: the Destroyer Length was in the log, on a line I did not find

Section 8.2 left Dunlap's Length blank and said it was "not in this run's log". **It was.**
`local/closest_approach_usn01.log` line 1377 prints `unit hull input unit=Dunlap type_id=309 kind=7
length=110 width=10`, from the class data the gunnery host reads at
`src/game_hosts_gunnery.cpp:513` (`flat_scaled(type_id, "length", ...)`). My searches were for
`Length` and for `key=Destroyer`; the line spells it lowercase and carries neither. Recorded because
"not in the log" is a claim about a search, not about a log.

| torpedo | nearest | Length | half-Length | beam | closest approach | fraction of half-Length |
| --- | --- | --- | --- | --- | --- | --- |
| Mav1 | Dunlap | **110 m** | 55 m | 10 m | 51.5 m | **0.94** |
| Mav4 | SaltLakeCity | 180 m | 90 m | 16 m | 76.8 m | 0.85 |
| Mav5 | SaltLakeCity | 180 m | 90 m | 16 m | 67.9 m | 0.75 |

Northampton and SaltLakeCity are `length=180 width=16` (lines 1364 and 1379), confirming from the
class data what section 8.2 had quoted from the hydrodynamics line.

All three passes are **inside the target's along-track envelope**, and Mav1's is at 0.94 of it -
against a 5 m half-beam. That sharpens section 8.2's point rather than settling it: 51.5 m from the
centre of a 110 by 10 metre hull is either a clean miss abeam by some forty-six metres, or a pass
through the bow or stern line, and the crossing angle the section 8.1 census now records is the only
thing that can say which.

## 11. The census answers it: the misses are a stern chase, not a lead failure

`local/aim_census_usn01.log`, USN01, 3000 mission frames, commit `c91545d0c`.

**Every ordered target resolves, and they are named for the first time.** `0071EBF0`'s rule picks a
row per unit and resolves its token by name:

```
command target 0071EBF0: unit=Mav1 token="Dunlap"       -> Dunlap
command target 0071EBF0: unit=Mav2 token="Northampton"  -> Northampton
command target 0071EBF0: unit=Mav3 token="Northampton"  -> Northampton
command target 0071EBF0: unit=Mav4 token="SaltLakeCity" -> SaltLakeCity
command target 0071EBF0: unit=Mav5 token="SaltLakeCity" -> SaltLakeCity
```

| torpedo | ordered target | closest to it | at | that target moved | crossing angle |
| --- | --- | --- | --- | --- | --- |
| Mav1 | Dunlap | 51.5 m | 26.45 s | **449.9 m** | 0.094 rad = **5.4 deg** |
| Mav2 | Northampton | 77.6 m | 22.60 s | 345.2 m | 0.189 rad = 10.8 deg |
| Mav3 | Northampton | 66.6 m | 22.85 s | 349.1 m | 0.159 rad = 9.1 deg |
| Mav4 | SaltLakeCity | 76.8 m | 21.60 s | 313.4 m | 0.205 rad = 11.7 deg |
| Mav5 | SaltLakeCity | 67.9 m | 21.85 s | 317.1 m | 0.178 rad = 10.2 deg |

### 11.1 Premise 1 was right all along, and section 8.1's retraction of it was wrong

Section 5 predicted the target moves "about 340 m" during the run. **Measured: 313.4 to 449.9 m.**
Section 8.1 called that prediction three to six times too large and withdrew it. The displacement
was never the error.

* **was** (8.1): the miss is 51 to 77 m, so the 340 m displacement model is wrong by a factor of
  three to six.
* **is**: the displacement is 313 to 450 m, exactly as predicted. What was wrong is the step from
  displacement to miss - section 5 took a crossing angle of about 42 degrees from a ship heading and
  a commanded heading read at *different times*, and the real angle between the round's track and
  the target's course at closest approach is **5.4 to 11.7 degrees**.
* so **section 8.1's own retraction is retracted**, and section 5's arithmetic is reinstated with
  its conversion corrected.

### 11.2 The model closes to within ten metres

Cross-track miss should be `sin(crossing) * travel`:

| torpedo | `sin(crossing) * travel` | measured | residual |
| --- | --- | --- | --- |
| Mav1 | 42.2 m | 51.5 m | +9.3 |
| Mav2 | 64.9 m | 77.6 m | +12.7 |
| Mav3 | 55.3 m | 66.6 m | +11.3 |
| Mav4 | 63.8 m | 76.8 m | +13.0 |
| Mav5 | 56.1 m | 67.9 m | +11.8 |

Five for five, with a **consistent positive residual of 9 to 13 m** — the same sign and the same
order every time, so it is a systematic offset (the release point sits ahead of the aircraft, and the
round is filed at the drop rather than at the aim solution), not noise. Nothing is left over to
attribute to a lead.

### 11.3 Section 8.2's question, answered: abeam, not through the bow line

At 5 to 12 degrees the round runs **nearly along the target's axis**, so the closest approach is a
lateral separation rather than a pass near the bow or stern. Against half-beams of 5 m (Dunlap) and
8 m (the cruisers), the five rounds missed by **46 to 69 metres of open water**. The "0.94 of the
half-Length" figure in section 8.2.1 is real but reads the wrong way round: a scalar distance that
large only looks marginal because the hull is long, and the geometry says the round was never near
it.

### 11.4 What this does and does not settle about the lead

**Settled**: the misses need no lead to explain them. A zero-lead aim, a 313-to-450 m target
displacement and a 5-to-12 degree crossing angle predict the measured closest approach within 13 m
for all five rounds. Section 8.1's premise 3 argument - "a 51.5 m approach is much closer than a
zero-lead aim against a 15 m/s crosser can produce" - **is withdrawn**: the target is not a crosser
at this geometry, it is a stern chase, and a zero-lead aim produces exactly what was measured.

**Not settled, and this is the standing limit**: all of it measures the *host*, whose aim point is
the substituted present position by construction (section 8.4). The image may still lead; nothing
here can see it. What the census removes is the *evidence for* a lead that section 8.1 thought it
had. The producer question (section 6.3.3) is untouched and still needs a native trace.

Two further facts worth keeping. Premise 2 is half true: Mav1, Mav4 and Mav5 came nearest to the
ship they were aimed at, but **Mav2 and Mav3 came nearest to shore structures** (`Storage, 04 01` at
26.1 m, `Hangar, Small, 04 01` at 17.3 m) while their ordered target Northampton was 77.6 and 66.6 m
away - so the nearest-unit census alone would have mis-attributed two of the five. And Mav1's target
moved 449.9 m, a third more than the others, because Dunlap is the Destroyer and makes
`reference_speed` 19.24 against the cruisers' 16.72.

#### 6.3.4 The out-pointer LEA form, closed with its control — and a much better thread

The one encoding that could hide the writer without a store in the owning function is the
out-pointer: `LEA reg,[base+0D0h]` handed to a getter that fills `[reg]`, `[reg+4]`, `[reg+8]` -
the shape `009D0670` itself has from the other side. It **was** in the census, run image-wide, and
here is the accounting it was missing:

* `8D ?? D0 00 00 00`: **29 hits image-wide**, listed in full. Positive control: the sibling form
  `8D ?? AC 00 00 00` (a `+ACh` member of the same objects) returns **21**, so MSVC does emit this
  encoding for members at this displacement range.
* Of the 29, **none is in the `009C`/`009D` torpedo band**. Twenty-six are effects, GUI, HUD,
  collision, shader and vertex-pool code. The three in bot bands are:
  * `009A6FB3` in `FUN_009A6D40` - a Lua property descriptor, not an out-pointer (section 6.3.3);
  * `009B493E` in `FUN_009B4690` - **also** a Lua property descriptor, same shape:
    `SUB ESP,8 / MOV [EAX],5 / LEA ECX,[EDI+0D0h] / MOV [EAX+4],ECX` then
    `MOV [EAX],0 / MOV [EAX+4],00D200AC`;
  * `009B5C50` - Ghidra answers "No instruction at address", so the byte match is inside a region it
    has not disassembled and the containing-function attribution is the nearest-preceding artefact
    again.

So the out-pointer form is genuinely absent for `approach+D0h` in the torpedo class, and **the static
hunt is closed** as section 6.3.3 said, now with the control the claim needed.

**But the second descriptor names the field, and the name is worth more than the negative.**
`00D200AC` is `"calcHitPos"`. `FUN_009B4690`'s class registers its own `+D0h` as a **calculated hit
position** - not a target position, a *computed* one. `009B5760`, in the same band, is one of
`009FD570`'s six callers, and `009B5C80` and `009B6670` read `[ECX+0D0h]` as a float exactly the way
`009D0670` does.

The same caveat as `"inattackrange"` applies and must not be waved away: this is a **different
class**, `FUN_009B4690` is not `BotApproachTorpedo`, and a name at the same offset in a sibling is
not a name in this one. What makes it worth recording anyway is that `"inattackrange"` and
`"calcHitPos"` sit at the *same offset in two different classes*, which is itself evidence that
`+D0h` is **not** a shared base-class field and that each class uses it for its own purpose - so the
torpedo's `+D0h` has to be named from the torpedo's own code, and neither name transfers.

**The thread for the next packet**, and it is the best one this stream has: find who computes
`calcHitPos` in `FUN_009B4690`'s class. If that producer takes a target position and a time and
returns a point ahead of it, the image leads, and the same routine is the first place to look for
the torpedo's `+D0h`. If it copies a position, it does not. Either way it is a body to read rather
than a byte pattern to scan, which is where section 6.3.3 said this had to go.

## 12. `009D4C10` read whole: the ordnance byte gates the break-off, and section 10.3 is wrong

Answering "why is the break-off test false at the tick the range first exceeds 700" found two things:
the host's answer, and the arm of `009D4C10` I had left unread.

### 12.1 The host answer: it is not computed at all

`src/game_hosts_units.cpp:4074` is `in.should_break_off = false;`, and the vtable binding at
`:3901`-`:3904` is

```
bool should_break_off(void*) override {
    // task->vtable[1Ch] == 009D4C10. contract: unread.
    record("BotTaskTorpedo::should_break_off", "009d4c10");
    return false;
}
```

**Hardwired false, because the contract was unread when it was written.** So `009D4030`'s first test
has never fired in any run this stream has taken, whatever the range. That is the direct answer to
the question and it is not about `approach+24h` or about ordering.

### 12.2 The arm I had not read is the ordnance byte

Sections 7.4 and 10 read `009D4C10` except for `009D4C5C`-`009D4C63`. Its bytes are
`80 BE 2A 05 00 00 00 75` = **`CMP byte ptr [ESI+52Ah],0` then `JNZ 009D4C1D`** (return false). The
routine whole:

```
if (!0099C230(this))                         return false;
if (target == 0 || target->+5Dh)             return true;    ; target gone
if (ctl->+369h && [00E17BF2])                return false;
if (IsAttackState(current) && task+52Ah)     return false;    ; STILL HAS ORDNANCE
return (range > SafeDist * approach+24h);
```

`task+52Ah` is `approach+132h`, the has-ordnance byte (section 10.2). So **the image's break-off is
gated on ordnance**: an aircraft that still has a torpedo never breaks off by range, and one that has
dropped does, as soon as the range opens past `SafeDist * approach+24h`. That is the same shape as
the dive bomb's `009C8A90`, which `src/dive_bomb_task.cpp:791`-`804` already reconstructs with
`!has_bomb_ordnance_4c9` guarding its range test; the torpedo adds the `IsAttackState` conjunct.

### 12.3 Section 10.3 is falsified by 12.1

Section 10.3 explained the decrement run by "clearing the byte released the aim hold while
`009D4C10` was already true, so the first test retired the task". **`009D4C10` is hardwired false in
this host**, so that test cannot have fired and the explanation is impossible.

* **was** (10.3): the break-off test retired the task before the aim branch was reached.
* **is**: it could not have. The route the decrement run actually took to `done` is **unexplained**,
  and this document does not guess at it a third time. What is certain is that with
  `should_break_off` pinned false, `009D4030`'s aim branch with the byte clear returns `kGoAway` —
  and the run recorded `goaway enters=0`, so something before the switch diverted it. The candidates
  are `engaged` (`009D3210`) going false and the `!attacking` arm, both of which this packet has not
  instrumented.
* 10.1 and 10.2 are untouched: the one call site with its zero dword scan, and the one byte with two
  names, are both from the listing and stand.

### 12.4 What the fix actually is, and why it is two parts not one

The image retires a spent bomber through `009D4C10`, and `009D4C10` needs **both** halves:

1. `should_break_off` bound to the routine above instead of `false` — the host has the target
   pointer, the target's death byte, `range_90`, `Pilot/Torpedo/SafeDist` and a `speed_ratio` whose
   formula is known (`max(desc+188h MaxSpd / the reference at +4D8h, 1.0)`, so it is at least 1.0
   and the threshold is at least 700); and
2. the ordnance byte clearing on a drop — **the decrement this document reverted in section 9**.

Neither alone does anything. With only (2), as section 9 measured, the goaway is lost and deaths go
back to 5. With only (1), the byte stays set for the whole mission (section 7.2), the
`IsAttackState && task+52Ah` arm returns false on every tick, and nothing changes at all. **That is
why section 9's experiment failed, and it is a better reason than either section 9.2 or section 10.3
gave.**

Both parts together are the spent-bomber fix, and its before/after is already defined: `goaway`
entries 82 to one per aircraft, task reaching `done`, deaths no worse than 2. Not attempted here —
it is a binding plus a revert-of-a-revert plus a mission-length run, and this packet is at its
context limit. The reading it needs is complete and is in this section.

## 13. Two "headings" for one ship, and it is where section 5's 42 degrees came from

Item 2 asks for each Mav's bearing relative to its target's course at attackrun entry. Taking it
from the log the obvious way gives a number that **contradicts section 11**, and chasing that
contradiction found the original error.

### 13.1 The contradiction

`ship ai step` reports Dunlap's `heading= -1.1519` through step 1200 and `-1.0472` from step 1600.
Mav1's `yaw_C6C` over its run-in is `-1.8795` at aim tick 1 (range 2199) easing to `-1.8254` at tick
251 (range 459). Differencing those two columns gives **40 to 45 degrees** throughout, and it barely
moves. Section 11 measured the crossing at closest approach as **5.4 degrees**. Both cannot be the
angle between the same two directions.

### 13.2 The census is the commensurable one, checked in the source

`GameGunneryHost` computes the round's track as `atan2(velocity.x, velocity.z)` and the target's
course as `units.unit_heading_radians(i)`, which is
`GameUnitsHost::Impl::pose_heading_radians` = **`atan2(pose_row2[0], pose_row2[2])`**
(`src/game_hosts_units.cpp:1427`-`1430`) — `atan2(fx, fz)`, the same convention, on the hull's own
pose row. So the two are the same kind of angle and section 11's number is sound.

`ship ai step`'s `heading` is a **different field**: the ship-AI control block's own heading, which
on this placement sits some 0.7 to 0.9 rad away from the hull's pose heading. Differencing an
aircraft's `yaw_C6C` against it is comparing two quantities that are not the same angle — the id
spaces trap of section 8.3 in another dress, this time in radians.

**So the 40-to-45-degree figure is withdrawn before it is used**, and section 11 stands unchanged.

### 13.3 This is where section 5's 42 degrees came from

Section 5 derived its crossing angle by comparing `heading = -1.0472` from `ship ai step` against
`cmd_2C0` from the aim census, and got about 42 degrees — which is exactly the difference the two
*incommensurable* fields produce. That single comparison is the root of the whole chain: it made the
340 m displacement look like a 220 m cross-track miss, which made the measured 51 to 77 m look three
to six times too small, which produced section 8.1's retraction of a prediction that was right. One
wrong pairing of fields, three documents of consequences.

### 13.4 An independent check that the travel measurements are sound

The census's `target_moved` divided by its own time, against each class's authored
`reference_speed`:

| torpedo | target | travel / time | speed | `reference_speed` | fraction |
| --- | --- | --- | --- | --- | --- |
| Mav1 | Dunlap | 449.9 / 26.45 | 17.01 m/s | 19.24 | 0.88 |
| Mav2 | Northampton | 345.2 / 22.60 | 15.27 m/s | 16.72 | 0.91 |
| Mav3 | Northampton | 349.1 / 22.85 | 15.28 m/s | 16.72 | 0.91 |
| Mav4 | SaltLakeCity | 313.4 / 21.60 | 14.51 m/s | 16.72 | 0.87 |
| Mav5 | SaltLakeCity | 317.1 / 21.85 | 14.51 m/s | 16.72 | 0.87 |

Five ships at 87 to 91 per cent of their authored reference speed, and the two SaltLakeCity rounds
agreeing to two decimal places. That is a consistency check on the census rather than a new fact, and
it also retires section 6.4's `d32c`-derived "15.0 m/s, as a lower bound": the real figures are
14.5 to 17.0 m/s, so the bound held.

### 13.5 What item 2 still needs, and it is one field

The bearing at **attackrun entry** cannot be taken from this log at all: nothing prints the target's
pose heading, and the only per-tick heading in the log for a ship is the ship-AI field that must not
be used. The census already computes `pose_heading_radians` for the target at closest approach; the
same value at the drop, plus the aircraft's own `yaw_C6C` at the drop, is what item 2 needs, and it
is two more fields in the structure section 8.1 already extended.

Whether the run-in is a stern chase **by the image's design or by pure pursuit** is therefore still
open. The pure-pursuit reading is the cheaper hypothesis and it predicts something this log can
almost test: aiming continuously at a target's present position turns any approach into a tail chase
as range closes, with no state ever choosing it. Section 11's angles at closest approach — 5.4 to
11.7 degrees, all small, all the same sign of smallness across three different ships and two classes
— are what that looks like. Not proved here.

## 13. `009D4C10` re-read independently, `0099C230` named, and the three vtables of `BotTaskTorpedo`

Packet `cc8_torpedo_breakoff`. Section 12 was read by the previous worker and acted on here, so the
first thing this packet did was re-derive it from the listing rather than trust it. It holds in
every particular. What follows is the confirmation, two things section 12 did not have, and one
correction to how the vtable slot has been described throughout this document.

### 13.1 Confirmation: every jump sense of `009D4C10`, from the branch bytes

Body `009D4C10`-`009D4C8E`, `__thiscall(task)`. Both exits are a bare `RET` with no immediate, and
the entry is `PUSH ECX / PUSH ESI / MOV ESI,ECX`, so it takes `this` in ECX and no stack argument.

| address | bytes read as | taken means |
| --- | --- | --- |
| `009D4C1B` | `JNE 009D4C22` | `0099C230` false falls to `009D4C1D XOR AL,AL` -> **false** |
| `009D4C2A` | `JE 009D4C8A` | target pointer null -> `MOV AL,1` -> **true** |
| `009D4C30` | `JNE 009D4C8A` | `target+5Dh` set -> **true** |
| `009D4C3F` | `JE 009D4C4A` | `ctl+369h` clear **skips** the global test |
| `009D4C48` | `JNE 009D4C1D` | `ctl+369h` and `[00E17BF2]` both set -> **false** |
| `009D4C5A` | `JE 009D4C65` | not in an attack state **skips** the ordnance test |
| `009D4C63` | `JNE 009D4C1D` | in an attack state and `task+52Ah` set -> **false** |
| `009D4C88` | `JA 009D4C1D` | `SafeDist * ratio > range` -> **false**, else fall to **true** |

The range arm in full, and every width is from the loading instruction:

```
009d4c65: fld   dword ptr [esi+488h]     ; FLOAT, 4 bytes -- `dword ptr`
009d4c6b: fstp  dword ptr [esp+4]        ; spilled across the call, the x87 stack freed
009d4c6f: call  0042E740                 ; BSP_GameTuning_GetSingleton
009d4c74: fld   dword ptr [esp+4]        ; st0 = range
009d4c78: fld   dword ptr [eax+438h]     ; st0 = Pilot/Torpedo/SafeDist, st1 = range
009d4c7e: fmul  dword ptr [esi+41Ch]     ; st0 = SafeDist * speed ratio
009d4c84: fcompi st(1)                   ; compare threshold against range
009d4c86: fstp  st(0)
009d4c88: ja    009D4C1D                 ; threshold > range -> false
```

So the predicate is `range >= SafeDist * ratio`, not `>`. `FCOMPI` sets CF/ZF the way `COMISS` does,
so an unordered compare leaves `JA` untaken and the routine returns **true** on a NaN range.

Every offset checks out arithmetically against the approach at `task+3F8h`: `4C4h-3F8h = CCh` the
target pointer, `404h-3F8h = 0Ch` the control block, `488h-3F8h = 90h` the range, `41Ch-3F8h = 24h`
the speed ratio, `52Ah-3F8h = 132h` the ordnance byte. `task+310h` is the state pointer and is not
an approach field. The call at `009D4C53` is `009D31D0`, which the ledger already carries as
`BSP_BotTaskTorpedo_IsAttackState`; it is reached as `__thiscall(task, current_state)`.

**Section 12 is confirmed in full and nothing in it is withdrawn.**

### 13.2 `0099C230` is the player-aircraft exemption, and that makes `base_gate` a proof here

Section 12 carried `0099C230` as an unnamed base gate. Its body, `0099C230`-`0099C266`:

```
eax = [00E198C4];          if (eax == 0)            return true;
                           if ([eax+8Ch] == 0)      return true;
eax = [eax+8Ch];           if ([eax+4] == 0)        return true;
eax = [eax+34h];           if (eax == [ecx+2F4h])   return false;
                           if (eax != [ecx+2FCh])   return true;
                                                    return false;
```

`00E198C4` is the **in-mission interface manager** (`docs/FRONTEND_MANAGERS.md:11`,
`docs/FIXED_STEP_FANOUT.md:187`, `docs/FRONTEND_STATE_MACHINE.md:99`). So the routine returns false
only for the task whose unit is the one the in-mission interface is attached to: **the break-off is
disabled for the player's own aircraft**, and for whichever second unit `task+2FCh` holds.

Two things follow.

* It is shared, not torpedo-specific, and this is from an **exhaustive** rel32-plus-absolute-dword
  census (`tools/callsite_census.py`), not from `bsp.py ghidra callers`, which under-reports. The
  census finds **exactly five** direct call sites and they are the five ordnance break-off bodies:
  `009A65F6` in `009A65F0` depth charge, `009AE1B3` in `009AE1B0` drop-kamikaze, `009B8D86` in
  `009B8D80` level bomb, `009C8A96` in `009C8A90` dive bomb, and `009D4C14` -- which the census
  attributes to `009D4A70` for the same reason section 13.3 gives, that `009D4C10` is not a defined
  function. That is independent confirmation that `009D4C10` is the torpedo's `ShouldBreakOff`,
  arrived at without the vtable.
* The census also returns **22 `.rdata` references** that the caller query never showed, so
  `0099C230` is itself a virtual occupying 22 vtable slots across the image. It does not weaken the
  reading: all five break-off bodies reach it by a **direct** `CALL`, not through a slot, so they
  always get this implementation whatever a derived class puts in its own slot. It does mean the
  routine is a shared base predicate rather than a private helper of these five, and any future
  packet that meets it through a vtable must check that slot rather than assume this body.
* **`base_gate = true` in this host is a proof, not a stand-in.** `bsp_game` publishes no in-mission
  interface manager for an AI-flown Mav, and with `[00E198C4]` null the image takes its own first
  branch and returns true. The host is not substituting for an unread contract here; it is computing
  what the image computes. The existing dive-bomb binding at `src/game_hosts_units.cpp:1140` already
  passes `base_0099c230 = true` and was right to, for this reason rather than by assumption.

### 13.3 Correction: `009D4C10` is slot `1Ch` of a vtable that is **not** `00D213C0`

Throughout sections 7, 10 and 12 the break-off has been called `task->vtable[1Ch]`, and the aim-point
reader `009D0670` has been called `approach->vtable[0]`. Both are right, but they are slots of two
different vtables, and `00D213C0` -- the address this document has used for "the torpedo approach
vtable" -- is only one of three. `BSP_BotTaskTorpedo_Construct` writes all three, read from the
listing at `009D3080`-`009D30B7`:

```
009d3080: lea   edi, [esi+3F8h]          ; esi = the task, edi = the approach subobject
009d3091: call  009D2DA0                 ; BSP_BotTaskTorpedo_ConstructStates, ecx = edi
009d30a1: mov   dword ptr [esi],      0x00D213C8   ; the TASK vptr
009d30a7: mov   dword ptr [edi],      0x00D213C0   ; the APPROACH vptr, at task+3F8h
009d30ad: mov   dword ptr [esi+530h], 0x00D213BC   ; a third subobject vptr, at task+530h
```

This also settles that the approach really is embedded at `task+3F8h` from the constructor's own
`LEA`, rather than only by the offset arithmetic that section 10.2 used.

So the layout is three adjacent vtables, and the approach's is two slots wide:

| vtable | belongs to | slots |
| --- | --- | --- |
| `00D213BC` | the subobject at `task+530h` | 1 (not read this packet) |
| `00D213C0` | the approach at `task+3F8h` | 2: `009D0670`, then **null** |
| `00D213C8` | the task itself | 27, listed below |

`009D0670` is slot 0 of `00D213C0`, so its `ECX` is the **approach**, and it reads
`approach+D0h/+D4h/+D8h` -- three floats copied to an out-pointer at `[ESP+4]`. The document's
naming has been right; only the vtable address attached to it was doing double duty.

**The task vtable `00D213C8`, slot by slot.** Recorded whole because the dive-bomb and depth-charge
classes need the same table and this is the cheap moment to take it.

| slot | target | slot | target | slot | target |
| --- | --- | --- | --- | --- | --- |
| `00h` | `009D4E10` | `24h` | `009D49A0` | `48h` | `007B4130` |
| `04h` | `009D4230` | `28h` | `007B4100` | `4Ch` | `009D4970` |
| `08h` | `007B40C0` | `2Ch` | `009D3270` | `50h` | `009D49E0` |
| `0Ch` | `009D4A30` | `30h` | `0099B6F0` | `54h` | `009D4A70` |
| `10h` | `009D3290` | `34h` | `0099B700` | `58h` | `009D3E80` |
| `14h` | `009D32A0` | `38h` | `0099B710` | `5Ch` | `007B4150` |
| `18h` | `009D4C90` | `3Ch` | `009D3280` | `60h` | `0099D060` |
| **`1Ch`** | **`009D4C10`** | `40h` | `009D3EF0` | `64h` | `009D4850` |
| `20h` | `007B40F0` | `44h` | `007B4120` | `68h` | null, the end |

Slot `1Ch` is `009D4C10`, exactly as section 7.1 said. Slot `54h` is `009D4A70`, which is why Ghidra
offered `BSP_BotTaskTorpedo_UpdateCruiseProfile` as the "enclosing candidate" when asked about
`009D4C10`: `009D4C10` is not a defined function in the project, and the attribution was the
nearest-preceding artefact again.

### 13.4 The sibling-vtable route to the aim point does not open, and here is why

The plan was: find the class that registers `"calcHitPos"` at its own `+D0h` (section 11's
`FUN_009B4690`), find which vtable slot produces it, and read the same slot of the torpedo's approach
vtable. Carried out, it fails at the second step, and the failure is worth more than a guess.

`FUN_009B4690` is called from `FUN_009B77C0`, which is a constructor of the same three-vptr shape:

```
009b77ed: call  009B4690
009b77f2: lea   edi, [esi+0DCh]          ; this class's approach subobject is at task+DCh
009b7817: mov   dword ptr [esi], 0x00D20204    ; the TASK vptr
009b781d: mov   dword ptr [edi], 0x00D20200    ; the APPROACH vptr
```

Its approach vtable `00D20200` is **one slot wide** and that slot is `009B77B0`, which is an
adjustor thunk:

```
009b77b0: sub   ecx, 0DCh
009b77b6: jmp   009B7970
```

`009B7970` is a scalar deleting destructor (`call 009B6480`, the `test byte [esp+8],1` /
`operator delete` tail, `RET 4`). So this sibling's approach-subobject vtable carries **only the
adjustor-thunked destructor**, while the torpedo's carries `009D0670` and a null and no destructor
thunk at all.

**The premise "sibling approach classes share a vtable layout" is therefore false for this pair**,
and there is no "same slot" to read across. `"calcHitPos"` names a field of a class whose secondary
vtable has one entry that is not a getter; nothing about slot numbering transfers to `00D213C0`. The
caution sections 11 and 6.3.3 attached to the name -- that it is a different class and the name does
not transfer by offset -- turns out to understate it: the *vtables* do not correspond either.

**What did come out of it: the writer census for `approach+D0h` is now closed on all three
encodings.** Section 11 closed the out-pointer `LEA` form with its control (29 image-wide, none in
the torpedo band, positive control `+ACh` = 21). This packet ran the two store forms it had not:

| form | encoding | image-wide | in the `009D` torpedo band |
| --- | --- | --- | --- |
| x87 store/load at `+D0h` | `D9 ?? D0 00 00 00` | 109 | one only: `009D0674`, the `FLD` **inside** `009D0670`. No `FSTP`. |
| integer store at `+D0h` | `89 ?? D0 00 00 00` | 59 | none in `009D0000`-`009D5000`; the nearest, `009DBE62` `MOV [ESI+D0h],ESI`, is an array-ctor helper past the band and stores a pointer, not a float |

**A band correction while this is being recorded.** Section 11 and section 6.3 both wrote "the
`009C`/`009D` torpedo band". `009C` is **not** torpedo: `009C8A90` is
`BSP_BotTaskDiveBomb_ShouldBreakOff` and `009CCED0` is reached from
`BSP_BotTaskStrafe_UpdateCruiseProfile`. The torpedo band is `009D`, and the existing
`docs/TORPEDO_APPROACH_UPDATE.md` bound is the right one: `009D0000`-`009D5000`. The earlier census
conclusions are unaffected -- a hit in `009C` was never a torpedo hit either way -- but the label was
wrong and anything counting on it should use `009D`.

Both have healthy image-wide counts, so neither negative is vacuous. The x87 form matters
specifically: **Capstone reports `FSTP [mem]` as a read**, so a census built on operand-access flags
would have classified every `D9 9x` store as a read and missed the form entirely. Scanning the
displacement bytes avoids that, and the sibling bands prove the form occurs on this very field --
`009B4636` and `009B53D9` (`FSTP [ESI+D0h]`), `009CA604`, `009CCF16` and `009CCFA1` in classes that
all call `009F9CE0 BSP_BotApproach_ConstructSpeedReference` and so are approach classes too.

So: **sibling approach classes do write their own `+D0h`; the torpedo class never writes its own by
any instruction that names the displacement.** What remains open is a block copy or a write through a
pointer aliased in a register, neither of which names `+D0h` -- the standing caveat, not a new one.
Taken with section 6.2 (`009D0670` does no arithmetic) and section 5, the reading this supports is
that the torpedo's `+D0h` is a **dead field** and the image does not lead. That is consistent with
section 11's measurement, which closed the misses as a stern chase with no lead term needed. It is
**not proved**: "no writer names the displacement" is not "no writer".

### 13.5 The run-in direction: the image does not fly to a beam position, and the stern chase is faithful

The question this packet was set is whether some torpedo state puts the bomber onto an attack line
before `attackrun`, using `009FD570`'s standoff ring with a `side` and an offset ramping 0 to pi --
which is how one flies from astern of a ship to its beam. If the image did that and this host did
not, section 11's 5.4-to-11.7-degree crossing angles would be a host gap and binding it would fix
the misses.

**It does not, and the answer was already in two documents that had not been put side by side.**

* `009FD570` has six callers and `docs/TORPEDO_FLY_TO_SOLVER.md` section 5 tabulates all six. Exactly
  one is in the torpedo band: `009D0C10 BSP_BotStateTorpedoGoAway_UpdateGeometry`, the **climb-away**.
  No torpedo state calls it before `attackrun`, so the standoff ring is the break-off's geometry in
  this class, never the run-in's.
* The torpedo class does choose a run-in direction, but by terrain. `009D3420` scans thirty-six
  sectors around the target, `sector_clear[i]` meaning a run-in along sector `i` is free of terrain;
  `009D3BC8` takes the sector the unit occupies as seen from the target, and `009D3BCE`-`009D3C56`
  walks to the nearest clear sector each way and takes the cheaper turn. The aim tick then flies
  "the bearing to the target plus the sector turn offset the approach update chose".
* `docs/TORPEDO_RUN_IN_PATH.md` section (4) already read the branch that settles it,
  `009D3C5A`-`009D3C88`: `if (ctl->+58h == 0x24 || ctl->+58h == 0)` -- **all thirty-six clear takes
  the same branch as none clear**, and both write `approach+5Ch = 0`. The gap search that produces a
  non-zero turn offset runs only on a mixture.

`local/breakoff_before_usn01.log` confirms the antecedent for every aircraft:
`clear_sectors=36 of 36`, `turn_5c=0.0000 rad`, on 15 to 18 scan runs each.

**So the target's heading and its velocity do not enter the run-in direction at all.** The only
target-relative quantity in the plan is the *bearing*, and on open water the plan resolves to a zero
turn offset by the routine's own rule. A torpedo bomber in USN01 therefore runs in along whatever
bearing it already holds toward the target's present position -- which is a stern chase when it was
vectored in from astern, exactly what section 11 measured.

**This retires the hypothesis rather than the miss.** There is nothing to bind for the run-in: the
stern chase is the image's behaviour under these conditions, not a gap in this host. Section 11's
geometry still explains the five misses, but the cause cannot be a missing beam approach, and the
remaining candidate is the lead -- which section 13.4 finds no displacement-naming writer for.

**One thing left unverified and flagged rather than waved past.** All five aircraft report
`home_sector=0`. That is consistent with their all lying in the same sector relative to the target,
and it is *moot* for USN01 because the all-clear branch above discards the home sector before it can
matter. But it was not independently checked against the bearings, so it is not evidence that
`009D3BC8`'s binding is right -- only that nothing in this mission depends on it.

## 14. The fix was three parts, not two, and `009D4C10` was never one of them

Packet `cc8_torpedo_retire`. Three USN01 runs, each 3200 frames / 3000 mission frames at 0.05 s,
same launcher arguments, one change under test per run.

### 14.1 What section 12.4 said, and what it got wrong

Section 12.4 says the spent-bomber fix is two parts - `should_break_off` bound to `009D4C10`, plus
the ordnance byte clearing on a drop - and that "both parts together are the spent-bomber fix".
**Both parts were in, and the outcome was deaths 5 and damage 2572.9 against a baseline of 2 and
1501.9.** The retiree's commit `9f0ec5c21` already recorded that as a falsification. This section
says what the fix actually is.

It is three parts, and the first two are not the two of section 12.4:

1. the ordnance byte clearing on a drop (section 12.4's part 2, and it was right);
2. the host's task arm no longer re-testing an install condition on every tick;
3. the goaway enter tail's second leg, which is the only producer of a spent bomber's climb
   altitude, bound instead of reported absent.

`should_break_off` bound to `009D4C10` - section 12.4's part 1 - is **correct and is not part of
the fix**. It changes nothing in this mission, for the reason measured below. It is kept because it
is faithful, not because it repairs anything.

### 14.2 `009D4C10` is innocent, and this is a measurement, not an argument

`local/retire_arm_usn01.log` is a build identical to `9f0ec5c21` apart from additive logging that
latches every input of the binding on the first tick it returns true, and counts the true ticks. It
reproduces that commit's run exactly - same log line numbers, deaths 5, damage 2572.9 - so the
comparison is same-binary in everything that matters:

```
Mav1 arm=5 at_arm_tick=0 true_ticks=1 state=moveto has_target=1 target_plus_one=45
     target_marked=0 in_attack=0 flag_52a=1 ordnance_132=1 range_90=4183.4 safe_dist=700.0
```

`true_ticks=1`. The predicate was true on **exactly one tick per aircraft**: arm tick 0, in
`moveto`, at 4.2 km, where the range arm is legitimately true and where `009D40A6` cannot consume it
because its guard needs an attack state. At the drop it was **false** - correctly, the range there
is about 455 m against a 700 m threshold - and it stayed false for all 678 arm ticks.

Two things recorded elsewhere are withdrawn by that line:

* **The leading suspect of `9f0ec5c21` is refuted.** It proposed that the `target == 0 ||
  target->+5Dh` arm returned true unconditionally because the binding feeds `has_target` from the
  command target rather than from `approach+CCh`. The capture shows `has_target=1`,
  `target_marked=0` and `command_target_plus_one` 44/45/46 on every aircraft. The feed is not the
  fault, it was not changed, and the `approach+CCh` store census the packet held in reserve was not
  needed.
* **No task reached `done` in that run.** The `done_last` column of the goaway census is `009D3150`
  evaluated on the last tick, not the `kDone` state. Reading it as "the task retired" is what made
  the failed run look like a retirement problem. It was not one.

One thing is confirmed rather than withdrawn: `bot_task_should_break_off` applies the target pair
**before** `class_extra`, which is the order of the listing (`009D4C22`/`009D4C2C` target,
`009D4C32`/`009D4C4A` class extra, `009D4C65` range). The generic body needed no change for the
torpedo, and the other four classes were not touched. The census also shows this host's two cells
for `task+488h` - `torpedo_engage_limit_90` and `torpedo_approach.range_90` - printing the same
number on the same tick, which is what section 10.2's "same storage" reading requires of them.

### 14.3 The cause: an install condition tested every tick

`run_torpedo_task_arm_009d4850` opened with

```cpp
if (!bsp::ordnance_has_torpedo_2bh(set)) return;
```

**ahead of** its install block, so the kind `2Bh` test ran on every tick rather than once. As long as
the loadout never shrank - which is the state section 7.2 describes and section 9 reverted to - the
guard was invisible. The moment a drop cleared the bit it fired on the next tick and the whole task
stopped being armed: no state machine, no aim tick, no goaway tick, no state ticks at all. Mav1's
`arm_ticks=678` is exactly `attackrun 424 + aim 254`, and the aircraft held its last commanded
descent into the sea 166 ticks later, at 69.4 m/s and `alt=-0.04`.

That is the whole of the deaths 2 -> 5 and damage 1501.9 -> 2572.9 regression, and it is a host
binding artefact with no counterpart in the image.

The image cannot behave that way, and `009D4C10` is what proves it: its ordnance arm
`if (IsAttackState(cur) && task+52Ah) return false` only lets the range test through once
`task+52Ah` is **clear**, so the image arms this task after the drop. An image that stopped arming a
spent task could never reach its own break-off. `0099A170` makes the kind `2Bh` test once, when the
order is issued, to choose which task class to construct. The test now sits inside
`if (!torpedo_task_installed)`.

The dive-bomb arm does not have this bug: it gates on `dive_bomb_task_installed_for_class`, a
property of the chosen command class, which is stable. A follow-up worth taking is to gate the
torpedo the same way, on `kAttackCmdTorpedo` `00E08F18`, which would also close the symmetric hole -
a unit that carries torpedoes but whose order chose another class.

### 14.4 The second hole, and the field name that hid it for a run

With the task armed again the goaway was entered once per aircraft and still commanded no climb:
`climb_1Ch=0.0 known=0`, 165 ticks all on the post-window **low** arm, `alt_cmd=0.0`,
`alt_range=[0.1,12.0]`, five water contacts, deaths still 5. `009D0E42` sends a spent bomber down
the second leg of the enter tail, and that leg was bound as absent, on the ground that it read a
squadron object this host does not model.

It reads no squadron object. From the listing:

```
009d0e3f  mov  eax,[esi+4]              esi+4 = the approach
009d0e42  cmp  byte ptr [eax+132h],0    the ordnance byte
009d0e4a  je   009d0e7c                 clear -> the second leg
009d0e4c  fld  dword ptr [eax+78h]      set -> band + UniformFloatRange(50,100)
009d0e52  fadd dword ptr [eax+74h]
009d0e7c  mov  ecx,[eax+0Ch]            approach+0Ch
009d0e7f  fld  dword ptr [ecx+394h]     the climb altitude
009d0e88  fstp dword ptr [esi+1Ch]
```

`009F9CE0` sets `approach+0Ch` to `unit+9D4h`, so the base is **the pilot control block** - the same
block `read_control_block()` in `src/game_hosts_units.cpp` already models, and whose `+398h` and
`+39Ch` that function already supplies. The confusion was with `009FBA9B`'s ceiling leg, which reads
a different object's `+394h`.

`ctl+394h` is the desired cruising altitude: step 4 of every task's `+54h` cruise profile writes
`ctl->+394h = <cruising alt>` behind the not-overridden gate (`docs/BOT_TASKS.md`, the constructor
shape and the class table), and for this class that value is `Pilot/Torpedo/CruisingAlt`, tuning
`+430h`. Bound from the live value, with `kPilotTorpedoCruisingAltDefault` only as a fallback. The
run reports `climb_1Ch=500.0 known=1 alt_cmd=500.0` and the aircraft climb from 12 m to 285 m.

**The reconstruction already knew this, and only the binding did not.** The ledger record for
`009D0D90` has carried "`else ctl+394h`" since packet `cc8_torpedo_goaway_release` read the tail's
extent. What went wrong is downstream of the reading: `include/bsp/torpedo_goaway_tick.hpp` named
the input `squadron_alt_limit_394`, and `src/game_hosts_units.cpp` then fed it `false` on the
strength of that name. The lesson is the field name, not a gap in the listing - so the ledger entry
now carries the `009D0E7C` bytes and what the base register is, where a name cannot mislead.

**Uncertainty, stated rather than waved past.** This host models no pilot control block, so it
cannot observe the write and assumes the cruise profile's write landed. That is exactly the
assumption `read_control_block()` already makes for `+398h` and `+39Ch` - no weaker and no stronger.
If the not-overridden gate refused the write, the image would read whatever the motion controller
left there. The input's name `squadron_alt_limit_394` in `include/bsp/torpedo_goaway_tick.hpp` is
now known to be wrong; renaming it reaches two files this packet does not hold, so it carries a
comment naming the error instead.

### 14.5 The measure

| | before | tip as handed over | gate fix only | all three parts |
| --- | --- | --- | --- | --- |
| log | `breakoff_before_usn01` | `breakoff_after_usn01`, reproduced as `retire_arm_usn01` | `retire_fix_usn01` | `retire_climb_usn01` |
| goaway enters, 5 aircraft | 82/80/86/68/69 = 385 | 0 | 1 each | 1 each |
| goaway ticks, Mav1 | 465 | 0 | 165 | 308 |
| tasks reaching `done` | none | none | none | **all five, after the climb-away** |
| `arm_ticks`, Mav1 | 1299 | 678 | 843 | 1299 |
| water contacts | none | 5 | 5 | **none** |
| deaths | 2 | 5 | 5 | **1** |
| damage | 1501.9 | 2572.9 | 2572.9 | **1285.0** |
| `torpedo_loadout_cleared` | absent | 5 | 5 | 5 |
| releases | 5 | 5 | 5 | 5 |

> **The `deaths` and `damage` rows of the last column are superseded from 2026-09-19.** On main
> after the impact burst (docs/TORPEDO_WARHEAD.md section 11.2), USN01 at 3000 mission frames reads
> **damage 3595.4, deaths 4** in place of `1285.0` and `1`. Nothing about the torpedo chain moved:
> `drops=5`, `swims_started=5`, all five tasks with `releases=1` and identical state histograms,
> `goaway` entered once per aircraft, torpedo 1 striking `Hangar, Small, 04 01` at
> `(4165.8, 0.00, -3294.7)` at life 51.75 and the other four expiring at 60.05 - byte for byte the
> same as this column. What changed is that an impact now spawns the class row's `Blast` burst, as
> `0084BC60` step 7 does, so the hangar this run has always hit now takes its warhead and every
> shell with a `Blast` table adds a record. **Every other row of this table still stands**, and they
> are the ones this section's argument rests on. Use the new pair for regressions from now on.

Mav1's shape in the passing run is `attackrun 424 -> aim 254 -> goaway 308 -> done 313`, one goaway
entry, `range_peak_in_goaway=701.3` against `break_off_24h=700.0`. Only Mav5 is lost, sunk at
86.65 s by SaltLakeCity; Mav2 survives on 4 health, Mav3 and Mav4 are untouched. The before run lost
Mav1 and Mav5.

**What a retired bomber then does in this host.** It reaches `kDone` when the goaway completes -
`009D4132`'s `attack_flag_52a ? kAim : kDone`, and with the byte clear that is `kDone` - and from
then on the arm still runs (313 to 385 ticks of it) but this host runs **no state tick for
`kDone`**, so nothing further is commanded and the aircraft holds the goaway's last heading and its
500 m climb for the rest of the mission. `009D4C10` is true throughout that tail (`true_ticks` rises
from 1 to 197-221), which is consistent but inert: `009D4097` returns `kNone` for a task already in
`kDone`, ahead of the break-off test. What the image's own `done` state ticks, and whether the bot
replaces the task afterwards, was not read - it is a separate state object at `task+618h` and not a
cheap read.

### 14.6 The speed ratio: `1.0` is exact here, and section 12.4 named the wrong tuning row

Section 12.4 gives the ratio's formula as `max(desc+188h MaxSpd / the reference at +4D8h, 1.0)`.
`+4D8h` is `Pilot/DiveBomb/ReferenceSpeed`. The torpedo's divisor is `+440h`, and the call site is
explicit:

```
009d03a1  call 0x42e740                 the game tuning singleton
009d03a6  fld  dword ptr [eax+440h]     Pilot/Torpedo/ReferenceSpeed, KMH(300)
009d03b1  fstp dword ptr [esp]          -> 009F9CE0's third argument
009d03b4  push eax                      the unit
009d03b7  call 0x9f9ce0
```

inside `BSP_BotApproachTorpedo_Reset`, which is where the torpedo's approach controller is built.
Measured live in all three runs:

```
max_spd_188h=69.44  reference_speed_440h=83.33  ratio_41Ch=1.0000  break_off_threshold=700.0
```

`69.44 / 83.33 = 0.833`, and `009F9CE0`'s `if (1.0 < r)` keeps the constant, so the `1.0` the
binding carries is **the image's own value for this aircraft**, not a floor beneath it, and the
break-off threshold is exactly 700.0 m. Nothing to bind and nothing to re-measure. The dive-bomb
side's `in.speed_ratio_41c = 1.0f` is the same quantity through a different row and is not settled
by this; its divisor is `+4D8h` and its numerator is its own class's `MaxSpd`.

### 14.7 The run-in crossing angle, measured at both ends of the swim

Section 11 measures a 5 to 12 degree crossing angle at the closest approach and reads the misses as
a stern chase. The angle the aircraft *starts* the run-in with had never been measured, so it was
not known whether that geometry is inherited or produced. Both ends are now censused in one
convention - hull **pose** headings at both, `atan2(pose_row2.x, pose_row2.z)`, the same quantity the
closest-approach crossing already differences the round's track against, and never the ship-ai step
heading section 13 warns about.

| aircraft | crossing at attackrun entry | at the drop | at closest approach |
| --- | --- | --- | --- |
| Mav1 | 0.8397 rad (48.1 deg) | 0.0945 (5.4) | 0.094 |
| Mav2 | 0.6150 (35.2) | 0.1888 (10.8) | 0.189 |
| Mav3 | 0.6150 (35.2) | 0.1592 (9.1) | 0.159 |
| Mav4 | 0.6150 (35.2) | 0.2050 (11.7) | 0.205 |
| Mav5 | 0.6150 (35.2) | 0.1776 (10.2) | 0.178 |

1. **The crossing angle collapses during the run-in**, from 35-48 degrees at attackrun entry to
   5-12 at the drop. The stern chase is **not** the geometry the run-in starts from; the run-in
   steering produces it. The pursuit curl is real and it lives in the attackrun.
2. **The drop angle is the closest-approach angle**, to three decimals on all five. That is two
   facts at once: the round holds its launch heading through a 21-26 s swim, and the ship's course
   barely moves over that window. The round leaves along the aircraft's nose, so the aim error at
   the drop *is* the miss.
3. **Both ends move, and the ship more than the aircraft.** The target's pose heading goes from
   -0.6155/-0.8397 at entry to -1.6474/-1.7297 at the drop, about a radian of turn, while the
   aircraft goes from its spawn heading of 0.0000 to -1.82/-1.88. The aircraft tracked a turning
   ship around and finished behind it.

**Caveat on row 1.** `entries=1` and the aircraft's heading at entry is still its spawn heading, so
the attackrun is entered before the aircraft has turned at all. The 35-to-48 degree figure is the
initial offset between a spawn heading and a target course, not a geometry the task chose. What the
table settles is that the convergence happens inside the attackrun; **whether the steering chases
the ship's current course or a lead point is the next question, and this census does not answer
it.**

## 15. The six USN04 torpedoes never reach the water: they die on their own squadron mates one tick after the drop

Packet `cc8_torpedo_swim`. `local/swim_trace_usn04.log` is the tip binary plus additive per-round
logging, keyed to the id on each round's own `torpedo drop N` line. It reproduces the defect
`docs/HANDOFF_TORPEDO_SWIM_START.md` handed over - `drops=6 refusals=0 water_entry_breakups=0`,
`swims_started=0`, `torpedo_closest_approach swims=0` - so the comparison below is same-binary in
everything that matters.

Note for anyone reading the two USN04 logs side by side: this mission is **not** deterministic
run to run. The handed-over run dropped squadron 2 first at mission frame 2799 and ended with
`deaths=1 total_damage=721.3 created=492`; this one drops squadron 4 first at frame 2791 and ends
with `deaths=2 total_damage=935.6 created=677`. The six drops and the zero swims reproduce; the
gunnery totals do not, and no before/after argument may rest on them.

### 15.1 What became of the six rounds, from the trace

Every one of the six exits on its **first** step, at `life=0.05`, as an entity impact on an
aircraft of its own squadron:

```
torpedo trace 1 spawn by B5N Kate #4.1      at=(12193.9,11.76,-12547.9) vel=(53.4,0.04,-60.8) bullet=69
torpedo trace 2 spawn by B5N Kate #4.1|.-2  at=(12193.9,11.76,-12547.9) vel=(53.4,0.04,-60.8) bullet=69
torpedo trace 3 spawn by B5N Kate #4.1|.-3  at=(12193.9,11.76,-12547.9) vel=(53.4,0.04,-60.8) bullet=69
torpedo trace 1 t=33.05 life=0.05 from_y=11.76 pos=(12196.6,11.75,-12550.9) vel=(53.4,-0.45,-60.8) swimming=0
torpedo trace 1 exit=entity_impact hit=B5N Kate #4.1|.-3 at=(12193.9,11.76,-12547.9) life=0.05
torpedo trace 2 exit=entity_impact hit=B5N Kate #4.1|.-3 at=(12193.9,11.76,-12547.9) life=0.05
torpedo trace 3 exit=entity_impact hit=B5N Kate #4.1|.-2 at=(12193.9,11.76,-12547.9) life=0.05
```

and the same three lines again for squadron 2 eight frames later, rounds 4, 5 and 6 hitting
`B5N Kate #2.1|.-3`, `#2.1|.-3` and `#2.1|.-2` at `(-12624.1,11.78,-12621.3)`.

Two things in those lines are the whole finding:

* **the three aircraft of a squadron report one position and one velocity at the drop**, to every
  digit printed, and the round spawns at its own aircraft's pose origin
  (`src/game_hosts_gunnery.cpp`, the labelled release-geometry substitution);
* **the impact point equals the spawn point.** `SegmentBinding::shape_trace_segment` clamps the
  slab test's `enter` at `0`, so a hit reported at the segment's start means the segment **began
  inside** the mate's hull box. The round is dead before it has fallen one centimetre: the step
  it dies on took it from `y=11.76` to `y=11.75`.

The sweep excludes exactly one unit, the owner, so the two co-located mates are live candidates.

**The closest-approach census and both water arms are downstream of this and are not defects.**
No round reached the water block at all, which is why `water_entry_breakups=0` *and*
`swims_started=0`; the census skips rounds that are not swimming.

### 15.2 Two suspects cleared, from the same run

* **`0078CF20` is not involved.** This host never calls the ocean sampler: the crossing test in
  `run_projectiles` is the literal plane `shot.position[1] <= 0.0f && from[1] > 0.0f`. The plane
  is reachable in this scene - the same run counts `water=44` crossings by other rounds in Coral
  Sea - so "whether this scene answers as USN01's does" has no bearing on the six drops. The
  handoff named it as a suspect, not a finding; it is now cleared.
* **The drops are not too late in the mission.** They land at mission frames 2791 and 2799 of
  3000, leaving 209 and 201 frames = 10.45 s and 10.05 s, against a 1.55 s free fall from 11.76 m (g = 9.81038)
  (`projectile_integrate_position_006e7670`, no drag). A 4500-frame run would change nothing.

### 15.3 The image gives a dropped torpedo no friendly-fire exemption, so this is not the fix

`0084BF00` step 6 is the only friendly-fire arm on this path, and it is **not** a torpedo rule.
From the listing:

```
0084c1d3  CALL 0098b370                  ; the entity sweep, AL = hit
0084c1d8  MOV  ESI,[ESP+0x44]            ; the owner's +9D4h, latched at 0084bf60
0084c1e4  JZ   0084c251                  ; no control block -> keep the hit
0084c1f3  PUSH 0xf
0084c1f5  CALL EAX                       ; hitEntity->vtable[5Ch](0Fh), is it a plane
0084c1fd  JZ   0084c213                  ; not a plane -> keep the hit
0084c1ff  MOV  EDX,[ESP+0x64]            ; the class descriptor
0084c203  CMP  dword ptr [EDX+0x8],0x11  ; the weapon sub-type
0084c207  JNZ  0084c213                  ; not 11h -> keep the hit
0084c209  CMP  dword ptr [ECX+0x9d4],ESI ; same control block as the owner
0084c20f  JNZ  0084c213
0084c211  XOR  BL,BL                     ; -> discard the hit
```

`classDesc[+8h] == 11h` is **Kamikaze** (`kProjectileSubTypeKamikaze`, `include/bsp/projectile_kinds.hpp`).
A torpedo is `0Ah`. The arm cannot fire for a torpedo, so binding step 6 into this host would
neither be faithful nor move the measure.

The sweep's own exclusion is one entity, not a formation: `0084C1B3`-`0084C1BF` loads
`EDI = owner->vtable[B0h]()`, which `docs/HIT_NARROWPHASE.md` and `docs/EXPLOSION_RADIAL_DAMAGE.md`
both read as the shooter's **collision root**, compared with `!=` against one entity and its
children at `0098AEE4` / `0098ACE3`. Nor can the wing members be sharing the leader's collision
node (which `0087BDC9`'s parent-chain walk allows for a child that has none): the AA guns hit and
damage each of the six Kates individually in this run, so each carries its own collision presence.

### 15.4 The cause is the missing formation offset, and it is not in the torpedo chain

The three aircraft of a `SpawnNew` squadron sit on one point because this host has no formation
ring. `src/game_hosts_ai.cpp`'s `tick_request_join_formation` records `0077C8D0`
(`BSP_Entity_RequestJoinFormation`, `contract: unread`) and returns; nothing ever displaces a wing
member from its leader. The image's own spacing for this is `Formation_UnitDist = 300.0`
(`00CE3AE8`, `src/ai_tuning_globals.cpp`), at which no dropped torpedo could be inside a mate's
hull box.

**So the fix for this defect is the formation ring, not anything in the torpedo chain**: binding
`0077C8D0` behind `tick_request_join_formation` so a wing member is displaced from its leader.
There is no faithful change in `src/game_hosts_gunnery.cpp` or the torpedo task files that reaches
`swims_started=6`, and 15.3 is the record of the one candidate that looked like a torpedo-side fix
and was refuted from the listing before it was written. Until the ring exists, USN04 cannot measure
the torpedo chain end to end without the probe of 15.5.

That also settles the question the handoff left open about the wingmen reporting numbers identical
to the last digit. **It is neither the flight-lead binding nor three tasks sharing one solution.**
The torpedo task's inputs are filled per unit from the unit's own position -
`host.unit_world_xz(unit_xz)` then
`state.range_90 = torpedo_approach_range_009d3519(unit_xz, target_xz)`
(`src/torpedo_approach_update.cpp:350` and `:368`, `009D357E`), reached through a binding built
on the calling unit's own `slot_` (`src/game_hosts_units.cpp:4106`) - so identical outputs are the
arithmetic of identical **inputs**. `range_peak 428.2` three times is three aircraft at
one place, not one solution copied three ways.

USN01 is consistent with this and is not a counter-example: its five torpedo bombers are
independent units (`Mav1`..`Mav5`, no `|.-n` wing suffix), so none of them has a co-located mate
to hit, and all five swim.

### 15.5 A labelled probe, to show that nothing else in the chain is broken

`local/swim_probe_usn04.log` is the same binary with one extra block in the projectile step,
**deliberately not the image's rule and not committed**: for traced torpedo rounds only, an entity
hit on a unit of the owner's own side is skipped instead of applied. It exists to answer one
question the defect hides - what the rest of the chain does once the six rounds are not killed on
their own squadron mates - and 15.3 is the reason it is a probe rather than a fix.

Under it, on USN04, 3000 mission frames:

```
summary mission gunnery torpedo_drop  drops=6 refusals=0 water_entry_breakups=0
summary mission gunnery torpedo_ranges_derived=34 swims_started=6 snaps=0
summary mission gunnery torpedo_closest_approach swims=6
torpedo from B5N Kate #2.1    nearest Lexington-class01 min=55.4 m at t=9.75 s of 9.75 s run
      | ordered Lexington-class01 min=55.4 m at t=9.75 s target_moved=0.0 m crossing=2.857 rad
      | at the drop: own_pose=-2.4137 target_pose=1.0123
torpedo from B5N Kate #4.1    nearest Yorktown-class01  min=34.0 m at t=10.45 s of 10.45 s run
      | ordered Yorktown-class01 min=34.0 m at t=10.45 s target_moved=0.0 m crossing=2.527 rad
      | at the drop: own_pose=2.4211 target_pose=-1.3351
```

with the other two rounds of each squadron reporting the same numbers, as 15.4 requires of three
aircraft at one point.

Four things that measurement establishes, and one it does not:

* **`swims_started=6`.** Everything from the water crossing through the swim to the
  closest-approach census does its job; nothing downstream of the drop is broken.
* **`t = 9.75 s of 9.75 s run` and `10.45 s of 10.45 s`: the minimum is on the last tick.** Both
  squadrons' rounds were still closing when the mission ended. They are not misses yet, and it
  would be wrong to read 55.4 m and 34.0 m as miss distances.
* **`target_moved=0.0 m`.** The carriers are stationary, as expected while the ship AI's
  path-following state has no body. This is the easiest target the chain will get.
* **The run-in is nearly reciprocal, not a stern chase.** `crossing` is 2.857 rad (164 deg) and
  2.527 rad (145 deg) between the round's track and the target's course, where section 11 measured
  5 to 12 degrees on USN01. A stationary target and an opposed approach are a different geometry
  from the one sections 11 and 14.7 describe, and nothing here extends to those.
* **What it does not establish: whether a round hits.** 3000 frames end 9.75 s after the drop.
  Section 15.6 runs it out.

The distances are centre to centre and horizontal. The Lexington's authored hull is 250 m by 30 m
(`unit hull input unit=Lexington-class01 ... length=250 width=30`), so 55.4 m from the centre is
inside the hull's own footprint along its length, which is another reason not to call it a miss.

### 15.6 Under the probe three of the six DO hit the Lexington - and the hit does nothing

Read from the same probe log, `local/swim_probe_usn04.log`, which 15.5 only read the census of.
The per-round trace carries the rest:

```
torpedo trace 1..3 swim_started life=1.60 pos=(12279.3,0.00,-12645.1)
torpedo trace 4..6 swim_started life=1.60 pos=(-12710.2,0.00,-12717.9)
torpedo trace 4 exit=entity_impact hit=Lexington-class01 at=(-12877.4,0.00,-12905.7) life=9.75
torpedo trace 5 exit=entity_impact hit=Lexington-class01 at=(-12877.4,0.00,-12905.7) life=9.75
torpedo trace 6 exit=entity_impact hit=Lexington-class01 at=(-12877.4,0.00,-12905.7) life=9.75
torpedo trace 1..3 STILL IN FLIGHT at mission end life=10.45 pos=(12459.6,0.00,-12850.5)
                                                  vel=(20.4,0.00,-23.2) swimming=1
```

* **Squadron 2's three rounds hit the Lexington** at 9.75 s of swim. Their census row's
  `min=55.4 m` is not a miss distance at all: it is the centre-to-centre distance at the moment
  the round met the hull, 55.4 m forward or aft of a 250 m ship's centre point.
* **Squadron 4's three were still closing on the Yorktown** when the 3000-frame mission ended,
  34.0 m from its centre, still on the swim at exactly 30.9 m/s (`|(20.4, 0, -23.2)| = 30.9`).
* The swim starts at `life=1.60` against the 1.55 s free fall 15.2 predicts. Before that the round
  spends its first **0.75 s inside a squadron mate's hull box** - it is launched with the
  formation's own velocity, so nothing but gravity separates them - which is the same co-location
  15.1 measures, seen from the other side.

**And the three hits did nothing.** The carrier's census row reads

```
Lexington-class01  side 0  guns 22  ...  shots 72  hits_taken 3  dealt 146  taken 0  health 8000
```

Three hits taken, **zero damage**, full health. The arithmetic is `hull_damage_00470510`:
`(owner_modifier * hull_damage_base - armour) * weapon_scale`, with `008777D0`'s `> 0` test above
it. `hull_damage_base` is a uniform draw between the **bullet class row's** `DamageMin`/`DamageMax`
(`006E7C60` through `00BD2F10`, `src/game_hosts_gunnery.cpp`), and for bullet 69 that draw is about
**44**: in the unprobed run of 15.1 each Kate's `damage_dealt` is exactly `44` where its torpedo
struck an unarmoured squadron mate. A carrier's `Armour` is not below that, so every torpedo hit on
a ship resolves to zero.

**That is a second defect, and it is not this packet's.** It is separable from the swim start, it
is visible only once the rounds survive the drop, and naming it costs nothing to act on later:
either the torpedo's warhead is not the bullet row's `DamageMin`/`DamageMax` at all - the torpedo
class descriptor is `0FCh` bytes against the bullet class's `0D4h`, so it has fields the bullet row
does not - or the ship-hit path owes a torpedo an armour rule of its own.

One pointer for whoever takes it, and it is a pointer rather than an answer. The hit record already
carries `shot_is_torpedo` (`shot->vtable[5Ch](2Bh)`), and `src/ship_hit_record.cpp:230` reads it -
but only to build the roll torque, where `0082712E` takes `host.hull_damage(0.0f)`, **the same
formula with no armour at all**. So the image itself computes an armour-free damage figure for a
torpedo hit in one place on this path, while the hull damage beside it keeps the armour
subtraction. Whether that asymmetry is the image's intent or whether the warhead comes from
somewhere else entirely is unread. **Nothing above is a reading of the image's torpedo warhead; it
is a measurement of what this host does.**

> **Answered, and both guesses above retracted — see docs/TORPEDO_WARHEAD.md.** The warhead is the
> class row's `Blast` sub-table, not its `DamageMin`/`DamageMax`. `0084BC60` step 7
> (`0084BE25`..`0084BEE3`) spawns a radial burst carrying a draw between `BlastDamageMin` and
> `BlastDamageMax` over `BlastRange`, and this host never spawned it at all. For bullet 69 in this
> installation that is **1200 over 50 m**, against a `DamageMin`/`DamageMax` of **50** — and both
> carriers' `Armour` is exactly **50**, so the contact damage is exactly `(50 - 50) = 0`. The
> measured zero is the formula landing on its exact value, not an approximation, and the same
> arithmetic gives the `44` this section saw on a squadron mate (a B5N Kate's `Armour` is 6).
> Retracted: the `0FCh` torpedo descriptor has **no** torpedo-only damage field — a torpedo's
> `vtable[54h]` is `006E2820`, which draws between the same `DamageMin`/`DamageMax` fields every
> other weapon entity uses — and `0082712E`'s armour-free `hull_damage(0.0f)` is only the roll
> torque, never a damage figure.

### 15.7 Run out to 4500 frames: six hits on two carriers, and both carriers end at full health

`local/swim_probe4500_usn04.log`, the same probe binary, `--frames 4700 --mission-frames 4500`.
The longer mission lets all four Kate squadrons reach their release, so the sample doubles:

```
summary mission gunnery torpedo_drop  drops=12 refusals=0 water_entry_breakups=0
summary mission gunnery torpedo_ranges_derived=34 swims_started=12 snaps=0
summary mission gunnery torpedo_closest_approach swims=12

torpedo trace 4,5,6 exit=entity_impact hit=Lexington-class01 at=(-12877.4,0.00,-12905.7) life=9.75
torpedo trace 1,2,3 exit=entity_impact hit=Yorktown-class01  at=( 12464.4,0.00,-12855.9) life=10.70
torpedo trace 7..12 STILL IN FLIGHT at mission end, swimming, 4.15 s and 5.15 s of run
```

**Twelve drops, twelve swims, six hits.** Squadron 2's three take the Lexington at 9.75 s and
squadron 4's three take the Yorktown at 10.70 s - 15.6's three still-closing rounds do arrive, and
the 34.0 m they were short of in the 3000-frame run closes to a hull hit at 26.3 m centre to
centre. Squadrons 6 and 8 release much later, and their six rounds are 197.4 m and 228.5 m out and
still swimming when 4500 frames end; nothing here says whether they would have arrived.

And the damage answer of 15.6 holds at twice the sample:

```
Lexington-class01  ... shots 95   hits_taken 3  dealt 294  taken 0  health 8000
Yorktown-class01   ... shots 216  hits_taken 3  dealt 719  taken 0  health 8000
```

**Six torpedo hits on two carriers, zero damage, both at full health**, for the reason 15.6 reads
off `hull_damage_00470510`. Every accuracy question the torpedo stream has been asking since
section 5 is answered in the affirmative here - against a stationary carrier the run-in, the drop,
the swim and the terminal geometry all work - and the round then does nothing when it arrives.

Three cautions on reading this section, because the probe is doing work in it:

* the probe is **not** the image's rule (15.3) and is not committed; every number above depends on
  the six-to-twelve rounds not being killed on their squadron mates, which is a defect the probe
  hides rather than fixes;
* the targets are stationary (`target_moved=0.0 m`). A moving carrier is a different problem and
  this says nothing about it;
* the run is not deterministic, so these counts reproduce the shape, not the digits.
