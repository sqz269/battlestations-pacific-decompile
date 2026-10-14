# Segments 36 & 37 (`0070bc30` through `0071b710`) FlagModel, FlakProjectile, UnitGroup, LogicalVertexStream & ResourceItem Note/Aux/ZoneDesc Reconstruction (`argon-seg36-37`)

## Overview

This document records the evidence, ABI notes, x87 `DC`-form inspection, and graph verification for the 141 previously un-reconstructed non-funclet functions in Segments 36 and 37 (`0070bc30` through `0071b710`).

- **Header:** `include/bsp/flak_note_seg36_37_argon.hpp`
- **Implementation:** `src/flak_note_seg36_37_argon.cpp`
- **Report:** `reports/argon_seg36_37_flak_note.json`
- **Status:** `reconstructed_build_tested` (semantic C++ reconstructions; not drop-in binary ABI replacements)

## x87 `DC`-Form Register-Destination Audit

Running `python tools/x87_dc_form_scan.py` over `0070bc30` through `0071b710` reported 11 sites in 4 functions (`00710bb0` was already reconstructed and differential-tested in `src/model_groups.cpp`):

1. **`0x0070c4e1` (`dc c9`: `FMUL ST1, ST0`) in `0070c370` (`BSP_FlakProjectile_TickAdvance`):**
   Loads `0.5` (`double ptr [0x00d7a280]`) into `ST0` above `pos.x + prev.x` in `ST1`, executes `FMUL ST1, ST0` (`dc c9`), `FXCH`, `FSTP mid.x` while keeping `0.5` in `ST0` to scale `mid.y` (`0070c4ed`), `mid.z` (`0070c4fd`), and `0.5 * segLen + 2 * blastRadius` (`0070c51e`).
2. **`0x0070ff59` (`dc f9`: `FDIV ST1, ST0`) in `0070fdb0` (`BSP_LogicalVertexStream_ReadNormal`):**
   Loads divisor `32767.0` (`[0x00ce5f98]`, case 10 signed short) or `65535.0` (`[0x00ce5f90]`, case 12 unsigned short) into `ST0` above `nx` in `ST1`, executes `FDIV ST1, ST0` (`dc f9`), `FXCH`, `FSTP nx` while retaining the divisor in `ST0` to divide `ny` (`0070ff65`) and `nz` (`0070ff6b: FDIVR`). The 6 `dc ff` byte pairs from `0x0071005c` through `0x0071008c` are non-instruction jump-table data words (`0x0070ffdc`) in `.text` at `0x00710058`.
3. **`0x00710184` (`dc f9`: `FDIV ST1, ST0`) in `007100a0` (`BSP_LogicalVertexStream_ReadUV`):**
   Same divisor-retention pattern as `0x0070ff59`: divides `u`, `v`, and the unused third lane by `32767.0` (case 9) or `65535.0` (case 11) while keeping the divisor in `ST0`.

## Reconstructed Functions and Verified Sites

