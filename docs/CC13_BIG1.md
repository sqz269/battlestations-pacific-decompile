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
