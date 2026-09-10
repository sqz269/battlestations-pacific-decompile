# Per-frame update of a unit instance

Addresses: 006fe460, 008255b0, 00904bf0, 004c40a0, 006fe530, 006fe670, 006ff270, 004c0890,
00487270, 00904390, 00cfc3d0 (data).

Packet `unit_instance_update`, worktree `agent/unit-update`. Ghidra was **read-only** for this
packet: no renames, comments, prototypes or saves. Every name below is a hypothesis, not a
recovered symbol; the only recovered class name in this area is `MDestroyer`, read out of the data
after the vehicle-class descriptor vtable at `00D1AD28` (`docs/SCENE_UNIT_CREATORS.md`).

Reconstruction in `include/bsp/unit_instance.hpp` / `src/unit_instance.cpp`, machine-readable facts
in `reports/unit_instance_update.json`.

## The dispatch chain, end to end

`docs/SCENE_UNIT_CREATORS.md` stops at the allocation: `descriptor->vtable[28h](0)` = `006FE590`
allocates 0x1188 bytes, calls the instance constructor `006FE460` and stores the descriptor at
`instance+354h`. This packet picks the object up from there and follows it into the frame.

```
004E4A40  BSP_Game_OnMove, phase 18 (docs/GAME_SIMULATION_GATE.md)
  004C40A0  BSP_Game_UpdateInMissionSubsystems, ECX = game
      00875BB0(scaledDelta)
      004C3CB0()                      ; ECX = game
      world = [game+19CCh]
      world->vtable[0Ch](scaledDelta) ; == 00904BF0
      00447B80(scaledDelta)           ; ECX = [game+30h]
  00904BF0  BSP_World_UpdateEntities, ECX = world, RET 4
      for (e = [world+4]; e; e = [e+38h])
          if (e->byte_5Ch != 0) e->vtable[0DCh](scaledDelta)
      00904600(scaledDelta)           ; ECX = world, the timed-attachment pass
  008255B0  BSP_UnitInstance_Update, ECX = unit, RET 4
```

`004C40A0`'s middle call is the one that matters: the delta is loaded from `game+21F0h` with
`FLD`/`FSTP` around the `PUSH ECX` slot, so the world update receives the **scaled** frame delta of
`docs/GAME_FRAME_CONTROL.md`, not the raw one.

**The per-frame update virtual is vtable slot `0DCh`.** For the `MDestroyer` vtable `00CFC3D0` that
slot holds `008255B0`, which lives in the vehicle base (`0082xxxx`), not in the destroyer's own code
range: the destroyer does **not** override the frame update.

The gate is `entity+5Ch != 0`, a plain byte on the entity, and the chain link is `entity+38h`. The
same chain is walked with the same gate by the world destructor at `00904CD8` (it calls slot `0DCh`
one last time with `FLDZ`, a zero delta) and by `00904CA4` with a second gate at `entity+5Eh`.

### Correction to `docs/GAME_WORLD_ENTITIES.md`

`00481640` (`BSP_EntityManager_Update`) is **not** the path a unit instance takes. It dispatches
`[entityManager+8]->vtable[1]`, whose target is `00487270`: a `std::list` walk over the list at
`+0Ch` (`_Myfirstiter` `+0Ch`, `_Myhead` `+10h`, `_Mysize` `+14h`; node `{_Next, _Prev, value}`)
calling `value->vtable[4]` with the delta. In the `MDestroyer` vtable slot `4` is `0042B970`
(`MOV EAX,ECX; RET`), a this-returning accessor with `RET 0`, so a unit cannot be an element of
that list. That container belongs to the soldier/land-vehicle family of segment 7.

### Correction to `docs/GAME_ON_MOVE_MAP.md`, step 5

