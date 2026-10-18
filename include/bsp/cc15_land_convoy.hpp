#pragma once

#include <cstddef>
#include <cstdint>

// Land convoy bodies 007420B0, 00742180 and 007422A0 (packet cc15-rD1). New
// C++ interfaces with host seams; names are hypotheses, not recovered symbols.
// docs/CC15_LAND_CONVOY.md records the evidence, original ABIs, limits and the
// mission reach list. No host is wired to any of these.
//
// Native layout the three bodies read (LandConvoy vtable 00CEA570: word
// 00CEA5F0 = slot +080h = 00742180, word 00CEA610 = slot +0A0h = 007420B0):
//   +030h  entity registry pointer (ECX of 00925A90)
//   +070h  kill cause, written by 00926D90 on the convoy
//   +0C0h  scene property bag: +4h state word, +8h property records
//   +344h  path interface stored by 007420B0
//   +394h  member vector (VC9 checked std::vector<LandVehicle*>):
//          +398h first, +39Ch last; slots are nulled in place, never erased
//   +3BCh  live member count
// Member (land vehicle, vtable 00CFFDE0):
//   +738h  owning convoy, cleared by 00742180; read by 0074CD10
//   vtable +128h = 006D1EB0 BSP_Unit_SetSkillLevel_Forward
namespace bsp::cc15_land_convoy {

inline constexpr std::uint32_t kConvoyRegistry30 = 0x030u;
inline constexpr std::uint32_t kConvoyKillCause70 = 0x070u;
inline constexpr std::uint32_t kConvoyPropertyBagC0 = 0x0C0u;
inline constexpr std::uint32_t kConvoyPathInterface344 = 0x344u;
inline constexpr std::uint32_t kConvoyMemberVector394 = 0x394u;
inline constexpr std::uint32_t kConvoyLiveCount3BC = 0x3BCu;
inline constexpr std::uint32_t kMemberConvoy738 = 0x738u;
// The cause 007422A0 passes when the live count is zero (0074231C PUSH 2).
inline constexpr std::int32_t kConvoyEmptiedKillCause = 2;
// "Path", the key string at 00CEA738 that 007420B0 pushes for 008F2260.
inline constexpr std::uint32_t kPathKeyAddress = 0x00CEA738u;
inline constexpr const char kPathKey[] = "Path";

// ---------------------------------------------------------------------------
// Member walk seam shared by all three bodies.
// ---------------------------------------------------------------------------
// The image walks [first, last) of the vector at +394h with a slot pointer
// fixed at first on entry and last re-read at every check. Here the slot
// pointer is an index i and "last" is member_slot_count(), called at every
// point the image re-reads +39Ch. The index form matches the image as long as
// first does not move during the walk (no reallocation); the image keeps its
// stale pointer if it does. The checked-iterator asserts become
// invalid_parameter_noinfo_00bf6713() calls at the same places: the image
// reports and carries on (each CALL 00BF6713 falls through).
struct LandConvoyMemberSlots {
    virtual ~LandConvoyMemberSlots() = default;
    // (last - first) / 4 of the vector at convoy+394h.
    virtual std::size_t member_slot_count(std::uint8_t* convoy) = 0;
    // The member word in slot i (may be null: slots are nulled in place).
    virtual std::uint8_t* member_slot(std::uint8_t* convoy, std::size_t i) = 0;
    // 00BF6713 _invalid_parameter_noinfo (LIBCRT_unmatched_00bf6713), RET.
    virtual void invalid_parameter_noinfo_00bf6713() = 0;
};

// ---------------------------------------------------------------------------
// 00742180 BSP_LandConvoy_DisbandAndKillMembers (LandConvoy vtable +080h)
// ---------------------------------------------------------------------------
// __fastcall(ECX = convoy), no stack arguments, no x87. Ends with
// JMP 00779AF0 at 00742209 after restoring EBX/EBP/ESI/EDI, ECX = convoy:
// a tail call, so 00779AF0's RET is the caller's return.
struct LandConvoyKillHost : LandConvoyMemberSlots {
    // MOV dword [member+738h], 0 (007421D5).
    virtual void clear_member_convoy_738(std::uint8_t* member) = 0;
    // MOV EBX, [convoy+70h] (007421E2), re-read for every member.
    virtual std::int32_t convoy_kill_cause_70(std::uint8_t* convoy) = 0;
    // 00926D90 BSP_MissionEntity_Kill, __thiscall(ECX = entity, int cause),
    // RET 4. Returns at once when entity+5Fh is set; otherwise queues the
    // entity on the kill list 00F899B4 (vtable +080h runs on a later flush).
    virtual void mission_entity_kill_00926d90(std::uint8_t* entity, std::int32_t cause) = 0;
    // 00779AF0 BSP_MissionEntity_OnKilledPlayerUnit, __fastcall(ECX =
    // entity), reached by the tail JMP. Source:
    // native_unit_on_killed_00779af0, src/native_unit_killed_tail.cpp.
    virtual void mission_entity_on_killed_00779af0(std::uint8_t* convoy) = 0;
};

void land_convoy_kill_members_00742180(std::uint8_t* convoy, LandConvoyKillHost& host);

// ---------------------------------------------------------------------------
// 007422A0 BSP_LandConvoy_OnMemberDeath
// ---------------------------------------------------------------------------
// __thiscall(ECX = convoy, stack member), RET 4, no x87. The only caller is
// 0074CD10 (member vtable +080h) at 0074CD1E, when member+738h is non-null.
struct LandConvoyRemoveHost : LandConvoyMemberSlots {
    // MOV dword [slot], 0 (00742305).
    virtual void null_member_slot(std::uint8_t* convoy, std::size_t i) = 0;
    // [convoy+3BCh]: ADD -1 at 0074230B (read-modify-write), CMP 0 at 00742312.
    virtual std::int32_t live_count_3bc(std::uint8_t* convoy) = 0;
    virtual void set_live_count_3bc(std::uint8_t* convoy, std::int32_t count) = 0;
    // 00926D90, as in LandConvoyKillHost.
    virtual void mission_entity_kill_00926d90(std::uint8_t* entity, std::int32_t cause) = 0;
};

// Returns the index of the slot it nulled, or kMemberNotFound. The image
// returns nothing meaningful (0074CD10 ignores EAX); the result is for host
// bookkeeping only. The zero-count kill runs on both paths.
inline constexpr std::size_t kMemberNotFound = static_cast<std::size_t>(-1);
std::size_t land_convoy_remove_member_007422a0(std::uint8_t* convoy, std::uint8_t* member,
                                              LandConvoyRemoveHost& host);

// ---------------------------------------------------------------------------
// 007420B0 BSP_LandConvoy_ResolvePathReference (LandConvoy vtable +0A0h)
// ---------------------------------------------------------------------------
// __fastcall(ECX = convoy), RET, no x87 (00927A80's result comes back in EAX).
struct LandConvoyBindHost : LandConvoyMemberSlots {
    // 0077F0E0 BSP_UnitOwnerEntity_ActivateAndRegister, __fastcall(ECX =
    // convoy) (007420B3). Its EAX is not used.
    virtual void unit_owner_activate_and_register_0077f0e0(std::uint8_t* convoy) = 0;
    // MOV EAX, [convoy+C0h] (007420B8).
    virtual void* property_bag_c0(std::uint8_t* convoy) = 0;
    // [bag+4h] (007420C6) and [bag+8h] (007420D2).
    virtual std::int32_t property_bag_state(void* bag) = 0;
    virtual void* property_bag_records(void* bag) = 0;
    // 008F2260 BSP_ScenePropertyBag_Find, __thiscall(ECX = records, key), RET 4.
    virtual void* scene_property_bag_find_008f2260(void* records, const char* key) = 0;
    // 008F0DF0 BSP_SceneProperty_GetStringValue, __fastcall(ECX = property).
    // 007420E1 passes 008F2260's result without a null test.
    virtual const char* scene_property_string_008f0df0(void* property) = 0;
    // MOV ECX, [convoy+30h] (007420E8).
    virtual void* entity_registry_30(std::uint8_t* convoy) = 0;
    // 00925A90 BSP_EntityRegistry_FindEntityByName, __thiscall(ECX = registry,
    // name), RET 4; entity or 0.
    virtual void* entity_registry_find_by_name_00925a90(void* registry, const char* name) = 0;
    // 007AC9D0 BSP_Entity_PathInterfaceForKind, __fastcall(ECX = entity).
    virtual void* entity_path_interface_for_kind_007ac9d0(void* entity) = 0;
    // MOV [convoy+344h], EAX (00742104).
    virtual void set_path_interface_344(std::uint8_t* convoy, void* path_interface) = 0;
    // 00927A80 BSP_PropertyBag_GetSkill, __thiscall(ECX = convoy) -> EAX (0074210A).
    virtual std::int32_t property_bag_get_skill_00927a80(std::uint8_t* convoy) = 0;
    // member->vtable[128h](skill), __thiscall(ECX = member, int), stack
    // balanced by the callee (no ADD ESP at 00742165). Land vehicle vtable
    // 00CFFDE0 holds 006D1EB0 BSP_Unit_SetSkillLevel_Forward there.
    virtual void member_set_skill_vt128(std::uint8_t* member, std::int32_t skill) = 0;
};

void land_convoy_bind_motion_007420b0(std::uint8_t* convoy, LandConvoyBindHost& host);

} // namespace bsp::cc15_land_convoy
