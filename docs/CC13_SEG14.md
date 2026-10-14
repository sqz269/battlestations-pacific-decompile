# Segment 14 (cc13-seg14): binoculars, debug singletons, cheat menu, HUD screen 2Fh, pick-screen entries

Packet cc13-seg14, segment 14 (0051E7E0..00527C80). Code: `src/view_seg14_cc13.cpp`,
`include/bsp/view_seg14_cc13.hpp` (namespace `bsp::cc13_seg14`). Every row is reconstructed and
build-tested only: not ABI-compatible, not fixture-tested, not game-validated. Names are
hypotheses, not recovered symbols.

## Conventions

- Objects are the image's own storage addressed by byte offset; vtables are the image constants.
- Callees not reconstructed here are `ViewSeg14Hooks` methods, invoked at the same point with the
  same arguments; virtual calls are named by slot. Several hooked callees have host models
  elsewhere (0051EF00 / 0051F050 in `hud_warning_screen.cpp`, 00452B80 in
  `engine_seg4_6_9_cc13.cpp`, the GUI manager and observer bodies).
- Called directly: the NativeString resize 0041DD40 and `NativeStringStorage::release` (the
  inlined temporary page-name strings), and the shared lazy-singleton getter body
  `cc13_sys::lazy_singleton_get` for 0051F520 / 0051F5E0.
- Global words are passed by reference in `ViewGlobals`.

