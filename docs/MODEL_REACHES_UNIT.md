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

# The shape records come in through a virtual call (packet `cc7_part_shape_record_producer`)

Addresses: `007135C0` `BSP_UnitPartInstance_Construct`, `0087BCC0`
`BSP_UnitInstance_InitHealthAndParts`, `00712E74`-`00712EE8`, `00712C80`. Constant `00CED9E0`.

Ghidra **read-only**. **Exported / read only** — no C++, no tests. Mod-artefact caveat carried.

## The gate

`docs/COLLISION_SHAPES.md` records the shape records as living in the vector at
`[node+160h] + 3Ch`, producer `contract: unread`. `node+160h` is **not built by the node**: it is a
**constructor argument**, and the argument is the result of a virtual call.

```
0087BDF5  EAX = unit[+354h]
0087BDFB  EDI = EAX[+50h]                       ; the record source
0087BE3C  EAX = unit->vtable[190h]              ; called with a float from 00CED9E0
0087BE4B  CALL EAX
0087BE4D  EDX = [[EDI]+8h]                      ; EBX was [EDI], then ADD EBX,8
0087BE4F  PUSH EAX                              ; the vtable[190h] result   -> arg3
0087BE52  CALL EDX                              ; EDI->vtable[8h]()
0087BE54  PUSH EAX                              ; its result               -> arg2
0087BE55  PUSH ESI                              ; the unit                 -> arg1
0087BE58  CALL 007135C0
```

and in the constructor, with the frame walked rather than guessed — `PUSH -1` / handler /
`FS:[0]` (12), `SUB ESP,0Ch` (12), `PUSH EBX/EBP/ESI` (12), `PUSH EDI` (4) puts entry `ESP` 40
bytes above:

```
007135ED  EDI = [ESP+2Ch]   = [S+4]  = arg1     ; the unit
007135F1  node+4Ch  = arg1
007135F4  EAX = [ESP+30h]   = [S+8]  = arg2
00713604  node+160h = arg2                      ; <- the record source
0071360A  node+164h = arg1
```

**So `node+160h` is `unit[+354h][+50h]->vtable[8h]()`.** That call is the producer the collision
doc marks unread, and it is the part-damage gate.

**Not identified**: what `unit+354h` is, what its `+50h` holds, and what `vtable[8h]` on that object
does. Those are the next hop and this packet did not take them.

## The second vector is never filled

`node+16Ch`/`+170h`/`+174h` — the `10h`-byte-element vector — is zeroed by the constructor at
`00713610`-`0071361C`. Across the whole part-collision region `00700000`-`00720000` the only other
write is at `00712EB1`, and that is inside a **teardown loop**:

```
00712E74  EBP = [ESI+170h]                      ; end
00712E7E  ADD EDI,4 ; EAX = [EDI]
00712E87  free(EAX)                             ; 00BF65AC
00712E90  [EDI] = [EDI+4] = [EDI+8] = 0
00712E98  ADD EDI,10h                           ; 10h-byte stride, as the doc records
```

so it frees each element's `+4h` pointer and zeroes the triple. **Nothing in that region ever
populates it.** (Image-wide, `[reg+16Ch]` has 265 operands with many writes, so the scan is not
vacuous — but those are other objects at a common offset and were not attributed.)

## Two method corrections, both about function attribution

The lead's caveat — `scan-bytes` and `lookup` report the nearest **preceding** defined function,
not a container — cost me two mis-attributions in the previous packet. It has a twin that cost one
here:

**Ghidra's recorded body range can also under-report.** `FUN_00712C80`'s body is
`00712C80`-`00712D7C`, but the bytes at `00712D7C` continue straight into more code with **no `CC`
padding**, and the teardown loop at `00712E74`-`00712EE8` is in that continuation. So containment
cannot be decided from the recorded extent either: `00712EE8` looked like it was outside a function
when it is really past a short body range.

The working rule from both: **to decide containment, check the recorded body range *and* look for
the `INT3` padding that actually bounds the code** — neither alone is sound.

## Still open

* `unit+354h`, its `+50h`, and that object's `vtable[8h]` — the gate itself, one hop away.
* The record decode inside `BSP_UnitPartCollisionNode_BuildShapes`, still
  `contract: unread` in `docs/COLLISION_SHAPES.md`.