| Address | Original Name | Reconstructed Symbol | Size |
| :--- | :--- | :--- | ---: |
| `0070bc30` | `BSP_FlagModel_ScalarDelete` | `bsp::flak_note_seg36_37_argon::FlagModel_ScalarDelete_0070bc30` | 0 |
| `0070bc50` | `BSP_FlagParams_Assign` | `bsp::flak_note_seg36_37_argon::FlagParams_Assign_0070bc50` | 0 |
| `0070bcb0` | `BSP_FlagModel_BuildGeometry` | `bsp::flak_note_seg36_37_argon::FlagModel_BuildGeometry_0070bcb0` | 0 |
| `0070bee0` | `BSP_FlagModel_Setup` | `bsp::flak_note_seg36_37_argon::FlagModel_Setup_0070bee0` | 0 |
| `0070bfc0` | `BSP_FlagModel_Construct` | `bsp::flak_note_seg36_37_argon::FlagModel_Construct_0070bfc0` | 0 |
| `0070c030` | `BSP_FlakBulletClass_ReadLuaFields` | `bsp::flak_note_seg36_37_argon::FlakBulletClass_ReadLuaFields_0070c030` | 0 |
| `0070c0b0` | `BSP_FlakBulletClass_DeriveEngagementRange` | `bsp::flak_note_seg36_37_argon::FlakBulletClass_DeriveEngagementRange_0070c0b0` | 0 |
| `0070c0f0` | `BSP_FlakBullet_WriteState` | `bsp::flak_note_seg36_37_argon::FlakBullet_WriteState_0070c0f0` | 0 |
| `0070c1f0` | `BSP_Entity_AsKind05` | `bsp::flak_note_seg36_37_argon::Entity_AsKind05_0070c1f0` | 0 |
| `0070c370` | `BSP_FlakProjectile_TickAdvance` | `bsp::flak_note_seg36_37_argon::FlakProjectile_TickAdvance_0070c370` | 0 |
| `0070cae0` | `BSP_FlakProjectile_Construct` | `bsp::flak_note_seg36_37_argon::FlakProjectile_Construct_0070cae0` | 0 |
| `0070cb80` | `BSP_MFlakBullet_IsKindOf` | `bsp::flak_note_seg36_37_argon::MFlakBullet_IsKindOf_0070cb80` | 0 |
| `0070cc30` | `BSP_FlakBulletClass_CreateProjectile` | `bsp::flak_note_seg36_37_argon::FlakBulletClass_CreateProjectile_0070cc30` | 0 |
| `0070cea0` | `BSP_FlakBullet_Activate` | `bsp::flak_note_seg36_37_argon::FlakBullet_Activate_0070cea0` | 0 |
| `0070d1b0` | `BSP_Unit_ClassMaxSpeed` | `bsp::flak_note_seg36_37_argon::Unit_ClassMaxSpeed_0070d1b0` | 0 |
| `0070d7b0` | `BSP_UnitGroup_InitWithLeader` | `bsp::flak_note_seg36_37_argon::UnitGroup_InitWithLeader_0070d7b0` | 0 |
| `0070da00` | `BSP_UnitGroup_RefreshSpeedCeiling` | `bsp::flak_note_seg36_37_argon::UnitGroup_RefreshSpeedCeiling_0070da00` | 0 |
| `0070db20` | `BSP_UnitGroup_CreateForLeader` | `bsp::flak_note_seg36_37_argon::UnitGroup_CreateForLeader_0070db20` | 0 |
| `0070db60` | `BSP_UnitGroup_SwapSlotsByDistance` | `bsp::flak_note_seg36_37_argon::UnitGroup_SwapSlotsByDistance_0070db60` | 0 |
| `0070e3a0` | `BSP_Entity_AsKind06` | `bsp::flak_note_seg36_37_argon::Entity_AsKind06_0070e3a0` | 0 |
| `0070e3c0` | `BSP_UnitGroup_GetMaxFollowerTurnRadius` | `bsp::flak_note_seg36_37_argon::UnitGroup_GetMaxFollowerTurnRadius_0070e3c0` | 0 |
| `0070e450` | `BSP_ShipGroup_ControllerAreaKey` | `bsp::flak_note_seg36_37_argon::ShipGroup_ControllerAreaKey_0070e450` | 0 |
| `0070e4c0` | `BSP_UnitGroup_DetachMember` | `bsp::flak_note_seg36_37_argon::UnitGroup_DetachMember_0070e4c0` | 0 |
| `0070e620` | `BSP_UnitGroup_ReadSceneFields` | `bsp::flak_note_seg36_37_argon::UnitGroup_ReadSceneFields_0070e620` | 0 |
| `0070e890` | `BSP_ObserverMember_WriteRef_0070e890` | `bsp::flak_note_seg36_37_argon::ObserverMember_WriteRef_0070e890` | 0 |
| `0070eb00` | `BSP_ObserverMember_TargetWithinRange` | `bsp::flak_note_seg36_37_argon::ObserverMember_TargetWithinRange_0070eb00` | 0 |
| `0070eca0` | `BSP_UnitGroup_RemoveEntity` | `bsp::flak_note_seg36_37_argon::UnitGroup_RemoveEntity_0070eca0` | 0 |
| `0070ecf0` | `BSP_UnitGroup_CreateFromScene` | `bsp::flak_note_seg36_37_argon::UnitGroup_CreateFromScene_0070ecf0` | 0 |
| `0070f090` | `BSP_UnitPartNode_SphereOverlapForward` | `bsp::flak_note_seg36_37_argon::UnitPartNode_SphereOverlapForward_0070f090` | 0 |
| `0070f3d0` | `STL_PartGroupRows_Allocate_0070f3d0` | `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_Allocate_0070f3d0` | 0 |
| `0070f6b0` | `BSP_UnitPartCollisionShape_ConstructWithMesh` | `bsp::flak_note_seg36_37_argon::UnitPartCollisionShape_ConstructWithMesh_0070f6b0` | 0 |
| `0070f710` | `BSP_CollisionShapeD768_SegmentSlot` | `bsp::flak_note_seg36_37_argon::CollisionShapeD768_SegmentSlot_0070f710` | 0 |
| `0070f720` | `BSP_UnitPartCollisionShape_TestSphere` | `bsp::flak_note_seg36_37_argon::UnitPartCollisionShape_TestSphere_0070f720` | 0 |
| `0070f900` | `STL_LegacySboString_FindFirstOf_0070f900` | `bsp::flak_note_seg36_37_argon::STL_LegacySboString_FindFirstOf_0070f900` | 0 |
| `0070fdb0` | `BSP_LogicalVertexStream_ReadNormal` | `bsp::flak_note_seg36_37_argon::LogicalVertexStream_ReadNormal_0070fdb0` | 0 |
| `007100a0` | `BSP_LogicalVertexStream_ReadUV` | `bsp::flak_note_seg36_37_argon::LogicalVertexStream_ReadUV_007100a0` | 0 |
| `00710360` | `BSP_UnitPartEffectPointers_PushBack_00710360` | `bsp::flak_note_seg36_37_argon::UnitPartEffectPointers_PushBack_00710360` | 0 |
| `007103e0` | `BSP_UnitPartEntryPointers_PushBack_007103e0` | `bsp::flak_note_seg36_37_argon::UnitPartEntryPointers_PushBack_007103e0` | 0 |
| `007104e0` | `STL_List_Clear_007104e0` | `bsp::flak_note_seg36_37_argon::STL_List_Clear_007104e0` | 0 |
| `00710530` | `STL_List_Clear_00710530` | `bsp::flak_note_seg36_37_argon::STL_List_Clear_00710530` | 0 |
| `00710630` | `BSP_RefPtr_Release_00710630` | `bsp::flak_note_seg36_37_argon::RefPtr_Release_00710630` | 0 |
| `007106b0` | `BSP_RefPtr_Release_007106b0` | `bsp::flak_note_seg36_37_argon::RefPtr_Release_007106b0` | 0 |
| `00710700` | `STL_MoveDwords_00710700` | `bsp::flak_note_seg36_37_argon::STL_MoveDwords_00710700` | 0 |
| `007107e0` | `BSP_UnitPartEffectPointers_PushBack_007107e0` | `bsp::flak_note_seg36_37_argon::UnitPartEffectPointers_PushBack_007107e0` | 0 |
| `00710820` | `BSP_UnitPartEntryPointers_PushBack_00710820` | `bsp::flak_note_seg36_37_argon::UnitPartEntryPointers_PushBack_00710820` | 0 |
| `007109a0` | `BSP_RefPtr_Assign_007109a0` | `bsp::flak_note_seg36_37_argon::RefPtr_Assign_007109a0` | 0 |
| `007109e0` | `STL_FindDword_007109e0` | `bsp::flak_note_seg36_37_argon::STL_FindDword_007109e0` | 0 |
| `00710a60` | `BSP_RefPtr_Release_00710a60` | `bsp::flak_note_seg36_37_argon::RefPtr_Release_00710a60` | 0 |
| `00710e10` | `BSP_UnitPartEffect_AttachAtPart` | `bsp::flak_note_seg36_37_argon::UnitPartEffect_AttachAtPart_00710e10` | 0 |
| `00710f70` | `STL_List_Construct_UnitParts_00710f70` | `bsp::flak_note_seg36_37_argon::STL_List_Construct_UnitParts_00710f70` | 0 |
| `00710fa0` | `STL_List_Construct_UnitPartShapes_00710fa0` | `bsp::flak_note_seg36_37_argon::STL_List_Construct_UnitPartShapes_00710fa0` | 0 |
| `00711210` | `BSP_UnitPartEffect_StartByIndex` | `bsp::flak_note_seg36_37_argon::UnitPartEffect_StartByIndex_00711210` | 0 |
| `007114a0` | `STL_Vector_Tidy_007114a0` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_007114a0` | 0 |
| `00712100` | `STL_Vector_Assign_00712100` | `bsp::flak_note_seg36_37_argon::STL_Vector_Assign_00712100` | 0 |
| `00712270` | `STL_ConstructDwordVector_00712270` | `bsp::flak_note_seg36_37_argon::STL_ConstructDwordVector_00712270` | 0 |
| `00712330` | `STL_PartGroupRows_DestroyRange_Thunk_00712330` | `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_DestroyRange_Thunk_00712330` | 0 |
| `007123c0` | `STL_ConstructDwordVector_007123c0` | `bsp::flak_note_seg36_37_argon::STL_ConstructDwordVector_007123c0` | 0 |
| `00712900` | `STL_PartGroupRows_AssignForward_00712900` | `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_AssignForward_00712900` | 0 |
| `00712940` | `STL_PartGroupRows_AssignRepeated_00712940` | `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_AssignRepeated_00712940` | 0 |
| `00712970` | `STL_PartGroupRows_UninitializedFill_00712970` | `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_UninitializedFill_00712970` | 0 |
| `00712a10` | `STL_PartGroupRows_TransferConstruct_00712a10` | `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_TransferConstruct_00712a10` | 0 |
| `00712af0` | `STL_SwapBackward_Vector16_00712af0` | `bsp::flak_note_seg36_37_argon::STL_SwapBackward_Vector16_00712af0` | 0 |
| `00712f20` | `STL_PartGroupRows_UninitializedFillAdapter_00712f20` | `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_UninitializedFillAdapter_00712f20` | 0 |
| `00712f60` | `STL_PartGroupRows_TransferAdapter_00712f60` | `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_TransferAdapter_00712f60` | 0 |
| `00712f90` | `STL_PartGroupRows_BackwardTransferAdapter_00712f90` | `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_BackwardTransferAdapter_00712f90` | 0 |
| `00712ff0` | `STL_PartGroupRows_InsertCount_00712ff0` | `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_InsertCount_00712ff0` | 0 |
| `00713270` | `STL_PartGroupRows_Resize_00713270` | `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_Resize_00713270` | 0 |
| `00713750` | `BSP_ResourceItemNote_GetRuntimeTypeToken` | `bsp::flak_note_seg36_37_argon::ResourceItemNote_GetRuntimeTypeToken_00713750` | 0 |
| `00713760` | `BSP_ResourceItemNote_GetRuntimeTypeNameAddress` | `bsp::flak_note_seg36_37_argon::ResourceItemNote_GetRuntimeTypeNameAddress_00713760` | 0 |
| `00713790` | `BSP_ResourceItemAux_GetRuntimeTypeToken` | `bsp::flak_note_seg36_37_argon::ResourceItemAux_GetRuntimeTypeToken_00713790` | 0 |
| `007137a0` | `BSP_ResourceItemAux_GetRuntimeTypeNameAddress` | `bsp::flak_note_seg36_37_argon::ResourceItemAux_GetRuntimeTypeNameAddress_007137a0` | 0 |
| `007137d0` | `BSP_ResourceItemZoneDesc_GetRuntimeTypeToken` | `bsp::flak_note_seg36_37_argon::ResourceItemZoneDesc_GetRuntimeTypeToken_007137d0` | 0 |
| `007137e0` | `BSP_ResourceItemZoneDesc_GetRuntimeTypeNameAddress` | `bsp::flak_note_seg36_37_argon::ResourceItemZoneDesc_GetRuntimeTypeNameAddress_007137e0` | 0 |
| `00714030` | `BSP_GameResource_AsType44_00714030` | `bsp::flak_note_seg36_37_argon::GameResource_AsType44_00714030` | 0 |
| `00714060` | `BSP_GameResource_AsType54_00714060` | `bsp::flak_note_seg36_37_argon::GameResource_AsType54_00714060` | 0 |
| `00714090` | `BSP_ResourceItemZoneDesc_Cast` | `bsp::flak_note_seg36_37_argon::ResourceItemZoneDesc_Cast_00714090` | 0 |
| `007140c0` | `BSP_ResourceItemAux_Cast` | `bsp::flak_note_seg36_37_argon::ResourceItemAux_Cast_007140c0` | 0 |
| `007140f0` | `BSP_ResourceItemNote_Cast` | `bsp::flak_note_seg36_37_argon::ResourceItemNote_Cast_007140f0` | 0 |
| `007149b0` | `BSP_Matrix4f_InvertInto_007149b0` | `bsp::flak_note_seg36_37_argon::Matrix4f_InvertInto_007149b0` | 0 |
| `00715af0` | `STL_UninitCopy8_00715af0` | `bsp::flak_note_seg36_37_argon::STL_UninitCopy8_00715af0` | 0 |
| `00715b20` | `STL_UninitCopy8_00715b20` | `bsp::flak_note_seg36_37_argon::STL_UninitCopy8_00715b20` | 0 |
| `00715b50` | `STL_UninitCopy8_00715b50` | `bsp::flak_note_seg36_37_argon::STL_UninitCopy8_00715b50` | 0 |
| `00715b80` | `STL_UninitCopy8_00715b80` | `bsp::flak_note_seg36_37_argon::STL_UninitCopy8_00715b80` | 0 |
| `00715bb0` | `STL_UninitCopy8_00715bb0` | `bsp::flak_note_seg36_37_argon::STL_UninitCopy8_00715bb0` | 0 |
| `00716960` | `STL_Fill8_00716960` | `bsp::flak_note_seg36_37_argon::STL_Fill8_00716960` | 0 |
| `00716990` | `STL_Fill8_00716990` | `bsp::flak_note_seg36_37_argon::STL_Fill8_00716990` | 0 |
| `007169c0` | `STL_Fill8_007169c0` | `bsp::flak_note_seg36_37_argon::STL_Fill8_007169c0` | 0 |
| `007169f0` | `STL_Fill8_007169f0` | `bsp::flak_note_seg36_37_argon::STL_Fill8_007169f0` | 0 |
| `00716a20` | `STL_Fill8_00716a20` | `bsp::flak_note_seg36_37_argon::STL_Fill8_00716a20` | 0 |
| `00716b50` | `STL_UninitFillN8_00716b50` | `bsp::flak_note_seg36_37_argon::STL_UninitFillN8_00716b50` | 0 |
| `00716b80` | `STL_UninitFillN8_00716b80` | `bsp::flak_note_seg36_37_argon::STL_UninitFillN8_00716b80` | 0 |
| `00716bb0` | `STL_UninitFillN8_00716bb0` | `bsp::flak_note_seg36_37_argon::STL_UninitFillN8_00716bb0` | 0 |
| `00716be0` | `STL_UninitFillN8_00716be0` | `bsp::flak_note_seg36_37_argon::STL_UninitFillN8_00716be0` | 0 |
| `00716c10` | `STL_UninitFillN8_00716c10` | `bsp::flak_note_seg36_37_argon::STL_UninitFillN8_00716c10` | 0 |
| `00717510` | `STL_Vector_Tidy_00717510` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00717510` | 0 |
| `00717570` | `STL_Vector_Tidy_00717570` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00717570` | 0 |
| `007175a0` | `STL_Vector_Tidy_007175a0` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_007175a0` | 0 |
| `00717880` | `STL_CopyBackward8_00717880` | `bsp::flak_note_seg36_37_argon::STL_CopyBackward8_00717880` | 0 |
| `007178c0` | `STL_CopyBackward8_007178c0` | `bsp::flak_note_seg36_37_argon::STL_CopyBackward8_007178c0` | 0 |
| `00717900` | `STL_CopyBackward8_00717900` | `bsp::flak_note_seg36_37_argon::STL_CopyBackward8_00717900` | 0 |
| `00717940` | `STL_CopyBackward8_00717940` | `bsp::flak_note_seg36_37_argon::STL_CopyBackward8_00717940` | 0 |
| `00717980` | `STL_CopyBackward8_00717980` | `bsp::flak_note_seg36_37_argon::STL_CopyBackward8_00717980` | 0 |
| `00717c70` | `BSP_ResourceItemNote_Construct` | `bsp::flak_note_seg36_37_argon::ResourceItemNote_Construct_00717c70` | 0 |
| `00717ca0` | `BSP_ResourceItemNote_AcceptsRuntimeTypeToken` | `bsp::flak_note_seg36_37_argon::ResourceItemNote_AcceptsRuntimeTypeToken_00717ca0` | 0 |
| `00717cd0` | `BSP_ResourceItemNote_Destruct` | `bsp::flak_note_seg36_37_argon::ResourceItemNote_Destruct_00717cd0` | 0 |
| `00717d50` | `STL_String_Tidy_00717d50` | `bsp::flak_note_seg36_37_argon::STL_String_Tidy_00717d50` | 0 |
| `00717da0` | `BSP_ResourceItemZoneDesc_Construct` | `bsp::flak_note_seg36_37_argon::ResourceItemZoneDesc_Construct_00717da0` | 0 |
| `00717dd0` | `BSP_ResourceItemZoneDesc_AcceptsRuntimeTypeToken` | `bsp::flak_note_seg36_37_argon::ResourceItemZoneDesc_AcceptsRuntimeTypeToken_00717dd0` | 0 |
| `00717e00` | `BSP_ResourceItemZoneDesc_Destruct` | `bsp::flak_note_seg36_37_argon::ResourceItemZoneDesc_Destruct_00717e00` | 0 |
| `00717ef0` | `STL_String_Tidy_00717ef0` | `bsp::flak_note_seg36_37_argon::STL_String_Tidy_00717ef0` | 0 |
| `007180e0` | `STL_Vector_Tidy_007180e0` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_007180e0` | 0 |
| `00718110` | `STL_Vector_Tidy_00718110` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718110` | 0 |
| `00718140` | `STL_Vector_Tidy_00718140` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718140` | 0 |
| `007185f0` | `STL_Vector_Tidy_007185f0` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_007185f0` | 0 |
| `00718620` | `STL_Vector_Tidy_00718620` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718620` | 0 |
| `00718650` | `STL_Vector_Tidy_00718650` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718650` | 0 |
| `00718680` | `STL_Vector_Tidy_00718680` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718680` | 0 |
| `007186b0` | `STL_Vector_Tidy_007186b0` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_007186b0` | 0 |
| `00718710` | `CG_scalar_deleting_dtor_00718710` | `bsp::flak_note_seg36_37_argon::ResourceItemNote_ScalarDelete_00718710` | 0 |
| `00718a40` | `STL_Vector_Tidy_00718a40` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718a40` | 0 |
| `00718a70` | `STL_Vector_Tidy_00718a70` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718a70` | 0 |
| `00718aa0` | `STL_Vector_Tidy_00718aa0` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718aa0` | 0 |
| `00718ad0` | `STL_Vector_Tidy_00718ad0` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718ad0` | 0 |
| `00718b00` | `STL_Vector_Tidy_00718b00` | `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718b00` | 0 |
| `00718c70` | `BSP_ModelNoteNameEquals_Predicate` | `bsp::flak_note_seg36_37_argon::ModelNoteNameEquals_Predicate_00718c70` | 0 |
| `00718f50` | `BSP_StructuredNode_ReadCharacterString` | `bsp::flak_note_seg36_37_argon::StructuredNode_ReadCharacterString_00718f50` | 0 |
| `00719000` | `BSP_NoteResourceParser_Parse` | `bsp::flak_note_seg36_37_argon::NoteResourceParser_Parse_00719000` | 0 |
| `00719080` | `BSP_ZoneDescResourceParser_ReadItem` | `bsp::flak_note_seg36_37_argon::ZoneDescResourceParser_ReadItem_00719080` | 0 |
| `00719240` | `BSP_ZoneDescResourceParser_CreateItem` | `bsp::flak_note_seg36_37_argon::ZoneDescResourceParser_CreateItem_00719240` | 0 |
| `007192c0` | `STL_Vector_InsertN8_007192c0` | `bsp::flak_note_seg36_37_argon::STL_Vector_InsertN8_007192c0` | 0 |
| `00719510` | `STL_Vector_InsertN8_00719510` | `bsp::flak_note_seg36_37_argon::STL_Vector_InsertN8_00719510` | 0 |
| `00719760` | `STL_Vector_InsertN8_00719760` | `bsp::flak_note_seg36_37_argon::STL_Vector_InsertN8_00719760` | 0 |
| `007199b0` | `STL_Vector_InsertN8_007199b0` | `bsp::flak_note_seg36_37_argon::STL_Vector_InsertN8_007199b0` | 0 |
| `00719c00` | `STL_Vector_InsertN8_00719c00` | `bsp::flak_note_seg36_37_argon::STL_Vector_InsertN8_00719c00` | 0 |
| `00719fa0` | `BSP_ModelNoteList_FindIfNameEquals` | `bsp::flak_note_seg36_37_argon::ModelNoteList_FindIfNameEquals_00719fa0` | 0 |
| `0071aae0` | `BSP_ModelNoteList_FindByName` | `bsp::flak_note_seg36_37_argon::ModelNoteList_FindByName_0071aae0` | 0 |
| `0071abb0` | `BSP_ResourceItemAux_Construct` | `bsp::flak_note_seg36_37_argon::ResourceItemAux_Construct_0071abb0` | 0 |
| `0071ac10` | `BSP_ResourceItemAux_AcceptsRuntimeTypeToken` | `bsp::flak_note_seg36_37_argon::ResourceItemAux_AcceptsRuntimeTypeToken_0071ac10` | 0 |
| `0071ac40` | `BSP_ResourceItemAux_Destruct` | `bsp::flak_note_seg36_37_argon::ResourceItemAux_Destruct_0071ac40` | 0 |
| `0071ad50` | `BSP_Model_FindNoteNode` | `bsp::flak_note_seg36_37_argon::Model_FindNoteNode_0071ad50` | 0 |
| `0071b3e0` | `BSP_GameResource_ReadPointsItem` | `bsp::flak_note_seg36_37_argon::GameResource_ReadPointsItem_0071b3e0` | 0 |

