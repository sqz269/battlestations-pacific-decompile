# The `GeomMesh` resource payload: the authored hull segments, decoded

Addresses: 00727310 007149D0 00BE9A80 00BE99E0 00BE9B20 00BE9A00 00BE99D0 00BEA010 00BE9C40
00BF0280 00BF02C0 00CFDBC8 00CFDBCC 00E08138 00727A90 00726D80 00727260 00725B40 004215D0
00725AA0 007262A0 00726670 007263B0 00725380 00BF6713 00BF6989 00BF65AC

Packet `cc7_geom_mesh_resource_parser`. Ghidra was read-only for analysis; the only writes were the
four ledger records listed in §8. Every descriptive name below is a hypothesis, not a recovered
symbol. `docs/PART_DAMAGE_REACHABILITY.md` established *that* `00727310` writes the geometry
element's `+4h` and `+8h` and *why* that matters (rule R1 at `00826F62` skips the part, fire and
flood arms together when `hit+34h == -1`); it is cited, not restated. This document establishes the
payload's **wire format**, verified step by step against the listing and against authored data.

This installation is **modded** (BSPRM/AlterBSP). Every data file was opened read-only; the game
installation was not written to. Each file is listed with its mtime in §5.

## 0. The answer, in one paragraph

The `GeomMesh` payload is a counted element list, a vertex array, an edge array and a triangle
array. Each element is `{ counted name, u32 index }`; the name goes through `007149D0` against the
fifteen-entry table at `00E08138` (with `steering` (7) filed as `body` (9)) to become the element's
`+4h`, and the `u32` becomes its `+8h`. **Every integer on the wire is four bytes**, including the
reads through `00BE9A80` that `docs/PART_DAMAGE_REACHABILITY.md` §1e called `u16` — that clause is
refuted in §2. A triangle record is 32 bytes: eight `u32`. With that correction the decode of
`Farragut_1934.MMOD`'s chunk consumes its declared `0x1C5BA` payload with **zero bytes left over**
and yields exactly the documented element list `{body 0, underwater 0, fizika 0, magazine 0,
fizika 1, engineroom 0, fizika 2}`. Nine chunks across five ship files decode with zero leftover.

## 1. The format, verified step by step

The legend from `docs/PART_DAMAGE_REACHABILITY.md` §1e, re-derived from each helper's own body
rather than assumed:

| helper | body | primitive | width |
| --- | --- | --- | --- |
| `00BE9A00` `BSP_StructuredNode_ReadU32` | `8b 01 8d 48 20 51 8b 48 08 e8 72 68 00 00 c3` | `00BF0280` | 4 |
| `00BE99E0` | byte-identical but `e8 92 68 00 00` | `00BF0280` | 4 |
| `00BE9A80` | `00be9a95 MOV [EDX],EAX`, `00be9a97 MOV EAX,ESI`, `RET 4` | `00BF0280` | 4 |
| `00BE99D0` `BSP_StructuredNode_ReadFloat` | `e8 e2 68 00 00` | `00BF02C0` | 4 (float) |
| `00BE9B20` | `MOV EAX,[ECX]; MOV ECX,[EAX+8]; MOV EAX,[ECX+64h]; RET` | — | the remaining count |
| `00BEA010` `BSP_StructuredNodeHandle_ReadString` | — | — | `u32` length then raw bytes |
| `00BE9C40` `BSP_StructuredNode_SkipAndDetach` | — | — | — |

`00BF0280` is already in the ledger as `BSP_StreamReader_ReadDwordSlot34AndDebit` (stream virtual
`+34h`, DWORD); `00BF02C0` is `BSP_StreamWrapper_ReadFloatAndAccount` (virtual `+44h`). The three
displacements above were read from the bytes, not from Ghidra's rendered call target.

`00BE9A80` differs from `00BE9A00` only in that it stores the DWORD through an out-parameter and
returns the node handle in `EAX`, which is what lets `00727694`/`0072769B`/`007276A2` chain
`MOV ECX,EAX` between calls.

