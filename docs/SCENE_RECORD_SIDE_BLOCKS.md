# Scene record side blocks (the eight player slots at `record+4h`)

Addresses: 004da2a0 004c9800 004c9820 004c6750 004c6760 004f1d70 004e82f0 004c6890 00469bf0 008f5a00 004c3840 004bb160 004d87b0

`docs/MISSION_LOAD_PATH.md` reconstructed `004E1D70 BSP_Game_BuildSceneRecord` and left one thing
open: the `0x120` side blocks and the count at `record+988h` are read by `004C6890`, the `.scn`
header pass is the only thing that touches the record between `operator new` and that read, and no
write to the block range was found in the reader's own reconstruction. This packet found the
writer. It is **not** in the reader's grammar at all: it is in the *property applier* the header's
`properties` block runs.

The blocks are the multiplayer player-slot definitions authored in the scene file. The count at
`+988h` is the `MaxPlayerNum` property, not a block count the reader computes.

## The chain

| Step | Address | What it does |
| --- | --- | --- |
| 1 | `004E1D70` | `operator new(0x109c)` then `004DA2A0`, the record constructor |
| 2 | `0046DF00` | header pass over the `.scn`, `004E1D70` passes the record as argument 4 |
| 3 | `00469BF0` | the `header` block; its `properties` key parses a `0x114` bag with `008F5A00` and publishes it as `SceneRootProps` |
| 4 | `004F1D70` | the bag is applied to the record; `004F21B4..004F21E3` runs step 5 eight times |
| 5 | `004E82F0` | one side block from `MultiPlay.PlayerN` |
| 6 | `004C6890` | copies `record+988h` blocks into the eight `0x118` slot records at `game+1008h` |

Step 4 runs only when the header carries a `properties` block, and step 5 only when that bag
carries a `MultiPlay` sub-block. Both are true for all 259 installed `.scn` files.

## The record constructor, `004DA2A0`

The ledger called this `CG_array_ctor_helper_004da2a0`. It is the constructor of the whole `0x109c`
record, and reading it settles the base of the side-block array:

```
004da2d7 MOV dword ptr [ESI],0xce7950          ; vtable at record+0h
004da2ca LEA EAX,[ESI + 0x4]                   ; base of the array
004da2cd PUSH 0x120 / PUSH 0x8                 ; element size, count
004da2bc PUSH 0x4c9820 / PUSH 0x4c9800         ; element dtor, element ctor
004da2dd CALL 0x00bf7cd1                       ; eh vector constructor iterator
```

| Offset | Constructed as |
| --- | --- |
| `+0h` | vtable `00CE7950` |
| `+4h`..`+903h` | 8 side blocks of `0x120`, ctor `004C9800`, dtor `004C9820` |
| `+904h`/`+905h` | not touched here; `GroundCollision` and the precache flag |
| `+90Ch`/`+910h` | native string, zeroed |
| `+914h` | **not** zeroed; only `004E1D70`'s reuse arm writes it |
| `+918h`..`+927h` | two native strings, zeroed |
| `+928h`..`+97Fh` | 11 native strings of 8 bytes, ctor `00415270`, dtor `0041DD20` |
| `+980h`/`+984h` | native string, zeroed |
| `+988h`/`+98Ch` | **not** touched here; written by `004F1D70` |
| `+990h` | sub-object, `004CB420` |
| `+0C24h`.. | zero runs and further sub-objects |

So the array base is `record+4h`, and the eight blocks end at `+904h`, which is where the
`GroundCollision` byte sits. `8 * 0x120 = 0x900` still holds; the array is just shifted by the
vtable pointer.

## One side block

`004C9800` builds five `0x38` elements at `block+8h` (ctor `004C6750`, dtor `004C6760`) and leaves
`block+0h` and `block+4h` uninitialised. `004E82F0` fills all of it from `MultiPlay.PlayerN`:

| Offset | Size | Key | Notes |
| --- | --- | --- | --- |
| `+0h` | 4 | `Party` (`00CE5804`) | the resolved enum ordinal; reaches slot record `+28h` |
| `+4h` | 4 | `Race` (`00CE8EE0`) | the resolved enum ordinal; reaches slot record `+24h` |
| `+8h` + n*`0x38` | `0x38` | `Unit0`..`Unit4` | five unit elements |

One `0x38` unit element, relative to its own base:

