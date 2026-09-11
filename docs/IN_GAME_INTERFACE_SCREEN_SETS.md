# The level-1 in-game interface table (packets `cc_main_menu_screens`, `cc_interface_runtime`)

Addresses: 0068aca0, 0068ad48, 0068b390, 0068ad5b, 0068ad7c, 0068ada9, 0068adc9, 0068adea,
0068ae0b, 0068af2f, 0068afab, 0068afb8, 0068b008, 0068b055, 0068b093, 0068b0d4, 0068b115,
0068b153, 0068b1a1, 0068b1e7, 0068b212, 0068b230, 0068b23a, 004f8530, 004d8a50, 0068af68,
0068b255, 0068b282, 0068b2c9, 0068b2f5, 007bb9a0, 004e7bb0, 0054d510, 0054d4c0, 00a7acf0,
00a7b0a0, 00a7e490.

**No longer partial.** The first pass (`cc_main_menu_screens`) read the dispatch, the jump table and
the sixteen published id lists to the instruction and left two regions unread: 0068AE0B..0068AF2E,
the body of the id 20h arm, and 0068AF68 onward, the shared epilogue. Packet `cc_interface_runtime`
read both, and resolved the screen ids inside the lists to their registry classes. Nothing here is
guessed.

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

## The id 20h arm in full, 0068AE0B..0068AF2E

The row above records what id 20h *publishes*. That list is reached only when the payload is null.
With a payload, 20h never touches a setter at all: it is a unit-kind classifier that re-enters the
object's own virtual `+10h` with a different id and then falls into the shared tail.

Each test is `payload->vtable[5Ch](code)`, the unit `IsKindOf` probe
(`docs/UNIT_INSTANCE_UPDATE.md` line 60 establishes both the global and the probe). The first match
wins; the re-entry is `this->vtable[10h](newId, payload)`.

| Test site | Kind code | Re-entered id | Extra condition |
| --- | --- | --- | --- |
| 0068AE1C | 0Eh | 27h INTF_TBOATHELMSMAN | |
| 0068AE3C | 08h | 28h INTF_SUBMARINE | |
| 0068AE5C | 06h | 25h INTF_CAPTAIN | |
| 0068AE7C | 0Fh | 22h INTF_PLANE, else 24h INTF_PLANESPAWN | 007BB9A0(payload) at 0068AE84 decides |
| 0068AEAE | 45h | 2Eh INTF_AIRFIELD | |
| 0068AECE | 1Ch | 2Fh INTF_COMMANDBUILDING | |
| 0068AEEE | 46h | 30h INTF_SHIPYARD_STAREDUMB | |
| 0068AF0E | 18h | 20h again, with `payload->+3D0h` as the new payload | |
| (no match) | | none; straight to 0068B23A | |

`ECX` is reloaded with the payload before every probe except the 45h one at 0068AEAE, which reuses
the `MOV ECX,EDI` at 0068AE80. The 18h arm at 0068AF18 reads `payload->+3D0h` and pushes it **with
no null test**, so a carrier-group host whose child pointer is null re-enters 20h with a null
payload and lands on the publish path below.

The null-payload path at 0068AF2F is the only one that publishes: it redispatches 2Ah
INTF_IDLECAMERA when `game+1FE4h != 0` (0068AFAB), and otherwise calls 004F8530 with
`{29h, 49h, 44h, 35h}` and 004D8A50 with `{4, 11h, 12h, 0Ch, 0Bh}`, then falls straight into the
epilogue at 0068AF68 without passing through the shared tail.

## The shared tail, 0068B23A

Not an arm. Every arm except the 20h publish path falls into it. It decides whether the interface
change should restart the unit's ambient voice line.

```
0068B23A  if (payload == 0)                       goto epilogue
0068B24B  if (!payload->vtable[5Ch](6))           goto epilogue
0068B255  if (id == 29h || 2Bh || 2Ch || 2Dh || 34h) goto epilogue
```

