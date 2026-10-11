# Particle geometry pass and quad emitters (packets cc13_water2 and cc13_particles1, segment 82)

Code is in `src/particle_emitters_cc13.cpp` and `include/bsp/particle_emitters_cc13.hpp`
(namespace `bsp::particle_emitters`). It builds on the existing particle work (NATIVE_PARTICLE_* and
CC12_PARTICLE_* docs). Names are hypotheses.

- **Status.** Every routine below is reconstructed and build-tested. None is fixture-tested,
  game-validated or wired into the host.
- **Method.** Each block was traced against the listing with the cc13-water2 symbolic executor
  (`local/wa2_sym.py`, now `tools/x87_sym.py`), which decodes the x87 register forms from the opcode bytes. Ghidra's
  one-operand `FMUL ST1`/`FDIVRP` mnemonics are ambiguous: `DC C9` means ST1 *= ST0. Constants were
  read from the PE.
- **Precision.** Values round at every native float store and use double between stores, so they are
  close to native but not bit-exact.

## 1. Where the emitters sit

The particle type vtables share one layout:
- +18h InitializeState
- +1Ch ReleaseState
- +20h no-op
- +24h unnamed
- **+28h emit quads**

| Type | Table | +24h | +28h |
| --- | --- | --- | --- |
| Axial | D5DCC0 | **00B05DD0 BSP_ParticleAxial_ComputeBoundingSphere** | **00B06B10 BSP_ParticleAxial_EmitQuad** |
| Floating | D5DCEC | **00B07BF0 BSP_ParticleFloating_ComputeBoundingSphere** | **00B08210 BSP_ParticleFloating_EmitQuad** |
| Sprite | D5DD18 | **00B08950 BSP_ParticleSprite_ComputeBoundingSphere** | **00B09510 BSP_ParticleSprite_EmitQuad** |

The only +28h dispatch in 00AE0000..00BC0000 is 00B056FB/00B056FE inside **00B05550
BSP_ParticleContainer_BuildGeometry**. It passes (state, life percent, 0, matrix, float) and sums the
returned quad counts into container+28h. +1Ch (ReleaseState) is 00B00750 (Axial), 00B007A0 (Floating)
and the existing 00B007F0 (Sprite). cc13_particles1 defined 00B00750, 00B007A0, the three +24h bodies,
00B09510 and 00B094E0 as Ghidra functions (`reports/cc13_particles1_defined_functions.json`).

## 2. RNG draws (RNG RULE)

| Site | Routine | Call | Stream |
| --- | --- | --- | --- |
| 00B057C8 | 00B057C0 BSP_Particle_RandomIntInclusive | 00BD2FC0, ECX = 0 | RandomStream::primary |
| 00B0C9AA | 00B0C8B0 BSP_ParticleState_AdvanceTextureFrame | 00BD2FC0, ECX = 0 | RandomStream::primary |

- 00BD2FC0 looks up the calling thread's RandomThreads state (00BD2ED0, with fallback 00E14748) and
  draws stream 0. On the main thread this is the same Mersenne state the gameplay draws use (00BD2F10
  forwards to the same lookup).
- So random texture frames (def+61h) consume the gameplay stream natively.
- The port never draws itself; the caller supplies `ParticleRandom`. Before this is wired, give it a
  separate presentation stream or keep it behind an OFF switch.
- 00B057C0 has no rel32 caller in the image. Its Ghidra body (0x120 bytes) runs past its RET at
  00B057DC.
- 00B06B10, 00B08210 and 00B09510 draw only through 00B0C8B0 (00B0834C, 00B0987F), and only when def+4Ch
  and def+61h are set. No other routine in this doc draws.

## 3. Routines

**00B06A90 BSP_ParticleType_ResolveFrameIndex** — `int __thiscall(def, frame)`, RET 4.
- If def+62h is set (wrap): lo + (frame - lo) % (hi - lo + 1), with IDIV (signed). It returns lo
  when the span is <= 0.
- Otherwise it clamps the frame to [def+50h, def+54h].

**00B0C8B0 BSP_ParticleState_AdvanceTextureFrame** — `float __fastcall(state)`.
- state is the 6Ch state and state+64h is the definition. If def+4Ch is clear it returns with
  nothing pushed, leaving ST0 untouched.