Step 5 is described there as a "one-shot device bind" over `DAT_00E188DC`/`DAT_00E188D8`. The block
at `004E4A80..004E4B02` is an inlined copy of `004C0890`, instruction for instruction, and
`004C0890` writes `DAT_00E188D8 = ECX` where ECX is a **unit instance**. The "three vtable+5Ch
capability probes (5, 18h, 0Fh)" are `IsKindOf(classId)` calls (below). So `DAT_00E188D8` is the
**player-controlled unit** and `DAT_00E188DC` is the anchor object it publishes, not a device
handle. The tail is `DAT_00E188DC = resolved->vtable[18h]()` followed by `00B0D7B0(handle)` with
ECX `DAT_00F8D39C`, which stores it at `+1C0h`.

## `006FE460`, the unit instance constructor

`__thiscall(this, int flag)`, `RET 4`, body `006FE460..006FE4C2`, returns `this` in EAX. Called only
from `006FE590` (the descriptor's `vtable[28h]`), after `operator new(1188h)` and a full `memset 0`.

```
006fe460  MOV EAX,[ESP+4]        ; flag
006fe466  MOV ESI,ECX
006fe468  CALL 0081ED40          ; base constructor, __thiscall(this, flag)
006fe46d  [ESI+000h] = 00CFC3D0  ; primary vftable
006fe473  [ESI+010h] = 00CFC3B8
006fe47a  [ESI+024h] = 00CFC3B0
006fe481  [ESI+170h] = 00CFC3AC
006fe48b  [ESI+1E4h] = 00CFC3A4
006fe495  [ESI+310h] = 00CFC38C
006fe49f  [ESI+38Ch] = 00CFC388
006fe4a9  [ESI+72Ch] = 00CFC384
006fe4b3  [ESI+0C4h] = 7
006fe4c0  RET 4
```

Everything else the object needs is set by `0081ED40` and by `006FE590`'s tail. The constructor's
own contribution is exactly eight vptrs and one integer.

`+0C4h = 7` is the **class id**. `006FE530` (slot `5Ch`) compares its argument against it, so 7 is
`MDestroyer`'s own id and the constant is the derived class's identity, not a count or a flag word.

### Base subobjects, from the eight vptrs

MSVC emits a class's secondary vftables immediately **before** its primary one, in descending
subobject order. The seven tables between `00CFC384` and `00CFC3D0` are therefore the secondary
bases of this class, and their sizes follow from the gaps:

| Subobject | vptr at | vftable | Slots | First entry |
| --- | --- | --- | --- | --- |
| primary | `+000h` | `00CFC3D0` | see below | `006FE570` scalar deleting dtor |
| B | `+010h` | `00CFC3B8` | 6 | `006FE520` |
| ref-counted | `+024h` | `00CFC3B0` | 2 | `00BD30E0` `BSP_RefCounted_InvokeDeletingDestructor` |
| D | `+170h` | `00CFC3AC` | 1 | `006FE4D0` |
| E | `+1E4h` | `00CFC3A4` | 2 | `0077B0C0` |
| F | `+310h` | `00CFC38C` | 6 | `006FE510` |
| G | `+38Ch` | `00CFC388` | 1 | `0080DF90` |
| H | `+72Ch` | `00CFC384` | 1 | `006FE500` |

So the unit is one object of 0x1188 bytes with eight polymorphic bases, the last of them starting
at `+72Ch`, and roughly 0xA5C bytes of the layout past that point belong to the most derived class
and to non-polymorphic members.

## The primary vtable `00CFC3D0`

Bytes run without a break from `00CFC3D0` to a null dword at `00CFC694`, but the tail is **not all
one table**. `00CFC3D0+240h`, `+258h`, `+2A8h` and `+2ACh` all hold two-byte `JMP 006FEBE0` thunks,
and `006FEBE0` is the scalar deleting destructor that also opens the *next* class's primary vftable
at `00CFC698`; `00CFC3D0+248h` (`006FEC00`) opens with `LEA EDI,[ESI-3E4h]`, a negative `this`
adjustment. By the same "secondaries first" rule those entries are the secondary vftables of the
class whose primary is `00CFC698`. **Treat the unit's primary vtable as slots `000h..23Ch` only.**
Slot roles above `184h` are not established here.

Established slots:

| Slot | `MDestroyer` target | Role and evidence |
| --- | --- | --- |
| `000h` | `006FE570` | scalar deleting destructor, `RET 4`, calls `0081F3A0` then `00BF6989` |
| `004h` | `0042B970` | `MOV EAX,ECX; RET` — this-returning accessor, `RET 0`. Rules the unit out of the `00487270` list |
| `008h` | `00923030` | `RET 4` — an empty one-argument virtual |
| `05Ch` | `006FE530` | `bool IsKindOf(int classId)`, `RET 4`. Overridden |
| `018h` | `006D1E80` | queried by `004C0890` on the resolved controlled unit; produces `DAT_00E188DC` |
| `098h` | `006DFE40` | `JMP 00928860`, the scene-hierarchy `SetParent(arg, newParent, matrix)` (`docs/SCENE_UNIT_CREATORS.md`) |
| `0DCh` | `008255B0` | **the per-frame update**, `__thiscall(this, float scaledDelta)`, `RET 4` |
| `130h` | `006FE620` | attach to the world node's category lists. Overridden. Read only, owned elsewhere |
| `134h` | `006FE670` | detach from them. Overridden |
| `184h` | `004F17E0` | in the scene-creator segment; role not established |

Destroyer-specific overrides inside `000h..23Ch` are exactly `000h`, `05Ch`, `130h`, `134h` and
`184h`. Every other slot in that span points into the shared base ranges (`0042xxxx` root,
`006Dxxxx`/`006Exxxx` ship base, `0077xxxx`, `0080xxxx..0082xxxx` vehicle base, `0092xxxx`,
`0095xxxx`). There is no separate "movement", "damage" or "destruction" virtual reachable from the
world tick: the world tick calls `0DCh` and nothing else, and movement, damage and effects are
reached from inside it as **direct** calls on `this` and on member subobjects.

### `006FE530`, `IsKindOf`

`__thiscall(this, int classId)` returning AL, `RET 4`, listing only (no Ghidra function),
`006FE530..006FE568` inclusive.

```
return classId == 7 || classId == 6 || classId == 5 || classId == 4
    || classId == 2 || classId == 1 || classId == 0
    || classId == this->classId_0C4h;
```

The last comparison is what a base implementation would do alone; the seven literals are the
ancestor ids the destroyer answers to. Id **3 is deliberately absent**, so it names a sibling
branch. Ids `0Fh`, `18h` and `5` are queried by `004C0890`, and `0Eh` and `8` by `008252C0`, so the
id space is larger than the ancestry chain: it is a trait/class query, not a depth index.

## `008255B0`, the per-frame update, in order

`__thiscall(this, float scaledDelta)`, `RET 4`, body `008255B0..00825DD4`, 521 instructions, 88
basic blocks, 40 calls, **no flow gaps** (`bsp.py ghidra flow`). MSVC SEH frame with handler
`00C91888`. `ESI = this` throughout, and the argument stays at `[ESP+8Ch]` after the prologue.
Everything below is from the listing; the pseudocode drops most of the hidden `this` pointers and
reorders the x87 stack.

Constants used: `00D7A24C` = `1.0f`, `00CE3800` = `0.5f`, `00CFBC84` = `-5.0f`,
`00D7A280` = `0.5` (double), `00CEC9E0` = `-0.5` (double), `00CEDCD0` = `pi/4` (double),
`00D7A250` = `-1.0` (double).

1. **Submerged-effect cull**, `008255CD..0082568F`, over `count = [this+75Ch]` slots.
   `handles = [this+758h]` (pointer array, stride 4) and `offsets = [this+74Ch]` (stride 0Ch,
   the y component at `+4`). For each live handle: refresh the scene node's world matrix if
   `node->flags_5Ch & 2` is clear (`00B6DB70`, `BSP_Transform_RefreshWorldMatrix`), then
   `if (offsets[i].y + node->world[124h] < -0.5) { 00867B10(handle); handle->byte_9 = 1;
   release; slot = 0; }`. `node = [this+4A4h]`; `node+0F0h` is its 4x4 world matrix, so
   `node+124h` is row 3 column 1, the **world Y** of the unit.
2. **Underwater bubble timer**, `0082568F..00825824`, only when `this->byte_5Dh != 0` and
   `[game+19FCh] != 0` (the ocean object). Refresh that object's matrix the same way, and proceed
   only when `0.0 > ocean->world[124h]`. Then `this->f_BC8h -= delta`; when the result is `<= 0`,
   reload it from `00BD2F10(00424C40()->f_650h, 00424C40()->f_64Ch, ECX=1)` (a bounded random draw
   between two settings floats) and emit through `[[this+538h]+0F0h]`: build a stack identity 4x4,
   add-ref the template, `008687C0(&stackFrame, node)`, and set `result->byte_9 = 1`.
3. **Latch rotate**, `00825824..0082583E`. `this->byte_1011h = this->byte_1010h;
   this->byte_1010h = 0`. A one-frame edge latch: producers set `+1010h`, consumers read
   `+1011h` on the following frame.
4. **Gated sub-update**, `0082583E..0082586B`.
   `x = (0092D730([this+1018h]) >= 0.0f) ? 1.0f : 0.0f; 00815AA0(this, x)`. Note the argument is a
   0/1 scalar, **not** the delta.
5. **Bow and stern water anchors**, `00825870..008259CE`, only when `[this+9F0h]`, `[this+9F4h]`
   and `[this+4A4h]` are all non-null.
   `a = row3(descriptor->m_5B0h * node->world)`, `b = row3(descriptor->m_5F0h * node->world)`
   through `00413920` (`BSP_Matrix_Multiply4x4`), refreshing the node's matrix before each.
   `if (a.y > -5.0f) { a.y = 0078CF20(ocean, a.x, a.z) + 0.5; b.y = 0078CF20(ocean, b.x, b.z) +
   0.5; }` — the ocean surface sampler with a half-metre lift. Then `004842C0([this+9F0h], &a)` and
   `004842C0([this+9F4h], &b)`.
6. **Two direct sub-updates**, `008259CE..008259E6`: `0092BE80([this+1018h], delta)` then
   `0081C050(this)`.
7. **Intensity scalar**, `008259EB..00825A4A`.
   `playerFactor = (this == DAT_00E188D8) ? 0.5f : 1.0f;`
   `local = (DAT_00F87152 != 0 || this->byte_2F0h != 0) ? 1.0f : this->f_2F4h;`
   `this->f_10A4h = local * playerFactor;` then `008227E0(&this->f_10A0h, delta)`.
   This is the only place the update reads the controlled-unit pointer.
8. **Wake/bow-spray spawn**, `00825A59..00825BD4`, only when `this->byte_5Dh != 0`.
   `s = descriptor->f_A0h * 0.5;` and, guarded four times by `this->byte_C8h` (when clear, call
   `00414DB0(this)` first, a lazy refresh of the pose block at `+0CCh`),
   `left = this->f_100h + this->f_F0h * s;` `right = this->f_100h - this->f_F0h * s;` (`+0F0h` is
   reached as `[this+0CCh]+24h` in the listing)
   When `00424C40()->byte_680h` is set, `[this+1018h]->byte_88h` is clear and both `left` and
   `right` are below `-5.0f`, set `[this+1018h]->byte_88h = 1` and run the physics query
   `00C31F90 / 00C32000 / 00C33650 / 00C31FC0` on `[[this+1018h]+2Ch]`, feeding the difference of
   the two returned points into `00935540` with ECX `[this+0C4h]`.
9. **Rotor/prop wash**, `00825BD4..00825C11`. When `[this+1170h]` is set and
   `00CE69D0 > this->f_100h`, call `00B6DA70([this+1170h], 0.0f, 0)`.
10. **Five attachment anchors**, `00825C11..00825D4D`, only when `[[this+538h]+6A8h]` is set.
    Five iterations over pointer slots at `this+B54h` (stride 4) with local points at `this+B68h`
    (stride 0Ch): transform each with `004142E0` (`BSP_Vector3f_TransformAffinePoint`) against the
    refreshed node matrix, compare with `00CE7D7C`, and add or release an attachment through
    `008689C0` and the ref-count pair `00CE221C`/`00CE2220`.
11. **Three timed sub-updates**, `00825D4D..00825D83`, each `__thiscall(this, delta)`:
    `008252C0`, `00956600`, `00834E90`.
12. **Per-part tick**, `00825D83..00825DC0`. Vector at `this+A14h` (begin) with `this+A18h`
    elements, stride 4. For each non-null part: `f = 006FF270(this); 00815370(part, f)`.
13. `RET 4`.

Order matters and is fixed: nothing in the body branches on frame parity or on a random draw except
step 2's reload.

### `00956600`, the scalar timers

`__thiscall(this, float delta)`, body `00956600..00956B3E`. The head is the only part read here and
it is the clearest statement of how the delta is consumed:

```
this->f_524h += delta;                               ; unbounded accumulator
if (this->f_728h > 0.0f)
    this->f_728h = max(this->f_728h - delta, 0.0f);   ; clamped countdown
this->f_6D8h -= delta;                                ; firing countdown, not clamped
if (this->f_6D8h <= 0.0f && [this+354h] && [[this+354h]+0Ch]) { ... }
```

`this+354h` is the descriptor back-pointer `006FE590` wrote, confirming
`docs/SCENE_UNIT_CREATORS.md` from the consumer side.

### `006FF270`, the per-part scalar

`float __thiscall(this)`, `RET 0`, body `006FF270..006FF2A6`, returned on the x87 stack:

```
return (DAT_00F87152 != 0 || this->byte_2F0h != 0) ? 1.0f : this->f_2F4h;
```

The identical expression appears inline in step 7, so `+2F0h`/`+2F4h` are one enable byte and one
scalar, overridden globally by `DAT_00F87152`. What the scalar means is **not** established.

## `00904BF0`, the world entity update

`__thiscall(this, float scaledDelta)`, `RET 4`, body `00904BF0..00904C34`. `this` is
`[game+19CCh]`, the world object of `docs/GAME_WORLD_CONSTRUCT.md` (vtable `00CE7784`, gate byte
`+4ACh`, the 0x61-element list array at `+18h`).

```
for (e = [[this+4]]; e; e = [e+38h])
    if (e->byte_5Ch != 0) { FLD delta; e->vtable[0DCh](delta); }
00904600(this, delta);
```

World vtable `00CE7784` has five slots: `0` `004CB0B0` (destructor), `4` `00904390`
(`MOV ECX,[ECX+4]; JMP 009041A0`, the deferred-destroy drain), `8` `009035D0`, `0Ch` `00904BF0`
(this routine), `10h` `004CB370`.

## `004C0890`, the controlled-unit setter

`__thiscall(unit)`, `RET 0`, body `004C0890..004C0924`, no stack arguments.

```
DAT_00E188D8 = unit;
if (!unit) { 00B0D7B0(0) with ECX DAT_00F8D39C; return; }
target = unit->IsKindOf(5) ? unit
       : unit->IsKindOf(18h) ? [unit+3D0h]
       : null;
if (!target) { ...zero path... }
if (target->IsKindOf(0Fh) || target->IsKindOf(18h)) {
    DAT_00E188DC = target->vtable[18h]();
    00B0D7B0(DAT_00E188DC) with ECX DAT_00F8D39C;   ; stores it at +1C0h
}
```

Three callers: `00645600`, `007F3A60`, `00959450`. `004E4A80` inlines the whole body behind
`DAT_00E188DC == 0 && DAT_00E188D8 != 0`, which is the republish-if-lost path once per frame.

`unit+3D0h` is the pointer that turns a *carrier* or *turreted* unit into the object actually being
driven. `008255B0` only ever compares `this` against `DAT_00E188D8` (step 7); the player's steering
inputs do not enter the base update at all. `docs/GAME_INPUT_TICK.md`'s action records are read by
`00A92C40` earlier in the frame, and reach the unit through the controller subobject at
`this+1018h`, which steps 4, 6 and 8 consult but never write from input here.

## Layout of the unit instance (0x1188 bytes)

Only fields with listing evidence in this packet are listed. Offsets are from the object start.

| Offset | Type | Role | Evidence |
| --- | --- | --- | --- |
| `+000h` | vptr | primary vftable `00CFC3D0` | 006FE46D |
| `+010h`,`+024h`,`+170h`,`+1E4h`,`+310h`,`+38Ch`,`+72Ch` | vptr | secondary bases | 006FE473..006FE4A9 |
| `+030h` | ptr | parent scene node = `[game+19CCh]` | 006FE628, 006FE678 |
| `+034h`,`+038h` | ptr | previous/next sibling in the parent's child chain | 00904BF0, 009041A0 |
| `+05Ch` | byte | in-world/active gate for the frame update | 00904C00 |
| `+05Dh` | byte | second gate: steps 2 and 8 | 0082568F, 00825A59 |
| `+05Eh` | byte | dead flag, tested by the world destructor | 00904CA4 |
| `+061h` | byte | selects `+FDCh` over `+984h` in `00834E90` | 00834EBE |
| `+0C4h` | int | class id, `7` for `MDestroyer` | 006FE4B3, 006FE556 |
| `+0C8h` | byte | pose block `+0CCh` valid; when clear, `00414DB0(this)` refreshes it | 00825A64 |
| `+0CCh` | float[] | pose block; `+24h` within it is a lateral half-extent | 00825A99 |
| `+0F0h`,`+100h` | float | two pose scalars combined in step 8 | 00825AB4 |
| `+154h`,`+158h` | native string | entity name length and buffer | `docs/SCENE_UNIT_CREATORS.md` |
| `+2F0h`,`+2F4h` | byte,float | intensity override enable and value | 006FF27A, 00825A14 |
| `+354h` | ptr | vehicle-class descriptor, non-owning | 006FE590, 00956696 |
| `+3D0h` | ptr | driven sub-unit for the controlled-unit resolve | 004C08C8 |
| `+4A4h` | ptr | scene/transform node; world 4x4 at `+0F0h`, world Y at `+124h`, dirty bit `1` of `+5Ch` | 008255FA, 0082560D |
| `+524h` | float | age accumulator, `+= delta` | 00956626 |
| `+538h` | ptr | second class pointer; `+0A0h` scalar, `+0F0h` effect template, `+5B0h`/`+5F0h` anchor matrices, `+6A8h` and `+0F0h` presence flags | 00825728, 008258C9, 00825C11 |
| `+728h` | float | clamped countdown | 0095662C |
| `+6D8h` | float | firing countdown | 0095666F |
| `+74Ch` | ptr | array of 0x0C-byte local offsets parallel to `+758h` | 00825613 |
| `+758h`,`+75Ch` | ptr,uint | ref-counted effect handle array and its count | 008255CD |
| `+9F0h`,`+9F4h` | ptr | bow and stern water anchor receivers | 00825870 |
| `+980h`,`+984h` | float | scalars read by `008252C0` / `00834E90` | 00825331, 00834ECF |
| `+9C4h` | byte | gate at the head of `008252C0` | 008252CA |
| `+A14h`,`+A18h` | ptr,uint | part vector: begin pointer and element count, stride 4 | 00825D83 |
| `+B54h` | ptr[5] | five attachment slots | 00825C2D |
| `+B68h` | float3[5] | five local anchor points, stride 0x0C | 00825C33 |
| `+BC4h` | float | smoothed value driven by `0042AC60` in `008252C0` | 00825319 |
| `+BC8h` | float | underwater bubble timer | 008256D3 |
| `+FDCh` | float | alternate of `+984h` under `+61h` | 00834EC5 |
| `+1010h`,`+1011h` | byte | one-frame edge latch, current and previous | 00825824 |
| `+1018h` | ptr | controller/AI subobject; `+2Ch` its physics body, `+88h` a one-shot | 00825832 |
| `+107Ch`,`+1088h` | ptr,float[] | used by `00834E90` | 00834F22 |
| `+10A0h`,`+10A4h` | float | smoothing state and the intensity product | 00825A3E |
| `+1170h` | ptr | prop-wash emitter | 00825BD4 |

Globals: `DAT_00E188D8` controlled unit, `DAT_00E188DC` its published anchor, `DAT_00F8D39C` the
object that receives it at `+1C0h`, `DAT_00F87152` the global intensity override,
`DAT_00F876A4` the mission clock (`00904600`, `006FEC00`).

## World registration, for completeness

`006FE620` (slot `130h`, read only, owned elsewhere) calls `00928560` — which pushes onto the
parent's list at `+24h` — and then pushes onto `+30h`, `+48h`, `+54h`, `+60h` and `+6Ch` through
`00484540`. `00484540(list, value)` allocates a 0x0C-byte node `{prev, next, value}` and appends to
a list `{count, head, tail}`.

Against `docs/GAME_WORLD_CONSTRUCT.md`'s world layout (a `61h`-element array of `0Ch`-byte lists at
`+18h`), those six offsets are elements **1, 2, 4, 5, 6 and 7**. Element 1 is the base-class
category; the five above it are the unit's.

