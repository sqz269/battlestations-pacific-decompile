# Part damage, fire and flooding: the geometry element producer and the one gate that blocks all three

Addresses: 00727310 00727A90 007149D0 00726D80 00727260 00E08138 00CFDBD8 00CFDBC8 007268D0
0082203D 0080FA50 0093A470 0093A4F0 00939F90 00939FA0 00983780 009832F0 0088E320 0092D1F0
00723D60 00937C90 007135C0 0087BCC0 0087CA80

Packet `cc7_part_damage_reachability`, **read-only in Ghidra** (no renames, comments, prototypes or
saves). Every descriptive name below is a hypothesis, not a recovered symbol.
`docs/SHIP_HIT_RECORD.md`'s rules R1..R12, `docs/HIT_NARROWPHASE.md`'s record table,
`docs/HIT_HULL_SEGMENT.md`'s two chains and `docs/UNIT_PARTS.md`'s controller layout are cited, not
restated. This installation is **modded** (BSPRM/AlterBSP); every data file read is listed with its
mtime in §6.

## 0. The answer, in one paragraph

`docs/HIT_HULL_SEGMENT.md` left one open item: nothing was found that *writes* the geometry
element's `+4h` (the kind) or `+8h` (the segment index). **Both writers are at `007273DB` and
`007273DF`, inside `00727310`, the `GeomMesh` resource payload parser.** `element+4h` is the index
of the element's authored *name* in the fifteen-entry table at `00E08138`, and `0Dh` — the value
rule R4 gates on — is the name **`fizika`**. `element+8h` is a raw `u32` read from the resource
stream right after that name. The data exists in this installation: `Farragut_1934.MMOD` carries a
`GeomMesh` chunk whose seven elements are `{body 0, underwater 0, fizika 0, magazine 0, fizika 1,
engineroom 0, fizika 2}`. Separately, the run evidence shows `part`, `fires` and `floods` are **not
three problems but one**: `00826F62` (R1) skips `00826F6B..0082757B` whole when `hit+34h == -1`, so
the same missing segment index blocks the part arm, the flooding arm and the fire arm together.

## 1. The geometry element producer

### 1a. Where the parser is

`docs/GAME_RESOURCE_PARSER_REGISTRATION.md` row 4 records the `GeomMesh` parser singleton
`00716FE0`, primary vtable `00CFD800`, name getter `007258F0`, parse slot `00727A90`. That packet
states it "does not establish the ... `GeomMesh` ... payload format". This one does.

| routine | ABI (from the cleanup) | what it does |
| --- | --- | --- |
| `00727A90` | `__thiscall(parser /*ECX*/; NodeHandle*) -> GeomMesh*`, `RET 4` at `00727AED` | `operator new(0x50)` at `00727AA7`/`00727AA9`, `007268D0` constructs, `00727AD7` calls `00727310` with `ECX` = the new object and the node handle pushed at `00727ACC` |
| `007268D0` | `__thiscall(mesh /*ECX*/) -> mesh` | `BSP_ResourceItem_ConstructReferenceBase`, vtable `00CFDBD8`, then zeroes `+0Ch/+10h/+14h`, `+1Ch/+20h/+24h` and `+2Ch/+30h/+34h` — three vectors |
| `00727310` | `__thiscall(mesh /*ECX*/; NodeHandle*)`, `RET 4` at `00727357` | **the payload parser**; body `00727310`-onwards, `coverage: partial` (see §1e) |
| `00726D80` | `__thiscall(vec /*ECX*/; int n)` | element-vector reserve, called at `00727370` with `ECX = mesh+8h` |
| `00727260` | `__thiscall(vec /*ECX*/; const Element*)` | element-vector `push_back`; the `0x2E8BA2E9` / `SAR EDX,3` magic at `00727277`/`0072727E` is division by `0x2C`, so the stride is `2Ch` |

`00727260`'s `this` is `mesh+8h` (`00727363 ADD ESI,0x8`) and it reads begin at `[ESI+4]`, end at
`[ESI+8]`, capacity at `[ESI+0Ch]` — that is `mesh+0Ch/+10h/+14h`, exactly the vector
`docs/HIT_HULL_SEGMENT.md` says `00723D60` walks with stride `2Ch`. The `0x50`-byte object built
here is therefore the geometry the unit-part collision shape traces against.