## Call-Site Inventory

### `0070bc30` (`BSP_FlagModel_ScalarDelete` -> `bsp::flak_note_seg36_37_argon::FlagModel_ScalarDelete_0070bc30`)
- `0070bc33 CALL 0070b2c0` (`BSP_FlagModel_Destruct`)
- `0070bc45 CALL 0070acb0` (`BSP_FlagPool_Free`)

### `0070bc50` (`BSP_FlagParams_Assign` -> `bsp::flak_note_seg36_37_argon::FlagParams_Assign_0070bc50`)
- `0070bc76 CALL 0041dd40` (`BSP_NativeString_Resize`)
- `0070bc8b CALL 00bf7680` (`_memcpy`)

### `0070bcb0` (`BSP_FlagModel_BuildGeometry` -> `bsp::flak_note_seg36_37_argon::FlagModel_BuildGeometry_0070bcb0`)
- `0070bccf CALL 00b73b60` (`BSP_MeshObject_AllocationContextEntry`)
- `0070bce6 CALL 00b73d70` (`BSP_MeshObject_Construct`)
- `0070bd12 CALL 00b75170` (`BSP_GeneratedModel_SetGeometry`)
- `0070bd47 CALL 0070b9d0` (`BSP_FlagModel_BuildMaterial`)
- `0070bd5a CALL 00b6d890` (`BSP_Node_PropagateRootRegistration`)

