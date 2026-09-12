# The ship AI `follow` and `land` state steps

Addresses: 009E1610, 009E1950, 009DF2D0, 009DA3B0, 009DA610, 0082E850, 00CE3DE0, 006AC5D0, 0070D290, 0070D080, 00811150, 006F2E60, 006F2FB0, 006AC220, 006AC260, 00417B10, 0082ADC0, 00811A30, 00605070, 0070D100

Packet `cc_ai_follow_land`, worker `agent/cc-ai-follow-land`. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; Ghidra was read-only for this
packet. Every descriptive name is a hypothesis, not a recovered symbol. The reconstruction is
`include/bsp/ship_ai_follow_land.hpp` and `src/ship_ai_follow_land.cpp`; the call inventory with
one row per native call site is `reports/ship_ai_follow_land.json`.

Both steps are vtable `+0Ch` of their state object and both take one float. `follow` never reads
it; `land` reads it twice, as a countdown. Neither reads its command object
(`00E08F60` and `00E08FA0`): `follow` takes its station from the unit's formation and `land` takes
its target from a pad entity the unit already holds.

## 009E1610, the `follow` step

`__thiscall(state)(float seconds)`, `RET 4` at `009E18C0`, body `009E1610-009E18C2`, complete.
State object `brain+0B8Ch`, vtable `00D215F8`.

Three gates share one epilogue: the formation at `unit+284h`, its leader at `+14h`, and
`leader->vtable[5Ch](6)` at `009E1642`. A ship whose formation has no leader does nothing at all
this tick, not even a heading hold.

```
making_way = latch on leader speed                       ; 009E164C..009E1684
    0092D730(ECX = [leader+1018h])                       ; the LEADER's controller
    set when speed >= 0, cleared when speed <= -1.0f     ; 00D7A260
009DF2D0(state)                                          ; 009E1689, refills the state
pos = (unit+0FCh, unit+104h)                             ; 009E16A7, 009E16BA
d2  = |pos - station|^2                                  ; 009E16C7..009E16EC
r   = 0082E850(brain+0AACh) * 2.5                        ; 009E16F0, 00CE3DE0, a double
out_of_station = latch: enter d2 > 2r^2, leave d2 < r^2   ; 009E1717, 009E1723
if (out_of_station) {
    dot = (station - pos) . direction                    ; 009E1761..009E1773
    if (dot < 0) {                                       ; 009E1781
        s = min(-dot, 0082E850(brain+0AACh)) * 1.5       ; 009E17B5 00415510, 00CE3D78
        n = (-direction.z, direction.x)                  ; 009E17C4
        station += (n . (station - pos) < 0 ? +1 : -1) * s * n   ; 009E17F2, 009E180C
    }
}
009DE050(blk, &station, keep_mode = 0, final_leg = 1)    ; 009E1837
009DA3B0(blk, &request)                                  ; 009E18B6
```

The answers the packet asked for.

* **The formation point is not in the command.** `009DF2D0` calls `0070D290` with
  `ECX = [unit+284h]` and `(unit, &out[7], 1.0f, 1.0f)` (`009DF307`, `RET 10h`). `0070D290`
  resolves the unit's own member record through `0070D080` (the record array starts at
  `formation+18h`, stride `34h`, count at `formation+4F8h`, keyed by the unit pointer at
  `record+0h`) and reads **two offsets out of that record**, at `record+10h + 4*p` and
  `record+20h + 4*p`, where `p` is the formation pattern index at `formation+500h`. So a slot is
  (along, across) from a per-member table, selected by a formation-wide pattern id, scaled by the
  two 1.0f the caller passes. The leader's own pose reaches it through `00811150`
  (`ECX = [formation+14h]`, `RET 10h`, which forwards to `00810630` with `ECX = leader+0BD0h`);
  `0070D290` then builds `out[0..1] = base + along * (-dir.z, dir.x)` and `out[2..3] = dir`
  (`0070D2F9..0070D356`). When the unit **is** the leader (`0070D362`), the same routine answers
  its own position `unit+0FCh` / `+104h` and `(cos, sin)` of `wrap_2pi(pi/2 - vtable[50h]())`, so
  the leader's heading enters the slot frame through that branch.
