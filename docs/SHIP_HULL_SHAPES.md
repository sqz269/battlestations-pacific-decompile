# The hull's collision shapes: which model nodes become them, and the AABB that fixes the inertia

Addresses: 00C5C940, 00C55FC0, 00C57F50, 00C57C40, 00931A10, 00931300, 009313C0, 00931450,
00931530, 00937B70, 00937C00; read as contracts 00C5D580, 00C57B90, 00C585B0, 00C582E0,
00C58840, 00407ED0, 00C50470, 0071AD50, 0071BA20, 00B6F9A0, 00B6D800, 00B6DB60, 00B6E0A0,
00C336C0, 00C31F90, 00424C40.

Packet `cc_hull_shapes`, 2026-09-12. Reconstructed in `include/bsp/ship_hull_shapes.hpp` and
`src/ship_hull_shapes.cpp`; semantic C++ interfaces for MSVC Win32, not drop-in binary
replacements. Descriptive names are hypotheses, not recovered symbols. The saved project is
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. This worker made no Ghidra
mutation; the ledger records the new names.

`docs/SHIP_HULL_BODY.md` closed the hull body's mass, damping and inertia rule and left one
input open: the AABB the attached collision shapes produce, whose producer `00C5C940` it
recorded as "not read". `docs/DYN_COLLISION_PASS.md` and `docs/DYN_LCP_IMPULSE_MATH.md` both
list the same routine as the unread supplier of collision geometry. This packet reads it and
the model-side walk that feeds it.

## The short answer

A hull shape is **a convex mesh, not a box and not a capsule**, and the nodes that produce
one are **not** the `fizika_%02d` nodes. The walk at `00938F61..0093918C` goes over the
model's collision-record list at `model+4Ch` and keeps every record whose owning node is one
of four:

| # | owner | how it is resolved |
| --- | --- | --- |
| 1 | the node named `firstnode` (`00D1968C`) | `0071AD50` at `00938F4E`, compared at `00939026` |
| 2 | the node pointer at `model+0Ch` | read at `00938F9D`, compared at `00939039` |
| 3 | the node named `front` (`00D196A0`) | `0071AD50` at `00938DB9` into `controller+370h`, compared at `00939053` |
| 4 | the node named `back` (`00D19698`) | `0071AD50` at `00938DE2` into `controller+374h`, compared at `0093906C` |

`0071AD50` returns null for a name the model does not carry, so a model without a `front`
node simply contributes nothing through that owner; the comparison is a plain pointer test
and no shape is synthesised in its place.

Each kept record becomes one `48h`-byte shape descriptor. `Dyn::World::CreateBody` hands
each one to `00C5C940`, which allocates a `Dyn::ConvexMeshShape` and links it into the body.
The shape's own AABB is the mesh's local box, expanded by `0.02` and pushed through the
shape transform; the body's AABB is the union of those boxes, and `00939A8E..00939C05` turns
its span into the hull's inertia.

## The 48h-byte shape descriptor

`00931A10` stamps the default record and `009392FA..009396B8` overrides five fields of it.
The field meanings are settled by `00C57F50`, the type-4 constructor, which is the producer
of every shape-object field; `docs/DYN_COLLISION_PASS.md` had already recorded the shape
object's own offsets (`type +08h`, `restitution +24h`, `friction +28h`, `group +2Ch`,
`mask +30h`, list next `+208h`) from the collision pass, and they line up exactly.

