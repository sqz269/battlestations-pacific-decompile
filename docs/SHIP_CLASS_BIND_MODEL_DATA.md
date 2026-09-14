# What a ship class takes from its model: `0082FE30`

Addresses: 0082FE30 0082D700 0082D040 0095F500 00879AD0 00718870 00718000 00717F20 004FBA10
00484270 004215D0 004D1510 0074D190 00829180 0082E990 0082CA40 0082C560 0082F4E0 0082E1F0
0082F8E0 004F9B30 0085DC80 0071B2C0 007325A0 00759120 007D3E60 00D1ACC4 00D7A208 00D7A24C
00D7A280 00CFFD90 00D05A18 00D06560 00D099B8 00D099C0 00D099C8 00D099D0 00D099DC 00D099E8
00D099F4 00D09A00 00D09A0C

Packet `cc7_ship_class_bind_model_data`. **Ghidra was read-only throughout: no annotation, no
tagging, no write lock was taken, and no ledger record was written.** Every descriptive name in
this document is a hypothesis, not a recovered symbol; the two names that already existed
(`BSP_ShipClass_BindModelData_Provisional`, `BSP_ShipClass_BuildBuoyancyElements`) are repeated as
they stand in the ledger and are not endorsed here beyond what is proved below.

`docs/SHIP_BUOYANCY_ELEMENTS.md` established the first 64 bytes of this body (`0082FEA9`-`0082FEE8`)
and named the follow-up packet `ship_class_model_binding`; this document is that follow-up. It
cites that record rather than restating it.

## 0. The answer, in one paragraph

`0082FE30` is **not** a model loader. On entry the model is already there: the first instruction
after the prologue (`0082FE51`) calls `0095F500`, which dereferences `[class+50h]` at `0095F52A`
before this body has done anything at all. What `0082FE30` does is **query one already-open model
object by node name twelve times** and copy point data out of the matches into sixteen fields of
the ship class descriptor. The lookup key is a `(name, index)` pair: `00718870(model, &name, i)`
returns the last node whose counted name at `node+08h` equals `name` and whose `int` at `node+24h`
equals `i`, or null. Each matched node carries a `std::vector<Vec3>` at `node+44h`, and it is
those points — never mesh data, never triangles — that become class fields. Four of the twelve
names (`kemeny`, `farviz`, `explosion`, `path`) are swept as **indexed families starting at
index 1** until the lookup misses; the rest use a fixed index (`0` for nine of them, `1` and `2`
for the two `orrhullam` frames). The producer of `[class+50h]` is **not in this body and was not
established by this packet** — it is the same open question `docs/UNIT_PARTS.md` records for the
`30h` part descriptors.

## 1. The routine

| | |
| --- | --- |
| address | `0082FE30`, body `0082FE30`-`00831801` (9,938 bytes) |
| ABI | `void __fastcall(ship class descriptor /*ECX*/)`; `RET` at `00831801`, no stack args |
| frame | `PUSH -1 / PUSH 0C91FEFh / FS:[0] SEH frame`, `SUB ESP,10Ch`, `PUSH EBX/EBP/ESI/EDI`. Steady-state `ESP = entry - 128h` |
| EH state slot | `steadyESP+124h`; the value steps `0 → 1 → 2 → … → 0Ch`, one step per named-node region, and is reset to `-1` (`0FFFFFFFFh`) after each region's temporary string dies |
| held registers | `EDI` = the descriptor for the whole body; `EBX` = the current lookup index in the four swept families; `EBP` = scratch (also carries pre-computed EH-state and string-capacity constants) |
| dispatch | slot 8 (`vtable+20h`). Ship base vtable `00D1ACC4` (written by `009633C0`); slot 8 cell `00D1ACE4`. `00759120` (`MotherShip`) calls it at `0075913D` and then adds `runwaycenter` and `liftexitpoint` |
| instruction counts | Ghidra: 1,719 instructions, 228 basic blocks, 111 call sites, cyclomatic complexity 131 |

