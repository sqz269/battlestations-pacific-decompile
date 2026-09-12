# The bow and stern water anchors, and how a point reaches a point effect

Addresses: 004842C0 00815370 00822C20 00823F09 00825870 0082FE30 00867D00 00B6DAE0 00B6E0D0
004142E0 008687C0 00868420 008680B0 00815AA0 008255B0

`docs/UNIT_CONTROLLER.md` closed with one open object: the thing at `unit+9F0h` and `unit+9F4h`
that takes a **world point** from step 5 of `008255B0` through `004842C0` and a **scalar** from
`00815AA0` through `00815370`. This packet settles what it is, where it comes from, what the two
inputs do to it, and who reads the result.

The answer in one line: each anchor is an ordinary point-effect instance
(`docs/POINT_EFFECT_INSTANCE.md`, `PointEffectInstanceStorage`, `0x114` bytes) built from the ship
class's `BowParticle` template and parented to the unit's scene node; `004842C0` sets its scene
node's world position **and** caches the point in the effect's parent-relative matrix, and
`00815370` broadcasts the scalar to the two component arrays the effect owns.

## `004842C0`, the point publish

`void __thiscall(point effect, const float3* world_point)`, `RET 4` at `0048431D` and `0048433F`,
body `004842C0..00484341`, 42 instructions, no flow gaps. Full listing read.

```
004842c6  node = this->p_110h                       ; the effect's own scene node
004842d7  node->vtable[30h](world_point)            ; ECX = node, the caller's buffer pushed
004842d9  parent = this->p_8Ch
004842ea  if (parent == 0)
004842f1      this->f_100h..108h = world_point[0..2]         ; 00484320..00484335
          else
004842ec      m = 00B6E0D0(parent)                            ; inverse world matrix
004842f9      p = 004142E0(ECX = world_point, out = stack, m)
004842fe      this->f_100h..108h = p[0..2]                     ; 00484300..00484313
```

The `!= 0` test is the compiler's `NEG/SBB/TEST imm32` idiom, a plain null test. The node call
happens **before** the cache and is handed the caller's buffer; the slot it reaches is settled
below, and it only reads that buffer, so the cached value is the value the caller passed in.

### What the node slot is

The node at `+110h` is built by `00B6F5A0`, whose constructor stores the vtable `00D62C88`
(`00B6F5E2`; `00B6F5C0` writes the base identity `00CEB130` first). The word at `00D62CB8`, that
vtable's `+30h`, is `00B6DAE0` `BSP_Transform_SetWorldPosition`:

```
00b6dae0  world[+120h..+128h] = arg[0..2]       ; the translation row of node+F0h
00b6db04  JMP node->vtable[34h] with arg replaced by node+F0h
```

So step 1 writes the point straight into the node's world matrix and tells the node its world
matrix changed. The slot is virtual, so a different node class would do something else; for the
anchors it is this one.

### What the cache is, and who reads it

`+100h` is not a free float3. `PointEffectInstanceStorage` puts a `0x40`-byte matrix at `+D0h`
(`relative_d0`) and the node pointer at `+110h`, so `+100h..+10Fh` is that matrix's **translation
row**. `004842C0` therefore writes the point, expressed in the parent's space, into the effect's
parent-relative placement.

The reader is `00867D00` `BSP_Effect_AdvanceTransform`:

```
00867d12  parent = this->p_8Ch;  if (parent == 0) skip
00867d41  if (!(parent->byte_5Ch & 2)) 00B6DB70(parent)     ; refresh the parent's world
00867d63  m = 00413920(left = &this->m_D0h, dst = stack, right = parent+F0h)
00867d78  this->p_110h->vtable[34h](m)
```

That is the whole loop: `004842C0` places the effect for the current frame and records where it
sits on the parent, and the effect's own update re-derives the world placement from the parent
every frame afterwards. **Only the two anchors take that path.** The other three point effects a
ship drives (`docs/UNIT_TIMERS.md`) are created through `00868420`, which passes a null parent, so
for them `004842C0` stores the world point unchanged and `00867D00` returns at its parent test. Nothing reads `+100h..+108h` by that offset; a byte-pattern search over
`FLD`, `MOVSS`, `MOV` and `LEA` with a `disp32` base+offset form finds no access to `+100h` inside
the effect class's code range `00868000..0086F000` at all. The matrix as a whole is what is read.

### Call sites

22 sites. The six on the unit path were read in full:

