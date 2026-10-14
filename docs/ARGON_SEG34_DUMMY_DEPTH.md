# Segment 34 (`006fa540` through `007016c0`) DynWorld, ConvexHull, UnitDebugInspector, ElevatorPlatform, DepthCharge, TurningGun & Dummy Entity Reconstruction (`argon-seg34`)

## Overview

This document records the evidence, ABI notes, x87 `DC`-form inspection, and graph verification for the 76 previously un-reconstructed non-funclet functions in Segment 34 (`006fa540` through `007016c0`).

- **Header:** `include/bsp/dummy_depth_seg34_argon.hpp`
- **Implementation:** `src/dummy_depth_seg34_argon.cpp`
- **Report:** `reports/argon_seg34_dummy_depth.json`
- **Status:** `reconstructed_build_tested` (semantic C++ reconstructions; not drop-in binary ABI replacements)

## x87 `DC`-Form Register-Destination Audit

Running `python tools/x87_dc_form_scan.py over `006fa540` through `007016c0`` reported 3 sites in Segment 34, all of which reside in already-reconstructed functions outside this 76-function packet (`006fc123` `dc e9 FSUB ST1, ST0` in `BSP_ElevatorPlatform_CarryPlane`, `006fcefa` `dc c9 FMUL ST1, ST0` in `BSP_DepthChargeProjectile_AdvanceInWater`, and `006fda2b` `dc c1 FADD ST1, ST0` in `BSP_DepthCharge_Activate`). None of the 76 functions reconstructed in `argon-seg34` contain `DC`-form register-destination x87 instructions.

## Reconstructed Functions and Verified Sites