| desc | meaning | shape field | default (`00931A10`) | what `00937C90` writes |
| --- | --- | --- | --- | --- |
| `+00h` | restitution | `+24h` (`00C57F85`) | `0` | nothing |
| `+04h` | friction | `+28h` (`00C57F8B`) | `0` | the material's `Friction` at `00939370` |
| `+08h` | collision group | `+2Ch` (`00C57F92`) | `0` | `1` at `009394DD`, `2` for the periscope at `009398C4` |
| `+0Ch` | collision mask | `+30h` (`00C57F98`) | `0` | `0Dh` at `009394A9`, OR the class bit at `009395E2`; `5` for the periscope at `009398D6` |
| `+10h` | shape type | `+08h` (`00C57F7F`) | **`4`** (`00931A85`) | nothing for a hull shape; written again as `4` at `00939724` for the periscope |
| `+14h` | geometry | `*(+14h)` -> `+210h` (`00C57FB4`) | null | `record + 0Ch` at `009393D1` and `00939886` |
| `+18h`..`+44h` | 4x3 transform | `+34h`..`+60h` (`00C57FCA`, twelve dwords) | identity | only the translation `+3Ch..+44h` at `0093946E`; the periscope gets a real rotation |

The transform is four rows of three floats, the same row-vector layout
`DynBodyDescriptor` uses: basis rows at `+18h`, `+24h`, `+30h` and the translation at
`+3Ch`. The identity diagonal in the default record sits at `+18h`, `+28h`, `+38h`, which is
what fixes the row stride at twelve bytes.

**A hull shape therefore keeps the default type 4 and the default identity rotation.** The
fill loop never touches either. Its translation is the record's float3 verbatim, stored with
`MOV` at `0093946E`/`00939477`/`0093947A`, so the mesh is placed by an offset alone.

`00C5C940` switches on `+10h`: `CMP EAX,5` / `JA` at `00C5C968` makes anything above 5 a
no-op and case 3 has no jump-table entry either, so those two attach nothing at all.

| type | allocation | constructor | vtable it installs | slot 0 of that vtable | class |
| --- | --- | --- | --- | --- | --- |
| 0 | `operator new(214h)` at `00C5C9A7` | `00C585B0` | `00D7A058` | `00C58520` | sphere |
| 1 | `operator new(21Ch)` at `00C5C978` | `00C57B90` | `00D7A0C8` | `00C57880` | box |
| 2 | `operator new(218h)` at `00C5C9FA` | `00C582E0`, pushed `(shape, body)` with no `ECX` | `00D7A098` | `00C57FF0` | cylinder |
| 3 | none | none | - | - | nothing is attached |
| **4** | the pooled allocator `00407ED0` at `00C5CA20` | **`00C57F50`** | `00D7A0AC` | `00C57C40` | **convex mesh** |
| 5 | `operator new(230h)` at `00C5CA40` | `00C58840` | `00D7A194` | `00C58690` | terrain |

Every constructor writes `Dyn::Shape::vftable` `00D7A04C` first and then its own, and the
first dword of each of those is the bounds refresher the ledger had already named
(`Dyn_SphereShape_vslot0`, `Dyn_BoxShape_vslot0`, `Dyn_CylinderShape_vslot0`,
`Dyn_ConvexMeshShape_vslot0`, `Dyn_TerrainShape_vslot0`), so the class column is the
vtable's, not a guess. `00C57F50` names `Dyn::ConvexMeshShape::vftable` outright.
`00407ED0` is a critical-section-guarded fixed-size pool over `operator new(10D04h)`
blocks; its element size is not visible from this call site.

## From the descriptor to the body's AABB

`00C5C940` is `__fastcall(ECX = descriptor, EDX = body)`, `RET`, returning the new shape in
`EAX`. Its only caller is `00C5D580` at `00C5D8D4`, once per entry of the descriptor's shape
vector: the data pointer is `desc+78h` and **`desc+7Ch` is a count**, not an end pointer
(`LEA ECX,[EAX+EDX*4]` at `00C5D8C8`), which refines the `vector<shape*>` row in
`docs/SHIP_HULL_BODY.md`. After the constructor it links the shape in:

```
shape+208h      = old [body+70h]        ; 00C5CA7B
[old head+20Ch] = shape                 ; 00C5CA84
[body+70h]      = shape                 ; 00C5CA8A
```

