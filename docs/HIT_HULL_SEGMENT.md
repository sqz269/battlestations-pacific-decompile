# The hull-segment producer: where `record+34h` gets a real index, and who builds the part-hit array

Addresses: 00724510 00723E90 00723D60 00723AA0 00723F80 00723B70 006D2E30 0070F720 0070F090
00712440 00711020 00923ED0 00925050 0092D1F0 00826F10 004142E0 0098AAE0 00CFD768 00CFD7B8
00CEA050 00CEA05C 00CEA174 0042BAD0 0042BAF0 00470470

Packet `cc7_hit_shape_hull_segment`, read-only in Ghidra. Every descriptive name below is a
hypothesis, not a recovered symbol. `docs/HIT_NARROWPHASE.md`'s record table, `docs/UNIT_HIT_PATH.md`'s
part-entry row, `docs/COLLISION_SHAPES.md`'s vtable tables and `docs/SHIP_HIT_RECORD.md`'s rules R1,
R4 and R11 are cited, not restated.

## 0. The answer

`docs/HIT_NARROWPHASE.md`'s open item 1 asked for "the shape `vtable[0]` implementation that writes a
real hull segment index into `record+34h`". **It exists.** It is the unit-part collision shape, vtable
`00CFD768`, slot 0 `00724510`, which forwards to `00723E90`; `00723E90` copies `+4h` and `+8h` out of
the geometry element it hit:

```
00723f5f: MOV ECX,dword ptr [ESI + 0x4]     ; ESI = the geometry element 00723D60 returned
00723f62: MOV dword ptr [EAX + 0x30],ECX    ; record+30h = element+4h   <- the kind code
00723f65: MOV EDX,dword ptr [ESI + 0x8]
00723f68: MOV dword ptr [EAX + 0x38],ESI    ; record+38h = the element pointer
00723f6c: MOV dword ptr [EAX + 0x34],EDX    ; record+34h = element+8h   <- the index
```

Both values are **authored geometry data**, not constants, which is why no byte scan for an immediate
store finds them. `00723F68` also closes `docs/HIT_NARROWPHASE.md`'s open item 2 for this path:
`record+38h` is the address of the geometry element that was hit.

The part-hit array has the same origin on the sphere side: `00723F80` builds a `10h`-byte entry from
the same two element fields plus the test's distance, and `006D2E30` appends it to the record.

## 1. The two chains

### 1a. Segment trace: the direct hull-segment hit

| step | routine | ABI (from the cleanup) | what it does |
| --- | --- | --- | --- |
| 1 | `00724510` | `__thiscall(shape /*ECX*/; const float3* from, const float3* to, HitRecord* record) -> bool`, `RET 0Ch` at `0072453A` | slot 0 of shape vtable `00CFD768`. `MOV EAX,[ECX+1Ch]` the owning collision node, `MOV ECX,[ECX+24h]` **the geometry**; calls `00723E90(geom; node+50h, node+90h, from, to, record)`, `TEST AL,AL / SETNZ AL` |
| 2 | `00723E90` | `__thiscall(geom /*ECX*/; const Mtx* toWorld, const Mtx* toLocal, const float3* from, const float3* to, HitRecord* record) -> bool`, `RET 14h` at `00723F24`/`00723F75` | transforms both endpoints into node space, runs the closest-hit loop, and on a hit transforms the point back and writes `record+8h..+10h`, `+30h`, `+34h`, `+38h` |
| 3 | `00723D60` | `__thiscall(geom /*ECX*/; const float3* from, const float3* to, float3* outHit) -> const Element*`, `RET 0Ch` at `00723E84` | walks `[geom+0Ch, geom+10h)` with stride `2Ch`, shortening the far end on every hit; returns the **element pointer** of the closest hit, or `0` |
| 4 | `00723AA0` | `__thiscall(element /*ECX*/; const float3* from, const float3* to, float3* outHit) -> bool`, `RET 0Ch` at `00723B56`/`00723B5E` | per-element segment test over the element's `uint16` triangle list `[element+14h, element+18h)`; `contract: partial` — the leaf `007238E0` was not read |

