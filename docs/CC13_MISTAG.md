# CC13 library-tagged game rows (cc13-mistag)

Packet cc13-mistag, worker cc13-seg67n, 2026-10-19. It closes the census9 blind spot: rows the
classifier tagged as library or compiler code that are game code but store no vtable. Segments
40, 47, 48, 52, 53, 67, 59, 51, 58, 30, 41, 42, 50, 61, 54 and 46; struck ranges and
006C5000-006C5FFF excluded. Names are hypotheses, not recovered symbols. Code:
`include/bsp/cc13_mistag.hpp`, `src/cc13_mistag.cpp` (registered, wired into nothing).

## Filter

Scripts `local/mistag_filter2.py` and the tightening pass recorded in `local/mistag_keep.txt`.

1. Open (no reconstruction record), non-funclet rows that carry a library tag or a library name.
2. Disassembled from the disk bytes. A row is a candidate when it calls a named game routine
   that is not an element callback (Construct/Destruct/Assign/Copy/Release/Reserve/Insert/
   BuyNode and the like), stores to or loads a .data global (00E08000-0109FFFF, minus the
   shared-string allocator 00F8AF18), or references a C string that is not an STL error text
   ("vector<T> too long" etc.).
3. Every candidate hand-read from the bytes.

| Segment | Candidates |
|---|---|
| 40 | 3 |
| 47 | 4 |
| 48 | 2 |
| 52 | 1 |
| 53 | 8 |
| 67 | 15 |
| 59, 51, 58, 42, 54 | 0 |
| 30 | 1 |
| 41 | 2 |
| 50 | 1 |
| 61 | 8 |
| 46 | 2 |

All 47 are game code. Blind spot left: a game body whose only game callees are named like element
callbacks, and that touches no global and no string, is still not flagged.

## This batch (38 rows)

Destructor rule as in docs/CC13_CENSUS9.md: deleting wrappers are bound when their inlined body
has an out-of-line copy with a record; otherwise rebuilt.

- **23 getters**, `MOV EAX, imm32; RET`, each the target of one .rdata vtable slot: type names
  (`Torpedo`, `WaterMine`, `Paratrooper`, `MLandFort`, the seven 00951xxx `M*` unit classes),
  pilot-bot plan names (`closetoship`, `LAND`, `conditioned`, `gainaltitude`, `strafe`,
  `takeoff`) and six that return .data objects (00F8A424, 00E08FA0, 00F8A474, 00E08F78,
  00E08F20, 00E08F70). Rebuilt as the table `kTrivialGetters`.
- **4 singleton deleting destructors rebuilt** (no recorded out-of-line base): 00735880 (global
  010904F8), 00735900 (0109029C), 00866090 (00F87658), 00737110 (010904F8, base at +4h, outer
  vtable 00CFE9F4).
- **11 singleton deleting destructors bound**, each citing its out-of-line base: 0072A540
  (00729420), 007BA640 (007B98A0), 007C33E0 (007C33B0), 007DE2B0 and 007EAB80 (007DDFB0),
  00866280 (00865FB0), 00866300 (00866040), 00869CF0 (00869C50), 0086A400 (0086A200), 006D1930
  (006D0E90), 0093F760 (0093F520).

## Left for the next batch (9 rows)

Each is a real game body that the host only reaches as a hook (no body anywhere in `src/`):

| Address | Size | What |
|---|---|---|
| `007D03A0` | 532 | plane unit-instance destructor (tagged CG_vector_deleting_dtor) |
| `0083B120` | 560 | a class destructor (vtable 00CE3994), caller 00424C20 |
| `007CFD20` | 1573 | BSP_PlaneUnitInstance_Construct (hook in cc13_seg47.hpp) |
| `0086C240` | 2763 | Lua reader of an effect component (`Color`, `Color2`, `Distance`) |
| `00816B00` | 377 | BSP_UnitInstance_SendShipSyncMessage |
| `0099A880` | 181 | BSP_PilotBot_Construct (hook in cc13_big47b.cpp) |
| `0099BE30` | 164 | bot-task embedded constructor (hook in cc13_seg67b.cpp) |
| `007B1A80` | 134 | BSP_Point3PtrArray_Destruct (hook in cc13_seg47.cpp) |
| `00996200` | 101 | an `Unwind@`-named vtable slot calling BSP_Math_SubtractWrappedAngle |

## Uncertainty

- The getters' owning classes and slot positions are not established; names follow the string.
- 00737110 tests this for null and would store through address 0; the C++ takes a non-null object.