* **The slot becomes the station point** in `009DF2D0`: `state+30h = unit+9C8h * 0.5`
  (`009DF33A`, the hull radius from the *follower*), and the station point is the slot point
  offset by `state+30h` along the slot direction, **minus** while making way and **plus** while
  not (`009DF36x`). The point is then pushed out of the class's zone set by `00417B10` with a
  20.0f margin (`00CE3930`, `009DF432`), a second probe point 1.5 turn radii along the direction
  gets the same treatment (`009DF44A`, `009DF4C5`), and when the two land more than 1.0 apart the
  direction is re-derived from them through `00419260` (`009DF533`, `009DF571`, stored at
  `009DF556`). `state+24h` is `wrap_2pi(pi/2 - atan2(dz, dx))` of that direction
  (`009DF5AA`..`009DF5DF`) and `state+28h` the leader's body-axis speed (`009DF359`) scaled by the
  routine's own throttle blend (`009DF65B`). The tail publishes a per-member speed back into the
  formation record's `+30h` through `0070D100` (`009DF6AA`).
* **The seven-field request is nine fields, 1Bh bytes.** `009DA3B0` is
  `__thiscall(blk)(const request*)`, `RET 4`, body `009DA3B0-009DA408`, complete: six floats and
  three bytes copied straight to `blk+38Ch`..`blk+3A6h` with no test
  (`009DA3B6`, `009DA3BF`, `009DA3C8`, `009DA3D1`, `009DA3DA`, `009DA3E3`, `009DA3ED`, `009DA3F7`,
  `009DA400`). Those are exactly the fields the station-keeping arm of `009ED6B0` consumes:
  `docs/SHIP_AI_GOAL_VECTOR.md` records that `009EDA28`..`009EE57B` runs only while `blk+3A5h` is
  set and `blk+3A6h` clear, projects the goal difference onto the frame at `blk+38Ch` / `+390h`
  and takes a heading error against `blk+394h`. **The follow step is that arm's only producer in
  this chain**: in station it publishes the slot frame with `+3A5h = 1` and `+3A6h = 0`, so the
  arm runs; out of station it publishes zeros with `+3A6h = 1`, which shuts the arm off and lets
  the ordinary path follower take the ship back to its station.
* **The goal handed to `009DE050`** is the station point itself, with `keep_mode = 0` and
  `final_leg = 1` (`009E1830`, `009E1831`, `PUSH EBX` with `EBX = 1` from `009E1657`). A follower
  therefore re-plans against its station every tick and always reports the last leg, which is why
  `blk+1E4h` stays set for a ship in formation.
* **The exit** is a single epilogue at `009E18BB`; there is no `finished` posting and no call to
  `0071E430` anywhere in the body. `follow` never completes on its own.

`0082E850 BSP_ShipClass_GetTurnRadius` serves this step twice, both times with
`ECX = brain+0AACh`, the ship class descriptor cached next to the unit on the brain: once for the
station hysteresis (`009E16F0`) and once to cap the back-off (`009E1790`). `00CE3DE0` is the
double `2.5`; it is used nowhere else in either step. The hysteresis is therefore "out of station
beyond 3.54 turn radii, back in station inside 2.5".

## 009E1950, the `land` step

`__thiscall(state)(float seconds)`, `RET 4` at `009E1E54`, `009E1FFB` and `009E2018`, body
`009E1950-009E201A`, complete. State object `brain+0BE0h`, vtable `00D21658`.

**The target is a pad entity, not a position.** The unit carries one at `unit+1200h`, reachable
only behind `unit->vtable[5Ch](0Ch)` (`009E19C2`); on other unit kinds the same offset is a float
pair (`docs/SENSOR_TABLES.md`). The pad has its own pose (`+0C8h` dirty byte, `+0FCh` position,
forward at `+0ECh` / `+0F4h`), an occupant at `+1F8h` (`006AC220`, body `006AC220-006AC226`:
`MOV EAX,[ECX+1F8h]; RET`) and an owning base at `+220h`. The base holds a pad vector at `+794h` /
`+798h` that `006F2E60` searches, and a critical section at `+764h` that `006F2FB0` takes to
assign one. Whether a pad is a dock, a mooring or a beaching point is **not** settled by these
bodies; what is settled is that it is a pose-carrying entity with an occupant slot, owned by a
kind-1Ch base, and that its approach line is clipped against land.