### 1b. The two writes the previous packet could not find

The element is assembled on `00727310`'s frame at `[ESP+0x4C]` and copied in by `00727260`. `ESP` at
these instructions is `ESP0-0x84` (`PUSH -1`, `PUSH`, `PUSH`, `SUB ESP,0x68`, then `PUSH EBP/ESI/
EBX/EDI`), so `[ESP+0x4C]` is `ESP0-0x38`:

```
007273bf: MOV dword ptr [ESP + 0x4c],0xcfdbc8   ; element+0h  = the element vtable
007273db: MOV dword ptr [ESP + 0x50],EDI        ; element+4h  = the kind      <- writer 1
007273df: MOV dword ptr [ESP + 0x54],EAX        ; element+8h  = the index     <- writer 2
007273e3: MOV dword ptr [ESP + 0x58],ECX        ; element+0Ch = the owning mesh
007273c7..007273d7, 007273e7                    ; element+14h..+28h all EBX = 0
007273eb: LEA EDX,[ESP + 0x4c]
007273fa: CALL 0x00727260                       ; push_back
```

Register provenance, by filtering the whole listing rather than one idiom:

* `EDI` at `007273DB` — written once in the loop body, at `007273AA MOV EDI,EAX` from
  `007273A3 CALL 0x007149d0`, then conditionally at `007273B6 MOV EDI,0x9` under
  `007273B1 CMP EDI,0x7`. Nothing else writes `EDI` between those and the store.
* `EAX` at `007273DF` — `007273AC CALL 0x00be9a00` with `ECX = EBP`; `EBP` is the node handle,
  loaded once at `00727329 MOV EBP,[ESP+0x7c]` (= `ESP0+4`, argument 1) and never rewritten.
  `00BE9A00` is `BSP_StructuredNode_ReadU32`. So **`element+8h` is a raw authored `u32`**.
* `ECX` at `007273E3` — `007273BB MOV ECX,[ESP+0x10]` = `ESP0-0x74`, the slot `00727332` filled
  with `ESI` = the incoming `this`. So `element+0Ch` is the owning mesh, which is why
  `docs/HIT_HULL_SEGMENT.md` found the vertex and triangle vectors through it.

`element+0Ch`'s meaning is corroborated independently: `007268D0` zeroes `+1Ch/+20h/+24h` and
`+2Ch/+30h/+34h` on the `0x50` object, and `docs/HIT_HULL_SEGMENT.md` reads the owning mesh's
vertices at `[+1Ch,+20h)` (stride `0Ch`) and triangles at `[+2Ch,+30h)` (stride `6`).

### 1c. `007149D0`: the kind is an authored name

`007149D0` takes the name in `ECX`, walks the NULL-terminated pointer table at `00E08138` (Ghidra:
`PTR_s_lwing_00e08138`), compares with `00438E10 BSP_CString_CompareInsensitive` and returns the
index of the first match, or `-1` when it reaches the null at `00E08174`. The table is fifteen
entries:

| index | name | address | index | name | address |
| --- | --- | --- | --- | --- | --- |
| 0 | `lwing` | `00CE4454` | 8 | `magazine` | `00CE43FC` |
| 1 | `rwing` | `00CE444C` | 9 | `body` | `00CE43F4` |
| 2 | `fuselage` | `00CE4440` | **10** | **`none`** | `00CE43EC` |
| 3 | `engine` | `00CE4438` | 11 | `runway` | `00CE43E4` |
| 4 | `underwater` | `00CE442C` | 12 | `hangar` | `00CE43DC` |
| 5 | `engineroom` | `00CE4420` | **13** | **`fizika`** | `00CE43D4` |
| 6 | `fueltank` | `00CE4414` | 14 | `bullet` | `00CE43CC` |
| 7 | `steering` | `00CE4408` | | | |

Two rows settle the whole chain:

* **`0Dh` = `fizika`.** Rule R4's gate `hit+30h == 0Dh` is "the shot hit an element of the ship's
  *physics* hull mesh". `docs/UNIT_PARTS.md` already records that `00937C90` looks model nodes up by
  the format string `"fizika_%02d"` (`00CEB90C`, pushed at `0093804E`) and emits the Hungarian
  diagnostic `"Hajodarabnak nincs utkozoje:fizika_%d, kb %d. db"`. The collision elements and the
  model nodes use the same word because they are the same part set.