The body, with `ESP = ESP0-84h` through the loops (`PUSH -1`, `PUSH`, `PUSH`, `SUB ESP,68h`, then
`PUSH EBP/ESI/EBX/EDI`). `EBP` is the node handle, loaded once at `00727329 MOV EBP,[ESP+7ch]`
(= `ESP0+4`, argument 1) and never rewritten until `0072770B XOR EBP,EBP`, after the last read.

```
00727336  if (00be9b20(node) < 4) { 00be9c40(node); return; }    ; CMP EAX,4 / JNC 0072735a
0072735c  u32  elementCount        -> 00726d80(mesh+8h, n)       ; EDI, also to [ESP0+4]
00727377  CMP EDI,EBX / JLE                                      ; signed: <= 0 skips the loop
          repeat elementCount:
0072738a    string name                                          ; u32 length, then bytes, no NUL
0072739e    if (name.characters == NULL) name = 00E19BF4          ; the empty literal
007273a3    kind = 007149d0(name);  if (kind == 7) kind = 9      ; 007273b1 CMP EDI,7
007273ac    u32 index
007273fa    push_back{ 00CFDBC8, kind, index, mesh, 0... }       ; the 2Ch temporary at [ESP+4ch]
00727471  u32  vertexCount         -> 00725b40(mesh+18h, n)
          repeat vertexCount: three floats -> 004215d0(mesh+18h, &v)
007274c4  u32  pairCount
          repeat pairCount: two u32 (00be9a80 x2) into ONE slot  ; both discarded
007274ef  u32  triangleCount
00727504    per element: 00725aa0(element[i]+10h, triangleCount / elementCount)   ; CDQ / IDIV
00727563    007262a0(mesh+28h, triangleCount)
          repeat triangleCount:
00727599    append a zeroed 6-byte record to mesh+28h            ; 007275ab LEA EAX,[EDI-6]
007275d9    u32 elementIndex                                     ; bounds-checked, 00bf6713 throws
00727646    element[elementIndex].ordinals.push_back((u16)i)     ; i = [ESP0+4], the loop counter
00727672    if (00be9b20(node) >= 7) 00be99e0(node)              ; one u32, discarded
00727694    u32 v0 ; 0072769b u32 v1 ; 007276a2 u32 v2           ; stored as words at 007276d4..e5
007276ae    u32 ; 007276ba u32 ; 007276c6 u32                    ; three more, all discarded
00727704  the same-kind merge pass (reads nothing)               ; see section 3
00727a76  00725380(mesh)                                         ; a finalizer, body unread
```

Corrections and confirmations against §1e of the earlier document, clause by clause:

| §1e clause | verdict |
| --- | --- |
| `< 4` bail through `00BE9B20`/`00BE9C40` | **holds** |
| `u32 elementCount` then `00726D80` | **holds**; the count is also stashed into the incoming argument slot at `00727369` (`MOV [ESP+8ch],EDI` with `ESP = ESP0-88h`) and reused as scratch |
| `string name` | **holds** |
| `kind = 007149d0(name); if (kind == 7) kind = 9` | **holds**; the null-pointer fallback to `00E19BF4` at `0072739E` is an addition |
| `u32 index` | **holds** |
| `push_back { vtable 00CFDBC8, kind, index, mesh, 0... }` | **holds**; stride `2Ch` confirmed twice, below |
| `u32 vertexCount`, three floats each | **holds** |
| `u32 n2 ; repeat: two u16 reads` | **wrong**. The reads are `u32`. Eight bytes per entry, not four. See §2 |
| `u32 triangleCount`, `00725AA0(element[i]+10h, triangleCount/elementCount)`, `007262A0` | **holds** |
| `00727580..` "the triangle loop (u16 reads, per-element index lists)" | the per-element index lists **hold**; the reads are `u32`, stored truncated to `u16` |
| `coverage: partial` from `00727580` to the end | now decoded; see §3 |

The element stride is `2Ch`, established two independent ways: `00727260`'s `0x2E8BA2E9` /
`SAR EDX,3` magic is division by `0x2C` (recorded by the earlier packet), and the temporary this
parser assembles spans `[ESP+4Ch]`..`[ESP+74h]`, which is `0x2C` bytes.

