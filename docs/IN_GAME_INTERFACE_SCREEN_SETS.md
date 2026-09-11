# The level-1 in-game interface table (packet `cc_main_menu_screens`, partial)

Addresses: 0068aca0, 0068ad48, 0068b390, 0068ad5b, 0068ad7c, 0068ada9, 0068adc9, 0068adea,
0068ae0b, 0068af2f, 0068afab, 0068afb8, 0068b008, 0068b055, 0068b093, 0068b0d4, 0068b115,
0068b153, 0068b1a1, 0068b1e7, 0068b212, 0068b230, 0068b23a, 004f8530, 004d8a50.

**This document is partial and deliberately so.** It was the stretch item of packet
`cc_main_menu_screens`, taken up after the main deliverable
(`docs/MAIN_MENU_SCREENS_RUNTIME.md`) was finished. The dispatch, the jump table and the published
id lists are read to the instruction and are complete. The bodies of the arms beyond their two
screen-set calls are not analysed, and the id 20h arm is only partly read. Nothing here is guessed.

`docs/MAIN_MENU_PATH.md` established that level 1 is the in-mission HUD level and that all but one
of its 18 setter call sites live inside 0068ACA0. This document resolves which interface id each
site serves.

## The dispatch

0068ACA0 is `__thiscall(this, int interfaceId, void* payload)`, `RET 8`, with an SEH frame
(handler 00C7DD88). It calls the base 00684600 first at 0068ACC7 and returns immediately when that
rejects the request, so a rejected request never reaches the table.

Two things happen before the dispatch that can change the id:

- 0068ACE4: when the payload is non-null and its `+5Dh` byte is set, 00565FB0 runs on
  `this+E8h` with `payload+70h` and **the id is forced to 34h**, `INTF_LIMBO` per the existing
  ledger record for this address.
- 0068AD02: 00646040 runs on `this+40h` with the (possibly forced) id and the payload.

Then:

```
0068AD48  lea eax, [ebx - 20h]      ; index = interfaceId - 20h
0068AD4B  cmp eax, 15h
0068AD4E  ja  0068B230              ; out of range: clear the level-1 set
0068AD54  jmp dword ptr [eax*4 + 0068B390]
```

**The table covers interface ids 20h..35h, not 25h..50h.** The packet brief's range was the range of
*published screen ids*, which is a different space: the ids inside the lists run up to 50h, but the
ids the table dispatches on stop at 35h. The 16h-entry jump table at 0068B390 is 22 dwords,
0068B390..0068B3E7.

## The level-1 sets, by interface id

"Screens" is the list handed to 004F8530 `BSP_FrontEndScreens_SetLevel1Set`; "contexts" is the list
handed to 004D8A50, the level-1 input-context setter. Both are varargs terminated by a zero, pushed
right to left, so the argument order is the reverse of the push order. An empty screens cell means
the arm does not call 004F8530 at all and the previous level-1 set survives.

| Id | Arm | Screens (004F8530) | Contexts (004D8A50) | Other calls in the arm |
| --- | --- | --- | --- | --- |
| 20h | 0068AE0B, 0068AF2F | `29h, 49h, 44h, 35h` when `game+1FE4h == 0` | `4, 11h, 12h, 0Ch, 0Bh` | when `game+1FE4h != 0` it re-enters its own virtual `+10h` with `(2Ah, 0)` at 0068AFB4 instead |
| 21h | 0068B230 | *(empty list: clears level 1)* | none | also the `ja` out-of-range target |
| 22h | 0068B008 | `29h, 49h, 44h, 27h, 4Dh, 3Eh, 3Fh, 35h, 50h` | `0Ah, 4, 11h, 12h, 0Ch, 0Bh` | 00608D50 on `this+6Ch`, 00609390 on `this+68h` |
| 23h | 0068AFB8 | `29h, 49h, 44h, 27h, 4Dh, 3Eh, 25h, 26h, 2Eh, 35h, 50h` | `9, 4, 11h, 12h, 0Ch, 0Bh` | 00519B00 on `this+70h`, 00609390 on `this+68h` |
| 24h | 0068B055 | `44h, 27h, 4Dh, 50h, 41h, 35h` | `17h, 4, 11h, 12h, 0Ch, 0Bh` | 0060CF90 on `this+74h` |
| 25h | 0068B115 | `29h, 49h, 44h, 27h, 4Dh, 45h, 46h, 26h, 2Eh, 35h, 50h` | none | 00689FA0 on `this`, 0064DA40 on `this+7Ch`, 0064D590 on `this+78h` |
| 26h | 0068B153 | `27h, 4Dh, 24h` | `5, 4, 0Ah` | 00644230 on `this+40h` twice, 0067B4D0 on `this+8Ch`, result stored to `*(this+8Ch)+34h` |
| 27h | 0068B093 | `29h, 49h, 44h, 27h, 4Dh, 45h, 4Ah, 26h, 2Eh, 35h, 50h` | none | 00689FE0 on `this`, 0067C1A0 on `this+80h`, 0064D590 on `this+78h` |
| 28h | 0068B0D4 | `29h, 49h, 44h, 27h, 4Dh, 45h, 47h, 48h, 2Eh, 35h, 50h` | none | 00689FC0 on `this`, 00651760 on `this+84h`, 0064D590 on `this+78h` |
| 29h | 0068AD5B | `2Bh` | `2` | — |
| 2Ah | 0068AD7C | `2Ch, 29h, 49h` | `4, 11h, 12h, 0Ch, 0Bh` | — |
| 2Bh | 0068ADA9 | `36h` | `3` | — |
| 2Ch | 0068ADC9 | `37h` | `3` | — |
| 2Dh | 0068ADEA | `38h` | `3` | — |
| 2Eh | 0068B23A | *(untouched)* | none | jumps straight to the shared tail |
| 2Fh | 0068B1A1 | `29h, 49h, 44h, 27h, 4Dh, 2Fh, 26h, 2Eh, 35h, 50h` | `6, 4, 11h, 12h, 0Ch, 0Bh` | 005213D0 on `this+B0h` |
| 30h | 0068B23A | *(untouched)* | none | as 2Eh |
| 31h | 0068B23A | *(untouched)* | none | as 2Eh |
| 32h | 0068B23A | *(untouched)* | none | as 2Eh |
| 33h | 0068B1E7 | `29h, 49h, 44h, 27h, 4Dh, 35h` | `16h, 11h, 12h` | — |
| 34h | 0068B212 | `32h` | `13h` | — |
| 35h | 0068B23A | *(untouched)* | none | as 2Eh |

