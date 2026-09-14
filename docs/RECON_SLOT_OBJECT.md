# The recon slot object: allocation, the published enemy list, and the host prescription

Packet `cc7_recon_slot_object`. Addresses: `008053C0`, `008050E0`, `008073C0`, `00804E10`,
`008042B0`, `00793670`, `007A84D0`, `00864FE0`, `009F5D30`, `004C3CB0`, `005B2D70`, `00806480`,
`00F874BC`.

`docs/GAME_EXECUTABLE.md` records that `[recon + DE8h]`, the side's published enemy contact list,
has no producer in the reconstructed process: "this process builds no recon slot object". The rule
that writes into the object is reconstructed (`docs/GUNNERY_RECON_DETECTION.md`) and the authored
rows exist (`docs/SENSOR_TABLE_DATA.md`). This packet reads the **producer** of the object itself,
the writer of the `+DE8h` field, and the walk that every consumer performs, and turns them into a
host prescription.

`docs/RECON_SLOT_LISTS.md` (packet `cc2_recon_slot_lists`) already carries the slot layout table,
the membership rule and the rebuild step list. Everything below is either a producer reading that
document states as a table without the instruction, or new. Where this document repeats a fact it
is to attach the instruction that proves it; the two do not disagree anywhere.

---

## 1. The object, its owner, and its construction site

The recon slot object is **`0x12A0` bytes**, one per party, held in a three-entry static pointer
table at `00F874BC`, and constructed lazily on first use.

```
008053C0  BSP_Recon_EnsureSlot   __fastcall void*(int party)   body 008053C0-00805423
008053D9  MOV EAX,[ESI*4 + 00F874BC]      the table, indexed by party
008053E0  TEST EAX,EAX ; JNZ 00805414     already built -> return it
008053E4  PUSH 12A0h
008053E9  CALL 00BF681B                   operator new
008053F5  TEST EAX,EAX ; JZ 0080540B      allocation failed -> store null
00805401  PUSH ESI
00805404  CALL 008050E0                   the constructor, with the party index
0080540D  MOV [ESI*4 + 00F874BC],EAX      publish the slot
00805414  ...                             both paths reach the epilogue with EAX set
```

The `12A0h` operand at `008053E4` is the only size evidence and it is exact: the constructor's last
array starts at `+0E14h` and is `61h * 0Ch = 48Ch` long, and `0xE14 + 0x48C == 0x12A0`, so the
object ends on its last member with no trailing padding.

**Owner.** `00F874BC` is a plain three-pointer array in `.data`; the dword immediately after it,
`00F874C8`, is the scanned-class-id singleton and not a fourth slot (`00806480`). Six routines
touch the table, and this is the complete census (`scan-bytes 'bc 74 f8 00'`, six matches, all in
`.text`):

| site | function | what |
| --- | --- | --- |
| `008053DC`, `00805410` | `008053C0 BSP_Recon_EnsureSlot` | the lazy read and the store |
| `00803BA3` | `00803BA0 BSP_Recon_MarkSlotDirty` | set `+25h` on one slot |
| `00805288` | `00805240 BSP_Recon_DestroySlot` | clear the entry as the slot dies |
| `008034E2` | `008034E0 BSP_Recon_DestroyAllSlots` | tear all three down |
| `008079F8` | `008079B0 BSP_Recon_ServicePeriodicRefresh` | walk all three on the countdown |

Nothing else in the image reaches the table, so **`008053C0` is the only way to obtain a slot** and
every consumer either calls it or was handed a pointer that came from it.

**Construction site.** `008050E0`, `__thiscall ReconSlot*(ReconSlot*, int party)`, body
`008050E0`-`0080522C`. Read in full. It is the sole producer of the layout, and it writes, in this
order:

| offset | constructor instruction | value |
| --- | --- | --- |
| `+4h`, `+8h`, `+0Ch` | early stores | `0` |
| `+10h` | byte store | `0` |
| `+0h` | after the zeroing | vptr `00D08E94` (`PTR_CG_scalar_deleting_dtor_00805d70`) |
| `+14h` | after the zeroing | vptr `00D08E88`, the second base |
| `+18h`, `+1Ch`, `+20h` | early stores | `0` |
| `+34h` | `_eh_vector_constructor_iterator_(this+0Dh dwords, 0Ch, 61h, 008039B0, 00804E00)` | array **A**, 97 empty lists |
| `+4C0h` | the same helper, `this + 130h` dwords | array **B** |
| `+94Ch` | the same helper, `this + 253h` dwords | array **C** |
| `+0DD8h`..`+0E10h` | fifteen dword stores, `param_1[0x376]`..`param_1[0x384]` | the **five** `{count, head, tail}` triples, all zero |
| `+0E14h` | the same helper, `this + 385h` dwords | array **D**, the carry-over |
| `+28h` | `param_1[10] = param_2` | the **party index** |
| `+2Ch` | `param_1[0xB] = DAT_00F876A4` | the global time, so the first `+30h` delta is zero |
| `+25h` | `*(byte*)(this+25h) = 1` | publish-dirty, set at birth |