| Address | Original Name | Reconstructed Symbol | Size |
| :--- | :--- | :--- | ---: |
| `006fa540` | `BSP_DynWorld_ForEachBodyDebugDraw` | `bsp::dummy_depth_seg34_argon::DynWorld_ForEachBodyDebugDraw_006fa540` | 0 |
| `006fa6d0` | `BSP_DynWorld_ForEachBodyTransform` | `bsp::dummy_depth_seg34_argon::DynWorld_ForEachBodyTransform_006fa6d0` | 0 |
| `006faf00` | `BSP_ConvexObjectResourceParser_Create` | `bsp::dummy_depth_seg34_argon::ConvexObjectResourceParser_Create_006faf00` | 0 |
| `006fb030` | `BSP_ConvexHull_DebugDrawEdges` | `bsp::dummy_depth_seg34_argon::ConvexHull_DebugDrawEdges_006fb030` | 0 |
| `006fb680` | `BSP_UnitDebugInspector_OnTargetGone` | `bsp::dummy_depth_seg34_argon::UnitDebugInspector_OnTargetGone_006fb680` | 0 |
| `006fb6a0` | `BSP_UnitDebugInspector_SetTarget` | `bsp::dummy_depth_seg34_argon::UnitDebugInspector_SetTarget_006fb6a0` | 0 |
| `006fb750` | `BSP_UnitDebugInspector_PrintFlooded` | `bsp::dummy_depth_seg34_argon::UnitDebugInspector_PrintFlooded_006fb750` | 0 |
| `006fb7c0` | `BSP_UnitDebugInspector_PrintLine` | `bsp::dummy_depth_seg34_argon::UnitDebugInspector_PrintLine_006fb7c0` | 0 |
| `006fb8f0` | `BSP_UnitDebugInspector_ResetOnUnwind` | `bsp::dummy_depth_seg34_argon::UnitDebugInspector_ResetOnUnwind_006fb8f0` | 0 |
| `006fb920` | `BSP_UnitDebugInspector_Construct` | `bsp::dummy_depth_seg34_argon::UnitDebugInspector_Construct_006fb920` | 0 |
| `006fb9f0` | `BSP_UnitDebugInspector_Destruct` | `bsp::dummy_depth_seg34_argon::UnitDebugInspector_Destruct_006fb9f0` | 0 |
| `006fba70` | `BSP_Entity_AsKind04_006fba70` | `bsp::dummy_depth_seg34_argon::Entity_AsKind04_006fba70` | 0 |
| `006fba90` | `BSP_Entity_AsKind04_006fba90` | `bsp::dummy_depth_seg34_argon::Entity_AsKind04_006fba90` | 0 |
| `006fbad0` | `BSP_UnitDebugInspector_PrintUnitPanel` | `bsp::dummy_depth_seg34_argon::UnitDebugInspector_PrintUnitPanel_006fbad0` | 0 |
| `006fbb80` | `BSP_UnitDebugInspector_Draw` | `bsp::dummy_depth_seg34_argon::UnitDebugInspector_Draw_006fbb80` | 0 |
| `006fbbc0` | `BSP_UnitDebugInspector_FrameCallback` | `bsp::dummy_depth_seg34_argon::UnitDebugInspector_FrameCallback_006fbbc0` | 0 |
| `006fbc20` | `BSP_OwnedPtrWithDefault_Assign_006fbc20` | `bsp::dummy_depth_seg34_argon::OwnedPtrWithDefault_Assign_006fbc20` | 0 |
| `006fbce0` | `BSP_ElevatorPlatform_StartTravel` | `bsp::dummy_depth_seg34_argon::ElevatorPlatform_StartTravel_006fbce0` | 0 |
| `006fbf00` | `BSP_ElevatorPlatform_ReadState` | `bsp::dummy_depth_seg34_argon::ElevatorPlatform_ReadState_006fbf00` | 0 |
| `006fbfd0` | `BSP_ElevatorPlatform_WriteState` | `bsp::dummy_depth_seg34_argon::ElevatorPlatform_WriteState_006fbfd0` | 0 |
| `006fc2e0` | `BSP_ElevatorPlatform_Destruct` | `bsp::dummy_depth_seg34_argon::ElevatorPlatform_Destruct_006fc2e0` | 0 |
| `006fc960` | `BSP_DepthChargeProjectile_Construct` | `bsp::dummy_depth_seg34_argon::DepthChargeProjectile_Construct_006fc960` | 0 |
| `006fc9d0` | `BSP_DepthCharge_VSlot198_ForwardTo194` | `bsp::dummy_depth_seg34_argon::DepthCharge_VSlot198_ForwardTo194_006fc9d0` | 0 |
| `006fc9f0` | `BSP_SessionMessage_DEPTHCHARGE_CREATE_ApplyFields` | `bsp::dummy_depth_seg34_argon::SessionMessage_DEPTHCHARGE_CREATE_ApplyFields_006fc9f0` | 0 |
| `006fca50` | `BSP_Random_UniformBetweenB4andB8_006fca50` | `bsp::dummy_depth_seg34_argon::Random_UniformBetweenB4andB8_006fca50` | 0 |
| `006fcb60` | `BSP_SessionMessage_DEPTHCHARGE_CREATE_Write` | `bsp::dummy_depth_seg34_argon::SessionMessage_DEPTHCHARGE_CREATE_Write_006fcb60` | 0 |
| `006fd0d0` | `BSP_DepthCharge_SendCreateMessage` | `bsp::dummy_depth_seg34_argon::DepthCharge_SendCreateMessage_006fd0d0` | 0 |
| `006fd190` | `BSP_DepthCharge_RegisterInWorldLists` | `bsp::dummy_depth_seg34_argon::DepthCharge_RegisterInWorldLists_006fd190` | 0 |
| `006fd1d0` | `BSP_DepthCharge_DetachFromWorldLists` | `bsp::dummy_depth_seg34_argon::DepthCharge_DetachFromWorldLists_006fd1d0` | 0 |
| `006fded0` | `BSP_RapidTurningGun_Construct` | `bsp::dummy_depth_seg34_argon::RapidTurningGun_Construct_006fded0` | 0 |
| `006fdfc0` | `BSP_SingleTurningGun_Construct` | `bsp::dummy_depth_seg34_argon::SingleTurningGun_Construct_006fdfc0` | 0 |
| `006fe730` | `BSP_DummyKamikazePlaneClass_ResolveLinkedClass` | `bsp::dummy_depth_seg34_argon::DummyKamikazePlaneClass_ResolveLinkedClass_006fe730` | 0 |
| `006fe780` | `BSP_LuaTable_ReadNumberOrDefault` | `bsp::dummy_depth_seg34_argon::LuaTable_ReadNumberOrDefault_006fe780` | 0 |
| `006fe820` | `BSP_DummyKamikazePlane_InitAll` | `bsp::dummy_depth_seg34_argon::DummyKamikazePlane_InitAll_006fe820` | 0 |
| `006fe8b0` | `BSP_Dummy2D_Destruct` | `bsp::dummy_depth_seg34_argon::Dummy2D_Destruct_006fe8b0` | 0 |
| `006fe950` | `BSP_DummyKamikazePlane_Construct` | `bsp::dummy_depth_seg34_argon::DummyKamikazePlane_Construct_006fe950` | 0 |
| `006fea80` | `BSP_Dummy2D_RegisterInWorldLists` | `bsp::dummy_depth_seg34_argon::Dummy2D_RegisterInWorldLists_006fea80` | 0 |
| `006feac0` | `BSP_Dummy2D_DetachFromWorldLists` | `bsp::dummy_depth_seg34_argon::Dummy2D_DetachFromWorldLists_006feac0` | 0 |
| `006feb00` | `BSP_Dummy2D_GetName` | `bsp::dummy_depth_seg34_argon::Dummy2D_GetName_006feb00` | 0 |
| `006feb30` | `BSP_DummyKamikazePlane_RegisterInWorldLists` | `bsp::dummy_depth_seg34_argon::DummyKamikazePlane_RegisterInWorldLists_006feb30` | 0 |
| `006feb80` | `BSP_DummyKamikazePlane_DetachFromWorldLists` | `bsp::dummy_depth_seg34_argon::DummyKamikazePlane_DetachFromWorldLists_006feb80` | 0 |
| `006ff100` | `BSP_DummyKamikazePlaneClass_CreateEntity` | `bsp::dummy_depth_seg34_argon::DummyKamikazePlaneClass_CreateEntity_006ff100` | 0 |
| `006ff2f0` | `BSP_DummyShot_BindOwnerCommand` | `bsp::dummy_depth_seg34_argon::DummyShot_BindOwnerCommand_006ff2f0` | 0 |
| `006ff3f0` | `BSP_Dummy2D_Construct` | `bsp::dummy_depth_seg34_argon::Dummy2D_Construct_006ff3f0` | 0 |
| `006ff570` | `BSP_Dummy2D_Activate` | `bsp::dummy_depth_seg34_argon::Dummy2D_Activate_006ff570` | 0 |
| `006ff6a0` | `BSP_DummySubmarineClass_VSlot28_Finalise` | `bsp::dummy_depth_seg34_argon::DummySubmarineClass_VSlot28_Finalise_006ff6a0` | 0 |
| `006ff700` | `BSP_DummySubmarine_VSlot198_ForwardTo194` | `bsp::dummy_depth_seg34_argon::DummySubmarine_VSlot198_ForwardTo194_006ff700` | 0 |
| `006ff730` | `BSP_DummySubmarine_InitAll` | `bsp::dummy_depth_seg34_argon::DummySubmarine_InitAll_006ff730` | 0 |
| `006ff7c0` | `BSP_ObserverMember_SetTarget_006ff7c0` | `bsp::dummy_depth_seg34_argon::ObserverMember_SetTarget_006ff7c0` | 0 |
| `006ff800` | `BSP_DummySubmarine_Construct` | `bsp::dummy_depth_seg34_argon::DummySubmarine_Construct_006ff800` | 0 |
| `006ff890` | `BSP_DummySubmarineShot_IsInWater_True` | `bsp::dummy_depth_seg34_argon::DummySubmarineShot_IsInWater_True_006ff890` | 0 |
| `006ff920` | `BSP_DummySubmarine_DestructBody` | `bsp::dummy_depth_seg34_argon::DummySubmarine_DestructBody_006ff920` | 0 |
| `006ff970` | `BSP_DummySubmarine_RegisterInWorldLists` | `bsp::dummy_depth_seg34_argon::DummySubmarine_RegisterInWorldLists_006ff970` | 0 |
| `006ff9c0` | `BSP_DummySubmarine_DetachFromWorldLists` | `bsp::dummy_depth_seg34_argon::DummySubmarine_DetachFromWorldLists_006ff9c0` | 0 |
| `006fffe0` | `BSP_DummySubmarineClass_CreateEntity` | `bsp::dummy_depth_seg34_argon::DummySubmarineClass_CreateEntity_006fffe0` | 0 |
| `00700150` | `BSP_DummyTargetBulletClass_ResolveLinkedClass` | `bsp::dummy_depth_seg34_argon::DummyTargetBulletClass_ResolveLinkedClass_00700150` | 0 |
| `00700200` | `BSP_DummyTargetVehicleClass_ReconCategory13` | `bsp::dummy_depth_seg34_argon::DummyTargetVehicleClass_ReconCategory13_00700200` | 0 |
| `00700250` | `BSP_DummyTargetVehicleClass_ReconMarkDirty` | `bsp::dummy_depth_seg34_argon::DummyTargetVehicleClass_ReconMarkDirty_00700250` | 0 |
| `00700260` | `BSP_DummyTargetVehicle_Activate` | `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_Activate_00700260` | 0 |
| `00700290` | `BSP_LuaObject_ReadNumberFieldOr_00700290` | `bsp::dummy_depth_seg34_argon::LuaObject_ReadNumberFieldOr_00700290` | 0 |
| `00700330` | `BSP_LuaObject_ReadNumberField_00700330` | `bsp::dummy_depth_seg34_argon::LuaObject_ReadNumberField_00700330` | 0 |
| `007003b0` | `BSP_LuaObject_ReadNumberField_007003b0` | `bsp::dummy_depth_seg34_argon::LuaObject_ReadNumberField_007003b0` | 0 |
| `00700520` | `BSP_DummyTarget_Activate` | `bsp::dummy_depth_seg34_argon::DummyTarget_Activate_00700520` | 0 |
| `00700590` | `BSP_DummyTargetVehicle_Construct` | `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_Construct_00700590` | 0 |
| `00700840` | `BSP_DummyTargetVehicle_GetName` | `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_GetName_00700840` | 0 |
| `00700870` | `BSP_DummyTargetVehicle_Serialize` | `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_Serialize_00700870` | 0 |
| `00700940` | `BSP_ObserverMember_SetTarget_00700940` | `bsp::dummy_depth_seg34_argon::ObserverMember_SetTarget_00700940` | 0 |
| `00700980` | `BSP_DummyTargetVehicle_RegisterInWorldLists` | `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_RegisterInWorldLists_00700980` | 0 |
| `007009c0` | `BSP_DummyTargetVehicle_DetachFromWorldLists` | `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_DetachFromWorldLists_007009c0` | 0 |
| `00700a20` | `BSP_DummyTarget_Construct` | `bsp::dummy_depth_seg34_argon::DummyTarget_Construct_00700a20` | 0 |
| `00700b10` | `BSP_DummyTarget_RegisterInWorldLists` | `bsp::dummy_depth_seg34_argon::DummyTarget_RegisterInWorldLists_00700b10` | 0 |
| `00700b60` | `BSP_DummyTarget_DetachFromWorldLists` | `bsp::dummy_depth_seg34_argon::DummyTarget_DetachFromWorldLists_00700b60` | 0 |
| `00700dd0` | `BSP_DummyTargetBulletClass_CreateEntity` | `bsp::dummy_depth_seg34_argon::DummyTargetBulletClass_CreateEntity_00700dd0` | 0 |
| `00701170` | `BSP_DummyTargetVehicle_Create` | `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_Create_00701170` | 0 |
| `00701250` | `BSP_DummyTargetVehicle_HitOnTheFly` | `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_HitOnTheFly_00701250` | 0 |
| `007016c0` | `BSP_DummyTargetVehicle_InitAll` | `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_InitAll_007016c0` | 0 |