The seventeen 004F8530 sites in the table are 0068AD5F, 0068AD84, 0068ADAD, 0068ADCD, 0068ADEE,
0068AF48, 0068AFD0, 0068B01C, 0068B063, 0068B0AB, 0068B0EC, 0068B12D, 0068B170, 0068B1B7, 0068B1F5,
0068B216 and 0068B232, which is the full count the xref sweep reports inside this body. The
eighteenth site overall, 004DA9B8, is the teardown.

### Three shapes, not one

- **Clear (21h and out of range).** 0068B230 publishes a list containing only the terminator, which
  empties level 1. It is not a no-op.
- **Leave alone (2Eh, 30h, 31h, 32h, 35h).** These five ids jump straight to the shared tail at
  0068B23A and never reach a setter, so whatever level 1 held stays up. Distinguishing these from
  the clear arm is the point of reading the table rather than the xref list.
- **Publish.** The remaining sixteen ids.

`29h, 49h, 44h` opens ten of the sixteen published lists and `35h, 50h` closes seven of them, which
is consistent with the persistent-HUD-core reading in `docs/MAIN_MENU_PATH.md`. Six ids publish no
input contexts at all (25h, 27h, 28h, and the three arms that only clear or leave alone).

## The shared tail, 0068B23A

Not an arm. Every arm falls into it. It tests the payload, calls the payload's vtable `+5Ch` with 6,
and on a true result continues; both false results jump back to 0068AF68, which is also where the
id 20h arm rejoins. 0068AF68 onwards is not analysed.

## Corrections

- **The dispatch range is 20h..35h.** The packet brief gave 25h..50h, which is the range of the
  screen ids that appear *inside* the published lists, not the range the jump table indexes. The
  ledger record for 0068ACA0 already had 20h..35h right; only the brief was wrong. What is new here
  is the per-id attribution of all 17 call sites and the three arm shapes below.
- **Two lists quoted in `docs/MAIN_MENU_PATH.md` are truncated.** That document reads the 0068AFD0
  list as `{29h, 49h, 44h, 27h, 4Dh, 3Eh, 25h, 26h}` and the 0068B0EC list as
  `{29h, 49h, 44h, 27h, 4Dh, 45h, 47h, 48h}`. Both actually carry three more ids, `2Eh, 35h, 50h`,
  before the terminator: eleven ids each, not eight. The stack cleanups confirm it, `add esp, 50h`
  at 0068AFF1 covering twelve screen dwords plus eight context dwords, and `add esp, 30h` at
  0068B0F1 covering twelve screen dwords with no context call.
- **The `ja` default is not a no-op**, which the phrase "17 call sites" in the brief might suggest.
  It shares the 21h arm and clears level 1.

## Uncertainties

- The **id 20h arm** is read only from 0068AF2F, where its session-mode branch sits. 0068AE0B..0068AF2E
  is not analysed, so there may be further conditions ahead of the two lists recorded above.
- **No screen id is named.** The lists are raw registry indices; mapping 29h, 44h, 49h and the rest
  to screen classes needs the registry sweep that `docs/FRONTEND_SCREEN_SETS.md` uses, which this
  pass did not do.
- **The per-arm sub-object calls are unidentified.** `this+68h`, `+6Ch`, `+70h`, `+74h`, `+78h`,
  `+7Ch`, `+80h`, `+84h`, `+8Ch` and `+B0h` are each handed the payload by one or two arms; none of
  those routines was read.
- The **id-forcing at 0068ACE4** is recorded from the instructions; what `payload+5Dh` means is not
  established.

## Follow-up packets

- `in_game_interface_arm_bodies` — 0068AE0B..0068AF2E (the rest of the id 20h arm) and 0068AF68
  onwards (the shared epilogue), the two regions this pass left unread.
- `in_game_interface_screen_names` — resolve the level-1 screen ids in the table above to registry
  classes, so the sixteen lists become readable.
- `in_game_interface_subobjects` — the ten `this` offsets the arms dispatch the payload to.

## `no_ghidra_function`

**0068ACA0 has no Ghidra function.** The body must be defined with
`python tools/ghidra_define_function.py 0068aca0 0068b390` before the name
`BSP_InGameInterface_ApplyPendingInterface` can be applied or the range exported. The 22-dword jump
table at 0068B390..0068B3E7 follows the last instruction at 0068B38B and must stay outside the
function. This is an integrator action; this packet did not take the Ghidra write lock.

| Start | End (inclusive) | Note |
| --- | --- | --- |
| 0068aca0 | 0068b38f | `BSP_InGameInterface_ApplyPendingInterface`; last instruction 0068b38b, jump table at 0068b390 |