## Functions
| Address | Ghidra name | C++ | Original ABI | Behaviour (evidence) | Simplifications |
|---|---|---|---|---|---|
| 0051E7E0 | BSP_HudBinoculars_UpdateZoom | `binoculars_toggle_zoom_0051e7e0` | __thiscall(screen) RET (tail JMP 0051E6E0) | mode +30h == 1 and zoom +38h equal (ordered) to 0.25 (00CE3868) or 1.0: +38h = +28h = 1.0, +30h = 0, byte +35h = 1, bytes +2Ch..+2Eh = 0, and unless [00E18DA8] == 0.0 (00D7A218) 00B6FBF0(mover +40h -> camera +1BCh, [00E18DA8]); mode 0 with zoom == 1.0: +30h = 1, byte +34h = 1, +38h = 1.0, [00E18DA8] = camera+1D4h and 00B6FBF0(camera, 1.5 00CE380C); then [00E188D8] vslot 5Ch(6) true -> 00817380(unit, mode == 1); then 0051E6E0 | UCOMISS/LAHF/TEST AH,44h equality tests written as C++ ==, which is false for NaN exactly like the JNP path |
| 0051E8E0 | BSP_HudBinoculars_Reset | `binoculars_reset_0051e8e0` | __thiscall(screen) RET | +30h = 0; +38h = +28h = 1.0; 0051E6E0; byte +5 set: [+20h] vslot 34h(0) and 00452B80(ECX = 00E081A0, 0) | none |
| 0051E950 | BSP_HudBinocularsScreen_Construct | `binoculars_screen_construct_0051e950` | __thiscall(screen) RET, EAX screen | 004F7180; +8h = 00CE3CD4; +0Ch..+14h = 0; bytes +18h, +4Ch = 0; vftables 00CEC9B0 (+0) and 00CEC998 (+8h); floats +44h, +48h = 0 | none |
| 0051E9B0 | BSP_HudBinocularsScreen_Exit | `binoculars_screen_exit_0051e9b0` | __thiscall(screen) RET | byte-identical body to 0051E8E0 | expressed as a call to the 0051E8E0 reconstruction |
| 0051EA20 | BSP_HudBinocularsScreen_Destruct | `binoculars_screen_destruct_0051ea20` | __thiscall(screen) RET | vftables 00CEC9B0 / 00CEC998; 00AA31F0(004C12B0(), page +1Ch); 00695870(screen+8h); 004F71A0 | none |
| 0051ED60 | BSP_HudBinocularsScreen_Register | `binoculars_screen_register_0051ed60` | __thiscall(screen) RET | 004F71D0; bytes +2Ch..+2Eh = 0; +28h = 1.0; page +1Ch = 00AA5840(004C12B0(), "GUI_binoculars" 00CEC9E8, 1, 0) through a temporary NativeString; +38h = 1.0; +30h = 0; +40h = 0; bytes +34h, +35h, +24h = 0; +3Ch = 4.0 (00CE3D34) | none |
| 0051EE50 | BSP_HudBinocularsScreen_Enter | `binoculars_screen_enter_0051ee50` | __thiscall(screen) RET | +20h = 00AA7E00(page +1Ch, "Tavcso_Model" 00CEC9F8, 1) through a temporary NativeString; 0051E6E0 | none |
| 0051F040 | BSP_HudBinocularsScreen_CollectChildren | `binoculars_screen_collect_children_0051f040` | __thiscall(screen, vector*) RET 4 | 004D6790(vector, &screen+1Ch) | none |
| 0051F330 | BSP_HudBinoculars_UpdateView | `binoculars_update_view_0051f330` | __thiscall(screen, float dt) RET 4 | 0051EF00(dt); 0051F050(ECX = mover +40h, EDX = screen+24h, dt, zoom +38h) | both callees are hooks (host models of them exist in hud_warning_screen.cpp) |
| 0051F520 | BSP_DebugRenderFlags_GetSingleton | `debug_render_flags_get_0051f520` | __cdecl RET, EAX | lazy singleton: global 010901CC, operator new 00BF681B(8), constructor 00B854D0, 00BD0C30 registers the global itself; lock through 00415350 | uses lazy_singleton_get (engine_seg4_6_9_cc13.cpp) with its LazySingletonHost |
| 0051F5E0 | BSP_Singleton00E19AAC_GetOrCreate | `unit_debug_inspector_get_0051f5e0` | __cdecl RET, EAX | lazy singleton: global 00E19AAC, 24h bytes, constructor 006FB920, registered sub-object +14h (null when the allocation failed) | uses lazy_singleton_get |
| 0051F7E0 | BSP_CheatMenuScreen_RestoreInputAndCinematic | `cheat_menu_restore_input_and_cinematic_0051f7e0` | __thiscall(screen) RET | byte +1Ah set: 00A933F0(004BEC00(1Eh, 0)); 004CD0F0(game, byte +18h, byte +19h, 1); 00543E00(screen+20h); page +1Ch vslot 34h(0); +1Ah = 0 | none |
| 0051F830 | BSP_CheatMenuScreen_Register | `cheat_menu_screen_register_0051f830` | __thiscall(screen) RET | 004F71D0; 0051F520()+4 byte = 0; page +1Ch = 00AA5840(004C12B0(), "_CheatMenu" 00CECA18, 1, 0); byte +4 = 1 | none |
| 0051F910 | BSP_CheatMenuScreen_DestroyMembers | `cheat_menu_member_destruct_0051f910` | __thiscall(member) RET | vftable 00CECA24; 00543E00(member); 00427110(member+0Ch, 0); _free 00BF6989 of [member+0Ch] (no null test) | none |
| 0051F990 | BSP_CheatMenuScreen_Construct | `cheat_menu_screen_construct_0051f990` | __thiscall(screen) RET, EAX screen | 004F7180; vftable 00CECA28; +20h = 00CECA24; +2Ch, +30h, +34h, +40h, +10h, +14h = 0; bytes +1Ah, +0Eh, +0Dh, +0Ch = 0; +8h = 1 | none |
| 0051F9E0 | BSP_CheatMenuScreen_IsSelfManaged | `cheat_menu_screen_is_self_managed_0051f9e0` | RET, AL = 1 | MOV AL,1 | none |
| 0051F9F0 | BSP_CheatMenuScreen_Destruct | `cheat_menu_screen_destruct_0051f9f0` | __thiscall(screen) RET | vftable 00CECA28; 0051F910(screen+20h); 004F71A0 | none |
| 005211B0 | BSP_HudScreenSlot2F_Register | `hud_screen_2f_register_005211b0` | __thiscall(screen) RET | 004F71D0; +20h = 0 | none |
| 00521240 | BSP_HudScreenSlot2F_Update | `hud_screen_2f_update_00521240` | __thiscall(screen, float dt) RET 4 | 0051F330(binoculars [[00E198C4]+4Ch], dt); +20h non-null: 005454B0(driver [[00E198C4]+50h], 0.5 (00CE3800), 0.5, binoculars zoom +38h); 005484F0(driver) | 0051F330 is called as the reconstruction here |
| 005212A0 | BSP_HudScreenSlot2F_Enter | `hud_screen_2f_enter_005212a0` | __thiscall(screen) RET | 00549260(driver, +1Ch, +20h); +1Ch non-null and 00694AF0(+1Ch, screen+8h) false -> 00694A60(+1Ch, screen+8h) (tail JMP) | none |
| 00521310 | BSP_HudScreenSlot2F_OnObservedDestroyed | `hud_screen_2f_on_observed_destroyed_00521310` | __thiscall(screen, object) RET 4 | object == +14h -> +14h = 0; else object == +18h -> +18h = 0 | none |
| 00521670 | BSP_HudUnitPickScreen_Register | `unit_pick_screen_register_00521670` | __thiscall(screen) RET | 004F71D0; +B8h = 0 | none |
| 005216A0 | BSP_HudUnitPickScreen_OnObservedReleased | `unit_pick_screen_on_observed_released_005216a0` | __thiscall(observer = screen+18h, handle) RET 4 | target = handle vslot 4(); non-null and == +34h: 006952A0(target, observer unless it is 18h); +34h = 0; +98h = 0 | none |
| 00524920 | BSP_HudUnitPickScreen_EnterCopy_00524920 | `unit_pick_screen_exit_00524920` | __thiscall(screen) RET | mode +30h in 4..6: 00A933F0(004BEC00(4, 1)), 00A933F0(004BEC00(0Eh, 1)); +30h = 0 | none |
| 00524960 | BSP_HudUnitPickScreen_Enter | `unit_pick_screen_enter_00524960` | __thiscall(screen) RET | the same, then +ECh = 0.25 (00CE3868) | none |
| 00521E70 | BSP_Unit_RoleIsAiHeld | `unit_role_is_ai_held_00521e70` | __thiscall(unit, slot) RET 4, EAX 0/1 | role = unit+1ACh[slot]; 8 -> 1; else 00927F10(ECX = unit, role) ? 1 : 0 | none |
| 00522EE0 | BSP_ObserverHandle_SetTarget_00522ee0 | `observer_handle_set_target_00522ee0` | __thiscall(handle, target) RET 4 | different from +14h: old non-null -> 006952A0(old, handle); +14h = target; non-null -> 00694A60(target, handle) | none |
| 00523320 | BSP_ObserverHandle_Construct_00523320 | `observer_handle_construct_00523320` | __thiscall(handle, target) RET 4, EAX handle | +4h..+0Ch = 0; vftable 00CE74FC; +14h = target; byte +10h = 1; non-null target -> 00694A60(target, handle) | none |
| 00523480 | BSP_ObserverHandle_Construct_00523480 | `observer_handle_copy_construct_00523480` | __thiscall(handle, source) RET 4, EAX handle | +4h..+0Ch = 0; byte +10h = 0; vftable 00CE74FC; +14h = source+14h; byte +10h = 1; non-null -> 00694A60 | none |
| 00523B30 | BSP_ObserverHandle_Assign_00523b30 | `observer_handle_assign_00523b30` | __thiscall(handle, source) RET 4, EAX handle | the 00522EE0 retarget to source+14h | expressed through the 00522EE0 reconstruction |
| 00524090 | BSP_ObserverHandleDerived_PlacementConstruct_00524090 | `observer_handle_derived_placement_copy_00524090` | __fastcall(ECX dest, EDX source) RET | non-null dest: 00523480(dest, source), vftable 00CE75CC | none |
| 00524710 | BSP_ObserverHandleDerived_PlacementConstruct_00524710 | `observer_handle_derived_placement_copy_00524710` | __stdcall(dest, source) RET 8 | the same body on stack arguments | none |
| 00524560 | TMPL_ObserverHandle_DestroyRange_00524560 | `observer_handle_destroy_range_00524560` | __stdcall(first, last) RET 8 | vslot 0(0) on each 18h element | none |
| 00524590 | TMPL_ObserverHandleVector_Tidy_00524590 | `observer_handle_vector_tidy_00524590` | __thiscall(vector) RET | first +4h non-null: destroy [first, last +8h), _free 00BF65AC of first; +4h, +8h, +0Ch = 0 | none |
| 00522B70 | TMPL_DList_EraseNode_00522b70 | `dlist_erase_node_00522b70` | __thiscall(list, node) RET 4, EAX next | unlink {prev +0, next +4h} from {count, head +4h, tail +8h}, count--, _free 00BF65AC, return next | none |
| 005234F0 | TMPL_NodeList_Clear_005234f0 | `node_list_clear_005234f0` | __thiscall(list) RET | while count: unlink head, count--, _free | none |
| 00522E10 | TMPL_Pod8_UninitCopy_00522e10 | `pod8_uninitialized_copy_00522e10` | __fastcall(ECX first, EDX last, dest, 3 unused) RET 10h, EAX dest end | 8-byte copies | placement-new null test dropped |
| 005235A0 | TMPL_Pod8_Fill_005235a0 | `pod8_fill_005235a0` | __cdecl(first, last, value*) | 8-byte fill | none |
| 00523640 | TMPL_Pod8_UninitFillN_00523640 | `pod8_uninitialized_fill_n_00523640` | __fastcall(ECX dest, EDX count, value*, 3 unused) RET 10h | count (unsigned) 8-byte copies | placement-new null test dropped |
| 00523AF0 | TMPL_Pod8_CopyBackward_00523af0 | `pod8_copy_backward_00523af0` | __cdecl(first, last, dest_end), EAX dest_end - n*8 | backward 8-byte copy; the result computed first | none |
| 00523540 | BSP_Entity_CastIfKind01 | `entity_cast_if_kind_00523540` | __fastcall(ECX entity) RET, EAX | non-null and vslot 5Ch(1) -> entity, else 0 | none |
| 00523560 | BSP_Entity_CastIfKind18 | `entity_cast_if_kind_00523560` | __fastcall(ECX entity) RET, EAX | kind 18h | none |
| 00523580 | BSP_Entity_CastIfKind02 | `entity_cast_if_kind_00523580` | __fastcall(ECX entity) RET, EAX | kind 2 | none |
| 00527BE0 | BSP_ControlLayoutWidgetListener_Current28 | `control_layout_select_current_00527be0` | __thiscall(listener, unused, widget) RET 8 | [00E08670] = 00A9C920(ECX = widget); widgets +14h..+1Ch vslot 34h(i == [00E08670]) | none |
| 00527C30 | BSP_ControlLayoutWidgetListener_Current18 | `control_layout_dim_widget_00527c30` | __thiscall(ignored, widget, byte) RET 8 | widget vslot 5Ch() == 3: vslot 4Ch(byte ? 0.5 : 1.0) | none |

