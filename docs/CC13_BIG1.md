# cc13-big1: the large rows deferred by cc13-small2

Packet cc13-big1 takes the large rows that docs/CC13_SMALL2.md deferred. Code is in
`src/cc13_big1.cpp` and `include/bsp/cc13_big1.hpp`. It is registered on bsp_core and
is not wired into the game.

Every name is a hypothesis, not a recovered symbol. Each body is a new C++ interface over
raw native records, not a drop-in replacement of the original ABI. Every call the image
makes goes through a named `Big1Host` method, at the same point and with the same
arguments. Checked-iterator failures go to the CRT invalid-parameter hook at the same
points as in the image.

Status: reconstructed and build-tested. Nothing here is fixture-tested, ABI-compatible or
game-validated.

## Batch 1

| Address | Name | ABI | Notes |
| --- | --- | --- | --- |
| 004C7AA0 | BSP_World_GetBorderZoneApproach | thiscall(world; out, position, radius, side) RET 10h | 004C7730 for the nearest zone; out = {zone, direction, edge point - radius * normalize(direction), normalize(2 * direction + the x/z unit vector to the corner average)} |
| 005AAC00 | BSP_HudMapIcon_RefreshCarriedUnitIcons | thiscall(icon; template widget) RET 4, EAX | builds a std::set of the HUD icon indices of the classes that the unit's support sites supply; clones one widget per index onto icon+B8h; returns the set size |
| 0064AE20 | BSP_HudShipScreen45_ApplyUnit | fastcall(screen) | VehicleClass[..].NoRepairGUI; throttle seed +44h; gauge flags +104h..+107h and their widgets; 0064AAD0, 0064A9F0 and 0064A960 |

Precision and register notes:

- **004C7AA0.**
  - ECX is not read. It is passed through to 004C7730, which uses it as the world.
  - The y term of the first length is the FLDZ left on the x87 stack at 004C7B2A, so
    the length is (x² + 0·0) + z².
  - Both reciprocals follow FCOMI/JBE, so a zero or NaN length gives 0.
  - The DC-form ops (004C7B2C, 004C7BBA, 004C7C08, 004C7C7C) were read from the bytes
    with tools/x87_dc_form_scan.py.
  - Constants: 0.25 (00D7A348), 0.0 (00D7A258) and 2.0 (00D7A308), all doubles.
  - Modelled at 24-bit precision. The only caller, 0094BFF0, may run on a fixed-step
    job. At 53 bits the unstored partial sums and the constant products would round
    once, at their stores. Which precision applies is uncertain.
- **005AAC00.**
  - The set is a std::set<unsigned> (005A18E0 compares unsigned keys). Its node key at
    +0Ch is passed to vtable[88h] as a zero-extended word.
  - The two list teardowns are inlined in the image and are reproduced node by node.
  - The set's proxy dword is value-initialised here; the image leaves it unset.
  - When the clone is null, the iterator is still advanced.
- **0064AE20.**
  - The constants are 0.5 (00D7A280), 1.5 (00CE3D78) and the float 1.0 (00D7A24C).
  - Each step of (unit+980h + 0.5) / 1.5 rounds to float at 24 bits. A NaN passes both
    clamps unchanged (FCOMIP/JBE, then COMISS/JBE).
  - The bytes pushed for vtable[34h] carry stale upper EAX bits. Only the low byte is
    modelled.
  - +104h is written only on the vtable[5Ch](8) arm.

## Batch 2

| Address | Name | ABI | Notes |
| --- | --- | --- | --- |
| 00641910 | BSP_InGameHudMarkers_AddKind1CMarker | thiscall(markers; unit, is_self, is_friendly, is_target) RET 10h | the IsKindOf(1Ch) sibling of 00642040; on screen a 0063C620 box marker, off screen a 0063D1E0 arrow marker |
| 005E12A0 | BSP_MultiGameLobby_LoadWeatherTable | thiscall(lobby screen) | a private Lua state runs Weather.lua; each Weathers entry with a UniqueID is pushed onto screen+2E0h with its SubScenes ID and Text strings |

Notes:

- **00641910.**
  - It takes `Cc13Pkt642Host` (include/bsp/cc13_pkt642.hpp): every callee already has a
    hook there. 0063C620 and 0063D1E0 are called as the pkt642 bodies, as 00642040 does.
  - The clip point's y is [class+A8h] + y. The bounds (0063A6C0) and the off-screen
    arrow use the raw unit position.
  - record+4 is left unwritten when is_friendly and is_target are both set. Otherwise
    it is set when neither is_target nor is_self is set.
  - On screen, the colour is read from 0077A2F0's out buffer. Off screen, it is read
    through the returned pointer, after 00923BE0 and the copy of the distance string.
  - The marker's show flag is set when markers+1Ch is set and the unit's vtable[140h]
    value equals that object's.
  - 0063D1E0 receives is_self as a whole dword.
- **005E12A0.**
  - The LuaStateOwner is a 4CCh-byte stack buffer, the room between [B+E8h] and the EH
    record.
  - The pushed 24h-byte entry has only its two vectors' first/last/end dwords zeroed;
    +0, +4 and +14h are not written.
  - The id is stored after the checked subscript of the last element, even when that
    check calls 00BF6713.

## Batch 3

| Address | Name | ABI | Notes |
| --- | --- | --- | --- |
| 006697E0 | BSP_HudSupportManager_CollectCatalog (new name) | fastcall(screen), EBP frame | for the current team's units in the selected row's unit list: copies the row map (+40h to +50h, +4Ch), reports InferiorFailure units, and appends their 0084D560 supply classes to list +370h |
| 0066A8A0 | BSP_HudSupportManager_RebuildLists (new name) | fastcall(screen) | rebuilds entry list +3E0h from own units (kinds 6 and 0Fh) and the [00F89B3C] records; unit lists +3ACh, +3B8h and +3C4h; the scroll reset; copies +3E0h to +3ECh |

Notes:

- **006697E0.**
  - The selected row is the checked iterator at screen+350h/354h. Its value
    {+8h, a unit list at +0Ch, +18h} is copied, and the list is copied again; the first
    copy is freed by 005A0B80.
  - A unit is taken when its +54h or its +344h object's +54h is the team.
  - The map +40h is probed with 0065B160, and the node is then found again by a linear
    scan from begin.
  - Map +50h is replaced only when its node is a different object: 006641B0 erases
    begin..end, then 0065CE20 copies.
  - The InferiorFailure report goes through unit+3Ch's vtable[198h]. Its bool out skips
    that unit's supply classes.
  - The loop runs while its counter is below the copy's current size.
- **0066A8A0.**
  - ingame_gui.cpp calls this routine `rebuild_list_c_0066a8a0` and calls +3E0h "list C".
    The doc names the lists A (+3E0h), B (+3ACh), C (+3B8h), D (+3C4h) and E (+3ECh).
  - Entries are inserted at the front. The kind-6 insert is inlined; the kind-0Fh insert
    calls 006652C0.
  - The C/B scan's owner checks compare an address with itself (CMP EAX,EAX) and cannot
    fire, so they are omitted.
  - 006681D0 and 00668350 take a trailing by-value tag dword whose stack contents are
    stale. It is passed as 0.
  - The 14h-byte entry's bytes beyond those written keep whatever the stack held.

Adoption notes:

- **004C7AA0.** cc13_large61's hook `game_query_004c7aa0` (0094BFF0) can forward to this
  body. Its zone record is the native 44h record. src/world_map_bounds.cpp models
  004C7730 over a BorderZoneSet, and a host can bind the 004C7730 hook to that.
- **005AAC00.** cc13_big5ad's hook `hud_map_icon_refresh_carried_unit_icons_005aac00`
  (005ADDA0) can forward to this body.
- **0064AE20.** cc9's `game_hosts_hud.cpp` (near line 385) records
  `HudShipScreen::apply_unit_0064ae20` under `kHudShipScreenGaugesBound`, as a labelled
  substitution: the flags keep the enter's clear values. This body is the replacement.
  The hooks in cc13_sweepB2 and ingame_gui_cc13 can forward to it.
- **00641910.** Not hosted. hud_markers_runtime.cpp (near line 268) lists it as follow-up
  work.
- **005E12A0.** frontend_screens_cc13's `load_weather_table_005e12a0` hook can forward to
  this body. scene_contents_hosts reads the same table for a different routine.
- **006697E0 and 0066A8A0.** seg27_cc13's `support_collect_catalog_006697e0` and
  ingame_gui's `rebuild_list_c_0066a8a0` hooks can forward to these bodies. cc9 does not
  host either.