Kind 6 is the same code the 20h arm uses for INTF_CAPTAIN, so only a ship-kind payload carries a
line. The five excluded ids are the four camera interfaces plus INTF_LIMBO; they are exactly
`in_game_interface_suppresses_ambience` in `include/bsp/ingame_interface.hpp`.

```
0068B282  cur = this->+104h
0068B29B  if (cur != 0 && this->+100h == *(payload->+538h + 10Ch))  return true   // already playing it
0068B2B7  if (cur != 0) { cur->vtable[8h](0); 0054D510(&this->+104h) }             // stop, release
0068B2DE  004E7BB0(&this->+100h, payload->+538h + 10Ch)                            // refcounted assign
0068B2E9  if (this->+100h == 0)                   return true
0068B2F5  type  = SoundManager::FindTypeByName("Normal")        // 00CECD20, 00A7B0A0
0068B33E  klass = SoundManager::FindClassByName("3DEffect")     // 00CE7860, 00A7ACF0
0068B34C  tmp   = SoundManager::CreateTrackedSound(&out, this->+100h, klass, type, 0)
0068B359  0054D4C0 BSP_VoiceSoundReference_Assign(&this->+104h, tmp)
0068B35E  destroy the temporary and both strings; return true
```

`payload->+538h` is the unit descriptor (`docs/SCENE_UNIT_CREATORS.md`), so `+10Ch` on it is a
per-unit-class sound resource. The two string constants are the ones the `0068ACA0` lookup reports
as immediates in the body. The pairing follows `docs/VOICE_SLOT_START.md`, where `Normal` is a
**type** and the class is a separate name; the stack arithmetic agrees, and it only closes if
00A7B0A0 is `RET 4` rather than `RET 8`, with the `PUSH 0` at 0068B321 belonging to
`CreateTrackedSound` rather than to the type lookup. Four independent checks confirm that reading:
the exception-state writes at 0068B30C, 0068B328, 0068B354 and 0068B362 all land on the same slot,
and the two `BSP_NativeString_DestroyStorage` calls at 0068B375 and 0068B386 land on the two string
frames.

## The shared epilogue, 0068AF68

Every path ends here, and it does one thing: if the screen holds a playing line it stops and
releases it.

```
0068AF68  cur = this->+104h
0068AF7A  test (cur != 0 ? -1 : 0), 00E19311      // encoded f7 c2 11 93 e1 00
0068AF82  if (cur != 0) { cur->vtable[8h](0); 0054D510(&this->+104h) }
0068AF94  unlink the SEH frame; return AL = 1
```

The `NEG / SBB / TEST imm32` idiom is a null test written strangely: `SBB` leaves 0 or FFFFFFFFh,
and the immediate has bit 0 set, so the `TEST` is nonzero exactly when the pointer is. The constant
is the address-shaped value 00E19311 and 0068B294 uses the same one; nothing explains why the
compiler chose it, and only its low bit matters. Verified against the raw bytes, not the listing.

So the routine returns `true` unconditionally once the base request has been accepted. The only
early `false` is the base rejection at 0068ACC7.

## The published screen ids resolved

The ids inside the sixteen lists are **registry slot ids**, a different space from the interface ids
the jump table dispatches on. Joining the slot table in `docs/IN_MISSION_INTERFACE_MANAGER.md` with
the page table in `docs/HUD_SCREEN_PAGES.md` names all twenty-six of them:

