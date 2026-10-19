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

The 28 suspect and untagged rows were each read in full from the disk bytes. Seven are game
bodies with no reconstruction and are rebuilt below. The other 21 are compiler wrappers
(adjustor-style destructor thunks, scalar or vector deleting destructors, two single-JMP thunks)
whose inlined class destructor is either trivial or a copy of an already reconstructed body;
they get bind-only ledger records that cite that body. The ~1490 remaining library rows stay
unbound by the lead's ruling.

## Reconstructed

| Address | Name | C++ | ABI | Notes |
|---|---|---|---|---|
| `0080FCF0` | BSP_SessionMessage_SHIP_GUNS_SYNC_Construct | `ship_guns_sync_construct_0080fcf0` | __thiscall(ECX = message, unit, phase, force), RET 0Ch | 0075B430(8Dh); +18h/+1Ah/+4h zero; +1Ch unit; +20h = phase > 1; vtable 00D03374; +22h = 0; +21h = phase > 0 or force; 64h records of 44h at +24h (0075FE90 / 0075A5B0). Only caller 00816C12 in BSP_UnitInstance_SendShipSyncMessage. |
| `00760C10` | BSP_SessionMessage_SHIP_GUNS_SYNC_ConstructForRead | `ship_guns_sync_construct_for_read_00760c10` | __thiscall(ECX = message), RET | Factory row 8Dh / 1AB4h. Header fields zeroed inline (no 0075B430 call), vtable, records, +1Ch = 0. |
| `00760CB0` | BSP_SessionMessage_SHIP_GUNS_SYNC_Destruct | `ship_guns_sync_destruct_00760cb0` | __thiscall(ECX = message), RET | Records destroyed, then vtable 00CE4974. |
| `00939CB0` | BSP_UnitController_Construct | `unit_controller_construct_00939cb0` | __thiscall(ECX = controller, unit), RET 4 | Was only a host hook (0080DEC0 in docs/CC13_SWEEPC.md). Listener subobject +20h (00D1961C, mask 7FF9h, back pointer +28h); 14h slots of 20h at +8Ch (009318D0 / 0092FD50); five vector triples; +378h and +384h from the zero vector 00F87574; ends with 00937C90. |
| `00932840` | BSP_UnitController_Destruct | `unit_controller_destruct_00932840` | __thiscall(ECX = controller), RET | docs/UNIT_DESTRUCTOR_LEVELS.md left the body unmodelled. Vectors +360h (009316F0), +33Ch, +32Ch (00931760), +31Ch (009316F0), each range destroy with ECX = first, EDX = last, stack (vector, controller); then +310h, the slot array, +34h, +8h. |
| `00940E40` | BSP_GuiSoundSet_DestroyBody | `sound_request_queue_destruct_00940e40` | __thiscall(ECX = queue), RET | Five RefPtrs at +4h (004C3810), [00F89B34] = 0, base vtable 00CE3818. The class is called the GUI sound set in include/bsp/sound_streaming_cc13.hpp and the sound request queue in src/engine_seg4_6_9_cc13.cpp; the ledger name is kept. |
| `006C23D0` | BSP_AirOpsHolder_ScalarDeletingDestruct | `air_ops_holder_scalar_deleting_destruct_006c23d0` | __thiscall(ECX = holder, flags), RET 4 | Holder destructor inlined: vtable 00CF86A4, 006BF880(ECX = 00E19948, holder); _free when flags & 1. |

## Bound with a citation

`0081ACF0`, `006D37A0` (inlines 006D3770), `007378B0`, `007416B0` (inlines 00741180),
`00743E30`, `00743E60`, `00749780` (JMP 0080E380, renamed thunk_BSP_SessionMessage_SHIP_CREATE_Destruct;
it had shared its target's name; status `existing_recorded_thunk`), `0074CC90`, `0074CCC0`, `00750520`, `008139E0` (inlines 0080E380),
`0093CC80`, `0093CC90`, `00943F70` (JMP 00943BB0), `0094CAD0` (JMP 0094CA40), `0094E2A0` (TOrderedMap
vector deleting destructor over 0094CA40 / 0094CAD0, iostdnet allocators 00A51E70 / 00A51EB0),
`007AC950`, `007AC980`, `007B0ED0`, `007B1050`, `007B2D30`.

## Uncertainty

- SEH state stores are not modelled in any of the seven bodies.
- The controller's vector element types (009316F0 / 00931760 destroy ranges) are not established.
- 00F87574 is read through the host; every xref reads it, so it is the zero vector in practice.
- The suspect filter is a heuristic: a library-tagged game body with no vtable store would not be
  flagged. Untagged rows were all read.
