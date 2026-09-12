# The fixed-step countdown at `00F874B8` (`008079B0`)

Addresses: 008079b0 00f874b8 00d7a2b0 00d08e6c 00f874bc 00807a50 00803b80 00803ba0 006f23c0 006f4d10 008053c0 008050e0 008034e0 008073c0 00806b10 00805240 00875bb0

`docs/IN_MISSION_SUBSYSTEM_TICK.md` left `fixed_step_countdown_008079b0` as a follow-up: the first
of the sixteen per-step calls maintains a countdown global and nothing else was known. This packet
reads the routine, the global, its reload constant and the three objects it services, and
reconstructs it as `include/bsp/fixed_step_countdown.hpp` / `src/fixed_step_countdown.cpp`. Names
are hypotheses, not recovered symbols; the strings quoted from the image are not.

## The routine

`__stdcall void(float step)`, body `008079B0..00807A41`, two `RET 4` exits (`00807A3A`, `00807A41`).
`ECX` is not read, so the site's `this` is irrelevant. Called from two places:

| call site | containing function | argument |
| --- | --- | --- |
| `00875CA5` | `00875BB0` `BSP_Game_RunFixedSimulationSteps` | the float at `00D0DE84` = `0.05f`, the fixed step |
| `00807A66` | `00807A50` | `0.0f` (`FLDZ`, `FSTP [ESP]`), after `00807A50` has already forced the countdown negative |

### The rule, `008079B0..008079ED`

```
008079b0: FLD  [00F874B8]      ; acc
008079b6: FSUB [ESP+4]         ; acc - step
008079ba: FSTP [00F874B8]      ; acc = acc - step          (rounded to float)
008079c0: FLD  [00F874B8] ; FLDZ ; FCOMI ST0,ST1 ; JBE 00807a3d
                               ; returns when 0.0 <= acc, i.e. runs only when acc < 0
008079cc: FXCH ; FADD qword [00D7A2B0] ; FSTP [ESP+4]
                               ; reload = (float)(acc + 3.0), computed in the x87 register
008079d8: FLD [ESP+4] ; FCOMIP ST0,ST1 ; FSTP ST0 ; JBE 008079ea
008079e2: XMM0 = reload   (reload > 0.0f)    /    008079ea: XMM0 = 0.0f
008079ef: [00F874B8] = XMM0
```

So: subtract the step; if the accumulator is still `>= 0` do nothing; otherwise reload it to
`max(acc + 3.0f, 0.0f)` and run the pass below. `JBE` also takes the early exit for a NaN
accumulator. The reload keeps the period phase-correct: the overshoot stays in the accumulator
unless a single step overran the whole period, in which case the clamp to `0.0f` makes the pass run
again on the next step.

| constant | address | value | evidence |
| --- | --- | --- | --- |
| period | `00D7A2B0` | `3.0` (double, `00 00 00 00 00 00 08 40`) | `008079CE FADD qword ptr [00D7A2B0]`. It is the image's shared `3.0` literal, read from 117 sites |
| "expire now" | `00D08E6C` | `-1.0e-4f` (`17 B7 D1 B8`) | `00807A52` and `00803B80` store it into `00F874B8` |
| step | `00D0DE84` | `0.05f` | the argument at `00875CA5`; already `kFixedSimulationStepFloat` in `bsp/in_mission_subsystem_tick.hpp` |

At `0.05f` per fixed step the pass runs every **60 steps**, i.e. every 3 seconds of simulated time.

### The pass, `008079ED..00807A3A`

```
EDI = 00F874BC
do {
    ESI = [EDI];
    if (ESI != 0) {
        008073C0(ESI);                                   ; 00807A08, ECX = ESI
        if (ESI->byte_25h != 0)
            00806B10(ESI, [[00E188A8 + 1A08h] + 4]);     ; 00807A24, ECX = ESI, one stack argument
        ESI->byte_25h = 0;                               ; 00807A29, cleared even when it was clear
    }
    EDI += 4;
} while (EDI < 00F874C8);
```

