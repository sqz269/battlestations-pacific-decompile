# CC13 segment 27 (packet cc13-seg27)

Range 00656250..0067F4D0: the support-manager screen (HUD slot 4Eh, vtable 00CF6B44, register
0066EA60), session message 82h, the support-site lists the AI spawn code also uses, the bomb view
(interface 26h) and two front-end leftovers. Code: `src/seg27_cc13.cpp`,
`include/bsp/seg27_cc13.hpp` (namespace `bsp::seg27_cc13`).

All functions are new C++ interfaces over raw image objects (`ingame_gui_cc13::Raw`). Every call
the native makes is a named host method, and the checked-iterator reports (00BF6713) are kept
where the native makes them. These are build-tested only. None of them is ABI-compatible or
game-validated, and nothing here was run in bsp_game. Names are hypotheses.

## Reconstructed (33)

| Address | C++ | ABI | What it does |
|---|---|---|---|
| 00656250 | `locked_read_50_00656250` | fastcall(obj) | Reads +50h under the lock at +0Ch (depth at cs+18h). |
| 00656280 | `session_message130_construct_00656280` | thiscall, RET 14h | Message 82h constructor. The first stack argument is not read. |
| 00656320 | `session_message130_write_00656320` | thiscall, RET 4 | Message 82h write: bool, 6, 3, a presence bit plus 12-bit word, then 4. |
| 0065A200 | `support_manager_selected_icon_kind_0065a200` | fastcall | 0Fh / 2 / 15h / 14h / 0 from the cursor unit's kind. |
| 0065A4D0 | `support_unit_slot_kind_0065a4d0` | stdcall(iter), RET 8 | 0 (45h), 1 (9), 2 (46h), -1. |
| 0065A550 | `support_manager_update_slot_strip_0065a550` | fastcall | The four slot icons around the cursor (+308h/+30Ch) in list +1ECh. |
| 0065A740 | `support_manager_update_payload_strip_0065a740` | fastcall | The same over +2E4h or +37Ch with 00653200 frames. |
| 0065AB40, 0065AE90 | `support_manager_catalog_entry_*` | fastcall | Node +8h of list +37Ch / +2E4h at index +310h. |
| 0065BBA0 | `support_manager_list_remove_0065bba0` | thiscall, RET 4 | 0065B1F0 on list +1F8h. |
| 0065C9A0 | `support_manager_select_value_0065c9a0` | thiscall, RET 4 | +334h from map +50h, else entity+134h. |
| 0065D070 | `spawn_request_order_members_0065d070` | thiscall, RET 10h | Spawn-request members get 00E08F68 or 00E08F78 commands on the leader +21Ch. |
| 0065EE90 | `support_manager_sync_catalog_cursor_0065ee90` | fastcall | +3A4h / +310h / +314h from +4Ch, then 0065C9A0. |
| 0065EFA0 | `support_manager_sync_weapon_cursor_0065efa0` | fastcall | +2A0h / +32Ch / +330h from map +414h. |
| 00660190 | `support_manager_select_deck_slot_00660190` | thiscall(screen, record), RET 4 | Deck slot row from the unit's slot table, or +220h from the kind-46h vector. |
| 00668570 | `support_manager_selection_health_00668570` | fastcall -> ST0 | Health of the selection, the second list's unit or the controlled unit, else 1.0. |
| 00669170 | `support_manager_select_weapon_value_00669170` | fastcall | +334h from map +408h (the native's unused map copy kept). |
| 0066A130 | `support_manager_build_entries_0066a130` | fastcall -> EAX | Rebuilds the entry lists +2E4h/+37Ch/+394h and their icon states. |
| 0066D020 | `support_manager_refresh_selection_0066d020` | fastcall | The selection refresh: strips, entry, payload icon (+158h) and groups. |
| 0066DD00 | `support_sites_build_0066dd00` | fastcall(list) | (site, units) pairs from game+19CCh lists +16Ch and +3B8h. |
| 0066DF30 | `support_sites_build_with_icons_0066df30` | fastcall(list, EDX depth) | The same with one cloned map icon per pair. |
| 0066E2B0 | `support_sites_filter_0066e2b0` | fastcall(ECX side, EDX pairs), RET 8 | Pairs of a side, optionally able to supply a class. |
| 0066E3C0 | `support_sites_rebuild_and_filter_0066e3c0` | fastcall, RET 0Ch | 0066DF30 then the filter. |
| 0066E510 | `support_sites_collect_0066e510` | fastcall, RET 4 | Local pair list, 0066DD00 and 0066E2B0. |
| 0066E590 | `support_sites_collect_nonempty_0066e590` | fastcall, RET 4 | 0066E510, then empty pairs erased (callers are the AI planner and spawn code). |
| 0066FB00 | `support_manager_refresh_site_icons_0066fb00` | fastcall | Rebuilds +344h and moves each map icon to its first unit's map position. |
| 00672660 | `support_manager_screen_destruct_00672660` | fastcall | The screen's destructor body. |
| 00673130 | `support_manager_update_info_panel_00673130` | fastcall | Info panel visibility, CP text and bars. |
| 006735D0 | `support_manager_screen_construct_006735d0` | fastcall -> EAX | The screen's constructor. |
| 0067B390 | `hud_screen24_register_0067b390` | thiscall | `jmp 004F71D0`. |
| 0067B4D0 | `bomb_view_ensure_camera_0067b4d0` | thiscall, RET 8 | Builds the bomb or torpedo view camera and installs it. |
| 0067B7E0 | `bomb_view_update_0067b7e0` | thiscall, RET 4 | The queued pick, then the camera velocity for the renderer (00B0D6A0). |
| 0067C970 | `rebind_primary_input_0067c970` | void, RET | Removes the first device, configures class 2, sets 00E1987C. |

## Notes, ABI corrections and uncertainty

- 00660190 is `__thiscall(screen, record*)` with RET 4. Ghidra shows no parameter. The only call
  (006779BF..006779C9 in 00675C40) pushes `lea ecx,[esp+D0h]`, and the tail call at 006604F7
  stores its argument in that slot. The record is filled by 0065CA50 and tested by 00658530.
- 0065D070 issues its commands with ECX = the vector element (0065D144 / 0065D1C5 `mov ecx,esi`).
  The decompiler shows them on the leader instead. The leader's kind-0Fh test runs twice on the
  second path, as in the native.
- 00669170 copies the matched inner map into a local (00665340) and destroys the copy unused
  (00664450, free). The walk reads the original. Both steps are kept.
- 00668570 returns the second-list unit's health rounded to a float (00668720 `fstp float ptr`).
  The other two paths return the raw ST0 value. The host returns a double.
- 0066A130 and 00660190 read through a null slot table when the cursor unit pointer is null (the
  native computes 0 and reads [0+44h] / [0+0Ch]). No guard was added.
- 0066A130's last list assign (00668290) passes an uninitialised tag dword ([esp+20h]). The
  reconstruction passes 0.
- The bomb view's 0.25 delay constant is the double at 00D7A348. 0.0 is 00D7A218, 1.0 is
  00D7A24C (float) and 00D7A210 (double). The depth reset in 0066DF30 is -12.0f (00CE781C).
- The `CMP reg,reg` iterator-compatibility tests the compiler left in (0065EE90, 0066A130,
  0066FB00 and others) can never report and are not repeated.
- STL temporaries (push_back of an empty pair, list clear and destroy, tree erase) are spelled out
  call by call: _Buynode, _Incsize and the two link stores. They are not collapsed into a single
  push_back.

## Verified elsewhere, recorded here

- 0065F560: the `sm_launch_unit` player, one row of `kSupportManagerSounds`, reconstructed by
  `play_support_manager_sound` in `src/ingame_gui.cpp`. The body matches the shared shape (guard
  00E19854, class 00E19850, sample +20h, type "Normal", handle released).
- 0067BB50: cc9's host model `GameUnitsHost::Impl::role_screen_update_0067bb50`
  (`src/game_hosts_units.cpp`) is a semantic model. It has no observer register/unregister, the
  page byte +4h is labelled and the slot permissions are modelled. It belongs to cc9 and was left
  as is.

## STL instantiations (named, not reconstructed)

00667EB0 (`map<u16,int>::operator[]`, node isnil +15h), 0066E610 (`map<u16, map>::operator[]`,
isnil +1Dh), 0066FC30 (`map<u16, map>::operator[]`, isnil +21h) and 0067D5A0
(`wstring::assign(const wchar_t*, size_t)`, already a host method `wide_string_assign_n_0067d5a0`).

## Skipped

- 00675C40 (5595 instructions): the support-manager per-frame update. It is cc9's
  (docs/CC13_INGAME_GUI.md).
- 00670020 (2147), 0066EA60 (1280, the register; `src/hud_screens.cpp` holds only its page and
  widget catalogue), 00673A10 (1276) and 00674B80 (1212, Enter): too large for this packet. Each
  needs a full listing pass.
- 0065BF00 (807, the map icon pass), 0065D220 (657, RebindSlots), 00660B10 / 00661240 (scroll
  forward/back, about 550 each), 0065FBF0 (431, slot highlights), 00668770 (431), 0066A8A0 (523)
  and 00671D60 (473) were not reached. Each repeats native-string construction or inlined STL
  copies, and the decompiler loses the stack layout, so each needs a full listing read.
- 006697E0 (372): the catalog collection into +370h. The decompile mixes up the copied
  {unit, reference holder} value and the "InferiorFailure" virtual (+198h) argument frames, and
  the listing was not walked.