### `0070bee0` (`BSP_FlagModel_Setup` -> `bsp::flak_note_seg36_37_argon::FlagModel_Setup_0070bee0`)
- `0070bef1 CALL 0070bc50` (`BSP_FlagParams_Assign`)
- `0070bef8 CALL 0070bcb0` (`BSP_FlagModel_BuildGeometry`)
- `0070bf74 CALL 00b74640` (`BSP_GeneratedModel_GetGeometry`)
- `0070bf7b CALL 00b72650` (`BSP_NativeParticleGeometry_SetBounds`)
- `0070bf8b CALL 00b74640` (`BSP_GeneratedModel_GetGeometry`)
- `0070bf92 CALL 00b732c0` (`BSP_MeshObject_GetElement`)
- `0070bf99 CALL 00b855b0` (`BSP_MeshElement_SetBounds`)
- `0070bfa5 CALL 00b74390` (`BSP_GeneratedModel_SetLocalBounds`)

### `0070bfc0` (`BSP_FlagModel_Construct` -> `bsp::flak_note_seg36_37_argon::FlagModel_Construct_0070bfc0`)
- `0070bfe2 CALL 00b75030` (`BSP_GeneratedModel_Construct`)
- `0070bffb CALL 0070bba0` (`BSP_FlagParams_InitDefaults`)
- `0070c00c CALL 0070bee0` (`BSP_FlagModel_Setup`)

### `0070c030` (`BSP_FlakBulletClass_ReadLuaFields` -> `bsp::flak_note_seg36_37_argon::FlakBulletClass_ReadLuaFields_0070c030`)
- `0070c051 CALL 006e8770` (`BSP_BulletClass_ReadLuaFields`)
- `0070c06c CALL 00b67800` (`BSP_LuaObject_GetByName`)
- `0070c07b CALL 00b66290` (`BSP_LuaObject_GetInteger`)
- `0070c095 CALL 00b67700` (`BSP_LuaObject_Destruct`)

### `0070c0b0` (`BSP_FlakBulletClass_DeriveEngagementRange` -> `bsp::flak_note_seg36_37_argon::FlakBulletClass_DeriveEngagementRange_0070c0b0`)
- `0070c0b3 CALL 006e9890` (`BSP_ProjectileClass_DeriveEngagementRange`)

### `0070c0f0` (`BSP_FlakBullet_WriteState` -> `bsp::flak_note_seg36_37_argon::FlakBullet_WriteState_0070c0f0`)
- `0070c0f9 CALL 006e71b0` (`BSP_Bullet_WriteState`)

### `0070c1f0` (`BSP_Entity_AsKind05` -> `bsp::flak_note_seg36_37_argon::Entity_AsKind05_0070c1f0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `0070c370` (`BSP_FlakProjectile_TickAdvance` -> `bsp::flak_note_seg36_37_argon::FlakProjectile_TickAdvance_0070c370`)
- `0070c387 CALL 006e6490` (`BSP_Projectile_TickAdvance`)
- `0070c3c8 CALL 0070c210` (`BSP_FlakProjectile_Detonate`)
- `0070c417 CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070c45f CALL 0042b2f0` (`BSP_Vector3_LengthFloatThreshold`)
- `0070c49b CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070c52a CALL 008053c0` (`BSP_Recon_EnsureSlot`)
- `0070c59d CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070c677 CALL 00427eb0` (`BSP_EntityPose_GetWorldPositionRefreshed`)
- `0070c680 CALL 00901c20` (`BSP_GunBot_InterceptSolution`)
- `0070c79b CALL 0070c210` (`BSP_FlakProjectile_Detonate`)
- `0070c7f7 CALL 00bd2f10` (`BSP_Random_UniformFloatRange`)
- `0070c849 CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070c89b CALL 0084c430` (`BSP_Projectile_SweepStepSegment`)
- `0070c8c6 CALL 00b6db70` (`BSP_Transform_RefreshWorldMatrix`)
- `0070c900 CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070c98b CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070c99d CALL 004842c0` (`BSP_PointEffect_SetWorldPoint`)
- `0070ca6f CALL 0085dc80` (`BSP_Matrix_OrthonormalizeBasisRows`)
- `0070ca7f CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070cac2 CALL 0053d9c0` (`BSP_PointEffect_SetWorldMatrix`)

### `0070cae0` (`BSP_FlakProjectile_Construct` -> `bsp::flak_note_seg36_37_argon::FlakProjectile_Construct_0070cae0`)
- `0070cae3 CALL 006e7b00` (`BSP_ProjectileTickableEntity_Construct`)

### `0070cb80` (`BSP_MFlakBullet_IsKindOf` -> `bsp::flak_note_seg36_37_argon::MFlakBullet_IsKindOf_0070cb80`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `0070cc30` (`BSP_FlakBulletClass_CreateProjectile` -> `bsp::flak_note_seg36_37_argon::FlakBulletClass_CreateProjectile_0070cc30`)
- `0070cc57 CALL 00bf55be` (`FUN_00bf55be`)
- `0070cc67 CALL 00bf79f0` (`_memset`)
- `0070cc7d CALL 0070cae0` (`BSP_FlakProjectile_Construct`)
- `0070cd16 CALL 00923870` (`FUN_00923870`)
- `0070ce16 CALL 0085dc80` (`BSP_Matrix_OrthonormalizeBasisRows`)
- `0070ce32 CALL 0042ed50` (`BSP_SceneNode_InvalidateSubtreePose`)

### `0070cea0` (`BSP_FlakBullet_Activate` -> `bsp::flak_note_seg36_37_argon::FlakBullet_Activate_0070cea0`)
- `0070cebb CALL 006e7e80` (`BSP_Bullet_Activate`)
- `0070cee4 CALL 009238a0` (`SEntity_GetSavedEntityLuaData`)
- `0070ceed CALL 004425c0` (`BSP_LuaReader_Construct`)
- `0070cf14 CALL 00bd8e20` (`BSP_LuaReader_EnterKey`)
- `0070cf4b CALL 00bd6830` (`BSP_LuaReader_ReadField`)
- `0070cf82 CALL 00bd6830` (`BSP_LuaReader_ReadField`)
- `0070cfb9 CALL 00bd6830` (`BSP_LuaReader_ReadField`)
- `0070cff0 CALL 00bd6830` (`BSP_LuaReader_ReadField`)
- `0070cff9 CALL 00bd7a20` (`BSP_LuaReader_LeaveKeyThunk`)
- `0070d00a CALL 00441a20` (`BSP_LuaReader_DestroyStackInstance`)

### `0070d1b0` (`BSP_Unit_ClassMaxSpeed` -> `bsp::flak_note_seg36_37_argon::Unit_ClassMaxSpeed_0070d1b0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `0070d7b0` (`BSP_UnitGroup_InitWithLeader` -> `bsp::flak_note_seg36_37_argon::UnitGroup_InitWithLeader_0070d7b0`)
- `0070d834 CALL 00694a60` (`BSP_Observer_RegisterPair`)

### `0070da00` (`BSP_UnitGroup_RefreshSpeedCeiling` -> `bsp::flak_note_seg36_37_argon::UnitGroup_RefreshSpeedCeiling_0070da00`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `0070db20` (`BSP_UnitGroup_CreateForLeader` -> `bsp::flak_note_seg36_37_argon::UnitGroup_CreateForLeader_0070db20`)
- `0070db29 CALL 00bf681b` (`operator_new`)
- `0070db37 CALL 0070dab0` (`BSP_UnitGroup_Construct`)
- `0070db41 CALL 0070d7b0` (`BSP_UnitGroup_InitWithLeader`)
- `0070db50 CALL 0070d7b0` (`BSP_UnitGroup_InitWithLeader`)

### `0070db60` (`BSP_UnitGroup_SwapSlotsByDistance` -> `bsp::flak_note_seg36_37_argon::UnitGroup_SwapSlotsByDistance_0070db60`)
- `0070dc29 CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070dc46 CALL 00413920` (`BSP_Matrix_Multiply4x4`)
- `0070dd01 CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070dd1e CALL 00413920` (`BSP_Matrix_Multiply4x4`)
- `0070dddc CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070ddf9 CALL 00413920` (`BSP_Matrix_Multiply4x4`)
- `0070deb4 CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070ded1 CALL 00413920` (`BSP_Matrix_Multiply4x4`)
- `0070df8f CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070dfac CALL 00413920` (`BSP_Matrix_Multiply4x4`)
- `0070e066 CALL 00811180` (`BSP_Unit_DecomposeAgainstWake`)
- `0070e07f CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `0070e09c CALL 00413920` (`BSP_Matrix_Multiply4x4`)
- `0070e156 CALL 00811180` (`BSP_Unit_DecomposeAgainstWake`)
- `0070e1d1 CALL 00bf7030` (`LIBCRT_sqrt`)
- `0070e22b CALL 00bf7030` (`LIBCRT_sqrt`)
- `0070e285 CALL 00bf7030` (`LIBCRT_sqrt`)
- `0070e2df CALL 00bf7030` (`LIBCRT_sqrt`)

### `0070e3a0` (`BSP_Entity_AsKind06` -> `bsp::flak_note_seg36_37_argon::Entity_AsKind06_0070e3a0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `0070e3c0` (`BSP_UnitGroup_GetMaxFollowerTurnRadius` -> `bsp::flak_note_seg36_37_argon::UnitGroup_GetMaxFollowerTurnRadius_0070e3c0`)
- `0070e405 CALL 0082e850` (`BSP_ShipClass_GetTurnRadius`)

