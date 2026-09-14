# Collision shapes: the class set, who installs them into a node, and which one answers a blast

Addresses: 0098AAE0, 0098AAB0, 0098AC20, 0098C460, 0098C510, 004E6480, 004E6540, 004E6470,
006D1400, 006D1420, 006D1990, 006D3C10, 00711020, 00711460, 00712440, 0070F090, 0070F710,
0070F720, 00724510, 00747244, 00851850, 00882AC0, 0092AAE0, 00929E60, 00428800, 004F11C0,
0087FF80, 00929B80, 00929FF0, 004F13A0, 006D30F0, 006D3100, 00CE89DC, 00CE89E8, 00CEA050,
00CEA05C, 00CF8B18, 00CF8B94, 00CFD768, 00CFD7B8, 00D194A8, 00D194B8

This packet closes the labelled gap left by `docs/EXPLOSION_RADIAL_DAMAGE.md`: that doc enumerated
the shape classes from the three known segment traces, found every sphere slot stubbed to false,
and said the way to close it is the producer of `node+D0h`. Reading the producers found a fifth
shape class the trace-first enumeration could not see, and it is the one a blast hits.

**The answer.** A blast on a ship hull is answered by the **unit-part collision shape**, vtable
`00CFD768`, sphere slot `0070F720`. It is reached through the part's collision node (vtable
`00CFD7B8`), not through the ship entity's own node. The ship entity's node carries exactly one
shape, the transformed box at `entity+1A4h`, whose sphere slot `00929FF0` is `XOR AL,AL; RET 0Ch`,
so the hull node itself returns false for every sphere query.

## Correction to the node+D0h reading

`docs/EXPLOSION_RADIAL_DAMAGE.md` describes `node+D0h` as "the inline shape array". It is an inline
array **of shape pointers**, and the element type matters because the installers store addresses of
objects that live elsewhere. `0098AAE0` walks it at `0098ABB2`-`0098ABF3`:

```
0098abb2: MOV EAX,dword ptr [ECX + 0xf8]        ; count
0098abba: LEA EBP,[ECX + EAX*0x4 + 0xd0]        ; end
0098abc1: LEA ESI,[ECX + 0xd0]                  ; begin
0098abd6: MOV ECX,dword ptr [ESI]               ; *it  -> the shape pointer, becomes `this`
0098abd8: MOV EDX,dword ptr [ECX]
0098abda: MOV EAX,dword ptr [EDX + 0x4]         ; shape->vtable[4]
0098abdd: PUSH EBX / PUSH ECX / FSTP [ESP] / PUSH EDI
0098abe3: CALL EAX
0098abee: ADD ESI,0x4
```

The `PUSH ECX` at `0098ABDE` is a stack-slot reservation that `FSTP float ptr [ESP]` immediately
overwrites, so the three stack arguments are `(centre, radius, record)` and ECX stays the shape:
`__thiscall bool shape->vtable[4](const float* centre, float radius, HitRecord* record)`, `RET 0Ch`.

Element stride 4 and the count field at `+F8h` give 10 slots (`(0xF8-0xD0)/4`), which is what
`kCollisionNodeShapeSlots` in `include/bsp/hit_narrowphase.hpp` already records.

Two further corrections to the same doc's node reading, from the base sub-constructor `004E6480`:

| field | evidence | meaning |
| --- | --- | --- |
| `+F8h` | `004E64E1` `MOV [ESI+0F8h],EBX` with EBX=0 | shape count, starts empty |
| `+FCh` | `004E64F7` `PUSH 8; CALL 00BF55BE`, then `004E64FC` `MOV [ESI+0FCh],EAX` | child array, heap, **not** inline |
| `+100h` | `004E64E7`, zero | child count |
| `+104h` | `004E64ED`, `2` | child **capacity**, a growable vector, not a second count |

So the node's children are a growable pointer vector with an initial capacity of two, and the
shape array is the only inline one.

## The shape interface

Base vtable `00CE89DC` = `{ 00BF698E __purecall, 00BF698E __purecall, 004E6470 }`. Three slots.
Slot 8 is `RET 4` in the base, one stack argument. Every concrete class must define slots 0 and 4.