`006FE670` (slot `134h`), `__thiscall(this)`, `RET 0`, body `006FE670..006FE69A`, calls `006DFFC0`
and then unlinks from element 7 (`+6Ch`) only. `006DFFC0` calls `006D3620` and unlinks `+54h` and
`+60h`, so the detach chain mirrors the class chain while the attach was flattened into one
function by the compiler.

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 006FE460 | BSP_UnitInstance_Construct | exported, analyzed, reconstructed, compiled |
| 008255B0 | BSP_UnitInstance_Update | exported, analyzed, reconstructed, compiled |
| 00904BF0 | BSP_World_UpdateEntities | exported, analyzed, reconstructed, compiled |
| 006FE530 | BSP_UnitInstance_IsKindOf | analyzed, reconstructed, compiled (listing only) |
| 006FE670 | BSP_UnitInstance_DetachFromWorldLists | exported, analyzed, reconstructed, compiled |
| 006FF270 | BSP_UnitInstance_GetIntensityScale | analyzed, reconstructed, compiled |
| 004C0890 | BSP_Game_SetControlledUnit | analyzed, reconstructed, compiled |
| 00487270 | BSP_EntityList_UpdateAll | analyzed only (listing only) |
| 00904600 | — | read, not analyzed; described as an external contract |
| 004C40A0 | already named `BSP_Game_UpdateInMissionSubsystems` | read only |

