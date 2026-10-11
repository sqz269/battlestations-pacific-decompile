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

### The Object and Tracer types

The particle types have six vtables in all. Each has ClampRecordRange 00B00920 at +0Ch, except Tracer:

| Type | Table | +24h bounds | +28h per-frame |
| --- | --- | --- | --- |
| Object | D5DB00 | **00AF82B0 BSP_ParticleObject_ComputeBoundingSphere** | **00AF85B0 BSP_ParticleObject_UpdateModel** |
| Tracer | D5E048 | **00B0A3F0 BSP_ParticleTracer_ComputeBoundingSphere** | 00B0A110 BSP_NativeParticleTracer_Update (existing) |
| base | D5DDC0, D5DF30, D5DFB0, D5DFF4 | pure / per type | pure |

**00AF85B0 BSP_ParticleObject_UpdateModel** — `int __thiscall(def, state, t, 0, matrix, float)`, RET 14h,
returning 0.
- **No quads.** It does nothing when state+34h (the model instance) is null.
- **Angle** as in 00B08210, but RotationSpeed is def+80h.
- **Position** as in 00B09510: the shared `full_position` helper, same order and spills, stored to
  state+0..8.
- **Node.** node = [state+34h]+0Ch. It calls node vtable +38h (00B6DB10 SetLocalMatrix in the model table
  D5DA50) with 00AF8500([state+30h], angle), then +2Ch (00B6DAB0 SetLocalPosition) with the state pointer.
- There is no fade gate, colour, texture frame or RNG draw.

**00AF8500 BSP_ParticleObject_ComposeRotation** — `__thiscall(matrix, destination, angle)`, RET 8.
- It transposes the matrix **in place**, then computes destination = (M' · Rz(angle)) · M'. Rz is 00B64780;
  the products are 00413920.
- Its only caller passes the state's own 40h matrix (state+30h, built by 00AF8440
  ParticleObject_ConstructRandomMatrix, docs/NATIVE_PARTICLE_OBJECT_STATE.md). So the stored matrix
  flips between M and Mᵀ on every frame. That is what the bytes do; whether it is intended is open.

**Bounds.**
- Object: radius = state+58h · curve(def+84h) · 0.5.
- Tracer: radius = 100.0f [00CE3D08], centred on the state position.

**Tracer resources.** These are name-only:
- 00B0ACA0 BSP_NativeParticleTracerResources_Destruct releases the refcounted +8 and +4 and clears the
  singleton 00F8D38C.
- 00B0A420 ..._DestructSingletonBase is the same tail, reached only from the unwind funclets.

### The particle model's render hook

**00AF73A0 BSP_NativeParticleModel_SubmitRender** — `__thiscall(model, render context, 3 unread ints)`,
RET 10h. It is the particle model table D5DA50, slot +20h.
- **Pass gates.** camera = context+8 and pass = camera+198h.
  - It skips pass 2.
  - It skips pass 1 unless system(+18Ch)+7Ah is set.
  - When system+70h == 1, only pass 3 runs.
- **Skips.** It also returns when:
  - ModelEffectOptions +5 ("hide particles") is set;
  - 00B71530 classifies the vtable +48h world sphere as 0AAAh (outside);
  - +1D8h <= 0.0035 or is NaN.
- **Render entries.** When mesh +1B4h element 0 has +10h set, it calls 00B72F80(mesh, context, model, 1.0,
  1365.0, 0).
- **Debug draw.** With debug "show boundings" (0051F520 +4), [00F8D394] vtable +C8h(sphere, camera,
  classification) draws the sphere.
- **Ghidra's pseudocode.** It gives the second +48h call two arguments. The listing shows that +48h takes
  none; those pushes are the last two arguments of +C8h.

Also named: 00AF6B90 BSP_TypeInfo_InitializeMwParticleSystem (the "MwParticleSystem" string anchor) and
its unreferenced wrapper 00AF7490.