Two consequences the constructor settles that a consumer cannot:

* there are **four** `97 x 0Ch` arrays, not three. `+0E14h` is built by the same
  `{0Ch, 61h, 00804E00}` triplet as A, B and C, so it is a constructed member and not slack.
* there are **five** triples, not four. `+0E08h` is zeroed by the same run of stores as the four
  published ones; `00806B10` simply never publishes it.

**A caution for readers of the packet brief.** `[slot+28h]` is the **party index** written at
construction, not "the winning observer pointer". The winning observer pointer is `00805AF0`'s
fourth native argument and it lands in the unit's detection record at `record+28h`
(`unit + 1E8h + party*34h`, `+28h`). Two different `+28h` fields on two different objects.

---

## 2. Lists and nodes, from their producers

Three independent producers agree, at three different offsets, on the same two shapes.

**`00804E10`**, `__thiscall void(List* this, List* source)`, `RET 4`, body `00804E10`-`00804E6D`,
read in full. It is the copy-append the rebuild uses to fill a triple:

```
for (node = source->[+4h]; node; node = node->[+4h])   head at +4h, next at +4h
    payload = node->[+8h]                              payload at +8h
    fresh = operator new(0Ch); zero its three words
    fresh->[+8h] = payload
    fresh->[+0h] = this->[+8h]                         prev := old tail
    if (this->[+0h] == 0) this->[+4h] = fresh          empty -> it is the head
    else                  this->[+8h]->[+4h] = fresh   otherwise chain onto the tail
    this->[+8h] = fresh                                tail := fresh
    fresh->[+4h] = 0
    this->[+0h] += 1                                   count
```

| shape | `+0h` | `+4h` | `+8h` | size |
| --- | --- | --- | --- | --- |
| list ("triple") | `count` | `head` | `tail` | `0Ch` |
| node | `prev` | `next` | `payload` | `0Ch` |

The source is never modified, which is why one `1Ch` entry record is simultaneously in a class
bucket and in one or more triples: the triples hold **copies of the nodes**, sharing the payload.

**Second producer, at `+0DFCh`.** The drain's copy-into-unknown builds the same shape by hand:

```
0080766E  MOV [EAX+8],EBX                 fresh->payload = the drained payload
00807671  MOV EDX,[ESI+0E04h]             the unknown triple's tail
00807677  MOV [EAX],EDX                   fresh->prev = tail
00807679  CMP [ESI+0DFCh],EBP             count == 0 ?
0080768C  MOV [ESI+0E00h],EAX             yes -> head = fresh
00807687  MOV [ECX+4],EAX                 no  -> tail->next = fresh
00807692  MOV [ESI+0E04h],EAX             tail = fresh
00807698  MOV [EAX+4],EBP                 fresh->next = 0
0080769B  ADD [ESI+0DFCh],1               count
```

so `+0DFCh` count, `+0E00h` head, `+0E04h` tail.

**Third producer, at `+0DD8h`.** `007A84D0` reads `[slot+0DD8h]` as a count and `[slot+0DDCh]` as
the head (`007A8500`, `007A8514`). Same `{count, head, tail}` stride at the own triple.

**The `1Ch` payload record.** Producer `00804E70` (per `docs/RECON_SLOT_LISTS.md` §2, not re-read
here); the consumers in §4 below read `+4h` as the unit and the drain reads `+0Ch` as the level:

| offset | what |
| --- | --- |
| `+0h` | vptr `00D08E78` |
| `+4h` | the unit |
| `+8h` | the unit's category |
| `+0Ch` | the detection level, `0` / `1` / `2` |
| `+10h`/`+14h`/`+18h` | a nested `{count, head, tail}` member list, group records only |

---

## 3. What writes `[recon + DE8h]`, when, and with what

`+0DE4h` is the enemy triple's `count`, **`+0DE8h` its `head`**, `+0DECh` its `tail`.

### 3.1 The census, and the encoding trap in it

A byte census of every `disp32 == 0xDE8` access in `.text`:

| pattern | matches | recon-slot ones |
| --- | --- | --- |
| `8b ?? e8 0d 00 00` (`mov reg,[base+DE8h]`) | 16 | 13 (see §4) |
| `89 ?? e8 0d 00 00` (`mov [base+DE8h],reg`) | 4 | 3 |
| `8d ?? e4 0d 00 00` (`lea reg,[base+DE4h]`) | 7 | 6 |

The three non-recon matches are a different object at the same offset
(`007D010A` in `BSP_PlaneUnitInstance_Construct`, `004B58A0` in `FUN_004B5800`).