`00F874BC..00F874C7` is a three-entry pointer table. `EDI` starts at `00F874BC` and the bound is the
`CMP EDI,0xF874C8 / JL` at `00807A30`, so the slots are `00F874BC`, `00F874C0` and `00F874C4`. The
dirty byte at `+25h` gates the second call only; the rebuild runs on every period for every present
slot.

## The three slots

| routine | body | role |
| --- | --- | --- |
| `008053C0` | `008053C0..00805423` | `__fastcall void(int index)`. If `[index*4 + 00F874BC]` is null, `operator new(12A0h)`, construct with `008050E0(this, index)` and store it in the slot. Lazy, one object per index |
| `008050E0` | `008050E0..0080522C` | the constructor. Installs vtables `00D08E94` (`+0h`) and `00D08E88` (`+14h`), builds three arrays of `61h` elements of `0Ch` bytes at `+34h`, `+4C0h` and `+94Ch` through the MSVC array-constructor helper `00BF7CD1`, then zeroes `+DD8h..+E10h`: five `{count, head, tail}` triples |
| `00805240` | `00805240..008053BC` | the destructor. Clears its own table entry through its stored index (`00805272` reads `[this+28h]`, `00805285` writes `[EAX*4 + 00F874BC] = 0`), clears the five triples through `008042B0` and destroys the three arrays |
| `008034E0` | `008034E0..008034FF` | destroys all three slots: walk `00F874BC..00F874C8`, `obj->vtable[0](1)` on each |
| `00803BA0` | `00803BA0..00803BD7` | `__fastcall void(int index)`, marks one slot dirty: `[index*4 + 00F874BC]->byte_25h = 1`, then reads `[00E188A8 + 18ECh]` and the local-player slot array |

Each slot is `0x12A0` bytes and holds three `97`-entry arrays of `0Ch`-byte elements. `97` is `0x61`,
one more than the largest class id seen in the `IsKindOf` sweep of
`docs/LOCAL_PLAYER_UNIT_LISTS.md`, and `0Ch` is the `{count, head, tail}` stride, so the arrays are
plausibly per-class unit lists and the three of them plausibly the relations the report names.
**Uncertain:** both readings come from the counts and the stride; no indexing site was read, and the
report has four relations (`own`, `enemy`, `neutral`, `unknown`) against three arrays.

The five triples at `+DD8h..+E08h` are the same five lists `004C3CB0` walks through
`[[game+18CCh + slot*4] + 30h]` (`docs/LOCAL_PLAYER_UNIT_LISTS.md`). `008073C0` clears all five at
`008073D0..00807416` and repopulates them; at `00807634..008076E2` it moves entries from triple 1
(`+DE4h`) into triple 3 (`+DFCh`), allocating a fresh `0Ch` node per entry and freeing the old one.
So the lists `004C3CB0` snapshots once per scene load are rebuilt every three seconds by this pass.

## What the pass publishes: the Lua `recon` table

`00806B10`, `__thiscall void(slot, luaInstance)`, body `00806B10..00806CCD`, `RET 4`. The argument
is `[[00E188A8 + 1A08h] + 4]`, read at the call site (`00807A13..00807A21`): the Lua instance whose
`+4h` holds the `lua_State*`, the same owner `docs/RECON_VALUES.md` resolves for
`install_recon_values_00803a40`.

| call | helper | argument | meaning |
| --- | --- | --- | --- |
| `00806B37` | `006B8190` `BSP_LuaInstance_PushGlobalTableIfNil` | `"recon"` (`00CE74A0`) | push the global `recon` table |
| `00806B45` | `00803750` `BSP_ReconTableScope_ConstructIndexed` | `[slot+28h]` | descend into `recon[index]`, the slot's own table index |
| `00806B59` | `006B8390` | `"enemy"` (`00D08E64`) | set that field, `lua_pushstring` + `lua_pushnil` + `lua_settable` |
| `00806B6F` | `006B8390` | `"own"` (`00D08E60`) | the same |
| `00806B71` | `006B8390` | `"neutral"` (`00CE5604`) | the same |
| `00806B7D` | `006B8390` | `"unknown"` (`00CEB808`) | the same |
| `00806B96` | `00A673E0` `lua_settop` | `-2` | the scope's inlined cleanup |
| `00806B9D` | `006B8210` `BSP_LuaInstance_PopTable` | none | pop `recon` |