- percent = min(100, age+40h * 100 / life+44h), with double 100 [00D7A220] and clamp 100.0f
  [00CE3D08].
- frame = float(v * life/100 * state+3Ch + state+34h), where v is the integral of def+5Ch over the
  percent (kind 0 inline; 00AFFD20 or 00AFFCB0 otherwise).
- When def+61h is set:
  - It returns delta = frame - old frame.
  - When fmod(frame,1) + delta > 1 (00BF857A), it sets state+30h = state+38h.
  - It then sets state+38h = fmod(old next,1) + def+50h + draw % span.
- When def+61h is clear: state+38h = frame + 1 and it returns frame.
- All callers discard the result: 00B06B10, 00B08210, and 00B0987F inside the not-yet-function
  00B09510.

**00B05420 BSP_ParticleRows_SortDescending** — `__fastcall(lo ECX, hi EDX, rows)`, RET 4.
- Hoare quicksort of 8-byte rows {u16 state index, float key at +4}, into descending key order.
- mid = (lo+hi) >> 1 (SAR). The pivot is re-read from rows[mid] at every partition step.
- The left part is recursed into; the right part is handled by the loop.

**00B05550 BSP_ParticleContainer_BuildGeometry** — `__thiscall(container, view[16], float)`, RET 8.
- It sets container+28h = 0 first. It does nothing more when container+18h is null or
  [[container+8]+0Ch]+2Ch is clear.
- If owner+24h is set, each row key = ((y*v[6] + x*v[2]) + z*v[10]) + v[14] of the state position.
  It then sorts with 00B05420(0, count-1), but only when container+10h > 1 and count >= 2.
- Per row:
  - matrix = 00B6DB60(model) * model+298h, through 00413920. The model is container+4. The matrix is
    identity when model+1B0h or +1B1h is set.
  - percent is computed as above.
  - It calls [[state+64h]]+28h.
- Finally container+28h = the quad total.

**00B06B10 BSP_ParticleAxial_EmitQuad** — `int __thiscall(def, state, t, 0, matrix, float)`,
RET 14h. The last argument is unused.
- **Fade gate.** It returns 0 when 0.004 [00CEC178] > emitter+24h (emitter = state+68h).
- **Position.** position = state+0Ch origin plus three terms, each times s = life+44h / 100:
  - k(speed def+2Ch)·s·dir+18h
  - g(gravity def+30h)·matrix row 1·s·state+48h
  - w(wind def+34h)·s·state+4Ch·[00E13028..30]

  Here k, g and w are parameter integrals over t (kind 0 inline as c·t). g is spilled to a float
  before the matrix product. The position is stored back to state+0..8.
- It then calls 00B0C8B0 when def+4Ch is set.
- **Alpha gate.** alpha = state+5Ch · curve(def+38h) · emitter alpha. It returns 0 under 0.004.
- **Colour.** Colour is premultiplied: R/G/B = curve(def+3Ch/+40h/+44h) · alpha·255 ([00CE4B48]).
  Each channel is clamped to [0, 255] (NaN passes through) and truncated with CVTTSS2SI. The packing
  is B | G<<8 | R<<16 | A<<24.
- **Additive.** If def+7Ch == 1 (blend mode 1, "additive"), the alpha byte is 1 and vertex
  half +36h = 1.0h; otherwise half +36h = 0.0h.
- **Axis.** The axis is def+94h..9Ch. If FollowDirection (def+80h) is set, it is instead the
  normalised (00419510) instantaneous velocity from the same three terms, using curve values rather
  than integrals.
- **Size.** length = state+58h · curve(def+8Ch) and width = state+58h · curve(def+90h).
- **Alignment (def+A0h).** The corners are (x0, width) (x0, y1) (x1, width) (x1, y1):

  | Value | Name | x0 | x1 | y1 |
  | --- | --- | --- | --- | --- |
  | 0 | Center | L | -L | -W |
  | 1 | Bottom | 2L | 0 | -W |
  | 2 | Top | 0 | -2L | -W |
  | 3, 4 | Left, Right | L | 0 | 0 |

  **Left and Right are not handled.**