`00724510`'s five pushes reverse to `(node+50h, node+90h, from, to, record)`:

```
00724514: MOV EAX,dword ptr [ECX + 0x1c]    ; the collision node
00724517: MOV ECX,dword ptr [ECX + 0x24]    ; the geometry  -> becomes the callee's this
00724525: LEA EDX,[EAX + 0x90]              ; pushed 2nd  (node+90h)
0072452c: ADD EAX,0x50                      ; pushed 1st  (node+50h)
0072453a: RET 0xc
```

`node+50h` / `node+90h` are the world and inverse-world matrices `docs/COLLISION_SHAPES.md` already
attributes ("the box's `this+70h` is `node+50h` and its `this+B0h` is `node+90h`"). `00723E90` uses
the second on both endpoints and the first on the hit point; `004142E0` is
`BSP_Vector3f_TransformAffinePoint(point /*ECX*/, float3* dst, const Mtx* m)`, `RET 8` — established
here by the argument order, since `EDI` holds the same third argument across both calls at
`00723E99` and `00723EBC` while `ECX` changes from stack arg 4 to stack arg 3.

The miss path is `00723F1B JNZ` over `XOR AL,AL; POP ESI; ADD ESP,0x24; RET 0x14`: **on a miss the
record is not touched at all**, so an un-reset record keeps whatever `00470470` left, which is `-1`.

### 1b. Sphere overlap: the part-hit array

| step | routine | ABI | what it does |
| --- | --- | --- | --- |
| 1 | `0070F090` | four stack arguments per `docs/HIT_NARROWPHASE.md`'s slot table; the cleanup was **not** re-read here | slot `0Ch` of node vtable `00CFD7B8`; per `docs/COLLISION_SHAPES.md` it re-pushes its four arguments, calls `0098AAE0` and normalises with `SETNE`. **It allocates nothing** — the brief's phrasing "the `node->vtable[0Ch]` override that allocates `record+3Ch`" is off by three frames |
| 2 | `0098AAE0` | the base node sphere pass | walks the `0Ah` shape pointers at `node+D0h`, calling `shape->vtable[4]` |
| 3 | `0070F720` | `__thiscall(shape /*ECX*/; const float3* centre, float radius, HitRecord* record)`, `RET 0Ch` | slot 4 of `00CFD768`; refreshes `[shape+20h]` and tails into `00723F80(geom = [shape+24h]; body+F0h, invWorld, centre, radius, record)` |
| 4 | `00723F80` | `__thiscall(geom /*ECX*/; void* bodyF0 /*arg 1, use unread*/, const Mtx* toLocal /*arg 2*/, const float3* centre, const float* radius, HitRecord* record /*arg 5*/) -> bool`, `RET 14h` at `007240D8` | same `2Ch`-stride element walk; for **every** overlapping element it builds a `10h`-byte entry and appends it |
| 5 | `00723B70` | `__thiscall(element /*ECX*/; const float3* centre, const float* radius, float* outDistance) -> bool`, `RET 0Ch` at `00723BA4`/`00723D4E`/`00723D58` | rejects on `radius*radius < 0085BF90(element+24h)`, then walks the same `uint16` triangle list. `contract: partial` — `0085BF90`, `0085DF60` unread |
| 6 | `006D2E30` | `__thiscall(record /*ECX*/; const PartHitEntry* src)`, `RET 4` at `006D2ECE`, body `006D2E30-006D2ECE` | **the allocator and counter of the part-hit array** |