`docs/RECON_VALUES.md` already describes the shape this writes into:
`recon[0..2] -> {enemy, neutral, unknown, own} -> nineteen category tables` (`mothership`,
`destroyer`, ... `landfort`, `airfield`, `shipyard`, `path`). The three slots of this table are
therefore the three `recon` indices, and `[slot+28h]`, the index `008053C0` constructed the slot
with, is the key. The periodic pass is the **recon table refresh**: rebuild each context's lists,
and, when the context was marked dirty, rewrite its `recon[index]` subtree for the script.

**Partial:** only `00806B10..00806BAE` was read, one full push/clear/pop cycle. The body repeats
from `00806BA2` with a second `"recon"` push, and what fills the four relation tables with values is
in the part not read.

## Who else writes the countdown

| site | containing function | write |
| --- | --- | --- |
| `00807A5E` | `00807A50`, body `00807A50..00807A6B` | `= -1.0e-4f`, then calls `008079B0(0.0f)`, so the pass runs immediately. Callers: `004DFB70` `BSP_Game_LoadMissionScene`, `008AADF0`, `00925F20` `BSP_SEntity_InitAll` |
| `00803B88` | `00803B80..00803B90`, no Ghidra function | `= -1.0e-4f` and nothing else: arm the pass for the next step |
| `006F23C3` | `006F23C0..006F23CB`, no Ghidra function | `= 0.0f`, which the next step drives negative: the same arming, one step later |
| `006F4E33` | `006F4D10` | `= 0.0f` (`XORPS XMM0,XMM0` at `006F4E27`) |

`bsp.py disasm-raw` renders `006F4E33` as a 16-byte `MOVUPS` starting one byte later, which would
have zeroed the countdown *and* all three slot pointers. The Ghidra listing shows `MOVSS
[00F874B8],XMM0`; the raw view had resynchronised mid-instruction (checklist rule 8).

## Coverage

| routine | reconstruction | coverage |
| --- | --- | --- |
| `008079B0` | `step_fixed_countdown_008079b0`, `run_recon_refresh_008079b0` | complete |
| `00807A50` | `force_recon_refresh_00807a50` | complete |
| `00803B80`, `006F23C0` | `arm_recon_refresh_00803b80`, `clear_recon_countdown_006f23c0` | complete |
| `008053C0` | `ensure_recon_slot_008053c0` | complete for the lazy-creation rule; the constructor body is a host call |
| `008073C0` | none, one host method | partial: `008073D0..00807416` (the five clears) and `00807634..008076E2` (the triple-1-to-3 move) were read; `00807419..00807633` and `008076E5..008079A5` were not |
| `00806B10` | none, one host method | partial: `00806B10..00806BAE` was read; the rest of the body was not |
| `008050E0` | none | partial: the array and triple layout only |

## Corrections

| what | was | is | evidence |
| --- | --- | --- | --- |
| `008079B0` | "the countdown global the first per-step call maintains", unread (`docs/IN_MISSION_SUBSYSTEM_TICK.md` follow-up table) | a 3-second periodic driver for three recon contexts, not a per-step accumulator | `008079CE FADD qword [00D7A2B0]` (3.0) and the three-slot loop at `008079F7..00807A36` |
| `00D7A2B0` | listed as one of the packet's addresses, role unknown | the image's shared `3.0` double, used here as the reload period | `008079CE`; 117 other read sites |
| `008050E0` | `CG_array_ctor_helper_008050e0`, a heuristic tag name | `BSP_Recon_ConstructSlot`: the `12A0h` slot constructor, which calls the MSVC array-constructor helper `00BF7CD1` three times but is not one | it installs vtables `00D08E94` and `00D08E88`, takes the slot index, and zeroes the five `{count, head, tail}` triples at `+DD8h..+E10h`. The superseded record is in git history and in the `--replace` output of `bsp.py ledger add-name` |

Nothing else is superseded: no prior reconstruction of these addresses existed.

## Follow-up packets

