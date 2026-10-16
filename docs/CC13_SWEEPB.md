# cc13-sweepB: leftover sweep over segments 15-51

Packet `cc13-sweepB`, Claude worker in the cc13 lineage, 2026-10-16. The pool is the functions the
earlier cc13 waves left without a reconstruction record in segments 15-29, 31, 32, 35, 37, 38 and
42-46 and 51 (`local/cc13_poolnames.py`, about 380 functions). Each was closed with one outcome:
reconstructed, existing implementation recorded, unreferenced recorded, out of scope (library or
compiler thunk), cc9 lane, skipped with a reason, or not reached.

Names are hypotheses, not recovered symbols. Files: `include/bsp/cc13_sweepB.hpp`,
`src/cc13_sweepB.cpp` (registered in `cmake/startup.cmake` on bsp_core), this document. No host
file was edited, nothing is wired and no game was run. Ghidra was read only.

Method, per function: `bsp.py lookup` / `docs-for` and a `git grep` of src/ and include/ for the
address (most hits were host hook declarations, not bodies); earlier skip reasons were respected
(cc9 lane, vendor library); bodies were read from the disk bytes (`bsp.py disasm-raw`) or the live
listing, with x87 reverse-form operands (FSUBRP/FDIVRP) checked against the raw bytes; constants
were read from the image with their width. EH frames are noted but unwinding is not modelled.

x87 note: float expressions are written in the image's order with a float store wherever the image
stores to a float slot. That is exact at the main thread's 24-bit precision control; at 53 or 64
bits the image keeps the unstored intermediates wider (noted per function where a double constant
is involved).

## Reconstructed in src/cc13_sweepB.cpp

Build-tested only (compiled in the bsp_core build); none is ABI-compatible, fixture-tested or
game-validated, and nothing is wired. Every call the image makes is a named `Cc13SweepBHost`
method at the same point with the same arguments.