```
goal = (brain+0B2Ch, brain+0B34h)                        ; 009E195B, the fallback
pos  = (unit+0FCh, unit+104h)                            ; 009E198E
pad  = unit->vtable[5Ch](0Ch) ? [unit+1200h] : 0         ; 009E19C2
if (pad && occupant(pad) && occupant(pad) != unit)       ; 009E19E4, 009E19EF
    pad = 0 ; rescan = -1.0f                             ; 009E1A02
if (unit is kind 0Ch && !final) {                        ; 009E1A07
    rescan -= seconds                                    ; 009E1A1B
    if (rescan < 0 || !pad) {                            ; 009E1A30
        rescan = uniform(3.0f, 5.0f)                     ; 009E1A51, 00BD2F10
        base = pad ? [pad+220h] : 0                      ; 009E1A6C
        if (!base && [brain+0B20h]->vtable[5Ch](1Ch))     ; 009E1A79, 009E1A8C
            base = [brain+0B20h]
        p = 006F2E60(base)(unit, 0)                      ; 009E1A97
        if (p && p != pad) { 006F2FB0(base)(unit, p) ; pad = p }   ; 009E1AAE
    }
}
blk+300h = 1.0f                                          ; 009E1AC2
if (pad) goal = 006AC5D0(pad)(&out, &unit+0FCh, [[unit+538h]+570h], 200.0f)  ; 009E1B0A
         hold_heading = 0                                ; 009E1B24
state+18h/+1Ch/+20h = (goal.x, 0, goal.z)                ; 009E1B28
if (!final) {
    if (|goal - pos|^2 < 122500.0) {                     ; 009E1B8C, 350^2
        h = wrap_2pi(pi/2 - atan2(dz, dx))               ; 009E1B9D
        if (|wrap(heading() - h)| < pi/4) final = 1      ; 009E1BFE
    }
    if (!final) goto approach                            ; 009E1C0E
}
goto final
```

`006F2E60` (`__thiscall(base)(unit, char skip_owned)`, body `006F2E60-006F2FA2`, read) walks the
pad vector: a pad with no occupant competes on squared 3D distance from the unit, and a pad whose
occupant is already this unit is returned immediately when `skip_owned` is clear, which is what
the land step passes. `006F2FB0` (body `006F2FB0-006F3009`, read) early-outs when the pad's
occupant is already this unit and otherwise does the assignment under the base's critical section;
its two inner calls `006F2DE0` and `006AC490` are **contract: unread**.

### The landing geometry, 006AC5D0

`__thiscall(pad)(float3* out, const float3* from, int zone_group, float half_width)`, body
`006AC5D0-006ACB37`. The land step passes the unit's world position, the ship class's zone group
id at `[[unit+538h]+570h]`, and `200.0f` (`00CE386C`). The facing comes from `006AC260`: the pad's
own pose row at `+0ECh` / `+0F4h`, normalised in the plane with y forced to zero.

Everything before `006AC927` is a cache keyed on `pad+208h`, the zone group id, and it is what
makes this a *landing* line rather than a bearing:

* the pad's world position goes to `pad+20Ch`..`+214h` with y zeroed, then a ray is cast from it
  along its facing (`100000.0`, `00CF81F0`, or `-100000.0` when the pad is already inside a zone)
  and, on a hit, the origin is pulled back to the hit point minus one direction unit. The origin
  therefore sits at the **water's edge along the pad's facing**, not at the pad's own position;
* a back-cast fills `pad+218h` with how far astern of that origin the water reaches (default
  `1000.0`, `00CE3804`);
* a forward cast from 5.0 units ahead fills `pad+21Ch` with how far ahead it reaches, times `0.9`
  (`00D7A390`; default `800.0`, `00CE3950`).