**The trap: the routine that actually publishes the list appears in none of the store rows.**
`00804E10` writes the head as `this->[+4h]` through `ECX`; `008042B0` clears it the same way. The
absolute offset only ever appears when the *slot* register is the base, which happens in the
constructor, the destructor and the one inlined unlink. A scan for `89 ?? e8 0d 00 00` alone would
have concluded that the enemy list has no producer outside the constructor — which is exactly the
conclusion `docs/GAME_EXECUTABLE.md` was left with. The producer set has to be reached through the
`lea ecx,[esi+0DE4h]` rows instead.

### 3.2 The five writers

| # | writer | reached from | effect on `+0DE8h` |
| --- | --- | --- | --- |
| 1 | `008050E0` at `0080519F` | the constructor | zero at birth |
| 2 | `008042B0` (through `ECX`) | `008073F5`, after `008073EF LEA ECX,[ESI+0DE4h]` | count/head/tail := 0 at the top of every rebuild |
| 3 | `00804E10` (through `ECX`) | `00807627`, after `00807621 LEA ECX,[ESI+0DE4h]` | **the publication**: appends a copy node per record |
| 4 | the inlined unlink `008076AB`..`008076DA` | inside `008073C0` | `008076BC MOV [ESI+0DE8h],EDX` when the drained node was the head |
| 5 | `00806ED0` at `008070FF` | the scalar deleting destructor | teardown |

### 3.3 The publication, instruction by instruction

All of it is inside one `008073C0` call, so the field is never observed half-built by another
system on the same thread.

```
008073EF  LEA ECX,[ESI+0DE4h] ; 008073F5 CALL 008042B0     clear: count = head = tail = 0
   ...    the scan, the carry-over splice and the sensor pass run here
00807615  LEA EDI,[ESI+4C0h]                               array B, the enemy class buckets
0080761B  LEA EBX,[EBP+61h]                                97 iterations
00807620  PUSH EDI
00807621  LEA ECX,[ESI+0DE4h]
00807627  CALL 00804E10                                    triple 1 += copy of B[i]
0080762C  ADD EDI,0Ch ; 0080762F SUB EBX,1 ; JNZ 00807620
00807634  MOV EDI,[ESI+0DE8h]                              now drain what was just published
00807644  MOV EBX,[EDI+8]                                  node->payload
00807647  MOV EAX,[EBX+0Ch]                                payload->level
0080764A  CMP EAX,1 ; JL  008076A2                         level 0 -> no copy
0080764F  CMP EAX,2 ; JGE 008076A2                         level 2 -> no copy
00807654  PUSH 0Ch ; CALL 00BF681B                         level 1 -> copy into "unknown"
008076A2  MOV EDX,[EDI+8] ; 008076A5 CMP [EDX+0Ch],2
008076A9  JGE 008076EE                                     level 2 -> the node stays
008076AB..008076DA                                          level < 2 -> unlink and free
```

So the field's value when `008073C0` returns is: **a chain of `0Ch` nodes, one per enemy-relation
record whose detection level is `2` (identified)**, in class-bucket order over the 97 buckets, with
`+0DE4h` holding the surviving count. A level-1 record is published into the `unknown` triple and
removed from `enemy`; a level-0 record is removed with no copy. The own triple is never filtered.

Without the drain every node survives with `payload[+0Ch] == 0` from `00804E70`, which is the
membership-only list the current host stands in for.

### 3.4 When

`008073C0` is called from exactly four sites (`ghidra xrefs`, four rows, below the 25-row cap so
this is a census, not a page):

| site | function |
| --- | --- |
| `00807A08` | `008079B0 BSP_Recon_ServicePeriodicRefresh` — the 3-second countdown, all present slots |
| `004E059B` | `BSP_Game_LoadMissionScene` — once at load, the active player's slot |
| `004C9E82` | `BSP_Game_ApplyInGameInterface` — the local player's slot |
| `007C631C` | `BSP_Plane_PlaceOnLaunchSpotLocked` — the active player's slot |

The consumers in §4 read the field on their own tick, between rebuilds, so the list they see is
always the last completed rebuild's output.

---

## 4. How a consumer walks it

### 4.1 The walk

Every consumer performs the same five steps. From `00864FE0 BSP_UnitGunneryAi_Tick` (Ghidra's
stored body is truncated before this region; the listing is `disasm-raw` over the disk bytes, and
the containing function is `00864FE0` per `ghidra proto`, body `00864FE0`-`008658B8`):

```
008651FE  MOV ECX,[EDI+54h]          the owning unit's party
00865201  CALL 008053C0              that party's slot (built if absent)
00865206  MOV EAX,[EAX+0DE8h]        the enemy head
0086520C  TEST EAX,EAX ; JE 00865442 empty -> nothing to sweep
00865220  MOV EAX,[ESP+18h]          the cursor
00865224  MOV ECX,[EAX+8]            node->payload
00865227  MOV EAX,[ECX+4]            payload->unit
0086522A  LEA EDX,[ESP+2Ch] ; PUSH EDX ; PUSH EAX ; PUSH ESI ; MOV ECX,EBX
00865237  CALL 00863990              score the candidate
0086523C  TEST AL,AL ; JE 0086542F
00865248  CALL 00862440              the AI suppression test
0086525D  CALL 00864D90              the visibility cache
```