### LOD, visibility and the per-emitter frame chain

This chain explains where emitter+24h (the alpha that every quad emitter gates on at 0.004) comes from.

| Address | Name | What it does |
| --- | --- | --- |
| 00AF4130 | BSP_ParticleSystem_ComputeLodFade | FOV-normalised distance d (below) and the near/far fade from system +68h/+6Ch |
| 00AF6300 | BSP_NativeParticleModel_ComputeLodFade | World refresh, then 00AF4130 on +18Ch with the world position; d goes to model+1E0h |
| 00AF6360 | BSP_NativeParticleModel_UpdateVisibility | model+1D8h = fade · model+1DCh; visible when > 0.0035 (the 00AF73A0 threshold) |
| 00AFAA20 | BSP_ParticleEmitter_DistanceFade | Per-emitter window over d: start +30h, fade-in +34h, hold-end +38h, fade-out +3Ch |
| 00AFF7A0 | BSP_ParticleEmitter_Update | emitter+24h = window fade; above 0.004, container vtable slot 0(context) |
| 00AFF7F0 | BSP_ParticleEmitter_BuildGeometry | emitter+24h = min(model+1D8h, window fade); above 0.004, 00B05550 |
| 00AF5EE0 / 00AF5E90 | BSP_NativeParticleModel_UpdateEmitters / _BuildEmitterGeometry | Loop the +194h/+198h emitters with d = model+1E0h |

- **00AF4130's distance.**
  - d = |centre of 007C1180(sphere, 00B70490 view-projection)| · (1/tan 20°) / (1/tan(fov/2)).
  - The first product is spilled as a double. The tangent is the native 00412E20 FSINCOS helper.
  - The result is 0 at or beyond far, 1 at or inside near, and linear in between.
- **00AFAA20's NaN paths** are not checked against its FCOMI/JAE/JB chain.
- **Callers.** 00AF6360 and the large 00AF63A0 (which calls 00AF5E90) are called from 00AF0C50
  BSP_FoliageGroups_BuildVisibleSet (segment 81, not leased here). That name looks wrong for a routine
  that drives particle models; it is open.

### The model's frame geometry driver

**00AF63A0 BSP_NativeParticleModel_BuildFrameGeometry** — `__thiscall(model, camera)`, RET 4, with an
SEH frame. It owns the buffers that the quad emitters write into; emitter+8 is this model.
- **Visibility.** +1D8h = 00AF6300 · +1DCh. The cursors +1C0h and +1D0h are cleared.
- **Lock.** Under the device lock (EnterCriticalSection([[00F8C274]+30h]), depth word +18h):
  - mesh A (+1B4h) is locked for +1F4h·4 vertices into +1C0h;
  - mesh B (+1C8h) is locked for +1F0h·4 vertices into +1D0h, then 00B0D170([00F8D39C], model, B) runs.
  The lock is 00B73260(mesh, 0) → stream vtable +10h(count, 0, 0).
- **Floating buffer and rooms.** The floating cursor +1C4h is the shared scratch [00F8C274]+28h. The
  rooms are +1B8h/+1CCh/+1BCh = the capacities.
- **Build.** view = 00B6FCB0(camera), or world (+F0h) · view when +1B0h is set. Then 00AF5E90(view,
  float(1 / camera+1C4h)). **That second argument is the Sprite emitter's view_size_scale**, so the
  near-camera fade ratio is size / (fov · z). Then +1D4h = 1.
- **Unlock and counts.**
  - Both streams are unlocked under the lock (vtable +14h).
  - +1F8h is overwritten with the floating count used, flushed through 00AF0540(used·4).
  - Element 0 of each mesh gets vertices = used·4 (+10h) and primitives = used·2 (+18h).

### Emitter object and remaining particle-side helpers

The emitter object has the vtable D5DCB4. Its fields are +08h model, +0Ch definition (refcounted),
+10h container and +24h alpha.