- **Frames.** A = 00B06A90(ftol(floor(state+30h))) and B = the same for state+38h. floor is
  00BF85B0: its x87 path sets control word [00E16460] = 173Fh (round down) and calls _frnd. ftol is
  00BF7420. blend = fmod(state+30h, 1).
- **Vertices.** It writes four 38h-byte vertices:
  - xyz, 0.0
  - ARGB
  - axis xyz
  - blend
  - frame A (u,v) and frame B (u,v), as u16 halves from the 1Ch-byte frame rows at def+68h:
    u = +10h/+14h, v = +12h/+16h
  - corner x, y
  - halves 3C00h and the additive flag
- **Buffer.** The target is [emitter+8]. It writes at cursor +1C0h, or +1D0h when def+65h is set.
  The cursor advances by E0h and the room count (+1B8h or +1CCh) drops by 1. There is no bounds check
  here. It returns 1.

**00B08210 BSP_ParticleFloating_EmitQuad** — `int __thiscall(def, state, t, 0, matrix, float)`,
RET 14h. The matrix and the last argument are unused.
- **Fade gate** as in 00B06B10. The cursor is [emitter+8]+1C4h, read before anything else. There is no
  def+65h second buffer.
- **Angle.** angle = state+50h + state+54h · (∫RotationSpeed(def+84h) · pi/180 [00D5DAF8]) · s, with
  s = life/100. It is spilled to float (B082C1).
- **Position.** position = ∫speed(def+2Ch) · s · dir+18h + origin+0Ch. There is no gravity or wind term.
  Then 00B0C8B0 when def+4Ch is set.
- **Alpha gate and colour** as in 00B06B10, with no flag half. After the colour, **state+4 (y) is set to
  0.0** (B084DA), so the particle sits on y = 0.
- **Size.** size = state+58h · curve(def+88h). Frames A/B and blend come from floor/00B06A90/fmod, as in
  00B06B10.
- **Vertices.** It writes four 24h-byte vertices: xyz, corner angle, ARGB, A.u, A.v, B.u, B.v (u16),
  blend, size.

  | Vertex | Corner angle | u | v |
  | --- | --- | --- | --- |
  | 0 | angle + pi [00CE3D28] | +14h | +16h |
  | 1 | angle − pi/2 [00CE3830] | +10h | +16h |
  | 2 | angle + pi/2 | +14h | +12h |
  | 3 | angle | +10h | +12h |

- **Buffer.** It skips the writes when the cursor is null, but the room count +1BCh drops by 1 either way.
  The cursor advances by 90h. It returns 1.

**00B09510 BSP_ParticleSprite_EmitQuad** — `int __thiscall(def, state, t, 0, matrix, float
view_size_scale)`, RET 14h. It is the only emitter that reads the last argument.
- **Body.** cc13_particles1 defined it as a Ghidra function, 00B09510..00B09FF7, with INT3 padding on both
  sides. It also defined the unreferenced 00B094E0 just before it.
- **Gates and angle.** Fade gate, buffer selection (def+65h) and angle as in 00B08210/00B06B10.
- **Position.** Speed, gravity (matrix row 1, state+48h) and wind (state+4Ch) as in 00B06B10, plus an
  optional term: when def+48h is non-null, s · ∫def+48h along the state vector +24h..2Ch. Every product is
  spilled; the sum is stored to state+0..8. Then 00B0C8B0 (00B0987F).
- **Alpha and size.** alpha = state+5Ch · curve(def+38h) · emitter alpha, **with no 0.004 gate**.
  size = state+58h · curve(def+88h).
- **Near-camera fade.**
  - view is 00B6FCB0 on [[00F8C274]+1Ch], copied with REP MOVSD.
  - z is the view-space depth of the position, from 004142E0.
  - r = float(double(size · view_size_scale) / z), clamped to [0, 1] (NaN passes through).
  - fade = (1 − r)^2 by MSVC's inline pow (FYL2X, FRNDINT, F2XM1, FSCALE; exponent 2.0f [00CE3958]; a
    negative base is negated as −0.0f − f). fade = 0 when 1 − r == 0.
  - **The fade only gates.** Unless def+65h is set, it returns 0 when 0.007 [00D5E028] > fade · alpha. The
    colour does not use it.