`009F5D30 BSP_AutoTarget_ScanPartyList` is identical in shape: `009F5D3A CALL 008053C0`,
`009F5D4E MOV EDI,[EAX+0DE8h]`, `009F5D60 MOV ECX,[EDI+8]`, `009F5D63 MOV ESI,[ECX+4]`.

**The element type is therefore the `0Ch` node, and what the consumer wants out of it is
`node->payload->unit`.** No consumer read of `payload[+0Ch]` was found on either walk: the level is
consumed by the drain, not by the reader. That is the whole point of the drain — the published list
is pre-filtered, so a consumer can treat "in the list" as "identified".

### 4.2 Who walks it

Fifteen functions call `008053C0`. Ten of them read `+0DE8h`; the other five use other triples or
only ensure the slot exists.

| function | reads `+0DE8h` |
| --- | --- |
| `006435D0 BSP_InGameHudMarkersScreen_Update` | `00643CC0` |
| `0070C370 BSP_FlakProjectile_TickAdvance` | `0070C52F` |
| `0080A000 FUN_0080A000` | `0080A5CC` |
| `00856BB0 BSP_TorpedoProjectile_Steer` | `00856C36` |
| `00864FE0 BSP_UnitGunneryAi_Tick` | `00865206` |
| `008C74B0 FUN_008C74B0` | `008C75FE` |
| `009C1FD0 FUN_009C1FD0` | `009C21AC` |
| `009E9190 BSP_ShipAi_ApproachRefreshAvoidance` | `009E9225` |
| `009F5D30 BSP_AutoTarget_ScanPartyList` | `009F5D4E` |
| `009F7810 FUN_009F7810` | `009F7846` |
| `004C9CA0`, `004DFB70`, `006F4D10`, `008FFF20`, `009F1420` | no `+0DE8h` read |

Three more reach the same field without calling `008053C0`, through the **cached** slot pointer the
local player's slot record keeps at `+30h` (`[game+18ECh]` selects the record inside
`[game+18CCh + i*4]`):

| function | resolution | read |
| --- | --- | --- |
| `004C3CB0 BSP_Game_BuildLocalPlayerUnitLists` | `[[game+18CCh+i*4]+30h]` | `004C3D61` |
| `005B2D70 FUN_005B2D70` | `005B319F`..`005B31B1` | `005B31B4` |
| `007A84D0 FUN_007A84D0` | `007A84EA`..`007A84FD` | `007A852D` |

So the enemy contact list is not a gunnery-private structure: the HUD markers, flak, torpedo
steering, ship-AI avoidance and the auto-target scan all read the same chain. Anything the host
publishes there is seen by all of them.

### 4.3 `00793670`, the accessor that proves the element type

`00793670`, `__thiscall int(ReconSlot* this, Unit* unit)`, `RET 4`, body `00793670`-`007936BB`
(`RET 4` at `007936B9`), read in full. Sole caller `007A85DE` inside `007A84D0` (`ghidra xrefs`,
one row).

```
00793673  MOV EDI,[ECX+0DE4h]        count
0079367D  JLE 007936B3               empty -> return -1
0079367F  MOV EBX,[ESP+10h]          the unit being looked for
0079368B  MOV EDX,[ECX+0DE8h]        head
00793697  CMP EAX,ESI ; JZ 007936BC  walk `index` links
0079369B  MOV EDX,[EDX+4]            node = node->next
007936C0  MOV EDX,[EDX+8]            node->payload
007936A7  CMP [EDX+4],EBX            payload->unit == the argument ?
007936AA  JZ 007936B6                yes -> return the index
007936AC  ADD EAX,1 ; CMP EAX,EDI ; JL 00793683
007936B3  OR EAX,0FFFFFFFFh          not found -> -1
```

It answers "where in the published enemy list is this unit", returning the **ordinal** or `-1`. It
is a quadratic re-walk per index, which is why `+0DE4h`'s count matters to it.

`007A84D0` is the caller: when `[this+38Ch]` is zero it takes the active player's slot and picks
the first non-empty published triple, own (`+0DD8h`/`+0DDCh`) before enemy (`+0DE4h`/`+0DE8h`),
then asks `00793670` for a position — a cursor over the published lists, most likely the HUD's
target cycler. Its tail was not read, so this is a partial reading of `007A84D0`
(`007A84D0`-`007A853F` read; `007A8540`-end not read).

---

## 5. Buckets, sides and observers

**A bucket is one `{count, head, tail}` list and its index is the vehicle class id.** The
constructor builds `61h = 97` of them per array, the arrays are indexed directly, and
`008073C0` addresses `A[i]` as `ESI + 34h + i*0Ch` (`00807529 LEA EDI,[ESI+34h]`, `0080753F ADD
EDI,0Ch`, 97 iterations). Nothing hashes; the class id **is** the subscript.

