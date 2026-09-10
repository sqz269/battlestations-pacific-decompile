# The unit motion controller at `unit+1018h`

Addresses: 0092BE80, 0080DEC0, 00939CB0, 00939E50, 00939F20, 00939F40, 009329C0, 00936DC0,
00937440, 00937630, 009377E0, 009373C0, 00852210, 00857C20, 00749790.

## Correction: `0092BE80` is not a forwarder

The packet assumed step 6 of `008255B0` reaches the controller's real update through `0092BE80`.
It does not. The whole body is one instruction:

```
0092be80: RET 0x4
```

`void __thiscall(controller, float delta)`, `RET 4`, body `0092BE80..0092BE82`. Ghidra's
`trivial_body` tag is literal: the routine does nothing. The call site is

```
008259ce: fld dword ptr [esp + 0x8c]      ; delta
008259d5: push ecx
008259d6: mov ecx, dword ptr [esi + 0x1018]
008259dc: fstp dword ptr [esp]
008259df: call 0x92be80
```

a direct (non-virtual) call, so no override can intercept it. The second call site is `00826B6A`
inside `00825F20`, also direct. **The unit's per-frame call into the controller is a no-op**, which
closes the question `docs/UNIT_TIMED_SUBUPDATES.md` left open: position and heading are not
integrated anywhere on the unit-update path. They are integrated by the physics library, and the
controller contributes to that only through a force callback the library invokes.

## What the object at `unit+1018h` is

It is the unit's **hydrodynamic force model**: the object that turns the ocean surface, the hull's
authored buoyancy points and the unit's accumulated external forces into a force and a torque on
one rigid body, once per physics step. It owns no integrator of its own.

### Creation

`unit+1018h` is filled by a virtual on the unit's own vtable. Four overrides exist, one per force
model; each allocates `0x390` bytes and runs a constructor:

| creator | ctor | vptr installed | force callback | notes |
|---|---|---|---|---|
| `0080DEC0` | `00939CB0` | `00D19630` | `009329C0` | base model; the override in six unit vtables (`00CF92E0`, `00CFA9A8`, `00CFB968`, `00CFC600`, `00D01860`, `00D098A8`) |
| `00852210` | `00939E50` | `00D196EC` | `00936DC0` | |
| `00857C20` | `00939F20` | `00D196F0` | `00937440` | calls `0080FC30` (reference speed) |
| `00749790` | `00939F40` | `00D196F4` | `00937630` | creator is in segment 41 (`landvehicle`, `landingship`, `landfort` keywords) |

`0080DEC0`, `void __thiscall(unit)`, `RET 0`, body `0080DEC0..0080DF17`, MSVC SEH frame
`00C90A4B`:

```
0080ded7: push 0x390
0080dede: call operator new
0080dee6: mov [esp+4], eax
0080deec: mov [esp+0x10], 0        ; __$EHRec$ state 0
0080def4: jz  0080df00             ; allocation failed
0080def6: push esi                 ; the unit
0080def7: mov ecx, eax
0080def9: call 0x939cb0
0080df00: xor eax, eax
0080df06: mov [esi + 0x1018], eax
```

An allocation failure stores a null controller; nothing on the unit-update path null-checks
`+1018h` before dereferencing it (`0092D730` at step 4 does not), so a failed allocation is a
crash, not a degraded mode.

The three derived creators are the same shape with the derived constructor address substituted;
each derived constructor calls `00939CB0` first and then overwrites the vptr at `+0h`.

### Constructor `00939CB0`

`controller* __thiscall(this, unit)`, `RET 4`, body `00939CB0..00939E43`, SEH frame `00CA7834`.

```
this->vptr        = 00D19630          ; -> 009329C0
this->+08h,+0Ch,+10h = 0
this->+14h        = 0   (byte)
this->+18h        = 0
this->+1Ch        = unit
this->+20h        = 00D1961C          ; -> 009377E0, the second interface's vptr
this->+24h        = 0x7FF9            ; collision mask
this->+28h        = this              ; back pointer
this->+2Ch        = 0                 ; the physics body
this->+30h..+58h  = 0
this->+60h        = 0   (byte)
this->+64h        = 1   (byte)
this->+80h        = 0
eh_vector_constructor_iterator(this+8Ch, 0x20, 0x14, 009318D0, 0092FD50)
this->+310h..+36Ch = 0                ; selected dwords, see the layout table
this->+378h..+38Ch = {0,0,0},{0,0,0}  ; from 00F87574/78/7C, all 0.0f
00937C90(this)                        ; build the hull physics bodies
```

