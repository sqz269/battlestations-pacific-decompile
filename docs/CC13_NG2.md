# CC13 NG2: the non-gameplay raw functions left after cc13-mopup1

Packet `cc13-ng2` (cc13 lineage, Claude), 2026-10-12. Input: the 176 functions that
`python tools/raw_ownership_cc13.py list ng` reported after cc13-mopup1 (docs/CC13_MOPUP_1.md, open
items): a `FUN_` name, no tag row and no ledger name, outside cc9's gameplay table.

## Result

| item | count |
|---|---|
| ledger names added (`config/names`, Ghidra renamed with evidence comments) | 100 |
| tag rows added (`config/tags`, source `docs/CC13_NG2.md`): 32 `trivial_body`, 15 `stl_shape`, 7 `block_dyn` | 54 |
| names marked provisional in their evidence | 40 |
| reconstructed and build-tested (`src/ng2_cc13.cpp`) | 2 |
| fixture-tested, ABI-compatible, game-validated | none |
| left raw in the `ng` list | 22 |

`python tools/raw_ownership_cc13.py table` after the packet: `total 4716 {'cc9': 4238, 'ng': 22,
'sweep79': 476}` (before: `ng` 176).

## Method

- 129 of the 176 had no export; they were exported first (`bsp.py ghidra export`).
- Per function: callers and callees from the index, vtable membership from a scan of `.rdata`/`.data`
  for runs of function pointers (`local/ng2_vt_ref.py`, the cc13-mopup1 scanner), neighbouring slot
  names (`local/ng2_vtdump.py`), string references, and for the 71 functions with no caller and no
  vtable slot a byte scan of the image for absolute dwords and rel32 CALL/JMP targets
  (`local/ng2_dwscan.py`). 60 of those 71 have no reference anywhere in the image (dead code kept
  by the linker); they were named from body shape and neighbours only.
- Assembly was read where the pseudocode had register inputs or x87: 00BB8580, 0059AA40, 00BE4410,
  00B34400, 00ADEB00, 00B87100/00B87600, 00C16D26, 00C10440/00C14DE0, 00A9E428.
- Ledger rows through `ledger.upsert` after `coordination.check_writable`; Ghidra names through
  `tools/ghidra_annotate.py --apply`; tags through `ghidra_tag.apply` under `ghidra_lock`
  (`local/ng2_apply.py`, a copy of `mopup_apply.py` with a `dyn` row kind). Decision files:
  `local/ng2_dec1..7.tsv` in the worktree.
- Tags reuse existing evidence ids: `st1_trivial` (trivial body), `mop1_stl` (template container
  helper), `e0638` (inside the Dyn physics library range 00C30930..00C5DF60, where the neighbours carry
  `DYN_physics_*` tags). The per-address notes are in the decision table below and in this file's
  history; the tag rows themselves carry only the evidence id.

## Findings worth keeping

- **MPKG decode helpers.** 00BB8580 is an unreferenced out-of-line copy of the reversed-block
  source index that `BSP_MPKG_OpenDecodedArchive` 00BB9920 inlines (block length 2F1h, final block
  length `size % 2F1h`, global index modulo the short length). 00BB9180 is an unreferenced
  whole-stream variant of the same decode (reversed blocks XOR table 00E144F0 period 219h) that
  short-circuits when the stream is already a memory stream, and 00BB9100 is a second copy of
  `BSP_MemoryStream_CreateFromBacking` with the seg-93 vtable 00D64328.
- **CRT positional/secure wide output cores.** Open question 1 of cc13-mopup1 is answered for the two
  unnamed cores: 00C10440 is `_woutput_p_l` (pushed by `_vscwprintf_p`/`_vscwprintf_p_l` at
  00BF97A3/00BF97BD and by `_vswprintf_p`/`_vswprintf_p_l` at 00BFAD3E/00BFAD65; the non-positional
  pair pushes 00C0F943) and 00C14DE0 is `_woutput_s_l` (`__vswprintf_s_l`, `__vsnwprintf_s_l`). Both
  read the format as words (MOVZX ...,word ptr), so they are the wide variants. 00C0F943 stays
  `LIBCRT_unmatched` here (tagged, not in the list); by the same thunks it is `_woutput_l`.