| element offset | written at | meaning |
| --- | --- | --- |
| `+00h` | `007273BF` | vtable `00CFDBC8`, whose slot 0 is `00725880` (called at `00727A48` when an element is erased) |
| `+04h` | `007273DB` | the kind |
| `+08h` | `007273DF` | the node index |
| `+0Ch` | `007273E3` | the owning mesh |
| `+10h` | — | MSVC `vector<u16>`: proxy `+10h`, begin `+14h`, end `+18h`, capacity `+1Ch`. `00727643` writes a word and advances end by 2 |
| `+20h`, `+24h` | `007273D3`, `007273D7` | zeroed here; the temporary's teardown frees both through `00BF6989` (`00727410`, `00727421`), so both are heap pointers |
| `+28h` | `007273E7` | zeroed here, not freed |

`+20h`/`+24h`/`+28h` are **not** decoded by this packet: `00727310` only zeroes them. That leaves
`docs/HIT_HULL_SEGMENT.md`'s reading of `+24h` as a scalar exactly where the earlier packet left
it — an open item, not a correction.

## 2. Why `00BE9A80` is four bytes, not two

Three independent lines, two static and one from data:

1. **The body.** `00BE9A80` calls `00BF0280`, byte-verified from `e8` displacements, which the
   ledger already records as the stream's DWORD method (virtual `+34h`). `00BE9A00`, the
   established `ReadU32`, calls the same primitive; `00BE99E0` is byte-identical to `00BE9A00`
   apart from the displacement. `00BE9A95 MOV dword ptr [EDX],EAX` stores a full dword.
2. **The frame.** The three chained corner reads write to `[ESP0-60h]`, `[ESP0-5Ch]`, `[ESP0-58h]`
   — four bytes apart, not two.
3. **The data.** Decoding `Farragut_1934.MMOD`'s chunk both ways:

   | `00BE9A80` width | triangleCount | bytes consumed | declared | leftover | first triangles |
   | --- | --- | --- | --- | --- | --- |
   | 2 | 830 | `0xB3E2` | `0x1C5BA` | 70104 | `(829,0,829,0)`, `(0,830,0,826)` |
   | **4** | **2212** | **`0x1C5BA`** | **`0x1C5BA`** | **0** | `(0,0,1,2)`, `(1,3,4,5)` |

The arithmetic closes exactly at four bytes: `4 + 106 + (4 + 1394*12) + (4 + 3565*8) + 4 +
2212*32 = 116154 = 0x1C5BA`.

The `remaining >= 7` guard at `00727672` therefore fires for every triangle in well-formed data —
at the last triangle, 28 bytes still remain when it is tested. It is modelled because it is there,
but no chunk in this installation takes the other arm.

## 3. What `00727704` does, and why `fizika` survives it

`00727704`..`00727A6D` reads **nothing** from the stream (there is no `00BE9*` call in that range).
It is a forward walk over the element vector:

* `0072776C` `CMP [EAX+EDI+4],0Dh` / `JZ 00727A60` — an element of kind `0Dh` (`fizika`) is skipped.
* `0072779D` `CMP [ECX+EDI+4],0Eh` / `JZ 00727A60` — so is kind `0Eh` (`bullet`).
* `007277A3` `CMP EBX,EBP` / `JLE 00727A60` — index 0 has no earlier element, so it is skipped.
* `007277B0`..`00727820` searches indices `0..i-1` for the **first element with the same kind**
  (`00727810` compares `+4h`).
* On a hit, `00727834` records that index and `00727840`..`0072797D` appends element `i`'s
  ordinals to it one at a time (fast path `00727934`, grow path `0072796B`).
* `00727A51 ADD [ESI+8],-2Ch` then drops element `i`; `00727A55 SUB EBX,1` against `00727A67
  ADD EBX,1` leaves the index unchanged, so the slot is re-examined.

So duplicate kinds are collapsed, and **`fizika` and `bullet` elements are exempt**. That is the
mechanism by which a hull keeps one element per authored `fizika_NN` node, and therefore one
distinct `+8h` per hull segment — which is what rule R4's `hit+30h == 0Dh` plus a usable
`hit+34h` needs.