* Whether `00727A90`'s return is the type `shape+24h` expects. **Not taken this packet** — still
  consistent by size only, and still worth proving before anything is wired.

# A null `class+50h` is handled, not assumed (packet `cc7_class50_final`)

Addresses: `0087BCC0` `BSP_UnitInstance_InitHealthAndParts` (232 instructions; the relevant window
`0087BDF5`-`0087BF73` read from the listing), `0087BE02`, `0087BF5B`, `00711BE0`.

Ghidra read-only. **Exported / read only** — no C++, no tests. Mod-artefact caveat carried.

## The guard

The live reader of `class+50h` tests it for null **before** using it, and skips everything:

```
0087BDF5  EAX = unit[+354h]              ; the class descriptor, kUnitOffDescriptor
0087BDFB  EDI = EAX[+50h]                ; the model handle
0087BDFE  XOR EBX,EBX
0087BE00  CMP EDI,EBX
0087BE02  JZ  0087BF5B                   ; <-- null -> straight to the tail
...
0087BE15  PUSH 1ACh                      ; a part instance is 1ACh bytes; the push is shared
0087BE1C  operator new                   ;    by both arms of the 0087BE1A branch
0087BE3A  EBX = [EDI]                    ; the model's vtable - never reached when EDI is null
0087BE52  CALL [EBX]                     ; EDI->vtable[8h]()  -> node+160h
0087BE58  CALL 007135C0 BSP_UnitPartInstance_Construct
...
0087BF3C  00711BE0(unit[+360h], EDI)     ; the normal path publishes the collision-node list
0087BF5B  unit[+360h] = 0                ; the null path clears it and returns
```

So with a null `class+50h` the function **sets `unit+360h` to zero and returns**. No part instance
is allocated, `BSP_UnitPartInstance_Construct` never runs, `node+160h` is never set, `BuildShapes`
has no records to copy, no `UnitPartCollisionShape` is built, and the narrowphase has no element to
name.

## What that proves

`part=0 fires=0 floods=0` is **the native behaviour when `class+50h` is null**, not a
reconstruction defect. The executable does not assume the model is there; it tests and degrades.

Stated exactly, because the conditional matters:

* **Proved:** if `class+50h` is null, the native code builds no parts and clears `unit+360h`.
* **Established earlier:** nothing in this image writes `class+50h` in any of the scanned forms
  beyond the null at `0087C6A3`; `0082FE30`, which reads it thirteen times, is dead code; and
  `0070F6B0`, the constructor that would take a decoded mesh into `shape+24h`, is referenced
  nowhere.
* **Therefore:** in the reconstructed host, where the field is null, a zero `part` count is
  **faithful** — it is what this executable does with a null model handle.
* **Not proved:** that the field is null in the *shipped game* at runtime. It cannot be, or ships
  would have no parts. The writer remains unfound, and that now blocks only making `part`
  non-zero — it no longer blocks the faithfulness claim.

## The block-copy angle, narrowed but not closed

The last byte-reachable form named in `docs/MODEL_HANDLE_PRODUCER.md`. Control first, per the rule
that document opens with: **628 `REP MOVSD` and 43 `REP MOVSB` image-wide**, so the scan is not
vacuous.

| window | `REP MOVSD` | `REP MOVSB` |
| --- | --- | --- |
| `0087C000`-`0087D000`, around `BSP_DamageableClass_ConstructBase` | **0** | **0** |
| `00949000`-`00970000`, the vehicle-class region | 13 | 0 |
| `00700000`-`00760000`, the model and part-collision region | 26 | 0 |
| `004F0000`-`004F2000`, the scene unit creators | 1 | 0 |

The class base constructor's own region performs **no block copy at all**. The 13 sites in the wider
vehicle-class region have **not** had their destinations traced, so the `memcpy` hypothesis is
**narrowed, not eliminated** — each would need its `EDI` followed. SIB-indexed writes and
write-then-alias remain unexcluded as before.

## Where this leaves part damage

`docs/PART_DAMAGE_WIRING_PLAN.md` can record the zero as **proved faithful** rather than inferred.
Its remaining question is no longer "why is `part` zero" — that is answered — but "what populates
`class+50h` in the shipped game", which is a smaller and better-posed question, and one that byte
scanning has now largely exhausted.

# None of the block copies can reach `class+50h` (packet `cc7_block_copy_destinations`)