| packet | addresses | question |
| --- | --- | --- |
| `recon_slot_rebuild` | 008073c0 008042b0 00804e10 00805490 00806480 008065b0 008069a0 | The whole of `008073C0`: what fills the three `97`-entry class arrays and what the five triples separate |
| `recon_report_values` | 00806b10 006b8390 00885a70 | The rest of `00806B10` (`00806BAE..00806CCD`): what it writes into the nineteen category tables under each relation, and whether they are counts or lists. `docs/RECON_VALUES.md` has the table shape but not the values |
| `recon_slot_indices` | 008053c0 00803ba0 00805240 | Who calls `008053C0` and `00803BA0`, and what distinguishes `recon[0]`, `recon[1]` and `recon[2]` |

## no_ghidra_function

Start and inclusive end (the last byte of the final `RET`).

| start | end | what it is | boundary evidence |
| --- | --- | --- | --- |
| `00803B80` | `00803B90` | `00F874B8 = -1.0e-4f`, `void()` | `MOVSS [00F874B8],XMM0` at `00803B88..00803B8F`, `RET` at `00803B90`; `00803B91..00803B9F` are `CC` |
| `006F23C0` | `006F23CB` | `00F874B8 = 0.0f`, `void()` | `XORPS XMM0,XMM0` at `006F23C0`, `MOVSS` at `006F23C3..006F23CA`, `RET` at `006F23CB`; `006F23CC` is `CC` |

## Correction from docs/BOT_FIRE_TARGET.md

Packet `cc2-bot-fire-target` found that `009F5D30` uses the `EAX` returned by `008053C0` as the list
owner, so `008053C0` is not `void`: it returns the slot's object pointer (the one it lazily constructs
into `[index*4 + 00F874BC]`). The row above and the ledger evidence should read `__fastcall void* (int index)`;
the lazy-construction rule is unchanged.

## Correction from docs/RECON_SLOT_LISTS.md (packet cc2_recon_slot_lists)

- **Was:** the slot holds three arrays of 61h elements of 0Ch bytes at +34h, +4C0h and +94Ch
  **Is:** four. The fourth is the carry-over array at +E14h
  **Evidence:** 00805300..0080530C in the destructor destroys slot+0E14h with the same array helper 00BF7C6E and the same {0Ch, 61h, 00804E00} triplet as the three at 00805353, 0080536D and 00805384; 0xE14 + 0x61*0x0C == 0x12A0, the whole object. 008065D5 and 0080543C index it as [slot + classId*0Ch + 0E18h].
- **Was:** 008042B0 unread; the five calls at 008073D0..00807416 described as clears on the strength of the caller
  **Is:** 008042B0 really is a full clear, but only because it loops; the listing tool hides the loop
  **Evidence:** python tools/bsp.py ghidra disasm 008042B0 jumps from 008042E7 CALL 00BF65AC straight to 008042F4 POP ESI. The disk bytes at 008042EC are 83 C4 04 83 3E 00 75 C4: ADD ESP,4 / CMP dword [ESI],0 / JNZ 008042B8. The same omission hides 00804D7A..00804D82 in 00804D20.
- **Was:** the arrays are plausibly per-class unit lists and plausibly the relations; uncertain, because the report has four relations against three arrays
  **Is:** settled. The three arrays are exactly own, enemy and neutral, indexed by class id; the fourth Lua relation, unknown, is triple 3 at +DFCh, drained out of the enemy and neutral triples by detection level
  **Evidence:** 008067EA, 008067AC and 0080676B select +34h, +4C0h and +94Ch from the relation 008065FF computes; 00806BE4/00806C1A/00806C50/00806C86 map enemy, own, neutral, unknown to +DE4h, +DD8h, +DF0h, +DFCh; only 0080769B and 00807819 write +DFCh.
- **Was:** the five triples at +DD8h..+E08h are the same five lists 004C3CB0 walks
  **Is:** they are own, enemy, neutral, unknown and the union of those four; the fifth is never published to Lua
  **Evidence:** 00807933..00807966 appends triples 0, 1, 2 and 3 into +E08h and nothing reads +E08h afterwards; 00806B10 publishes only the first four.