Two of the nine chunks decoded in §5 exercise the merge, and both exercise the `7 -> 9` remap at
the same time — they are the only chunks whose element list contains a `steering`. `Yamato1945.mmod`
authors `{underwater 0, magazine 0, body 0, engineroom 0, fueltank 0, steering 0}`; `steering`
becomes kind 9, matches the earlier `body 0`, and after the pass five elements remain with `body`'s
ordinal list grown from 1381 to 1393 — exactly the 12 that `steering` carried. `yamato.mmod`'s
chunk at `0x246EEE` does the same, 151 + 11 = 162. No chunk was found in which two elements of a
kind other than `body` collide.

`coverage` for `00727310`: **complete** for the stream reads and for which elements survive the
merge. **Partial** only for the element-shifting instructions at `00727982`..`00727A4F`, which were
read far enough to identify the erase (`00727A48 CALL EAX` with `PUSH 0` is the vtable-slot-0
teardown, applied over a range) but not transcribed instruction by instruction.

## 4. The reconstruction

`include/bsp/geom_mesh_resource.hpp`, `src/geom_mesh_resource.cpp`, registered to `bsp_game` by one
appended line in `cmake/startup.cmake` next to `src/part_damage_reachability.cpp`, which supplies
the `00E08138` lookups this module reuses rather than redeclares.

* `parse_geom_mesh_resource_00727310(StructuredNode&, GeomMeshResourcePayload&, std::string&)` —
  the parser. Reads through `StructuredNode::read_u32` / `read_float` / `read_string`.
* `merge_same_kind_elements_00727704(std::vector<GeomMeshElement>&)` — §3's pass, published
  separately so it can be inspected on its own; `parse_...` runs it, as the native does.
* `GeomMeshStructuredResourceParser` — derives from the registry's published
  `StructuredResourceParser`; `type_name()` returns `"GeomMesh"`, the literal `007258F0` pushes
  (`00CFDBCC`, byte-verified).

Three places where the host is deliberately not the native:

* **The `< 4` bail.** The native skips the node and returns; that is not a dispatcher failure. The
  host returns `true` with `payload_too_small` set rather than `false`, because
  `dispatch_items_00b7e970` treats `false` as a hard abort of the whole container.
* **Short reads.** The native has no bounds or short-read checks anywhere; it consumes whatever the
  stream returns. The host returns `false` with a message when a read fails, when a count cannot
  fit in the remaining payload, or when a triangle names an element outside the list (which is the
  `00BF6713` throw at `00727612`). That rejection is a host addition.
* **`00BE9B20`.** The native reads the budget off the stream object at `+64h`. The host reader
  exposes only the node's own remaining count. For a chunk read directly under its container those
  coincide; for a deeper nesting they need not, and the guard at `00727672` is the only place where
  the difference could change behaviour.

**The registry seam.** `StructuredResourceParser::decode` writes into `StructuredResourcePayload`,
a closed `std::variant` of `MeshResourcePayload` / `NoteResourcePayload` /
`GroupParamsResourcePayload` declared in `include/bsp/structured_resource_registry.hpp`, which this
packet does not own and did not edit. Deriving from the base needed no change to that header, and
registration and dispatch work unmodified. But a GeomMesh payload is none of the three
alternatives, so `decode` leaves the variant argument untouched and delivers the payload through a
caller-supplied sink and `last_payload()`. A caller using `dispatch_items_00b7e970` therefore gets
a `DecodedStructuredResource` whose `type_name` is `"GeomMesh"` and whose `payload` holds the
variant's default alternative, carrying no GeomMesh data — **read the sink, not that record**.
Adding a fourth alternative is the coordination change that would remove the seam, and it belongs
to the registry's owner.

No test was added. `bsp_math_tests` links `bsp_core` only and cannot reach a `bsp_game` source, and
the acceptance case needs a 116 KB authored payload, which is not a fixture that belongs in
`tests/math_tests.cpp`. The ad hoc driver `local/geom_mesh_probe.cpp` carries it instead; it is
compiled with `cl /MD ... /link /MANIFEST:EMBED` against `build/win32/Release/bsp_core.lib` and is
not part of the CMake build.