### `006fa540` (`BSP_DynWorld_ForEachBodyDebugDraw` -> `bsp::dummy_depth_seg34_argon::DynWorld_ForEachBodyDebugDraw_006fa540`)
- `006fa547 CALL 00c31dc0` (`DYN_physics_00c31dc0`)
- `006fa559 CALL 00c32000` (`BSP_Object_FieldAt8Accessor`)
- `006fa562 CALL 00c32430` (`DYN_physics_00c32430`)
- `006fa6aa CALL 00c33650` (`Dyn_Transform_Expand3x4To4x4`)
- `006fa6b5 CALL 006fa320` (`BSP_ConvexObject_BuildDebugMesh`)

### `006fa6d0` (`BSP_DynWorld_ForEachBodyTransform` -> `bsp::dummy_depth_seg34_argon::DynWorld_ForEachBodyTransform_006fa6d0`)
- `006fa6da CALL 00c31dc0` (`DYN_physics_00c31dc0`)
- `006fa6f7 CALL 00c32430` (`DYN_physics_00c32430`)
- `006fa6fe CALL 00c33650` (`Dyn_Transform_Expand3x4To4x4`)
- `006fa70d CALL 00413920` (`BSP_Matrix_Multiply4x4`)
- `006fa7c7 CALL 006fa320` (`BSP_ConvexObject_BuildDebugMesh`)

### `006faf00` (`BSP_ConvexObjectResourceParser_Create` -> `bsp::dummy_depth_seg34_argon::ConvexObjectResourceParser_Create_006faf00`)
- `006faf19 CALL 00bf681b` (`operator_new`)
- `006faf31 CALL 006f9cd0` (`BSP_ConvexObject_Construct`)
- `006faf47 CALL 006fad70` (`BSP_ConvexObject_ReadPayload`)