The per-call arm is a pure rule, projected in `ship_ai_land_pad_approach_point_006ac5d0`. With
`along` the projection of (origin - from) on the facing and `across` its absolute perpendicular:
inside the corridor (`across <= half_width`, `along <= half_width`, `-along <= pad+21Ch`) the
answer is a carrot on the approach axis, `interp(0.5, 1.0, 1.0, 0.0, across/half_width) *
half_width` ahead of the ship's own projection (`006ACA2C`), never further back than `pad+218h`
(`006ACA3C`); outside it the answer is a hold-off point `min(pad+218h, half_width)` **behind** the
origin. A ship off the line is sent to the entry, a ship on the line is drawn up it.

### The approach arm, 009E1E57

```
zones = 0082ADC0([unit+538h])                            ; 009E1E66, EAX is the zone set
goal  = 00417B10(zones)(&out, &goal, 10.0f, 1)           ; 009E1E83, 00CE38B8
blk+3F4h = 1                                             ; 009E1EA2
009DE050(blk, &goal, keep_mode = 0, final_leg = 0)       ; 009E1EB6
blk+3F0h = 3                                             ; 009E1EC2
r = max(2 * 00811A30(unit)(1.0f), 300.0f)                ; 009E1F21, 009E1F3C
if (|pos - goal|^2 < r^2 &&                              ; 009E1F60
    (state->vtable[2Ch](&goal) || 009DA610(blk, &goal) || |pos - goal|^2 < 12000.0f))
    final = 1                                            ; 009E1F9B
if (state+8h <= -1.0f) { brain+0AF0h = 1.0f ; return }    ; 009E1FA4
state+8h -= seconds                                      ; 009E1FBA
brain+0AF0h = interp(-1.0f, 1.0f, 0, 0, state+8h)         ; 009E1FE6
```

`0082ADC0` (`__thiscall(class)()`, body `0082ADC0-0082ADD7`) looks `class+570h` up in the
singleton `004218E0` returns and hands the result back **in EAX**; that return, not the brain, is
the `this` of the `00417B10` call at `009E1E83`, because EAX is dead across `0082ADC0` otherwise.
`00417B10` (`RET 10h`, body `00417B10-00417BF9`) copies the point, then walks the vector at
`set+4h` / `set+8h` and replaces the point through `00416F30` for every zone whose bounds at
`+14h`/`+18h`/`+1Ch`/`+20h` contain it and whose `00416B50` agrees; `00416B50` and `00416F30` are
**contract: unread**. The arrival radius is two turn-circle radii (`00811A30` divides the class
radius by the gameplay modifier at index 5) with a 300.0f floor, and three ways to declare
arrival: the state's own `vtable[2Ch]`, the block's latched-arrival test `009DA610`, or a flat
12000.0f (about 110 units). `state+8h` is a one-second speed ramp: while it is above -1.0f the
brain speed scale climbs from 0 to 1, and once it passes -1.0f the step returns early with the
scale pinned to 1.0f and never decrements it again.

`009DA610` is `__thiscall(blk)(const float* goal2d)`, `RET 4`, body `009DA610-009DA671`, complete:
true only while `blk+2FDh` is latched and the offered goal is within 50 units of `blk+1DCh` /
`+1E0h`, and any further goal clears the latch. It compares against the same `00D20278` float32
that `009DE050` uses to drop the crossing state, so the two agree by construction. Its other two
callers are `009E2020` and `009E23B0`.

### The final arm, 009E1C18

```
rescan = 99.0f ; blk+3F4h = 0 ; blk+3F0h = -1            ; 009E1C23, 009E1C28, 009E1C32
if (!hold_heading) {
    state+14h = wrap_2pi(pi/2 - atan2(dz, dx))           ; 009E1C66..009E1CA3
    hold_heading = (|goal - pos|^2 < 10000.0f)           ; 009E1CBE, 00CE3D64
} else {
    goal = pos + 006BC0C0(state+14h) * 100.0             ; 009E1CE0, 00D7A220
}
err = |00438B10(unit->vtable[50h](), state+14h)|         ; 009E1D49, 009E1D4F
if (blk+1C4h != 1) { blk+368h = 0 ; blk+360h = 0 ; blk+1C4h = 1 }   ; 009E1D72
009DA4E0(blk)                                            ; 009E1D9F
blk+1D8h = state+14h ; 00605070(&blk+1D8h)               ; 009E1DB0, 009E1DB4
t = interp(pi/12, 1.0f, pi/4, 0.5f, err)                 ; 009E1DEF
blk+1C8h = 0 ; blk+1CCh = 0 ; blk+1D0h = clamp(t, -1, 1) ; 009E1E12, 009E1E34, 009E1E3B
brain+0AF0h = t                                          ; 009E1E48
```