* **`0Ah` = `none`.** `docs/HIT_NARROWPHASE.md` records that all three hard-coded shapes
  (`0087FEC0`, `0087FF80`, `00929B80`) write `0Ah` into `record+30h`. That is not a magic number:
  those shapes are authored as *no category*, which is why they can never satisfy R4.

The parser applies one remap the Lua reader does not:
`007273B1 CMP EDI,0x7 / 007273B4 JNZ / 007273B6 MOV EDI,0x9` — a collision element named
`steering` is filed as `body`.

### 1d. The same pair on the Lua side (independent corroboration)

`007149D0`'s only other caller is `0087CA80 BSP_DamageableClass_ReadLuaFields`, at `0087CF7B`. In
that loop `ESI` steps by `-0x30` (`0087CEAE ADD ESI,-0x30`), so the record is the `30h`-byte part
descriptor of `docs/UNIT_PARTS.md`, and:

```
0087cebb: PUSH 0xd0e190                    ; the Lua key "MshCategory"
0087cf07: MOV EBX,dword ptr [ESP + 0x1c]   ; its characters
0087cf35: PUSH 0xce55b4                    ; the Lua key "Index"
0087cf57: CALL 0x00bf7420                  ; -> integer
0087cf60: MOV dword ptr [ESI + 0x8],EAX    ; descriptor+8h  = Index
0087cf7b: CALL 0x007149d0                  ; ECX = the MshCategory characters
0087cf8e: MOV dword ptr [ESI + 0x4],EAX    ; descriptor+4h  = the kind
```

The authored part descriptor and the authored collision element carry the **same `(kind at +4h,
index at +8h)` pair**, which is exactly the pair `00723F62`/`00723F6C` copies into the hit record's
`+30h`/`+34h`. Whether the Lua `Index` and the geometry `element+8h` share one index space is
**not** established here.

### 1e. The `GeomMesh` payload format

Read from `00727310`'s listing; `00BE9A00` is `ReadU32`, `00BEA010` is
`BSP_StructuredNodeHandle_ReadString`, `00BE99D0` reads a float and `00BE9A80` a `u16`.

```
00727336  if (00be9b20(node) < 4) { 00be9c40(node); return; }   ; too small to hold a count
0072735c  u32  elementCount                 -> 00726d80(mesh+8h, elementCount)
          repeat elementCount:
0072738a    string name                     ; u32 length then the characters, no NUL
007273a3    kind  = 007149d0(name); if (kind == 7) kind = 9
007273ac    u32   index
007273fa    push_back { vtable 00CFDBC8, kind, index, mesh, 0... }
00727471  u32  vertexCount                  -> 00725b40(mesh+18h, vertexCount)
          repeat vertexCount: three floats (00be99d0 x3) -> 004215d0(mesh+18h, &v)
007274c4  u32  n2 ; repeat n2: two u16 reads (00be9a80 x2)
007274ef  u32  triangleCount
00727504    per element i: 00725aa0(element[i]+10h, triangleCount / elementCount)
00727563    007262a0(mesh+28h, triangleCount)
00727580..  the triangle loop (u16 reads, per-element index lists)
```

`coverage: partial` — everything from `00727580` to the function's end is the triangle decode and
was read only far enough to see that it fills `element+10h`'s vector (begin `+14h`, end `+18h`),
which is the list `00723AA0` and `00723B70` walk. The `n2` section at `007274C4` and the element
fields `+20h`/`+24h`/`+28h` were **not** decoded.

One observation that does not agree with `docs/HIT_HULL_SEGMENT.md` and is **not** resolved here:
that doc reads `element+24h` as "a scalar `0085BF90` turns into the squared-radius reject". The
parser zeroes `+20h`, `+24h` and `+28h` at construction (`007273CF`..`007273E7`) and the temporary's
teardown frees `[ESP+0x6C]` (= `+20h`) and `[ESP+0x70]` (= `+24h`) through `00BF6989` at
`00727410`/`00727421`, which makes both heap pointers rather than scalars. `00723B70` and
`0085BF90` were not read in this packet, so this is recorded as an open item, not a correction.

