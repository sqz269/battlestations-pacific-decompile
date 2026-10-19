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

Each of the 47 read by hand turned out to be game code. The filter is not complete: rev-convoy's
spot check of the rows it dropped found game destructors with no global, string or game call
(batch 2 below), and the widened vtable scan finds more of that kind. What no scan here covers:
a game body that stores no vtable, touches no global or string, and whose game callees are all
named like element callbacks.

**Destructor rule (the same wording as docs/CC13_CENSUS9.md).** A class's out-of-line destructor
is rebuilt, including the form `MOV [ECX], vtable; JMP base`. A scalar or vector deleting
destructor is bound, citing the out-of-line copy when it has a record; otherwise cite the called
routine, or rebuild when there is none.

## Batch 1 (38 rows)

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

## Batch 2 (2 rows)

- **00996200 BSP_AngleToleranceCondition_IsMet** (`angle_within_tolerance_00996200`). Ghidra
  named it `Unwind@00996200`, but it is no funclet: it has no EH frame, no parent in the index,
  and its only reference is slot 0 of the vtable at 00D1EA30. It is rebuilt rather than bound
  as a funclet. d = 00438B10([this+1Ch], [this+18h]->vtable[50h]()); |d| is d when d > 0, else
  the image's -0.0f at 00D7A208 minus d; the result is [this+20h] > |d|.
- **007B1A80 BSP_Point3PtrArray_Destruct** (`point3_ptr_array_destruct_007b1a80`). Game code:
  the class's own out-of-line destructor (vtable 00D053A4), freeing each owned element through
  a checked index, then the buffer. The `<PathBase>` string the filter saw is in the next
  routine, 007B1B00 (the dump ran past the end).

## Named bodies (3 rows)

Rule-1 check for each of the four named bodies: `bsp.py lookup`, `docs-for`, `git grep` by
address, by name and by behaviour across `src/` and `include/`. None has a host body: `src/`
holds only hooks (`cc13_seg47.hpp`, `cc13_seg48.hpp`, `cc13_seg41_land.hpp` for 007CFD20;
`cc13_big47b.hpp` for 0099A880; `cc13_seg67b.hpp` for 0099BE30) and field-default comments
in game_hosts_units.cpp that cite single 007CFDxx/007D01xx stores. 00816B00's send side is in
no file (docs/UNIT_STATE_MESSAGE.md covers the apply side). Three are rebuilt here:

- **0099A880 BSP_PilotBot_Construct**: 0072BBD0 with the robots table [00F8A30C]; vtables
  00D1F348 / +1Ch 00D1F32C; +58h..+6Ch zero; two 1Ch slots at +84h; +70h 0.0f, +74h -1.0f,
  +78h 0.24f (00E0E2EC), +7Ch 0, +80h 256.0f.
- **0099BE30 BSP_PilotPlan_Construct**: three C8h slots, +2F0h = base with its +538h/+9D4h,
  +2E0h = 7, the control constants (1.0f, 0.12f, 0.24f), then 0099B450.
- **00816B00 BSP_UnitInstance_SendShipSyncMessage**: MT_SHIP_SYNC every call (full when the
  per-holder counter reaches 3*phase+1 or the state's +0Fh is set), then MT_SHIP_GUNS_SYNC for
  phase 1..4, or for phase <= 0 when the state's +18h or +19h is set (phase 0 scans the gun
  parts for damage first). The state's +19h takes the damage result and +18h takes phase > 0.

## Batch 2, part A: rows the filter dropped (21 rows)

rev-convoy's scan of the 548 filter-dropped rows (`local/mistag_dropped_vt.py`, a copy of its
mt_dropped_vt.py) and the widened scan `local/mistag_vtscan2.py` (every open library-tagged row
of the 16 segments; also the `C7 41 d8` and `8D 49` forms) agree on the six VT+JMP destructors.

Rebuilt (out-of-line destructors):

| Address | Name | Body |
|---|---|---|
| `00728B80` | BSP_SessionMessage_GUN_CREATE_Destruct | 00CFDE1C; JMP 007668E0 |
| `00858150` | BSP_EffectInstanceBase_Destruct | 00D0C88C, +8h 00D0C888; JMP 00BD30F0 |
| `009A1BF0` | BSP_BotApproachCloseToShip_Destruct | 00D1F470; ECX+2Ch; JMP 0064A610 |
| `009BE2B0` | BSP_BotApproachMoveTo_Destruct | 00D209CC; ECX+30h; JMP 0042BCA0 |
| `007B1BD0` | BSP_PathComponent_Destruct_007b1bd0 | 00D053D8; JMP 007B1A80 |
| `007B1BE0` | BSP_PathComponent_Destruct_007b1be0 | byte-identical copy of 007B1BD0 |

Bound, each scalar deleting destructor with the out-of-line copy it inlines: `00729020` (inlines
00728B80), `007B2220` and `007B2920` (inline 007B1BD0), `00858220`, `00873060` and `00873420`
(inline 00858150), `009A1C70` (inlines 009A1BF0), `009BE2E0` (inlines 009BE2B0), `006D2EE0`
(inlines 006D1460), `007B3450` (inlines 007B3400), `007B3E40` (inlines 007B3E20), `00862630`
(inlines 00861B30). Three whose out-of-line copy is an undefined chunk cite the routine their
body reaches: `00729510` (chunk 00727B50; 004704B0 on +4h), `0085EB60` (chunk 0085EA70;
006E2A20), `00865CB0` (chunk 00865B50; the pool pair 00419CC0 / 00BD1510).

## Left open (4 rows)

| Address | Size | What |
|---|---|---|
| `007CFD20` | 1573 | BSP_PlaneUnitInstance_Construct (no host body; see above) |
| `007D03A0` | 532 | plane unit-instance destructor, vtable 00D05F20 (tagged CG_vector_deleting_dtor) |
| `0083B120` | 560 | gameplay tuning settings destructor, vtable 00CE3994 (include/bsp/gameplay_settings.hpp has no destructor), caller 00424C20 |
| `0086C240` | 2763 | the Flare effect component's Lua reader (docs/GAMEPLAY_EFFECT_COMPONENTS.md row Flare) |

## Uncertainty

- The getters' owning classes and slot positions are not established; names follow the string.
- 00737110 tests this for null and would store through address 0; the C++ takes a non-null object.