The final arm abandons navigation entirely: it forces steering mode 1 (heading), drops the path
plan through `009DA4E0`, and writes the four fields `009DBF90` owns inline rather than calling it.
`00605070` (body `00605070-006050BD`) rewrites `blk+1D8h` in place: `fmod` by `2pi` through
`00BF857A`, then `+2pi` at or below `-pi` and `-2pi` above `+pi`, so the desired heading ends in
`(-pi, pi]`. The throttle is full ahead under `pi/12` of heading error and half at `pi/4` and
beyond; the clamped value reaches `blk+1D0h` and the unclamped one `brain+0AF0h`.

The heading latch reads the opposite way round from what its name suggests: `hold_heading` goes up
**inside** 100 units of the landing point (`009E1CBE` jumps to the clear on `10000 <= d2`). So the
ship re-aims at the pad every tick while it is still far, and once inside 100 units it freezes the
heading and steers a carrot 100 units ahead of itself, which is what drives it straight onto the
pad instead of curving as the bearing swings.

There is no `finished` posting here either: the land step never calls `0071E430` and never clears
its own state. Completion is somebody else's, presumably the arrival predicate the director polls.

### The state objects

Both are produced by the brain constructor `009F39C0`, which is also where their sizes show:

| field | follow, `brain+0B8Ch` | land, `brain+0BE0h` |
| --- | --- | --- |
| `+0h` | vtable `00D215F8`, `009F3A42` | vtable `00D21658`, `009F3A6B` |
| `+4h` | the brain, `009F3A3C` | the brain, `009F3A68` |
| `+8h` | out-of-station latch, **cleared** at `009F3A53` | speed ramp timer, **not written** |
| `+0Ch` | slot point x | rescan timer, `1.0f` at `009F3A72` |
| `+10h` | slot point z | final flag, 0 at `009F3A77` |
| `+11h` | - | hold-heading flag, 0 at `009F3A7B` |
| `+14h` | station point x | held heading |
| `+18h` | station point z | goal x |
| `+1Ch` | direction x | goal y, always stored 0 |
| `+20h` | direction z | goal z |
| `+24h` | slot heading | - |
| `+28h` | leader speed | - |
| `+2Ch` | making-way latch, **set** at `009F3A4C` | - |
| `+30h` | station radius | - |

## Coverage