### 1f. The authored data confirms it

`models/ships/us/Farragut_1934.MMOD`, `GeomMesh` chunk at file offset `0x1587E3`
(`08 00 00 00 "GeomMesh"` counted tag, then payload size `0x1C5BA`):

```
07 00 00 00                                      elementCount = 7
04 00 00 00 "body"        00 00 00 00            kind 9  index 0
0a 00 00 00 "underwater"  00 00 00 00            kind 4  index 0
06 00 00 00 "fizika"      00 00 00 00            kind 13 index 0
08 00 00 00 "magazine"    00 00 00 00            kind 8  index 0
06 00 00 00 "fizika"      01 00 00 00            kind 13 index 1
0a 00 00 00 "engineroom"  00 00 00 00            kind 5  index 0
06 00 00 00 "fizika"      02 00 00 00            kind 13 index 2
72 05 00 00                                      vertexCount = 1394
```

and a second chunk at `0xEEB3F7` with four elements `{body 1, underwater 1, fueltank 0, fizika 3}`
and `vertexCount = 630`. `North_Carolina.mmod` carries 46 `fizika` strings. So a real ship authors
`fizika` elements with indices `0, 1, 2, 3, ...`, which is precisely the input R4 wants.

## 2. `0092D1F0` and the object at `unit+1018h`

`docs/UNIT_PARTS.md` already corrects `unit+1018h` from "a breakable-parts object" to **the unit
motion controller**, allocated `390h` bytes at `0080DED7`/`0080DEDE` and constructed by `00939CB0`,
and lists `controller+310h/+314h/+318h` (per-part health) and `controller+34Ch + i` (the signed
group-slot byte). `0092D1F0`'s existing ledger record already states its body. Neither is restated.

What this packet adds is the **count**, from `00937C90`:

```
00937fef: ADD ESI,0x30c                ; the health vector object, begin at +310h
00937ff5: PUSH 0x14
00937ff9: CALL 0x004a8f10              ; resize to 20 with 0.0f
...
00938042: (loop head)  EBP = [ESP+0x20]
0093804e: PUSH 0xceb90c                ; "fizika_%02d", formatted with EBP
0093813a: MOV byte ptr [EDI + EBP*1 + 0x34c],0xff     ; -1 first
009381b5: MOV byte ptr [EDI + EBP*1 + 0x34c],AL       ; then the running counter, if nodes were found
00938d88: CMP EAX,0x14
00938d8f: JL 0x00938042                ; the loop runs indices 0..19
...
009383a9: CVTSI2SS XMM0,[ESP+0x18]     ; how many fizika_NN nodes actually resolved
009383c0..0093841b:  for (i = 0; i < 0x14; ++i)
                       health[i] = [[controller+1Ch]+538h]+48h / (float)thatCount
```

So **a unit has exactly 20 hull segments, indices `0..19`**, `controller+310h` has 20 floats, and
each starts at the vehicle class's `+48h` divided by the number of `fizika_NN` model nodes the model
actually carries. An `element+8h` outside `0..19` fails `0092D1F0`'s bounds checks; an index whose
`fizika_NN` node is absent leaves `controller+34Ch+index` at `-1` and `0092D1F0` returns at
`0092D210` without touching anything.

`0092D1F0`'s `dir` argument is never read (three arguments on the cleanup, two used) — already in
its ledger record.

## 3. The fire and flood arms, end to end

`docs/UNIT_FIRE_AND_REPAIR.md` records the repair task at `unit+A20h` and marks the message `9Eh`
handler **unread**, which left "which timer is fire and which is water" provisional. That handler is
read here.

### 3a. The producer

`0080FA50` `__thiscall(msg /*ECX*/; int selector, float value, bool flag) -> msg`, `RET 0Ch` at
`0080FA8F`: `0080FA51 PUSH 0x9E` into `0075B430`, then `msg+1Ch = value` (`0080FA84`),
`msg+20h = flag` (`0080FA74`), `msg+24h = selector` (`0080FA89`), vtable `00D0334C`. `ghidra xrefs`
lists **three** call sites and no more: `00827372` and `0082740F` (rules R7b and R7c in
`00826F10`) and `00827D9A` in `00827B90`.

### 3b. The consumer