| array | offset | relation | filled by |
| --- | --- | --- | --- |
| A | `+34h` | own | `008065B0` |
| B | `+4C0h` | enemy | `008065B0` |
| C | `+94Ch` | neutral | `008065B0` |
| D | `+0E14h` | carry-over from the previous pass | `00804150` at `0080743F`/`00807447`, retired by `00805430` |

Only 24 of the 97 subscripts are ever occupied: the twenty-two scanned class ids from
`00806480`'s first vector, plus the two aggregate ids `18h` (`PlaneSquadronGen`) and `1Ah`
(`LandConvoy`) the grouping passes create. The scan reaches subscript `46h` at most. **97 is a
capacity, and where the number comes from is not proven here** — no producer of the world
registry's class table was read, so the claim "97 is the vehicle-class count" is not made.

The scan proves the bucket index and the class list in one loop (`0080749C`..`00807527`):

```
008074B0  CALL 00806480              the singleton
008074B5  MOV EDX,[EAX+4]            its id array
008074B8  MOV EBX,[EDX+EDI*4]        classId = ids[i]
008074BB  MOV ECX,[00E188A8] ; 008074C1 MOV EDX,[ECX+19CCh]     the world registry
008074C7  LEA EAX,[EBX+EBX*2] ; 008074CA MOV EBP,[EDX+EAX*4+1Ch]  registry list for classId
008074D5..008074FA  the four gate bytes and IsKindOf(2)
008074FD  PUSH EDI ; PUSH EBX ; 00807500 CALL 008065B0            add or refresh in bucket classId
00807510  PUSH EBX ; 00807513 CALL 00805430                       retire D[classId]
00807524  CMP EDI,[EAX+8]            the singleton's count
```

**Sides.** Three slots, subscripted by party index: `0` and `1` the belligerents, `2` neutral.
`[slot+28h]` holds the party. `008065B0` derives a unit's relation from `slot->+28h` against
`unit->+54h` with the rule `docs/RECON_SLOT_LISTS.md` §1(c) states; slot 2 never classifies
anything as enemy, so **the enemy triple of slot 2 is always empty** and only slots 0 and 1 ever
publish a contact list worth reading.

**Observers.** The slot's own triple, `+0DD8h`, is the observer list. `00807529`..`00807545` fills
it with a copy of every own bucket *before* any grouping, then `00807552`/`00807570` pass it as the
second argument to `00806840` for each enemy and each neutral bucket. So:

* **observer** = any unit in the slot's own relation, i.e. any live unit of the same party in a
  scanned class — the player's own ships, planes and land vehicles, individually, not their
  squadron aggregates;
* **slot** = the party;
* **bucket** = the class within the party's three relation arrays.

Every enemy record is evaluated against every own record, so the pass is `|own| x |enemy|` sensor
row evaluations per rebuild, capped by the early-out inside `00806840`
(`008068C2 PUSH 5 ; CALL [observer->vtable+5Ch]`, only a unit-base entry observes).

---

## 6. The host prescription

What `src/game_hosts_gunnery.cpp` needs so `recon_contact_count_008053c0` returns the real list.
Everything the rule needs is already published; the missing piece is the **object and the tick
step**, not the arithmetic.

### 6.1 The object

One per party, three of them, matching `00F874BC[0..2]`. It does **not** need the native's
`0x12A0` bytes, only the members the rebuild and the consumers touch:

```
struct ReconSlotState {                 // 008050E0
    int   party;                        // +28h
    float last_refresh_time;            // +2Ch, 00F876A4 at construction
    float elapsed_seconds;              // +30h, the sensor dt
    bool  contents_changed;             // +24h
    bool  publish_dirty;                // +25h, true at birth
    std::array<std::vector<Entry*>, 4> arrays; // A/B/C/D, each 97 buckets, class id = subscript
    std::array<std::vector<Entry*>, 5> triples; // own/enemy/neutral/unknown/all
};
struct ReconEntry {                     // 00804E70, 1Ch
    void* unit;                         // +4h
    int   category;                     // +8h
    int   level;                        // +0Ch, ReconDetectionLevel
};
```

`bsp/fixed_step_countdown.hpp` already declares `kReconSlotTableAddress` (`00F874BC`),
`kReconSlotCount` (3), `kReconSlotSize` (`0x12A0`), `kReconSlotIndexOffset` (`0x28`),
`kReconSlotDirtyOffset` (`0x25`) and `kReconSlotArrayOffsets/Length/Stride`;
`bsp/recon_slot_lists.hpp` adds the fourth array, the five triple offsets,
`kReconEnemyChainHeadOffset` (`0x0DE8`), the relation rule, the gate rule, the class-id vectors,
the detection rules and `recon_triple_filter_00807647`. **Nothing new needs to be declared for the
rule.** The `{count, head, tail}` / `{prev, next, payload}` strides and the `1Ch` record's field
offsets from §2 are the only constants this packet found that no header carries; they belong in
`bsp/recon_slot_lists.hpp` next to `kReconTripleOffsets`, and only matter to a host that models the
native chain rather than a `std::vector`.