then calls `00C55FC0` with the body still in `ESI`, and, only when the body had no shape
before (`EBP` loaded at `00C5C963`), `00C50470([[body]+444h])` at `00C5CA9F`.

`00C57C40` (vtable slot 0 of `00D7A0AC`, also called directly by the constructor at
`00C57FCE`) fills the shape's own box:

```
mesh   = shape[+210h]                                  ; 00C57C4C
lo     = mesh[+18h..+20h] - 0.02                       ; 00C57C63..00C57CA1, the double at 00D7A2F8
hi     = mesh[+24h..+2Ch] + 0.02                       ; 00C57CA5..00C57CBD
centre = (lo + hi) * 0.5                               ; 00C57CC1..00C57D1F, the double at 00D7A280
half   = (hi - lo) * 0.5                               ; 00C57D23..00C57D56, three FSUBRP then the same 0.5
c'     = centre * R + t                                ; 00C57D5A..00C57DCE, R = shape+34h/+40h/+4Ch, t = +58h
h'     = |R| * half                                    ; 00C57DD2..00C57EBA, nine 00401170 magnitudes
shape[+0Ch..+14h] = c' - h'                            ; 00C57EBE..00C57F0E
shape[+18h..+20h] = c' + h'                            ; 00C57F19..00C57F37
```

The x87 order is taken from the raw bytes, not the printed mnemonics: `dc c9` at `00C57D07`
is `FMUL ST(1),ST(0)` while `d8 c9` at `00C57D13` and `00C57D1D` is `FMUL ST(0),ST(1)`, and
`de e6`/`de e1`/`de e2` are `FSUBRP`, so `[ESP+0Ch..14h]` holds `hi - lo` and the halving
happens afterwards at `00C57D3D..00C57D56`. This is the same conservative box rotation
`docs/DYN_COLLISION_PASS.md` records for the broad-phase proxy at `00C5715C`.

`00C55FC0` then produces the body's box. The body is in `ESI` and there are no stack
arguments; both call sites establish the register by filtering their own listings
(`00C5C961 MOV ESI,EDX`, the body argument; `00C57F3A MOV ESI,[ESI+4]`, which `00C57F7C`
had set to the constructing body).

```
B+38h..+40h = +FLT_MAX (00D7A248)       ; 00C55FC0..00C55FD2
B+44h..+4Ch = -FLT_MAX (00D7A244)       ; 00C55FD7..00C55FEC
for (shape = B+70h; shape; shape = shape[+208h])        ; 00C55FF1, 00C56174
    merge shape[+0Ch..+14h] into both ends              ; 00C56000..00C560B7
    merge shape[+18h..+20h] into both ends              ; 00C560BA..00C56171
if (B+50h & 8) refresh the broad-phase proxy            ; 00C56182 on
```

Every corner is offered to both the minimum and the maximum, so the result does not depend
on the shape box being ordered. `00C31F90`, which `00939A89` calls, copies `B+38h..+40h`
into its first out-parameter and `B+44h..+4Ch` into its second (`00C31F90..00C31FBA`,
`RET 8`), so `extent = max - min` per axis, exactly as `docs/SHIP_HULL_BODY.md` read it.

**A body with no shapes never reaches `00C55FC0` at all.** All six callers -- `00C5C940` and
the five class bounds refreshers -- run it with a shape already on the list, so the
`FLT_MAX` seed is never what a body keeps. A hull whose model supplies no collision record
keeps the zeros `00C43CA0` writes at `00C43E5E`..`00C43E77`, which is the zero extent and
hence the zero inertia `docs/SHIP_HULL_BODY.md` already describes. Nothing in this packet
observed that case at run time.

## The collision group, mask and friction a hull shape gets

The friction is the physics material's `Friction`, record `+34h`, i.e.
`settings+514h + material*38h`, read at `00939365` as `FLD [EBX + EDX*8 + 514h]` with `EDX`
already `material*7`. `docs/SHIP_HULL_BODY.md` names the three records and the installed
`shipglobals.lua` values: `0.5` for `Ship` and `TBoat`, `1.0` for `Submarine`.

