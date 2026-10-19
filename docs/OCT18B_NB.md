# oct18b-nB primary reconstruction

32 assigned primary rows: 28 source bodies, four compiler/library classifications.

Existing GunBase, frame-hook, landing-ship, land-convoy, plane and MMothership source families are extended. No host wiring or game run. Explicit source dependencies preserve the existing family conventions; these source interfaces are not drop-in binary replacements.

The original assembly in shared exports/bsp/functions/<address>/assembly.txt was reviewed, including ECX adjustors, hidden read-stream cursor +4, x87 loads/stores and RET cleanup. Unknown virtual argument meanings and descriptive names remain provisional. The fish-school and other lane rows are covered in the child reports.

| Address | Symbol / classification | Original ABI | Evidence |
|---|---|---|---|
| 00728080 | gun_apply_damage_if_enabled_00728080 | ECX gun; float stack argument; RET 4 | CMP byte +3B8h; if zero FLD/FSTP argument and call 00442AE0 |
| 007289b0 | gun_failure_slot101_007289b0 | ECX ignored; two stack words; RET 8; AL result | XOR AL,AL; RET 8; GunBase vtables 00CFDC58 and 00D0DF70 slot 101 |
| 007289c0 | gun_failure_slot102_007289c0 | ECX ignored; three stack words; RET 0Ch; AL result | XOR AL,AL; RET 0Ch; same GunBase tables slot 102 |
| 007289d0 | gun_failure_slot103_007289d0 | ECX ignored; two stack words; RET 8; AL result | XOR AL,AL; RET 8; same GunBase tables slot 103 |
| 00737860 | frame_hook_slot08_noop_00737860 | ECX ignored; one ignored stack word; RET 4 | Bare RET 4; frame-hook table 00CFEA98 slot 2 |
| 00737870 | frame_hook_process_cloth_00737870 | ECX ignored; float dt on stack; RET 4 | FLD/FSTP dt; call 008E2560; frame-hook table 00CFEA98 slot 3 |
| 00741110 | land_convoy_slot164_false_00741110 | ECX ignored; one ignored stack word; RET 4; AL result | XOR AL,AL; RET 4; MLandConvoy table 00CEA570 slot 89 |
| 00744a00 | CG_LandFortEffect_ScalarDeletingDestructor_00744a00 | ECX message; stack deletion flags; RET 4; EAX self | Only reset vptr to 00CE4974 and conditional _free at 00744A11 |
| 00749d20 | landing_ship_create_read_00749d20 | ECX message; read-stream stack argument; RET 4 | 00813BD0 base read; stream+4 cursor reads WORD +12Ch/13, WORD +12Eh/12, signed DWORD +130h/4 |
| 00749e10 | land_at_pad_write_00749e10 | ECX message; cursor stack argument; RET 4 | 0075B480 header; zero-extended WORD +1Ch/13 and +1Eh/12 |
| 00749e40 | land_at_pad_read_00749e40 | ECX message; read-stream stack argument; RET 4 | 0075B4C0 header; stream+4 reads WORD +1Ch/13 and +1Eh/12 |
| 0074ccb0 | land_vehicle_create_read_0074ccb0 | Tail JMP preserves ECX message and stack stream; target RET 4 | JMP 00767E30 with no object adjustment |
| 0074cd90 | land_vehicle_handle_message_0074cd90 | Tail JMP preserves ECX vehicle and stack message; target RET 4; AL result | JMP 0095ABE0 with no object adjustment |
| 0074cef0 | land_vehicle_message_write_0074cef0 | Tail JMP preserves ECX message and stack cursor; target RET 4 | JMP 0075B480 with no object adjustment |
| 0074cf00 | land_vehicle_message_read_0074cf00 | Tail JMP preserves ECX message and stack stream; target RET 4 | JMP 0075B4C0 with no object adjustment |
| 0074d000 | land_vehicle_message_write_0074d000 | Tail JMP preserves ECX message and stack cursor; target RET 4 | JMP 0075B480 with no object adjustment |
| 0074d010 | land_vehicle_message_read_0074d010 | Tail JMP preserves ECX message and stack stream; target RET 4 | JMP 0075B4C0 with no object adjustment |
| 0074ddf0 | land_vehicle_register_ai_0074ddf0 | ECX vehicle; no stack arguments; RET | Reads game 00E188A8, selected bank index +18ECh and manager +21A4h+index*4; pushes vehicle to 008DDF00 |
| 0074e170 | plane_observer_0074e170 | ECX plane; no stack arguments; RET; EAX interior pointer | Null returns zero; otherwise LEA EAX,[ECX+72Ch] |
| 0074e270 | plane_copy_matrix_e00_0074e270 | ECX plane; stack destination; RET 4; EAX destination | Sixteen forward DWORD REP MOVSD copies from +E00h |
| 0074e290 | plane_copy_matrix_e40_0074e290 | ECX plane; stack destination; RET 4; EAX destination | Sixteen forward DWORD REP MOVSD copies from +E40h |
| 0074e3c0 | plane_observer_speed_0074e3c0 | ECX observer subobject; no stack arguments; RET; ST0 result | SUB ECX,72Ch; JMP 007B8E60 native cached-speed leaf |
| 0074e3d0 | CG_LargeReconPlane_DeletingDestructorAdjustor_0074e3d0 | ECX subobject; forwards stack deletion flags and cleanup | SUB ECX,10h; JMP scalar deleting destructor 0074E4C0 |
| 0074e3f0 | plane_observer_vector_0074e3f0 | ECX observer subobject; stack destination; RET 4; EAX destination | SUB ECX,72Ch; JMP 007BBB70; target three ordered FLD/FSTP copies from +AC8h |
| 0074eb20 | ship_sink_write_0074eb20 | ECX message; cursor stack argument; RET 4 | SHIP_SINK vtable 00D00518 slot 1; header, unsigned six-bit count +1Ch; each float +20h uses FLD/FSTP and numeric flags 0,1,live scale 00D7A248,32 |
| 0074eb80 | ship_sink_read_0074eb80 | ECX message; read-stream stack argument; RET 4 | SHIP_SINK table slot 2; header, stream+4 cursor, six-bit count; numeric results copied with MOVSS and count reloaded each iteration |
| 00752000 | STL_TreeIterator_PostIncrement_00752000 | ECX iterator; two stack words including postfix dummy; RET 8; EAX old-copy output | Copies iterator two DWORDs to output then calls STL_TreeIter_Inc_00750F00; no game behavior |
| 00758200 | mothership_air_ops_speed_00758200 | ECX ignored; no stack arguments; RET; ST0 result | FLD immutable binary32 [00CE89CC]; verified bytes 00 00 C8 41 = 25.0 |
| 00758230 | mothership_air_ops_class_00758230 | ECX air-ops subobject; no stack arguments; RET; EAX pointer | MOV EAX,[ECX-C50h]; ADD EAX,830h; no null check; vtable 00D015C4 slot 6 |
| 007584a0 | CG_Mothership_DeletingDestructorAdjustor_007584a0 | ECX embedded block; forwards stack deletion flags and cleanup | SUB ECX,1188h; JMP scalar deleting destructor 00758610 |
| 007589d0 | mothership_get_failure_007589d0 | ECX ship; native name, frame, byte output stack args; RET 0Ch; AL result | MMothership table 00D01630 slot 102; runway _stricmp first, hangar 00425850 second; bytes +11A4h/+11A5h; fallback 008135C0 forwards all args |
| 00758a50 | mothership_failure_expired_00758a50 | ECX ship; native name and frame stack args; RET 8; AL result | Runway/hangar construct message 73h/75h with profiles 00CF8B54/00CF8B7C; route 0077C2A0(this,msg,7,0); otherwise 00813660; compiler EH scope has no game cleanup effects |

Validation: Win32 scripts/build.ps1 passed; both existing CTests passed. Live call-site verifier checked 41 rows with zero failures. No fixture, original binary ABI, runtime or gameplay claim.

