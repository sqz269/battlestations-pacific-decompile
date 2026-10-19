# CC13 census of nine segments (cc13-census9)

Packet cc13-census9, worker cc13-seg67n, 2026-10-19. Segments 51, 58, 30, 41, 42, 50, 61, 54
and 46, struck ranges excluded (none of these segments overlaps them; 0081B000-0081EFFF in
segment 51 is released). Names are hypotheses, not recovered symbols. Code:
`include/bsp/cc13_census9.hpp`, `src/cc13_census9.cpp` (registered in `cmake/startup.cmake`,
wired into nothing). Status: reconstructed and build-tested; not ABI-compatible, not
game-validated.

## Census

Open rows mean non-funclet functions with no reconstruction record, from the index of
2026-10-19 (`local/seg59_census.py`). A library-tagged row was flagged as suspect when it stores
an image vtable through a non-ESP register and calls a non-library function.

| Segment | Range | Open | Library | Suspect | Untagged |
|---|---|---|---|---|---|
| 51 | 0081AA60-00828870 | 101 | 100 | 1 | 0 |
| 58 | 008E3DE0-008EE5F0 | 124 | 124 | 0 | 0 |
| 30 | 006BEB70-006D4340 | 161 | 159 | 2 | 0 |
| 41 | 00735F30-00758EB0 | 315 | 307 | 7 | 1 |
| 42 | 00758F90-0076A970 | 195 | 193 | 2 | 0 |
| 50 | 0080DA00-0081AA10 | 94 | 92 | 2 | 0 |
| 61 | 0092E0B0-00951F20 | 341 | 333 | 7 | 1 |
| 54 | 008742A0-00878350 | 43 | 43 | 0 | 0 |
| 46 | 007A49A0-007B2D80 | 65 | 60 | 5 | 0 |

The 28 suspect and untagged rows were each read in full from the disk bytes. Twelve are rebuilt
below, 14 are bound with a citation and two are thunk records. The ~1490 remaining library rows
stay unbound by the lead's ruling.

**Rule (the same as docs/CC13_SEG59.md).** A class's out-of-line destructor is rebuilt, including
the two-instruction form `MOV [ECX], vtable; JMP base` (seg59's 008F4160 / 008F4D50 / 008F4D60;
here 00743E30, 0074CC90, 0093CC80, 007AC950, 0094CAD0). A scalar or vector deleting destructor is
compiler-generated and bound; its record cites the out-of-line destructor whose body it repeats
inline. Where that out-of-line copy is not a defined function (00CFEA98's 007378A0 and 00D005C0's
007504E0 are unreferenced undefined chunks), the record cites the routine the inlined body calls,
which does all the work after the vtable store. A pure single-JMP with no store is a thunk record
(`existing_recorded_thunk`).

## Reconstructed