The entry is assembled on `00723F80`'s frame at `[ESP+14h]` (its address is taken at `007240A8
LEA ECX,[ESP+0x14]`, and the frame is `E-30h` inside the loop, so the entry lives at `E-1Ch`):

| entry offset | instruction | value |
| --- | --- | --- |
| `+0h` | `00724078 MOV ECX,[EAX+EBP*1+0x4]`, `0072407C MOV [ESP+0x14],ECX` | `element+4h`, the kind code |
| `+4h` | `007240A4 MOV EAX,[EDX+EBP*1+0x8]`, `007240B1 MOV [ESP+0x1c],EAX` | `element+8h`, the part index |
| `+8h` | `00724044 MOV dword ptr [ESP+0x1c],0x0` | always zero |
| `+0Ch` | `0072403E MOVSS XMM0,[ESP+0x38]`, `0072404C MOVSS [ESP+0x20],XMM0` | the float `00723B70` wrote through the pointer pushed at `0072401F` |

`[ESP+38h]` at `ESP = E-30h` is `E+8h`, the routine's **second stack argument slot**, whose address
`0072401B LEA EDX,[ESP+0x38]` hands to `00723B70` as an out parameter. That slot arrives holding the
inverse-world matrix and is consumed once, before the loop, at `00723F83`/`00723F9B`; the routine
then reuses it as scratch. The distance is therefore a produced value, and it is the numerator
`004705C0` divides by the blast radius `hit+24h` (`docs/UNIT_HIT_PATH.md`).

`0070F720` pushes only three of the five arguments itself — `(centre, radius, record)` at `0070F73C`,
`0070F73D`/`0070F742` and `0070F745` — calls `00B6E0D0`, which **returns without cleaning them**, and
then pushes its result and `[shape+20h]+F0h` on top:

```
0070f746: MOV ECX,dword ptr [ESI + 0x20]   ; the collision body
0070f749: CALL 0x00b6e0d0                  ; returns the inverse-world matrix in EAX
0070f74e: MOV ECX,dword ptr [ESI + 0x24]   ; the geometry -> the callee's this
0070f751: PUSH EAX                         ; arg 2
0070f752: ADD EDI,0xf0
0070f758: PUSH EDI                         ; arg 1
0070f759: CALL 0x00723f80
0070f760: RET 0xc
```

so the five slots at the call are `(body+F0h, invWorld, centre, radius, record)` and argument 5 is
the `HitRecord*` `0070F720` was given. That is the value `007240AD` loads into `ECX`.

The `this` of the append is stack argument 5:

```
007240a8: LEA ECX,[ESP + 0x14]   ; &entry
007240ac: PUSH ECX
007240ad: MOV ECX,dword ptr [ESP + 0x48]   ; ESP = E-34h -> E+14h = stack arg 5
007240b5: CALL 0x006d2e30
```

So `006D2E30`'s `this` is the hit record `0070F720` was handed, and its `+3Ch`/`+40h`/`+44h` are the
record's fields.

## 2. The record layout, from its producer

`006D2E30`, whole body, from the listing:

```
006d2e33: MOV EAX,[ESI+0x44]            ; capacity
006d2e36: CMP [ESI+0x40],EAX            ; count == capacity ?
006d2e39: JNZ 0x006d2ea4                ;   no: append in place
006d2e3b: LEA EAX,[EAX + EAX*0x1 + 0x2] ; newCapacity = capacity*2 + 2
006d2e3f: MOV [ESI+0x44],EAX
006d2e42: SHL EAX,0x4                   ; * 10h
006d2e47: CALL operator_new
006d2e56..006d2e8c                      ; copy [ESI+40h] entries of 10h bytes from [ESI+3Ch]
006d2e90: MOV EAX,[ESI+0x3c]
006d2e95: JZ 0x006d2ea0
006d2e98: CALL _free                    ; free the old buffer
006d2ea0: MOV [ESI+0x3c],EBX            ; publish the new buffer
006d2ea4: MOV EAX,[ESI+0x40]
006d2ea7: SHL EAX,0x4
006d2eaa: ADD EAX,[ESI+0x3c]            ; &array[count]
006d2eaf..006d2ec6                      ; copy 10h bytes from the source entry
006d2ec9: ADD dword ptr [ESI+0x40],0x1  ; ++count
006d2ece: RET 0x4
```

| offset | meaning | producer evidence |
| --- | --- | --- |
| `+30h` | element kind code | `00723F62` from `element+4h`; the three hard-coded shapes write `0Ah` |
| `+34h` | **part index** | `00723F6C` from `element+8h`; `00470470` resets it to `-1` |
| `+38h` | the geometry element that was hit | `00723F68`, `MOV [EAX+38h],ESI` |
| `+3Ch` | **pointer** to the part-hit array, `10h`-byte elements | `006D2EA0` writes the `operator_new` result; `006D2EAA` indexes it; `004704B0` frees it |
| `+40h` | **count** | `006D2EC9 ADD dword [ESI+40h],1`; the copy source count at `006D2E89` |
| `+44h` | **capacity** | `006D2E36` compares it to the count; `006D2E3F` writes `count*2+2`; never dereferenced |

This is corroborated by the record's two copy routines, read for this packet:

* `00923ED0`, the copy constructor (`__thiscall(dest, const src)`, one stack argument, sole callee
  `operator_new`): `dest[0x11] = src[0x10]` then `operator_new(that << 4)` into `dest[0xf]`, then
  `dest[0x10] = dest[0x11]` and a `10h`-byte copy loop. In dword indices `0xf/0x10/0x11` are
  `+3Ch/+40h/+44h`. A copy is made exactly-sized, so `+44h == +40h` after it.
* `00925050` `BSP_HitRecord_Assign`: `param_1[0x10]` (`+40h`) is the count, `param_1[0x11]` (`+44h`)
  is grown only when `param_1[0x11] < src count`, and `param_1[0xf]` (`+3Ch`) is the buffer that
  `operator_new`, `_free` and the copy loop all address.

### The correction

> **`docs/HIT_NARROWPHASE.md` and `docs/UNIT_HIT_PATH.md` are right; the ledger evidence on
> `00925050 BSP_HitRecord_Assign` is wrong.** That record says the routine "reallocates the part
> array at `dest+44h`". `dest+44h` is the capacity; the array pointer is `dest+3Ch`. Both the
> producer `006D2E30` and `00925050`'s own body agree.

`include/bsp/hit_narrowphase.hpp` already declares `kHitRecordOffPartCapacity = 0x44` with the
comment `// 00470470 only`; that comment can now cite `006D2E36`. `include/bsp/unit_hit_path.hpp`'s
comment that its hit-record layout is consumer-derived is now only partly true — `+30h`, `+34h`,
`+38h`, `+3Ch`, `+40h`, `+44h` and the `10h`-byte entry are producer-proven by this packet. **Neither
file is edited here**; both belong to other packets.