**Method.** The body is too large to decompile as evidence. The whole listing was scripted out and
swept by address window, with `ESP` anchored on the prologue and re-anchored at every `PUSH`/`SUB`
between an anchor and a frame reference; the twelve EH-state stores (`MOV [ESP+12Ch],N` at `ESP =
steady-8`, i.e. `steadyESP+124h`) were used as region boundaries, exactly as
`docs/WORKER_VERIFICATION_CHECKLIST.md`'s oversized-body rule prescribes. No decompiler output was
used for any claim. §7 states which windows were read and which were not.

## 2. The model side: three helpers and one record layout

These three are **not** part of the ship-class packet; they are read here only far enough to make
the call shape unambiguous, and their own owner is the model/resource packet.

| address | body | shape |
| --- | --- | --- |
| `00717F20` | `00717F20`-`00717FFC` | `bool __thiscall(model, const CharacterString* name, int index)` — counts matches, returns `count != 0`. `RET 8` |
| `00718000` | `00718000`-`007180D5` | `Node* __thiscall(model, const CharacterString* name, int index)` — returns the **last** match (the result slot is overwritten, not broken out of), null if none. `RET 8` |
| `00718870` | `00718870`-`0071889E` | `Node* __thiscall(model, name, index)` = `00717F20(...) ? 00718000(...) : 0`. `RET 8` — two full scans per lookup |
| `004FBA10` | `004FBA10`-`004FBA2D` | `int __thiscall(node)` = `([node+4Ch] - [node+48h]) / 0Ch`. Sole caller is `0082FE30` |
| `00484270` | ledger `STL_inst_00484270` | `Vec3* __thiscall(node, Vec3* out, int i)` — bounds-checked element accessor over the same vector |

**The model container** (`[class+50h]`), as used from here:

| offset | evidence | content |
| --- | --- | --- |
| `+10h` / `+14h` | `docs/SHIP_BUOYANCY_ELEMENTS.md` §3, `0082D067`..`0082D192` | part array / count (read by `0082D040`, result discarded) |
| `+54h` / `+58h` / `+5Ch` | `0082D707`-`0082D72B` | `std::vector<T*>`, checked-iterator layout (`_Myproxy` `+54h`, `_Myfirst` `+58h`, `_Mylast` `+5Ch`) |
| `+64h` / `+68h` / `+6Ch` | `00717F26`-`00717F2C`, `00718006`-`0071800C`, `0095F52A`-`0095F533` | `std::vector<Node*>` — the named node groups |

**The node record**, from the two scan loops and the point readers:

| offset | evidence | content |
| --- | --- | --- |
| `+00h`..`+07h` | never touched here | unknown |
| `+08h` | `00717F75`+`00717F97`/`00717F9D`/`00717FA2` (buffer at `+0Ch`, size `+1Ch`, capacity `+20h`) | counted name; same 1Ch-byte `CharacterString` layout `00408720` builds (`+00h` ptr, `+04h` inline buffer[16], `+14h` size, `+18h` capacity, inline while capacity `< 10h`) |
| `+24h` | `00717FC5`, `007180A5` | `int` index, compared for equality against the lookup's second argument |
| `+44h`..`+50h` | `0082FED9`/`0082FEDD` (`node+44h` handed to `0082D040` as a `std::vector<Vec3>*`), `004FBA10`, `00484270` | `std::vector<Vec3>` — the node's points. `_Myfirst` `+48h`, `_Mylast` `+4Ch`. Element stride `0Ch` is proved by the `2AAAAAABh` reciprocal-multiply divide that appears at every use |

`00CF*`/`00D0*` name constants and their lengths, in the order this body builds them:
`00D09A0C "deckline"` (8), `00D09A00 "bottomline"` (0Ah), `00D099F4 "debarkation"` (0Bh),
`00D099E8 "orrhullam"` (9, twice), `00D06560 "wave"` (4), `00D099DC "shipcenter"` (0Ah),
`00D099D0 "wave_stern"` (0Ah), `00CFFD90 "kemeny"` (6), `00D099C8 "farviz"` (6),
`00D05A18 "explosion"` (9), `00D099C0 "path"` (4), `00D099B8 "idle"` (4).
`orrhullam` and `kemeny` are Hungarian (`orrhullám` = bow wave; `kemény` = hard), consistent with
the `fizika`/`emberke` names elsewhere in this binary.