### `006fb030` (`BSP_ConvexHull_DebugDrawEdges` -> `bsp::dummy_depth_seg34_argon::ConvexHull_DebugDrawEdges_006fb030`)
- `006fb054 CALL 00c32d20` (`DYN_ConvexHull_VertexCount_00C32D20`)
- `006fb092 CALL 00421ae0` (`STL_Vector_Resize_00421ae0`)
- `006fb0b7 CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `006fb0c3 CALL 00c32ce0` (`DYN_physics_00c32ce0`)
- `006fb0e3 CALL 00c37430` (`DYN_physics_00c37430`)
- `006fb12c CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `006fb174 CALL 004142e0` (`BSP_Vector3f_TransformAffinePoint`)
- `006fb1d4 CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `006fb218 CALL 004142e0` (`BSP_Vector3f_TransformAffinePoint`)
- `006fb288 CALL 0085fb30` (`BSP_DebugDraw_LineRgbaFloatsOneColour`)
- `006fb2c8 CALL 00bf6989` (`_free`)
- `006fb2d9 CALL 00bf65ac` (`_free`)

### `006fb680` (`BSP_UnitDebugInspector_OnTargetGone` -> `bsp::dummy_depth_seg34_argon::UnitDebugInspector_OnTargetGone_006fb680`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `006fb6a0` (`BSP_UnitDebugInspector_SetTarget` -> `bsp::dummy_depth_seg34_argon::UnitDebugInspector_SetTarget_006fb6a0`)
- `006fb6ac CALL 006952a0` (`BSP_Observer_UnregisterPair`)
- `006fb6be CALL 00694a60` (`BSP_Observer_RegisterPair`)

### `006fb750` (`BSP_UnitDebugInspector_PrintFlooded` -> `bsp::dummy_depth_seg34_argon::UnitDebugInspector_PrintFlooded_006fb750`)
- `006fb75d CALL 0074fa80` (`BSP_LeakManager_FloodedFraction`)
- `006fb7a6 CALL 004c7070` (`BSP_DiagnosticPrintf`)

### `006fb7c0` (`BSP_UnitDebugInspector_PrintLine` -> `bsp::dummy_depth_seg34_argon::UnitDebugInspector_PrintLine_006fb7c0`)
- `006fb81e CALL 004c7070` (`BSP_DiagnosticPrintf`)

### `006fb8f0` (`BSP_UnitDebugInspector_ResetOnUnwind` -> `bsp::dummy_depth_seg34_argon::UnitDebugInspector_ResetOnUnwind_006fb8f0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `006fb920` (`BSP_UnitDebugInspector_Construct` -> `bsp::dummy_depth_seg34_argon::UnitDebugInspector_Construct_006fb920`)
- `006fb971 CALL 00bf681b` (`operator_new`)
- `006fb991 CALL 004d63d0` (`STL_List_PushBack_004d63d0`)

### `006fb9f0` (`BSP_UnitDebugInspector_Destruct` -> `bsp::dummy_depth_seg34_argon::UnitDebugInspector_Destruct_006fb9f0`)
- `006fba4e CALL 00695870` (`BSP_CallbackOwner_Destroy`)

### `006fba70` (`BSP_Entity_AsKind04_006fba70` -> `bsp::dummy_depth_seg34_argon::Entity_AsKind04_006fba70`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `006fba90` (`BSP_Entity_AsKind04_006fba90` -> `bsp::dummy_depth_seg34_argon::Entity_AsKind04_006fba90`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `006fbad0` (`BSP_UnitDebugInspector_PrintUnitPanel` -> `bsp::dummy_depth_seg34_argon::UnitDebugInspector_PrintUnitPanel_006fbad0`)
- `006fbae4 CALL 006fb7c0` (`BSP_UnitDebugInspector_PrintLine`)
- `006fbafb CALL 006fb750` (`BSP_UnitDebugInspector_PrintFlooded`)
- `006fbb2f CALL 00bf6713` (`LIBCRT_unmatched_00bf6713`)
- `006fbb5f CALL 006fbad0` (`BSP_UnitDebugInspector_PrintUnitPanel`)

### `006fbb80` (`BSP_UnitDebugInspector_Draw` -> `bsp::dummy_depth_seg34_argon::UnitDebugInspector_Draw_006fbb80`)
- `006fbbb4 CALL 006fbad0` (`BSP_UnitDebugInspector_PrintUnitPanel`)

### `006fbbc0` (`BSP_UnitDebugInspector_FrameCallback` -> `bsp::dummy_depth_seg34_argon::UnitDebugInspector_FrameCallback_006fbbc0`)
- `006fbbc3 CALL 0051f5e0` (`BSP_Singleton00E19AAC_GetOrCreate`)
- `006fbbf9 CALL 006fbad0` (`BSP_UnitDebugInspector_PrintUnitPanel`)

### `006fbc20` (`BSP_OwnedPtrWithDefault_Assign_006fbc20` -> `bsp::dummy_depth_seg34_argon::OwnedPtrWithDefault_Assign_006fbc20`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `006fbce0` (`BSP_ElevatorPlatform_StartTravel` -> `bsp::dummy_depth_seg34_argon::ElevatorPlatform_StartTravel_006fbce0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `006fbf00` (`BSP_ElevatorPlatform_ReadState` -> `bsp::dummy_depth_seg34_argon::ElevatorPlatform_ReadState_006fbf00`)
- `006fbf3b CALL 006cf0a0` (`BSP_SaveReader_ReadObserverRef`)
- `006fbfaf CALL 00410630` (`BSP_ArchiveReaderRef_Destroy`)

### `006fbfd0` (`BSP_ElevatorPlatform_WriteState` -> `bsp::dummy_depth_seg34_argon::ElevatorPlatform_WriteState_006fbfd0`)
- `006fc005 CALL 006ce8a0` (`BSP_FollowObserverMember_CopyConstruct`)
- `006fc02a CALL 006cead0` (`BSP_SaveWriter_WriteObserverRef`)
- `006fc0a8 CALL 004107b0` (`BSP_ArchiveWriterRef_Destroy`)

### `006fc2e0` (`BSP_ElevatorPlatform_Destruct` -> `bsp::dummy_depth_seg34_argon::ElevatorPlatform_Destruct_006fc2e0`)
- `006fc354 CALL 006cdd70` (`BSP_FollowObserverMember_Destroy`)

### `006fc960` (`BSP_DepthChargeProjectile_Construct` -> `bsp::dummy_depth_seg34_argon::DepthChargeProjectile_Construct_006fc960`)
- `006fc968 CALL 006e2670` (`BSP_BombProjectile_Construct`)

### `006fc9d0` (`BSP_DepthCharge_VSlot198_ForwardTo194` -> `bsp::dummy_depth_seg34_argon::DepthCharge_VSlot198_ForwardTo194_006fc9d0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `006fc9f0` (`BSP_SessionMessage_DEPTHCHARGE_CREATE_ApplyFields` -> `bsp::dummy_depth_seg34_argon::SessionMessage_DEPTHCHARGE_CREATE_ApplyFields_006fc9f0`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `006fca50` (`BSP_Random_UniformBetweenB4andB8_006fca50` -> `bsp::dummy_depth_seg34_argon::Random_UniformBetweenB4andB8_006fca50`)
- `006fca6b CALL 00bd2f10` (`BSP_Random_UniformFloatRange`)