| site | containing function | receiver | point |
| --- | --- | --- | --- |
| `008259B9` | `008255B0` | `unit+9F0h` | bow water point, step 5 |
| `008259C9` | `008255B0` | `unit+9F4h` | stern water point, step 5 |
| `008243B2`, `008244E9`, `008245C4` | `00822C20` | the effect just created | the setup pass |
| `008349F6` | `00834820` | `unit+9E8h` | the bow-wave water line |
| `00834C85` | `00834A70` | `unit+A00h[i]` | a clamped spray point |
| `00834E72` | `00834CC0` | `unit+9ECh` | the stern-wave water line |

Seven of the remaining sites were sampled (`00488027`, `0070C350`, `007C3759`, `006E1129`,
`00854CB7`, `0080A9CD`, `00857290`). Every one loads `ECX` from a pointer field or an array slot
and pushes the address of a float3, twice (`00854CAD`, `0080A9C1`) next to the same `+54h` scalar
store and the same `00414DB0` pose refresh the unit path uses. The contract is uniform.

## The anchors themselves

### Producer

`00822C20`, the ship instance's setup pass, at `00823F09..008240BF`. Both anchors are created
together, under one gate:

```
00823f12  if ([class+5ACh] == 0) skip both               ; BowParticle, the shared template
00823f27  if ([class+5E0h] != 0) create                  ; the bow matrix translation
00823f38  else if ([class+5E4h] != 0) create
00823f49  else if ([class+5E8h] != 0) create
00823f58  else skip both
```

The three component tests are `UCOMISS` / `LAHF` / `TEST AH,0x44` pairs, so the creation arm is
taken on "not equal", which an unordered compare satisfies as well: a NaN component creates the
anchors. Creation itself is `008687C0` `BSP_PointEffect_CreateWithParentMatrix` twice:

| slot | `ECX` | `EDX` parent | template | matrix | transform | option | tail |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `unit+9F0h` | stack handle | `unit+4A4h` | `[class+5ACh]` | `class+5B0h` | 1 | 0 | 0 |
| `unit+9F4h` | stack handle | `unit+4A4h` | `[class+5ACh]` | `class+5F0h` | 1 | 0 | 0 |

`class+5B0h` and `class+5F0h` are two `0x40`-byte 4x4 matrices: `0082FE30` writes the bow one as
an identity at `00830195` and the stern one at `008301FF`, and fills the bow translation at
`008302B9` from element 0 of a vector at its source object `+48h`. The gate above is that
translation. The transform byte is 1, so admission uses `matrix * parent.world`
(`docs/POINT_EFFECT_CREATION_VARIANTS.md`), and the constructor keeps the original matrix and the
actual parent. The returned handle is retained on `obj+4` through `[00CE221C]` and the previous
occupant released through `[00CE2220]`.

### Layout the two inputs touch

From the producer (`008680B0`, `docs/POINT_EFFECT_INSTANCE.md`), not from the consumers:

| offset | field | what the anchors use it for |
| --- | --- | --- |
| `+0Ch`, `+10h`, `+14h` | pointer, count, capacity | the scalar targets `00815370` walks |
| `+18h`, `+1Ch`, `+20h` | pointer, count, capacity | the second scalar array |
| `+50h`, `+54h` | floats, both `1.0f` from the constructor | written by the wake routines, not by the anchors |
| `+84h` | retained template | `BowParticle` |
| `+8Ch` | parent transform | `unit+4A4h`, written at `0086823F` |
| `+90h` | 4x4 | the world cache the constructor snapshots |
| `+D0h` | 4x4 | the relative placement; `+100h` is its translation row |
| `+110h` | node | built by `00B6F5A0`, receives the world position |

### The scalar, `00815370`

`void __thiscall(effect, float value)`, `RET 4` at `008153DC`, body `00815370..008153DD`. Full
listing read. Two walks over `begin .. begin + count*4`, each element receiving
`element->vtable[14h](value)`. The first walk null-tests each element at `00815388`; **the second
does not** (`008153C0` dereferences straight away). `00815AA0` calls it on `unit+9F0h` and
`unit+9F4h` with the same value, behind the change latch at `unit+9D0h`
(`docs/UNIT_CONTROLLER.md`).

### The position, step 5 of `008255B0`

`00825870..008259CE`, read from the listing. Gate: `unit+9F0h`, `unit+9F4h` and `unit+4A4h` all
non-null. For each anchor the node's world matrix is refreshed when `node->byte_5Ch & 2` is clear
(`008258B2`, `00825903`), then `00413920(left = class+5B0h or class+5F0h, right = node+F0h)` and
the product's translation row is copied out (`008258D4`, `00825925`). Then:

```
0082594c  if (bow.y > -5.0f) {                                 ; 00CFBC84
00825972      bow.y   = (float)(0078CF20(ocean, bow.x, bow.z) + 0.5)
0082599f      stern.y = (float)(0078CF20(ocean, stern.x, stern.z) + 0.5)
          }
008259b9  004842C0(unit+9F0h, &bow)
008259c9  004842C0(unit+9F4h, &stern)
```