`src/unit_instance.cpp` compiles clean under Win32 MSVC with warnings as errors
(`build/win32/bsp_core.dir/Release/unit_instance.obj`). The `bsp_core` link and the test run are
blocked by a pre-existing failure in `src/render_tail.cpp` (`'owned_records': is not a member of
'bsp::ParticleClock'`, four errors), which belongs to another packet's in-flight header change and
names none of this packet's files. "compiled" above therefore means translation-unit clean, not
link- or test-verified.

## Routines without a Ghidra function

The orchestrator must define these before a name can be applied. Ends are inclusive, at the address
of the last instruction.

| Address | End | Role |
| --- | --- | --- |
| `006FE530` | `006FE568` | `IsKindOf`, `RET 4`, followed by `INT3` padding |
| `00487270` | `004872C8` | entity `std::list` update walk, `RET 4`, followed by `INT3` padding |
| `00904390` | `00904393` | `MOV ECX,[ECX+4]; JMP 009041A0`, world vtable slot 1 |

`006FF270` and `004C0890` do have Ghidra functions.

## Uncertainties

- The primary vtable's exact length. `000h..184h` is proven in use; `240h` and beyond is proven to
  belong to another class. `188h..23Ch` is unresolved and no slot in it is claimed.
- The meaning of `+2F4h` / `006FF270`. The expression is exact; the semantics are not. A 0.5 factor
  for the locally controlled unit fits either an audio attenuation or a camera-shake amount.