- **_RTC_Terminate.** 00C16D26 is registered by `__cinit` (push 0C16D26h; call _atexit at
  00BFBC86) and walks an empty pointer table at 00D83E7C.
- **Split function.** 00A9E428 is not a function: it is the continuation of
  `BSP_GuiListbox_ReadProperties` 00A9E400 after `CALL BSP_GuiWidget_ReadPropertiesAndChildren`
  at 00A9E423, where the Ghidra body of 00A9E400 ends. It is named `..._SplitTail`; merging it back
  (delete the 00A9E428 function, extend 00A9E400) is a listing repair that was not applied. 0056B188
  is the same kind of split (register frame of 0056B0F0) and is tagged with its head.
- **RNG.** 00BC7C10 (`BSP_SceneNode_SetWorldMatrixRandomRollFrom`, unreferenced) draws from the
  shared gameplay stream 0 (`BSP_Random_UniformFloatRange(0, 0, [00CE3D9C])`). It is not
  reconstructed; any host port must use a presentation stream (RNG rule).
- **Name families confirmed.** `BSP_LuaObject_IsBound` 00B65F70 is the negation of
  `BSP_LuaObject_IsUnbound` 00B66420; `BSP_LuaWriter_EnterNewTable` 00BD8EC0 is the writer twin of
  `BSP_LuaReader_EnterKey` 00BD8E20; `BSP_MsarProvider_EnumerateNames` 00BBAD60 is the MSAR twin of
  `BSP_MpkgProvider_EnumerateNames`; `BSP_TextureSourceBase_GetRuntimeTypeNameAddress` 00C301C0 follows
  the token+0Ch pattern of `BSP_Object_GetRuntimeTypeNameAddress` 00B74340.
- **Foliage teardown.** 00AE1C80 / 00AE4C20 are the bush-group and tree-group destructors that
  `BSP_FoliageSystem_RemoveLandscape` runs over FoliageSystem+58h and +68h
  (docs/CC13_FOLIAGE_TERRAIN_SETUP.md); 00AE4F70 is the tree quad-node twin of
  `BSP_BushQuadNode_DestroyChildren`.

## Reconstructed (build-tested only)

`include/bsp/ng2_cc13.hpp`, `src/ng2_cc13.cpp`, registered in `cmake/startup.cmake` (bsp_core).

| address | C++ | native ABI | notes |
|---|---|---|---|
| 00BB8580 | `mpkg_reversed_source_index_00bb8580` | ECX index (unsigned divide), EDX size (signed divide), EAX, RET | same formula as `src/native_mpkg_archive.cpp:102-106`; a zero-length final block is a native divide fault, returned as 0 here |
| 0059AA40 | `piecewise_lerp4_0059aa40` | __stdcall(k0,k1,k2,k3,t), x87, RET 14h | thresholds 1.999/2.999/3.999 (doubles 00CEFF80/88/90), `f = t - (float)floor(t)`; the native sum is rounded from 80-bit to float, the C++ from double to float (both products exact), so only a double-rounding tie can differ |

`./scripts/build.ps1` in the worktree: build and the two CTest suites passed. No fixture compares
either function against the image yet.

## Left raw (22)