The mask starts at `0Dh` and ORs in one bit chosen by the vehicle class descriptor's virtual
slot `1Ch`, called at `009394F9` with the class from `unit+538h` in `ECX`. The jump table is
at `00939C90`, entered at `00939507` after `ADD EAX,-7 / CMP EAX,7 / JA`, so a category
outside `7..14` adds nothing (`EDI` stays at the zero set at `009394F7`).

| category | bit | address |
| --- | --- | --- |
| 7 | `40h` | `0093951C` |
| 8 | `2000h` | `00939570` |
| 9 | `20h` | `00939515` |
| 10 | `1000h` when `class+808h` is non-zero, else `800h` | `0093952A`..`0093954B` |
| 11 | `400h` | `00939523` |
| 12 | `200h` when `class+808h` is non-zero, else `100h` | `0093954D`..`0093956E` |
| 13 | `10h` | `0093950E` |
| 14 | `80h` | `00939577` |

The callee of slot `1Ch` was not read, so the category is a raw id here, not a named enum.
`class+808h` is a byte this packet only observed being read.

## The periscope shape

`009396BA..009399BF` adds one more shape, on a stack-local descriptor rather than an element
of the vector. It runs only when `class+510h <= 0` and `class+514h <= 0` (`009396F5`,
`00939706`; neither field is named by any doc) and the model has a node named `periszkop`
(`00D0C1F0`, looked up at `009396D8`, tested at `0093970F`).

It is the one hull shape with a real rotation: `00B6DB60` at `0093988D` returns the node's
local matrix and `00C336C0` at `0093989A` copies it into `desc+18h` as a 4x3, with the
destination `LEA ECX,[ESP+1D4h]` taken after the `PUSH EAX`, i.e. `desc+18h`. Its
translation is that matrix's origin **plus** the record's float3 (`009398E1..00939919`),
where a hull shape takes the float3 alone. `00939883` also stores the same `record+0Ch` at
`controller+18h`, and `00939927` stores the resulting `y` at `controller+390h`.

Its type is written explicitly as `4` at `00939724`, its friction is the `1.0f` at
`00D7A24C` (`0093987A`), its group `2` and its mask `5`.

## What the `fizika_%02d` and `hajobelso` walks actually do

They do not feed the body. `00938042..00938D8F` runs twenty times (`CMP EAX,14h` at
`00938D88`), formats `fizika_%02d` (`00CEB90C`) at `00938054` and collects matching nodes at
`0093812F` through `00B6F9A0`, which walks the subtree of `unit+4A4h` and keeps every node
whose name **contains** the pattern. It then splits that list three ways with `strstr`
(`00BF9440`):

| list | test | destination | treatment |
| --- | --- | --- | --- |
| plain `fizika_NN` | neither test matches | `controller+320h` via `00937B70` at `0093874E` | hidden, `00B6DA70` at `00938523` |
| `ep_fizika` (`00CEB900`) | `0093820A` | `controller+330h` via `00937C00` at `00938991` | left visible |
| `roncs_fizika` (`00D196DC`) | `0093831D` | `controller+340h` via `00937C00` at `00938CB5` | hidden and moved `+100000` per axis, `00938AC1..00938B0C` |

`docs/UNIT_PARTS.md` already records those three vectors and the per-part health vector at
`controller+310h`. None of the three is ever read by `009392FA..009396B8`; the descriptors
that reach `CreateBody` come only from `controller+34h`, which the hull walk fills.

`hajobelso` (`00CEB8F4`) is not a shape source either. `0071BA20` at `00938E13` collects the
nodes with that exact name out of the model's name index, and the loop at
`00938E30..00938F17` hides each one (`00B6DA70`) and moves it `+100000` on every axis
through node vtable slot `2Ch`, the same "push it out of the world" treatment the
`roncs_fizika` nodes get. The double `100000.0` is at `00CF81F0`.