## 3. What the twelve regions produce

Physical layout is not program order: the compiler placed the `debarkation` fallback (§4) and the
`shipcenter` tail after the `wave_stern` block, and the back-edge at `00830DD2` rejoins the
`orrhullam` head. The order below is the **executed** order, keyed by the EH state.

| EH | window | lookup | what lands on the descriptor |
| --- | --- | --- | --- |
| `1` | `0082FEA9`-`0082FEE8` | `deckline` idx 0, `bottomline` idx 0, both through `00718000` (no `Has` pre-check) | not stored: both point vectors (`node+44h`) are passed straight to `0082D040` `BSP_ShipClass_BuildBuoyancyElements` together with the float at `class+71Ch`. Already recorded in `docs/SHIP_BUOYANCY_ELEMENTS.md` |
| `2` | `0082FF3B`-`008300B1` | `debarkation` idx 0 | `class+584h` `std::vector<Vec3>` resized to the node's point count (`004D1510`) and filled element by element. **Null node ⇒ the synthetic ring of §4**, not a skip |
| `3` | `008300B5`-`0083011E` | `orrhullam` idx **1** | node kept in `ESI` for EH state 5's frame build |
| `4` | `0083011F`-`00830283` | `orrhullam` idx **2** | node kept in `EBP`; both 4x4 matrices `class+5B0h` and `class+5F0h` are set to identity here (`00D7A24C` = `1.0f`) |
| `5` | `00830285`-`0083083B` | (uses the two nodes above) | two 4x4 frames: `class+5B0h` from `orrhullam 1`, `class+5F0h` from `orrhullam 2`. See §5 |
| `5` | `0083083C`-`00830925` | `wave` idx 0 | `class+594h` = `points[0]`, `class+5A0h` = `points[1]`. **No null check** — see §6 |
| `6` | `00830926`-`008309EC`, `00830DD7`-`00830DE2`, `00830DF5` | `shipcenter` idx 0 | `class+578h` = `points[0].x`, `+57Ch` = `.y`, `+580h` = `.z`. Null node ⇒ all three zeroed |
| `7` | `00830DEC`-`00830F27` | `wave_stern` idx 0 | `class+638h` = `points[0]`, `class+644h` = `points[1]`. Null node ⇒ both zeroed (`00830EFA`-`00830F27`) |
| `8` | `00830F32`-`00831051` | `kemeny` idx **1,2,3,…** | appends `{points[0].xyz, i}`, stride `10h`, to the raw array at `class+678h` (`ptr`, `count`, `cap` at `+0h/+4h/+8h`); grows through `00829180`. The stored `i` is the lookup index |
| `9` | `00831060`-`0083117D` | `farviz` idx **1,2,3,…** | appends `points[0]`, stride `0Ch`, to `class+650h`; grows through `0074D190` |
| `0Ah` | `00831182`-`0083129F` | `explosion` idx **1,2,3,…** | appends `points[0]`, stride `0Ch`, to `class+684h`; grows through `0074D190` |
| `0Bh` | `008312AB`-`00831425` | `path` idx **1,2,3,…**, requires `≥ 2` points | per hit: `0082E990(class+6CCh, i)` appends a `10h`-byte path record carrying the index, then every point of the node is pushed into that record's own vector through `004215D0`. If no `path` at all matched, `0082F4E0` is called once on `class+6DCh` |
| `0Ch` | `00831430`-`008316FD` | `idle` idx **1,2,3,…**, requires `≥ 2` points | per hit: a 4x4 frame built from `points[0..2]` (§5), orthonormalized, appended to `class+6ECh` by `0082CA40` |
| — | `00831702`-`008317E9` | — | reconciles `class+6ECh` (stride `40h`) against `class+6FCh` (stride `1Ch`, the Lua `Idle` records). See §5.3 |

Two blocks run **before** the first lookup and are the reason the model must already exist:

* `0082FE51` `0095F500` (`0095F500`-`0095FED5`, 630 instructions) — walks the whole
  `[class+50h]+64h` node vector and matches names against `00D04EC8 "camera"` (6 bytes). Not read
  beyond its first 70 instructions; **contract: unread** here.