| address | why it is still open |
|---|---|
| 005B5D20, 005B83E0 | in 005B5C00..005BCFFF (voice manager, blackout fade, narrative screen), cc9's lanes per docs/CC13_FRONTEND_MENUS.md |
| 005CAFF0, 005CB160 | HUD movie screen destructor body (vtable 00CF17A8) and a helper called from `BSP_EngineMovie_Start` 0045DB40: engine-movie wiring is cc9's |
| 00453570 | JMP thunk to 00465610 (a destructor of an abstract class, vtable 00CE5498, called from unwind funclets); the target is unnamed and outside the list |
| 0043AD80 | unreferenced; vtbl+A0h/+9Ch getters and a vertex-buffer write with x87 constants 00CE42C8..D8; owner class not found |
| 0047F200, 0048C6A0, 00481510, 00484A10, 004D62C0 | segment 7/10 entity, quad-tree and border-zone helpers (matrix build with RotationY; per-cell entity kill; border-zone list rebuild); gameplay-adjacent, need the owning classes |
| 004EE300 | scene command record builder ("Type" property, `BSP_ScenePropertyBag_Find`, `BSP_PropertyRecord_PublishPhaseThenRelease_Provisional`); unreferenced except a data pointer at 004F29E3, cc12's scene-property area |
| 0050B440, 0050B730 | TacticalLibraryScreen list rebuilds (0x2C records by id via `_atol`); the two lists were not told apart |
| 00544FA0 | debug text list layout (rows of 1Ch records, widget cloning); unreferenced |
| 005D7070, 005D8B80, 005D8FC0, 005D9A80 | multiplayer lobby scene/option tables (434h-byte scene records, 24h-byte setting rows, an `_atol` + `__allshl` bitmask); need the lobby settings layout |
| 00626C50, 00627960 | session game-setup builders (`BSP_Session_SendGameSetupMessage`, mission scoring commit records, Lua option tables); callers 008BDEC0/00920A20 are in gameplay bands |
| 00AE32A0 | 207-line foliage placement routine next to `BSP_FoliageCell_ClassifyPlane`; unreferenced |

## Decision table

All 154 records. "provisional" marks names whose evidence says the role or the owning class is not
established. The evidence text of every name is in its ledger row (`bsp.py lookup <address>`).