| Screen id | Manager slot | Register virtual | Ledger name | Pages |
| --- | --- | --- | --- | --- |
| 24h | +8Ch | 0067B390 | `BSP_FrontEndScreen_Register` | -- |
| 25h | +70h | 00519830 | `BSP_HudBomberScreen_Register` | `GUI_bomber` |
| 26h | +4Ch | 0051ED60 | `BSP_HudBinocularsScreen_Register` | `GUI_binoculars` |
| 27h | +44h | 0067B3F0 | `BSP_HudCommonOverlayScreen_Register` | -- |
| 29h | +CCh | 00521670 | *(unnamed)* | -- |
| 2Bh | +90h | 00540FE0 | `BSP_HudFreeCameraScreen_Register` | `GUI_freecam` |
| 2Ch | +94h | 0054ED00 | *(unnamed)* | -- |
| 2Eh | +50h | 005468B0 | `BSP_HudShipCrosshairScreen_Register` | `GUI_cross_gunstate`, `GUI_cross_ship` |
| 2Fh | +B0h | 005211B0 | *(unnamed)* | -- |
| 32h | +E8h | 00565EE0 | `BSP_HudLimboScreen_Register` | `GUI_limbo` |
| 35h | +58h | 005BEC50 | `BSP_HudMinimapScreen_Register` | `GUI_minimap` |
| 36h | +98h | 005CC770 | `BSP_HudMovieCameraDebugScreen_Register` | `GUI_moviecamera_debug` |
| 37h | +9Ch | 005CCCB0 | `BSP_HudMovieScreen_Register` | `GUI_movie` |
| 38h | +A0h | 005CCE70 | `BSP_HudEngineMovieScreen_Register` | `GUI_movie` |
| 3Eh | +68h | 00606A90 | *(unnamed)* | `GUI_cross_all`, `GUI_plane_effects`, `GUI_plane` |
| 3Fh | +6Ch | 00607BE0 | `BSP_HudPlaneEffectsScreen_Register` | -- |
| 41h | +74h | 0060D1C0 | `BSP_HudPlaneSpawnScreen_Register` | `GUI_plane_spawn` |
| 44h | +40h | 00645F10 | `BSP_InGameHudRootScreen_Register` | `GUI_powerups`, `GUI_unit`, `GUI_selector` |
| 45h | +78h | 0064BB90 | *(unnamed)* | `GUI_ship`, `GUI_repair`, `GUI_ship_effects`, `GUI_ship_damage` |
| 46h | +7Ch | 0064A360 | `BSP_HudScreenSlot46_Register` | -- |
| 47h | +84h | 00650E00 | `BSP_HudPeriscopeScreen_Register` | `GUI_periscope` |
| 48h | +88h | 00650170 | *(unnamed)* | `GUI_sub` |
| 49h | +B4h | 0067BE80 | *(unnamed)* | -- |
| 4Ah | +80h | 0067C430 | `BSP_HudBinocularsOverlayScreen_Register` | `GUI_binoculars` |
| 4Dh | +48h | 0063E280 | `BSP_HudMarkersScreen_Register` | `GUI_markers` |
| 50h | +C4h | 00682740 | `BSP_HudWarningScreen_Register` | `GUI_Warning` |

Reading the lists with that table in hand: `29h, 49h, 44h` opens ten of the sixteen and is the HUD
root plus two page-less screens; `35h, 50h` closes seven and is the minimap plus the warning
banner; `27h, 4Dh` is the common overlay plus the markers layer. The per-vehicle screens are the
distinguishing part of each list - 45h and its four ship pages for INTF_CAPTAIN, 47h/48h for
INTF_SUBMARINE, 3Eh for INTF_PLANE.

Six register virtuals have no ledger name. Slot 45h is plainly the ship HUD and 3Eh the plane HUD
from their page sets; 29h, 2Fh, 49h and 2Ch load no page at all. Those addresses belong to packet
`hud_screen_pages` and are not renamed here.

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
- **The id 20h row publishes only on the null-payload path.** The table above reads as if 20h always
  installs `{29h, 49h, 44h, 35h}` when `game+1FE4h == 0`. With a payload the arm is a kind
  classifier that reaches no setter at all, matched or not. The row is now qualified above.
- **Both "unread" regions were already modelled in C++.** `include/bsp/ingame_interface.hpp` carries
  `choose_scene_interface_0068ae0b`, the eight `kUnitType*` codes and the ambient-sound tail, and
  `src/ingame_interface.cpp` implements them; this document's Uncertainties said neither was
  analysed. The independent listing read done for `cc_interface_runtime` agrees with that model on
  every branch, including the five ids at 0068B255 that
  `in_game_interface_suppresses_ambience` excludes. Only the doc was stale.
