#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {
struct WeaponClassRowView;
struct WeaponClassLoadHost;
struct WeaponClassDescriptor;
struct WeaponClassFinaliseInput;
struct WeaponClassFinaliseResult;
}

namespace bsp::flak_note_seg36_37_argon {

// ============================================================================
// Segment 36 & 37 (0x0070bc30..0x0071b710) Reconstruction Declarations
// ============================================================================

// --- Segment 36: FlagModel (0x0070bc30..0x0070bfc0) ---
void* FlagModel_ScalarDelete_0070bc30(void* self, std::uint8_t flags);
void* FlagParams_Assign_0070bc50(void* self, const void* src);
void FlagModel_BuildGeometry_0070bcb0(void* self);
void FlagModel_Setup_0070bee0(void* self, const void* params);
void* FlagModel_Construct_0070bfc0(void* self, void* arg0, const void* params);

// --- Segment 36: FlakBullet / FlakProjectile (0x0070c030..0x0070cea0) ---
// 0070C030: original thiscall(ECX=class, stack=Lua row), RET4. Typed
// projection of the SAME descriptor and row; not a native ABI replacement.
// Chains 006E8770, clears +D4h, then plain GetInteger(MinRange) -> +58h.
void FlakBulletClass_ReadLuaFields_0070c030(const WeaponClassRowView&,
    WeaponClassLoadHost&, WeaponClassDescriptor&);
// Original thiscall(ECX=class), RET0: call the base, then FLD/FDIV/FSTP
// +58h/+50h -> +D8h when base AL==1, even when its +64h latch was set.
WeaponClassFinaliseResult FlakBulletClass_DeriveEngagementRange_0070c0b0(
    const WeaponClassFinaliseInput&, bool already_finalised) noexcept;
void FlakBullet_WriteState_0070c0f0(void* self, void* stream);
void* Entity_AsKind05_0070c1f0(void* self);
void FlakProjectile_TickAdvance_0070c370(void* self, float dt);
void* FlakProjectile_Construct_0070cae0(void* self);
std::uint32_t MFlakBullet_IsKindOf_0070cb80(const void* self, std::uint32_t kind_id);
void* FlakBulletClass_CreateProjectile_0070cc30(
    void* self,
    void* out_ref,
    std::uint32_t owner_id,
    const float* start_pos,
    const float* direction,
    std::uint8_t activate_flag,
    std::uint32_t unused_arg,
    const float* speed_scale_pair
);
void FlakBullet_Activate_0070cea0(void* self);

// --- Segment 36: UnitGroup / ShipGroup / ObserverMember (0x0070d1b0..0x0070ecf0) ---
float Unit_ClassMaxSpeed_0070d1b0(void* self);
// Borrowed state views for the existing bodies; raw entries below delegate to
// these same algorithms. Names describe recovered offsets, not native symbols.
class UnitClassMaxSpeedView {
public:
    virtual ~UnitClassMaxSpeedView() = default;
    virtual bool is_kind_6() const = 0;
    virtual float ship_class_speed_0500() const = 0;
    virtual float squadron_plane_class_speed_0188() const = 0;
};
float Unit_ClassMaxSpeed_0070d1b0(const UnitClassMaxSpeedView& unit);

class UnitGroupSpeedCeilingView {
public:
    virtual ~UnitGroupSpeedCeilingView() = default;
    virtual float seed_00cfd6f4() const = 0;
    virtual float unset_00cf4888() const = 0;
    virtual std::int32_t member_count_04f8() const = 0;
    virtual bool member_present(std::int32_t index) const = 0;
    virtual float member_class_speed(std::int32_t index) const = 0;
    virtual void set_missing_member_speed_30(std::int32_t index, float value) = 0;
    virtual float ceiling_0504() const = 0;
    virtual void set_ceiling_0504(float value) = 0;
};
void UnitGroup_InitWithLeader_0070d7b0(void* self, void* leader);
void UnitGroup_RefreshSpeedCeiling_0070da00(void* self);
void UnitGroup_RefreshSpeedCeiling_0070da00(UnitGroupSpeedCeilingView& group);
void* UnitGroup_CreateForLeader_0070db20(void* leader);
void UnitGroup_SwapSlotsByDistance_0070db60(void* self);
void* Entity_AsKind06_0070e3a0(void* self);
float UnitGroup_GetMaxFollowerTurnRadius_0070e3c0(void* self);
// [0x0070e3c0] typed view overload (wire goal 2 chain 71); the raw entry above wraps it.
// View supplies, by the image's offsets: seed_00ce3d08() (the float at 00CE3D08),
// member_count_4f8() (read once, as the raw entry does), member_18(i) (the record at
// +18h + i*34h, a View::Member), member_present(m) (non-null), member_is_kind_6(m)
// (vtable[5Ch](6)), member_is_leader_14(m) (equal to [group+14h]) and
// member_turn_radius_0082e850(m) (0082E850 on [m+538h]). A present member answering
// kind 6 that is not the leader raises the result when !(max > r).
template <class View>
float UnitGroup_GetMaxFollowerTurnRadius_0070e3c0(const View& group) {
    float max_r = group.seed_00ce3d08();
    const std::int32_t count = group.member_count_4f8();
    for (std::int32_t i = 0; i < count; ++i) {
        const auto member = group.member_18(i);
        if (group.member_present(member)) {
            if (group.member_is_kind_6(member) && !group.member_is_leader_14(member)) {
                const float r = group.member_turn_radius_0082e850(member);
                if (!(max_r > r)) {
                    max_r = r;
                }
            }
        }
    }
    return max_r;
}
std::int32_t ShipGroup_ControllerAreaKey_0070e450(void* self);
void UnitGroup_DetachMember_0070e4c0(void* self, void* member, std::uint32_t arg1);
void UnitGroup_ReadSceneFields_0070e620(void* self, void* reader);
void ObserverMember_WriteRef_0070e890(void* self, void* writer);
bool ObserverMember_TargetWithinRange_0070eb00(void* self, float max_dist);
void UnitGroup_RemoveEntity_0070eca0(void* self, void* entity);
void* UnitGroup_CreateFromScene_0070ecf0(void* reader);

// --- Segment 36: UnitPart Collision / LogicalVertexStream / Containers (0x0070f090..0x007114a0) ---
bool UnitPartNode_SphereOverlapForward_0070f090(
    void* self,
    const void* center,
    float radius,
    const void* filter,
    void* out_list
);
void* STL_PartGroupRows_Allocate_0070f3d0(std::uint32_t count);
void* UnitPartCollisionShape_ConstructWithMesh_0070f6b0(
    void* self,
    void* part_node,
    void* mesh_shape,
    std::uint32_t user_word
);
std::uint32_t CollisionShapeD768_SegmentSlot_0070f710(void* self, void* segment_query);
bool UnitPartCollisionShape_TestSphere_0070f720(
    void* self,
    const void* center,
    float radius,
    void* out_result
);
std::int32_t STL_LegacySboString_FindFirstOf_0070f900(
    const void* self,
    const char* needle,
    std::uint32_t start_pos,
    std::uint32_t needle_len
);
bool LogicalVertexStream_ReadNormal_0070fdb0(void* self, float* out_xyz);
bool LogicalVertexStream_ReadUV_007100a0(void* self, float* out_uv);
void UnitPartEffectPointers_PushBack_00710360(void* self, const void* elem_ptr);
void UnitPartEntryPointers_PushBack_007103e0(void* self, const void* elem_ptr);
void STL_List_Clear_007104e0(void* self);
void STL_List_Clear_00710530(void* self);
void* RefPtr_Release_00710630(void* self);
void* RefPtr_Release_007106b0(void* self);
void* STL_MoveDwords_00710700(void* self, const void* first, const void* last, void* dst);
void UnitPartEffectPointers_PushBack_007107e0(void* self, const void* elem_ptr);
void UnitPartEntryPointers_PushBack_00710820(void* self, const void* elem_ptr);
void* RefPtr_Assign_007109a0(void* self, void* new_ptr);
void* STL_FindDword_007109e0(
    void* out_iter,
    const std::uint32_t* key_ptr,
    std::uint32_t container_tag,
    std::uint32_t* first,
    std::uint32_t unused_tag,
    std::uint32_t* last
);
void RefPtr_Release_00710a60(void* self);
void UnitPartEffect_AttachAtPart_00710e10(void* self, void* part_ref);
void* STL_List_Construct_UnitParts_00710f70(void* self);
void* STL_List_Construct_UnitPartShapes_00710fa0(void* self);
void UnitPartEffect_StartByIndex_00711210(void* self, std::int32_t index, void* effect_arg);
void STL_Vector_Tidy_007114a0(void* self);

// --- Segment 37: PartGroupRows / ResourceItem Note, Aux, ZoneDesc / Model Notes (0x00712100..0x0071b3e0) ---
void* STL_Vector_Assign_00712100(void* self, const void* rhs);
void STL_ConstructDwordVector_00712270(void* dst, const void* src);
void STL_PartGroupRows_DestroyRange_Thunk_00712330(void* self, void* first, void* last);
void STL_ConstructDwordVector_007123c0(void* dst, const void* src);
void* STL_PartGroupRows_AssignForward_00712900(void* first, void* last, void* dst);
void STL_PartGroupRows_AssignRepeated_00712940(void* first, void* last, const void* value);
void STL_PartGroupRows_UninitializedFill_00712970(
    void* dst,
    std::uint32_t count,
    const void* value,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
);
void* STL_PartGroupRows_TransferConstruct_00712a10(
    void* first,
    void* last,
    void* dst,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
);
void STL_SwapBackward_Vector16_00712af0(void* first, void* last, void* dst_last);
void* STL_PartGroupRows_UninitializedFillAdapter_00712f20(
    void* self,
    void* dst,
    std::uint32_t count,
    const void* value
);
void* STL_PartGroupRows_TransferAdapter_00712f60(
    void* self,
    void* first,
    void* last,
    void* dst
);
void* STL_PartGroupRows_BackwardTransferAdapter_00712f90(
    void* self,
    void* first,
    void* last,
    void* dst_last
);
void STL_PartGroupRows_InsertCount_00712ff0(
    void* self,
    std::uint32_t pos_owner,
    void* pos_ptr,
    std::uint32_t count,
    const void* value
);
void STL_PartGroupRows_Resize_00713270(
    void* self,
    std::uint32_t new_size,
    std::uint32_t val_w0,
    void* val_first,
    void* val_last,
    void* val_end
);
std::uint32_t ResourceItemNote_GetRuntimeTypeToken_00713750();
std::uint32_t ResourceItemNote_GetRuntimeTypeNameAddress_00713760();
std::uint32_t ResourceItemAux_GetRuntimeTypeToken_00713790();
std::uint32_t ResourceItemAux_GetRuntimeTypeNameAddress_007137a0();
std::uint32_t ResourceItemZoneDesc_GetRuntimeTypeToken_007137d0();
std::uint32_t ResourceItemZoneDesc_GetRuntimeTypeNameAddress_007137e0();
void* GameResource_AsType44_00714030(void* self);
void* GameResource_AsType54_00714060(void* self);
void* ResourceItemZoneDesc_Cast_00714090(void* self);
void* ResourceItemAux_Cast_007140c0(void* self);
void* ResourceItemNote_Cast_007140f0(void* self);
void* Matrix4f_InvertInto_007149b0(void* self);
void* STL_UninitCopy8_00715af0(
    const void* first,
    const void* last,
    void* dst,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
);
void* STL_UninitCopy8_00715b20(
    const void* first,
    const void* last,
    void* dst,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
);
void* STL_UninitCopy8_00715b50(
    const void* first,
    const void* last,
    void* dst,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
);
void* STL_UninitCopy8_00715b80(
    const void* first,
    const void* last,
    void* dst,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
);
void* STL_UninitCopy8_00715bb0(
    const void* first,
    const void* last,
    void* dst,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
);
void STL_Fill8_00716960(void* first, void* last, const void* value);
void STL_Fill8_00716990(void* first, void* last, const void* value);
void STL_Fill8_007169c0(void* first, void* last, const void* value);
void STL_Fill8_007169f0(void* first, void* last, const void* value);
void STL_Fill8_00716a20(void* first, void* last, const void* value);
void STL_UninitFillN8_00716b50(
    void* dst,
    std::uint32_t count,
    const void* value,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
);
void STL_UninitFillN8_00716b80(
    void* dst,
    std::uint32_t count,
    const void* value,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
);
void STL_UninitFillN8_00716bb0(
    void* dst,
    std::uint32_t count,
    const void* value,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
);
void STL_UninitFillN8_00716be0(
    void* dst,
    std::uint32_t count,
    const void* value,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
);
void STL_UninitFillN8_00716c10(
    void* dst,
    std::uint32_t count,
    const void* value,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
);
void STL_Vector_Tidy_00717510(void* self);
void STL_Vector_Tidy_00717570(void* self);
void STL_Vector_Tidy_007175a0(void* self);
void* STL_CopyBackward8_00717880(const void* first, const void* last, void* dst_last);
void* STL_CopyBackward8_007178c0(const void* first, const void* last, void* dst_last);
void* STL_CopyBackward8_00717900(const void* first, const void* last, void* dst_last);
void* STL_CopyBackward8_00717940(const void* first, const void* last, void* dst_last);
void* STL_CopyBackward8_00717980(const void* first, const void* last, void* dst_last);
void* ResourceItemNote_Construct_00717c70(void* self);
bool ResourceItemNote_AcceptsRuntimeTypeToken_00717ca0(const void* self, std::uint32_t token);
void ResourceItemNote_Destruct_00717cd0(void* self);
void STL_String_Tidy_00717d50(void* self);
void* ResourceItemZoneDesc_Construct_00717da0(void* self);
bool ResourceItemZoneDesc_AcceptsRuntimeTypeToken_00717dd0(const void* self, std::uint32_t token);
void ResourceItemZoneDesc_Destruct_00717e00(void* self);
void STL_String_Tidy_00717ef0(void* self);
void STL_Vector_Tidy_007180e0(void* self);
void STL_Vector_Tidy_00718110(void* self);
void STL_Vector_Tidy_00718140(void* self);
void STL_Vector_Tidy_007185f0(void* self);
void STL_Vector_Tidy_00718620(void* self);
void STL_Vector_Tidy_00718650(void* self);
void STL_Vector_Tidy_00718680(void* self);
void STL_Vector_Tidy_007186b0(void* self);
void* ResourceItemNote_ScalarDelete_00718710(void* self, std::uint8_t flags);
void STL_Vector_Tidy_00718a40(void* self);
void STL_Vector_Tidy_00718a70(void* self);
void STL_Vector_Tidy_00718aa0(void* self);
void STL_Vector_Tidy_00718ad0(void* self);
void STL_Vector_Tidy_00718b00(void* self);
bool ModelNoteNameEquals_Predicate_00718c70(const void* self, const void* element_pair);
void* StructuredNode_ReadCharacterString_00718f50(void* node_handle, void* dst_string);
void* NoteResourceParser_Parse_00719000(void* self, void* node_handle);
void ZoneDescResourceParser_ReadItem_00719080(void* self, void* node_handle);
void* ZoneDescResourceParser_CreateItem_00719240(void* self, void* node_handle);
void STL_Vector_InsertN8_007192c0(
    void* self,
    std::uint32_t pos_owner,
    void* pos_ptr,
    std::uint32_t count,
    const void* value
);
void STL_Vector_InsertN8_00719510(
    void* self,
    std::uint32_t pos_owner,
    void* pos_ptr,
    std::uint32_t count,
    const void* value
);
void STL_Vector_InsertN8_00719760(
    void* self,
    std::uint32_t pos_owner,
    void* pos_ptr,
    std::uint32_t count,
    const void* value
);
void STL_Vector_InsertN8_007199b0(
    void* self,
    std::uint32_t pos_owner,
    void* pos_ptr,
    std::uint32_t count,
    const void* value
);
void STL_Vector_InsertN8_00719c00(
    void* self,
    std::uint32_t pos_owner,
    void* pos_ptr,
    std::uint32_t count,
    const void* value
);
void* ModelNoteList_FindIfNameEquals_00719fa0(
    void* first,
    void* last,
    void* predicate_sbo_string_by_value
);
void* ModelNoteList_FindByName_0071aae0(
    void* out_iter,
    void* first_owner,
    void* first_ptr,
    void* last_owner,
    void* last_ptr,
    const void* key_string
);
void* ResourceItemAux_Construct_0071abb0(void* self);
bool ResourceItemAux_AcceptsRuntimeTypeToken_0071ac10(const void* self, std::uint32_t token);
void ResourceItemAux_Destruct_0071ac40(void* self);
void* Model_FindNoteNode_0071ad50(void* self, const char* key);
void GameResource_ReadPointsItem_0071b3e0(void* self, void* node_handle);

}  // namespace bsp::flak_note_seg36_37_argon