## Existing implementation reviewed

- 00522050 `pick_screen_distance_00522050` (`src/hud_warning_screen.cpp`): ledger record added
  with status `existing_reconstruction_reviewed_minor_rounding_defect`. The image stores dy*dy
  as a float first, then dx*dx as a float, and returns their float sum; the C++ keeps dy*dy in
  double before the add, and its comment says the x square is stored first. Not edited here.
  Fixed later by cc13-seg1: both squares are now rounded to float, dy*dy first, and the record
  status is `semantic_build_tested`.

## Not reconstructed here

- Belongs to cc9 / existing host models of screen 29h (unit pick): 00526A40 and 00527260
  (implemented in `src/hud_warning_screen.cpp` and `GameHudHost::update_unit_pick_screen_00527260`,
  not re-verified here), 005241D0 (issue command to target), 005213D0 (command-building unit
  camera), 0051EF00 / 0051F050 (host models exist), 0051EAA0 (input axes, 200 lines).
- Skipped, large: 0051FA50 BSP_CheatMenuScreen_BuildLines (1679 lines), 00523B70, 005253F0,
  00525720, 00524E90 and the PtrFloat sort/heap templates (00521D40, 00522CF0, 00522D60,
  00523740, 005237C0, 00523810, 005247D0), the ObserverHandle range copies (00524000, 005240E0,
  00524690, 00524770, 00524820, 00524A90), 00521370 (x87 pitch/yaw).
- No export body (FUN_ without a Ghidra function body in the export): 00521F10, 00521FD0,
  005220C0, 005224C0, 00522680, 005226D0, 00522860, 00522F10, 00523020, 00523130, 00523940,
  005251C0, 00525250, 00525F80, 00526180, 00526720.