The ocean is `[[00E188A8]+19F0h]`, the lift `00D7A280` (the double `0.5`). The gate reads the
**bow** height only and decides for both points, so a stern anchor is never lifted on its own.

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/UNIT_INSTANCE_UPDATE.md` step 5: "`a = row3(descriptor->m_5B0h * node->world)`" | the matrices are in the **class block** at `unit+538h`, not the descriptor at `unit+354h`; `docs/UNIT_TIMED_SUBUPDATES.md` keeps those two apart | `MOV EAX,[ESI+0x538]` at `008258B7` and `00825908` |
| `docs/UNIT_CONTROLLER.md`: "the object at each is an emitter group that takes both a position and a scalar" | it is a point-effect instance, the same class `008689C0`/`008687C0` build; "emitter group" describes only its two component arrays | `008687C0` at `00823F9C` and `00824051`, and the `0x114` storage in `docs/POINT_EFFECT_INSTANCE.md` |
| implied by the field name `kUnitOffBowAnchorSink`: `+100h` is a free cached point | `+100h` is the translation row of the relative matrix at `+D0h`, read as part of that matrix by `00867D00` | `PointEffectInstanceStorage::relative_d0`, and `LEA ECX,[ESI+0xD0]` at `00867D63` |

Nothing in the earlier docs is contradicted on the call sequence itself.

## Coverage

| routine | body | coverage |
| --- | --- | --- |
| `004842C0` | `004842C0..00484341` | complete |
| `00815370` | `00815370..008153DD` | complete |
| the creation gate in `00822C20` | `00823F09..00823F5E` | complete for the gate; the rest of `00822C20` is not this packet's |
| the anchor creation in `00822C20` | `00823F5E..008240BF` | complete as an argument table; the ref-count and EH scaffolding is `docs/POINT_EFFECT_OWNER.md`'s |
| step 5 of `008255B0` | `00825870..008259CE` | complete for the water rule; the step belongs to `docs/UNIT_INSTANCE_UPDATE.md` |
| `00867D00` | `00867D00..00867EDA` | partial: only `00867D00..00867D7B`, the parent-times-relative path, was read |
| `0082FE30` | `0082FE30..00831801` | partial: only `00830195..008302D1`, the two matrix identities and the bow translation |

## Uncertainties

1. The node vtable slot `+30h` is resolved for the class `00B6F5A0` builds. Any other node class
   under `+110h` would take a different path; no such case was found, but none was excluded.
2. `0082FE30`'s source object (`ESI`) and the vector at its `+48h` were not identified, so where
   the bow anchor position ultimately comes from (a model locator is the obvious candidate) is
   **not** established. The ship Lua reader `00831840` does not write `+5B0h` or `+5E0h`: they are
   not among its 67 keys.
3. The second scalar array of `00815370` has no null test. Whether the producer guarantees it is
   dense was not checked; the reconstruction skips nulls rather than reproduce a crash.
4. `+50h` and `+54h` of an anchor keep the constructor's `1.0f`: no writer of either was found on
   the anchor path. Only the wake effects write them.

## no_ghidra_function

| start | inclusive end | evidence |
| --- | --- | --- |
| none | | Every address this packet documents lies inside a Ghidra function: `004842C0..00484341`, `00815370..008153DD`, `00822C20..00824B57`, `0082FE30..00831801`, `00867D00..00867EDA`, `008255B0..00825DD4`, `00B6DAE0..00B6DB0E`. `bsp.py ghidra flow` reports 0 gaps for `004842C0`. |

## Follow-up packets

| id | addresses | files | contract |
| --- | --- | --- | --- |
| `ship_class_anchor_matrices` | `0082FE30` `00830195` `008302B9` | `docs/SHIP_CLASS_ANCHOR_MATRICES.md`, `reports/ship_class_anchor_matrices.json` | Where `class+594h`, `+5A0h`, `+5B0h`, `+5E0h`, `+5F0h`, `+638h`, `+644h`, `+650h` and `+654h` come from, given that the Lua reader `00831840` writes none of them. The vector at the setup object's `+48h` is the entry point. |
| `point_effect_transform_advance` | `00867D00` `00B6DB70` `00866F50` | `docs/POINT_EFFECT_TRANSFORM_ADVANCE.md`, `reports/point_effect_transform_advance.json` | The rest of `00867D00`: the `+80h` age accumulator, the `byte_Ah` branch into `00866F50`, and the no-parent path the anchors never take. |