## 3. What a real index means

`docs/SHIP_HIT_RECORD.md` R4 gates on `hit+30h == 0Dh`, and the listing confirms both halves:

```
00826fac: CMP dword ptr [ESI + 0x30],0xd    ; ESI = the hit record
00826fb0: JNZ 0x00827043
00826fe1: MOV EBP,dword ptr [ESI + 0x34]    ; the index
00827026: MOV ECX,dword ptr [EDI + 0x1018]  ; parts = *(unit + 1018h)  -- a LOAD, not a LEA
00827037: PUSH EBP
0082703e: CALL 0x0092d1f0
```

So `this+1018h` **holds a pointer to** the unit's breakable-parts object; `0092D1F0` receives that
pointer in `ECX`. Its body (read for this packet; `docs/SHIP_HIT_RECORD.md` owns the routine):

* `-1 < *(char*)(parts + 34Ch + index)` — a signed byte table indexed by the same index gates the
  whole call, so an index the table marks negative is silently ignored;
* the health lives in a `vector<float>` at `[parts+310h, parts+314h)` and the index is bounds-checked
  against `(end - begin) / 4` into `00BF6713`;
* `health[index] -= damage`, and on reaching `0.0f` the slot is parked at `-10000.0f` (`00D19620`)
  and message `99h` is routed.