| Address | Name | What it does |
| --- | --- | --- |
| 00AFF700 | BSP_ParticleEmitter_Spawn | Creates the 30h container on first use (00B053D0), then 00B04C80 Spawn |
| 00AFF8C0 | BSP_ParticleEmitter_Destruct | Name only. Deletes the container, releases the definition, then the base destructor |
| 00AF6790 | BSP_ParticleModel_SetEmitterValue | model+2D8h = value; record+B4h = value for each 108h record matched by emitter name |
| 00AF3F80 | BSP_ParticleSystem_IsSphereVisible | The 00AF4130 sphere, classified by 00B71530 |

- **00AF6790's matching.** Names compare with _stricmp. A null name matches every record; an empty name
  matches records that have no name.
- **00AF6790's caller.** It is reached from the RegisteredModelEffect vtable entry 00871FF0.
- **Left unnamed:**
  - generic owned-pointer and refcount helpers 00AFF5A0, 00B0A070, 00AFF890, 00AFFA20/30/50, which are
    probably COMDAT copies;
  - the dword-array assign 00B0A710;
  - the unreferenced name→pointer multimap group 00AFEB60..00AFF290.

## 4. Handoff

- **State.** Leases are released and the work is committed on `agent/cc13-particles1`. Everything in
  section 3 is reconstructed and build-tested; none of it is wired into the host, fixture-tested or
  game-validated. All draws stay inside 00B0C8B0, on the caller's `ParticleRandom`.
- **The render half 00B0CF80..00B1BD20 is not particle code.** It is the scene-renderer object: setters on
  this+21Ah..268h are called from HUD screens, Settings_ApplyAll and OptionsScreen; 00B0D190 is called by
  BSP_Game_Render and DestroyWorld; 00B156C0 is the scene render. It has about 50 unnamed FUN_. Its
  ownership was asked of the lead (cc13-render1/render2?) and not answered before this handoff.
- **Next particle targets.**
  - 00AF0C50 BSP_FoliageGroups_BuildVisibleSet (segment 81) is already reconstructed in
    `src/render_tail.cpp`, with this = [00F8C274]. It calls 00AF6360 and 00AF63A0, and [00F8C274] is the
    object whose +28h scratch and +30h lock 00AF63A0 uses, so it is the particle manager's visible-set
    pass and the "Foliage" name looks wrong. Its host calls for 00AF6360/00AF63A0 can now use the ports
    here.
  - 00B0D170, the renderer hook that 00AF63A0 hands mesh B to.
  - 00AF0540, the scratch flush.
  - The consumer of slot +24h (bounding spheres), which a MOV/CALL-reg scan of segment 82 did not find.
  - 00AF6B90's caller chain (type registration).
- **Unreferenced bodies with no rel32 or absolute reference.** Possibly dead code: 00AF3E60, 00AF5B70,
  00AF5D80, 00AF61E0, 00AF6820, 00AF6B30, 00AF81A0, 00AF81C0, 00AF8400, 00AF88F0, 00AFB790, 00AFB7C0,
  00AFB7F0, 00AFBA40, 00AFBE90, 00AFD820 (local p1_rel32/p1_vtref scans over the image). 00AF5EE0, the
  emitter Update loop reconstructed above, is also unreferenced, so the live update path for emitters is open.
- **Generic containers.** 00AFEB60..00AFF290 is a string/tree container group, and 00B0A710 is a
  dword-vector assign whose `_free` call site Ghidra marks no-return. Both are low value.
- **Tool.** `tools/x87_sym.py` (single-path, opcode-decoded x87) does not model fmod (00BF857A), FYL2X or
  F2XM1, or partial-register stores. Read those from the listing.
- **Identified CRT routines**, not renamed (outside this lease): 00BF85B0 = floor, 00BFA910 = _CIlog,
  00BFEEEC = _CIexp.