### `006fcb60` (`BSP_SessionMessage_DEPTHCHARGE_CREATE_Write` -> `bsp::dummy_depth_seg34_argon::SessionMessage_DEPTHCHARGE_CREATE_Write_006fcb60`)
- `006fcb69 CALL 006e0690` (`BSP_SessionMessage_BOMB_CREATE_Write`)
- `006fcb8a CALL 004295c0` (`BSP_BitCursor_WriteNumericFloat`)
- `006fcbab CALL 004295c0` (`BSP_BitCursor_WriteNumericFloat`)

### `006fd0d0` (`BSP_DepthCharge_SendCreateMessage` -> `bsp::dummy_depth_seg34_argon::DepthCharge_SendCreateMessage_006fd0d0`)
- `006fd102 CALL 006e0d20` (`BSP_SessionMessage_BOMB_CREATE_Construct`)
- `006fd14b CALL 00779fc0` (`BSP_Entity_SendMessageToPeer`)
- `006fd167 CALL 007668e0` (`BSP_NetEntityCreateMessage_Destruct`)

### `006fd190` (`BSP_DepthCharge_RegisterInWorldLists` -> `bsp::dummy_depth_seg34_argon::DepthCharge_RegisterInWorldLists_006fd190`)
- `006fd193 CALL 00928560` (`BSP_GameEntity_RegisterInParentEntityList`)
- `006fd19f CALL 00484540` (`BSP_UnitList_PushBack`)
- `006fd1ae CALL 00484540` (`BSP_UnitList_PushBack`)
- `006fd1bd CALL 00484540` (`BSP_UnitList_PushBack`)

### `006fd1d0` (`BSP_DepthCharge_DetachFromWorldLists` -> `bsp::dummy_depth_seg34_argon::DepthCharge_DetachFromWorldLists_006fd1d0`)
- `006fd1d3 CALL 006e29c0` (`BSP_BombFamily_DetachFromWorldLists`)
- `006fd1ff CALL 004837d0` (`BSP_UnitList_EraseNode`)

### `006fded0` (`BSP_RapidTurningGun_Construct` -> `bsp::dummy_depth_seg34_argon::RapidTurningGun_Construct_006fded0`)
- `006fded8 CALL 006fdda0` (`BSP_TurningGunBase_Construct`)

### `006fdfc0` (`BSP_SingleTurningGun_Construct` -> `bsp::dummy_depth_seg34_argon::SingleTurningGun_Construct_006fdfc0`)
- `006fdfc8 CALL 006fded0` (`BSP_RapidTurningGun_Construct`)

### `006fe730` (`BSP_DummyKamikazePlaneClass_ResolveLinkedClass` -> `bsp::dummy_depth_seg34_argon::DummyKamikazePlaneClass_ResolveLinkedClass_006fe730`)
- `006fe733 CALL 006e97f0` (`BSP_BulletClass_ResolveLinkedClass`)

### `006fe780` (`BSP_LuaTable_ReadNumberOrDefault` -> `bsp::dummy_depth_seg34_argon::LuaTable_ReadNumberOrDefault_006fe780`)
- `006fe7ad CALL 00b67800` (`BSP_LuaObject_GetByName`)
- `006fe7bb CALL 00b65fb0` (`BSP_LuaObject_IsNil`)
- `006fe7d0 CALL 00b66270` (`BSP_LuaObject_GetNumber`)
- `006fe7ec CALL 00b67700` (`BSP_LuaObject_Destruct`)
- `006fe7fd CALL 00b67700` (`BSP_LuaObject_Destruct`)

### `006fe820` (`BSP_DummyKamikazePlane_InitAll` -> `bsp::dummy_depth_seg34_argon::DummyKamikazePlane_InitAll_006fe820`)
- `006fe823 CALL 0077fad0` (`BSP_NetEntity_ReadReplicationState`)
- `006fe82c CALL 006e0b40` (`BSP_BombProjectile_SetVisible`)
- `006fe835 CALL 00923810` (`FUN_00923810`)
- `006fe843 CALL 00923810` (`FUN_00923810`)
- `006fe84e CALL 00923810` (`FUN_00923810`)
- `006fe86a CALL 00923810` (`FUN_00923810`)
- `006fe875 CALL 00923810` (`FUN_00923810`)

### `006fe8b0` (`BSP_Dummy2D_Destruct` -> `bsp::dummy_depth_seg34_argon::Dummy2D_Destruct_006fe8b0`)
- `006fe8db CALL 0042bca0` (`BSP_TargetObserver_Destruct`)
- `006fe8eb CALL 006e0860` (`BSP_ObserverMember9348_Destruct`)
- `006fe8fa CALL 006e2a20` (`BSP_BombProjectile_Destruct`)

### `006fe950` (`BSP_DummyKamikazePlane_Construct` -> `bsp::dummy_depth_seg34_argon::DummyKamikazePlane_Construct_006fe950`)
- `006fe958 CALL 006ff3f0` (`BSP_Dummy2D_Construct`)

### `006fea80` (`BSP_Dummy2D_RegisterInWorldLists` -> `bsp::dummy_depth_seg34_argon::Dummy2D_RegisterInWorldLists_006fea80`)
- `006fea83 CALL 00928560` (`BSP_GameEntity_RegisterInParentEntityList`)
- `006fea8f CALL 00484540` (`BSP_UnitList_PushBack`)
- `006fea9e CALL 00484540` (`BSP_UnitList_PushBack`)
- `006feaad CALL 00484540` (`BSP_UnitList_PushBack`)

### `006feac0` (`BSP_Dummy2D_DetachFromWorldLists` -> `bsp::dummy_depth_seg34_argon::Dummy2D_DetachFromWorldLists_006feac0`)
- `006feac3 CALL 006e29c0` (`BSP_BombFamily_DetachFromWorldLists`)
- `006feaef CALL 004837d0` (`BSP_UnitList_EraseNode`)