Slot 0 is the segment trace and slot 4 the sphere test. The corroboration is the stack cleanup:
every slot-4 body found ends in `RET 0Ch`, matching the three pushes `0098AAE0` makes, and every
slot-0 body ends in `RET 0Ch` matching the three pushes `0098AC20` makes at `0098AC58`ff.

### The complete class table

Enumerated by two independent sweeps of the on-disk image, not by following traces:

1. every 4-byte-aligned occurrence of the base slot-8 stub `004E6470` in `.rdata` (4 vtables);
2. every occurrence of the second slot-8 body `0070F710`, found through the copy constructor
   `00711020`, which writes vtable `00CFD768` at `00711044` (1 vtable).

The first sweep is what `docs/EXPLOSION_RADIAL_DAMAGE.md` effectively did; it cannot see `00CFD768`
because that class overrides slot 8, which is exactly why the gap existed.

| vtable | class | constructor | slot 0, segment trace | slot 4, sphere test | slot 8 | record |
| --- | --- | --- | --- | --- | --- | --- |
| `00CE89DC` | base shape | inlined; `00711020` writes it at `0071102F` before the derived one | `00BF698E` `__purecall` | `00BF698E` `__purecall` | `004E6470` `RET 4` | `0x1C` |
| `00CEA050` | subobject shape at `entity+344h` | `004F11C0` (`004F1206` base, `004F1296` derived); also `008828E0` at `00882928` | `0087FF80` | `004F13A0` = `XOR AL,AL; RET 0Ch` | `004E6470` | `>= 0x90` |
| `00CF8B94` | the factory-built shape of `006D1990` | `006D1990` (`006D199A` base, `006D19FD` derived) | `006D30F0` = `XOR AL,AL; RET 0Ch` | `006D3100` = the same stub | `004E6470` | `contract: unread` |
| `00CFD768` | **unit-part shape** | copy ctor `00711020`; list node built by `00711460` | `00724510` | **`0070F720`, a real test** | `0070F710` | `0x28` |
| `00D194B8` | transformed box at `entity+1A4h` | `00929E60` (`00929E9D` base, `00929F3A` derived); `0092A040` at `0092A080` | `00929B80` | `00929FF0` = `XOR AL,AL; RET 0Ch` | `004E6470` | `0x20` |

`006D30F0` and `006D3100` are two separate copies of `32 C0 C2 0C 00`, read from the raw image;
neither has a Ghidra function. So the `00CF8B94` class stubs **both** geometry slots, and only
`00CFD768` answers a sphere at all.

### The node classes, re-differenced