| address | name | record |
|---|---|---|
| 0043DD80 | `BSP_FloatPairVector_SetSorted` | name, provisional |
| 0043E4D0 | `BSP_FloatStringTable_RecordTimestamp` | name, provisional |
| 00481090 | `BSP_EntityViewCheck_IsInViewWithFade_00481090` | name, provisional |
| 004812F0 | `BSP_EntityViewCheck_IsInView_004812f0` | name, provisional |
| 00487360 | `STL_inst_00487360` | tag stl_shape |
| 004BBDD0 | `BSP_Game_ClipSegmentToWorldBounds` | name |
| 004BEAD0 | `BSP_Unit_CastIfClassFlag0F` | name |
| 004E6820 | `TRIV_body_004e6820` | tag trivial_body |
| 004FAAB0 | `BSP_TacticalLibraryScreen_StopTutorialMovieCallback` | name |
| 0056B0F0 | `STL_inst_0056b0f0` | tag stl_shape |
| 0056B188 | `STL_inst_0056b188` | tag stl_shape |
| 0059A910 | `BSP_HudMap_ClassifyOffscreenEdge` | name, provisional |
| 0059AA40 | `BSP_Math_PiecewiseLerp4` | name |
| 0059AC50 | `BSP_Unit_GetHudHealth` | name, provisional |
| 0059BD20 | `BSP_OptionalVector3_GetOrDefault` | name, provisional |
| 0059C920 | `BSP_Game_GetWorldBoundsCenter` | name |
| 0059CEF0 | `BSP_HudMapIcon_UpdateSizeAndVisibility` | name |
| 0059D150 | `BSP_HudMapIcon_QueryUnitStatusFlags` | name, provisional |
| 0059D2A0 | `BSP_HudMapIcon_RefreshSideTexture` | name |
| 0059D350 | `BSP_HudMapLabel_CenterOnWidget` | name, provisional |
| 0059D410 | `BSP_HudMap_IsPointInsideMapArea` | name |
| 0059D4A0 | `BSP_HudMapIcon_PlaceWidget` | name |
| 0059D6D0 | `BSP_HudMapIcon_UpdateOffscreenEdge` | name |
| 0059D940 | `BSP_HudMapScreen_IsUnitShownAtZoom` | name |
| 0059DD20 | `BSP_HudMapArrowWidget_SetLine` | name |
| 0059E120 | `BSP_Game_ClampPointToWorldBoundsXZ` | name |
| 0059F7C0 | `BSP_HudMapScreen_WorldToClampedMapPoint` | name |
| 0059F9B0 | `BSP_HudMapScreen_HideUnusedArrowWidgets` | name |
| 0059FAD0 | `TMPL_PointerArray_PushBack_0059fad0` | name |
| 005A07B0 | `BSP_GeneratedModel_FindMaterialByEffectName` | name |
| 005A1220 | `BSP_HudMapScreen_FindIconCarryingUnit` | name, provisional |
| 005A1C60 | `BSP_HudMapIcon_ReleaseWidgets` | name, provisional |
| 005A31A0 | `BSP_HudMapScreen_RefreshUnitIconTexture` | name |
| 005A3610 | `BSP_HudMapArrowWidget_Construct` | name |
| 005A3830 | `BSP_HudMapArrowWidget_SetAiArrow` | name |
| 005A4A00 | `BSP_HudMapScreen_AcquireArrowWidget` | name |
| 005A7550 | `BSP_HudMapArrowWidget_SetWaypointPath` | name, provisional |
| 005A7C20 | `BSP_HudMapIcon_RefreshPowerUpIcons` | name, provisional |
| 005A8420 | `BSP_HudMapIcon_RefreshUnitBadge` | name, provisional |
| 005A9DB0 | `BSP_HudMapScreen_BuildUnitOrderOverlays` | name, provisional |
| 005AAC00 | `BSP_HudMapIcon_RefreshCarriedUnitIcons` | name, provisional |
| 005ADDA0 | `BSP_HudMapScreen_UpdateUnitIcon` | name, provisional |
| 005B25D0 | `BSP_HudMapScreen_UpdateMarkerModels` | name, provisional |
| 005D0B10 | `BSP_MultiChatScreen_SendCurrentLine` | name, provisional |
| 005D6BA0 | `BSP_MultiGameLobby_SlotsMeetContentMasks` | name, provisional |
| 005D6C10 | `BSP_MultiGameLobbyScreen_RefreshMapAvailabilityIcon` | name, provisional |
| 005DAA60 | `STL_inst_005daa60` | tag stl_shape |
| 005DADB0 | `STL_inst_005dadb0` | tag stl_shape |
| 005F5070 | `BSP_OptionsScreen_RefreshGamePageRows` | name |
| 005FD420 | `TRIV_body_005fd420` | tag trivial_body |
| 006395E0 | `BSP_InGameHudMarkers_ProjectPartBounds` | name, provisional |
| 006FB030 | `BSP_ConvexHull_DebugDrawEdges` | name |
| 0098F490 | `STL_inst_0098f490` | tag stl_shape |
| 00990E00 | `TRIV_body_00990e00` | tag trivial_body |
| 00990E90 | `STL_inst_00990e90` | tag stl_shape |
| 00991310 | `std_basic_ifstream_dtor_00991310` | name |
| 009913C0 | `STL_inst_009913c0` | tag stl_shape |
| 009916C0 | `std_facet_ctor_009916c0` | name |
| 00991910 | `STL_inst_00991910` | tag stl_shape |
| 00991A90 | `std_basic_istream_Ipfx_00991a90` | name |
| 00991CE0 | `std_basic_istream_ctor_00991ce0` | name |
| 009926A0 | `std_basic_ifstream_ctor_009926a0` | name |
| 00993140 | `BSP_XLiveStatsField_Construct` | name, provisional |
| 00994EA0 | `TRIV_body_00994ea0` | tag trivial_body |
| 00A9ADC0 | `TRIV_body_00a9adc0` | tag trivial_body |
| 00A9E428 | `BSP_GuiListbox_ReadProperties_SplitTail` | name |
| 00AC2DC0 | `STL_inst_00ac2dc0` | tag stl_shape |
| 00ADB6F0 | `BSP_TerrainHeightField_StampByteAtWorld` | name, provisional |
| 00ADC8E0 | `BSP_TerrainTileGrid_ReleaseTiles` | name, provisional |
| 00ADEB00 | `BSP_TerrainTile_RefreshHeightRange` | name, provisional |
| 00AE1C80 | `BSP_BushGroup_Destruct` | name |
| 00AE2CB0 | `BSP_GeneratedModelFinder_Visit` | name, provisional |
| 00AE2EC0 | `BSP_TreeQuadNodePtr_Delete_Unreferenced` | name |
| 00AE4C20 | `BSP_TreeGroup_Destruct` | name |
| 00AE4DD0 | `TMPL_PointerArray_EraseValueSwapLast_00ae4dd0` | name |
| 00AE4F70 | `BSP_TreeQuadNode_DestroyChildren` | name |
| 00AEE220 | `BSP_TextureAtlasItem_Destruct` | name |
| 00AEF090 | `BSP_TextureAtlasRegistry_Destruct` | name, provisional |
| 00AF0540 | `BSP_NativeParticleModel_AdvanceVertexCursor` | name |
| 00AFF290 | `BSP_NamedPointerListArray_AddUnderName` | name, provisional |
| 00B171C0 | `BSP_SceneRenderer_ToggleStatsFilterName` | name |
| 00B1BD20 | `BSP_LuaGlobals_SetFieldsNamedByStringEntries` | name, provisional |
| 00B1F3C0 | `BSP_RenderCommand_CreateAndQueueInstanceGroup` | name, provisional |
| 00B24160 | `BSP_D3D9BindingState_Reset` | name, provisional |
| 00B25E10 | `TRIV_body_00b25e10` | tag trivial_body |
| 00B27A40 | `BSP_D3D9BindingState_Construct` | name, provisional |
| 00B28480 | `TRIV_body_00b28480` | tag trivial_body |
| 00B2F470 | `TRIV_body_00b2f470` | tag trivial_body |
| 00B2F670 | `TRIV_body_00b2f670` | tag trivial_body |
| 00B2F840 | `TRIV_body_00b2f840` | tag trivial_body |
| 00B34400 | `BSP_ShaderBinaryCache_AppendRecord_Unreferenced` | name, provisional |
| 00B34560 | `TRIV_body_00b34560` | tag trivial_body |
| 00B347C0 | `TRIV_body_00b347c0` | tag trivial_body |
| 00B3D5E0 | `TRIV_body_00b3d5e0` | tag trivial_body |
| 00B40120 | `TRIV_body_00b40120` | tag trivial_body |
| 00B403F0 | `TRIV_body_00b403f0` | tag trivial_body |
| 00B40580 | `TRIV_body_00b40580` | tag trivial_body |
| 00B40750 | `TRIV_body_00b40750` | tag trivial_body |
| 00B40780 | `TRIV_body_00b40780` | tag trivial_body |
| 00B436C0 | `TRIV_body_00b436c0` | tag trivial_body |
| 00B65F70 | `BSP_LuaObject_IsBound` | name |
| 00B687D0 | `BSP_LuaObject_DeepCopyToIndex` | name |
| 00B68AD0 | `BSP_LuaObject_DeepCopyToField` | name |
| 00B6A390 | `BSP_LuaObject_WriteLuaSourceText` | name, provisional |
| 00B6CB90 | `STL_inst_00b6cb90` | tag stl_shape |
| 00B7AD70 | `TRIV_body_00b7ad70` | tag trivial_body |
| 00B832F0 | `STL_inst_00b832f0` | tag stl_shape |
| 00B87090 | `TRIV_body_00b87090` | tag trivial_body |
| 00B870E0 | `TRIV_body_00b870e0` | tag trivial_body |
| 00B870F0 | `TRIV_body_00b870f0` | tag trivial_body |
| 00B87100 | `TRIV_body_00b87100` | tag trivial_body |
| 00B872A0 | `TRIV_body_00b872a0` | tag trivial_body |
| 00B872B0 | `TRIV_body_00b872b0` | tag trivial_body |
| 00B872C0 | `TRIV_body_00b872c0` | tag trivial_body |
| 00B872D0 | `TRIV_body_00b872d0` | tag trivial_body |
| 00B872E0 | `TRIV_body_00b872e0` | tag trivial_body |
| 00B875E0 | `STL_inst_00b875e0` | tag stl_shape |
| 00B87600 | `TRIV_body_00b87600` | tag trivial_body |
| 00B876B0 | `TRIV_body_00b876b0` | tag trivial_body |
| 00B87810 | `STL_inst_00b87810` | tag stl_shape |
| 00B87A50 | `TRIV_body_00b87a50` | tag trivial_body |
| 00B8A270 | `BSP_ResourceInterface_GetTypeDescriptor_00b8a270` | name, provisional |
| 00B94020 | `BSP_MeshResource_DispatchToVisitor` | name |
| 00BB3CE0 | `BSP_OceanHeightGenerator_ReadbackHeightmap` | name |
| 00BB8580 | `BSP_MPKG_ReversedSourceIndex` | name |
| 00BB8690 | `BSP_MemoryStream_CastFromStream_00bb8690` | name |
| 00BB9100 | `BSP_MemoryStream_CreateFromBacking_00bb9100` | name |
| 00BB9180 | `BSP_MPKG_DecodeStreamToMemory` | name |
| 00BBA500 | `BSP_MsarProvider_OpenEntryByIndex` | name |
| 00BBAD60 | `BSP_MsarProvider_EnumerateNames` | name |
| 00BC3580 | `BSP_CloudClass_PuffEdgeFactorAtPoint` | name, provisional |
| 00BC64C0 | `BSP_CloudPuff_WriteQuadVertices` | name |
| 00BC66B0 | `BSP_LightningSegment_WriteQuadVertices` | name |
| 00BC6FA0 | `BSP_LightningModel_FillSegmentVertices` | name |
| 00BC7C10 | `BSP_SceneNode_SetWorldMatrixRandomRollFrom` | name, provisional |
| 00BC9160 | `BSP_LightningModel_AgeAndRemoveExpired` | name |
| 00BD44C0 | `STL_inst_00bd44c0` | tag stl_shape |
| 00BD8EC0 | `BSP_LuaWriter_EnterNewTable` | name |
| 00BE4410 | `BSP_StreamRef_ReadFloatOperator_Unreferenced` | name |
| 00BEAA80 | `BSP_PcStorage_GetZeroPair` | name, provisional |
| 00C10440 | `_woutput_p_l` | name |
| 00C14DE0 | `_woutput_s_l` | name |
| 00C16D26 | `_RTC_Terminate` | name |
| 00C301C0 | `BSP_TextureSourceBase_GetRuntimeTypeNameAddress` | name |
| 00C327F0 | `DYN_physics_00c327f0` | tag block_dyn |
| 00C32830 | `DYN_physics_00c32830` | tag block_dyn |
| 00C32A60 | `DYN_physics_00c32a60` | tag block_dyn |
| 00C33250 | `DYN_physics_00c33250` | tag block_dyn |
| 00C33350 | `DYN_physics_00c33350` | tag block_dyn |
| 00C33420 | `DYN_physics_00c33420` | tag block_dyn |
| 00C33490 | `DYN_physics_00c33490` | tag block_dyn |
| 00C37030 | `DYN_SAPBroadPhaseManager_Construct_00C37030` | name |
| 00C44360 | `DYN_CollideBodyShapePairs_00C44360` | name, provisional |
| 00C55DF0 | `DYN_PointerList_RemoveAndDestroy_00C55DF0` | name |

## Verification notes

- Caller attributions for addresses the index has no edge for (00AFD90D, 00B052A3, 00C5B183,
  005DAC53, 005F59E0) come from the rel32 byte scan plus sp.py lookup's enclosing-candidate
  function; the containing bodies were not re-checked against INT3 padding.
- `tools/verify_report_calls.py` checks JSON reports; this packet has no JSON report, so it was not
  run. Argument counts for the two reconstructions come from the RET immediates (00BB85CA RET,
  0059AA80 RET 14h); 0059AA40 multiplies only by memory operands (D8 /1 m32), no register-register
  FMUL.
- No `bsp_game` run (run slots are cc9's).

## Handoff

Branch `agent/cc13-ng2`. Scripts and decision files in the worktree's `local/ng2_*`. The 22 open
functions above are the next step; most need an owning-class map (segment 7 entity helpers, lobby
settings layout, session setup) or belong to cc9 / cc12 lanes. The 00A9E428 split tail should be
merged back into 00A9E400 by a listing repair.