### `006feb00` (`BSP_Dummy2D_GetName` -> `bsp::dummy_depth_seg34_argon::Dummy2D_GetName_006feb00`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `006feb30` (`BSP_DummyKamikazePlane_RegisterInWorldLists` -> `bsp::dummy_depth_seg34_argon::DummyKamikazePlane_RegisterInWorldLists_006feb30`)
- `006feb33 CALL 00928560` (`BSP_GameEntity_RegisterInParentEntityList`)
- `006feb3f CALL 00484540` (`BSP_UnitList_PushBack`)
- `006feb4e CALL 00484540` (`BSP_UnitList_PushBack`)
- `006feb5d CALL 00484540` (`BSP_UnitList_PushBack`)
- `006feb6c CALL 00484540` (`BSP_UnitList_PushBack`)

### `006feb80` (`BSP_DummyKamikazePlane_DetachFromWorldLists` -> `bsp::dummy_depth_seg34_argon::DummyKamikazePlane_DetachFromWorldLists_006feb80`)
- `006feb83 CALL 006e29c0` (`BSP_BombFamily_DetachFromWorldLists`)
- `006febaf CALL 004837d0` (`BSP_UnitList_EraseNode`)
- `006febd6 CALL 004837d0` (`BSP_UnitList_EraseNode`)

### `006ff100` (`BSP_DummyKamikazePlaneClass_CreateEntity` -> `bsp::dummy_depth_seg34_argon::DummyKamikazePlaneClass_CreateEntity_006ff100`)
- `006ff11f CALL 00bf55be` (`operator_new`)
- `006ff12e CALL 00bf79f0` (`_memset`)
- `006ff14d CALL 006fe950` (`BSP_DummyKamikazePlane_Construct`)

### `006ff2f0` (`BSP_DummyShot_BindOwnerCommand` -> `bsp::dummy_depth_seg34_argon::DummyShot_BindOwnerCommand_006ff2f0`)
- `006ff2fe CALL 00923810` (`FUN_00923810`)
- `006ff30f CALL 00923810` (`FUN_00923810`)
- `006ff31a CALL 00923810` (`FUN_00923810`)
- `006ff336 CALL 00923810` (`FUN_00923810`)
- `006ff341 CALL 00923810` (`FUN_00923810`)
- `006ff386 CALL 006e1820` (`BSP_ObserverMember_SetTarget_006e1820`)
- `006ff3e2 CALL 006e1f00` (`BSP_BombRack_DropCone`)

### `006ff3f0` (`BSP_Dummy2D_Construct` -> `bsp::dummy_depth_seg34_argon::Dummy2D_Construct_006ff3f0`)
- `006ff414 CALL 006e2670` (`BSP_BombProjectile_Construct`)
- `006ff4b8 CALL 006952a0` (`BSP_Observer_UnregisterPair`)

### `006ff570` (`BSP_Dummy2D_Activate` -> `bsp::dummy_depth_seg34_argon::Dummy2D_Activate_006ff570`)
- `006ff58d CALL 006e2e10` (`BSP_BombProjectile_Activate`)
- `006ff5b3 CALL 006952a0` (`BSP_Observer_UnregisterPair`)
- `006ff5cb CALL 0041dd40` (`BSP_NativeString_Resize`)
- `006ff5e6 CALL 00bf7680` (`_memcpy`)
- `006ff5f9 CALL 00927b40` (`BSP_MissionEntity_LuaSelfObject`)
- `006ff611 CALL 00b67460` (`BSP_LuaObject_SetNativeStringInteger`)
- `006ff61e CALL 00b67700` (`BSP_LuaObject_Destruct`)
- `006ff640 CALL 00419cc0` (`BSP_SizedStoragePool_GetSingleton`)
- `006ff647 CALL 00bd1510` (`BSP_SizedStoragePool_ReturnBlock`)

### `006ff6a0` (`BSP_DummySubmarineClass_VSlot28_Finalise` -> `bsp::dummy_depth_seg34_argon::DummySubmarineClass_VSlot28_Finalise_006ff6a0`)
- `006ff6a3 CALL 006e97f0` (`BSP_BulletClass_ResolveLinkedClass`)

### `006ff700` (`BSP_DummySubmarine_VSlot198_ForwardTo194` -> `bsp::dummy_depth_seg34_argon::DummySubmarine_VSlot198_ForwardTo194_006ff700`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `006ff730` (`BSP_DummySubmarine_InitAll` -> `bsp::dummy_depth_seg34_argon::DummySubmarine_InitAll_006ff730`)
- `006ff733 CALL 0077fad0` (`BSP_NetEntity_ReadReplicationState`)
- `006ff73c CALL 006e0b40` (`BSP_BombProjectile_SetVisible`)
- `006ff745 CALL 00923810` (`FUN_00923810`)
- `006ff753 CALL 00923810` (`FUN_00923810`)
- `006ff75e CALL 00923810` (`FUN_00923810`)
- `006ff77a CALL 00923810` (`FUN_00923810`)
- `006ff785 CALL 00923810` (`FUN_00923810`)

### `006ff7c0` (`BSP_ObserverMember_SetTarget_006ff7c0` -> `bsp::dummy_depth_seg34_argon::ObserverMember_SetTarget_006ff7c0`)
- `006ff7d5 CALL 006952a0` (`BSP_Observer_UnregisterPair`)
- `006ff7e5 CALL 00694a60` (`BSP_Observer_RegisterPair`)

### `006ff800` (`BSP_DummySubmarine_Construct` -> `bsp::dummy_depth_seg34_argon::DummySubmarine_Construct_006ff800`)
- `006ff808 CALL 006ff3f0` (`BSP_Dummy2D_Construct`)

### `006ff890` (`BSP_DummySubmarineShot_IsInWater_True` -> `bsp::dummy_depth_seg34_argon::DummySubmarineShot_IsInWater_True_006ff890`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `006ff920` (`BSP_DummySubmarine_DestructBody` -> `bsp::dummy_depth_seg34_argon::DummySubmarine_DestructBody_006ff920`)
- `006ff94b CALL 0042bca0` (`BSP_TargetObserver_Destruct`)
- `006ff95a CALL 006fe8b0` (`BSP_Dummy2D_Destruct`)

