# The ordnance `kind` enumeration: it is the entity class-id space

Addresses: `007B91C0`, `007B9230`, `007B9320`, `007B93E0`, `007B93F0`, `007B9480`, `007B94F0`,
`007B9500`, `007ED830`, `007EE8F0`, `006E4060`, `006E8400`, `006EA2A0`, `006EA370`, `006EA3E0`,
`006EA550`, `006EA620`, `006EA6F0`, `006EA760`, `006EA830`, `006EA8B0`, `006EA910`.
Descriptor vtables `00CFA138`, `00CFA440`, `00CFA478`, `00CFA4B0`, `00CFA4DC`, `00CFA508`,
`00CFA53C`, `00CFA56C`, `00CFA59C`, `00CFA5CC`, `00CFA604`, `00CFA63C`, `00CFA678`.

## The answer

The `kind` argument of `007B91C0` is a value of the **entity class-id space** that
`docs/ENTITY_CLASS_IDS.md` surveys — the same namespace as `+C4h` and `vtable[5Ch]`. It is not a
private ordnance enumeration, and the agreements with `2A MBomb`, `2B MTorpedo`, `2C MDepthCharge`,
`2F MDummyKamikazePlane` and `33 MRocket` are not coincidence.

`31h` really is `MParatrooper`, and `007B9500` really does pass `31h`. The contradiction dissolves
on the third branch of the brief's trichotomy: **`levelbomb` is the wrong label for `007B9500`.**
That helper asks "does any slot carry Paratrooper-class ordnance"; the `levelbomb` command class
`00E08F28` uses its answer to pick between two admission paths, one of which is a paradrop against a
command building. See "The `31h` resolution" below.

Everything here is **exported / read** evidence: the Ghidra listing, `.rdata` bytes and disk bytes.
Nothing in this document is reconstructed C++, built, or game-validated.

## The call shape at `007B91C0`

`__thiscall(this = plane entity, int kind, int loadout)`, `RET 8`. Four register saves put the
arguments at `ESP+14h` and `ESP+18h`.

```
007b91c8  CMP  dword ptr [EBX + 994h], EDI      ; slot count
007b91d0  MOV  EBP, dword ptr [ESP + 18h]       ; arg2 = loadout index
007b91d4  LEA  ESI, [EBX + 974h]                ; slot array
007b91e0  MOV  ECX, dword ptr [ESI]             ; slot
007b91e4  MOV  EDX, dword ptr [EAX + 220h]      ; slot->vtable[220h]
007b91eb  CALL EDX                              ; -> descriptor in EAX
007b91f1  MOV  ECX, dword ptr [ESP + 14h]       ; arg1 = kind
007b91f7  MOV  EDX, dword ptr [EDX + 8h]        ; descriptor->vtable[8]
007b91fd  CALL EDX
007b91ff  TEST AL, AL                           ; a predicate, not a getter
```

`descriptor->vtable[8]` is a **`bool(int)` predicate**, not a "return the kind" accessor. The
earlier reading in `docs/ATTACK_COMMANDS.md` ("ask the descriptor `vtable[8](kind)`") is right about
the shape and this document settles what the predicate computes.

## The six constants, verified from their own bytes

Read from each helper's listing, not from the table in `docs/ATTACK_COMMANDS.md`.

| helper | site | constant | how |
| --- | --- | --- | --- |
| `007B9320` | `007B935C` | `2Ah` | `PUSH 2Ah` inline, own loop |
| `007B93E0` | `007B93E5` | `2Fh` | `PUSH 2Fh`, tail-calls `007B91C0` |
| `007B93F0` | `007B93F5` | `2Bh` | `PUSH 2Bh`, tail-calls `007B91C0` |
| `007B9480` | `007B94B2` | `33h` | `PUSH 33h` inline, own loop |
| `007B94F0` | `007B94F5` | `2Ch` | `PUSH 2Ch`, tail-calls `007B91C0` |
| `007B9500` | `007B9505` | `31h` | `PUSH 31h`, tail-calls `007B91C0` |