* `0082FE58` `0082D700` (`0082D700`-`0082D7D1`, 76 instructions, fully read) — copies **every**
  pointer from the model's `+54h`/`+58h`/`+5Ch` vector into the descriptor's own
  `std::vector<T*>` at `class+6BCh` (`_Myfirst +6C0h`, `_Mylast +6C4h`, `_Myend +6C8h`), using the
  fast path when capacity allows and `0071B2C0` when it does not. This is a wholesale borrow of a
  second model-side list, distinct from the named node groups.

## 4. The `debarkation` fallback: a synthetic 12-point ring

When `00718870(model,"debarkation",0)` returns null, `0082FFAE` jumps to `008309F1`, which does
**not** leave the field empty:

```
008309FE  004D1510(class+584h, 12)                 ; resize to exactly 12 Vec3
00830A07  half.z = class+A0h * 0.5                 ; 00D7A280 is the double 0.5
00830A20  half.x = class+A4h * 0.5
00830A2C  half.y = class+A8h * 0.5
00830A54  p[0] = { -0.0 - half.x, -0.0 - half.y, -0.0 - half.z }   ; 00D7A208 is -0.0f
…         eleven further sign/axis variants, one per 4Ch-byte block
00830DD2  JMP 008300B3                             ; rejoin at the orrhullam head
```

`class+A0h` is `Length` per `docs/SHIP_AI_NAV_BLOCK_CTOR.md`; `+A4h` and `+A8h` are the other two
box dimensions, and which is beam and which is height is **not established here**. The twelve
per-point blocks (`00830A9C`-`00830DA7`) were read only in outline: the first point is read in
full, the remaining eleven were confirmed to be the same `JZ/JA/CALL 00BF6713` bounds-check idiom
followed by three `MOVSS` stores, and their individual sign patterns were **not** transcribed.
So: *the fallback exists, is 12 points, and is derived from the class box half-extents* — the
exact corner ordering is unread.

## 5. The frames

Three places build a 4x4 row-major matrix out of a node's first three points and then call
`0085DC80` `BSP_Matrix_OrthonormalizeBasisRows`. The row convention is the same in all three:
`+30h` is the translation row, `+10h` and `+20h` are basis rows, and `+00h` is left at whatever
the identity seed put there and is regenerated by the orthonormalize.

### 5.1 `idle` — read in full (`008314CD`-`008316F3`)

```
m = identity                                    ; 008314D8-0083155F, on the stack at steadyESP+88h
m[30h..38h] = points[0]                         ; 008315A8-008315BA
m[20h..28h] = points[2] - points[0]             ; 008315C3-00831608, via 00484270(0) and (2)
t          = points[1] - points[0]              ; 00831625-00831661
m[10h..18h] = 004F9B30(cross of the two edges)  ; 008316AB-008316D7
0085DC80(m)                                     ; 008316E0
0082CA40(class+6ECh, &m)                        ; 008316F3  append
```

The two operands handed to `004F9B30` are `[ESP+60h]` and `[ESP+24h]`, both edge vectors; **which
is the first cross operand is not claimed** — the sign of the resulting axis therefore is not
established here.

### 5.2 `orrhullam` — read in outline (`00830285`-`0083083B`)

Two structurally identical halves. Half A writes `class+5E0h` (translation, `= points[0]`),
`class+5C0h` and `class+5D0h` (`= 004F9B30(...)` at `00830524`), then `0085DC80(class+5B0h)` at
`00830551`. Half B writes `class+620h` (`= points[0]`), `class+600h`, `class+610h`
(`= 004F9B30(...)` at `008307FD`), then `0085DC80(class+5F0h)` at `0083082E`. One arithmetic window
was read in detail (`008302FE`-`0083035D`): it loads `points[1]` and `points[2]` under a
`count > 2` bounds check and forms `points[2] - points[1]`. **This differs from the `idle`
construction, which differences against `points[0]`.** The remaining arithmetic windows
(`00830360`-`00830524` and `008305A8`-`008307FD`) were **not read**, so which difference feeds
which row in the `orrhullam` case is *open*, and the "same as idle" reading must not be assumed.

