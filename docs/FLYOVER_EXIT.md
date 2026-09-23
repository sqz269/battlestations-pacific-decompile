# Fly-over exit: where the image hands the dive over, and the one term this host lacks on the way

Addresses: 009C62B0, 009C62CF, 009C62E8, 009C62ED, 009C6305, 009C6320, 009C6342, 009C6351, 009C641C, 009C6453, 009C65FD, 009C663E, 009C6674, 009C66E3, 009C67B0, 009C680E, 009C69B1, 009C6A37, 009C6D6F, 009FA2E0, 009C4220, 009C42B8, 009C42D3, 007F0280, 007F05F7, 007F06AF, 007F0936, 007F0A96

Packet `cc9_flyover_exit`, 2026-09-22. It follows `docs/FLYOVER_SPEED.md`. The report is
`reports/flyover_exit.json`. All names are hypotheses, not recovered symbols.

**Result.**
- **The image hands over off the point.** Its exit law leaves the aircraft on the heading it
  already has, anywhere within a dead band of up to 0.52 rad (30°) of the 3-second lead point. It exits
  on range alone, with no alignment test.
- **The host matches that law.** Its bound heading arm, dead band, slew limiter, roll-in and
  can-dive flags agree term by term. The traced 72-270 m side offsets are the residual bearing,
  0.06-0.24 rad, times the range at hand-over, and that is inside the band. So the fly-over does not
  create the offset. As `docs/DIVE_BOMB_FLYOVER_FLAGS.md` section 8 found, the offset is delivered
  to it.
- **The one differing term is 007F0280, the near-field probe.** On this path it is the only source
  of the attackrun's lateral weave and of the fly-over's speed trim. The host leaves it at zero, and
  in USN04's formations the image's probe would NOT return zero.
- **Not bound.** Its per-axis accumulator (007F06AF-007F0A96) is still unread, so nothing is bound
  and no run was made. Both switches are unchanged.

## 1. The heading arm and its lead point

**009FA2E0 returns the target's velocity only.** It is `__thiscall(sub, float* out)` with RET 4,
and I named it `BSP_BotApproachTargetRef_GetTargetVelocity`:
- If sub+18h is null, it returns the zero seed from 00F87574.
- Otherwise it calls vtable+34h on sub+14h (009FA2EE-009FA2FB), or on sub+18h when sub+14h is
  null (009FA303-009FA313).

**The fly-over forms the relative velocity itself:**
- 009C62CF calls vtable+34h on the aircraft to get its own velocity.
- 009C62ED and 009C6305 subtract the target's velocity from it.
- 009C6320 loads 3.0, a double at 00D7A2B0.
- 009C6342 reads the aim point through vtable[0].
- 009C6346 and 009C6351 build `aim - 3 x (v_own - v_tgt)`, less the unit's position.

That gives the lead range R and the lead bearing. The heading error E is
`SubtractWrapped(bearing, heading)` at 009C641C.

**What the heading arm writes:**
- **A wings-level bank target:** cmd+2C4h = 0.0 with cmd+2CCh = 1, at 009C69B1/009C69B9.
- **The heading itself:** `AddWrapped(base, clamp(dead_band(E, T), -L, +L))`, with mode 2. The
  dead band is at 009C6A37 and the slew limiter at 009C6D6F.
- **The dead band T** is at 009C664B-009C6674. It is `interp(0 -> 0.5236 rad, 200 m -> 0, span)`
  (00CEC724 float, 00CE386C float), so it widens to 30° as the span closes to zero. The bank
  arm's T at 009C6A43, printed by the census, reached 0.5708 rad on the last bank tick.

The host binds all of these; see `docs/DIVE_BOMB_FLYOVER_FLAGS.md` and
`include/bsp/dive_bomb_task.hpp`. It computes the same `aim - 3 x (v_own - v_tgt)` lead point from
the fed aim point, since `docs/HULL_AIM_TURNDOWN.md`. **No term differs.**

## 2. The exit, and the image's hand-over geometry

**The flags on the fly-over's last tick:**
- **+18h, can-dive** (009C680E): the height above the aim point is above approach+D4h, 675 m.
- **+19h, roll-in** (009C67B0): either |E| is above 1.6 rad (00CE3D48, double), meaning the target
  is behind the wing line; or the span is not positive. The span is `max(R - (0.7 x max(h,100) +
  200), 0)`, from 009C65FD.

**The edge.** Per the transition rule the host binds, 009C83E0 moves fly-over to turndown when both
flags are set. With roll-in set but can-dive clear, it goes to aimglide instead (009C8557). I read 009C83E0 only; it is cc9_goaway_reattack's and not annotated here. So the
edge tests range and height. It does not test alignment or position over the point.