| Seg | Address | Name (hypothesis) | Original ABI | What the code does, and simplifications |
|---|---|---|---|---|
| 15 | 0052C640 | BSP_HudCounterEntry_ConstructClone | __thiscall(ECX entry)(source) RET 4, EAX entry | listing 0052C640..0052C83E. Zeroes +70h..+80h,+88h..+94h; +A4h=1,+9Ch=3,+A5h/+A6h/+A8h=0; root=00AAB4C0([src],0); children +4,+8,+C,+2C,+10..+28 = 00AA7E00(root, srcwidget->vt+28h(), 1) (the PUSH 1 is 00AA7E00's 2nd arg); float copies +3C..+44,+30..+38,+60..+6C,+48..+5C; root vt+34h(0). EH state 3 / unwind not modelled. C++ cc13_sweepB_counter_entry_construct_clone_0052c640. |
| 15 | 00535430 | BSP_InterfaceView_DrawTargetOrtho | __thiscall(ECX view=interface manager+5Ch) RET; tail JMP 0093D250 ECX=view+8 | listing 00535430..00535618. Gate +4h; leader world +CCh refreshed (00414DB0) before each read group; +17Ch=w[+30h], +180h=w[+38h]; [+178h] vt+30h(+17Ch,+18Ch,w38); [+174h] vt+30h(+17Ch,+188h,+180h); look-at 00B63F10 (ECX dest, EDX eye (w30,10000f 00CE3D64,w38), target (w30,0,w38), up w+20h..28h) -> 00B71490 on [+10h]; ortho 00AA2020 (-w/2,w/2,-h/2,h/2,1,15000f 00CED55C), w=+30h h=+34h. Host game_render_frame.cpp only has the hook. C++ cc13_sweepB_interface_view_draw_target_ortho_00535430. |
| 15 | 00535F50 | BSP_HudFormationScreen_DragPickNearestMember | __thiscall(ECX screen) RET, AL result | listing 00535F50..005360F4. Leader [[+138h]+14h] refreshed via 00414DB0 when +C8h clear; count [+138h]+4F8h; cursor copied from 004C12B0()+5Ch/+60h; per member record 0070D070 (unit non-null, not leader): sx=-rec[+10h]/[+30h]+0.5, sy=rec[+20h]/[+34h]+0.5 (double 00D7A280), d=dx*dx+dy*dy float, dist=sqrt(d) (00BF7030) when d>1e-10 (00CE3820) else 0; when best (seed 0.03f 00CE3DB8) > dist: +8Ch=0070D030 index, listbox +ACh 00A9C7C0, +7Ch=1, cursor 00AA1FA0(sx,sy). Caller 0053A8E0. x87 intermediates written as float ops (exact at 24-bit PC). C++ cc13_sweepB_formation_drag_pick_00535f50. |
| 15 | 005411A0 | BSP_WidgetHolder18_Show_005411A0 | __thiscall(ECX holder) RET | 005411A0..005411AC: widget [holder+18h] vtable +34h(1). Byte census: no rel32 or absolute reference; no vtable slot. C++ cc13_sweepB_holder18_show_005411a0. |
| 15 | 005411B0 | BSP_WidgetHolder08_Show_005411B0 | __thiscall(ECX holder) RET | 005411B0..005411BC: widget [holder+8h] vtable +34h(1). Byte census: no reference. C++ cc13_sweepB_holder08_show_005411b0. |
| 15 | 005411C0 | BSP_WidgetHolder18_SetPixelXY_005411C0 | __thiscall(ECX holder)(int x, int y) RET 8 | 005411C0..005411F6: 00AA7D00(widget [holder+18h], (float)(x * 2^-10 [double 00CEDAE8]), (float)(y / 768.0 [double 00CE42B0])). Divide rounds once at double here (x87 precision in the image). Byte census: no reference. C++ cc13_sweepB_holder18_set_pixel_xy_005411c0. |
| 15 | 00541200 | BSP_Holder_SetField04_00541200 | __thiscall(ECX holder)(value) RET 4 | 00541200..00541207 stores the argument at +4h. Byte census: no reference. C++ cc13_sweepB_holder_set_field04_00541200. |
| 15 | 00541210 | BSP_WidgetHolder08_SetPixelXY_00541210 | __thiscall(ECX holder)(int x, int y) RET 8 | 00541210..00541246: as 005411C0 with the widget at +8h. Byte census: no reference. C++ cc13_sweepB_holder08_set_pixel_xy_00541210. |
| 16 | 005457C0 | BSP_Entity_IsHostileToParty | __thiscall(ECX entity)(party) RET 4, EAX 0/1 | 005457C0..005457D8: 1 when [entity+54h] != party and party != 2. Callers (rel32) 00547480, 00835F60, 00836920. C++ cc13_sweepB_entity_is_hostile_to_party_005457c0. |
| 16 | 00548300 | BSP_HudWeaponGroupScreen_UpdateGroupSight | __thiscall(ECX screen) RET | 00548300..00548339, jump table 0054833C: return when +40h null or +44h > 5; 0 -> JMP 005464E0; 1 -> RET; 2..5 -> 00547A20(5/2/7/9). Caller 005484F0. C++ cc13_sweepB_weapon_group_update_sight_00548300. |
| 16 | 00549430 | BSP_HudWeaponGroupScreen_RebindFirstUnit | __thiscall(ECX screen) RET | 00549430..005494BB: entry = [+20h] when +24h non-null, else a stack default entry (vtable 00CEDDA0, +4/+8/+C/+14 zero, +10h byte 1); 00549260(screen, entry+14h, +40h); temp destroyed via 00545600. EH 00C6CF01 not modelled. Callers 006F3660, 006F5460, 006F7360. C++ cc13_sweepB_weapon_group_rebind_first_unit_00549430. |
| 16 | 00554600 | BSP_GuiRect_DrawOutline | __fastcall(ECX a, EDX b)(rgba*) RET 4 | 00554600..00554673: a'/b' = a/b - 0.5 (double 00D7A280, float stores); renderer [00F8D394]; 004FB850 into the argument slot then a local; renderer vt+C0h(&a', &b', local, slot). Callers 00554680, 00581DF0, 005D63E0. C++ cc13_sweepB_gui_rect_draw_outline_00554600. |
| 18 | 00568CE0 | BSP_SessionSearchResult_CopyConstruct | __thiscall(ECX object)(source) RET 4, EAX object | 00568CE0..00568D3B: vtable 00CEED04; +4h..+18h dword moves, REP MOVSD 9/10/8 dwords at +1Ch/+40h/+68h (all of +4h..+87h). Caller 0057A9C0. C++ cc13_sweepB_session_search_result_copy_00568ce0. |
| 18 | 00568EB0 | BSP_Singleton00F8894C_ClearBase | __thiscall(ECX object) RET | 00568EB0..00568EC0: global 00F8894C = 0, vtable 00CE3818 into +0h. Callers: unwind funclets 00C6F360, 00CA2EB0. C++ cc13_sweepB_singleton_f8894c_clear_base_00568eb0. |
| 18 | 005692E0 | CG_scalar_deleting_dtor_005692e0 | __thiscall(ECX object)(flags) RET 4, EAX object | 005692E0..00569306: 00F8894C = 0, vtable 00CE3818, operator delete 00BF65AC when flags&1. Vtable slot 00CEED6C. C++ cc13_sweepB_singleton_f8894c_scalar_delete_005692e0. |
| 19 | 0056E360 | BSP_MultiSessionBrowserScreen_BeginFilterEdit | __thiscall(ECX screen) RET | 0056E360..0056E3D8: +244h..+24Ch -> +250h..+258h, bytes +10h..+18h -> +19h..+21h, 0056D660(screen), listbox [+34h] 00A9C7C0(0). menu_screens_2.cpp has only the hook. C++ cc13_sweepB_session_browser_begin_filter_edit_0056e360. |
| 19 | 00571EA0 | BSP_SessionBrowserRow_Create | __stdcall(dest, source) RET 8 | 00571EA0..00571FCA: copy-constructs a 0A0h session record: operator new(88h) 00BF681B, vtable 00CEED04, inline copy of only +4h..+3Fh from source[+0] (dest+0 = 0 and block leaked when source[+0] null, as in the image); +4h byte; +8h wstring via 0056F160(src+8, 0, npos) without prior _Tidy; dwords +24h..+44h, bytes +48h/+49h, 14 dwords +4Ch.., +88h,+8Ch,+98h,+9Ch,+94h,+90h. Callers 00576D20, 005789C0. C++ cc13_sweepB_session_record_copy_construct_00571ea0. |
| 19 | 0057A910 | BSP_MultiSessionBrowserScreen_ApplyFilterEdit | __thiscall(ECX screen) RET | 0057A910..0057A9BA: +250h..+258h -> +244h..+24Ch, +19h..+21h -> +10h..+18h, 005789C0(screen,[+2B0h]), 0056DD80(screen), 007FBA30(ECX=[00E188A8]+650h, +244h, +248h, +24Ch, screen+10h, 9). C++ cc13_sweepB_session_browser_apply_filter_edit_0057a910. |
| 20 | 005845D0 | BSP_MainMenuScreen_AdvanceBackgroundMovie | __thiscall(ECX screen) RET | 005845D0..00584743: +58h=(+58h+1)%3; 00AAC870(+340h); ".bik" (00CEC130) via 0041DD40(4,1)+memcpy len+1; 004260B0(int +44h+i*4); 0043C130("movies\FE_bg_0" 00CEF7C0, number); 004261A0(name, &suffix); 00AAE320(+340h, &path, 0); temporaries released path, name, number, suffix; 00AB0EB0(+340h). EH 00C6F930 not modelled. C++ cc13_sweepB_main_menu_advance_background_movie_005845d0. |
| 20 | 0059DA90 | BSP_HudMapLayer_Destruct | __thiscall(ECX layer) RET | 0059DA90..0059DADF: vtable 00CEFFE4; slots +4h then +8h..+24h: when non-null vtable +4h(1) and the slot cleared. Caller 0059FA20 (scalar deleting dtor). C++ cc13_sweepB_hud_map_layer_destruct_0059da90. |
| 20 | 0059E120 | BSP_Game_ClampPointToWorldBoundsXZ | stack (point*, unused, unused) RET 0Ch; ECX overwritten | 0059E120..0059E2B2 (FCOMIP pops): G=[00E188A8]; min/max of G+711Ch/G+7128h and G+7124h/G+7130h (tie/NaN takes the second operand), lo=min-500, hi=max+500 (double 00CE3840) as float; point[0], point[1] = lo when lo>p, else hi when p>hi, else p. The extent arguments the caller passes are never read. Caller 005B45C0. C++ cc13_sweepB_clamp_point_to_world_bounds_0059e120. |
| 20 | 0059E690 | BSP_HudMapIconArray_Reserve | __thiscall(ECX slot)(int n) RET 4 | 0059E690..0059E70F: engine counted array {data,size,capacity}; n > cap: 00BF55BE(n*4+18h), data=(block+18h)&~0Fh, memcpy capacity*4 (00BF7680) when header && capacity, header {data, size, n}, old header to 00BF6989 (even null), slot = block; else header->capacity = n (no null test, as the image). C++ cc13_sweepB_counted_array_reserve_0059e690. |
| 20 | 0059F460 | BSP_Entity_CastIfKind02_0059f460 | __thiscall(ECX entity) RET, EAX | 0059F460..0059F47B: null -> null; entity when vtable +5Ch(2) else null. Byte census: no rel32/absolute reference (unreferenced). C++ cc13_sweepB_entity_cast_if_kind02_0059f460. |
| 20 | 0059FAD0 | TMPL_PointerArray_PushBack_0059fad0 | __thiscall(ECX slot)(void* const* value) RET 4 | 0059FAD0..0059FB57: size==capacity -> 0059E690(max(cap+cap/4 signed, 20h)); data[size]=*value; size++. Engine counted array (not STL). Caller 005A4A00. C++ cc13_sweepB_counted_array_push_back_0059fad0. |
| 20 | 005A1B40 | BSP_HudMap_CreateAlphaBlendModel | ECX factory, EDX parent, stack (out_node, out_sub, out_model, out_material, alpha_source) RET 14h | 005A1B40..005A1C5D: node=factory vt+8h(6,1.0f); sub=node+0Ch; 00B6D890(sub,parent); model = sub vt+0Ch(00B74310()) ? sub : sub+34h; material=005A07B0(model, "alphablend" 00CF0020 or "mapmodel" 00CF0014 per alpha_source); alpha_source: 00B18B20(material, "cAlpha" 00CF000C, alpha_source); strings via 0041E870 released to the pool. Image passes uninitialised stack strings to 0041E870 (zeroed here). Callers 005ADDA0, 005B25D0. C++ cc13_sweepB_hud_map_create_alpha_blend_model_005a1b40. |
| 20 | 005A2650 | BSP_HudMapScreen_SetSpeakingUnit | __thiscall(ECX screen)(unit, fallback, bool flag) RET 0Ch | 005A2650..005A2724: u = unit ? unit : fallback; mute byte [00E188A8]+19C4h clear -> +53Ch=u, +540h=clock 00F876A4; set -> icon=005A22D0(u); none: 005A22D0(selected 00E188D8) when non-null; icon with all seven slots +48h..+60h null: each = 00AAB4C0([+A0h],0), icon+74h=(flag?1.0f:0)+clock. voice_playback.cpp calls the hook only. C++ cc13_sweepB_hud_map_set_speaking_unit_005a2650. |
| 20 | 005A3EF0 | BSP_Node_FindChildByName_005a3ef0 | __stdcall(node, NativeString name {len,data} by value, float value) RET 10h | 005A3EF0..005A401A: for each child (+34h, next +3Ch): name after first ':' (00B6D800 header, strstr 00BF9440, 00469840 substring from idx+1); equal length and (empty or _stricmp 00BF7FBF == 0) -> 00B6DA70(child, value, 0); substring and the by-value name returned to the sized pool (00419CC0/00BD1510 len+1, 1). It sets a factor on every match, not a find. EH 00C71690 not modelled. Caller 005A4020. C++ cc13_sweepB_node_set_child_factor_by_name_005a3ef0. |
| 20 | 005A48C0 | BSP_HudMapScreen_ResolveIconUnit | __thiscall(ECX screen)(unit_slot*, prev_slot*, bool flag) RET 0Ch | 005A48C0..005A49F5: unit kind 18h and +2Ch>=2 -> unit+3D0h; icon=005A22D0 (or 005A1220 with flag); cur==prev with no icon or icon+118h set -> end; prev!=cur: p=005A22D0(prev), p+118h set -> p+20h vt+4h(1) cleared, 005A21E0(p); cur and icon+118h==0: icon+20h=00AAB4C0(flag?+E8h:+E4h) when null and widget set, +28h=flag; +118h=1, +11Ch=flag, +110h=clock+0.0; *prev=*unit. Caller 005B45C0. C++ cc13_sweepB_hud_map_resolve_icon_unit_005a48c0. |
| 20 | 005A8420 | BSP_HudMapIcon_RefreshUnitBadge | __thiscall(ECX icon)(template, unread) RET 8, EAX icon+38h | 005A8420..005A853B: +38h=0; unit +4h kind 1Ch (vt+5Ch, called without a null test), template, unit and unit+760h required; +38h++; badge vector +98h size >= +38h -> return; clone=00AAB4C0(template,0); clone vt+34h(0); row=unit+760h+party*38h (party=[[G+[G+18ECh]*4+18CCh]+28h]); 00AB64C0(clone,[row+30h],row+34h); vt+88h(1,0,1.0f); 0043DCA0(+98h,&clone). Caller 005ADDA0. C++ cc13_sweepB_hud_map_icon_refresh_unit_badge_005a8420. |
| 23 | 00604A20 | BSP_InGameInterface_AllowsInput | __thiscall(ECX screen) RET, EAX 0/1 | 00604A20..00604A4B: 0 when +900h is 7, 6 or 3, or 4 with byte +904h clear; else 1. Caller 007BB9A0 (pilot_command_path.hpp global_block_00604a20 models the answer). C++ cc13_sweepB_interface_allows_input_00604a20. |
| 23 | 00604A50 | BSP_Entity_IsKind17AndReady | __thiscall(ECX entity) RET, EAX 0/1 | 00604A50..00604A73: vtable +5Ch(17h) non-zero and byte +C24h clear. Callers 0060ABD0, 007EE8F0. C++ cc13_sweepB_entity_kind17_ready_00604a50. |
| 24 | 006057E0 | BSP_ResponseCurve_Evaluate | __thiscall(ECX curve)(float x) RET 4, ST0 | 006057E0..00605899 (0060588A DE E1 FSUBRP = 1-c2): sign by COMISS (NaN 0); a = x>0 ? x : -0.0f-x; t=00419010(c0,0,c1,1,a); s=(float)(t*sign); q=(float)(t*s); result (float)(q*c2+s*(1-c2)). Caller 006062B0. C++ cc13_sweepB_response_curve_evaluate_006057e0. |
| 24 | 006062B0 | BSP_AxisFilter_Step | __thiscall(ECX filter)(float rate, float input, float dt) RET 0Ch, ST0 | 006062B0..00606463 (0060636F DE E1 = p-C): C=double 00D7A3A0 (0.1f widened); /rate/>C sets +0; /input-+0Ch/>C clears +0; +0Ch=input; +8h += T[688h]*rate (0042E740); p=dt+4h; p>C: +4h=p-C, when T[694h]>/+8h/ +8h*=T[698h]; clamp +8h to [-1,1]; +0 clear -> +10h=+0Ch else +10h=006057E0({T[68Ch],T[690h],T[69Ch]}, +8h). Same 0042E740 call count as the image. p-C rounds at double then float here. Caller 0060ABD0. C++ cc13_sweepB_axis_filter_step_006062b0. |
| 24 | 0060C5B0 | BSP_Object_Field198IsZero | __thiscall(ECX object) RET, EAX 0/1 | 0060C5B0..0060C5CD: UCOMISS [+198h] with 0.0f (00D7A218), LAHF/TEST AH,44h/JP: 1 only on ordered equality. Caller 007C0910. C++ cc13_sweepB_field198_is_zero_0060c5b0. |
| 24 | 0060CB40 | BSP_Unit_AsKind9OrNull | __thiscall(ECX unit) RET, EAX | 0060CB40..0060CB5B: null -> null; unit when vtable +5Ch(9) else null. Byte census: unreferenced. C++ cc13_sweepB_unit_as_kind9_0060cb40. |
| 25 | 00612DA0 | BSP_HudPowerupManagerScreen_RefreshPromptWidgets | __thiscall(ECX screen) RET | 00612DA0..00612EF6: return when +4h or [+B0h]+121h; [[00E198C4]+BCh]+5h set -> [+B0h] vt+34h(0); else [+B0h] vt+34h(008ED9C0([00F88C30])) (whole EAX pushed in the image, 0/1 here), [+D4h]/[+E8h]/[+ECh] vt+34h(!has) each with a fresh 008ED9C0, 00AA8240([+E4h], 00AA6750([+E8h])); p=(+F4h - (0066FFA0([[00E198C4]+BCh]) ? 0.1 : 0.3), +F8h, +FCh) (doubles 00D7A3A0/00CE3DC8) -> 00AA8240([+B0h], &p). Callers 00674B80, 0068C1F0. C++ cc13_sweepB_powerup_refresh_prompt_widgets_00612da0. |
| 25 | 0061A400 | BSP_HudPowerupManagerScreen_AssignLists | __thiscall(ECX carousel)(vector a, vector b by value) RET 20h | 0061A400..0061A472: 004F60E0(+18h,&a), 004F60E0(+28h,&b), 00618980(carousel), operator delete 00BF65AC of a.first then b.first when non-null. EH 00C77E50 not modelled. Caller 0061B0F0. C++ cc13_sweepB_powerup_assign_lists_0061a400. |
| 26 | 006441B0 | BSP_Unit_OrdnancePendingForHud | __thiscall(ECX unit) RET, AL | 006441B0..00644218: not kind 18h/0Fh (vtable +5Ch) -> AL 0 (EAX upper bytes from the last call); kind 18h re-asked, true -> u=[unit+3D0h]; 007BCA50(u) in {9,0Fh,0Ah,0Bh} -> !007B9140(u,0); else 0. Byte census: unreferenced. C++ cc13_sweepB_unit_ordnance_pending_006441b0. |
| 26 | 006515B0 | BSP_HudUnitView_UpdateLensEffect | __thiscall(ECX view)(float dt) RET 4 | 006515B0..00651754: +2Ch non-null: +74h==1 -> 00452BD0(00E081A0, dt, +5Ch); else node=[+2Ch]+1BCh (00B6DB70 unless +5Ch bit 1), 0 > node+124h -> statics 00E19810..00E19820 set once (guard 00E19830) from the settings singleton fields +5A8h..+598h (five getter calls); then the lens-effect setter 00B0D020([00F8D39C], 1, E19820, E1981C, E19818, E19810, E19814), 00452B00(flare, 100.0f 00CE3D08); else 00452B80(flare, 0). Always 0051F050(ECX=[+2Ch] even when null, EDX=view+78h, dt, +5Ch). EH 00C7AEDE not modelled. Caller 00651800. C++ cc13_sweepB_hud_unit_view_update_lens_effect_006515b0. |
| 28 | 00696450 | BSP_FrontEnd_ReadAction55 | __fastcall(ECX out byte*) RET | 00696450..00696463: *out = 004D92B0(ECX=[00E188A8], 55h). Caller 0051AA70. C++ cc13_sweepB_frontend_read_action55_00696450. |
| 29 | 006AC260 | BSP_Entity_ForwardXZNormalized | __thiscall(ECX entity)(float* out) RET 4 | 006AC260..006AC2EC (006AC2B7 DE F1 = 1/len; 006AC2DD D8 C9): 00414DB0 when +C8h clear; v=(world+20h,0,world+28h); len=00419440(&v); inv=len>0?1/len:0; out=(inv*v.x, 0.0*inv (double 00D7A258), inv*v.z). Callers 006AC5D0, 006F3CC0. C++ cc13_sweepB_entity_forward_xz_006ac260. |
| 29 | 006AC4D0 | BSP_EntryList_RegisterWithManager21D0 | __thiscall(ECX owner) RET | 006AC4D0..006AC54C: nothing when [00E188A8]+21D0h null; std::list at +1FCh, per node value=[node+8]: object=[value] or [value+8], entry=object+1E4h or null; 00491E70(ECX=[[00E188A8]+21D0h], entry, 0). SCL checks not modelled. Caller 006F3010. C++ cc13_sweepB_list_register_entries_006ac4d0. |
| 29 | 006AC550 | BSP_EntryList_UnregisterFromManager21D0 | __thiscall(ECX owner) RET | 006AC550..006AC5CA: as 006AC4D0 with 0048CB30(ECX=manager, entry). Caller 007497F0. C++ cc13_sweepB_list_unregister_entries_006ac550. |
| 29 | 006BE730 | BSP_NativeVector20_PushBack_006be730 | __thiscall(ECX array {data,count,capacity})(const element*) RET 4 | 006BE730..006BE7DD: full -> capacity=cap*2+2, block=00BF55BE(cap*14h), five-dword copies (skipped for a null block), old data freed when non-null, data=block; then data+count*14h gets the element (skipped when null) and count++. Byte census: unreferenced. C++ cc13_sweepB_native_vector20_push_back_006be730. |
| 35 | 0070BBA0 | BSP_FlagParams_InitDefaults | __thiscall(ECX params) RET, EAX params | 0070BBA0..0070BC2B: string +8h={0,null}; +0h=2.5f (00CF87C8); +10h=0; +4h=1.5f (00CE380C); 0041DD40(+8h,11,0); memcpy(data,"Zaszlok.tga" 00CFD4FC, length) when data non-null. EH 00C8479B not modelled. C++ cc13_sweepB_flag_params_init_0070bba0. |
| 37 | 0071B5C0 | BSP_AuxResourceParser_CreateItem | __thiscall(ECX parser, unread)(source) RET 4, EAX item | 0071B5C0..0071B61D: operator new(54h) 00BF681B; 0071ABB0 when non-null else null; 0071B3E0(item, source) without a null test. Vtable slot 00CFD848. EH 00C84F5B not modelled. C++ cc13_sweepB_aux_parser_create_item_0071b5c0. |
| 43 | 00786C40 | BSP_PacketStatRecord_Reset | __thiscall(ECX record) RET, EAX record | 00786C40..00786C55: dwords +0..+0Ch = 0, word +0Ch = 270Fh. Referenced by the immediate at 0078C441. C++ cc13_sweepB_packet_stat_record_reset_00786c40. |
| 44 | 0078D850 | TMPL_RefPtr_Release_0078d850 | __thiscall(ECX slot) RET | 0078D850..0078D878: object=*slot; non-null: InterlockedDecrement (IAT 00CE2220) on object+4h, zero -> object vtable +0h (no args); *slot=0 either way. Byte census: unreferenced. C++ cc13_sweepB_ref_ptr_release_0078d850. |
| 45 | 0078DAA0 | BSP_OceanEnvironment_Construct | __thiscall(ECX env)(source, char flag) RET 8, EAX env | 0078DAA0..0078DB9C: vtables 00CEB130 then 00D04390, +4h=1, +ACh=+B0h=0; 00B9A330([00F889F0]); +A8h=00BA6FB0(source, flag); spectrum=new(90h)+00B97100 or null; 00B9DD90(+A8h, spectrum); +B4h=new(94h)+00B84E50 or null; local spectrum ref released (InterlockedDecrement, vt+0h at zero). EH 00C8AC86 not modelled. C++ cc13_sweepB_ocean_environment_construct_0078daa0. |
| 45 | 0078E370 | BSP_MovCamInterpolator_Init | __thiscall(ECX this)(word, float a, b, c, d) RET 14h, EAX this | 0078E370..0078E3AC: +4h..+10h = a..d, +0h = word, +14h = 0. Caller 0076A8A0. C++ cc13_sweepB_movcam_interpolator_init_0078e370. |
| 45 | 0078FC60 | BSP_MovCam_GetPathPointPair | __thiscall(ECX owner)(float* out, int index) RET 8, EAX out | 0078FC60..0078FC7E: r=007AD070(owner,index); out=(r+34h, r+38h). Caller 007A7200. C++ cc13_sweepB_movcam_path_point_pair_0078fc60. |
| 45 | 0078FCC0 | MovieCameraKeyframe_GetPosition | __thiscall(ECX keyframe)(float* out) RET 4, EAX out | 0078FCC0..0078FCDE: out = +810h, +814h, +818h. Caller 00795650. C++ cc13_sweepB_movie_keyframe_get_position_0078fcc0. |
| 45 | 00793670 | BSP_Recon_EnemyContactIndexOf | __thiscall(ECX slot)(unit) RET 4, EAX | 00793670..007936C3: i in [0,+DE4h): walk list +DE8h (next +4h) to node i, record=[node+8h]; record+4h == unit -> i; else -1. The accessor's out-of-range arm dereferences a null record (faults) in the image and here. Caller 007A84D0. C++ cc13_sweepB_recon_enemy_contact_index_of_00793670. |
| 45 | 00794B50 | BSP_MovCam_AimDeltaToTargetDefault | __thiscall(ECX camera)(float* out, target) RET 8, EAX out | 00794B50..00794B70: 00794910(camera, out, target, [[camera+4D0h]+54h]). Caller 007A91A0. C++ cc13_sweepB_movcam_aim_delta_default_00794b50. |
| 45 | 00797E40 | MovieCameraTrack_ForwardToKeyModifiers | __thiscall(ECX track)(float) RET 4 | 00797E40..00797EC7: std::list at +68h; key=[node+8]; kind +F8h not 0/3 and modifier +E4h non-null -> modifier vtable +10h(value). SCL checks not modelled. Caller 00798C50. C++ cc13_sweepB_movie_track_forward_modifiers_00797e40. |
| 45 | 00798C50 | BSP_HudMovieCamera_ForwardToTrackModifiers | __thiscall(ECX camera)(float) RET 4 | 00798C50..00798C7A: 00797E40(camera+2A4h, v), 00797E40(camera+328h, v). Vtable slot 00D0471C. C++ cc13_sweepB_movie_camera_forward_modifiers_00798c50. |
| 45 | 00799DE0 | BSP_MovieKeyframe_SetModifier | __thiscall(ECX keyframe)(modifier) RET 4 | 00799DE0..00799E93: old +E4h vtable +0h(1) and cleared; +E4h=modifier; modifier vt+4h(keyframe); kind=vt+30h(); kind 3: +25h=1; unit +1Ch: kind 0Fh -> +C0h=0.6981317f (00CE7D20), and when [+E4h]+78h set +C0h*=0042E740()[84h]; else kind 6 -> +C0h=0.5235988f (00CEC724). Callers 007A0EB0, 007A2370. C++ cc13_sweepB_movie_keyframe_set_modifier_00799de0. |
| 45 | 0079C830 | BSP_MovCam_ClearPathRecords | __thiscall(ECX camera) RET | 0079C830..0079C8EF: +474h=0 when non-zero; pointer vector +478h: each non-null slot vtable +0h(1), cleared; inlined erase(first,last) with count 0 (memmove_s 00BF67A7 skipped), last=first. SCL checks not modelled. Callers 0079D640, 007A7200, 007A74F0, 007A7C60. C++ cc13_sweepB_movcam_clear_path_records_0079c830. |
| 51 | 0081AF80 | BSP_EntryList_EnsureSideRecordMark999 | __thiscall(ECX owner) RET | 0081AF80..0081B001: std::list +2A4h; element=node+8; element+0Ch null -> new(1Ch) with +0Fh/+18h/+19h=1, +10h/+14h=0 (or null); [element+0Ch]+14h=999. Byte census: unreferenced. SCL checks not modelled. C++ cc13_sweepB_list_entries_mark_999_0081af80. |
| 51 | 0081B1F0 | TMPL_RefPtrArray_Destroy_0081B1F0 | __thiscall(ECX array {data,count}) RET | 0081B1F0..0081B244: data null -> nothing; each slot i < count (re-read, unsigned) released as 0078D850; free(data) 00BF6989; +0h not cleared. C++ cc13_sweepB_ref_ptr_array_destroy_0081b1f0. |
| 51 | 0081B3C0 | TMPL_RefPtrArray_Grow_0081B3C0 | __thiscall(ECX array {data,count,capacity}) RET | 0081B3C0..0081B498: capacity=cap*2+2; block=00BF55BE(cap*4); per i<count: new slot zeroed and, with an old object, set + InterlockedIncrement; old slot released; old data freed when non-null; data=block. Byte census: unreferenced. EH 00C90EB7 not modelled. C++ cc13_sweepB_ref_ptr_array_grow_0081b3c0. |
| 51 | 0081B5B0 | TMPL_RefPtr_Release_0081B5B0 | __thiscall(ECX slot) RET | 0081B5B0..0081B5D8, byte-identical to 0078D850. Byte census: unreferenced. C++ cc13_sweepB_ref_ptr_release_0081b5b0. |
| 51 | 0081B5E0 | TMPL_RefPtr_Release_0081B5E0 | __thiscall(ECX slot) RET | 0081B5E0..0081B608, byte-identical to 0078D850. Byte census: unreferenced. C++ cc13_sweepB_ref_ptr_release_0081b5e0. |
| 51 | 0081B610 | BSP_FloatArray_Tidy_0081b610 | __thiscall(ECX array) RET | 0081B610..0081B626: 00818030(array, 0) then free([array]) via 00BF6989. Seven callers. C++ cc13_sweepB_float_array_tidy_0081b610. |
| 51 | 0081C0A0 | BSP_Unit_DisplayNameStatic | __thiscall(ECX unit) RET, EAX const char* | 0081C0A0..0081C148: function-static NativeString 00F8752C (guard 00F87534 bit 0, atexit 00CDE2B0 via 00BF6FF5); s=+160h (empty literal 00F87528 when null) when +15Ch != 0, else [+538h]+54h; 0041DD40(static, strlen(s), 0); memcpy when data non-null; returns data or the empty literal. Vtable slots 00CF90C4, 00CFA78C, 00CFB74C, 00CFC3E4. C++ cc13_sweepB_unit_display_name_0081c0a0. |
| 51 | 0081C830 | BSP_UnitInstance_ApplyControlledEffectScalar | __thiscall(ECX unit)(other, effect) RET 8 | 0081C830..0081C936: +700h=clock 00F876A4; other: kind [[other+354h]+80h] in {2,3,4,6} -> +708h=+700h; effect and kind [[other+3F4h]+80h] in {2,3,4}: InterlockedIncrement(effect+4) (00CE221C), 00815370(effect, byte +1008h ? 0 : 1.0f), 0074D780(unit+FFCh, &ref), InterlockedDecrement and vt+0h at zero. Vtable slots 00CF92B0, 00CFA978, 00CFB938, 00CFC5D0. EH 00C910A8 not modelled. C++ cc13_sweepB_unit_apply_controlled_effect_scalar_0081c830. |
| 51 | 0081C940 | BSP_RefPtrRange_Release | __fastcall(ECX first, EDX last)(2 unread stack words) RET 8 | 0081C940..0081C97B: each slot in [first,last) released as 0078D850 (IAT pointer loaded once). Seven callers. C++ cc13_sweepB_ref_ptr_range_release_0081c940. |
| 51 | 0081EC00 | BSP_UnitSubObject10A0_Destruct | __thiscall(ECX sub) RET | 0081EC00..0081ED14: vtable 00D09624; for each 8-byte element of vector +8h (size re-read) with a non-null pointer (NEG/SBB/TEST 00E186ECh idiom): 00867B10(p), p+9h=1; vector +18h: 004CC760(first,last,&vec,sub) + operator delete, cleared; vector +8h: 0081C980(first,last,&vec,sub) + operator delete, cleared. SCL checks and EH 00C91276 not modelled. Callers 0081F3A0, 00822660. C++ cc13_sweepB_unit_subobject_10a0_destruct_0081ec00. |
| 51 | 00827820 | BSP_Unit_Field114CExceedsSetting464 | __thiscall(ECX unit) RET, EAX 0/1 | 00827820..00827843: settings=00424C40(); 1 when unit+114Ch > settings+464h (FCOMIP, NaN -> 0). Byte census: unreferenced. C++ cc13_sweepB_unit_field114c_exceeds_setting_00827820. |
| 51 | 00827DD0 | BSP_LocalPointRecord_SetFromWorldXZ | __thiscall(ECX record)(float x, float y, float z) RET 0Ch | 00827DD0..00827E57: +4h=1; +8h=00424C40()+5ACh; entity=+0h; inverse byte +10Ch clear -> 00414DB0(entity) (no +C8h test), +10Ch=1, 00B63D50(entity+110h, entity+CCh); y slot overwritten with 0; 004142E0((x,0,z), +0Ch, entity+110h). Called at 00827F24. C++ cc13_sweepB_local_point_record_set_00827dd0. |

## Existing implementation, recorded (ledger only)

- 0081BEB0 BSP_UnitInstance_ExitMissionZone: existing host src/game_hosts_units.cpp route_unit_exited_00811e80 (cc9, labelled SUBSTITUTION, not edited). Image 0081BEB0..: returns when 004BCA50([00E188A8]) == 9; when [00F8A0C4] and [00E188A8] are non-null and 00927C50(unit, [00E188A8]+18ECh) is true, 00982120([00F8A0C4], unit, 1); Lua state lock (004C1570()+4, EnterCriticalSection IAT 00CE2218, +18h += 1); 00926D90(unit, 4) (Kill); 0092BD30([unit+1018h]); NativeString 00D05E00 via 0041E870, 00927B40(unit, &obj), 00B67460(obj, &name, arg) then releases. Adoption notes (LIVE): (1) the host skips the game-mode-9 early return (0081BED6) and the 00927C50 gate (0081BEFA) before the exitzone channel call; (2) 0092BD30 on +1018h (0081BF4C) is not modelled; (3) the Lua field store (0081BF6B..0081BF81) is a record only.
- 0081DCB0 BSP_Ship_ResetRandomTorpedoBarrel: existing host src/game_hosts_gunnery.cpp unload_random_torpedo_barrel_0081dcb0 (cc9, not edited). Image 0081DCB0..0081DDF3: reserves a 16-entry {gun, barrel} vector (0081D0C0), walks the category list +3ECh (next +4h, gun +8h), barrels i < gun+448h with timer [gun+414h][i] < FLT_MAX as double (00D7A278) pushed via 0081D900; n = element count (FILD, +2^32 when negative); u = 00BD2F10(ECX = 1, 0.0f, (float)n) (RNG 00BD2F10, stream 1); pick = FISTP truncation; then 0072CF00(ECX = gun, barrel, FLT_MAX 00D7A248, 0). Adoption notes (LIVE: the host runs this on the torpedo supply tick): (1) the image draws even with no loaded barrel and then reaches the secure-SCL handler 00BF6713 (instructions 0081DDB2..0081DDC1); the host returns before drawing; (2) a pick outside the vector reaches 00BF6713 in the image, the host clamps to size-1; (3) the host stores FLT_MAX directly instead of calling 0072CF00.
- 00827FB0 BSP_ShipClass_LandsTroops: 00827FB0..00827FC6: AL = [+78Ch] != 0 && [+790h] != 0. Existing host: src/game_hosts_units.cpp GameUnitsHost::unit_class_lands_troops_vtable_2c returns landing_ship_class != 0 && landing_ship_amount != 0 for the eight ship descriptors whose vtable +2Ch is 00827FB0 (same test; the field-to-offset mapping +78Ch/+790h is the host's). No standalone copy written; host file not edited.

## Unreferenced, recorded (ledger only)

Byte census with tools/callsite_census.py: rel32 E8/E9 in .text and the literal address as a dword in .text/.rdata/.data (so vtable slots are covered); positive control 005457C0 = 3 calls. Not reconstructed because each is over about 80 instructions or depends on unidentified helpers.

- 00544FA0 (seg 16): 218 instructions; byte census (rel32 E8/E9 + absolute dword in .text/.rdata/.data) finds no reference, positive control 005457C0 = 3 calls
- 00554680 (seg 17): 88 instructions (x87 outline of a widget's bounds through 00554600 four times); byte census 0
- 005D9A80 (seg 22): 141 instructions; byte census 0 (lobby option table helper, docs/CC13_NG2.md)
- 006058A0 (seg 24): 182 instructions; byte census 0
- 00722FB0 (seg 38): 38 instructions with ECX/EDX register inputs and three unidentified CRT helpers (00BFA910, 00BFA7B0, 00BF7420); byte census 0
- 0077A4B0 (seg 43): 28 instructions (health-colour helper over cc9's 0077A2F0); byte census 0
- 0077A510 (seg 43): 44 instructions; byte census 0

## Out of scope: library instantiations (ledger only)

STL instantiations (user ruling 2026-10-16).

- 00544B60 (seg 15): std::vector<byte-SBO pair>::_Insert_n instantiation
- 005630F0 (seg 17): std::vector<40h leaderboard row>::_Insert_n instantiation
- 0056B850 (seg 18): std::set<uint32>/_Tree lower-bound find instantiation
- 00571B70 (seg 19): STL copy instantiation (session records)
- 00571BF0 (seg 19): STL copy_backward instantiation
- 00571C30 (seg 19): STL _Push_heap instantiation
- 00571CE0 (seg 19): STL sort helper instantiation
- 00572670 (seg 19): STL copy instantiation
- 00572720 (seg 19): STL copy_backward instantiation
- 00572790 (seg 19): STL _Med3 instantiation
- 005727F0 (seg 19): STL _Adjust_heap instantiation
- 005729B0 (seg 19): STL _Pop_heap instantiation
- 00573030 (seg 19): STL _Median instantiation
- 005730E0 (seg 19): STL _Make_heap instantiation
- 00573220 (seg 19): std::vector<session record>::erase instantiation
- 005736A0 (seg 19): STL _Unguarded_partition instantiation
- 005738C0 (seg 19): STL _Insertion_sort instantiation
- 00573FD0 (seg 19): STL _Sort_heap instantiation
- 00574550 (seg 19): std::vector<session record>::_Insert_n instantiation
- 00574A50 (seg 19): STL _Sort intro-sort loop instantiation
- 0057CF20 (seg 19): std::pair<vector<string>,string> copy-construct instantiation (also unreferenced: byte census 0)
- 0058EE40 (seg 20): std::map<NativeString,...> find-or-insert (operator[]) instantiation (00443D00 less, 0058D130 insert)
- 005F6220 (seg 23): std::vector<OptionValue>::_Insert_n instantiation

## Compiler adjustor thunks (ledger only)

`SUB ECX,8; JMP target` this-adjusting thunks emitted by the compiler.

- 00573BC0 (seg 19): compiler adjustor thunk: SUB ECX,8; JMP 00574370 (vtable slot 00CEF120)
- 0057C240 (seg 19): compiler adjustor thunk: SUB ECX,8; JMP 0057C540 (vtable slot 00CEF28C)
- 005CA8F0 (seg 21): compiler adjustor thunk: SUB ECX,8; JMP 005CAA20 (vtable slot 00CF16F8)
- 005CB860 (seg 21): compiler adjustor thunk: SUB ECX,8; JMP 005CBAD0 (vtable slot 00CF1790)
- 0060FE10 (seg 25): compiler adjustor thunk: SUB ECX,8; JMP 0060FFA0 (vtable slot 00CF4B84)

## cc9 lane (skipped again, no record)

Earlier docs leave these to cc9; listed for the lead.

- 0053D230 (seg 15): left to cc9 (docs/CC13_FE_10.md, CC13_FE_4.md): free-camera forward to the unit's vslots
- 00565D80 (seg 17): limbo respawn-input poll, cc9 per include/bsp/frontend_seg12_17_cc13.hpp
- 005CD0F0 (seg 22): in-game movie-screen wiring left to cc9 (docs/CC13_FE_10.md: 005CD0F0..005CDF40)
- 005CD160 (seg 22): in-game movie-screen wiring left to cc9 (docs/CC13_FE_10.md)
- 005CD9C0 (seg 22): in-game movie-screen wiring left to cc9 (docs/CC13_FE_10.md)
- 005CDB00 (seg 22): in-game movie-screen wiring left to cc9 (docs/CC13_FE_10.md)
- 005CDC50 (seg 22): in-game movie-screen wiring left to cc9 (docs/CC13_FE_10.md)
- 005CDD70 (seg 22): in-game movie-screen wiring left to cc9 (docs/CC13_FE_10.md)
- 005CDF30 (seg 22): in-game movie-screen wiring left to cc9 (docs/CC13_FE_10.md)
- 005CDF40 (seg 22): in-game movie-screen wiring left to cc9 (docs/CC13_FE_10.md)
- 005D36B0 (seg 22): surrender prompt result (cc9's BSP_Session_RequestPlayerSurrender path, docs/CC13_FE_10.md)
- 005DECA0 (seg 22): resets the mission statistics, "check with cc9 first" (docs/CC13_FE_10.md)
- 00644A20 (seg 26): in-game HUD root wiring left to cc9 (docs/CC13_FE_10.md)
- 00645BB0 (seg 26): in-game HUD root listener left to cc9 (docs/CC13_FE_10.md)
- 00647040 (seg 26): controlled-unit interface push left to cc9 (docs/CC13_FE_10.md)
- 00648930 (seg 26): controlled-unit selection left to cc9 (docs/CC13_FE_10.md)
- 0064A270 (seg 26): repair priority (gameplay) left to cc9 (docs/CC13_FE_10.md)
- 0064A2F0 (seg 26): ship order menu left to cc9 (docs/CC13_FE_10.md)
- 0066EA60 (seg 27): support manager register left to cc9 (docs/CC13_FE_10.md)
- 006BA670 (seg 29): airops band 006B0000-0070FFFF, gameplay left to cc9 (docs/CC13_GP_006B_0070.md "Left for cc9")
- 006BC030 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md)
- 006BC120 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md)
- 006BC3E0 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md)
- 006BC530 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md)
- 006BC5E0 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md); host copies in air_operations.cpp
- 006BC690 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md)
- 006BC6F0 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md)
- 006BC730 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md); air_operations.cpp
- 006BC800 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md); byte census 0
- 006BC870 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md); byte census 0
- 006BC890 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md)
- 006BC8E0 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md); air_operations.cpp
- 006BC960 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md); game_hosts_units.cpp
- 006BCA40 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md); game_hosts_units.cpp
- 006BCA80 (seg 29): airops band, cc9; game_hosts_units.cpp
- 006BCCE0 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md); byte census 0
- 006BCF40 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md)
- 006BCFD0 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md); byte census 0
- 006BD080 (seg 29): airops band, cc9; game_hosts_units.cpp
- 006BD230 (seg 29): airops band, cc9; cc13_seg30.cpp hook
- 006BD620 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md)
- 006BD800 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md)
- 006BD9E0 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md)
- 006BDAC0 (seg 29): airops band, cc9; game_hosts_units.cpp
- 006BDBF0 (seg 29): airops band, cc9 (docs/CC13_GP_006B_0070.md)
- 00758FE0 (seg 42): seg 42 mothership family: cc9 carrier lane, SHIP_CREATE layout unreconstructed (lead brief)
- 00759020 (seg 42): seg 42 MOTHERSHIP_CREATE family: cc9 carrier lane (lead brief)
- 007590E0 (seg 42): seg 42 MOTHERSHIP_CREATE family: cc9 carrier lane (lead brief)
- 007592A0 (seg 42): seg 42 MOTHERSHIP_CREATE family: cc9 carrier lane (lead brief)
- 00759330 (seg 42): seg 42 MOTHERSHIP_CREATE family: cc9 carrier lane (lead brief)
- 007593D0 (seg 42): seg 42 mothership InitAll: cc9 carrier lane (lead brief)
- 00759920 (seg 42): seg 42 LANDINGSHIP_CREATE: cc9 carrier lane (lead brief)
- 00766EF0 (seg 42): seg 42 MOTHERSHIP_CREATE: cc9 carrier lane (lead brief)
- 00778820 (seg 43): unit/formation behaviour, cc9 (docs/CC13_SEG43.md "Belongs to cc9")
- 00778860 (seg 43): cc9 (docs/CC13_SEG43.md)
- 007788D0 (seg 43): cc9 (docs/CC13_SEG43.md)
- 00779820 (seg 43): cc9 (docs/CC13_SEG43.md)
- 00779AA0 (seg 43): cc9 (docs/CC13_SEG43.md)
- 00779EB0 (seg 43): cc9 (docs/CC13_SEG43.md); byte census 0
- 0077A080 (seg 43): MT_FORMATION_SET construct, cc9 (docs/CC13_SEG43.md)
- 0077A2F0 (seg 43): cc9 (docs/CC13_SEG43.md)
- 0077A650 (seg 43): cc9 (docs/CC13_SEG43.md)
- 0077B050 (seg 43): cc9 (docs/CC13_SEG43.md)
- 0077C880 (seg 43): cc9 (docs/CC13_SEG43.md)
- 0077C980 (seg 43): cc9 (docs/CC13_SEG43.md)
- 0077CA60 (seg 43): cc9 (docs/CC13_SEG43.md)
- 0077CD80 (seg 43): cc9 (docs/CC13_SEG43.md)
- 0077D270 (seg 43): cc9 (docs/CC13_SEG43.md)