**A real `record+34h` is therefore an index into the unit's per-part health vector**, not a
geometry-local identifier: the same integer selects the gate byte at `parts+34Ch+index` and the float
at `parts+310h[index]`. `docs/UNIT_HIT_PATH.md`'s part loop R11b passes `entry+4h` to the same
routine, so the segment field and the part-array field carry the same kind of index.

`0Dh` is the element kind that makes an element a breakable part. It never appears as an immediate in
`.text`: a scan of every `C7 /0 disp8 imm32` store in the image (`C7 ?? ?? 0D 00 00 00`) returns eight
sites, none of them at `+30h` or `+0h` of a hit record. The value is authored in the geometry.

## 4. The geometry element, `2Ch` bytes

Both walkers divide `(geom+10h) - (geom+0Ch)` by `2Ch` (`IMUL 0x2E8BA2E9 / SAR EDX,3` at `00723DA4`
and `00724059`), so `geom+0Ch`/`+10h` is a `vector<Element>` and `geom+24h` is the shape's field
`[shape+24h]`.

| element offset | meaning | evidence | status |
| --- | --- | --- | --- |
| `+4h` | kind code; `0Dh` selects the breakable-part path | `00723F5F`, `00724078` | producer-read, **writer unread** |
| `+8h` | part index | `00723F65`, `007240A4` | producer-read, **writer unread** |
| `+0Ch` | the owning mesh: `[+1Ch, +20h)` vertices of `0Ch` bytes, `[+2Ch, +30h)` triangles of `6` bytes (three `uint16`) | `00723B70` divides by `0xC` and by `6` | partial |
| `+14h`, `+18h` | this element's `uint16` triangle-index list; the count is `(end - begin) >> 1` | `00723AA0`, `00723B70` | complete for the count |
| `+24h` | a scalar `0085BF90` turns into the squared-radius reject | `00723B70` head | `contract: unread` |
| `+0h`, `+10h`, `+1Ch`, `+20h`, `+28h` | — | — | unread |

**The loader that writes `element+4h` and `element+8h` was not found.** See §6.

## 5. What is proven, and what is assumed

Proven from the listing in this packet:

* `00724510` -> `00723E90` writes a non-constant `record+34h`, `+30h` and `+38h` from a geometry
  element (`00723F5F`..`00723F6C`), and writes nothing on a miss (`00723F1B`..`00723F24`).