`00821E80 BSP_UnitInstance_HandleMessage` switches on `msg+10h` with `ADD EAX,-0x4B / CMP EAX,0x55`,
the byte table at `00822400` and the 27 dwords at `00822394`. For `9Eh`: `0x9E - 0x4B = 0x53`; the
byte at `00822453` is `0x17`; dword 23 at `008223F0` is **`0082203D`**. That arm is:

```
0082203d: MOV EAX,dword ptr [ESI + 0x24]      ; the selector
00822040: SUB EAX,0x0 / JZ 0x00822090         ; selector 0
00822045: SUB EAX,0x1 / JNZ (fall through)    ; selector 1
          both arms: ECX = EDI + 0xA20 (the repair task), argument = [ESI+1Ch] (the float)
0082204a  selector 1, [ESI+20h] != 0  -> 0082205c CALL 0093a4f0
0082204a  selector 1, [ESI+20h] == 0  -> 00822076 CALL 00939fa0
00822090  selector 0, [ESI+20h] != 0  -> 008220a3 CALL 0093a470
00822090  selector 0, [ESI+20h] == 0  -> 008220bd CALL 00939f90
```

Each of the four setters has exactly one call site, this one.

| routine | body | effect |
| --- | --- | --- |
| `0093A470` | adds the value to `task+38h` and to `task+3Ch`, clears `task+44h`, and calls `00983780(unit, value)` unconditionally | selector 0, flag set |
| `00939F90` | `task+38h = value` flat, three instructions | selector 0, flag clear |
| `0093A4F0` | adds the value to `task+34h`, mirrors the new total into `task+40h`, clears `task+44h`, and calls `009832F0(unit, value)` **only when `value > 0.0f`** (`00D7A218`) | selector 1, flag set |
| `00939FA0` | `task+34h = value` flat | selector 1, flag clear |

`00983780` and `009832F0` are the Lua-facing notifiers (both reach
`00887E50 BSP_MissionLuaHost_CallNamedThreadSafe`); their bodies were not decoded.

Both `0093A470` and `0093A4F0` end with the same predicted-damage test against the unit's health:
`task+34h * task+2Ch + task+38h * task+30h > [unit+370h]` tail-calls `0090E6C0` through
`[[00E188A8]+21A0h]`. The pairing `(+34h, +2Ch)` and `(+38h, +30h)` is proved by the FPU order at
`0093A4B7`..`0093A4C3`.

### 3c. Which selector is fire

Settled without relying on any doc: `0088E320 BSP_LuaBinding_SetFireDamage` builds the same `9Eh`
message inline at `0088E4AC`, storing `msg+24h = EBP` at `0088E4E9`. Filtering the whole listing for
`EBP` gives one write, `0088E343 XOR EBP,EBP`, and no other. **Selector 0 is fire, selector 1 is
water.** Therefore:

* fire → `task+38h` seconds, `task+30h` per second, `task+3Ch`, stepped by `0093C210`
* water → `task+34h` seconds, `task+2Ch` per second, `task+40h`, stepped by `0093C120`

This **confirms** the `cc2_settings_tail` renames already in the ledger
(`0093C120 = BSP_RepairTask_ApplyWaterDamage`, `0093C210 = BSP_RepairTask_ApplyFireDamage`, each
recorded as replacing the opposite `cc2_fire_flooding` name). It also means
`docs/UNIT_FIRE_AND_REPAIR.md`'s layout rows for `+2Ch`, `+30h`, `+34h`, `+38h`, `+3Ch` and `+40h`
and its section headings "**4.** Fire damage, `0093C120`" / "**5.** Water damage, `0093C210`" are
stale. That doc belongs to a merged packet and is **not edited here**.

One correction of substance: the float the message carries is added to a **seconds** field, not to a
per-second rate. `0093C120`/`0093C210` subtract `dt` from `+34h`/`+38h` and apply
`elapsed * rate / div`. The weapon virtuals `vtable[10h]` (`WaterDamage`) and `vtable[14h]`
(`FireDamage`) therefore author **durations**. No writer of the per-second fields `+2Ch`/`+30h` was
looked for in this packet; `docs/UNIT_FIRE_AND_REPAIR.md` attributes them to "message `9Eh` selector
0/1, handler unread", and that attribution is now **refuted** — the `9Eh` handler writes seconds
only, so `+2Ch` and `+30h` have **no known writer**.