### `0070e450` (`BSP_ShipGroup_ControllerAreaKey` -> `bsp::flak_note_seg36_37_argon::ShipGroup_ControllerAreaKey_0070e450`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `0070e4c0` (`BSP_UnitGroup_DetachMember` -> `bsp::flak_note_seg36_37_argon::UnitGroup_DetachMember_0070e4c0`)
- `0070e4dd CALL 0070d8d0` (`BSP_UnitGroup_SelectSuccessor`)
- `0070e54e CALL 00815e20` (`BSP_UnitWake_HandoffPoseAndTrail`)
- `0070e564 CALL 006952a0` (`BSP_Observer_UnregisterPair`)
- `0070e5ff CALL 0070da00` (`BSP_UnitGroup_RefreshSpeedCeiling`)

### `0070e620` (`BSP_UnitGroup_ReadSceneFields` -> `bsp::flak_note_seg36_37_argon::UnitGroup_ReadSceneFields_0070e620`)
- `0070e77d CALL 00694a60` (`BSP_Observer_RegisterPair`)

### `0070e890` (`BSP_ObserverMember_WriteRef_0070e890` -> `bsp::flak_note_seg36_37_argon::ObserverMember_WriteRef_0070e890`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `0070eb00` (`BSP_ObserverMember_TargetWithinRange` -> `bsp::flak_note_seg36_37_argon::ObserverMember_TargetWithinRange_0070eb00`)
- `0070eb79 CALL 00811150` (`BSP_Unit_WakePointAtDistance`)
- `0070eba8 CALL 0042b260` (`BSP_Geometry_NormalizeVectorWithFloor`)
- `0070ec0a CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)

### `0070eca0` (`BSP_UnitGroup_RemoveEntity` -> `bsp::flak_note_seg36_37_argon::UnitGroup_RemoveEntity_0070eca0`)
- `0070ecdd CALL 0070e4c0` (`BSP_UnitGroup_DetachMember`)

### `0070ecf0` (`BSP_UnitGroup_CreateFromScene` -> `bsp::flak_note_seg36_37_argon::UnitGroup_CreateFromScene_0070ecf0`)
- `0070ecf9 CALL 00bf681b` (`operator_new`)
- `0070ed07 CALL 0070dab0` (`BSP_UnitGroup_Construct`)
- `0070ed11 CALL 0070e620` (`BSP_UnitGroup_ReadSceneFields`)
- `0070ed20 CALL 0070e620` (`BSP_UnitGroup_ReadSceneFields`)

### `0070f090` (`BSP_UnitPartNode_SphereOverlapForward` -> `bsp::flak_note_seg36_37_argon::UnitPartNode_SphereOverlapForward_0070f090`)
- `0070f0a7 CALL 0098aae0` (`BSP_CollisionNode_SphereOverlapDefault`)

### `0070f3d0` (`STL_PartGroupRows_Allocate_0070f3d0` -> `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_Allocate_0070f3d0`)
- `0070f3dd CALL 00bf681b` (`operator_new`)
- `0070f405 CALL 00bf6340` (`exception`)
- `0070f41c CALL 00bf6885` (`FUN_00bf6885`)

### `0070f6b0` (`BSP_UnitPartCollisionShape_ConstructWithMesh` -> `bsp::flak_note_seg36_37_argon::UnitPartCollisionShape_ConstructWithMesh_0070f6b0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `0070f710` (`BSP_CollisionShapeD768_SegmentSlot` -> `bsp::flak_note_seg36_37_argon::CollisionShapeD768_SegmentSlot_0070f710`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `0070f720` (`BSP_UnitPartCollisionShape_TestSphere` -> `bsp::flak_note_seg36_37_argon::UnitPartCollisionShape_TestSphere_0070f720`)
- `0070f72f CALL 00b6db70` (`BSP_Transform_RefreshWorldMatrix`)
- `0070f749 CALL 00b6e0d0` (`BSP_Node_GetInverseWorldMatrix`)
- `0070f759 CALL 00723f80` (`BSP_GeomMesh_CollectSphereHits`)

### `0070f900` (`STL_LegacySboString_FindFirstOf_0070f900` -> `bsp::flak_note_seg36_37_argon::STL_LegacySboString_FindFirstOf_0070f900`)
- `0070f950 CALL 00bfa700` (`FUN_00bfa700`)

### `0070fdb0` (`BSP_LogicalVertexStream_ReadNormal` -> `bsp::flak_note_seg36_37_argon::LogicalVertexStream_ReadNormal_0070fdb0`)
- `0070fec0 CALL 00475f80` (`BSP_Vertex_UnpackNineBitTriple`)
- `0070ffd7 CALL 00a4c2ae` (`FUN_00a4c2ae`)

### `007100a0` (`BSP_LogicalVertexStream_ReadUV` -> `bsp::flak_note_seg36_37_argon::LogicalVertexStream_ReadUV_007100a0`)
- `00710202 CALL 00a4c2ae` (`FUN_00a4c2ae`)

### `00710360` (`BSP_UnitPartEffectPointers_PushBack_00710360` -> `bsp::flak_note_seg36_37_argon::UnitPartEffectPointers_PushBack_00710360`)
- `00710378 CALL 007102a0` (`BSP_UnitPartEffectPointers_Reserve`)

### `007103e0` (`BSP_UnitPartEntryPointers_PushBack_007103e0` -> `bsp::flak_note_seg36_37_argon::UnitPartEntryPointers_PushBack_007103e0`)
- `007103f8 CALL 0070faa0` (`BSP_UnitPartEntryPointers_Reserve`)

### `007104e0` (`STL_List_Clear_007104e0` -> `bsp::flak_note_seg36_37_argon::STL_List_Clear_007104e0`)
- `00710503 CALL 00bf65ac` (`_free`)

### `00710530` (`STL_List_Clear_00710530` -> `bsp::flak_note_seg36_37_argon::STL_List_Clear_00710530`)
- `00710553 CALL 00bf65ac` (`_free`)

### `00710630` (`BSP_RefPtr_Release_00710630` -> `bsp::flak_note_seg36_37_argon::RefPtr_Release_00710630`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `007106b0` (`BSP_RefPtr_Release_007106b0` -> `bsp::flak_note_seg36_37_argon::RefPtr_Release_007106b0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00710700` (`STL_MoveDwords_00710700` -> `bsp::flak_note_seg36_37_argon::STL_MoveDwords_00710700`)
- `00710722 CALL 00bf67a7` (`FUN_00bf67a7`)

### `007107e0` (`BSP_UnitPartEffectPointers_PushBack_007107e0` -> `bsp::flak_note_seg36_37_argon::UnitPartEffectPointers_PushBack_007107e0`)
- `007107f8 CALL 007102a0` (`BSP_UnitPartEffectPointers_Reserve`)

### `00710820` (`BSP_UnitPartEntryPointers_PushBack_00710820` -> `bsp::flak_note_seg36_37_argon::UnitPartEntryPointers_PushBack_00710820`)
- `00710838 CALL 0070faa0` (`BSP_UnitPartEntryPointers_Reserve`)

### `007109a0` (`BSP_RefPtr_Assign_007109a0` -> `bsp::flak_note_seg36_37_argon::RefPtr_Assign_007109a0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `007109e0` (`STL_FindDword_007109e0` -> `bsp::flak_note_seg36_37_argon::STL_FindDword_007109e0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00710a60` (`BSP_RefPtr_Release_00710a60` -> `bsp::flak_note_seg36_37_argon::RefPtr_Release_00710a60`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00710e10` (`BSP_UnitPartEffect_AttachAtPart` -> `bsp::flak_note_seg36_37_argon::UnitPartEffect_AttachAtPart_00710e10`)
- `00710ed5 CALL 008687c0` (`BSP_PointEffect_CreateWithParentMatrix`)
- `00710f16 CALL 007102a0` (`BSP_UnitPartEffectPointers_Reserve`)

### `00710f70` (`STL_List_Construct_UnitParts_00710f70` -> `bsp::flak_note_seg36_37_argon::STL_List_Construct_UnitParts_00710f70`)
- `00710f73 CALL 007103a0` (`STL_UnitPartList_BuySentinel_007103a0`)

### `00710fa0` (`STL_List_Construct_UnitPartShapes_00710fa0` -> `bsp::flak_note_seg36_37_argon::STL_List_Construct_UnitPartShapes_00710fa0`)
- `00710fa3 CALL 007103c0` (`STL_UnitPartShapeList_BuySentinel_007103c0`)

### `00711210` (`BSP_UnitPartEffect_StartByIndex` -> `bsp::flak_note_seg36_37_argon::UnitPartEffect_StartByIndex_00711210`)
- `0071124b CALL 00871ba0` (`BSP_EffectHandle_AcquireByName`)
- `0071129f CALL 00710e10` (`BSP_UnitPartEffect_AttachAtPart`)

### `007114a0` (`STL_Vector_Tidy_007114a0` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_007114a0`)
- `007114ab CALL 00bf65ac` (`_free`)

### `00712100` (`STL_Vector_Assign_00712100` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Assign_00712100`)
- `00712125 CALL 004fdca0` (`FUN_004fdca0`)
- `0071214d CALL 004fc490` (`FUN_004fc490`)
- `007121b6 CALL 004fc490` (`FUN_004fc490`)
- `007121c9 CALL 00710700` (`STL_MoveDwords_00710700`)
- `007121df CALL 00bf65ac` (`_free`)
- `007121fd CALL 00505750` (`TMPL_DwordVector_ConstructN_00505750`)
- `00712214 CALL 00710700` (`STL_MoveDwords_00710700`)

### `00712270` (`STL_ConstructDwordVector_00712270` -> `bsp::flak_note_seg36_37_argon::STL_ConstructDwordVector_00712270`)
- `0071229c CALL 00712040` (`STL_CheckedDwordVector_CopyConstruct_00712040`)

### `00712330` (`STL_PartGroupRows_DestroyRange_Thunk_00712330` -> `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_DestroyRange_Thunk_00712330`)
- `0071233e CALL 00711f70` (`STL_PartGroupRows_DestroyRange_00711f70`)

### `007123c0` (`STL_ConstructDwordVector_007123c0` -> `bsp::flak_note_seg36_37_argon::STL_ConstructDwordVector_007123c0`)
- `007123f2 CALL 00712040` (`STL_CheckedDwordVector_CopyConstruct_00712040`)

### `00712900` (`STL_PartGroupRows_AssignForward_00712900` -> `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_AssignForward_00712900`)
- `00712926 CALL 00712100` (`STL_Vector_Assign_00712100`)

### `00712940` (`STL_PartGroupRows_AssignRepeated_00712940` -> `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_AssignRepeated_00712940`)
- `00712956 CALL 00712100` (`STL_Vector_Assign_00712100`)

### `00712970` (`STL_PartGroupRows_UninitializedFill_00712970` -> `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_UninitializedFill_00712970`)
- `007129b8 CALL 00712040` (`STL_CheckedDwordVector_CopyConstruct_00712040`)

### `00712a10` (`STL_PartGroupRows_TransferConstruct_00712a10` -> `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_TransferConstruct_00712a10`)
- `00712a68 CALL 00712040` (`STL_CheckedDwordVector_CopyConstruct_00712040`)
- `00712ad0 CALL 00bf65ac` (`_free`)

### `00712af0` (`STL_SwapBackward_Vector16_00712af0` -> `bsp::flak_note_seg36_37_argon::STL_SwapBackward_Vector16_00712af0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00712f20` (`STL_PartGroupRows_UninitializedFillAdapter_00712f20` -> `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_UninitializedFillAdapter_00712f20`)
- `00712f44 CALL 00712970` (`STL_PartGroupRows_UninitializedFill_00712970`)

### `00712f60` (`STL_PartGroupRows_TransferAdapter_00712f60` -> `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_TransferAdapter_00712f60`)
- `00712f7c CALL 00712a10` (`STL_PartGroupRows_TransferConstruct_00712a10`)

### `00712f90` (`STL_PartGroupRows_BackwardTransferAdapter_00712f90` -> `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_BackwardTransferAdapter_00712f90`)
- `00712fb7 CALL 00712af0` (`STL_SwapBackward_Vector16_00712af0`)

### `00712ff0` (`STL_PartGroupRows_InsertCount_00712ff0` -> `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_InsertCount_00712ff0`)
- `0071301d CALL 00712040` (`STL_CheckedDwordVector_CopyConstruct_00712040`)
- `0071305c CALL 00711e60` (`STL_xlen_throw_00711e60`)
- `007130bd CALL 0070f3d0` (`STL_PartGroupRows_Allocate_0070f3d0`)
- `007130e0 CALL 00712a10` (`STL_PartGroupRows_TransferConstruct_00712a10`)
- `007130f0 CALL 00712f20` (`STL_PartGroupRows_UninitializedFillAdapter_00712f20`)
- `0071310c CALL 00712a10` (`STL_PartGroupRows_TransferConstruct_00712a10`)
- `00713132 CALL 00711f70` (`STL_PartGroupRows_DestroyRange_00711f70`)
- `0071313b CALL 00bf65ac` (`_free`)
- `007131a3 CALL 00712f60` (`STL_PartGroupRows_TransferAdapter_00712f60`)
- `007131c0 CALL 00712f20` (`STL_PartGroupRows_UninitializedFillAdapter_00712f20`)
- `007131dd CALL 00712940` (`STL_PartGroupRows_AssignRepeated_00712940`)
- `0071321b CALL 00712f60` (`STL_PartGroupRows_TransferAdapter_00712f60`)
- `00713229 CALL 00712f90` (`STL_PartGroupRows_BackwardTransferAdapter_00712f90`)
- `00713239 CALL 00712940` (`STL_PartGroupRows_AssignRepeated_00712940`)
- `00713249 CALL 00bf65ac` (`_free`)

### `00713270` (`STL_PartGroupRows_Resize_00713270` -> `bsp::flak_note_seg36_37_argon::STL_PartGroupRows_Resize_00713270`)
- `007132c2 CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `007132d3 CALL 00712ff0` (`STL_PartGroupRows_InsertCount_00712ff0`)
- `007132f0 CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `007132fd CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `00713315 CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `00713325 CALL 00712b80` (`STL_PartGroupRows_EraseChecked_00712b80`)
- `00713337 CALL 00bf65ac` (`_free`)

### `00713750` (`BSP_ResourceItemNote_GetRuntimeTypeToken` -> `bsp::flak_note_seg36_37_argon::ResourceItemNote_GetRuntimeTypeToken_00713750`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00713760` (`BSP_ResourceItemNote_GetRuntimeTypeNameAddress` -> `bsp::flak_note_seg36_37_argon::ResourceItemNote_GetRuntimeTypeNameAddress_00713760`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00713790` (`BSP_ResourceItemAux_GetRuntimeTypeToken` -> `bsp::flak_note_seg36_37_argon::ResourceItemAux_GetRuntimeTypeToken_00713790`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `007137a0` (`BSP_ResourceItemAux_GetRuntimeTypeNameAddress` -> `bsp::flak_note_seg36_37_argon::ResourceItemAux_GetRuntimeTypeNameAddress_007137a0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `007137d0` (`BSP_ResourceItemZoneDesc_GetRuntimeTypeToken` -> `bsp::flak_note_seg36_37_argon::ResourceItemZoneDesc_GetRuntimeTypeToken_007137d0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `007137e0` (`BSP_ResourceItemZoneDesc_GetRuntimeTypeNameAddress` -> `bsp::flak_note_seg36_37_argon::ResourceItemZoneDesc_GetRuntimeTypeNameAddress_007137e0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00714030` (`BSP_GameResource_AsType44_00714030` -> `bsp::flak_note_seg36_37_argon::GameResource_AsType44_00714030`)
- `0071403a CALL 00721c90` (`BSP_GameResource_Type44Token`)

### `00714060` (`BSP_GameResource_AsType54_00714060` -> `bsp::flak_note_seg36_37_argon::GameResource_AsType54_00714060`)
- `0071406a CALL 006f9a80` (`BSP_GameResource_Type54Token`)

### `00714090` (`BSP_ResourceItemZoneDesc_Cast` -> `bsp::flak_note_seg36_37_argon::ResourceItemZoneDesc_Cast_00714090`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `007140c0` (`BSP_ResourceItemAux_Cast` -> `bsp::flak_note_seg36_37_argon::ResourceItemAux_Cast_007140c0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `007140f0` (`BSP_ResourceItemNote_Cast` -> `bsp::flak_note_seg36_37_argon::ResourceItemNote_Cast_007140f0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `007149b0` (`BSP_Matrix4f_InvertInto_007149b0` -> `bsp::flak_note_seg36_37_argon::Matrix4f_InvertInto_007149b0`)
- `007149ba CALL 00b632d0` (`BSP_Matrix4f_InvertGeneral`)
- `007149c2 CALL 004134f0` (`BSP_Matrix_Copy4x4X87`)

### `00715af0` (`STL_UninitCopy8_00715af0` -> `bsp::flak_note_seg36_37_argon::STL_UninitCopy8_00715af0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00715b20` (`STL_UninitCopy8_00715b20` -> `bsp::flak_note_seg36_37_argon::STL_UninitCopy8_00715b20`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00715b50` (`STL_UninitCopy8_00715b50` -> `bsp::flak_note_seg36_37_argon::STL_UninitCopy8_00715b50`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00715b80` (`STL_UninitCopy8_00715b80` -> `bsp::flak_note_seg36_37_argon::STL_UninitCopy8_00715b80`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00715bb0` (`STL_UninitCopy8_00715bb0` -> `bsp::flak_note_seg36_37_argon::STL_UninitCopy8_00715bb0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00716960` (`STL_Fill8_00716960` -> `bsp::flak_note_seg36_37_argon::STL_Fill8_00716960`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00716990` (`STL_Fill8_00716990` -> `bsp::flak_note_seg36_37_argon::STL_Fill8_00716990`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `007169c0` (`STL_Fill8_007169c0` -> `bsp::flak_note_seg36_37_argon::STL_Fill8_007169c0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `007169f0` (`STL_Fill8_007169f0` -> `bsp::flak_note_seg36_37_argon::STL_Fill8_007169f0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00716a20` (`STL_Fill8_00716a20` -> `bsp::flak_note_seg36_37_argon::STL_Fill8_00716a20`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00716b50` (`STL_UninitFillN8_00716b50` -> `bsp::flak_note_seg36_37_argon::STL_UninitFillN8_00716b50`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00716b80` (`STL_UninitFillN8_00716b80` -> `bsp::flak_note_seg36_37_argon::STL_UninitFillN8_00716b80`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00716bb0` (`STL_UninitFillN8_00716bb0` -> `bsp::flak_note_seg36_37_argon::STL_UninitFillN8_00716bb0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00716be0` (`STL_UninitFillN8_00716be0` -> `bsp::flak_note_seg36_37_argon::STL_UninitFillN8_00716be0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00716c10` (`STL_UninitFillN8_00716c10` -> `bsp::flak_note_seg36_37_argon::STL_UninitFillN8_00716c10`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00717510` (`STL_Vector_Tidy_00717510` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00717510`)
- `0071751b CALL 00bf65ac` (`_free`)

### `00717570` (`STL_Vector_Tidy_00717570` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00717570`)
- `0071757b CALL 00bf65ac` (`_free`)

### `007175a0` (`STL_Vector_Tidy_007175a0` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_007175a0`)
- `007175ab CALL 00bf65ac` (`_free`)

### `00717880` (`STL_CopyBackward8_00717880` -> `bsp::flak_note_seg36_37_argon::STL_CopyBackward8_00717880`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `007178c0` (`STL_CopyBackward8_007178c0` -> `bsp::flak_note_seg36_37_argon::STL_CopyBackward8_007178c0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00717900` (`STL_CopyBackward8_00717900` -> `bsp::flak_note_seg36_37_argon::STL_CopyBackward8_00717900`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00717940` (`STL_CopyBackward8_00717940` -> `bsp::flak_note_seg36_37_argon::STL_CopyBackward8_00717940`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00717980` (`STL_CopyBackward8_00717980` -> `bsp::flak_note_seg36_37_argon::STL_CopyBackward8_00717980`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00717c70` (`BSP_ResourceItemNote_Construct` -> `bsp::flak_note_seg36_37_argon::ResourceItemNote_Construct_00717c70`)
- `00717c73 CALL 00b868b0` (`BSP_ResourceItem_ConstructReferenceBase`)

### `00717ca0` (`BSP_ResourceItemNote_AcceptsRuntimeTypeToken` -> `bsp::flak_note_seg36_37_argon::ResourceItemNote_AcceptsRuntimeTypeToken_00717ca0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00717cd0` (`BSP_ResourceItemNote_Destruct` -> `bsp::flak_note_seg36_37_argon::ResourceItemNote_Destruct_00717cd0`)
- `00717cdd CALL 00bf65ac` (`_free`)
- `00717cf7 JMP 00b86890` (`BSP_ResourceItem_DestroyReferenceBase`)

### `00717d50` (`STL_String_Tidy_00717d50` -> `bsp::flak_note_seg36_37_argon::STL_String_Tidy_00717d50`)
- `00717d5e CALL 00bf65ac` (`_free`)
- `00717d7f CALL 00bf65ac` (`_free`)

### `00717da0` (`BSP_ResourceItemZoneDesc_Construct` -> `bsp::flak_note_seg36_37_argon::ResourceItemZoneDesc_Construct_00717da0`)
- `00717da3 CALL 00b868b0` (`BSP_ResourceItem_ConstructReferenceBase`)

### `00717dd0` (`BSP_ResourceItemZoneDesc_AcceptsRuntimeTypeToken` -> `bsp::flak_note_seg36_37_argon::ResourceItemZoneDesc_AcceptsRuntimeTypeToken_00717dd0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00717e00` (`BSP_ResourceItemZoneDesc_Destruct` -> `bsp::flak_note_seg36_37_argon::ResourceItemZoneDesc_Destruct_00717e00`)
- `00717e0d CALL 00bf65ac` (`_free`)
- `00717e27 JMP 00b86890` (`BSP_ResourceItem_DestroyReferenceBase`)

### `00717ef0` (`STL_String_Tidy_00717ef0` -> `bsp::flak_note_seg36_37_argon::STL_String_Tidy_00717ef0`)
- `00717efd CALL 00bf65ac` (`_free`)

### `007180e0` (`STL_Vector_Tidy_007180e0` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_007180e0`)
- `007180eb CALL 00bf65ac` (`_free`)

### `00718110` (`STL_Vector_Tidy_00718110` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718110`)
- `0071811b CALL 00bf65ac` (`_free`)

### `00718140` (`STL_Vector_Tidy_00718140` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718140`)
- `0071814b CALL 00bf65ac` (`_free`)

### `007185f0` (`STL_Vector_Tidy_007185f0` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_007185f0`)
- `007185fb CALL 00bf65ac` (`_free`)

### `00718620` (`STL_Vector_Tidy_00718620` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718620`)
- `0071862b CALL 00bf65ac` (`_free`)

### `00718650` (`STL_Vector_Tidy_00718650` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718650`)
- `0071865b CALL 00bf65ac` (`_free`)

### `00718680` (`STL_Vector_Tidy_00718680` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718680`)
- `0071868b CALL 00bf65ac` (`_free`)

### `007186b0` (`STL_Vector_Tidy_007186b0` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_007186b0`)
- `007186bb CALL 00bf65ac` (`_free`)

### `00718710` (`CG_scalar_deleting_dtor_00718710` -> `bsp::flak_note_seg36_37_argon::ResourceItemNote_ScalarDelete_00718710`)
- `0071871d CALL 00bf65ac` (`_free`)
- `00718736 CALL 00b86890` (`BSP_ResourceItem_DestroyReferenceBase`)
- `00718743 CALL 00bf65ac` (`_free`)

### `00718a40` (`STL_Vector_Tidy_00718a40` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718a40`)
- `00718a4b CALL 00bf65ac` (`_free`)

### `00718a70` (`STL_Vector_Tidy_00718a70` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718a70`)
- `00718a7b CALL 00bf65ac` (`_free`)

### `00718aa0` (`STL_Vector_Tidy_00718aa0` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718aa0`)
- `00718aab CALL 00bf65ac` (`_free`)

### `00718ad0` (`STL_Vector_Tidy_00718ad0` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718ad0`)
- `00718adb CALL 00bf65ac` (`_free`)

### `00718b00` (`STL_Vector_Tidy_00718b00` -> `bsp::flak_note_seg36_37_argon::STL_Vector_Tidy_00718b00`)
- `00718b0b CALL 00bf65ac` (`_free`)

### `00718c70` (`BSP_ModelNoteNameEquals_Predicate` -> `bsp::flak_note_seg36_37_argon::ModelNoteNameEquals_Predicate_00718c70`)
- `00718c97 CALL 00711c30` (`BSP_PartNamedRecord_CopyLegacyName`)
- `00718cbf CALL 004beb60` (`STL_LegacySboString_CompareSubstring_004beb60`)
- `00718cd5 CALL 00bf65ac` (`_free`)

### `00718f50` (`BSP_StructuredNode_ReadCharacterString` -> `bsp::flak_note_seg36_37_argon::StructuredNode_ReadCharacterString_00718f50`)
- `00718f73 CALL 00bea010` (`BSP_StructuredNodeHandle_ReadString`)
- `00718fa3 CALL 00408720` (`BSP_CharacterString_AssignCounted`)
- `00718fc4 CALL 00419cc0` (`BSP_SizedStoragePool_GetSingleton`)
- `00718fcb CALL 00bd1510` (`BSP_SizedStoragePool_ReturnBlock`)

### `00719000` (`BSP_NoteResourceParser_Parse` -> `bsp::flak_note_seg36_37_argon::NoteResourceParser_Parse_00719000`)
- `00719019 CALL 00bf681b` (`operator_new`)
- `00719035 CALL 00b868b0` (`BSP_ResourceItem_ConstructReferenceBase`)
- `00719065 CALL 00718f50` (`BSP_StructuredNode_ReadCharacterString`)

### `00719080` (`BSP_ZoneDescResourceParser_ReadItem` -> `bsp::flak_note_seg36_37_argon::ZoneDescResourceParser_ReadItem_00719080`)
- `007190a0 CALL 00715bf0` (`BSP_StructuredNode_HasRemaining`)
- `007190b7 CALL 00bea680` (`BSP_StructuredNode_CreateChild`)
- `007190d5 CALL 00bf7fbf` (`FUN_00bf7fbf`)
- `007190ef CALL 00718f50` (`BSP_StructuredNode_ReadCharacterString`)
- `00719115 CALL 00438e10` (`BSP_CString_CompareInsensitive`)
- `00719152 CALL 00bf7fbf` (`FUN_00bf7fbf`)
- `0071916b CALL 00be9a80` (`BSP_StructuredNode_ReadU32ToOutParam`)
- `00719186 CALL 00bf7fbf` (`FUN_00bf7fbf`)
- `0071919f CALL 00be9a80` (`BSP_StructuredNode_ReadU32ToOutParam`)
- `007191b7 CALL 00bf7fbf` (`FUN_00bf7fbf`)
- `007191d8 CALL 00be9a60` (`BSP_StructuredNodeHandle_ReadFloat`)
- `007191df CALL 00be9a60` (`BSP_StructuredNodeHandle_ReadFloat`)
- `007191e6 CALL 00be9a60` (`BSP_StructuredNodeHandle_ReadFloat`)
- `007191f3 CALL 00be9a60` (`BSP_StructuredNodeHandle_ReadFloat`)
- `007191fe CALL 00be9c40` (`BSP_StructuredNode_SkipAndDetach`)
- `0071920f CALL 00be9ed0` (`BSP_StructuredNodeHandle_Release`)
- `00719216 CALL 00715bf0` (`BSP_StructuredNode_HasRemaining`)

### `00719240` (`BSP_ZoneDescResourceParser_CreateItem` -> `bsp::flak_note_seg36_37_argon::ZoneDescResourceParser_CreateItem_00719240`)
- `00719259 CALL 00bf681b` (`operator_new`)
- `00719275 CALL 00b868b0` (`BSP_ResourceItem_ConstructReferenceBase`)
- `007192a5 CALL 00719080` (`BSP_ZoneDescResourceParser_ReadItem`)

### `007192c0` (`STL_Vector_InsertN8_007192c0` -> `bsp::flak_note_seg36_37_argon::STL_Vector_InsertN8_007192c0`)
- `0071932a CALL 00718d20` (`FUN_00718d20`)
- `0071938b CALL 00714cb0` (`FUN_00714cb0`)
- `007193b0 CALL 00715af0` (`STL_UninitCopy8_00715af0`)
- `007193c0 CALL 00718180` (`FUN_00718180`)
- `007193d8 CALL 00715af0` (`STL_UninitCopy8_00715af0`)
- `007193fa CALL 00bf65ac` (`_free`)
- `0071945e CALL 00718950` (`FUN_00718950`)
- `00719481 CALL 00718180` (`FUN_00718180`)
- `00719497 CALL 00716960` (`STL_Fill8_00716960`)
- `007194c8 CALL 00718950` (`FUN_00718950`)
- `007194d6 CALL 00717880` (`STL_CopyBackward8_00717880`)
- `007194e6 CALL 00716960` (`STL_Fill8_00716960`)

### `00719510` (`STL_Vector_InsertN8_00719510` -> `bsp::flak_note_seg36_37_argon::STL_Vector_InsertN8_00719510`)
- `0071957a CALL 00718d90` (`FUN_00718d90`)
- `007195db CALL 00714d10` (`FUN_00714d10`)
- `00719600 CALL 00715b20` (`STL_UninitCopy8_00715b20`)
- `00719610 CALL 007181c0` (`FUN_007181c0`)
- `00719628 CALL 00715b20` (`STL_UninitCopy8_00715b20`)
- `0071964a CALL 00bf65ac` (`_free`)
- `007196ae CALL 00718980` (`FUN_00718980`)
- `007196d1 CALL 007181c0` (`FUN_007181c0`)
- `007196e7 CALL 00716990` (`STL_Fill8_00716990`)
- `00719718 CALL 00718980` (`FUN_00718980`)
- `00719726 CALL 007178c0` (`STL_CopyBackward8_007178c0`)
- `00719736 CALL 00716990` (`STL_Fill8_00716990`)

### `00719760` (`STL_Vector_InsertN8_00719760` -> `bsp::flak_note_seg36_37_argon::STL_Vector_InsertN8_00719760`)
- `007197ca CALL 00718e00` (`FUN_00718e00`)
- `0071982b CALL 00714d70` (`FUN_00714d70`)
- `00719850 CALL 00715b50` (`STL_UninitCopy8_00715b50`)
- `00719860 CALL 00718200` (`FUN_00718200`)
- `00719878 CALL 00715b50` (`STL_UninitCopy8_00715b50`)
- `0071989a CALL 00bf65ac` (`_free`)
- `007198fe CALL 007189b0` (`FUN_007189b0`)
- `00719921 CALL 00718200` (`FUN_00718200`)
- `00719937 CALL 007169c0` (`STL_Fill8_007169c0`)
- `00719968 CALL 007189b0` (`FUN_007189b0`)
- `00719976 CALL 00717900` (`STL_CopyBackward8_00717900`)
- `00719986 CALL 007169c0` (`STL_Fill8_007169c0`)

### `007199b0` (`STL_Vector_InsertN8_007199b0` -> `bsp::flak_note_seg36_37_argon::STL_Vector_InsertN8_007199b0`)
- `00719a1a CALL 00718e70` (`FUN_00718e70`)
- `00719a7b CALL 00714dd0` (`FUN_00714dd0`)
- `00719aa0 CALL 00715b80` (`STL_UninitCopy8_00715b80`)
- `00719ab0 CALL 00718240` (`FUN_00718240`)
- `00719ac8 CALL 00715b80` (`STL_UninitCopy8_00715b80`)
- `00719aea CALL 00bf65ac` (`_free`)
- `00719b4e CALL 007189e0` (`FUN_007189e0`)
- `00719b71 CALL 00718240` (`FUN_00718240`)
- `00719b87 CALL 007169f0` (`STL_Fill8_007169f0`)
- `00719bb8 CALL 007189e0` (`FUN_007189e0`)
- `00719bc6 CALL 00717940` (`STL_CopyBackward8_00717940`)
- `00719bd6 CALL 007169f0` (`STL_Fill8_007169f0`)

### `00719c00` (`STL_Vector_InsertN8_00719c00` -> `bsp::flak_note_seg36_37_argon::STL_Vector_InsertN8_00719c00`)
- `00719c6a CALL 00718ee0` (`FUN_00718ee0`)
- `00719ccb CALL 00714e30` (`FUN_00714e30`)
- `00719cf0 CALL 00715bb0` (`STL_UninitCopy8_00715bb0`)
- `00719d00 CALL 00718280` (`FUN_00718280`)
- `00719d18 CALL 00715bb0` (`STL_UninitCopy8_00715bb0`)
- `00719d3a CALL 00bf65ac` (`_free`)
- `00719d9e CALL 00718a10` (`FUN_00718a10`)
- `00719dc1 CALL 00718280` (`FUN_00718280`)
- `00719dd7 CALL 00716a20` (`STL_Fill8_00716a20`)
- `00719e08 CALL 00718a10` (`FUN_00718a10`)
- `00719e16 CALL 00717980` (`STL_CopyBackward8_00717980`)
- `00719e26 CALL 00716a20` (`STL_Fill8_00716a20`)

### `00719fa0` (`BSP_ModelNoteList_FindIfNameEquals` -> `bsp::flak_note_seg36_37_argon::ModelNoteList_FindIfNameEquals_00719fa0`)
- `00719fd5 CALL 00718c70` (`BSP_ModelNoteNameEquals_Predicate`)
- `00719ff1 CALL 00bf65ac` (`_free`)

### `0071aae0` (`BSP_ModelNoteList_FindByName` -> `bsp::flak_note_seg36_37_argon::ModelNoteList_FindByName_0071aae0`)
- `0071ab1d CALL 00408120` (`BSP_LegacySboString_AssignSubstring`)
- `0071ab2a CALL 00719fa0` (`BSP_ModelNoteList_FindIfNameEquals`)
- `0071ab44 CALL 00bf65ac` (`_free`)

### `0071abb0` (`BSP_ResourceItemAux_Construct` -> `bsp::flak_note_seg36_37_argon::ResourceItemAux_Construct_0071abb0`)
- `0071abc9 CALL 00b868b0` (`BSP_ResourceItem_ConstructReferenceBase`)

### `0071ac10` (`BSP_ResourceItemAux_AcceptsRuntimeTypeToken` -> `bsp::flak_note_seg36_37_argon::ResourceItemAux_AcceptsRuntimeTypeToken_0071ac10`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `0071ac40` (`BSP_ResourceItemAux_Destruct` -> `bsp::flak_note_seg36_37_argon::ResourceItemAux_Destruct_0071ac40`)
- `0071ac4e CALL 00bf65ac` (`_free`)
- `0071ac69 CALL 00bf65ac` (`_free`)
- `0071ac88 CALL 00bf65ac` (`_free`)
- `0071aca1 JMP 00b86890` (`BSP_ResourceItem_DestroyReferenceBase`)

### `0071ad50` (`BSP_Model_FindNoteNode` -> `bsp::flak_note_seg36_37_argon::Model_FindNoteNode_0071ad50`)
- `0071ad8d CALL 00408720` (`BSP_CharacterString_AssignCounted`)
- `0071ada0 CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `0071adad CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `0071adba CALL 0071aae0` (`BSP_ModelNoteList_FindByName`)
- `0071adc7 CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `0071add8 CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `0071adf4 CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `0071adfe CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)