- **Dynamic light (state+60h, when non-null).**
  - The node is [emitter+8]. The light position (+1ECh..1F4h) is the particle position. When node+1B0h is
    set, it is the particle position plus node+120h..128h; 00B6DB70(node) runs first when node+5Ch bit 1 is
    clear.
  - 004B62E0 BSP_Light_SetBaseDiffuse gets {raw R, G, B curve values, 1.0}.
  - light+1F8h = max(0.01 [00D7A238], size).
- **Colour.** R/G/B = curve · alpha·255, clamped and packed as in 00B06B10. Additive mode sets alpha byte 1
  and flag half 3C00h (NEG/SBB/AND C400h/ADD 3C00h).
- **Vertices.** It writes four 38h-byte vertices: xyz, corner angle, ARGB, 1.0 ×3, blend, A.u, A.v, B.u,
  B.v, size, angle, u16 0, u16 flag.

  | Vertex | Corner angle (+ pi/4 [00CEDCD0]) | u | v |
  | --- | --- | --- | --- |
  | 0 | angle − pi | +10h | +16h |
  | 1 | angle − pi/2 | +14h | +16h |
  | 2 | angle + pi/2 | +10h | +12h |
  | 3 | angle | +14h | +12h |

- **Buffer.** Cursor, room count and return as in 00B06B10.

**Slot +24h: bounding spheres** — `float* __thiscall(def, float sphere[4], state, t)`, RET 0Ch, returning
the sphere. These are provisional names: the dispatching consumer is not found yet. There is no
CALL [reg+24h] in 00AE0000..00BC0000; the MOV reg,[reg+24h] / CALL reg form was not scanned.
- **00B05DD0 Axial.**
  - L = state+58h · curve(+8Ch) and W = state+58h · curve(+90h).
  - The axis is def+94h..9Ch. With FollowDirection, it is the 00419510-normalised curve velocity. Its
    gravity term is (0, curve(+30h) · s · state+48h, 0): **y only, with no matrix**, unlike 00B06B10.
  - centre = position + L·axis (Alignment 1), position − L·axis (2), otherwise the position.
  - radius = sqrt(L·L + W·W) through 00BF7030.
- **00B07BF0 Floating and 00B08950 Sprite.** centre = position and radius = state+58h · curve(+88h) · 0.5
  [00D7A280]. The two bodies are identical except for the rel32 call displacements.

**Slot +1Ch: ReleaseState** — RET 8; both stack arguments are unread.
- **00B00750 Axial:** [00F8D380] −= 1. A byte scan finds no other literal reference to 00F8D380, and the
  Axial initializer 00B059C0 does not increment it. Natively this counter only goes down, unless a writer
  without a literal address exists.
- **00B007A0 Floating:** [00F8D384] −= 1. 00B077D0 increments it at 00B07BBA and 00B07BD6.
- Ghidra lists the 00B07BBA access as a reference to 00F8D380. That is an overlapping-global artefact;
  the instruction encodes 00F8D384.

**00B094E0 BSP_Light_SetPosition** (provisional, name only) — `void __thiscall(light, const float
xyz[3])`, RET 4.
- It sets light+1ECh..1F4h = xyz.
- There is no rel32 or absolute reference to it. 00B09510 inlines the same three stores at
  00B09B76..00B09B83.

## 4. Handoff

- Leases are released. The work is committed on `agent/cc13-water2`.
- Next targets:
  - 00B08210, the Floating emitter: same caller and similar shape, 0x5A0 bytes.
  - 00B09510, the Sprite emitter: it needs a Ghidra function first.
  - The +24h slots 00B05DD0, 00B07BF0 and 00B08950 (undefined).
  - The +1Ch slots 00B00750 and 00B007A0 (undefined).
- Other unnamed functions in the segment, by size:
  - 00AF63A0 (foliage visibility caller), 00AF85B0/00AF8500, 00AFDCF0/00AFD820, 00B05550's callers
    00AFF7F0/00AF5E90.
  - The 00AFEB60..00AFF290 string/pool group.
  - The render-side half of the segment (00B0D020..00B1BD20: settings, material records, the scene
    render 00B156C0).
- Identified CRT routines, not renamed (outside this lease):
  - 00BF85B0 = floor
  - 00BFA910 = _CIlog
  - 00BFEEEC = _CIexp