- **One divergence in that model.** `choose_scene_interface_0068ae0b` gates the 18h case on
  `query.unit_is_kind_of(18h) && query.has_delegate_unit()`, so a null delegate leaves `chosen`
  false and falls to the tail. The listing at 0068AF18 pushes `payload->+3D0h` with no test, so a
  null delegate re-enters 20h with a null payload and takes the publish path. The `&&` cannot
  express that, and the edge is unreachable only if `+3D0h` is never null for a kind-18h unit, which
  nothing establishes.
- **The tail is longer than "tests the payload".** The previous text stopped at the `+5Ch(6)` probe.
  0068B255 adds a five-id exclusion, and 0068B282..0068B38B is a whole voice-line block.

## Uncertainties

- **Six register virtuals have no ledger name** (29h, 2Ch, 2Fh, 3Eh, 45h, 48h, 49h). Their slots,
  vtables and pages are known; the class identities are not, and those addresses belong to another
  packet.
- **The per-arm sub-object calls are unidentified.** `this+68h`, `+6Ch`, `+70h`, `+74h`, `+78h`,
  `+7Ch`, `+80h`, `+84h`, `+8Ch` and `+B0h` are each handed the payload by one or two arms; none of
  those routines was read.
- The **id-forcing at 0068ACE4** is recorded from the instructions; what `payload+5Dh` means is not
  established.
- **007BB9A0** decides INTF_PLANE against INTF_PLANESPAWN. Its head reads `unit+C0Ch`, `unit+AA0h`
  against 00D7A218 and `*(unit+DECh)+44h`; the body was not read, so "in flight" is a hypothesis
  from the call site, not from the routine.
- **The `TEST` mask 00E19311** at 0068AF7A and 0068B294 is unexplained. Only its low bit can matter
  given the `SBB` that feeds it, so the test is a null test, but the constant is address-shaped and
  nothing accounts for it.
- **The sound-lookup pairing** at 0068B2F5..0068B33E rests on 00A7B0A0 being `RET 4`. That follows
  from four stack cross-checks and from `docs/VOICE_SLOT_START.md`, not from a Ghidra prototype;
  none of the three sound routines has one.
- **`payload->+538h + 10Ch`** is read as a per-unit-class sound resource because
  `docs/SCENE_UNIT_CREATORS.md` establishes `+538h` as the descriptor. `+10Ch` on the descriptor was
  not otherwise traced.

## Follow-up packets

- `in_game_interface_subobjects` — the ten `this` offsets the arms dispatch the payload to.
- `hud_screen_class_names` — the six unnamed register virtuals in the table above, 00521670,
  0054ED00, 005211B0, 00606A90, 0064BB90, 00650170 and 0067BE80.
- `plane_in_flight_predicate` — 007BB9A0 and the fields it reads, to replace the hypothesis behind
  the INTF_PLANE / INTF_PLANESPAWN split.
- `unit_descriptor_voice_resource` — `descriptor+10Ch` and the `3DEffect` / `Normal` sound class and
  type it is played through.
- `in_game_interface_delegate_null` — whether a kind-18h unit can carry a null `+3D0h`, which is the
  one place the C++ model and the listing part company.

## `no_ghidra_function`

**Resolved.** 0068ACA0 now has a Ghidra function, `BSP_InGameInterface_ApplyPendingInterface`, body
0068ACA0..0068B38F, with the 22-dword jump table at 0068B390..0068B3E7 left outside it. The first
pass's entry below is kept for the record.

none — every routine read for the `cc_interface_runtime` extension has a Ghidra function.

| Start | End (inclusive) | Note |
| --- | --- | --- |
| 0068aca0 | 0068b38f | *(closed)* defined since the first pass; last instruction 0068b38b, jump table at 0068b390 |