### `0071b3e0` (`BSP_GameResource_ReadPointsItem` -> `bsp::flak_note_seg36_37_argon::GameResource_ReadPointsItem_0071b3e0`)
- `0071b402 CALL 00715bf0` (`BSP_StructuredNode_HasRemaining`)
- `0071b417 CALL 00bea680` (`BSP_StructuredNode_CreateChild`)
- `0071b435 CALL 00bf7fbf` (`FUN_00bf7fbf`)
- `0071b451 CALL 00718f50` (`BSP_StructuredNode_ReadCharacterString`)
- `0071b458 CALL 00be9a80` (`BSP_StructuredNode_ReadU32ToOutParam`)
- `0071b473 CALL 00bf7fbf` (`FUN_00bf7fbf`)
- `0071b48b CALL 00718f50` (`BSP_StructuredNode_ReadCharacterString`)
- `0071b4a3 CALL 00bf7fbf` (`FUN_00bf7fbf`)
- `0071b4d5 CALL 00be9a60` (`BSP_StructuredNodeHandle_ReadFloat`)
- `0071b4dc CALL 00be9a60` (`BSP_StructuredNodeHandle_ReadFloat`)
- `0071b4e3 CALL 00be9a60` (`BSP_StructuredNodeHandle_ReadFloat`)
- `0071b4f0 CALL 004215d0` (`MSVC_Vector12_PushBack`)
- `0071b4fb CALL 00be9c40` (`BSP_StructuredNode_SkipAndDetach`)
- `0071b50c CALL 00be9ed0` (`BSP_StructuredNodeHandle_Release`)
- `0071b513 CALL 00715bf0` (`BSP_StructuredNode_HasRemaining`)
