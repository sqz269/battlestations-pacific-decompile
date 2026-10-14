#pragma once

// Segment 34 (0x006fa540..0x007016c0) Reconstruction Declarations
// Packet: argon-seg34
// Evidence doc: docs/ARGON_SEG34_DUMMY_DEPTH.md
// Report: reports/argon_seg34_dummy_depth.json
//
// Reconstructed and build-tested C++ implementations read from Ghidra pseudocode
// and verified against x86/x87 assembly. Function and field names are hypotheses,
// not recovered symbols, and expose C++ interfaces rather than drop-in binary
// ABI replacements.

#include <cstddef>
#include <cstdint>

namespace bsp::dummy_depth_seg34_argon {

// --- DynWorld / ConvexObject / ConvexHull Debug & Transform (0x006fa540..0x006fb030) ---
void DynWorld_ForEachBodyDebugDraw_006fa540(void* self);
void DynWorld_ForEachBodyTransform_006fa6d0(void* self, const float* rhs_mat4x4);
void* ConvexObjectResourceParser_Create_006faf00(void* payload_stream);
void ConvexHull_DebugDrawEdges_006fb030(void* self, const void* world_mat);

// --- UnitDebugInspector & Entity Kind04 Casts (0x006fb680..0x006fbbc0) ---
void UnitDebugInspector_OnTargetGone_006fb680(void* self, void* notifier);
void UnitDebugInspector_SetTarget_006fb6a0(void* self, void* target);
void UnitDebugInspector_PrintFlooded_006fb750(void* self, void* unit);
void UnitDebugInspector_PrintLine_006fb7c0(void* self, const char* text);
void UnitDebugInspector_ResetOnUnwind_006fb8f0(void* self);
void* UnitDebugInspector_Construct_006fb920(void* self);
void UnitDebugInspector_Destruct_006fb9f0(void* self);
void* Entity_AsKind04_006fba70(void* self);
void* Entity_AsKind04_006fba90(void* self);
void UnitDebugInspector_PrintUnitPanel_006fbad0(void* self, void* unit);
void UnitDebugInspector_Draw_006fbb80(void* self);
void UnitDebugInspector_FrameCallback_006fbbc0(void* self);

// --- ElevatorPlatform & OwnedPtrWithDefault (0x006fbc20..0x006fc2e0) ---
void OwnedPtrWithDefault_Assign_006fbc20(void* self, void* new_ptr);
void ElevatorPlatform_StartTravel_006fbce0(void* self, std::uint8_t downward_flag, float elapsed_time);
void ElevatorPlatform_ReadState_006fbf00(
    void* self,
    std::uint32_t reader_w0,
    std::uint32_t reader_w1,
    void* reader_obj
);
void ElevatorPlatform_WriteState_006fbfd0(
    void* self,
    std::uint32_t writer_w0,
    std::uint32_t writer_w1,
    void* writer_obj
);
void ElevatorPlatform_Destruct_006fc2e0(void* self);

// --- DepthCharge / DEPTHCHARGE_CREATE & TurningGun Constructors (0x006fc960..0x006fdfc0) ---
void* DepthChargeProjectile_Construct_006fc960(void* self, void* arg0);
void DepthCharge_VSlot198_ForwardTo194_006fc9d0(void* self, float arg0);
void SessionMessage_DEPTHCHARGE_CREATE_ApplyFields_006fc9f0(const void* self, void* entity);
double Random_UniformBetweenB4andB8_006fca50(const void* self);
void SessionMessage_DEPTHCHARGE_CREATE_Write_006fcb60(void* self, void* bit_stream);
void DepthCharge_SendCreateMessage_006fd0d0(void* self, std::uint32_t peer_arg);
void DepthCharge_RegisterInWorldLists_006fd190(void* self);
void DepthCharge_DetachFromWorldLists_006fd1d0(void* self);
void* RapidTurningGun_Construct_006fded0(void* self, void* arg0);
void* SingleTurningGun_Construct_006fdfc0(void* self, void* arg0);

// --- DummyKamikazePlane & Dummy2D (0x006fe730..0x006ff570) ---
void DummyKamikazePlaneClass_ResolveLinkedClass_006fe730(void* self);
void LuaTable_ReadNumberOrDefault_006fe780(
    const char* key_name,
    float* out_val,
    void* lua_table_by_val,
    float default_val
);
void DummyKamikazePlane_InitAll_006fe820(void* self);
void Dummy2D_Destruct_006fe8b0(void* self);
void* DummyKamikazePlane_Construct_006fe950(void* self, void* arg0);
void Dummy2D_RegisterInWorldLists_006fea80(void* self);
void Dummy2D_DetachFromWorldLists_006feac0(void* self);
const char* Dummy2D_GetName_006feb00(const void* self);
void DummyKamikazePlane_RegisterInWorldLists_006feb30(void* self);
void DummyKamikazePlane_DetachFromWorldLists_006feb80(void* self);
void* DummyKamikazePlaneClass_CreateEntity_006ff100(void* self, void* arg0);
void DummyShot_BindOwnerCommand_006ff2f0(void* self, std::uint32_t arg0, std::uint32_t arg1);
void* Dummy2D_Construct_006ff3f0(void* self, void* arg0);
void Dummy2D_Activate_006ff570(void* self);

// --- DummySubmarine (0x006ff6a0..0x006fffe0) ---
void DummySubmarineClass_VSlot28_Finalise_006ff6a0(void* self, std::uint32_t arg0);
void DummySubmarine_VSlot198_ForwardTo194_006ff700(void* self, float arg0);
void DummySubmarine_InitAll_006ff730(void* self);
void* ObserverMember_SetTarget_006ff7c0(void* self, void* target);
void* DummySubmarine_Construct_006ff800(void* self, void* arg0);
std::uint32_t DummySubmarineShot_IsInWater_True_006ff890(const void* self);
void DummySubmarine_DestructBody_006ff920(void* self);
void DummySubmarine_RegisterInWorldLists_006ff970(void* self);
void DummySubmarine_DetachFromWorldLists_006ff9c0(void* self);
void* DummySubmarineClass_CreateEntity_006fffe0(void* self, void* arg0);

// --- DummyTarget & DummyTargetVehicle (0x00700150..0x007016c0) ---
void DummyTargetBulletClass_ResolveLinkedClass_00700150(void* self);
std::uint32_t DummyTargetVehicleClass_ReconCategory13_00700200(const void* self);
void DummyTargetVehicleClass_ReconMarkDirty_00700250(
    void* self,
    void* recon_slot,
    std::uint32_t arg1,
    std::uint32_t arg2
);
void DummyTargetVehicle_Activate_00700260(void* self);
void LuaObject_ReadNumberFieldOr_00700290(
    const char* key_name,
    float* out_val,
    void* lua_table_by_val,
    float default_val
);
void LuaObject_ReadNumberField_00700330(
    const char* key_name,
    float* out_val,
    void* lua_table_by_val
);
void LuaObject_ReadNumberField_007003b0(
    const char* key_name,
    float* out_val,
    void* lua_table_by_val
);
void DummyTarget_Activate_00700520(void* self);
void* DummyTargetVehicle_Construct_00700590(void* self, void* arg0);
const char* DummyTargetVehicle_GetName_00700840(const void* self);
void DummyTargetVehicle_Serialize_00700870(void* self, void* archive_writer);
void* ObserverMember_SetTarget_00700940(void* self, void* target);
void DummyTargetVehicle_RegisterInWorldLists_00700980(void* self);
void DummyTargetVehicle_DetachFromWorldLists_007009c0(void* self);
void* DummyTarget_Construct_00700a20(void* self, void* arg0);
void DummyTarget_RegisterInWorldLists_00700b10(void* self);
void DummyTarget_DetachFromWorldLists_00700b60(void* self);
void* DummyTargetBulletClass_CreateEntity_00700dd0(void* self, void* arg0);
void* DummyTargetVehicle_Create_00701170(
    void* vehicle_class,
    void* arg0,
    const char* name_str,
    std::uint32_t pos_arg0,
    std::uint32_t pos_arg1,
    std::uint32_t orient_arg
);
bool DummyTargetVehicle_HitOnTheFly_00701250(void* self, void* hit_arg);
void DummyTargetVehicle_InitAll_007016c0(void* self);

}  // namespace bsp::dummy_depth_seg34_argon
