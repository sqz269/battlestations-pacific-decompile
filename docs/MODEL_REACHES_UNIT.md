# The model reaches the unit, not the class

Addresses: `00717E80`, `00716FE0`, `00727A90`, `007268D0`, `00727310`, `00724510`, `0070F720`,
`0070F710`, `00723E90`, `00723D60`, `0070F6F7`-`0070F70C`, `00710630`, `007106A5`, `00710A53`,
`0071106B`. Vtables `00CFD800` (parser), `00CFD768` (collision shape). Globals `00E19BD0`,
`00F87574`.

Packet `cc7_model_reaches_unit`. Ghidra **read-only**. **Exported / read only** — nothing new is
reconstructible yet, so no C++ and no tests. Scans cover `battlestationspacific.exe` only; this
installation carries mod artefacts, so a loaded module is not excluded.

## Result

**`class+50h` was a red herring.** The decoded mesh never goes near the vehicle class descriptor: it
is a **constructor argument to a per-unit collision shape**, and it lands at `shape+24h`.

`docs/MODEL_HANDLE_PRODUCER.md` accumulated five negatives on the class hypothesis. This is why
they were all true.

## The parser is registered, and it returns the mesh rather than storing it

`00717E80 BSP_GameResourceParsers_Register` registers five parser singletons, the fourth being
`GeomMesh` (`docs/GAME_RESOURCE_PARSER_REGISTRATION.md` row 4): getter `00716FE0`, global
`00E19BD0`, vtable `00CFD800`, parse entry at vtable `+8h`. `00716FE0` has exactly one caller,
`00717EC4` inside that registration wrapper, so the singleton exists and nothing else reaches for it
directly.

`00727A90 BSP_GeomMeshResourceParser_Parse` is 0x5F bytes and was read in full:

```
00727AA7  operator new(50h)
00727AC1  007268D0(obj)                              ; construct the 50h-byte mesh
00727AD7  00727310 BSP_GeomMeshResourceParser_ParsePayload(mesh, payload)
00727AE0  return mesh                                ; RET 4
```

So the decoded element list lands in a **0x50-byte object handed back to the resource manager**. It
is not written onto any class, which is consistent with every negative in the other document.

## The consumer chain, end to end

```
BSP_UnitPartCollisionShape_TraceSegment  00724510
    EAX = shape+1Ch ; ECX = shape+24h                  ; 00724514 / 00724517
    args: shape+1Ch +50h and +90h (two frames)         ; 0072452C / 00724525
  -> 00723E90 BSP_GeomMesh_TraceSegmentIntoHitRecord   ; 00724530
  -> 00723D60 BSP_GeomMesh_TraceSegmentClosestElement  ; 00723F12, walks mesh+0Ch
  -> hit+30h / hit+34h                                 ; 00723F62 / 00723F6C
```

`00724510` is slot 0 of a **three-slot vtable at `00CFD768`**, `{00724510 TraceSegment,
0070F720 TestSphere, 0070F710}`. That table sits in `.rdata` between the string literals `"shape"`,
`"leader"` and `"num_"`, which is why it reads as a name table at first glance — `00CFD760` is the
string `"leader"`, not a vtable base. The ledger independently names two of the three slots
`BSP_UnitPartCollisionShape_*`, which corroborates the class.

## The link: the mesh is a constructor argument

Five sites install `00CFD768`; four of them are object constructions (the fifth, `00712689`, writes
it into a stack slot). Each is followed within ~0x25 bytes by a write to `+24h`:

| vtable install | `+24h` write | kind |
| --- | --- | --- |
| `0070F6F7` | `0070F706` `MOV [EAX+24h], ECX` | **primary constructor** |
| `00710680` | `007106A5` `MOV [EAX+24h], ECX` | copy constructor `FUN_00710630` |
| `00710A2E` | `00710A53` `MOV [ECX+24h], EDX` | copy |
| `00711046` | `0071106B` `MOV [EAX+24h], ECX` | copy |

The primary constructor's tail, read from the listing:

```
0070F6D7  +14h, +18h <- 00F87578 / 00F8757C      ; the read-only zero vector
0070F6F7  MOV [EAX],      0CFD768h               ; the shape vtable
0070F700  MOV [EAX+20h],  EDX                    ; second argument
0070F706  MOV [EAX+24h],  ECX                    ; THE MESH, straight from ECX
0070F70C  RET 0Ch
```

`FUN_00710630` is the copy constructor: it installs base vtable `00CE89DC`, then `00CFD768`, and
copies `+4h`..`+24h` field by field from the source shape — so `+24h` propagates by copy, and the
only origin is the primary constructor's `ECX`.

**So the mesh is passed in, not looked up.** The producer of `shape+24h` is whoever calls that
constructor.

## What is not established

* **Who calls the primary constructor with a mesh.** `0070F6D0`-`0070F70C` ends in `RET 0Ch`, but
  Ghidra has no function start for it — the enclosing candidate is `FUN_0070F4D0`, 0x200 bytes
  earlier — so the entry point has to be recovered before its callers can be listed. That is the
  single next step and it is small.
* **That the object `00727A90` returns is the same type `shape+24h` expects.** Both are "GeomMesh"
  by naming; the 0x50-byte allocation covers the `mesh+0Ch` walk and the `mesh+28h` ordinal list
  that `docs/PART_DAMAGE_WIRING_PLAN.md` records, so the sizes are consistent — but identity was
  **not proved** and should be before anything is wired on it.
* **Who asks the resource manager for a `GeomMesh` in the first place**, i.e. the `.MMOD` request
  site. The measurement in `docs/PART_DAMAGE_WIRING_PLAN.md` (`grep -ic mmod` = 0 over a full
  mission) is about the reconstructed host, not this image.

## Why this matters for `part`

The narrowphase already has the shape of the answer: `src/narrowphase_unit_part_shape.cpp` runs on
three hard-coded shapes writing kind `0Ah` and index `-1`. The native equivalent is a
`UnitPartCollisionShape` holding a decoded `GeomMesh` at `+24h`, and the element the ray hits
supplies `hit+30h`/`hit+34h`. So the wiring contract is now a **constructor call with a mesh**,
not a field write on a class descriptor — a different shape of fix from the one
`docs/PART_DAMAGE_WIRING_PLAN.md` assumed.