## 4. The reachability verdict

### 4a. `part`, `fires` and `floods` are one problem, not three

Rule R1 at `00826F62` is not only the part gate. When `hit+34h == -1` the native jumps from
`00826F62` to the part loop at `00827582`, so **everything from `00826F6B` to `0082757B` is
skipped**: the armour source (R2), the hull damage (R3), the part arm (R4), the difficulty scaling
(R5), `this+10D0h` (R6), the roll torque (R7a), **the flooding (R7b)**, **the fire (R7c)**, the
component-failure roll (R8), the damage arrow (R9) and the hull impact effect (R10).

The reconstruction reproduces this faithfully: `src/ship_hit_record.cpp:189` opens
`if (hit.hull_segment != kHitRecordNoHullSegment) {` and closes it at line 283, with the flood and
fire arms at lines 245..259 inside it. `src/game_hosts_gunnery.cpp:1762` sets
`hit.hull_segment = kDirectHitHullSegment` and line 1772 sets `view.segment_kind = 0x0A`, because
the host models the three simple shapes of `docs/HIT_NARROWPHASE.md`, all of which write `-1` and
`0Ah`.

So the summary's `part=0 fires=0 floods=0` is **one missing input**, not three. The `hull=163` and
`total_damage` in the same line come from R12 `apply_base_hit_record` (`008777D0`,
`src/game_hosts_gunnery.cpp:1718`), which sits outside the gate.

### 4b. The weapon data is already there; the gate is what stops it

Run on this tree, exit 0:

```
bsp_game.exe --frames 3200 --press-start-frame 30 --menu-select USN02 --mission-frames 3000
  --mission-frame-seconds 0.05 --log local/out/usn02.log
  --xlive-dll build/win32/Release/xlive_stub.dll --game-root "<install>"

summary mission gunnery damage queued_hits=163 dispatched=163 hit_records=163 hull=163 part=0
        fires=0 floods=0 attributions=163 deaths=2 kill_credits=2 total_damage=14871.6
        first_hit=49.55 s
```

The host's `record_flood_rate` / `record_fire_rate` (`src/game_hosts_gunnery.cpp:1667`, `:1673`)
count whenever the rate is `> 0`, and the rates are `GameBulletClassRow::water_damage` /
`fire_damage`, flattened from `Bullets[b].WaterDamage` / `.FireDamage`. Those values are authored
and non-zero for the weapons that actually fired:

* the log line `unit hull input unit=Kortenaer type_id=265` fixes the class;
* `VehicleClass[265]`'s gun device ids are `54, 224, 228, 299, 342`, of which `DeviceClass[299]` is
  the `LIGHTARTILLERYFLAK` that took 96 of the shots, with `Bullet[1].Bullet = 30`;
* `gamemode.lua` sets `GameMode = 0`, so the autoload picks `ArcadeTable`;
* `ArcadeTable[30]` is `"Kuma 5.5'' Shell"` with `WaterDamage = 1`, `FireChance = 10`,
  `FireDamage = 5`.

A hit from that shell would flood on every impact and roll for a fire, **if** R1 let the arm run. It
does not. The zeros are therefore **not** a missing-data finding and not a bullet-class finding.

### 4c. The second route to `part`, which needs no `hit+34h`

R11b is a separate door. The part loop at `00827582` runs whatever R1 does, over the `10h`-byte
entries at `hit+3Ch` counted by `hit+40h`, and calls `0092D1F0` for every entry whose `+0h` is `0Dh`
(`src/ship_hit_record.cpp:294`). That array's producer is `00723F80` -> `006D2E30`, the GeomMesh
**sphere** test, i.e. a blast hit; `docs/HIT_HULL_SEGMENT.md` owns that chain. The host sets
`hit.part_hits = nullptr` and `hit.part_hit_count = 0`
(`src/game_hosts_gunnery.cpp:1766`-`1767`), so the loop never iterates. **R11b reaches `part` but
never `fires` or `floods`**, which are R7's alone.

### 4d. What the host would have to build, and the contract boundary

To reach R4 the gunnery host needs a hit record whose `segment_kind` is `0Dh` and whose
`hull_segment` is a real `0..19` index. That requires, in order:

1. **A `GeomMesh` payload decoder** for the chunk format of §1e. The rebuild already reads MMOD
   structured nodes — `src/structured_resource_probe.cpp` decodes
   `models/misc/repulogepdarabok_004.mmod` and `models/clouds/cloud_10.mmod` through the parser
   registry in `include/bsp/structured_resource_registry.hpp`, which registers Mesh, Note and
   GroupParams parsers. There is **no** `GeomMesh` parser in it. This file is **model/resource
   territory, held by a peer orchestrator; it is read here as a contract and neither leased nor
   reconstructed.**
2. **A victim-model binding in the gunnery host.** The host builds units from Lua class tables only
   and has no model at all; nothing in `src/game_hosts_gunnery.cpp` names a `.MMOD`.
3. **The segment trace.** `00723E90` -> `00723D60` -> `00723AA0` is pure geometry over the decoded
   elements in the node's local frame; `docs/HIT_HULL_SEGMENT.md` has all three ABIs.

The native link from the resource to the shape is where this packet stops:
`00712440 BSP_UnitPartCollisionNode_BuildShapes` takes the `{geometry*, transform*}` pairs from
`[node+160h]+40h..+44h`, and `node+160h` is the **second stack argument** of
`007135C0 BSP_UnitPartInstance_Construct` (`00713604 MOV [ESI+0x160],EAX` with
`EAX = [ESP+0x30] = ESP0+8`; `node+164h` and `node+4Ch` take argument 1, the unit). That argument is
produced at `0087BE54` in `0087BCC0 BSP_UnitInstance_InitHealthAndParts` by a two-step virtual
dispatch, `EDI->vtable[8]( unit->vtable[190h](float 00CED9E0) )` (`0087BE4B`, `0087BE52`), both
`contract: unread`. **Whoever fills that vector is the model/scene system, and is not this packet's
to reconstruct.**

**Verdict.** The geometry element producer is recovered and the authored data is present in this
installation, so the chain is no longer unknown — but `part`, `fires` and `floods` stay at zero
until a `GeomMesh` parser lands in the peer-owned structured-resource registry and the gunnery host
can name the victim's model. Nothing else is missing: the weapon values, the 20-slot controller
arrays, `0092D1F0`, the `9Eh` message and both repair channels are all already in place.

### 4e. Smallest unblocking step

Add a `GeomMeshStructuredResourceParser` to `include/bsp/structured_resource_registry.hpp`'s
registry, shaped exactly like the existing `MeshStructuredResourceParser`, decoding §1e's chunk into
`{ kind, index }` per element plus the vertex and triangle arrays. It must provide, per element:
the kind from `bsp::geom_mesh_element_kind_00727310`, the `u32` index, the vertex array of the
owning mesh and the element's `u16` triangle-index list. That one parser turns `segment_kind` and
`hull_segment` real and unblocks `part`, `fires` and `floods` together.

## 5. What is proven, and what is assumed

Proven from the listing and, where stated, from the installed data:

* `00727310` writes `element+4h` and `element+8h` at `007273DB`/`007273DF`, from `007149D0(name)`
  (with `7 -> 9`) and `00BE9A00(node)`; `00727260` appends the element with stride `2Ch`.
* The `00E08138` table has fifteen entries in the order tabulated; index `13` is `fizika` and index
  `10` is `none`.
* `0087CA80` puts the same `007149D0` result at the Lua part descriptor's `+4h` and the `Index` key
  at its `+8h`, over a `30h` stride.
* `Farragut_1934.MMOD` authors `fizika` elements with indices `0..3` in two `GeomMesh` chunks.
* `00937C90` sizes the health vector to `0x14` and runs the `fizika_%02d` loop over `0..19`.
* `0080FA50` builds message `9Eh` at exactly three call sites; `00821E80` routes it to `0082203D`;
  selector `0` reaches `0093A470`/`00939F90` and selector `1` reaches `0093A4F0`/`00939FA0`.
* `0088E320`'s selector is `0` (`XOR EBP,EBP` is the only write to `EBP`).
* The R1 gate encloses R2..R10 in the native (`docs/SHIP_HIT_RECORD.md`) and in
  `src/ship_hit_record.cpp:189..283`; the 3000-frame USN02 run reproduces `part=0 fires=0 floods=0`
  with `total_damage=14871.6` from R12.