`00937C90` is the routine that emits the Hungarian diagnostics `"fizika_%02d"`, `"ep_fizika"`,
`"roncs_fizika"`, `"wreck_%d"` and `"Hajodarabnak nincs utkozoje:fizika_%d, kb %d. db"` ("the ship
piece has no collider"). It was not analysed in this packet beyond its callee set.

### Layout (0x390 bytes)

Only fields with listing evidence are listed.

| offset | type | meaning | evidence |
|---|---|---|---|
| `+00h` | vptr | force-model vtable, one slot | `00939CCE` |
| `+14h` | byte | disable gate; when set the callback zeroes both velocities and freezes the body | `009329C9` |
| `+1Ch` | ptr | owning unit instance | `00939CE9`, `00932A27` |
| `+20h` | vptr | second interface, one slot -> `009377E0` | `00939CE2` |
| `+24h` | u32 | `0x7FF9`, a collision mask | `00939CD5` |
| `+28h` | ptr | back pointer to the controller | `00939CDB` |
| `+2Ch` | ptr | the physics body | `00939CB0` ctor, `009329D2`, `0092D730` |
| `+60h` | byte | 0 in ctor | ctor |
| `+64h` | byte | 1 in ctor | ctor |
| `+68h..+73h` | float[3] | force staging for the step in progress | `00933A7A..00933AA1` |
| `+74h..+7Fh` | float[3] | torque staging for the step in progress | `00933AA7..00933ACE` |
| `+88h` | byte | one-shot latch set by step 8 of `008255B0`; **not written by the constructor** | `docs/UNIT_INSTANCE_UPDATE.md` |
| `+8Ch..+30Bh` | element[20] | `0x20`-byte records, ctor `009318D0`, dtor `0092FD50` | ctor |
| `+310h..+36Ch` | dwords | zeroed in the ctor (indices `0xC4..0xCA`, `0xCC..0xCE`, `0xD0..0xD2`, `0xD9..0xDB`; `0xCB`, `0xCF`, `0xD3..0xD8` are left alone) | ctor |
| `+378h..+383h` | float[3] | accumulated force, flushed with `00C35360` and then zeroed | `00933ADF`, `00933B48` |
| `+384h..+38Fh` | float[3] | accumulated torque, flushed with `00C35330` and then zeroed | `00933B11`, `00933B78` |

`+84h` and `+88h` are never written by the constructor. Step 8 of the unit update tests
`[unit+1018h]->byte_88h` before setting it, so the object is read there before any code in this
packet writes it.

## The physics library boundary

Every physics entry point reached from the controller is a leaf: no callees, no strings, no source
paths, no RTTI. There is no `Havok`, `PhysX`/`NovodeX`, `ODE`, `Bullet`, `Meqon`, `Tokamak` or
`Newton` string anywhere in the referenced-string index. The surrounding diagnostics are Hungarian
("fizika" = physics, "utkozo" = collider, "roncs" = wreck), matching Digital Reality's own
codebase. **Treat it as an in-house statically linked rigid-body library**; the middleware
question is answered negatively but the library is not positively named. Nothing here is ported.

The accessors decode two objects. The body `B`, and the motion state `M = B->+4h`:

| body | field |
|---|---|
| `B+04h` | ptr to `M` |
| `B+08h..+37h` | 3x4 affine transform: rows at `+08h`, `+14h`, `+20h`, translation at `+2Ch` |
| `B+38h..+43h` | vec3, one corner of the world bounds |
| `B+44h..+4Fh` | vec3, the other corner |
| `B+50h` | u32 flags: bit0 static, bit1 tested by the contact callback, bit2 set to freeze, bits `0x12` cleared by every mutator (wake) |
| `B+6Ch` | owner pointer |
| `B+70h` | first collision shape; shapes link through `shape+208h`, mask at `shape+30h`, group at `shape+2Ch` |

| motion state | field |
|---|---|
| `M+00h` | vec3 linear velocity |
| `M+0Ch` | vec3 angular velocity |
| `M+38h` | vec3 accumulated force |
| `M+44h` | vec3 accumulated torque |
| `M+50h` | float mass |

The API subset the controller uses, all `__thiscall` with the body in ECX unless noted:

| address | signature | behaviour |
|---|---|---|
| `00C31F40` | `vec3* (body, vec3* out)` `RET 4` | copy `M+00h`, the linear velocity |
| `00C31F20` | `vec3* (body, vec3* out)` `RET 4` | copy `M+0Ch`, the angular velocity |
| `00C37E50` | `void (body, const vec3*)` `RET 4` | write `M+00h`; `B+50h &= ~0x12` |
| `00C37E20` | `void (body, const vec3*)` `RET 4` | write `M+0Ch`; `B+50h &= ~0x12` |
| `00C35360` | `void (body, const vec3*)` `RET 4` | `M+38h += v`; `B+50h &= ~0x12` |
| `00C35330` | `void (body, const vec3*)` `RET 4` | `M+44h += v`; `B+50h &= ~0x12` |
| `00C32000` | `float* (body)` `RET 0` | `&B+08h`, the 3x4 transform |
| `00C35300` | `vec3* (body)` `RET 0` | `&B+2Ch`, the world position |
| `00C33650` | `void (const float* m34, float* out4x4)` `RET 4` | expand 3x4 to 4x4; the four `w` lanes become 0,0,0 and `00D7A24C` = 1.0f |
| `00C336C0` | `void (float* m34, const float* m4x4)` `RET 4` | the inverse compaction |
| `00C31F90` | `void (body, vec3* a, vec3* b)` `RET 8` | copy `B+38h` and `B+44h` |
| `00C31FC0` | `float (body)` `RET 0` | inverse mass: `0.0f` when `B+50h & 1`, else `1.0f / M+50h` |
| `00C31EA0` | `void* (body)` `RET 0` | `B+6Ch`, the owner |
| `00C31DC0` | `void* (body)` `RET 0` | `B+70h`, the first shape |
| `00C48020` / `00C48050` | `void (shape, u32)` `RET 4` | set `shape+30h` / `shape+2Ch`, then notify through `[00CE2218]` with `0109ECD8` |
| `00C47F60` | `void (shape, u32)` `RET 4` | `shape+30h &= ~bits`, same notify |
| `00C32450` / `00C32490` | `int (x)` `RET 0` | `x+2Ch` / `x+04h` on the contact record |

`0092D730` (`BSP_UnitController_GetBodyAxisSpeed`, already reconstructed in
`include/bsp/unit_motion.hpp`) dots `00C31F40`'s velocity with row 2 of `00C32000`'s transform,
i.e. `B+20h..+28h`. Row 2 is therefore the hull's forward axis.

## `009329C0`, the force callback

`void __thiscall(controller, float dt)`, `RET 4`, body `009329C0..00933BA9` (0x11EA bytes). It is
slot 0 of the controller's vtable and is called by the three derived callbacks
(`00936DF8`, `00937622`, `0093764B`) after their own work. No game-side caller exists: the library
invokes it.

### Disabled path

```
009329c9: cmp byte ptr [esi + 0x14], 0
009329cd: jz  0x00932a1f
          00C37E50(body, {0,0,0})       ; linear velocity := 0
          00C37E20(body, {0,0,0})       ; angular velocity := 0
00932a16: or dword ptr [eax + 0x50], 0x4
00932a1a: jmp 0x00933b40                ; straight to the accumulator reset
```

so a disabled controller still clears `+378h..+38Fh` on the way out.

### Model selection and effective mass

```
00932a2c: movss xmm0, [ecx + 0x10fc]    ; unit->f_10FCh, kept for the whole call
00932a37: push 8
00932a42: call [unit->vtbl + 0x5c]      ; IsKindOf(8)
          variant = 2                                     when IsKindOf(8)
          variant = (descriptor->f_B0h < 100.0) ? 1 : 0    otherwise
00932a82: EDI = variant * 0x38
```

`descriptor = unit->+538h`, `descriptor->f_B0h` is the hull mass, `100.0` is the double at
`00D7A220`. The settings singleton `00424C40()` is re-fetched before every field read; the row
base is `+4E0h + variant*38h` and the fields used are `+4E0h`, `+4E4h`, `+4E8h`, `+4ECh`, `+4F0h`,
`+4F4h`, `+4F8h` and `+50Ch`. When `variant == 1` the `+50Ch` value is replaced by the literal
`2.0f` (`00CE3958`).

```
effectiveMass = descriptor->f_B0h + unit->f_10FCh;
if (effectiveMass < 1.0f) effectiveMass = 1.0f;   ; 00D7A24C
```

`unit+10FCh` is an additive mass the unit carries (flooding is the obvious candidate; not proven
here). It is used a second time at the flush, see below.

### Per-element loop

The hull's buoyancy points live in a `std::vector`-shaped pair on the descriptor: begin at
`descriptor+52Ch`, end at `descriptor+530h`, element stride `0x24` (the `0x38E38E39` /
`SAR 3` idiom at `00932C4B`). The count is recomputed from the pointers on every iteration. An
empty vector calls `00BF6713` (a CRT trap) before the loop.

`XOR EBP,EBP` at `00932C1F` starts the index at zero, so every `i < 0` branch in the body is
**dead code**: the `3.0f` default span (`00CE3854`), the zero default for the second span, and the
whole-hull drag fallback `min(settings->f_5B8h * descriptor->f_B0h, settings->f_5B0h)` never run.
They are reported here because they document the intended per-element defaults.

Element record (offsets within the `0x24` bytes, from the listing):

| offset | use |
|---|---|
| `+00h` | float, the element's force scale |
| `+04h` | float, upper height |
| `+08h` | float, lower height |
| `+0Ch` | float, reference height |
| `+18h..+20h` | float[3], the attachment point in hull space |

Per element:

```
world      = TransformAffinePoint(e.point,                 bodyMatrix4x4)    ; 004142E0
worldFlat  = TransformAffinePoint({e.point.x, 0, e.point.z}, bodyMatrix4x4)  ; 004142E0
r          = worldFlat - bodyCentre
above      = world.y - 0078CF20(ocean, world.x, world.z)   ; ocean = (*00E188A8)->+19F0h
span       = e[+08h] - e[+0Ch]
if (span < above) above = span
if (above < 0)    above = 0
submerged  = span - above                                  ; in [0, span]
range      = e[+04h] - e[+0Ch]
vPoint     = linearVelocity + cross(...)                   ; 004F9B30
```

The operand order handed to `004F9B30` at `00932D8B` was not resolved from the listing: the two
`LEA`s straddle a `PUSH` and a `SUB ESP,8`, so `r` and the angular velocity could be either way
round and the sign of the correction with them. `009377E0` writes the same quantity out longhand
and settles it there.

`004F9B30` is the same cross product `include/bsp/world_ocean.hpp` already reconstructs, and
`0078CF20` the same ocean sampler step 5 of the unit update uses.

The submersion ratio and the element force scale:

```
ratio = submerged / span
if (curve != 1.0f) {
    if (curve == 2.0f)      ratio = ratio * ratio
    else if (curve == 0.5f) ratio = sqrtf(...)      ; through 00BF7030
    else                    ratio = <the previous element's ratio>
}
scale = (descriptor->f_B0h / (float)count) * ratio * 10.0    ; 10.0 is the double at 00CE3DC0
```

`count` is converted to float as **unsigned** (`00CE3978` = 2^32 is the sign fixup), and the
`curve` value is the settings field `+50Ch` (or the forced `2.0f` for variant 1). The stale-ratio
fallback for any other curve constant is what the listing does; it is recorded as observed, not
as intent.

The drag itself projects `vPoint` onto the three rows of the body matrix and, per axis, forms a
linear plus quadratic coefficient from the settings row, choosing between the `+4E0h/+4E4h` and
`+4E8h/+4ECh` pairs by the sign of the second projection; `+4F0h/+4F4h` and `+4F8h` supply the
other two axes. The resulting force is negated, multiplied by `scale` and by `dt / effectiveMass`,
and clamped per axis so that `F_axis / v_axis` never falls below `-1.0f / count`
(`00D7A250` is the double `-1.0`) — a cap that stops one element's drag impulse from reversing the
point velocity. This part is read from the decompiler with heavy register aliasing and **is
provisional**; the constants and the clamp form are from the listing, the exact assignment of each
settings field to each axis is not proven.

### The flush

```
00933a27: fld  [esi + 0x7c]      ; this->f_7Ch
00933a2b: fstp [esi + 0x70]      ; this->f_70h = this->f_7Ch      (unexplained)
00933a31: fld  [esi + 0x6c]
00933a3a: fld  [esp + 0xb4]      ; unit->f_10FCh, saved at entry
00933a41: fmul qword [00ce3dc0]  ; * 10.0
00933a4b: fsub
00933a4f: fstp [esi + 0x6c]      ; this->f_6Ch -= unit->f_10FCh * 10.0
00933a52: call 0x74f2e0          ; ECX = unit + 10D4h, out = &tmp
          this->f_74h += tmp.x
          this->f_78h += tmp.y
          this->f_7Ch += tmp.z
          this->f_378h += this->f_68h ; f_37Ch += f_6Ch ; f_380h += f_70h
          this->f_384h += this->f_74h ; f_388h += f_78h ; f_38Ch += f_7Ch
00933b01: 00C35360(body, this->+378h)   ; AddForce
00933b38: 00C35330(body, this->+384h)   ; AddTorque
00933b40: this->+378h..+38Fh = 0
```

`0074F2E0` lives in segment 41, whose keyword set includes `sumforces` and `sumleaks`; it zeroes
its output, walks a list on `unit+10D4h` whose length is at `+14h`, and transforms the entries with
`00413920`/`004134F0` before summing. **That is the channel through which everything outside the
physics library pushes force onto a unit** — engine thrust, damage, flooding, explosions. This
packet did not enumerate its producers.

Two consequences for the packet's contract:

* the controller has **no throttle field, no rudder field, no target speed and no target heading**.
  Nothing in the constructor or in `009329C0` reads a command. Commanded motion arrives as an
  entry in the `unit+10D4h` force list, and the buoyancy model only adds hydrostatics and drag.
* **AI-controlled and player-controlled units are indistinguishable at this level.** The only
  player test in the whole unit update is step 7's `this == DAT_00E188D8` comparison, which scales
  an intensity, not a command. `00818340`, the ship AI diagnostic dump, names `helmsmanControl`,
  `navigatorParams`, `thrust`, `thrustMod` and `maxSpeed`; that layer sits above `unit+10D4h` and
  is a separate packet.

### The `+70h` copy

`this->f_70h = this->f_7Ch` before the accumulation means the force's z component is taken from
the torque's z component of the previous step. No reading of the listing removed it and no
surrounding code explains it. It is transcribed as-is and flagged.

## `009377E0`, the contact callback

`void __thiscall(listener, contact)`, body `009377E0..00937B6F`. It is slot 0 of the vtable at
`00D1961C`, whose vptr sits at `controller+20h`; the `this` it receives is therefore
`controller+20h`, and the fields it reads at `+8h` and `+1Ch` are `controller+28h` (the back
pointer) and `controller+3Ch`. The decompiler prints them as `param_1+8` / `param_1+1Ch`.

```
kind  = 00C32450(contact)          ; contact->+2Ch
flags = 00C32450(contact)          ; second read of the same field
unit  = (*(controller+8h))->+1Ch   ; through the back pointer
        00C32490(contact) twice    ; contact->+04h, results discarded
other = 00C31EA0(body)             ; the other body's owner
if (other == 0 || !other->IsKindOf(6)) other = 0
if ((flags & 2) && other) 009373C0(...)   ; clears collision bits on the shape chain
if (kind == 8) { unit->byte_1010h = 1; return; }
```

`unit+1010h` is the one-frame edge latch step 3 of `008255B0` rotates into `+1011h`, so **the
"collided this frame" bit is produced here**, by the physics library, not by any game-side test.

The damage gate then runs when either party's descriptor has `+510h > 0` or `+514h > 0`, is
cancelled when `unit->+6B8h >= 0` and `00779AD0()` is below `3.0f` (`00CE3854`), or when
`0092CE70` on the other unit returns non-zero, and finally:

```
p    = 00C35300(body)                       ; body position
r    = contact->+8h..+10h  -  p             ; the contact point, relative
w    = 00C31F20(body)                       ; angular velocity
v    = 00C31F40(body)                       ; linear velocity
vAtContact = v + cross(w, r)                ; written out longhand, both bodies
...
if (gate && speedA <= speedB) 008145B0(&contact->+8h, magnitude, other, 0)
```

The point-velocity expression is spelled out twice, once per body, as
`v.x + (r.z*w.y - r.y*w.z)`, `v.y + (r.x*w.z - w.x*r.z)`, `v.z + (w.x*r.y - r.x*w.y)`. Component by
component that is exactly `v + w x r`, so the contact velocity here is the textbook rigid-body
point velocity and the sign convention for the whole class is fixed by it.

`009373C0`, `00937B70..` and `008145B0` were not analysed.

## Callers and callees

Callers of `0092BE80`: `008255B0` (step 6) and `00825F20`. Callers of `009329C0`: `00936DC0`,
`00937440`, `00937630`. Callers of the four creators: unit vtables only.

Analysed here: `0092BE80`, `0080DEC0`, `00939CB0`, `00939E50`, `00939F20`, `00939F40`, `009329C0`,
`009377E0`, and the four creators of the derived models.

Treated as external contracts: the whole `DYN_physics` block, `0078CF20` (ocean), `004142E0` /
`004F9B30` / `00413920` / `004134F0` (math), `00424C40` (settings singleton), `00937C90` (hull
body construction), `0074F2E0` / `0074F930` (the `unit+10D4h` force list), `008145B0` (collision
damage), `0092CE70`, `00779AD0`, `009318D0` / `0092FD50` (the `+8Ch` array element ctor/dtor).

`bsp.py ghidra flow` was not run; no `_free` fall-through gap was observed in the listings read.

## Uncertainties

1. The per-axis drag coefficient assignment inside the element loop is provisional.
2. The `this->f_70h = this->f_7Ch` copy at `00933A2B` is unexplained.
3. `unit+10FCh` is treated as an additive mass because it is added to `descriptor->f_B0h`; its
   producer was not found.
4. The `0x20`-byte records at `controller+8Ch` (20 of them) were not decoded.
5. The physics library is not positively identified, only shown not to be any of the common
   middleware of the era.
6. The `+50Ch` curve constants other than `1.0f`, `2.0f` and `0.5f` fall through to a stale ratio;
   whether any shipped settings row uses such a value is unknown.
7. Which vehicle families map to the three derived force models was not established.

## What remains

* `00937C90`, the hull physics construction, and the `fizika_%02d` node naming convention.
* `0074F2E0` and the `unit+10D4h` force list: the producers of every commanded force.
* `00936DC0`, `00937440` and `00937630`, the three derived force models.
* The `controller+8Ch` array of 20 records.

## Follow-up packets proposed

| id | addresses | files | contract |
|---|---|---|---|
| `unit_force_list` | `0074f2e0` `0074f930` `unit+10D4h` producers | `docs/UNIT_FORCE_LIST.md`, `reports/unit_force_list.json`, `include/bsp/unit_force_list.hpp`, `src/unit_force_list.cpp` | The `sumforces` list on `unit+10D4h`: its record layout, who appends engine thrust, damage and flooding entries, and how `0074F2E0` transforms and sums them. |
| `unit_hull_physics_build` | `00937c90` `009318d0` `0092fd50` | `docs/UNIT_HULL_PHYSICS_BUILD.md`, `reports/unit_hull_physics_build.json` | How `00937C90` walks the model's `fizika_%02d` nodes into physics bodies and shapes, and what the 20 records at `controller+8Ch` hold. |
| `unit_force_model_variants` | `00936dc0` `00937440` `00937630` | `docs/UNIT_FORCE_MODEL_VARIANTS.md`, `reports/unit_force_model_variants.json` | The three derived force callbacks and which unit families install them. |
| `ship_helmsman_control` | `00818340` `0081f980` `008362a0` | `docs/SHIP_HELMSMAN_CONTROL.md`, `reports/ship_helmsman_control.json` | The AI navigator and helmsman layer above the force list: `helmsmanControl`, `navigatorParams`, `thrustMod`, `steeringJam`. |

## State reached

| routine | state |
|---|---|
| `0092BE80` | reconstructed, build-tested |
| `0080DEC0` | analysed |
| `00939CB0` | analysed |
| `00939E50`, `00939F20`, `00939F40` | analysed |
| `009329C0` | partially reconstructed (frame, selection, submersion kernel, flush), build-tested; the drag core is analysed only |
| `009377E0` | reconstructed (contact-speed kernel and the latch), build-tested |
| `00936DC0` | exported only; **no Ghidra function**, `00936DC0..009373B2`, last instruction `009373B0` |
| `00937440`, `00937630` | exported only |
| `00852210`, `00857C20`, `00749790` | analysed |

Nothing here is ABI-compatible or game-validated.

## Corrections from docs/UNIT_FORCE_COMMANDS.md

The object at unit+10D4h is not an external force list but the leak (flooding) model: `0074f930` accumulates water per leak point and `0074f2e0` turns those weights into a heeling torque; the `_ship` diagnostic dump at `00818340` labels it `leakManager`, and its total water at leak+28h is the `unit+10FCh` term (settling uncertainty 3 above). Commanded motion is mostly kinematic: `AddForce` `00c35360` has exactly one caller (`009329c0`); the ship path computes `target = ((maxSpeed * gameplayScale) * throttle) * engineGate` in `00825f20`'s tail, `0092d300` rewrites the body's linear velocity toward it under an acceleration limit and `0092e8c0` rewrites the angular velocity; the one genuine command force is the rudder torque `00937440` adds. The writers of unit+980h/+984h, +9A0h/+9A4h and +102Ch..+1038h use a shifted base register and were not found by displacement scans; the ship AI's output stage is the proposed follow-up.