**The image's hand-over, predicted from the law.**
- **When:** the edge is reached when `R <= 0.7h + 200`. At the traced h of about 1049 m that is
  about 934 m of lead range, or about 1230 m of planar range once the 3-second lead of about 300 m
  is added back.
- **The dead band:** by then T has opened to 0.52-0.57 rad, so the heading arm stops correcting
  any bearing error below that.
- **Side offset:** R x sin|E|, anything from 0 up to about 500 m. It is whatever the fly-over
  received, reduced only where |E| exceeded T earlier.
- **Heading rate:** the slew-limited turn the heading arm was still commanding, or zero inside the
  band.

**Against the traces.** At turndown entry the host's residual bearing is 0.0591-0.0602 rad for the
Lexington flights, 72-74 m at 1220 m, and 0.18-0.24 rad for the Yorktown flights, 230-270 m at
1100-1280 m. All of it is inside the band. The 0.002 rad/tick of carried yaw is small against a
0.52 rad band. **The image's law hands over the same geometry for the same fly-over entry.**

## 3. The probe, 007F0280

**What it does.** Per `docs/BOT_PROBE_007F0280.md` section 0, it is a near-field avoidance box:
80 x 60 x 120 m half-extents at the attackrun's call 009C42B8, centred on the aircraft. It returns
zero unless another unit is inside the box; the zero-output proof is at 007F0936. It is the only
source of:
- the attackrun's lateral offset: `-probe x 30°` (009C42BD-009C42D3), added to the target
  bearing;
- the fly-over's speed trim: the slot the speed arm reads (`docs/FLYOVER_SPEED.md`).

**Would it fire in USN04?** From the control's trace (`local\v0_usn04.log`, `local\box.py`), I
counted the same-flight pairs inside that box in each aircraft's own frame. During the turndown:

| flight | aircraft with a mate in the box | ticks in the box | closest separation |
|---|---|---|---|
| each Lexington flight (movieval, #1.1, #5.1) | leader and one wingman | 49-53 of about 100 | 62-74 m |
| #3.1 | leader and .-3 | 40 | 107 m |
| #7.1 | leader and .-3 | 3 | 130 m |

The separations during the attackrun and fly-over are not printed. But these flights hold
formation until the turndown, so the box is occupied there too. **In the image the probe would
weave the run-in by up to 30° and trim the fly-over speed for these pairs. This host's zero is a
hole on this mission, not a proof.**

**Why it is not bound.** The rejection box, the tie-break (entity+9D0h) and the zero proof are
read. But the per-axis accumulator 007F06AF-007F0A96 is not: about 300 x87 instructions that
enter with the box values live in ST0-ST3 and XMM1/2/4. A faithful binding also needs the
neighbour enumeration: the entity list at unit+C50h filtered by `vtable[5Ch](15)` when the mode
byte is non-zero, otherwise the list at `this+3D0h`. The host has no equivalent of either yet.
Binding it from the gates alone would invent the magnitude, so `kNearFieldProbeBound` is not
introduced.

## 4. Runs

None. No term in the fly-over or its exit differs from the image. The one differing term is the
probe, which is not bounded. A fresh control would only repeat `local\v0_usn04.log` from
`docs/FLYOVER_SPEED.md`.

**Predictions for when the probe is bound.** Only the pairs listed in section 3 change.
- **Run-in weave:** these aircraft weave on the run-in by `-probe x 30°`, so their side offset at
  the fly-over's entry changes, in either direction.
- **Formation break:** the tie-break at 007F0792-007F082F pushes the two aircraft of a pair to
  opposite sides. So a pair's hand-over geometries diverge where they are now nearly equal.
- **Isolated aircraft:** those with no mate in the box, such as #1.1|.-3 and movieval|.-2, are
  unaffected.

## 5. Decision

* `kFlyoverSpeedBound` stays true. `kAimDiveTailBound` and `kHullAimOffsetEnabled` stay false.
* **The fly-over's exit is the image's.** The 72-270 m hand-off is not a fly-over defect.
* **Next read:** 007F06AF-007F0A96 (the accumulator), and a host neighbour list for the probe. That
  is what the attackrun weave, the fly-over speed trim and the goaway obstacle list share.
* **Still unexplained:** the aimdive's ±200 m swing, which the probe does not act on. The aimdive
  has no 007F0280 call. The swing is not yet shown to be the image's own; reading the image's pitch
  gain inputs at approach+64h/+68h against their authored values is the other open item.