| Routine | Coverage |
| --- | --- |
| `009E1610` | complete |
| `009E1950` | complete |
| `009DA3B0` | complete |
| `009DA610` | complete |
| `006AC220` | complete |
| `006F2E60` | complete: body read, not projected |
| `00605070` | complete: body read, not projected |
| `0082ADC0` | complete: body read, not projected; `004218E0` and `004120D0` contract: unread |
| `00811A30` | complete: body read, not projected |
| `006AC260` | complete: body read, not projected |
| `009DF2D0` | partial: field mapping and call inventory from the listing at `009DF2D0`..`009DF6AA`, body **not** projected; the speed match `009DF5EA-009DF6B4` read in pseudocode only |
| `006AC5D0` | partial: the per-call arm from `006AC927` projected; the cache refresh `006AC5D0-006AC926` read in pseudocode, not projected |
| `0070D290` | partial: the field mapping proved from the listing; `00811150`'s callee `00810630` not read, so the leader-frame basis is read-through |
| `0070D080` | partial: the record stride and key read from the listing; the tail `0070D0AA-0070D0B5` not transcribed |
| `00417B10` | partial: the loop and the bounds test read; `00416B50` and `00416F30` contract: unread |
| `006F2FB0` | partial: the guard and the lock read; `006F2DE0` and `006AC490` contract: unread |
| `00811150` | partial: the forwarding read (`ECX = leader+0BD0h`, `00810630`); `00810630` not read |
| `0070D100` | partial: the member walk read; its caller relationship to the speed match not projected |

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| `docs/SHIP_AI_STATE_STEPS.md`: the follow step "latches 'making way' in `state+2Ch` against `0092D730` and `00D7A260`" | the speed is the **leader's**, not the follower's: `ECX = [leader+1018h]` with the leader from `[[unit+284h]+14h]` | `009E1650 MOV ECX,[EDI+1018h]` with `EDI` set at `009E162E` |
| `docs/SHIP_AI_STATE_STEPS.md`: "hands a seven-field request to `009DA3B0`" | the request is nine fields in `1Bh` bytes: six floats and three bytes | `009DA3B0-009DA406`, nine stores |
| `docs/SHIP_AI_STATES.md`: "A state object is sixteen bytes: `+0h` the vtable, `+4h` the owner, `+8h` and `+0Ch` two floats seeded from `00CF5BFC` (-99.0f) at `009F3A22`/`009F3A27`" | those two stores are the **cruise** state at `brain+0B70h` only (`EBX = ESI+0B70h`). The follow state is `54h` bytes and is seeded with `+2Ch = 1` and `+8h = 0`; the land state is `24h` bytes and is seeded with `+0Ch = 1.0f`, `+10h = 0`, `+11h = 0` | `009F3A07`, `009F3A4C`, `009F3A53`, `009F3A72`, `009F3A77`, `009F3A7B` |
| the packet brief and `docs/ENTITY_COMMAND_CLASSES.md` (a file this branch does not have): the follow command's "target unit and offset" | the step and its producer never read a command object. The leader is `[[unit+284h]+14h]` and the offsets come from the formation member record at `formation+18h + 34h*i`, columns `+10h` and `+20h`, indexed by the formation pattern at `formation+500h` | `009E161F`, `009E162E`, `0070D080`, `0070D2B7`, `0070D2BD`, `0070D2CC` |
| `docs/SHIP_AI_STATE_STEPS.md`'s land call inventory (`006AC220` twice, `00BD2F10`, `006F2E60`, `006F2FB0`, `006AC5D0`, two atan2, `006BC0C0`, two `00438B10`, `009DA4E0`, `00605070`, `00419010`, `0082ADC0`, `00417B10`, `009DE050`) | incomplete: it omits three `00414DB0` calls, the two indirect `vtable[5Ch]` kind tests, the two indirect `vtable[50h]` heading reads, `00811A30` at `009E1F21`, `009DA610` at `009E1F83`, the state's own `vtable[2Ch]` at `009E1F72` and the second `00419010` at `009E1FE6` | `reports/ship_ai_follow_land.json`, twenty-eight rows |
| the ledger evidence on `009E1610` and `009E1950`: "The body was NOT read" | both bodies are now read in full, from the listing | this document |

## Uncertainties

* The land state's `+8h` speed-ramp timer has no writer in the constructor `009F39C0`, which seeds
  `+0Ch`, `+10h` and `+11h` explicitly. Its initial value is whatever the brain's allocation
  leaves. No search was made for another writer, so "it starts at zero" is **not** established.
* Kind `0Ch` (the landing unit) and kind `1Ch` (the base) are the literals the two
  `vtable[5Ch]` sites push; the callee is indirect and its body was not read, so what those kinds
  name is unknown. The same is true of the leader's kind `6` in the follow step.
* `009E1610`'s out-of-station arm leaves `request+0h` and `request+4h` uninitialised: the zeroing
  at `009E184C`..`009E185E` covers `request+8h`..`+14h` only, and only the in-station arm writes
  the first two. They reach `blk+38Ch` / `+390h`, which `docs/SHIP_AI_GOAL_VECTOR.md` says are read
  only while `blk+3A6h` is clear, which is exactly the case the arm does not take. The
  reconstruction publishes zeros there rather than reproducing the stale read; that the gate holds
  on every path into `009EDAA8` was **not** proved here.
* Whether a pad is a dock, a mooring buoy or a beaching point is not settled. What the bodies
  settle is the shape: a pose-carrying entity with an occupant at `+1F8h` and an owner at `+220h`,
  whose approach line is clipped against the ship class's avoid-zone group.