### `006ff970` (`BSP_DummySubmarine_RegisterInWorldLists` -> `bsp::dummy_depth_seg34_argon::DummySubmarine_RegisterInWorldLists_006ff970`)
- `006ff973 CALL 00928560` (`BSP_GameEntity_RegisterInParentEntityList`)
- `006ff97f CALL 00484540` (`BSP_UnitList_PushBack`)
- `006ff98e CALL 00484540` (`BSP_UnitList_PushBack`)
- `006ff99d CALL 00484540` (`BSP_UnitList_PushBack`)
- `006ff9ac CALL 00484540` (`BSP_UnitList_PushBack`)

### `006ff9c0` (`BSP_DummySubmarine_DetachFromWorldLists` -> `bsp::dummy_depth_seg34_argon::DummySubmarine_DetachFromWorldLists_006ff9c0`)
- `006ff9c3 CALL 006e29c0` (`BSP_BombFamily_DetachFromWorldLists`)
- `006ff9ef CALL 004837d0` (`BSP_UnitList_EraseNode`)
- `006ffa16 CALL 004837d0` (`BSP_UnitList_EraseNode`)

### `006fffe0` (`BSP_DummySubmarineClass_CreateEntity` -> `bsp::dummy_depth_seg34_argon::DummySubmarineClass_CreateEntity_006fffe0`)
- `006fffff CALL 00bf55be` (`operator_new`)
- `0070000e CALL 00bf79f0` (`_memset`)
- `0070002d CALL 006ff800` (`BSP_DummySubmarine_Construct`)

### `00700150` (`BSP_DummyTargetBulletClass_ResolveLinkedClass` -> `bsp::dummy_depth_seg34_argon::DummyTargetBulletClass_ResolveLinkedClass_00700150`)
- `00700153 CALL 006e97f0` (`BSP_BulletClass_ResolveLinkedClass`)

### `00700200` (`BSP_DummyTargetVehicleClass_ReconCategory13` -> `bsp::dummy_depth_seg34_argon::DummyTargetVehicleClass_ReconCategory13_00700200`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00700250` (`BSP_DummyTargetVehicleClass_ReconMarkDirty` -> `bsp::dummy_depth_seg34_argon::DummyTargetVehicleClass_ReconMarkDirty_00700250`)
- `00700254 CALL 00803ba0` (`BSP_Recon_MarkSlotDirty`)

### `00700260` (`BSP_DummyTargetVehicle_Activate` -> `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_Activate_00700260`)
- `00700263 CALL 00955420` (`BSP_UnitInstance_InitializeSceneBindings`)

### `00700290` (`BSP_LuaObject_ReadNumberFieldOr_00700290` -> `bsp::dummy_depth_seg34_argon::LuaObject_ReadNumberFieldOr_00700290`)
- `007002bd CALL 00b67800` (`BSP_LuaObject_GetByName`)
- `007002cb CALL 00b65fb0` (`BSP_LuaObject_IsNil`)
- `007002e0 CALL 00b66270` (`BSP_LuaObject_GetNumber`)
- `007002fc CALL 00b67700` (`BSP_LuaObject_Destruct`)
- `0070030d CALL 00b67700` (`BSP_LuaObject_Destruct`)

### `00700330` (`BSP_LuaObject_ReadNumberField_00700330` -> `bsp::dummy_depth_seg34_argon::LuaObject_ReadNumberField_00700330`)
- `0070035d CALL 00b67800` (`BSP_LuaObject_GetByName`)
- `00700369 CALL 00b66270` (`BSP_LuaObject_GetNumber`)
- `00700379 CALL 00b67700` (`BSP_LuaObject_Destruct`)
- `0070038a CALL 00b67700` (`BSP_LuaObject_Destruct`)

### `007003b0` (`BSP_LuaObject_ReadNumberField_007003b0` -> `bsp::dummy_depth_seg34_argon::LuaObject_ReadNumberField_007003b0`)
- `007003dd CALL 00b67800` (`BSP_LuaObject_GetByName`)
- `007003e9 CALL 00b66270` (`BSP_LuaObject_GetNumber`)
- `007003f9 CALL 00b67700` (`BSP_LuaObject_Destruct`)
- `0070040a CALL 00b67700` (`BSP_LuaObject_Destruct`)

### `00700520` (`BSP_DummyTarget_Activate` -> `bsp::dummy_depth_seg34_argon::DummyTarget_Activate_00700520`)
- `00700523 CALL 006ff570` (`BSP_Dummy2D_Activate`)
- `0070052c CALL 006e0b40` (`BSP_BombProjectile_SetVisible`)
- `00700535 CALL 00923810` (`FUN_00923810`)
- `00700543 CALL 00923810` (`FUN_00923810`)
- `0070054e CALL 00923810` (`FUN_00923810`)
- `0070056a CALL 00923810` (`FUN_00923810`)
- `00700575 CALL 00923810` (`FUN_00923810`)

### `00700590` (`BSP_DummyTargetVehicle_Construct` -> `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_Construct_00700590`)
- `0070059a CALL 0095cc90` (`BSP_UnitGameObject_Construct`)

### `00700840` (`BSP_DummyTargetVehicle_GetName` -> `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_GetName_00700840`)
- Leaf / indirect-only function (no direct `JMP` or `SUBROUTINE` transfer instructions).

### `00700870` (`BSP_DummyTargetVehicle_Serialize` -> `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_Serialize_00700870`)
- `0070087e CALL 0095bb10` (`BSP_VehicleUnit_LoadArchiveCommon`)
- `007008d7 CALL 00bd5680` (`BSP_TaggedValue_Equals`)

### `00700940` (`BSP_ObserverMember_SetTarget_00700940` -> `bsp::dummy_depth_seg34_argon::ObserverMember_SetTarget_00700940`)
- `00700955 CALL 006952a0` (`BSP_Observer_UnregisterPair`)
- `00700965 CALL 00694a60` (`BSP_Observer_RegisterPair`)

### `00700980` (`BSP_DummyTargetVehicle_RegisterInWorldLists` -> `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_RegisterInWorldLists_00700980`)
- `00700983 CALL 00928560` (`BSP_GameEntity_RegisterInParentEntityList`)
- `0070098f CALL 00484540` (`BSP_UnitList_PushBack`)
- `0070099b CALL 00484540` (`BSP_UnitList_PushBack`)
- `007009a7 CALL 00484540` (`BSP_UnitList_PushBack`)
- `007009b6 CALL 00484540` (`BSP_UnitList_PushBack`)

