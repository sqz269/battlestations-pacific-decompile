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

## Batch 2, big four

### 0083B120 BSP_GameSettings_Destruct

`game_settings_destruct_0083b120`, `__thiscall(ECX = settings)`, RET. The game settings singleton
(vtable 00CE3994; published at 00F8753C by 00424C40 BSP_GameSettings_GetSingleton); only caller
the scalar deleting destructor 00424C20. Rule-1: include/bsp/gameplay_settings.hpp names the
vtable (`kGameplayTuningSettingsVTableAddress`) but has no destructor, and no file in `src/`
has a body. In listing order:

1. vtable 00CE3994.
2. The RefPtr vector (first +664h, last +668h) by checked index: each non-null slot is released
   (InterlockedDecrement on +4h; zero calls its vtable[0] with no argument) and nulled.
3. The 30h-byte element vector +740h: 0041E7A0 on each, free the buffer; +740h..+748h zeroed.
4. `00BF7C6E(+72Ch, 4, 2, 0041DE40)`: two RefCountedHandles.
5. `0041F370(+6B8h, 0)`, then `00BF6989([+6B8h])` with no null test.
6. Free +674h and +664h (each triple zeroed).
7. The vector +3E4h (first +3E8h): `00421470(ECX = first, EDX = last; vector, settings)`, free.
8. RefPtrs +3A8h then +3A4h, each nulled only when it was set.
9. Free +1E0h (triple zeroed); `[00F8753C] = 0`; base vtable 00CE3818.

### 007D03A0 BSP_PlaneUnitInstance_Destruct

`plane_unit_instance_destruct_007d03a0`, `__thiscall(ECX = plane)`, RET (no flags argument). The
plane unit instance's out-of-line destructor. It is reached from nine scalar deleting destructors
(0074E4C0, 0074E520, 007D1D10, 007D7830, 007DDAC0, 0084CA30, 009534E0, 00953570, 00953600) and
from eight undefined 5-byte `JMP 007D03A0` chunks (0074E2C0, 0074E350, 007D77A0, 007DDA30,
0084C9A0, 00951BE0, 00951CC0, 00951DA0).

Rule-1: no body in `src/` (lookup, docs-for, git grep by address, by name and by the member
calls). The cc9 units host does not model the plane as bytes, but where it cites these offsets
they agree:
- `src/game_hosts_units.cpp` cites the physics body sync at +804h, the controller at `unit+AB0h`
  (007D7DA0, 007D99C0, 007D83D0) and the +C98h sub-block (007DDEA0, 007EB380).
- docs/CC13_BIG47A.md has the four 10h wing-tip records at `plane+A5Ch` and the checked vector at
  `plane+A4Ch` (first A50h, last A54h).
- docs/CC13_GPR_48.md has the effect slot at `unit+E80h`.
- The device at `unit+72Ch` is embedded (docs/BOT_TASK_STATES.md, `LEA ECX,[EAX+72Ch]`), which
  matches the in-place call to 006E25A0.

In listing order:

1. Vtables 00D05F20, +10h 00D05F08, +24h 00D05F00, +170h 00D05EFC, +1E4h 00D05EF4, +310h
   00D05EDC, +38Ch 00D05ED8, +72Ch 00D06130.
2. +804h, when set: `00C34F70(ECX = [[00E188A8]+18h], body)`, then nulled.
3. +E80h, when set: 007E2550 on it, free, null.
4. +AA4h, when set: RefPtr release (InterlockedDecrement on +4h; zero calls vtable[0] with no
   argument), null.
5. Four arrays of four owned pointers at +D9Ch + 10h*k. Their signed counts are at +DDCh + 4*k,
   and each count is decremented in place to 0, freeing from the top. The image forms the address
   as `[EDI + 4*(count + 367h + 4k)]` with EBP = FFFFF58Bh - EDI + ESI.
6. `007EC490(+C98h)`, then `007D7A10(+AB0h)`.
7. `00BF7C6E(+A5Ch, 10h, 4, 007C3680)`.
8. The vectors +A4Ch and then +A3Ch. When first is set: `007CD430(ECX = first, EDX = last;
   vector, plane)` (RET 8), then free. The triple is zeroed either way.
9. `004C9550(+A30h, 0)`, then `00BF6989([+A30h])` with no null test.
10. `006E25A0(+72Ch)`, then 00959940 BSP_UnitGameObject_Destruct on the plane.

### 007CFD20 BSP_PlaneUnitInstance_Construct

`plane_unit_instance_construct_007cfd20`, `__thiscall(ECX = plane; arg, cls)`, RET 8, EAX =
plane (007CFD20..007D0342). The name was already in the ledger (cc2_plane_flight). Rule-1: no body
in `src/`, only the hooks in cc13_seg41_land, cc13_seg47 and cc13_seg48. Constants were read at
their load sites:

| Address | Value |
|---|---|
| 00D7A260 | -1.0f |
| 00D7A24C | 1.0f |
| 00CE4C04 | 10000.0f |
| 00CE65D8 | -100.0f |
| 00CE3804 | 1000.0f |

EDI holds -1 from 007CFD69 to the end, and EBX holds 0. In listing order:

1. `0095CC90(arg, cls)`. Then `006E22D0(+72Ch)`, after which +72Ch takes vtable 00D06130.
2. The seven other vtables, exactly as 007D03A0 restores them.
3. Fields up to the controller:
   - +800h = -1; +808h and +80Ch = 0.
   - `007C4560(+810h, plane)`, then `007BAF90(+84Ch, plane)`.
   - +914h = -1.0f.
   - The bytes +8FCh, +8FDh, +911h, +9C0h, +9C1h, +9C8h, +9C9h and +9E0h..+9E2h = 0; +9C3h and
     +9CBh = 1.
   - +964h and +9B8h = 0.0f; +994h..+9B0h and +9D4h = 0.
   - +9D8h = the dword -1 (007CFE72); +9DCh = 1.0f (007CFE78).
   - The live controls +9E4h..+9F8h = 0 with the throttle +9F0h = 1.0f (007CFEB0).
   - The pilot command block +9FCh..+A10h = 0 with its throttle +A08h = 1.0f (007CFED6); +A14h = 0.
   - +A18h, +A1Ch, +A24h and +A28h = 0.0f; +A20h and +A2Ch = 1.0f.
   - The triples of +A30h, +A3Ch and +A4Ch = 0.
4. Wing-tip records and the controller:
   - `00BF7CD1(+A5Ch, 10h, 4, 007C3670, 007C3680)`.
   - +A9Dh and +AA8h = 0; +AA4h = 0.
   - `007D7EA0(+AB0h, plane)`.
5. Before the fx component:
   - +BB0h..+BC8h = 0; +BCCh..+BE4h = 1.0f.
   - +C04h = 0.0f; +C18h = 5; +C2Ch = 1.0f.
   - +C30h, +C50h and +C58h = 0; the bytes +C41h and +C49h = 0.
   - `007CD520(+C98h)`.
6. After it:
   - +969h = 1; +324h = 2; +C4Ch = 0.
   - +900h = 0; +904h is cleared when it is set.
   - +C10h = 10000.0f; +C14h = -1.0f.
   - +910h and +DFCh = 0; +908h and +90Ch = 0.0f.
   - +BC4h = 1.0f, over step 5's 0.
   - +C5Ch = -100.0f; +C60h = 0.0f.
7. The counter: +968h takes the .data byte 00F87151. The counter steps and is stored, then reset
   to 0 when the signed byte is not below 12.
8. The rest:
   - +DDCh..+DE8h = 0 (the counts 007D03A0 walks).
   - +C00h, +C01h, +C35h, +C3Ah, +BF8h and +C02h = 0.
   - +AACh = 0Ah; +C4h = 0Fh.
   - The unaligned dword +C36h = 0.
   - +7F8h = the plane itself; +770h = 1.
   - +9BCh and +C64h..+C88h = 0.0f; +BF4h = 0.
   - +DF8h = 1; +9D0h, +970h and +96Ch = 0.
   - +E8Ch = 0.0f; +E90h = -1; +BFCh = 1000.0f.
9. The matrices: an identity 4x4 at +E40h, then +E00h = 1.0f, and +E44h..+E7Ch are copied to
   +E04h..+E3Ch through FLD/FSTP. The copy is exact because the values are only 0.0f and 1.0f.

**Host agreement.** These citations were each checked against the bytes and agree:

| Where | Claim |
|---|---|
| game_hosts_units.cpp | +900h zeroed |
| game_hosts_units.cpp | +C04h 0.0 (007D0006), +BF4h, +BF8h and +C02h clear, +BFCh 1000.0 (007D01EB), +C18h 5 (007D000E) |
| game_hosts_units.cpp | the throttle 007CFEB0 and air brake 007CFEA4 |
| game_hosts_units.cpp | +914h -1.0 (007CFDDF) |
| game_hosts_units.cpp | +9D8h -1 (007CFE72) |
| game_hosts_units.cpp | 007D0136 (+C3Ah) |
| game_hosts_units.cpp | 007CFDC6 (007C4560) |
| game_hosts_units.cpp | 007CFD78 (00D05F20) |
| game_hosts_gunnery.cpp | +800h -1 (007CFDB4) |
| plane_death_modes.hpp | +800h -1 (007CFDB4) |
| plane_flight.hpp | +9E4h..+9F8h, +310h at 007CFDA0, +AB0h at 007CFF6A |
| plane_formation.hpp and plane_squadron_host.hpp | +9D0h at 007D01C3 |
| approach_target_ref.hpp | 007CFD78 |

**Two citations disagree with the bytes** (other owners' files, left for the lead):
- `include/bsp/plane_crash_sweep.hpp` says the cooldown +9DCh is "0 from the constructor
  (007CFE78)". 007CFE78 is `MOVSS [ESI+9DCh], XMM1`, and XMM1 holds 1.0f from 007CFDEA
  (00D7A24C); nothing writes XMM1 in between. The gunnery host's stand-in `cooldown_9dc` starts at
  0.0f (game_hosts_gunnery.cpp), and src/cc13_large3.cpp reads +9DCh as `turnEfficiency`
  against 1.0f.
- `include/bsp/pilot_controls.hpp` says the pilot command block is "zero-initialised ... at
  007CFEB8-007CFED6". Its throttle +A08h is set to 1.0f at 007CFED6.

## Left open (1 row)

| Address | Size | What |
|---|---|---|
| `0086C240` | 2763 | the Flare effect component's Lua reader (docs/GAMEPLAY_EFFECT_COMPONENTS.md row Flare) |

## Uncertainty

- The getters' owning classes and slot positions are not established; names follow the string.
- 00737110 tests this for null and would store through address 0; the C++ takes a non-null object.