* No run-time evidence: neither step is on a path `bsp_game.exe` reaches
  (`docs/GAME_EXECUTABLE.md` has no ship AI state tick), so rule 6 of the checklist does not
  apply and none was gathered.

## no_ghidra_function

none. Every address this packet read or cites is the start of a Ghidra function: `009E1610`
(`009E1610-009E18C2`), `009E1950` (`009E1950-009E201A`), `009DF2D0` (`009DF2D0-009DF6B4`),
`009DA3B0` (`009DA3B0-009DA408`), `009DA610` (`009DA610-009DA671`), `006AC5D0`
(`006AC5D0-006ACB37`), `0070D290` (`0070D290-0070D3FE`), `0070D080` (`0070D080-0070D0B5`),
`00811150` (`00811150-00811174`), `006F2E60` (`006F2E60-006F2FA2`), `006F2FB0`
(`006F2FB0-006F3009`), `006AC220` (`006AC220-006AC226`), `00417B10` (`00417B10-00417BF9`),
`0082ADC0` (`0082ADC0-0082ADD7`), `00811A30` (`00811A30-00811AAA`), `00605070`
(`00605070-006050BD`), `0070D100` (`0070D100-0070D132`).

## Follow-up packets

| id | addresses | question |
| --- | --- | --- |
| `ship_ai_follow_formation_producer` | `009DF2D0`, `0070D290`, `0070D080`, `00811150`, `00810630`, `009DACD0`, `0070D100` | project `009DF2D0` whole, including the speed match `009DF5EA-009DF6B4` and what `0070D100` publishes back into the member record's `+30h`. It is the only unprojected half of the follow chain. |
| `ship_ai_formation_records` | `formation+18h..+500h`, `0070D080`, `0070D060` | the member record layout: who writes `record+10h`..`+2Fh`, what the pattern index at `formation+500h` selects, and how many patterns there are. The follow slot is authored data and this is where it enters. |
| `ship_ai_landing_pads` | `006AC5D0` cache arm, `006AC260`, `006F2DE0`, `006AC490`, `pad+1F8h`, `pad+220h`, `base+794h` | the pad entity: who creates pads, what kind `0Ch` and kind `1Ch` are, and whether the avoid-zone clip means a shoreline. Settles what "land" lands on. |
| `ship_ai_zone_pushout` | `00417B10`, `00416B50`, `00416F30`, `0082ADC0`, `004120D0` | the zone set and the push-out. Three separate chains (`follow`, `land`, `006AC5D0`) route their goals through it, so its contract is load-bearing for all of them. |
| `ship_ai_station_keeping_arm` | `009EDA28`, `009EDB6B`, `009EDD9B`, `009EDE87` | unchanged from `docs/SHIP_AI_GOAL_VECTOR.md`, but now with its producer known: the arm's input is this packet's request and its gate is `blk+3A5h`/`+3A6h` as the follow step sets them. |
| `ship_ai_brain_speed_scale_0af0` | `brain+0AF0h` | unchanged from `docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md`; the land step adds two more writers (`009E1E48`, `009E1FEF`, `009E200C`) and no reader. |

## Correction from docs/SHIP_AI_FORMATION.md

Packet `cc_ai_formation` read the member record's three producers (`0070ED30` join, `0070EFD0`
reshape, `0070E620` the scene reader) and corrects two readings above: the slot columns are
(across, along), not (along, across): `record+10h + 4k` is the across-track offset and
`record+20h + 4k` the along-track distance back along the leader's wake (Lua `dist[k].x` and
`dist[k].z`, exactly four columns, `0070E84B CMP EDI,4`); and the station offset is added while
the making-way latch is set, not subtracted. Three 25-entry pattern tables at `00E08FE0`,
`00E090A8` and `00E09170` (line, column, diamond) are scaled by `FormationShipDist`. `009DF2D0` is
now whole, including the speed match that publishes `reference_speed * 1.25 / max(blend, 0.25)`
into `record+30h`; `00810630` never writes its fourth out-parameter on the `along <= 0` arm, so
`0070D290`'s `out[4]` is stale for a station abreast of or ahead of the leader.