All six agree with the recorded table. Hypothesis 1 of the brief ("`007B9500`'s kind is not actually
`31h`") is **ruled out**.

Two helpers carry extra conditions the shared body does not:

* `007B9320` requires `2Ah` **and** rejects the slot if any of `2Ch`, `31h`, `2Bh`, `33h`, `2Dh`
  also answers (`007B936B`, `007B937A`, `007B9389`, `007B9398`, `007B93A7`; the first four are
  `JNZ` to the next slot, the fifth `JZ` to success).
* `007B9480` requires `33h` **and** `descriptor[+E0h] > 0.0f`, as SSE:
  `007B94BC XORPS XMM0,XMM0` / `007B94BF COMISS XMM0, [ESI+E0h]` / `007B94C6 JC <success>`.
  No x87 is involved.

## What `vtable[8]` is

### The descriptor is the projectile class descriptor, not an entity

`007B91C0` reaches the descriptor through `slot->vtable[220h](loadout)`. For `MBombPlatform`
(class id `25h`, vtable `00CF96A8`), slot `220h` is at `00CF98C8` and holds **`006E4060`**. That
function is the proof of identity on its own:

```
006e4064  MOV  ESI, dword ptr [EDI + 48h]        ; child list head
006e4070  MOV  EDX, dword ptr [EAX + 5ch]        ; child->vtable[5Ch]   <- the ENTITY class test
006e4075  PUSH 2Ah                               ;   IsKindOf(2Ah)
006e407f  MOV  ESI, dword ptr [ESI + 44h]        ; next child
...
006e40ba  MOV  EAX, dword ptr [EAX + 34h]        ; gun[+3F8h][+34h]
006e40c1  MOV  EAX, dword ptr [EDX + 8h]         ; that object's vtable[8]
006e40c4  PUSH 2Ah                               ;   the SAME constant
006e40c6  CALL EAX
006e40cc  MOV  EAX, dword ptr [ECX + 34h]        ; return gun[+3F8h][+34h]
006e40da  MOV  EAX, dword ptr [ESI + 314h]       ; or the found child's +314h
```

One function passes the single constant `2Ah` into **both** `vtable[5Ch]` on an entity and
`vtable[8]` on the returned object. Whatever else is true, those two predicates read the same
number out of the same namespace.

The returned object is `gun[+3F8h][+34h]`, which `docs/WEAPON_CLASS_DESCRIPTOR.md` already
identifies as the **projectile ("bullet") class descriptor**, one per `Bullets` Lua row, built by
`006EA910 BSP_BulletClass_GetOrCreate`. Two independent confirmations:

* `007B9230` returns the same object for slot 0 with `loadout = 1`, and `007EEA6C` tests
  `descriptor[+8h] == 0Ah`. `0Ah` is exactly the Torpedo descriptor's sub-type constant in
  `docs/PROJECTILE_KINDS.md`, and `+F0h`/`+F4h`, read at `007EEA72`/`007EEA87`, lie inside the
  Torpedo descriptor's `0FCh` allocation.
* `00CFA6A3` holds the C string `MDummySubmarine`, immediately after the last slot of the
  descriptor vtable at `00CFA678` — the class-name-follows-vtable layout that
  `docs/PROJECTILE_KINDS.md` used to read the family.

It is **not** an entity. On entity vtables, slot `+8` is the shared do-nothing stub `00923030`,
whose entire body is `C2 04 00` (`RET 4`) — verified identical at `+8` for `MBomb` `00CF9438`,
`MTorpedo` `00D0C3E8`, `MDepthCharge` `00CFBA80`, `MParatrooper` `00D05060`, `MRocket` `00D090E8`,
`MDummyKamikazePlane` `00CFC698`, `2D` `00CFC910`, `MBullet` `00CF9DF0` and `Path` `00CE6290`.
Entities answer class questions at `+5Ch`; descriptors answer them at `+8`.

### Slot `+8` on the descriptor is a hand-written class test

Two of the thirteen, from the Ghidra listing:

```
006e8400  XOR  EAX,EAX / CMP dword ptr [ESP+4],29h / SETZ AL / RET 4      ; MBullet
006ea2a0  MOV  EAX,[ESP+4] / CMP EAX,2Ah / JZ ok / CMP EAX,29h / JZ ok    ; MBomb
          XOR EAX,EAX / RET 4  ...  ok: MOV EAX,1 / RET 4
```

`29h` is `MBullet` and `2Ah` is `MBomb` in `docs/ENTITY_CLASS_IDS.md`. The chain is the descriptor
class's own ancestry, spelled with entity class ids.

## Why the namespace is global, and not a lookalike

Three facts, each sufficient on its own:

1. `006E4060` feeds the same literal `2Ah` to `vtable[5Ch]` on an entity and to `vtable[8]` on a
   descriptor (above).
2. The `MDummyKamikazePlane` descriptor answers `17h` (`006EA834`) and the `MDummySubmarine`
   descriptor answers `08h` (`006EA8B4`). `17h` is `MPlaneKamikaze` and `08h` is `MSubmarine` —
   aircraft and ship class ids, far outside the `29h`..`34h` projectile band and meaningless in any
   ordnance-local enumeration. They are there because each dummy impersonates the real thing.
3. All thirteen descriptor classes and all thirteen Lua `Type` strings line up one-to-one with the
   `29h`..`34h` band of the entity table, with no leftovers on either side.

## The complete census: thirteen descriptor classes, none sampled

Every `Type` literal read from `.rdata`; every `vtable[8]` pointer read from the vtable bytes; every
test body disassembled. "Answers" is the exact set of ids for which the test returns 1.

| Lua `Type` | literal | descriptor vtable | `vtable[8]` | answers |
| --- | --- | --- | --- | --- |
| `Bullet` | `00CFA720` | `00CFA138` | `006E8400` | `29h` |
| `Artillery` | `00CE5454` | `00CFA440` | `006E8400` | `29h` |
| `Kamikaze` | `00CFA714` | `00CFA478` | `006E8400` | `29h` |
| `Flak` | `00CFA6BC` | `00CFA53C` | `006E8400` | `29h` |
| `Bomb` | `00CFA70C` | `00CFA4B0` | `006EA2A0` | `2Ah`, `29h` |
| `Torpedo` | `00CE544C` | `00CFA56C` | `006EA550` | `2Bh`, `2Ah`, `29h` |
| `DepthCharge` | `00CFA700` | `00CFA508` | `006EA3E0` | `2Ch`, `2Ah`, `29h` |
| `DummyTarget` | `00CFA6F4` | `00CFA5CC` | `006EA6F0` | `2Eh`, `2Ah`, `29h` |
| `DummyKamikazePlane` | `00CFA6E0` | `00CFA63C` | `006EA830` | `17h`, `2Fh`, `2Dh`, `2Ah`, `29h` |
| `DummySubmarine` | `00CFA6D0` | `00CFA678` | `006EA8B0` | `08h`, `30h`, `2Ah`, `29h` |
| `Paratrooper` | `00CFA6C4` | `00CFA604` | `006EA760` | `31h`, `2Ah`, `29h` |
| `Rocket` | `00CFA6B4` | `00CFA4DC` | `006EA370` | `33h`, `2Ah`, `29h` |
| `WaterMine` | `00CEA74C` | `00CFA59C` | `006EA620` | `34h`, `2Ah`, `29h` |

Each id in "answers" matches that `Type`'s entity class id in `docs/ENTITY_CLASS_IDS.md` exactly:
`29 MBullet`, `2A MBomb`, `2B MTorpedo`, `2C MDepthCharge`, `2E MDummyTarget`,
`2F MDummyKamikazePlane`, `30 MDummySubmarine`, `31 MParatrooper`, `33 MRocket`, `34 MWaterMine`.

### Two properties a host must copy, not derive

**The chains are hand-written and do not match the entity parent graph.** They are the *descriptor*
hierarchy's ancestry, labelled with entity ids:

* `MBomb`'s descriptor answers `2Ah, 29h`, but the `MBomb` **entity**'s parent is `02`, not `29h`.
  The descriptor derives from the `MBullet` descriptor; the entity does not derive from `MBullet`.
* `MDummyTarget` (`2Eh`) and `MDummySubmarine` (`30h`) have entity parent `2Dh`, but their
  descriptor tests **omit `2Dh`**. Only `MDummyKamikazePlane` lists it. Deriving the chains from the
  parent column of `docs/ENTITY_CLASS_IDS.md` would give the game's own `007B9320` the wrong answer
  for two of thirteen classes.

**Four classes never answer their own id.** `Artillery`, `Kamikaze` and `Flak` share `MBullet`'s
unmodified test `006E8400`, so they answer `29h` only. A descriptor cannot be asked "are you
`32h` (`MFlakBullet`)?" and say yes. This is a property of the shipped binary, not a gap in the
reading: the three vtables hold the same pointer as `MBullet`'s.

## The `31h` resolution

* Hypothesis 1 — `007B9500`'s constant is not `31h`: **ruled out**, `007B9505 PUSH 31h`.
* Hypothesis 2 — `31h` is not `MParatrooper` here: **ruled out**. `006EA760` is reached only from
  the `Paratrooper` descriptor's vtable `00CFA604`, and its chain `31h, 2Ah, 29h` is the
  `MParatrooper` -> `MBomb` -> `MBullet` chain of the entity table, id for id.
* Hypothesis 3 — `levelbomb` is the wrong label: **this is the answer.**

`007ED830` is the only caller of `007B9500` (`ghidra xrefs`, one site). It loops the squadron's
`+3CCh` planes at `+3D0h` and asks each for Paratrooper-class ordnance. Its own single caller is
`007EE967`, inside the `levelbomb` arm of `BSP_Unit_AttackCommandApplies`, gated by
`007EE946 CMP EAX, 0E08F28h`:

```
007ee94e  BL = plane[0]->IsKindOf(10h)          ; 10h = MPlaneBomber -> "level bomber"
007ee967  CALL 007ed830                         ; any Paratrooper ordnance?
007ee96e  JZ   007ee996                         ; no  -> the general-bomb path
007ee970  target_is_surface, BL, and target->IsKindOf(1Ch)   ; 1Ch = MCommandBuilding -> accept
007ee996  target_is_surface, BL, 007ed7e0 (general bomb), and NOT target->IsKindOf(0Eh)
007ee9b6                                                     ; 0Eh = MTorpedoBoat -> accept
```

So the `levelbomb` command class has two admission paths, and the paratrooper query selects between
them: **a paratrooper-carrying level bomber may be ordered only against a command building** (a
paradrop/capture), while a bomb-carrying level bomber may be ordered against any surface target
that is not a torpedo boat. Dropping paratroopers is indeed not a level bombing run — which is
exactly why the game tests for them first and sends them somewhere else.

Nothing about the enumeration is wrong. The label on the helper was.

## What the six queries actually admit

Derived by applying each query's constants to the census table above.

| helper | admits `Type` |
| --- | --- |
| `007B9320` (`2Ah` minus `2Ch`/`31h`/`2Bh`/`33h`/`2Dh`) | `Bomb`, `DummyTarget`, `DummySubmarine`, `WaterMine` |
| `007B93E0` (`2Fh`) | `DummyKamikazePlane` |
| `007B93F0` (`2Bh`) | `Torpedo` |
| `007B9480` (`33h`, `+E0h > 0`) | `Rocket` |
| `007B94F0` (`2Ch`) | `DepthCharge` |
| `007B9500` (`31h`) | `Paratrooper` |

`007B9320` is a residual class and its exclusion list is *not* "all children of `2Ah`": the six
children in the entity table are `2B`, `2C`, `2D`, `31`, `33` and `34 MWaterMine`, and `34h` is not
excluded. `DummyTarget` and `DummySubmarine` slip in only because their descriptor chains omit
`2Dh`. Whether those three ever occupy an aircraft weapon slot is a **data** question about the
authored `Bullets` rows and is not settled here; the code admits them.

## Wiring contract: authored `Type` string to `kind`

**Bullets do not use the `Type = E <Enum> : <symbol>` mechanism.** That mechanism belongs to the
scene-file property descriptors (`docs/SCENE_UNIT_CREATORS.md`, `docs/GAME_EXECUTABLE.md`) and
resolves `ShipClasses` / `PlaneClasses` symbols. The bullet path is a plain string match:

1. `006EA910 BSP_BulletClass_GetOrCreate(out, bulletId)` takes the Lua globals, indexes the
   `Bullets` table by id, and reads the entry's `Type` key — the literal string `Type` at
   `00CE4780` — as a native string defaulting to `""`.
2. It runs up to thirteen `00425850 BSP_NativeString_EqualsCStringInsensitive` tests against the
   literals in the census table (each site is `PUSH <literal>` / `LEA ECX,[ESP+1Ch]` / `CALL
   00425850`; verified sites: `006EAB26` `Bomb`, `006EAB56` `Torpedo`, `006EABB6` `DummyTarget`,
   `006EACC8` `WaterMine`, `006EAA92` `Artillery`, `006EAAE4` `Kamikaze`, and each falls through to
   the next by `JZ`). Comparison is **case-insensitive** (`__stricmp`). No match stores a null class
   pointer and returns.
3. The matching branch allocates that kind's descriptor size and runs that kind's constructor,
   which installs the vtable in the census table.

The host therefore needs no enum table. The mapping from an authored `Type` string to the set of
kinds the descriptor answers is the constant, closed table above:

```
Bullet, Artillery, Kamikaze, Flak  -> { 29 }
Bomb                               -> { 2A, 29 }
Torpedo                            -> { 2B, 2A, 29 }
DepthCharge                        -> { 2C, 2A, 29 }
DummyTarget                        -> { 2E, 2A, 29 }
DummyKamikazePlane                 -> { 17, 2F, 2D, 2A, 29 }
DummySubmarine                     -> { 08, 30, 2A, 29 }
Paratrooper                        -> { 31, 2A, 29 }
Rocket                             -> { 33, 2A, 29 }
WaterMine                          -> { 34, 2A, 29 }
```

`AttackFeasibilityInputs` (`include/bsp/attack_commands.hpp`) can be answered by walking a unit's
planes, walking each plane's weapon slots, resolving each slot's `Type` string, and taking the
union of these sets — provided the host can resolve a slot to its `Type`. That last step is **not**
established here; see below.

## What this packet did not establish

* **How a weapon slot resolves to a `Bullets` row.** `007B91C0` reaches the descriptor through
  `slot->vtable[220h](loadout)`, which for `MBombPlatform` is `006E4060` and for
  `MMultipleBombPlatform` (`00CF9918 + 220h` = `00CF9B38`) is `006E4640`. Only `006E4060` was read.
  The `+48h`/`+44h` child-list arm and the `gun[+3F8h][+34h]` arm may resolve to different rows, and
  `006E4060`'s middle branch (`+3F0h`, `vtable[1E4h]`, the `TEST ECX,0E19983h` idiom at `006E40B2`)
  was not analysed. A host cannot answer the ordnance inventory from `Type` strings alone until this
  is read.
* **Which descriptor kinds actually occupy aircraft slots** in the authored data. The claim that
  `007B9320` admits `DummyTarget`, `DummySubmarine` and `WaterMine` is a statement about the code,
  not an observation of any shipped loadout.
* **The `2Dh` class's name.** It sits between `MBomb` and the three `Dummy*` classes and is unnamed
  in `docs/ENTITY_CLASS_IDS.md`; no `Type` literal builds it, so it is abstract in the descriptor
  family too.
* **The `loadout` argument's meaning** beyond what `docs/ATTACK_COMMANDS.md` already records.
* Nothing here was compiled or run. No C++ was written for this packet.

## Corrections owed to other documents

Not applied — `docs/ATTACK_COMMANDS.md` is outside this packet's lease.

| doc | line | says | should say |
| --- | --- | --- | --- |
| `docs/ATTACK_COMMANDS.md` | 107 | `007ED830` / `007B9500` / `31h` is class `levelbomb, primary` | the query is **paratrooper ordnance**; it selects the paradrop admission path *inside* the `levelbomb` command class |
| `docs/ATTACK_COMMANDS.md` | 102 | "ask the descriptor `vtable[8](kind)`" | correct in shape; `vtable[8]` is the descriptor's class test in the entity class-id space, and the descriptor is the projectile class descriptor `gun[+3F8h][+34h]` |
| `docs/ATTACK_COMMANDS.md` | 78 | "level-bomb ordnance and target `IsKindOf(1Ch)`" | "**paratrooper** ordnance and target `IsKindOf(1Ch)` = `MCommandBuilding`" |

## Host follow-up: no `BOMBPLATFORM` gun is ever built

With the table above wired into the gunnery host, `USN01` reports its first real ordnance inventory:

```
ordnance units_with torpedo=5 general_bomb=0 drop_kamikaze=0 paratrooper=0  (of 41 units with guns)
```

The torpedo count is corroborated by the per-category table's 15 `TORPEDO` guns, and
`general_bomb=0` is internally consistent: the only `2Ah`-answering guns present are the 10
`DEPTHCHARGE` ones, and `007B9320` excludes `2Ch`. So the inventory itself is behaving.

But the zero is caused by something upstream, and it is a host gap rather than authored absence:

* `deviceclasses.lua` authors **34** devices with `Function = "BOMBPLATFORM"`.
* `RealisticTable[78]` is one of them.
* `VehicleClass[108]` "SBD Dauntless" lists a platform whose `Gun[1] = 78`, alongside `95`
  (`PLANEGUN`), `95` and `104` (`AAMACHINEGUN`).
* `ConSBD1` in `USN01` is `type_id=108`, and the host builds it **3** guns - `0:2 1:1` - not 4.
* The per-category table has no `0Ah` row at all, in any mission run so far.

So the bomb platform is dropped between the authored `Platforms` table and `GameGunRow`. It is not
the category range filter (`0Ah` is inside `kUnitGunneryCategoryCount`), and it is not the
`Function` spelling (the flatten chunk emits all twelve names, `BOMBPLATFORM` among them). The
flatten requires `type(dev) == 'table'` after looking `p.Gun[1]` up in the device table, so the
likeliest cause is that the device lookup fails for that id - and one candidate worth checking first
is which of `classtables/arcade` and `classtables/realistic` the host actually loaded, since the two
tables are indexed independently and `78` need not be the same device in both.

**Why it matters:** until a `BOMBPLATFORM` gun exists, no aircraft can answer
`has_general_bomb_ordnance` or `has_level_bomb_ordnance`, so `007EEC50` could never choose
`divebomb` or `levelbomb` for one - even with the ordnance table wired, which it now is. This is on
the critical path to directed air attack.