* `0070F720` -> `00723F80` -> `006D2E30` is the only chain that allocates and counts the part-hit
  array, and the record is its `this` (`007240AD`, `0070F720`'s fifth argument).
* `+3Ch` is the pointer, `+40h` the count, `+44h` the capacity; the growth law is `cap*2 + 2` and the
  element stride is `10h`. Three independent bodies agree (`006D2E30`, `00923ED0`, `00925050`).
* The entry is `{ element+4h, element+8h, 0, distance }`.
* `0092D1F0` uses the index twice, against `parts+34Ch` and `parts+310h`, and `unit+1018h` is a
  pointer slot that is loaded, not a subobject.
* `00723D60`/`00723AA0`/`00723B70`/`00723E90`/`00723F80`/`00724510`/`006D2E30` argument counts all
  come from the `RET imm` listed in §1, not from the decompiler: Ghidra renders `00723E90` with five
  parameters and no `this`, yet `00723E9E MOV ESI,ECX` and `00723F0A MOV ECX,ESI` show `ECX` is a
  live input forwarded to `00723D60`.

Assumed, or read only through an existing doc:

* That the shape-class set is closed. `docs/COLLISION_SHAPES.md` enumerated it with two on-disk
  sweeps keyed on the slot-8 bodies `004E6470` and `0070F710`; a class that overrides slot 8 with a
  third stub would be invisible to both. See §6 for one candidate.
* `0098AAE0`'s shape walk, `00B6DB70`, `00B6E0D0`, `0085BF90`, `0085DF60`, `007238E0`, `00723170` —
  all `contract: unread` here.
* Rules R4 and R11b themselves belong to `docs/SHIP_HIT_RECORD.md`; only the four instructions quoted
  in §3 were re-read.
* No run-time evidence: on the current tree `bsp_game.exe` reports `part=0`, i.e. the executable does
  not reach this producer, so checklist rule 6 does not apply and none of the above is run-confirmed.

Negative results, stated as negative results:

* **No immediate store of `0Dh` into `+30h` or into a part entry's `+0h` exists in the image.** Scan
  `C7 ?? ?? 0D 00 00 00`: `004477E8`, `00465F46`, `006EA808`, `007487A9`, `007827D0`, `007B95AB`,
  `007BCB01`, `00851022` — displacements `0Ch`, `10h`, `08h`, `0Ch`, a disp32 form, `0F4h`, `0F4h`,
  `0Ch`. The kind code only ever arrives from data.
* **The byte scan the packet brief started from cannot settle this question.** `89 ?? 34` (every
  `MOV [reg+34h], r32` with a `disp8` and no SIB) matches 471 sites in 374 functions, and the subset
  that also writes `+30h` is 207 functions. `00723F6C` is `89 50 34` — a different base and source
  register from the `89 4E 34` the brief scanned, which is why it was missed. The producer was found
  through the class table, not the scan.

## 6. Follow-up packets

1. **The geometry loader.** Nothing was found that *writes* `element+4h` or `element+8h`. The element
   vector is reached as `[shape+24h]`, and `00712440` `BSP_UnitPartCollisionNode_BuildShapes` takes it
   from an 8-byte-element vector at `[node+160h]+40h..+44h` whose pairs are
   `{ geometry*, transform* }` (`*local_5c` -> `shape+24h`, `local_5c[1]` -> `shape+20h`, stored at
   the stack shape `local_28..local_4` before `00711460`). `docs/COLLISION_SHAPES.md` calls
   `shape+24h` "the owner back-pointer"; `00724514`..`00724517` show it is the geometry the trace
   runs against. Whoever fills `[node+160h]+40h` is the next step, and it decides whether `element+8h`
   is authored per mesh group or assigned at load time.
2. **A possible sixth shape vtable.** `0087FEC0`, which `docs/HIT_NARROWPHASE.md` calls a shape
   `vtable[0]`, is referenced from `00CEA174`. The three dwords there are
   `{ 0087FEC0, 0042BAD0, 0042BAF0 }`; `0042BAF0` is `XOR AL,AL; RET 4`, the same shape of stub as the
   base slot 8 `004E6470`, but `0042BAD0` is `CMP [ECX+3Ch],0 / MOV ECX,[ECX+3Ch] / JMP [[ECX]+0E8h]`
   with an `XOR AL,AL; RET` (no immediate) fallback, which does not match a `RET 0Ch` sphere slot.
   Either `00CEA174` is a shape vtable that `docs/COLLISION_SHAPES.md`'s slot-8 sweep could not see,
   or `docs/HIT_NARROWPHASE.md`'s attribution of `0087FEC0` to a shape interface is wrong. Settling it
   would close the "is the class set complete" gap in §5. **Unresolved here.** Either way `0087FEC0`
   writes `-1`, so the answer in §0 does not depend on it.
3. **`element+24h` and `0085BF90`** — the sphere reject's squared radius.
4. **`007238E0`** — the leaf segment/triangle test `00723AA0` calls; the only unread part of the
   segment chain.
5. **`docs/UNIT_INSTANCE_LAYOUT.md` vs `docs/UNIT_FIRE_AND_REPAIR.md` on `+1018h`.** The first calls
   it "controller, `kUnitOffController`", the second "parts object". `00827026` on the ship entity
   agrees with the second. Not this packet's address.