## The rest of the packet's range

| range | what it does |
| --- | --- |
| `00937D3F..00937D8D` | `controller+74h..+7Ch` take the global float3 at `00F87574`; `+68h`/`+6Ch`/`+70h` copy them; `controller+84h` is zeroed |
| `00937D8E..00937DB8` | a **second** physics-material index, in `EBX`: `Mass < 100.0` gives 1, otherwise the unit's slot `5Ch` with id 8 gives 2, else 0 |
| `00937DBB..00937F7A` | a sum over the class's `24h`-byte records at `class+52Ch..+530h`, shaped by `settings+50Ch + i*38h` (1 linear, 2 square via `00CE3958`, 0.5 `sqrt` via `00BF7030`/`00CE3800`), divided by the double `10.0` at `00CE3DC0`, then `controller+84h = sum - settings[+510h + i*38h] * Mass` |
| `00937F5A..00937FE5` | the string `rope` (`00D09494`) is built and `00B6F9A0` fills `controller+4h` with every node whose name contains it |
| `00937FE9..00938038` | `004A8F10` resizes the per-part health vector `controller+30Ch` to 20 entries of `0.0f`, then zeroes them again |
| `009391C2..009392F9` | the body descriptor's default construction, already in `docs/SHIP_HULL_BODY.md` |

The second material index differs from the one `docs/SHIP_HULL_BODY.md` documented at
`00937CF1..00937D38` in exactly one case: a unit whose slot `5Ch` answers yes to id 8 **and**
whose `Mass` is below `100.0` gets index 2 for the friction and the inertia multiplier but
index 1 for this displacement sum. Both indices are read; which one is intended is not
established here.

`0093918D..009391C1` sizes the shape-descriptor vector to the hull-node count **plus** the
`controller+330h` group count (`SAR EAX,4` at `009391B3`), then `009392FA` fills only the
hull-node prefix and pushes only that prefix. The surplus records are default-stamped and
never attached; nothing this packet read consumes them.

## The concrete result for `VehicleClass[20]`, the DeRuyter