The same raw sweep over `0098B9C0` (the node's default segment slot 8) finds exactly five node
vtables, confirming the earlier doc's differencing and closing it against the xref gap: the Ghidra
bridge's `xrefs` only reports references whose source lies inside a function, so vtable slots in
unclaimed `.rdata` are invisible to it.

| node vtable | slot 8, segment | slot 0Ch, sphere | owner |
| --- | --- | --- | --- |
| `00CE89E8` | `0098B9C0` | `0098AAE0` | base collision node, `004E6540` |
| `00CEA05C` | `0098B9C0` | `0098AAE0` | the `004F11C0` class |
| `00CF8B18` | `0098B9C0` | `0098AAE0` | the `006D1430` / `006D1C20` class |
| `00CFD7B8` | `0098B9C0` | **`0070F090`** | unit part, the only override |
| `00D194A8` | `0098B9C0` | `0098AAE0` | tickable game entity |

`0070F090` re-pushes its four arguments, calls `0098AAE0` and normalises with `SETNE`. It changes
no geometry: the part's hit comes entirely from its shapes.

## The shape record

The common prefix is written by `0098AAB0`, `__thiscall(shape; const float* min, const float* max)`,
`RET 8`, six `FLD`/`FSTP` pairs into `shape+4h` through `shape+18h`:

| offset | field | evidence |
| --- | --- | --- |
| `+0h` | vtable | `0071102F`, `00711044` |
| `+4h`..`+0Ch` | bounds min | `0098AAB6`, `0098AABF`, `0098AAC6` |
| `+10h`..`+18h` | bounds max | `0098AACB`, `0098AAD1`, `0098AAD8` |

`00711020`, the unit-part shape's copy constructor, copies that prefix and three more dwords, and
`00711460` allocates `0x30` bytes for a list node whose value starts at `+8h`. `0x30 - 8 = 0x28`,
so the unit-part record is exactly `0x28`:

| offset | field | evidence | use |
| --- | --- | --- | --- |
| `+1Ch` | collision node pointer | `00711035`/`0071103E` copy it | `00724510` reads `+50h` and `+90h` off it |
| `+20h` | collision body | `0071104D`/`00711053` | `0070F720`: flag `+5Ch` bit 1, prepare `00B6DB70`, test `00B6E0D0`, tail `+F0h` |
| `+24h` | owner back-pointer | `00711065`/`0071106B` | the `this` of `00723F80`, `00723E90` and `00723840` |

`+50h` and `+90h` off the node are `kSpatialNodeOffWorldMatrix` and `kSpatialNodeOffInverseMatrix`
in `include/bsp/spatial_index.hpp`. That settles the two magic offsets the earlier doc recorded for
the transformed box: the box is embedded at `entity+1A4h` and its node at `entity+1C4h`, so the
box's `this+70h` is `node+50h` and its `this+B0h` is `node+90h`. Every shape class reaches the same
two matrices; the embedded ones by a fixed positive displacement from themselves, the unit-part one
through the explicit pointer at `+1Ch`. The box record therefore ends at `entity+1C4h`, size `0x20`.

The subobject shape's `this+8Ch` is `entity+3D0h` (`0x344 + 0x8C`), which the installer at
`00883FA8` loads as `MOV ECX,[ESI+3D0h]` four instructions after publishing the shape. Its record
is at least `0x90` and its interior is `contract: unread`.

## Who writes node+D0h and node+F8h

Seven indexed stores to `+D0h` exist in `.text`, each paired one-to-one with a `+F8h` bump within
seven bytes. There are no others: the scan covered every `<store op> <modrm mod=10> [<sib>] <disp32
= D0h>` encoding in the section, and no unindexed store to `+D0h` belongs to a collision node.

| # | store | bump | container | node | shape installed |
| --- | --- | --- | --- | --- | --- |
| A | `006D140A` | `006D1411` | none (`006D1400`-`006D141A`) | `ECX`, the node | the `[ESP+4]` argument |
| B | `006D3DF4` | `006D3DFB` | `FUN_006D3C10` | `[ESI+828h]` | `EAX` from `006D1990`, cached at `[ESI+82Ch]` |
| C | `0071274A` | `00712751` | `FUN_00712440` | `ESI`, the node | `EBX+8`, a list element |
| D | `00747244` | `0074724B` | none | `[ESI+360h]` | `EBP+8`, a list element |
| E | `00851904` | `0085190B` | `FUN_00851850` | `[ESI+19Ch]` | `EBP+8`, a list element |
| F | `00883F9A` | `00883FA1` | none | `ESI+1E4h` | `ESI+344h`, the subobject shape |
| G | `0092B2DA` | `0092B2E1` | `FUN_0092AAE0` | `ESI+1C4h` | `ESI+1A4h`, the transformed box |

`006D1400` is the canonical append, `__thiscall(node; shape)`, `RET 4`:

```
006d1400  mov  eax, [ecx + 0xf8]
006d1406  mov  edx, [esp + 4]
006d140a  mov  [ecx + eax*4 + 0xd0], edx
006d1411  add  dword [ecx + 0xf8], 1
006d1418  ret  4
```

and `006D1420`-`006D142A` is `node->shapeCount = 0`, `RET`. Neither has a Ghidra function and a
scan of every `E8`/`E9` rel32 in `.text` finds **zero** direct calls to either: they are the
out-of-line copies of an inline member, and rows B, C, D, E, F and G are that same idiom inlined.
The store never bounds-checks against the 10 slots.

Rows F and G read the count through the owner rather than the node: `[ESI+2DCh]` is
`0x1E4 + 0xF8` and `[ESI+2BCh]` is `0x1C4 + 0xF8`, the same field.

### The installer sequence per owner kind

**Tickable game entity (a ship), `FUN_0092AAE0`, called by `FUN_00935D30`.** One shape, always.
`00929E60` has already constructed the box in place: base vtable at `entity+1A4h` (`00929E9D`),
node base `004E6480` on `entity+1C4h` (`00929EEF`, `00929F0F`), derived box vtable at `entity+1A4h`
(`00929F3A`). `FUN_0092AAE0` then publishes it and registers the node:

```
0092b289  MOV ECX,[ESI + 0x2bc]          ; = node->shapeCount
0092b2b3  LEA EDI,[ESI + 0x1c4]          ; the node
0092b2cb  LEA EAX,[ESI + 0x1a4]          ; the transformed box
0092b2da  MOV [EDI + ECX*4 + 0xd0],EAX
0092b2e1  ADD [EDI + 0xf8],EBX           ; EBX = 1, set at 0092b050
0092b2f9  CALL 0x0098a920                ; node bounds from two stack vectors
0092b305  CALL 0x0042e630                ; the spatial index, no arguments
0092b30c  CALL 0x0098ba10                ; insert the node into the index
```

**Unit part, `FUN_00712440`, called by `BSP_UnitPartInstance_Construct` at `007135C0`.** The node
owns a `std::list` of shapes; the installer publishes element addresses into the array. The list
lives at `node+194h` (the checked-iterator proxy), `node+198h` (head) and `node+19Ch` (size); the
element value sits at list-node `+8h`. `00711460` is `_Buynode`: `operator new(0x30)`, links at
`+0h`/`+4h`, then `00711020` copy-constructs the `0x28` value at `+8h`. Before publishing, each
element gets its bounds through `0098AAB0` at `00712723`. The shape data is read from the vector at
`[node+160h] + 3Ch` (iterated `+4h` to `+8h`) and a second `0x10`-byte-element vector at
`node+16Ch`..`+170h`; the producer of those records is `contract: unread`.

**Rows D and E** repeat the list walk against a node held by pointer, `[owner+360h]` and
`[owner+19Ch]`, with the same `node+194h` list and the same `+8h` value offset. Neither store site
has a containing Ghidra function.

**Row F**, the subobject owner, publishes the embedded `owner+344h` into the node at `owner+1E4h`.
**Row B** is the only factory form: `FUN_006D1990` returns a shape in EAX, `FUN_006D3C10` caches it
at `[ESI+82Ch]` and appends it to `[ESI+828h]`; the shape it builds stubs both geometry slots.

Nothing in the seven rows reads the model's collision data directly; the part path reaches it
through `[node+160h]`, and the model loader is another owner's file.

## The real sphere test, 0070F720

`__thiscall(shape; const float* centre, float radius, HitRecord* record)`, `RET 0Ch`.

```
0070f724  MOV EDI,[ESI + 0x20]           ; the collision body
0070f727  TEST byte ptr [EDI + 0x5c],0x2 ; prepared?
0070f72d  MOV ECX,EDI / CALL 0x00b6db70  ; prepare, only when the bit is clear
0070f734..0070f745                        ; push record, reserve+FSTP radius, push centre
0070f746  MOV ECX,[ESI + 0x20]
0070f749  CALL 0x00b6e0d0                ; the library sphere query -> EAX
0070f74e  MOV ECX,[ESI + 0x24]           ; the owner
0070f752  ADD EDI,0xf0 / PUSH EDI / PUSH EAX
0070f759  CALL 0x00723f80                ; -> AL, the return
```

Argument offsets are settled after the two pushes: at `0070F734` `ESP+14h` is the third argument and
`ESP+10h` the second, and after the two argument pushes `ESP+14h` is the first. `00B6DB70`,
`00B6E0D0` and `00723F80` are contracts; `00B6xxxx` is the physics library and is not ported.

This is why the sphere slot is a stub on the segment classes. The transformed box and the subobject
shape carry no volume of their own: the box's trace transforms a segment into the node's local
frame and hands it to a physics-library box test, and the subobject's trace forwards to
`[this+8Ch]->vtable[3Ch]`. Both are ray-shaped queries whose library entry takes two endpoints.
Only the unit-part shape holds a body pointer (`+20h`) whose library interface has a sphere entry
at `00B6E0D0`, so it is the only class that can answer one.

## 0098C460, the append: copy, not ownership

`__thiscall(vector; const HitRecord* record)`, `RET 4`. The vector is the four-field MSVC checked
container: `+0h` proxy, `+4h` first, `+8h` last, `+0Ch` end.

The element stride is `0x54`: both size computations multiply by `30C30C31h` and `SAR EDX,4`
(`0098C477`-`0098C486` and `0098C491`-`0098C4A0`), which is division by `0x54`. That matches the
`54h`-byte element `00904470` walks.

* fast path, `0098C4A6`-`0098C4CD`: `0098BDE0(dest=[ESI+8]; EDX=1, [src, vector, src, &flag])`
  constructs the element in place, then `[ESI+8] += 0x54`.
* slow path, `0098C4D8`ff: `0098C3B0(vector; &tmp, vector, end, src)`, the reallocating insert.

Both paths **construct a new element from the source**; neither stores the source pointer. So the
append copies and `004704B0` destroying the stack record at `0098C606` is safe. Whether `+3Ch` is
deep-copied is `0098BDE0`'s contract, but it must be, or every gathered record would hold a pointer
freed one instruction later. This answers the `contract: unread` that
`docs/EXPLOSION_RADIAL_DAMAGE.md` left on `0098C460`.

## 00428800, the pre-gate call

`RET 8`: two stack arguments, the radius and the damage as floats, as
`docs/EXPLOSION_RADIAL_DAMAGE.md` records. `BSP_Explosion_ApplyRadialDamage` at `0084BAD0` is its
only caller, and the call precedes the world-mode test, so a suppressed blast still runs it.

What is established:

* `00428803`-`0042880F`: the container is `[[00E188A8] + 21D0h] + 28h`, reached off the world
  singleton.
* `00428812`: `if ([container+8h] == 0) return` — an empty container skips everything.
* `00428824`-`00428833`: `FLD [ESP+74h]; FMUL ST0` — the **radius is squared** and kept at
  `[ESP+10h]`.
* `0042886B`-`00428890`: per element, `ESI = [EBX+10h]`, then `0085BF90(ESI+9Ch, ESI+A8h)` and
  `FCOMIP` against the squared radius with `JBE` skipping the element. A squared-distance reject.
* `00428896`-`004288C9`: elements that survive enter a nested list at `ESI+B4h`.
* the surviving body calls `00414DB0`, `00413920` (on `ESI+74h`), `00480BF0`, `00BD2FC0`,
  `004B0F80`, `00480C20` and `00427FE0`.

`coverage: partial`. The radius is used as a squared-distance filter against a world-owned
container and the damage argument's consumer was not located in the budget; `00428900`-`00428B66`
is unread, and every callee above is `contract: unread`. The routine writes nothing through either
argument pointer, so nothing on the blast path depends on its result.

## Host table

One row per native call site the reconstruction turns into a virtual method. Argument detail is in
`reports/collision_shapes.json`.

| site | callee | method | this / args | ret | gate |
| --- | --- | --- | --- | --- | --- |
| `0092B2F9` | `0098A920` | `set_node_bounds` | the node; two vectors | — | after the install |
| `0092B305` | `0042E630` | `spatial_index` | none | the index | always |
| `0092B30C` | `0098BA10` | `insert_node` | the index; node, 0, 0, 0 | — | always |
| `00712723` | `0098AAB0` | `set_shape_bounds` | the shape; min, max | — | per list element |
| `00711466` | `00BF681B` | `allocate_list_node` | none; `0x30` | the block | per shape |
| `00711493` | `00711020` | `copy_construct_shape` | ignored; dest, src | — | per shape |
| `0070F72F` | `00B6DB70` | `prepare_body` | the body | — | `body+5Ch` bit 1 clear |
| `0070F749` | `00B6E0D0` | `body_sphere_query` | the body; centre, radius, record | opaque | always |
| `0070F759` | `00723F80` | `owner_accept_sphere_hit` | `shape+24h`; `body+F0h`, the query result | bool | always |
| `00724530` | `00723E90` | `owner_accept_segment_hit` | `shape+24h`; node matrices, three args | bool | always |
| `0098C4C5` | `0098BDE0` | `construct_record` | the storage end; src, vector, src, flag | — | spare capacity |
| `0098C4F3` | `0098C3B0` | `insert_record_grow` | the vector; tmp, vector, end, src | — | no spare capacity |
| `006D3DD9` | `006D1990` | `build_factory_shape` | the factory; float, arg | the shape | non-null branch |

## Coverage

| routine | coverage |
| --- | --- |
| `0098AAE0` shape-array walk, `004E6480`, `006D1400`, `006D1420`, `00711020`, `00711460`, `0098AAB0`, `0070F710`, `0070F720`, `00724510` | complete |
| `0098C460` | complete; `0098BDE0` and `0098C3B0` are contracts |
| the seven `node+D0h` writers | complete as sites; the containing routines of C, D, E, F, G are read only around the install |
| `00428800` | partial: `00428803`-`004288C9` only; `004288CA`-`00428B6A` unread |
| `FUN_00712440` | partial: the publish loop and the source vectors at `node+160h`/`+16Ch`; the record decode is unread |
| `00CF8B94` record layout, `00CEA050` record interior, `00B6xxxx`, `00723F80`, `00723E90` | `contract: unread` |

Open questions, in the order a later packet should take them:

1. the producer of the vector at `[part node+160h]+3Ch` — whether the part's shapes come from the
   model's collision data, the `fizika_%02d` nodes or the class descriptor is still unanswered;
   this packet establishes only that the part node reads them from there.
2. rows D and E: which owners hold a node at `+360h` and `+19Ch`, and when they re-publish.
3. `00428800`'s damage consumer and the world container at `world+21D0h`.
4. whether `0070F720` can write `record+34h`, the hull segment index, through `00723F80`. The
   earlier doc concluded the blast path cannot produce one; that conclusion rested on the class set
   this packet has now extended, so it is reopened rather than settled.

## Flagged by docs/HIT_HULL_SEGMENT.md (packet cc7_hit_shape_hull_segment) - not resolved

Two readings of `shape+24h` are on the table and this note records the disagreement rather than
settling it. The field table above calls `+24h` an **owner back-pointer**, and names the calls it
receives `owner_accept_segment_hit` / `owner_accept_sphere_hit`. Packet cc7_hit_shape_hull_segment
reads it instead as **the geometry**, on the grounds that `00724517 MOV ECX,[ECX+24h]` makes it the
receiver of `00723E90`, whose body walks a geometry element list through `00723D60` and reads the
selected element's `+4h` and `+8h` into the hit record.

What is established either way: `00724517` does load `shape+24h` into `ECX` as the receiver of the
`00724530` call, and `00723E90` saves that receiver in `ESI` at `00723E9E`. What is not established
is whether that object is the owning entity that happens to expose geometry, or the geometry
container itself. Deciding it needs the producer of `shape+24h` at `00711065`/`0071106B`, which
neither packet read. Until someone reads it, prefer the neutral description "the object `00723E90`
is invoked on" over either label.

A second item flagged by the same packet: the framing that `node->vtable[0Ch]` allocates the
part-hit array at `record+3Ch` is off by three frames. The chain is
`0070F090` -> `0098AAE0` -> `0070F720` -> `00723F80` -> `006D2E30`, and `0070F090` allocates
nothing; `006D2E30` is the allocator.

## Correction from NATIVE_UNIT_PART_COLLISION_AH.md

The complete `00712440` caller is now reconstructed and original-byte fixture
checked. Only the base list at node+188 (head+18C) publishes its new shape
addresses into node+D0. Matches in non-base damage rows enter the separate list
at+194 (head+198), which does not publish here. Both lists contribute aggregate
bounds. The former description of +194 as the list directly supplying this
publication was incomplete. Record+0 supplies bounds through `00723170` and
becomes shape+24; record+4 is the identity compared with group rows and becomes
shape+20. These producer relationships alone do not prove the old pointer-class
labels. The new module also includes the complete local bounds/extent/centre
setter at `0098A920`; see the linked packet for ABI and validation boundaries.