Assumed or partial:

* The names in the `00E08138` table are the authors' words, not recovered symbols; the mapping of
  each index to a gameplay meaning beyond `fizika` and `none` is not established.
* `00727310`'s triangle decode from `00727580` on, the `n2` section at `007274C4`, and element
  fields `+20h`/`+24h`/`+28h` are unread — `coverage: partial`.
* `00983780` and `009832F0` are named only by their callee set (both reach the mission Lua host);
  their bodies are `contract: unread`.
* Whether the Lua descriptor's `Index` and the geometry element's `+8h` index the same space is
  not established.
* `element+24h` as a heap pointer (this packet) versus a scalar (`docs/HIT_HULL_SEGMENT.md`) is
  unresolved; `00723B70` and `0085BF90` were not read.

Negative results, stated as negative results:

* **`docs/HIT_HULL_SEGMENT.md`'s open item 1 is closed, and its byte-scan negative stands.** That
  packet reported "no immediate store of `0Dh` into `+30h`" and was right: the kind is never an
  immediate anywhere. It arrives from `00E08138`'s table index. The producer was found through the
  resource-parser registration table, not through any byte scan.
* **No writer of the per-second fields `task+2Ch`/`task+30h` is known.** The `9Eh` handler, which
  `docs/UNIT_FIRE_AND_REPAIR.md` names as their writer, provably writes only the seconds fields.
* **`ghidra xrefs` was checked against the row cap on both censuses that matter**: `0080FA50`
  returned three rows and each of the four `9Eh` setters returned one, all well under the 25-row
  cut-off, so neither "three call sites" nor "one call site each" rests on a truncated listing.

## 6. Files read from the installation

Read-only; the installation was not modified. This installation is **modded** (BSPRM/AlterBSP), so
these are this installation's files, not retail.

| path (under the install root) | mtime | size |
| --- | --- | --- |
| `models/ships/us/Farragut_1934.MMOD` | 2024-07-13 11:24:44 -0700 | 18201279 |
| `models/ships/us/North_Carolina.mmod` | 2024-07-13 11:24:54 -0700 | 29328089 |
| `scripts/datatables/autoload/vehicleclasses.lua` | 2026-05-09 21:52:13 -0700 | 3349364 |
| `scripts/datatables/classtables/arcade/bulletclasses.lua` | 2026-05-09 23:04:46 -0700 | 75048 |
| `scripts/datatables/classtables/realistic/bulletclasses.lua` | 2025-06-02 10:26:31 -0700 | 64887 |
| `scripts/datatables/classtables/arcade/deviceclasses.lua` | read for device ids 54/224/228/299/342 | — |
| `scripts/datatables/autoload/bulletclasses.lua`, `gamemode.lua` | 2024-07-13 11:27:48 -0700 (`gamemode.lua`) | 229 |

## 7. The module published

`include/bsp/part_damage_reachability.hpp` / `src/part_damage_reachability.cpp` publish only the one
pure rule this packet recovered with explicit inputs: the `00E08138` table,
`mesh_category_from_name_007149d0`, `geom_mesh_element_kind_00727310` (the `7 -> 9` remap included),
and the 20-slot hull-segment range. It includes `bsp/ship_hit_record.hpp` and `static_assert`s that
`fizika`'s index equals the existing `kShipHitSegmentKindBreakable`, rather than redeclaring it.
Nothing else is published: the rest of this packet is scoping evidence. No test was added.

## 8. Follow-up packets

1. **The `GeomMesh` parser** in the peer-owned structured-resource registry — §4e. This is the
   single unblocking step for `part`, `fires` and `floods`.
2. **`00727310`'s triangle decode**, the `n2` section and element `+20h`/`+24h`/`+28h`, which would
   also settle the `+24h` scalar-versus-pointer question against `00723B70`/`0085BF90`.
3. **The writer of `task+2Ch`/`task+30h`**, the fire and water per-second rates, now that the `9Eh`
   handler is excluded.
4. **`[node+160h]+40h`'s producer**, reached from `0087BE4B`/`0087BE52` — model/scene territory.
5. **`00983780` / `009832F0`**, the two mission-Lua notifiers a fire or a leak raises.