## 5. The decode of authored data

Driver: the real chunk bytes copied into a synthetic `Resource` container, decoded through the
peer's **unmodified** `StructuredResourceRegistry::dispatch_items_00b7e970` with
`GeomMeshStructuredResourceParser` registered. `root remaining` is 0 after every run, which is the
reader's own statement that the payload was consumed exactly.

| file (this installation) | mtime | chunk | decoded elements |
| --- | --- | --- | --- |
| `models/ships/us/Farragut_1934.MMOD` | 2024-07-13 11:24:44 -0700 | `0x1587DF` | `{body 0, underwater 0, fizika 0, magazine 0, fizika 1, engineroom 0, fizika 2}` |
| same | | `0xEEB3F3` | `{body 1, underwater 1, fueltank 0, fizika 3}` |
| `models/ships/us/North_Carolina.mmod` | 2024-07-13 11:24:54 -0700 | `0x1BEFC64` | `{fizika 0, fizika 1, magazine 0, body 0, engineroom 0, fueltank 0}` |
| `models/ships/japan/Yamato1945.mmod` | 2024-07-13 11:22:10 -0700 | `0x2C111` | authored 6, 5 after the merge (§3) |
| `models/ships/japan/yamato.mmod` | 2024-07-13 11:23:20 -0700 | `0x1E4EEA` | `{fizika 1, fizika 2, fizika 5, fizika 6, fizika 0, fizika 4, fizika 3}` |
| same | | `0x1ED108` | `{fizika 7, fizika 8, fizika 9, fizika 10}` |
| same | | `0x246EEE` | `{engineroom 0, body 0, fueltank 0, underwater 0, steering 0, magazine 0}` |
| `models/ships/us/Colorado.MMOD` | 2024-07-13 11:24:44 -0700 | `0x3AA55B4` | `{body 0, magazine 0, fueltank 0, engineroom 0, fizika 1, fizika 0}` |
| `models/ships/us/Essex.mmod` | 2024-07-13 11:24:44 -0700 | `0x71` | `{body 0, magazine 0, engineroom 0, fizika 0, fueltank 0}` |

The Farragut chunk at `0x1587DF` reproduces `docs/PART_DAMAGE_REACHABILITY.md` §1f exactly:
1394 vertices, 3565 pairs, 2212 triangles, first vertex `(2.151, 2.269, 40.278)`, first triangle
`(0, 1, 2)`.

One correction to §1f's address: `0x1587E3` is where the eight characters `GeomMesh` begin. The
**counted tag** — the `08 00 00 00` length prefix the reader consumes first — starts at `0x1587DF`,
the `u32` payload size `0x1C5BA` is at `0x1587EB`, and the body at `0x1587EF`.

And one clarification to §1f's last sentence: `North_Carolina.mmod` does carry 46 occurrences of
the string `fizika`, but only **two** of them are `GeomMesh` elements. The rest live elsewhere in
the file. A string count is not an element count.

## 6. What is proven vs. what is assumed

**Proven** (listing, bytes, or an exact-fit decode of authored data):

* Every integer read in `00727310` is four bytes wide, and the `00BE9A80` clause of §1e is wrong.
* The wire order in §1, the `2Ch` element stride, and the element field table of §1.
* The `7 -> 9` remap, and that kinds `0Dh` and `0Eh` are exempt from the merge pass.
* The triangle record is 32 bytes and the three corner values are stored truncated to `u16`.
* The element ordinal appended at `00727646` is the triangle's index in `mesh+28h`.
* `00727310`'s only caller is `00727A90` (one xref; the checklist's 25-row cap is not in play).
* `type_name()` is `"GeomMesh"`: `007258F0` pushes `00CFDBCC`, whose bytes are `GeomMesh`.

**Observed in data but not proven by the code**, because `00727310` discards these values:

* The `pairCount` block is an **edge list**. Every value is a vertex index, and every pair is a
  distinct undirected pair, in the three chunks measured pair by pair (both Farragut chunks and
  `yamato.mmod`'s first).
* The three trailing `u32` of each triangle are **edge indices**: across the nine chunks of §5,
  62349 of 62349 of them index the pair array and name an edge of their own triangle — 0 failures.
* The guarded read at `0072767E` is zero for every triangle of every chunk decoded here. That it
  is an integer and not a float is proven (`00BF0280`, not `00BF02C0`); what it means is not.

**Not established:**

* `element+20h`, `+24h`, `+28h`. `00727310` only zeroes them; their producers are elsewhere.
* Whether the geometry element's `+8h` and the Lua descriptor's `Index` (`0087CF60`) share one
  index space. Both producers write the same shape of pair; nothing here links the two spaces.
* `00726D80`, `00727260`, `00725B40`, `00725AA0`, `007262A0`, `00726670`, `007263B0` and
  `00725380` were **not read**. Their contracts above are inferred from `00727310`'s call sites and
  from the vector arithmetic inlined at those sites. They are named by address only.
* `00727A90` was not re-read in this packet; it is cited from `docs/PART_DAMAGE_REACHABILITY.md`.

## 7. Follow-up packets: what is still needed for a non-zero `part` count

Decoding the payload does not by itself move `part=0 fires=0 floods=0`. What the host still needs,
smallest first:

1. **A caller that runs this parser on the mission's ship models.** Nothing in `bsp_game.exe`
   registers `GeomMeshStructuredResourceParser` yet. The model load path would have to reach the
   `Resource` container of an `.MMOD` and dispatch it. `docs/GAME_RESOURCE_PARSER_REGISTRATION.md`
   has the native registration row; the host side of it is not built.
2. **Somewhere to put the decoded elements.** The narrowphase in `docs/HIT_NARROWPHASE.md` uses
   three hard-coded shapes that write kind `0Ah` (`none`) and index `-1`. Until a unit carries a
   decoded element list, `00723D60`'s walk has nothing to walk, so `hit+30h`/`hit+34h` keep coming
   from the hard-coded shapes. This is the real blocker and it is a *narrowphase* packet, not a
   parser one: the hit record's `+30h`/`+34h` must be filled from the element the ray actually hit
   (`00723F62`/`00723F6C` are the native copies).
3. **The triangle-to-element mapping is already in hand.** Each element's ordinal list indexes
   `mesh+28h`, so once a ray-triangle test names a triangle, the owning element — and therefore the
   `(kind, index)` pair — is a lookup. That is why the ordinal lists are retained in the payload.
4. **The controller slot has to exist.** `00937C90` sizes the per-part health vector to exactly 20
   and walks `fizika_%02d` for 0..19 (`docs/PART_DAMAGE_REACHABILITY.md` §2). A decoded `+8h` must
   land in that range *and* the model must actually carry that `fizika_NN` node, or `0092D1F0`
   returns at once. Farragut authors indices 0..3; `yamato.mmod` authors 0..10. Both fit.
5. **Only then is rule R1 satisfiable.** `00826F62` needs `hit+34h != -1`; R4 additionally needs
   `hit+30h == 0Dh`, which only a `fizika` element provides.

An additional piece of leverage this packet did not chase: the `fizika` element's `+8h` values in
authored data are dense and start at 0 per file, which matches the `fizika_%02d` naming `00937C90`
walks. Confirming that the geometry index and the controller slot index are the same number is the
natural next evidence step, and it is the same open question as §6's Lua `Index` item.

## 8. Ledger records written

| address | record |
| --- | --- |
| `00727310` | evidence appended: the verified wire format, the 32-byte triangle record, the merge pass, and an explicit refutation of the `u16` clause |
| `00BE9A80` | `BSP_StructuredNode_ReadU32ToOutParam` |
| `00BE99E0` | `BSP_StructuredNode_ReadU32Discarded` |
| `00BE9B20` | `BSP_StructuredNode_RemainingBytes` |

Leased: `00727310`, `007149D0`, `00BE9A80`, `00BE99E0`, `00BE9B20`. `00723E90`, `00724510`,
`006D2E30`, `0092D1F0` and `00937C90` were read as contracts from merged packets and deliberately
not leased.

Report: `reports/cc7_geom_mesh_resource_parser.json` — 41 call rows, 0 failures under
`tools/verify_report_calls.py`.