Ghidra read-only. **Exported / read only** — no C++, no tests. Mod-artefact caveat carried.

## Result: the byte-reachable space is exhausted

**None of the thirteen `REP MOVSD` sites in `00949000`-`00970000` writes `class+50h`**, and the
reason is arithmetic rather than attribution: every one of them copies **0x10 or 0x11 dwords — 64 or
68 bytes** — and the ones that target an object take it at its **base**, so they reach `+00h`..`+43h`
and stop **twelve bytes short of `+50h`**.

| destination | sites | count |
| --- | --- | --- |
| a stack buffer — `LEA EDI,[ESP+…]`, `LEA EDI,[EBP-…]`, or `MOV EDI,ESP` | `00949E94`, `0094A171`, `0094A6FC`, `0094C213`, `009559D4`, `0095B15B`, `0095D13D`, `0095D3DD`, `0095D470` — **nine** | 0x10 / 0x11 |
| an object pointer at offset 0 — `MOV EDI,EAX`, `MOV EDI,ECX`, `MOV EDI,[ESP+8]` | `00952ECF`, `009552CD`, `00955163`, `00955FBD` — **four** | 0x11 |

The nine stack destinations cannot be class descriptors at all. The four object copies are
**by-value copies of a 68-byte struct**: three of the stack ones confirm the size independently by
pairing `SUB ESP,44h` with `MOV EDI,ESP` and `MOV ECX,11h`, so `0x44` bytes is the type's size, not
a coincidence of the loop.

The four were each resolved from the listing rather than assumed:

```
00952ECF  PUSH EDI ; MOV EDI,ECX ; TEST EDI,EDI ; JZ ; MOV ECX,11h ; MOV ESI,EDX
009552CD  TEST EAX,EAX ; JZ ; MOV ECX,11h ; MOV ESI,EBX ; MOV EDI,EAX
00955163  (function entry after CC padding) PUSH EDI ; MOV EDI,[ESP+8] ; TEST EDI,EDI ; JZ ;
          PUSH ESI ; MOV ESI,[ESP+10h] ; MOV ECX,11h
00955FBD  same shape as 009552CD
```

## The model region's twenty-six, bulk-characterised

Taken as the follow-up. Counts across all 26 in `00700000`-`00760000`: **9, 15, 16, 18 and 19
dwords — at most 76 bytes**, so the largest reaches `+4Bh` and still falls short of `+50h`. By
destination shape: nine stack, twelve object, five unclassified.

That characterisation is **bulk** — the destinations were bucketed by a byte-pattern heuristic, not
traced per site as the thirteen were. The solid part is the count bound, which is exact and applies
to all 26.

## The line is closed to byte search

Across both regions, **39 block copies, none exceeding 76 bytes, none starting below a destination's
base**. Together with the earlier scans — `MOV` reg and immediate in both encodings, `LEA`, `MOVSS`,
all negative in the class region, all with non-zero image-wide controls — the byte-reachable space
for a writer of `class+50h` is **exhausted**.

What remains cannot be reached by scanning:

* a **SIB-indexed** write, `[reg+reg*n+50h]`, which every scan here skips by design;
* a write on an object **later aliased** to the class;
* a writer in a **module outside `battlestationspacific.exe`** — this installation carries mod
  artefacts and that was never excluded.

All three need the call graph or a runtime observation. **Byte scanning has nothing left to say
about this question**, and the part-damage line should be recorded as closed to it: the zero is
proved faithful, the feature gap is bounded, and the missing writer is a call-graph question now.


## Correction from NATIVE_DAMAGEABLE_CLASS_MODEL_BE.md

The GeomMesh `shape+24h` relationship above remains valid, but the claim that class+50h was a red herring is incorrect. They are different owners. `00879590` publishes the `007188A0` game-resource result at class+50h (`00879768`), and `00879AA0` subsequently invokes class slot+20h (`00879ABA`). The concrete vehicle factory reaches this path through slot+10h and `009598D0`; the class-owned resource later supplies the established per-instance graph and item route. Positive disk/live byte and controlled original/source caller evidence is recorded in [NATIVE_DAMAGEABLE_CLASS_MODEL_BE.md](NATIVE_DAMAGEABLE_CLASS_MODEL_BE.md). This establishes the native producer and source caller composition, not current executable/gameplay admission.