| Offset | Size | Key | Notes |
| --- | --- | --- | --- |
| `+0h`/`+4h` | 8 | `Name` (`00CE8ED0`) | native `{length, pointer}` pair |
| `+8h` | 4 | `Icon` (`00CE8EC8`) | preset to `-1` at `004E8384` **before** the `UnitN` test |
| `+0Ch` | 4 | `ClassId` (`00CE6920`) | |
| `+10h`/`+14h` | 8 | `ClassName` (`00CE6930`) | native string pair |
| `+18h` + j*8 | 8 | `Pool0`..`Pool3` (`00CE8EC0`) | `{Icon, Num}`; `Num` is `00CE8B94` |

`Unit0`..`Unit4` and `Pool0`..`Pool3` are built by overwriting the last byte of the templates
`UnitN` (`00CE8ED8`) and `PoolN` (`00CE8EC0`) with `'0' + index`, so the index is a single digit and
the key length never changes.

### What an absent sub-block leaves behind

- No `PlayerN` group, or a `PlayerN` whose node type tag is not 6: `004E8307`/`004E8311` return
  before the record is touched. The block keeps whatever the constructor left, which for
  `block+0h`/`+4h` is uninitialised heap.
- No `UnitN`: only the unit's `Icon` dword is written, with `-1`. The name and class-name strings
  are the constructor's zeroed pairs; `ClassId` and the four pool entries are untouched.
- No `PoolN`: `004E84D1` writes `-1` into that pool's `Icon` dword and leaves its `Num` dword alone.
  `004C6750` never initialises `Num`, so a scene that authors a `UnitN` without a `PoolN` leaves the
  count dwords reading uninitialised memory. No shipped file authors a `PoolN` at all.

`Party` and `Race` are read with no null check: a `PlayerN` group missing either key dereferences
`node+0Ch` at address `0Ch`. Every shipped `PlayerN` carries both.

## The key lookup is case-insensitive

`008F2260` splits the key on `'.'` and looks it up through `0043B8B0`, which matches on the hash
bucket and the length and then compares with `__stricmp`. That matters here: the applier passes
`MultiPlay` (`00CEA438`) and every shipped `.scn` spells the block `"Multiplay"`. With a
case-sensitive compare no scene would fill a single side block.

## What consumes them

`004C6890` copies each block into the eight `0x118` slot records at `game+1008h`:

| Slot record field | Source |
| --- | --- |
| `+8h` | 1 for a copied block, 0 for the cleared tail |
| `+24h` | side block `+4h` (`Race`) |
| `+28h` | side block `+0h` (`Party`) |
| `+50h`, `+70h` | `strcpy` of the empty literal `00CE3A0C` |

`004C3840 BSP_Game_AssignPartyPlayerSlots` and `004BB160 BSP_Game_ResetParticipantTable` read the
same `[game+5FCh]+988h` count, and `004D87B0 BSP_Game_CheckMultiplayerPlayerCount` reads it too,
which is the corroboration that the field is a player count.

`004C6890`'s loop runs `record+988h` times, **not** `min(count, 8)`: the bounds test at `004C6AD1`
(`if (7 < i)`) only decides whether the tail clear runs. A `MaxPlayerNum` above 8 therefore writes
past the end of the eight-record array. No shipped scene authors one: 258 of 259 files carry
`MaxPlayerNum = I 8` and one carries `4`.

## Installed-file evidence

`src/mission_scene_probe.cpp --sweep` walks every `.scn` under the install, parses each with the
reconstructed reader and applies `read_scene_record_slot_table_004f1d70`:

| Measure | Value |
| --- | --- |
| files | 259 |
| files with a `MultiPlay` block | 259 |
| `MaxPlayerNum = 8` | 258 |
| `MaxPlayerNum = 4` | 1 |
| authored `UnitN` entries | 16 |
| authored `PoolN` entries | 0 |

The sixteen units are in `universe/scenes/missions/multi/scene6.scn` and `scene901.scn`, eight each,
all of the form

```
"Player1" {
  Party = E Party : Allied  ;
  Race = E Races : USA  ;
  "Unit0" {
    ClassId = I 241 ;
    ClassName = S "globals.unitclass_headquarter" ;
    Icon = I 0 ;
    Name = S "" ;
  }
}
```

which is the packet's ground truth for the element layout: exactly the four keys the disassembly
reads, in the order the record stores them.