Read-only from the installed model tree,
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/models/ships/us/deruyter.mmod`
(46,865,813 bytes) and the install's own `model_dump/ships/us/deruyter/deruyter.gltf`.

A byte scan of the whole `.mmod` finds **zero** occurrences of `firstnode`, `front`, `back`,
`hajobelso` and `periszkop`. So for this class:

* owners 1, 3 and 4 resolve to null and contribute nothing;
* the hull's collision shapes are exactly the records owned by the node at `model+0Ch`;
* the `hajobelso` loop and the periscope block do nothing.

What the file does carry, by chunk name: **9 `ConvexObject` chunks**, one `GeomMesh` chunk
whose name/index table is `magazine`->3, `body`->0, `fizika`->0, `engineroom`->1,
`fueltank`->2, `fizika`->1, and 47 `Note` chunks including `fizika_0`, `fizika_1`,
`ep_fizika_0`, `ep_fizika_1`, `roncs_fizika_0` and `roncs_fizika_1`. The node names in the
`Hierarchy` chunk are of the form `De_Ruyter:bridge-ep_fizika_00`,
`De_Ruyter:bridge-roncs_fizika_00`, `De_Ruyter:smoke-ep_fizika_01` and
`De_Ruyter:smoke00-fizika_01` -- 9 `ep_fizika`, 2 `roncs_fizika`, the rest plain. The
two-digit suffix is why the substring search for `fizika_00` matches them while the one-digit
`Note` strings do not; the notes are a separate per-node tag list.

Across the whole `models/ships` tree, 196 of the ship models carry at least one of these
names; `periszkop` appears only on submarines (`I-54`, `I-56`, `I-58`, `i-400`, `kaiten`,
`minisub`, `type7`, `type_b`), which is consistent with the `class+510h`/`+514h` gate, and
`hajobelso` appears on many surface ships but not on the DeRuyter.

**What could not be read.** The numeric extent of the DeRuyter's `body` convex object, and
therefore the exact inertia its hull gets, is not reported here: it needs the `.mmod` chunk
layout and the model loader that writes `model+4Ch` and `model+0Ch`, both of which belong to
Codex's model packets. This packet did not decode the `ConvexObject` payload.

## Coverage

| routine | state | coverage |
| --- | --- | --- |
| `00C5C940` | reconstructed as a contract, dispatch read whole | complete for the dispatch, the list link and the two tail calls. The four constructors other than type 4 are identified by the vtable they install, and their bodies beyond that vtable store are unread |
| `00C57F50` | read whole | complete: every field it writes is attributed |
| `00C57C40` | reconstructed from the listing | complete for `00C57C40..00C57F37`; the tail hop into `00C55FC0` is modelled as a separate call |
| `00C55FC0` | reconstructed | partial: `00C55FC0..00C5617C` is projected; `00C56182..00C56441`, the broad-phase proxy refresh gated on `B+50h` bit 3, is read and described, not reconstructed |
| `00931A10` | reconstructed | partial: only the 48h-byte prototype at `00931A18..00931A8C`; the resize itself and `00931610` are ordinary vector growth |
| `00937C90` `00938F61..009399BF` | projected as a sequence over a host | complete for the walk, the fill loop and the periscope block. Not annotated in Ghidra: `00937C90` is leased to another worker, so the evidence is below under "Evidence for 00937C90 (apply later)" |
| `00937C90` `00937D3F..00938F60` | read | partial: the displacement sum, the `rope` collection, the health vector and the twenty-slot part loop are described and their destinations attributed, but only the shape-relevant conclusion is reconstructed. `00930780`, `00930A30`, `00931B70`, `00931C10`, `009376C0`, `00937750` are unread growth helpers |
| `0071AD50`, `0071BA20`, `00B6F9A0` | read for their contracts | none: model and scene-graph routines, owned by Codex. Read to state what they return, never renamed or re-documented |
| `00407ED0`, `00C50470`, `00C57B90`, `00C585B0`, `00C582E0`, `00C58840` | not read | none; contracts from the call site only |
| the model collision record and `model+0Ch` | not read | none: their producer is the model loader, Codex's packet |

## Evidence for 00937C90 (apply later)

`00937C90` is leased to `cc_exe_2r`, so nothing below was written to Ghidra or to the name
ledger. Its body is `00937C90..00939C8F`.

| address | evidence |
| --- | --- |
| `00937D3F` | `controller+74h/+78h/+7Ch` = the float3 at `00F87574`; `+68h`/`+6Ch`/`+70h` copy it; `+84h` = 0 |
| `00937D94` | the second material index: `Mass < 100.0` -> 1, else slot `5Ch`(8) -> 2, else 0 |
| `00937DBB` | the displacement sum over `class+52Ch`, `24h` per record, into `controller+84h` at `00937F74` |
| `00937FB6` | `00B6F9A0(unit+4A4h, controller+4h, "rope")`: `controller+8h/+Ch` is the vector of nodes whose name contains `rope` |
| `00937FF9` | `004A8F10(controller+30Ch, 20, 0.0f)`, the per-part health vector |
| `00938042` | the twenty-slot `fizika_%02d` loop begins; ends `00938D8F` |
| `00938204` / `0093831D` | `strstr` against `ep_fizika` and `roncs_fizika` |
| `0093861B`/`00938622` | a `model+4Ch` entry's `+4h` compared against a node from the name search: that is what makes `+4h` a node pointer |
| `00938DB9` / `00938DE2` | `controller+370h` = node `front`, `controller+374h` = node `back` |
| `00938E13` | `0071BA20(model, &vec, "hajobelso")`; the loop to `00938F17` hides and banishes each node |
| `00938F4E` | the `firstnode` lookup, kept in a stack slot |
| `00938F5C` | `controller+30h` vector cleared |
| `00938F61`..`0093918C` | the hull collision-record walk; `009390C9`/`009390F3` push `record+0Ch` into `controller+34h` and a parallel local vector keeps the record itself |
| `009391BD` | the shape-descriptor vector resized to hull nodes + `controller+330h` groups |
| `009392FA`..`009396B8` | the fill loop; `009396A1` pushes the descriptor's address into `desc+78h` |
| `009396D8`..`009399BF` | the periscope shape |

## Corrections

**To `docs/SHIP_HULL_BODY.md`, the descriptor table row for `+78h`/`+7Ch`/`+80h`.** It reads
"`vector<shape*>` | attached one by one | `00C5D8C0`". The three slots are a data pointer, a
**count** and a capacity, not a begin/end/capacity triple: `00C5D8C0`..`00C5D8C8` computes
the end as `LEA ECX,[EAX + EDX*4]` from `desc+78h` and `desc+7Ch`, and the growth at
`00939628` is `capacity = capacity*2 + 2` with `desc+7Ch` incremented by one per push
(`009396A3`).

**To `docs/SHIP_HULL_BODY.md`, the `ship_hull_shapes` follow-up row.** It describes the
packet as "the `fizika_%02d` and `hajobelso` node walks". Neither walk produces a hull
collision shape. The producers are the four named owners above; the `fizika_%02d` nodes
become the controller's part and debris groups and the `hajobelso` nodes are hidden and
moved out of the world.

**To `docs/UNIT_PARTS.md`, the `+8h`/`+Ch` row.** It reads "vector of node ptr | every hull
node | `00937C90`". The producer at `00937FB6` is `00B6F9A0(unit+4A4h, controller+4h, &s)`
with `s` the string `rope` (`00D09494`, assigned at `00937F7A`..`00937F9A`), and `00B6F9A0`
keeps a node only when `strstr(name, pattern)` succeeds. So `controller+8h`/`+Ch` is the
vector of nodes whose name contains `rope`, not every hull node.

**To `docs/DYN_COLLISION_PASS.md` and `docs/DYN_LCP_IMPULSE_MATH.md`.** Both say the
collision AABB producer `00C5C940` is unread. It is read here; the AABB itself is produced
by `00C57C40` per shape and `00C55FC0` per body.

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `model_collision_records` | `model+4Ch`, `model+0Ch`, `model+7Ch`, the `GeomMesh` and `ConvexObject` chunk parsers behind `007258F0` and `006FB000` | the producer of the record whose `+0Ch` is the convex mesh and whose `+14h` is the offset, and which node `model+0Ch` is. Codex owns the model routines; this is the one input the hull inertia still takes on trust |
| `dyn_shape_classes` | `00C57B90`, `00C585B0`, `00C582E0`, `00C58840`, `00407ED0` | the bodies of the sphere, box, cylinder and terrain constructors and the pool the convex mesh comes from; which game systems build each type |
| `unit_type_query_5c` | the unit vtable slot `5Ch` and the class vtable slot `1Ch` | what category id 8 means and what the `7..14` ids of the mask table are. Both are still raw ids |
| `ship_physics_material_record` | `settings+4E0h + i*38h`, `0083FEE7..008403B7` | the `+50Ch` exponent and the `+510h` coefficient the displacement sum at `00937DBB` uses, and why two different material indices are computed in the same function |
| `hull_body_no_shape_case` | `00C43E5E`, `00939A8E` | what a hull whose model supplies no collision record actually does in a mission. Statically its AABB stays at `00C43CA0`'s zeros and its inertia is zero on every axis |

## no_ghidra_function

none. Every address named, documented or reconstructed in this packet lies inside an existing
Ghidra function body, checked with `python tools/bsp.py ghidra proto <addr> --brief`.