### 6.2 The tick step, in native order

`bsp/recon_slot_lists.hpp` already declares the whole rebuild as a pure-virtual host interface
(`move_relation_list_into_carry_over_00804150`, `scanned_class_ids_00806480`,
`world_units_of_class_008074ca`, `unit_gate_bytes_008074d5`, `add_or_refresh_unit_008065b0`,
`run_sensor_pass_00806840`, `group_members_00805490`/`00805680`,
`append_class_list_to_triple_00804e10`, `drain_triple_to_unknown_00807634`, ...). The prescription
is to implement it and call it on the right cadence:

| step | native | what the host does |
| --- | --- | --- |
| 0 | `00807A08` in `008079B0` | every **3 s** of sim time, for each of the three slots. Not per frame: the sensor `dt` at `+30h` is the interval, and a per-frame rebuild changes the gain integration |
| 1 | `008073D6`, `008073E1`, `008073E6` | `elapsed = now - last_refresh; last_refresh = now; contents_changed = false` |
| 2 | `008073EA`..`00807416` | clear all five triples |
| 3 | `0080741B`..`00807460` | splice A, B, C into D for all 97 buckets |
| 4 | `00807462`..`0080749A` | reset this slot's detection record on every world unit (`00805BE0`) |
| 5 | `0080749C`..`00807527` | for each of `kReconScannedClassIds`: walk the class list, apply `recon_unit_gate_passes_008074d5` and `IsKindOf(2)`, `008065B0` into A/B/C by `recon_relation_for_008065ff`; own units are forced to full detection; then retire D[classId] |
| 6 | `00807529`..`00807545` | own triple := copy of every A bucket — **this is the observer list** |
| 7 | `00807547`..`0080757F` | for each B then each C bucket, run the sensor pass with the own triple as observers: per record, best gain over observers via `recon_evaluate_sensor_row_008048a0`, then `recon_detection_accumulate_00805af0`, then `recon_detection_level_00805b3d`, then `recon_effective_level_0080695c` into `entry->level` |
| 8 | `00807581`..`00807610` | enemy grouping into `18h`/`1Ah` |
| 9 | `00807615`..`00807632` | **enemy triple := copy of every B bucket** — the publication |
| 10 | `00807634`..`008076F3` | drain by `recon_triple_filter_00807647(level)`: level 1 copied to unknown and removed, level 0 removed, level 2 kept |
| 11 | `008076F9`..`00807871` | the same three steps for neutral |
| 12 | `00807877`..`00807966` | own grouping, own triple additions, triple 4 |

Then `recon_contact_count_008053c0()` becomes `slots[unit_side_0054(unit_)].triples[enemy].size()`
and `recon_contact(i)` becomes `handle(entry->unit)` — the two host methods keep their signatures,
which is why no change is needed in `src/unit_gunnery_pass.cpp`. The sweep loop there
(`src/unit_gunnery_pass.cpp:394`, native `008651D7`..`00865284`) is already an exact mirror of the
native walk in §4.1.

### 6.3 Two corrections the stand-in needs regardless of rule (c)

`src/game_hosts_gunnery.cpp:738`'s stand-in admits a unit when its side differs from the owner's,
it is alive and visible, and it is `IsKindOf(ship base)`. Against `008065B0`'s relation rule that
is wrong in two ways that have nothing to do with detection:

1. **Party 2 is neutral, not enemy.** `recon_relation_for_008065ff(slot, party)` sends every
   party-2 unit to the neutral array, never to B. A `side != own_side` test counts them as enemies.
   The size of this error depends on how many party-2 units the mission has and could be zero.
2. **Individual planes and land vehicles.** They enter B under their own class ids and are then
   folded into `18h`/`1Ah` by the grouping, but the grouping does not remove the member records, so
   both the members and the aggregate are in the enemy triple. A host that publishes only units, or
   only aggregates, will not match the count either way. (The Lua publish `00805D90` *does* drop
   the members — that exclusion is script-side only and does not apply to `+0DE8h`.)

---

## 7. What should happen to `candidates`

### 7.1 The arithmetic says the filter is a hard range test, not a ramp

From `docs/SENSOR_TABLE_DATA.md`'s authored rows on **this installation** (arcade variant, because
`gamemode.lua` sets `GameMode = 0`; `scripts/datatables/classtables/arcade/reconclasses.lua`,
mtime `2024-07-13 08:26`; the installation is modded, BSPRM/AlterBSP, and that file carries the
timestamp of the untouched bulk) and `docs/GUNNERY_RECON_DETECTION.md`'s rule:

* almost every authored `Gain` is `1`, which the producer stores as `+8h = Gain * 0.5 = 0.5`;
* the rebuild period is 3 s, so one pass contributes `0.5 * 3.0 = 1.5`;
* a vision row's `MaxLevel = 2` maps to a cap of `1.0f`, and the identify threshold is `0.5f`.