## Reconstruction

`include/bsp/scene_record_side_blocks.hpp`, `src/scene_record_side_blocks.cpp`. The fill is a pure
rule over the parsed property bag; it needs no host because `004E82F0` calls nothing but the key
lookup and the native string resize, and both are the bag itself. Status: reconstructed and
build-tested, exercised against all 259 installed scenes through the probe. Not ABI-compatible and
not game-validated.

The one thing the rule cannot recover is the dword an `E` property resolves to. The file authors
`Party = E Party : Allied`; the record holds the ordinal the enum table assigns. The reconstruction
carries the authored `{type, symbol}` pair and says so; the ordinal table is a follow-up.

## Corrections

- **`docs/MISSION_LOAD_PATH.md` put the side-block array at `record+0h`.** It is at `record+4h`;
  `004DA2D7` stores the vtable `00CE7950` at `+0h`. Every offset inside a block in that doc is four
  too high as a result: what it calls side block `+4h` is block `+0h` and what it calls `+8h` is
  block `+4h`. Corrected there and in `include/bsp/mission_load_path.hpp`, whose
  `kSceneSideBlockSelectorOffset`/`kSceneSideBlockSecondaryOffset` carried the shifted values.
- **`docs/MISSION_LOAD_PATH.md` called `record+988h` the "side-block count".** It is the
  `MaxPlayerNum` property, defaulting to 8 at `004F2145`. The two happen to agree for shipped
  content but they are not the same quantity, and the copy loop is bounded by the property, not by
  the array.
- **The meaning of the two dwords.** That doc recorded the `+4h` dword as "the value that becomes
  the side selector" and `+8h` as "not established". With the base corrected they are `Party` and
  `Race`; the side selector `004DFB70` reads through the participant record is the `Party` ordinal.
- **The ledger name at `004DA2A0` was `CG_array_ctor_helper_004da2a0`** (tag `cg_array_ctor_helper`,
  confidence high, action rename). Old value recorded here; replaced with
  `BSP_SceneRecord_Construct`. It does run a vector constructor iterator, but it is the record's own
  constructor and it runs three of them plus a sub-object.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `scene_enum_ordinals` | 008f2260 0043b8b0 008f3710 | docs/SCENE_ENUM_ORDINALS.md | How an `E` property's `type : symbol` pair becomes the dword at property node `+0Ch`, which is what `Party` and `Race` actually store |
| `scene_record_script_table` | 004f1d70 004dfb70 | docs/SCENE_RECORD_SCRIPT_TABLE.md | The 11 native strings at `record+928h`: which `StageScript*` key lands in which slot, and the 8-or-9 rule that picks one |
| `scene_record_tail` | 004da2a0 004cb420 004c26b0 | docs/SCENE_RECORD_TAIL.md | The record above `+990h`: the sub-object at `+990h`, the intrusive list at `+0C78h` and the fog/camera fields `docs/WORLD_CONSTRUCT.md` already reads |
| `multiplayer_slot_overrun` | 004c6890 004c3840 004bb160 | - | Whether anything clamps `MaxPlayerNum` before `004C6890` runs, or whether a hand-authored value above 8 really writes past `game+18C0h` |

## Uncertainties

- **`Icon` at unit `+8h`.** The key is `00CE8EC8` = `Icon`, and the same literal is reused for the
  first dword of each pool entry. Nothing in this packet reads either field back, so "icon" is the
  authored key's name, not an established role.
- **The enum ordinals.** See the follow-up above. `Party` and `Race` are recovered as authored
  tokens only.
- **`record+98Ch`, `CompetitiveModeParty`.** Written by the same applier from a presence-tested key,
  so it defaults to 0 rather than faulting. No reader was traced in this packet.
- **Uninitialised reads.** `block+0h`/`+4h` for a slot with no `PlayerN`, and every pool `Num`, are
  read from memory nothing wrote. This is what the code does; whether a consumer ever reaches those
  fields was not established.

## no_ghidra_function

| Start | End (inclusive) | Name given |
| --- | --- | --- |
| 004c6750 | 004c675f | BSP_SceneRecordSlotUnit_Construct |
| 004c9800 | 004c981d | BSP_SceneRecordPlayerSlot_Construct |
| 004c9820 | 004c9832 | BSP_SceneRecordPlayerSlot_Destroy |