| Address | Name | C++ | ABI | Notes |
|---|---|---|---|---|
| `0080FCF0` | BSP_SessionMessage_SHIP_GUNS_SYNC_Construct | `ship_guns_sync_construct_0080fcf0` | __thiscall(ECX = message, unit, phase, force), RET 0Ch | 0075B430(8Dh); +18h/+1Ah/+4h zero; +1Ch unit; +20h = phase > 1; vtable 00D03374; +22h = 0; +21h = phase > 0 or force; 64h records of 44h at +24h (0075FE90 / 0075A5B0). Only caller 00816C12 in BSP_UnitInstance_SendShipSyncMessage. |
| `00760C10` | BSP_SessionMessage_SHIP_GUNS_SYNC_ConstructForRead | `ship_guns_sync_construct_for_read_00760c10` | __thiscall(ECX = message), RET | Factory row 8Dh / 1AB4h. Header fields zeroed inline (no 0075B430 call), vtable, records, +1Ch = 0. |
| `00760CB0` | BSP_SessionMessage_SHIP_GUNS_SYNC_Destruct | `ship_guns_sync_destruct_00760cb0` | __thiscall(ECX = message), RET | Records destroyed, then vtable 00CE4974. |
| `00939CB0` | BSP_UnitController_Construct | `unit_controller_construct_00939cb0` | __thiscall(ECX = controller, unit), RET 4 | Was only a host hook (0080DEC0 in docs/CC13_SWEEPC.md). Listener subobject +20h (00D1961C, mask 7FF9h, back pointer +28h); 14h slots of 20h at +8Ch (009318D0 / 0092FD50); five vector triples; +378h and +384h from the zero vector 00F87574; ends with 00937C90. |
| `00932840` | BSP_UnitController_Destruct | `unit_controller_destruct_00932840` | __thiscall(ECX = controller), RET | docs/UNIT_DESTRUCTOR_LEVELS.md left the body unmodelled. Vectors +360h (009316F0), +33Ch, +32Ch (00931760), +31Ch (009316F0), each range destroy with ECX = first, EDX = last, stack (vector, controller); then +310h, the slot array, +34h, +8h. |
| `00940E40` | BSP_GuiSoundSet_DestroyBody | `sound_request_queue_destruct_00940e40` | __thiscall(ECX = queue), RET | Five RefPtrs at +4h (004C3810), then the inlined singleton base destructor ([00F89B34] = 0, base vtable 00CE3818), which is out of line as `singleton_base_destruct_00940d80` (src/support_seg61_cc13.cpp:527). The class is called the GUI sound set in include/bsp/sound_streaming_cc13.hpp and the sound request queue in src/engine_seg4_6_9_cc13.cpp; the ledger name is kept. |
| `006C23D0` | BSP_AirOpsHolder_ScalarDeletingDestruct | `air_ops_holder_scalar_deleting_destruct_006c23d0` | __thiscall(ECX = holder, flags), RET 4 | Holder destructor inlined: vtable 00CF86A4, 006BF880(ECX = 00E19948, holder); _free when flags & 1. |
| `00743E30` | BSP_SessionMessage_LANDFORT_CREATE_Destruct | `landfort_create_message_destruct_00743e30` | __thiscall(ECX = message), tail JMP | vtable 00CFF358, +110h vtable 00CFF354, JMP 007668E0. |
| `0074CC90` | BSP_SessionMessage_LANDVEHICLE_CREATE_Destruct | `land_vehicle_create_message_destruct_0074cc90` | same | vtable 00CFFD18, JMP 007668E0. |
| `0093CC80` | BSP_Simple3dScene_Destruct | `simple3d_scene_destruct_0093cc80` | same | vtable 00D19724, JMP 0093CC20. |
| `007AC950` | BSP_PathSource_Destruct | `path_source_destruct_007ac950` | same | vtable 00D052DC, JMP 00BD30F0. Class name provisional (007B2CF0 OwnedPointsPathSource also stores 00D052DC). |
| `0094CAD0` | BSP_IostdnetOrderedMap_Destruct | `ordered_map_destruct_0094cad0` | same | vtable 00D25170, JMP 0094CA40. Rebuilt under the rule rather than given a thunk status: it stores a vtable before the JMP. |

## Bound with a citation

Deleting destructors, each with the out-of-line destructor whose body it inlines:
`0081ACF0` (inlines 00818100), `006D37A0` (inlines 006D3770), `007378B0` (inlines the
unreferenced undefined chunk 007378A0; both end in 00736F70), `007416B0` (inlines
00741180), `00743E60` (inlines 00743E30), `0074CCC0` (inlines 0074CC90), `00750520` (inlines the
unreferenced undefined chunk 007504E0; both end in 0074FC40), `008139E0` (inlines
0080E380), `0093CC90` (inlines 0093CC80), `0094E2A0` (TOrderedMap vector deleting destructor;
inlines 0094CAD0, iostdnet allocators 00A51E70 / 00A51EB0), `007AC980`, `007B0ED0`, `007B1050`
(each inlines 007AC950), `007B2D30` (inlines 007B2CF0 BSP_OwnedPointsPathSource_FreePoints).
"Unreferenced" means no rel32 CALL/JMP/Jcc and no absolute dword anywhere in the image names the
chunk's address.

Thunks (`existing_recorded_thunk`): `00749780` (JMP 0080E380, renamed
thunk_BSP_SessionMessage_SHIP_CREATE_Destruct; it had shared its target's name), `00943F70`
(JMP 00943BB0; Ghidra had FUN_00943f70, renamed thunk_STL_List_Tidy_00943BB0 under the lock, log local/ghidra-annotations-20261019T081012Z.json).

## Uncertainty

- SEH state stores are not modelled in any of the seven bodies.
- The controller's vector element types (009316F0 / 00931760 destroy ranges) are not established.
- 00F87574 is read through the host; every xref reads it, so it is the zero vector in practice.
- The suspect filter is a heuristic: a library-tagged game body with no vtable store would not be
  flagged. Untagged rows were all read.