`1.5` clamped to `cap - current` saturates a fresh record at `1.0` in **one pass**. So there is no
ramp-up: a unit that comes inside a vision row's `Dist` is `identified` on the very rebuild that
first sees it, and a unit outside every row's `Dist` stays at `0` and is deleted from the triple.
Rule (c) degenerates, for the arcade table, into:

> the enemy list is the set of enemy-relation units within some own-relation observer's authored
> `Dist * sqrt(rangeScale)`, measured horizontally, and inside the bearing window when the row
> authors an `Angle`.

The one refinement that matters: **radar rows do not qualify a unit for the enemy list.** The radar
ship's `Surface -> Surface` radar row has `Dist = 6000` but `MaxLevel = 1`, a cap of `0.25f`, and
`0.25` is `blip`, not `identified` (`recon_detection_level_00805b3d`: `< 0.5` is `blip`). So a
contact between the 4000 m vision ring and the 6000 m radar ring saturates at `0.25`, the drain
copies it into `unknown` and removes it from `enemy`, and the gunnery sweep never sees it. The
gating distance for gunnery is the **vision** row, `Dist = 4000` arcade.

### 7.2 The prediction

**`candidates` should collapse early in the run and then recover almost completely, and the run
total should fall by roughly the fraction of the run the fleets spend more than ~4 km apart —
not by a fraction that reflects how many units exist.**

The reasoning, in the order it constrains the answer:

1. The sweep's per-call contact count goes from "every live enemy unit" to "every live enemy unit
   within 4000 m of *any* own-party observer". It is a union over all observers, and a task force
   is a formation a few hundred metres across, so the union's envelope is barely larger than one
   ship's. This is a **fleet-to-fleet range test**, effectively.
2. While the fleets are apart, the list is **empty** and `candidates` for that stretch is exactly
   **zero** — not reduced, zero. `recon_sweeps` stays where it is, because the sweep still runs and
   still calls the host; only the inner loop has nothing to iterate.
3. Once the gap closes below 4 km, formations being what they are, **most of the opposing fleet
   crosses the threshold within a few rebuilds of each other**, and the list jumps to near the
   stand-in's membership. From then on `candidates` per sweep is close to today's.
4. `docs/GAME_EXECUTABLE.md`'s milestone 2s notes that on USN02 the nearest enemy pair is 1854 m
   apart at 25 s — already well inside 4000 m. So on that mission the closing happens early and the
   zero stretch is short. **The honest prediction for that run is that `candidates` moves
   comparatively little** — a reduction concentrated in the opening seconds, plus whatever stragglers
   (a detached submarine, a distant airfield at `Dist` 0, the `ReconClass[12]` never-detects class)
   stay outside the ring for the whole run.
5. Two effects push in the same direction but are small and separable: party-2 units leaving the
   enemy list (§6.3.1), and any unit whose observer-side class authors no row reaching it.

**What to measure, so the prediction is falsifiable.** Per sweep, log the contact count together
with the minimum own-to-enemy horizontal distance. The prediction is wrong if the contact count is
a smooth function of time rather than a step at the 4000 m crossing; it is wrong in the other
direction if the count stays at zero after the fleets close, which would mean the observer side of
`00806840` is not finding rows. A run total alone cannot distinguish "the filter works" from "the
filter admits nothing", so the per-sweep pair is the measurement, not the total.

**Counter-prediction worth stating.** If USN02's opposing units are placed inside 4 km from frame
0, `candidates` will barely move at all and the only visible change will be the party-2 and
straggler effects. That outcome does not mean rule (c) is unimplemented; it means this mission
does not exercise it. A mission that opens at 8-10 km, or an air mission where the 4000 m ring is
crossed at speed, is the one that shows the difference.

---

## 8. Proven versus assumed

**Proven from a producer, in this packet**

* The object's size (`008053E4 PUSH 12A0h`), its owner (`00F874BC`, six-site census), its lazy
  construction (`008053E9`/`00805404`/`0080540D`) and its construction site `008050E0` read in full.
* Four `97 x 0Ch` arrays and five triples, from the constructor's own stores.
* `{count, head, tail}` and `{prev, next, payload}`, from `00804E10` read in full and confirmed
  independently at `+0DFCh` (`00807671`..`0080769B`) and `+0DD8h` (`007A8500`/`007A8514`).
* The five writers of `+0DE8h`, and that a disp32 byte census misses the publishing one.
* The publication and drain, `00807615`..`008076F3`, instruction by instruction.
* The consumer walk in `00864FE0` (`008651FE`..`0086525D`) and `009F5D30`
  (`009F5D3A`..`009F5D63`).
* `00793670` read in full; its sole caller `007A85DE` from `ghidra xrefs` (one row).
* The four callers of `008073C0` from `ghidra xrefs` (four rows, well under the 25-row cap).
* The bucket subscript, from `00807529`/`0080753F` and the scan loop's
  `008074B8 MOV EBX,[EDX+EDI*4]` / `00807500 PUSH EBX`.

