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
