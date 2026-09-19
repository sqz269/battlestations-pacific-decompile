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