### 5.3 `idle` matrices against Lua `Idle` records (`00831702`-`008317E9`, read in full)

```
matCount = (class+6F0h..+6F4h) / 40h            ; the frames just built
recCount = (class+700h..+708h) / 1Ch            ; the vector at class+6FCh
if (recCount < matCount) {
    0082E1F0(stack 1Ch prototype)               ; BSP_ShipIdleRecord_Construct
    0082F8E0(class+6FCh, matCount)              ; BSP_ShipClass_ResizeIdleRecords
} else if (recCount > matCount) {
    copy the last built 40h matrix off the stack and
    0082C560(class+6ECh, recCount)              ; grow the frame vector with it
}
```

So the model's `idle<i>` node family and the class's Lua-authored `Idle` records are **forced to
the same length**, in whichever direction is short. `0082E1F0` and `0082F8E0` are already in the
ledger from `BSP_ShipClass_ReadLuaFields` `00831840`; this is their second caller.

## 6. `wave` is dereferenced without a null check

At `0083086A` the result of `00718870(model,"wave",0)` goes into `ESI`; the only code between that
and `0083089D MOV EAX,[ESI+48h]` is the temporary string's destructor and header reset
(`0083086F`-`00830898`). There is no `TEST ESI,ESI`. Every other optional group in this body has
one (`0082FFAE`, `00830F8D`/`00830FA4`, `008310BB`/`008310CE`, `0083130B`/`0083131A`,
`0083148B`/`0083149E`, `00830E49`/`00830E58`, `008309A2`). A ship model with no `wave` node at
index 0 therefore faults in this routine. Two readings are possible and **this packet does not
decide between them**: either every shipped ship model is guaranteed to carry `wave`, or this is a
latent bug the shipped data never triggers.

## 7. Coverage — what was read and what was not

Read instruction by instruction:

* `0082FE30`-`0082FF3A` (prologue, `deckline`/`bottomline`)
* `0082FF3B`-`008300B4` (`debarkation` copy loop)
* `008300B5`-`00830285` (both `orrhullam` lookups, both identity seeds)
* `008302FE`-`0083035D` (one `orrhullam` arithmetic window)
* `0083083C`-`00830925` (`wave`)
* `00830993`-`00830A35` (`shipcenter`, and the head of the fallback ring)
* `00830A36`-`00830A9C` (the fallback ring's first point)
* `00830DD7`-`00830DFC` (`shipcenter` null tail)
* `00830E23`-`00830F34` (`wave_stern`, both paths)
* `00830F32`-`00831051` (`kemeny`)
* `00831060`-`0083117D` (`farviz`)
* `00831182`-`008312B1` (`explosion`)
* `008312E5`-`00831431` (`path`)
* `00831465`-`00831801` (`idle` and the whole tail)
* callees read in full: `00717F20`, `00718000`, `00718870`, `004FBA10`, `0082D700`;
  read in part: `00484270` (first 16 of 31 instructions), `0095F500` (first 70 of 630)

Read in outline only — calls, branch targets and `EDI`-relative stores enumerated by filtering the
full listing, but the arithmetic between them not transcribed:

* `00830360`-`00830524` and `008305A8`-`008307FD` — the `orrhullam` edge/cross arithmetic
* `00830A9C`-`00830DA7` — eleven of the twelve fallback-ring points

Not read at all:

* `0095F500` past `0095F5E2` (the `camera` node pass, 560 instructions)
* `00879AD0` (595 instructions), called first by `0095F500` and by `007325A0`; strings
  `explosion`, `fakeexplosion`, `emberke`
* the bodies of `0082E990`, `0082CA40`, `0082C560`, `0082F4E0`, `00829180`, `0074D190`,
  `004D1510`, `004215D0`, `0071B2C0`, `004F9B30`, `0085DC80` — their roles above are inferred from
  argument shape and from existing ledger records, not from their listings

## 8. The four questions

**1. What model does a ship class open, and how does it reach it?** It opens none.
`0082FE30` reads `[class+50h]` 13 times (`0082FEA9`, `0082FEBF`, `0082FF5C`, `008300DA`,
`0083014D`, `00830856`, `00830959`, `00830E0F`, `00830F57`, `00831081`, `008311A3`, `008312D1`,
`00831451`) and **never writes it**; `0095F500`, called before any of them, already dereferences
it. The same `+50h` field is read by `007325A0` (gun class, `007325E6`/`00732640`) and by the
plane-class equivalent `007D3E60`, so it sits on a base shared by the ship, plane and gun class
descriptors.

**Its producer was not found by this packet**, and the search is reported rather than filled in.
Ruled out by direct inspection: `009633C0` `BSP_ShipClass_ConstructBase` (sets the vtable and zeroes
~80 fields, `+50h` not among them), `00960230` (the class base reader), `00964020` (the class
factory — no `+50h` store, no indirect call through a class vtable). A byte scan for
`MOV [reg+50h], reg` over `.text` (`89 4x 50`, `89 5x 50`, `89 7x 50`) produced no candidate inside
the vehicle-class code. A byte scan for the slot-8 dispatch itself (`FF 5x 20`, and the
`MOV reg,[reg+20h]; CALL reg` forms) found **no call site that reaches a ship-class vtable**, so
the routine that invokes this virtual is also unidentified. This matches the open item already
recorded in `docs/UNIT_PARTS.md` ("filled somewhere else, most likely on the model side that also
owns `class+50h`. Finding its producer is the first follow-up") — that follow-up is still open,
and this packet adds only the negative result above plus the two consumption contracts in §2.

**2. What does it keep?** Sixteen descriptor fields, all derived from node **points**, never from
mesh geometry:

| field | stride / kind | source |
| --- | --- | --- |
| `+578h`/`+57Ch`/`+580h` | 3 floats | `shipcenter[0]` `points[0]` |
| `+584h`..`+590h` | `std::vector<Vec3>` | `debarkation[0]` points, or the §4 synthetic ring |
| `+594h`, `+5A0h` | `Vec3`, `Vec3` | `wave[0]` `points[0]`, `points[1]` |
| `+5B0h` | 4x4 | `orrhullam[1]` frame |
| `+5F0h` | 4x4 | `orrhullam[2]` frame |
| `+638h`, `+644h` | `Vec3`, `Vec3` | `wave_stern[0]` `points[0]`, `points[1]` |
| `+650h`/`+654h`/`+658h` | raw array, stride `0Ch` | `farviz[1..]` `points[0]` |
| `+678h`/`+67Ch`/`+680h` | raw array, stride `10h` = `{Vec3, int index}` | `kemeny[1..]` `points[0]` + its index |
| `+684h`/`+688h`/`+68Ch` | raw array, stride `0Ch` | `explosion[1..]` `points[0]` |
| `+6BCh`..`+6C8h` | `std::vector<T*>` | wholesale copy of the model's `+54h` pointer vector (`0082D700`) |
| `+6CCh`..`+6D8h` | records, stride `10h` | `path[1..]`, one record per index, each holding that node's whole point list |
| `+6DCh`..`+6E8h` | vector | touched by `0082F4E0` only when **no** `path` matched |
| `+6ECh`..`+6F8h` | vector, stride `40h` | `idle[1..]` frames |
| `+6FCh`..`+708h` | vector, stride `1Ch` | Lua `Idle` records, length-matched to `+6ECh` |
| `+52Ch` (indirect) | buoyancy elements | via `0082D040` from `deckline`+`bottomline` — see `docs/SHIP_BUOYANCY_ELEMENTS.md` |

**3. Does it reach the `Resource` container that `GeomMeshStructuredResourceParser` would parse?**
No, not on any path read here. `0082FE30` touches exactly two model-side containers — the node
vector at `model+64h` and the pointer vector at `model+54h` — and neither carries the
`{kind at +4h, index at +8h}` elements that `00727310` produces per
`docs/GEOM_MESH_RESOURCE.md`. The element list that the narrowphase needs lives on the *same*
model object (`docs/SHIP_BUOYANCY_ELEMENTS.md` puts the part array at `model+10h`/`+14h`, read by
`0082D040` and discarded), but this routine never reads it and never invokes a parser. So
`0082FE30` is downstream of whatever runs `GeomMeshStructuredResourceParser`, not a route to it.

**4. Is the `"fire"` node group bound here?** No. `0082FE30` contains no `fire` string and no
lookup for one. `007325A0` `BSP_GunClass_LoadFireNodeMuzzleOffsets` does it, on its own `this`
(`[gunclass+50h]` at `007325E6` and `00732640`), with the string `00CE6798 "fire"` length 4 and
indices **0 and 1** — note the different index base from the four families here. The two routines
share only the model-side helper `00718870` and the pre-pass `00879AD0`. Binding barrels is the
gun class's job, not the ship class's.

## 9. The wiring contract

What a host must hold before `0082FE30`'s behaviour can be reproduced, stated as the minimum
surface this body actually touches. **Nothing in this section is implemented, built or tested by
this packet — it is a specification derived from the listing.**

**A. The model object** (whatever ends up at `class+50h`) must expose three things:

1. `std::vector<Node*> nodes` — the named node groups.
2. `std::vector<T*> borrowed` — copied wholesale into `class+6BCh` by `0082D700`. What `T` is was
   not established.
3. the part array the narrowphase wants (`+10h`/`+14h` per `docs/SHIP_BUOYANCY_ELEMENTS.md`).
   `0082FE30` does not fill it; the parser side must.

**B. `Node`** must expose `name` (counted), `index` (`int`), and `points` (`std::vector<Vec3>`),
and the container must offer `find_last(name, index)` with the "last match wins, null if none"
semantics of `00718000` — not "first match", which would change which duplicate wins.

**C. The naming convention** the host must honour when it decodes a model, because the class side
depends on it:

* fixed index `0`: `deckline`, `bottomline`, `debarkation`, `wave`, `shipcenter`, `wave_stern`
* fixed indices `1` and `2`: `orrhullam`
* swept from `1` upward until a miss: `kemeny`, `farviz`, `explosion`, `path`, `idle`
* swept from `0` upward, by the **gun** class, not here: `fire`

**D. The call order.** `0082FE30` is slot 8 and must run *after* whatever sets `class+50h` and
after `BSP_ShipClass_ReadLuaFields` (slot 2) has filled `class+6FCh`, because the tail at
`00831702` length-matches against it. The routine that performs that sequencing is **not
identified** (§8.1), so a host cannot yet reproduce the real call site — only the body.

**E. What this does *not* unblock.** The `part=0 fires=0 floods=0` measurement in
`docs/PART_DAMAGE_WIRING_PLAN.md` is about the element list with `kind` and `index`, which
`00727310` produces and which reaches a unit through `class+50h`'s part array, not through any
field in §8.2. Wiring `0082FE30` alone would populate wave, bow-wave, debarkation, flight-idle and
effect-anchor data — visual and AI anchors — and would still leave `part` at `-1`. The blocking
edge remains the producer of `class+50h`.

## 10. Follow-ups

| id | scope | what is open |
| --- | --- | --- |
| `model_object_producer` | the writer of `class+50h`; the invoker of ship vtable slot 8 | neither found. `009633C0`, `00960230`, `00964020` ruled out by inspection; no `MOV [reg+50h]` and no `CALL [reg+20h]` candidate in the vehicle-class code. The dispatch is presumably a computed or thunked indirect call the byte scans used here do not match |
| `orrhullam_frame_axes` | `00830360`-`00830524`, `008305A8`-`008307FD` | which point difference feeds `+10h` and which feeds `+20h`, and whether the construction really differs from `idle` as `008302FE`-`0083035D` suggests |
| `debarkation_ring_corners` | `00830A9C`-`00830DA7` | the sign pattern of the eleven unread synthetic points, and which of `+A4h`/`+A8h` is beam and which is height |
| `class_camera_pass` | `0095F500` past `0095F5E2`, and `00879AD0` | the `camera` node pass and the `explosion`/`fakeexplosion`/`emberke` pass, both of which run before this body's first lookup and both of which also read `class+50h` |
| `path_record_shape` | `0082E990`, `004215D0`, `0082F4E0` | the `10h`-byte `path` record's own layout, and what `0082F4E0` does to `class+6DCh` on the no-path branch |