**Negative results, stated as such**

* **`00862CD0` is not a contact-list consumer.** The packet brief names it as "the engagement
  list". Its body (read from the decompiler, `00862CD0` onward) touches no recon slot, no
  `+0DE8h` and no `008053C0`: it updates one engagement record's accuracy from the `00E19994`
  table using a distance ratio over `[target+490h]` and the times at `record+80h`/`+84h`. The name
  in `docs/GAME_EXECUTABLE.md` milestone 2s is the **host method** name
  (`update_target_records_00862cd0`, `src/game_hosts_gunnery.cpp:712`), not a claim about the
  native's relationship to the contact list. Checked against the producer account: it neither
  confirms nor contradicts it, because they do not touch.
* No consumer read of `payload[+0Ch]` (the level) was found on either walk examined. That is a
  negative result over two walks, not over all thirteen readers.

**Assumed, or inherited as a contract**

* `00804E70`'s `1Ch` record layout is taken from `docs/RECON_SLOT_LISTS.md` §2; this packet read
  the consumers' use of `+4h` and the drain's use of `+0Ch`, not the constructor.
* `008042B0`'s body (the triple clear) is inherited; only its call sites and `ECX` setup were read.
* The `+8h` gain, `+0Ch` cap and threshold arithmetic in §7 are inherited from
  `docs/GUNNERY_RECON_DETECTION.md` and `docs/SENSOR_TABLE_DATA.md` and were not re-derived.
* The eight `+0DE8h` readers listed in §4.2 that are not `00864FE0` or `009F5D30` were identified
  by the byte census plus their membership in the `008053C0` caller list. Their walks were **not**
  read, so "they walk the same chain the same way" is an inference from the offset, not a reading.
* `007A84D0` is read only as far as `007A853F`; its purpose ("a cursor over the published lists")
  is a hypothesis.
* Where `61h` comes from is not established.

**Coverage**

| routine | coverage |
| --- | --- |
| `008053C0` | complete |
| `008050E0` | complete |
| `00804E10` | complete |
| `00793670` | complete |
| `008073C0` | partial: `008073D0`-`0080757F` and `00807615`-`008076F3` read here; the rest inherited from `docs/RECON_SLOT_LISTS.md` §3 |
| `00864FE0` | partial: `008651EE`-`00865264` only |
| `009F5D30` | partial: `009F5D3A`-`009F5D66` only |
| `007A84D0` | partial: `007A84D0`-`007A853F`; `007A8540`-end not read |
| `00862CD0` | partial: the head of the body, enough to establish the negative result |

---

## 9. `no_ghidra_function`

None. Every address above lies inside a Ghidra function body. Two regions were read with
`disasm-raw` over the disk bytes because Ghidra's **stored body is truncated**, not absent:

| region read raw | containing Ghidra function | note |
| --- | --- | --- |
| `008651EE`-`00865264` | `00864FE0 BSP_UnitGunneryAi_Tick`, body `00864FE0`-`008658B8` | `ghidra disasm --start` refuses these addresses as "not an instruction start inside the stored body"; the bytes disassemble cleanly from `008651F8` onward and the first two lines above are resync noise |
| `007A84D0`-`007A853F` | `007A84D0 FUN_007A84D0` | same |

A gap exists inside `008073C0` itself at `00807428`-`0080742F` (8 bytes Ghidra did not
disassemble); it is between the `JMP 00807426 -> 00807430` and the loop head, i.e. alignment
padding, and nothing above depends on it.

---

## 10. Follow-up packets

1. **`recon_enemy_list_consumers`.** Read the eight unexamined `+0DE8h` walks
   (`006435D0`, `0070C370`, `0080A000`, `00856BB0`, `008C74B0`, `009C1FD0`, `009E9190`,
   `009F7810`). Two of them, the HUD markers and flak, decide what the player *sees*; if the host
   ever publishes a real list they are the visible regression surface. `0080A000`, `008C74B0`,
   `009C1FD0` and `009F7810` are unnamed and worth naming.
2. **`recon_slot_destruction`.** `00805240 BSP_Recon_DestroySlot`, `00806ED0` and `008034E0`: what
   happens to the `1Ch` records and to `00694A60`'s registered observer pairs when a slot dies.
   Needed before a host models the object's lifetime rather than just its contents.
3. **`recon_player_slot_cache`.** `[game+18CCh + i*4] + 30h`, the cached slot pointer three
   consumers use instead of `008053C0`, and `[game+18ECh]`, the active index. `004E0575`,
   `004C9E5E`, `005B319F`, `007A84EA`, `004C3D50`.
4. **`recon_hud_target_cursor`.** `007A84D0` in full, with `00793670` as its helper.
5. **`world_class_registry`.** `[game+19CCh] + 18h + id*0Ch` and where `61h` comes from. The
   registry is read by `008073C0` step 5 and by 278 other sites; it is the last unproven number in
   the slot's shape.