### `007009c0` (`BSP_DummyTargetVehicle_DetachFromWorldLists` -> `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_DetachFromWorldLists_007009c0`)
- `007009c3 CALL 006d3620` (`BSP_UnitInstance_DetachFromEarlyWorldLists`)
- `007009e4 CALL 004837d0` (`BSP_UnitList_EraseNode`)
- `00700a0f CALL 004837d0` (`BSP_UnitList_EraseNode`)

### `00700a20` (`BSP_DummyTarget_Construct` -> `bsp::dummy_depth_seg34_argon::DummyTarget_Construct_00700a20`)
- `00700a28 CALL 006ff3f0` (`BSP_Dummy2D_Construct`)

### `00700b10` (`BSP_DummyTarget_RegisterInWorldLists` -> `bsp::dummy_depth_seg34_argon::DummyTarget_RegisterInWorldLists_00700b10`)
- `00700b13 CALL 00928560` (`BSP_GameEntity_RegisterInParentEntityList`)
- `00700b1f CALL 00484540` (`BSP_UnitList_PushBack`)
- `00700b2e CALL 00484540` (`BSP_UnitList_PushBack`)
- `00700b3d CALL 00484540` (`BSP_UnitList_PushBack`)
- `00700b4c CALL 00484540` (`BSP_UnitList_PushBack`)

### `00700b60` (`BSP_DummyTarget_DetachFromWorldLists` -> `bsp::dummy_depth_seg34_argon::DummyTarget_DetachFromWorldLists_00700b60`)
- `00700b63 CALL 006e29c0` (`BSP_BombFamily_DetachFromWorldLists`)
- `00700b8f CALL 004837d0` (`BSP_UnitList_EraseNode`)
- `00700bb6 CALL 004837d0` (`BSP_UnitList_EraseNode`)

### `00700dd0` (`BSP_DummyTargetBulletClass_CreateEntity` -> `bsp::dummy_depth_seg34_argon::DummyTargetBulletClass_CreateEntity_00700dd0`)
- `00700def CALL 00bf55be` (`operator_new`)
- `00700dfe CALL 00bf79f0` (`_memset`)
- `00700e1d CALL 00700a20` (`BSP_DummyTarget_Construct`)

### `00701170` (`BSP_DummyTargetVehicle_Create` -> `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_Create_00701170`)
- `00701191 CALL 00bf55be` (`operator_new`)
- `007011a1 CALL 00bf79f0` (`_memset`)
- `007011bc CALL 00700590` (`BSP_DummyTargetVehicle_Construct`)
- `007011db CALL 004f04c0` (`FUN_004f04c0`)
- `00701207 CALL 0041dd40` (`BSP_NativeString_Resize`)
- `00701218 CALL 00bf7680` (`_memcpy`)
- `00701223 CALL 009553d0` (`BSP_Unit_SetVehicleClass`)

### `00701250` (`BSP_DummyTargetVehicle_HitOnTheFly` -> `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_HitOnTheFly_00701250`)
- `0070126e CALL 004c1570` (`BSP_MissionEntityLock_Get`)
- `007012aa CALL 0041dd40` (`BSP_NativeString_Resize`)
- `007012c5 CALL 00bf7680` (`_memcpy`)
- `007012dc CALL 00927b40` (`BSP_MissionEntity_LuaSelfObject`)
- `007012f2 CALL 00b673a0` (`BSP_LuaObject_SetBoolean`)
- `00701303 CALL 00b67700` (`BSP_LuaObject_Destruct`)
- `00701323 CALL 00419cc0` (`BSP_SizedStoragePool_GetSingleton`)
- `0070132a CALL 00bd1510` (`BSP_SizedStoragePool_ReturnBlock`)
- `0070133c CALL 00414db0` (`BSP_EntityPose_RefreshWorld`)
- `00701402 CALL 00868420` (`BSP_PointEffect_CreateFromMatrix`)
- `00701431 CALL 00926d90` (`BSP_MissionEntity_Kill`)

### `007016c0` (`BSP_DummyTargetVehicle_InitAll` -> `bsp::dummy_depth_seg34_argon::DummyTargetVehicle_InitAll_007016c0`)
- `007016dd CALL 0095e5b0` (`BSP_Unit_ReadCommonLuaProperties`)
- `007016fc CALL 008f2260` (`BSP_ScenePropertyBag_Find`)
- `00701744 CALL 00805cf0` (`BSP_ReconDetection_ForceLevel`)
- `00701775 CALL 009238a0` (`SEntity_GetSavedEntityLuaData`)
- `0070177e CALL 004425c0` (`BSP_LuaReader_Construct`)
- `0070179d CALL 00bd8e20` (`BSP_LuaReader_EnterKey`)
- `007017ec CALL 00bd68d0` (`BSP_LuaReader_ReadFieldOrDefault`)
- `0070181f CALL 00bd6830` (`BSP_LuaReader_ReadField`)
- `00701828 CALL 00bd7a20` (`BSP_LuaReader_LeaveKeyThunk`)
- `00701841 CALL 00441210` (`BSP_LuaObjectVector_Tidy`)

## Lead review notes (cc13, 2026-10-14)

- Merged to main as parts B/C of the argon review: seg36-37, seg46 and seg34, without argon's first five segments.
- Blind audit: 2 of 30 sampled seg34 functions defective, confirmed by a second reviewer; both are fixed in this merge:
  - 006FF570 Dummy2D_Activate, and the same idiom in 00701250 DummyTargetVehicle_HitOnTheFly: the pool free.
    The binary pushes (data, size + 1, 1), calls 00419CC0 (no arguments; returns the pool), then 00BD1510 as a
    thiscall on the pool. The reconstruction had passed the arguments to 00419CC0 and called 00BD1510 bare.
  - 006FBF00 ElevatorPlatform_ReadState: 006CF0A0 and the "state" read take the 12-byte by-value reader itself
    (its w0 is a vptr), not ref.obj.
- OPEN: this file and src/flak_note_seg36_37_argon.cpp call original functions by absolute address through n_at
  (155 + 185 sites). bsp_game.exe loads at 0x10000000 and maps only read-only data spans of the original image, so
  those addresses hold no code in the game process. The code is not called by the game today; before anything is
  wired to it, convert each n_at call to a named hook or an existing reconstruction.