## Skipped with a reason (no record)

- 005D7070 (seg 22): register inputs (ECX game+60Ch, ESI list) and the 434h lobby scene record layout are unreconstructed (docs/CC13_NG2.md, CC13_SEGS_13_15_22_44.md)
- 005D8B80 (seg 22): lobby setting rows (24h records, _atol + __allshl bitmask) need the lobby settings layout (docs/CC13_NG2.md)
- 005D8FC0 (seg 22): lobby setting rows need the lobby settings layout (docs/CC13_NG2.md)

## Not reached in this pass (no record)

Large or x87-heavy bodies, or medium bodies with inline containers and temporaries, that this pass
did not reach. Instruction counts are from the export listing (or the live Ghidra listing).

- seg 15: 00528240 (553), 00529D70 (277), 0052A100 (271), 0052A470 (281), 0052A860 (215), 0052B080 (273), 0052BB10 (347), 0052C1B0 (368), 00534870 (241), 00536310 (382), 00536A90 (989), 00538F80 (1560), 0053BCC0 (501), 0053EE00 (310), 0053F2D0 (864)
- seg 16: 00547480 (265), 00547A20 (258)
- seg 18: 0056B610 (181), 0056D660 (234)
- seg 19: 0056D9A0 (237), 0056DD80 (421), 0056F320 (1061), 00570300 (324), 00578580 (198), 005787E0 (153)
- seg 20: 00583E50 (198), 00585C50 (174), 00588A80 (138), 0058CE30 (243), 00593CA0 (355), 00597870 (1081), 005987F0 (259), 0059D4A0 (136), 0059D7B0 (108), 005A1C60 (435), 005A2340 (251), 005A2730 (399), 005A2C30 (429), 005A3210 (257), 005A3610 (174), 005A3830 (186), 005A3BF0 (229), 005A4020 (144), 005A4200 (494), 005A6AD0 (734), 005A7550 (315), 005A7C20 (651), 005A8600 (678), 005A8F50 (173), 005A91E0 (405), 005A9770 (345), 005A9DB0 (868), 005AAC00 (239), 005AAEF0 (129), 005ADDA0 (5027), 005B25D0 (424), 005B2D70 (532), 005B3470 (1316), 005B45C0 (1465), 005B83E0 (96), 005B8810 (140), 005BE240 (804), 005C0BD0 (190), 005C4080 (259), 005C4D40 (371), 005C51F0 (149), 005C5470 (117)
- seg 21: 005C5860 (102), 005C5990 (136), 005CCE70 (133)
- seg 22: 005D18A0 (509), 005D20A0 (110), 005D26D0 (618), 005D43C0 (370), 005D63E0 (88), 005D73A0 (1155), 005DB050 (4159), 005DF510 (1391)
- seg 23: 005E0DB0 (219), 005E12A0 (359), 005E1CF0 (265), 005E2750 (630), 005E3290 (194), 005E3580 (1024), 005E7B10 (1206), 005E9BE0 (297), 005EA030 (193), 005EADC0 (1138), 005EBDD0 (331), 005EC250 (182), 005EE520 (387), 005F0CD0 (676), 005F24D0 (633), 005F3B50 (421), 005F65C0 (572), 005F98F0 (87), 005FA0F0 (723), 006003A0 (1017)
- seg 24: 00605630 (135), 0060ABD0 (1578)
- seg 25: 006113A0 (244), 00611770 (227), 00614320 (219), 00616C70 (314), 00617470 (255), 006179D0 (1139), 006191A0 (720), 0061B0F0 (576)
- seg 26: 00621C60 (569), 00623710 (631), 00626C50 (275), 00627960 (291), 00637080 (312), 00638C60 (140), 0063C110 (404), 0063C620 (838), 0063D1E0 (863), 00640A20 (160), 00640D70 (615), 00641910 (340), 00641E30 (142), 00642040 (641), 00642960 (164), 00642C20 (321), 00643360 (108), 0064AE20 (244), 0064B170 (124), 0064B370 (151), 00650450 (191), 00651370 (125), 00652C80 (348)
- seg 27: 0065BF00 (259), 0065D220 (657), 0065FBF0 (431), 00660B10 (545), 00661240 (562), 00668770 (261), 006697E0 (258), 0066A8A0 (253), 00670020 (267), 00671D60 (255), 00673A10 (259), 00674B80 (1212), 0067F4D0 (1222)
- seg 28: 0068BC80 (231)
- seg 29: 006AC030 (93), 006AC370 (84), 006ACB40 (100), 006ACDC0 (227), 006AD600 (164), 006AF780 (66), 006B9450 (818)
- seg 31: 006D5220 (910)
- seg 32: 006DE5C0 (354)
- seg 43: 0076F0E0 (78), 007700E0 (68), 00770E50 (90), 00775160 (101), 00776230 (187), 00776930 (1125), 0077B280 (301), 0077C160 (110), 0077C540 (100), 0077D3F0 (86), 0077D7F0 (85), 0077F5E0 (140), 0077F7B0 (95), 0077FAD0 (245), 0077FE80 (156), 00780DB0 (176), 00784B10 (93)
- seg 44: 007884D0 (1179), 00789560 (928), 0078A430 (729)
- seg 45: 0078DBA0 (105), 0078DCF0 (137), 00795D90 (103)
- seg 46: 007A49A0 (133), 007A4EF0 (156), 007A5110 (547), 007A5A00 (498), 007A6710 (124), 007A7200 (231), 007A74F0 (469), 007A7C60 (117), 007A84D0 (225), 007A87B0 (345), 007A8C60 (94), 007A8DA0 (315), 007A91A0 (1815), 007ADD70 (252)
- seg 51: 0081B630 (131), 0081B7C0 (213), 0081BA80 (293), 0081BFE0 (45), 0081C1F0 (94), 0081C550 (205), 0081E350 (144), 00822C20 (2069), 00824B60 (470)