- `+538h` versus `+354h`. Both are class-level pointers and both are read for per-class constants.
  `+354h` is proven to be the vehicle-class descriptor; `+538h` is only proven to be a second,
  distinct object with its own constant block. `docs/SCENE_UNIT_CREATORS.md` lists them together.
- `00904600`'s per-attachment work was not read past its head. It walks a `std::list` at `+4B0h`
  with `DAT_00F876A4` deadlines, matching the world construction doc's `+4B0h..+4B8h` list.
- The eight base subobjects are counted from vftable spacing, not from constructor bodies. Their
  sizes and their individual identities are not established.
- No damage entry point was found. Damage arrives through a non-virtual path or through a slot in
  the unresolved `188h..23Ch` span; step 3's `+1010h`/`+1011h` latch is the most likely consumer.

## Follow-up packets proposed

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `unit_pose_refresh` | 00414DB0, 00825A64 site, +0CCh block | docs/UNIT_POSE_REFRESH.md | The lazy pose block at `+0CCh` behind the `+0C8h` byte: what `00414DB0` computes and which fields of `+0CCh..+100h` are position, heading and speed |
| `unit_timers` | 00956600, 008252C0, 00834E90 | docs/UNIT_TIMERS.md | The three timed sub-updates of `008255B0` in full, including the descriptor fields each reads |
| `unit_controller` | 0092D730, 0092BE80, 00815AA0, +1018h ctor | docs/UNIT_CONTROLLER.md | The controller subobject at `+1018h`: its layout, how the input records of `docs/GAME_INPUT_TICK.md` reach it, and its physics body at `+2Ch` |
| `unit_base_vtable_tail` | 00CFC3D0+188h..23Ch | docs/UNIT_BASE_VTABLE.md | Resolve the unresolved slot span and locate the damage entry point |
| `world_timed_attachments` | 00904600, 004CB030 `+4B0h` list | docs/WORLD_TIMED_ATTACHMENTS.md | The world's deadline list walked after every entity update |

## Correction from docs/UNIT_TIMED_SUBUPDATES.md

The position integration is not in step 11: none of the three timed sub-updates writes a position, heading or speed, and none samples the ocean or terrain. `008252c0` is the engine-audio parameter update, `00956600` advances the age and two countdowns (the +6D8h countdown is a 0.2 s damage-threshold scan over the descriptor's 16-byte records, not a reload) and runs the animation chain and the +2F4h fade that becomes the scene node's visibility factor, and `00834e90` drives three steering nodes and four propeller nodes. The transform is owned by the controller at unit+1018h and its physics body (`0092d730` dots the body's linear velocity with a row of the block `00c32000` returns), so the integration lives behind step 6 (`0092be80`) or inside the physics library.
