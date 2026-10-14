#pragma once
// Segment 42 (00758F90-0076A970) session-message classes, cc13-seg42.
// Every class shares the 18h NativeSessionMessageStorage head (vtable +0,
// delivery +4, +8/+0Ch, type byte +10h, selected local slot record +14h).
// Names come from the MT_* table 00E0AB68 the Ghidra names cite; they are
// hypotheses, not recovered symbols. Field names describe offset and storage.
// Evidence, ABI and the per-function table: docs/CC13_SEG42.md.
//
// Native ABIs (checked in the listings):
//   Construct         __thiscall(ECX this), base 0075B430 inlined, returns
//                     this in EAX. Camera 00764960 is __thiscall RET 8.
//   ConstructForRead  __thiscall(ECX this): zeroes +8/+0Ch/+14h/+10h and the
//                     header without reading the session (no 0075B430 call).
//   Write             __thiscall(ECX this, cursor) RET 4; the cursor goes to
//                     the bit writers as ECX unchanged.
//   Read              __thiscall(ECX this, stream) RET 4; the bit readers get
//                     stream+4 (NativeSessionReadStream::cursor_04).
//   IsType            RET 4; most ignore ECX and test the argument against the
//                     class type and its category types (46h always, plus
//                     48h/49h/4Ah); four compare against this->type_10.
// The vtable store is a named host binding: the image address is passed to
// VtableBinding at the point the native stores it. The slot functions of
// most of these tables lie outside this segment.
#include <cstdint>
#include <cstddef>

#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_bit_cursor_numeric_array.hpp"
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_session_message_tags_04_05.hpp"
#include "bsp/scenes_seg1_cc13.hpp"

namespace bsp {
struct NativeStringRawPoolContext;
}

namespace bsp::seg42 {

// Codec scales the writers load from .rdata.
inline constexpr std::uint32_t kFltMaxBits_00d7a248 = 0x7F7FFFFFu;  // FLT_MAX: raw 32-bit path
inline constexpr std::uint32_t kOneBits = 0x3F800000u;              // immediate 1.0f
inline constexpr std::uint32_t kScale_00d032e4 = 0x40C9999Au;       // 6.3f
// ConstructForRead seeds (00CE3958 = 2.0f, 00CE7D7C = -2.0f, 00D7A24C = 1.0f).
inline constexpr std::uint32_t kTwoBits_00ce3958 = 0x40000000u;
inline constexpr std::uint32_t kMinusTwoBits_00ce7d7c = 0xC0000000u;
inline constexpr std::uint32_t kOneBits_00d7a24c = 0x3F800000u;

struct VtableBinding {
    virtual ~VtableBinding() = default;
    // The profile the host uses for the native vtable at image_address.
    virtual const std::uint32_t* vtable(std::uint32_t image_address) const = 0;
};

struct Context {
    void* volatile& current_game_00e188a8;  // read by the inlined 0075B430
    const VtableBinding& vtables;
};

// ---- shared prefixes ---------------------------------------------------
// Entity-addressed messages: 0075B480/0075B4C0 header (id 12 bits, relay bool).
struct HeaderedMessage {
    NativeSessionMessageStorage base;
    std::uint16_t id_18;
    std::uint8_t relay_1a;
    std::uint8_t pad_1b;
};
static_assert(sizeof(HeaderedMessage) == 0x1C);
// Category-49h messages add a bool at +1Ch that every writer sends.
struct UnitMessage {
    HeaderedMessage h;
    std::uint8_t flag_1c;
    std::uint8_t pad_1d[3];
};
static_assert(sizeof(UnitMessage) == 0x20);

// ---- full classes ------------------------------------------------------
struct FormationMemberMessage { UnitMessage u; std::uint16_t member_id_20; };  // 76h JOIN, 77h LEAVE
struct FormationSetMessage { UnitMessage u; std::uint8_t opaque_20[8]; std::uint32_t zero_28, zero_2c; };
struct ShipHelmsmanControlMessage {  // 8Eh
    HeaderedMessage h;
    float value_1c, value_20;
    std::uint32_t zero_24;
    float max_28, min_2c, max_30, min_34;
    std::uint8_t flag_38;
    std::uint8_t pad_39[3];
};
static_assert(offsetof(ShipHelmsmanControlMessage, flag_38) == 0x38);
struct PlaneSyncMessage {  // C1h: only the ConstructForRead stores are modelled
    HeaderedMessage h;
    std::uint32_t zero_1c;
    std::uint8_t opaque_20[0x18];
    std::uint32_t zero_38, zero_3c, zero_40;
    float one_44;
    std::uint32_t zero_48, zero_4c;
    std::uint8_t opaque_50[0x24];
    std::uint8_t zero_74, zero_75, zero_76, opaque_77;
    std::uint32_t zero_78, zero_7c;
    std::uint8_t opaque_80[0x1d];
    std::uint8_t zero_9d;
};
static_assert(offsetof(PlaneSyncMessage, one_44) == 0x44);
static_assert(offsetof(PlaneSyncMessage, zero_78) == 0x78);
static_assert(offsetof(PlaneSyncMessage, zero_9d) == 0x9D);
struct PlaneControlFactorMessage { UnitMessage u; float factor_20[3]; float one_2c[4]; };  // C9h
struct PlaneHitMessage { UnitMessage u; std::uint32_t kind_20; float point_24[3]; float vector_30[3]; };  // CAh
struct PlanePcmdLastTimeMessage { UnitMessage u; float time_20; std::int32_t command_24; };
struct DipRiseSyncMessage { UnitMessage u; std::uint32_t state_20; };  // A2h
struct LandfortEffectMessage { HeaderedMessage h; std::uint32_t zero_1c, zero_20; };
struct StartLandingMessage { HeaderedMessage h; std::uint16_t zero_1c, zero_1e; };
struct SpawnPointSetActiveMessage { HeaderedMessage h; std::uint32_t zero_1c; std::uint8_t active_20; };
struct MovieAddPositionMessage {  // 33h, no entity header
    NativeSessionMessageStorage base;
    void* keyframe_18;
    std::uint8_t owns_keyframe_1c;
};
static_assert(offsetof(MovieAddPositionMessage, owns_keyframe_1c) == 0x1C);
struct EntityRelocateMessage {  // 97h
    UnitMessage u;
    std::uint8_t has_heading_20, has_position_21, pad_22[2];
    float heading_24;
    std::int32_t value_28;
    float position_2c[3];
};
static_assert(offsetof(EntityRelocateMessage, position_2c) == 0x2C);
struct PlayerInfoMessage { NativeSessionMessageStorage base; std::uint32_t zero_18, zero_1c, zero_20; std::int32_t minus_one_24; };
struct MusicSetLevelMessage { NativeSessionMessageStorage base; std::uint32_t level_18; };  // 2Ah
struct BlackoutMessage { NativeSessionMessageStorage base; float value_18, value_1c; };  // 2Bh
struct HandModeTargetedMessage { NativeSessionMessageStorage base; std::uint16_t id_18; std::uint8_t flag_1a, flag_1b; };  // 2Dh
struct RefPosPolarMessage {  // 2Eh
    NativeSessionMessageStorage base;
    std::uint16_t id_18; std::uint8_t pad_1a[2];
    float value_1c, value_20, value_24;
    std::uint8_t flag_28, pad_29[3];
    float value_2c;
};
static_assert(offsetof(RefPosPolarMessage, value_2c) == 0x2C);
struct RefPosRadiusMessage {  // 2Fh
    NativeSessionMessageStorage base;
    std::uint16_t id_18; std::uint8_t pad_1a[2];
    float value_1c;
    std::uint8_t flag_20, pad_21[3];
    float value_24;
};
static_assert(offsetof(RefPosRadiusMessage, value_24) == 0x24);
struct PathWalkPropertyMessage {  // 30h
    NativeSessionMessageStorage base;
    float vector_18[3];
    float value_24;
    std::uint8_t flag_28, pad_29[3];
    float value_2c;
};
static_assert(offsetof(PathWalkPropertyMessage, value_2c) == 0x2C);
struct AddInterpolatorMessage { NativeSessionMessageStorage base; std::uint32_t kind_18; float value_1c[4]; };  // 31h
struct SetDeviationFactorMessage { NativeSessionMessageStorage base; float factor_18; };  // 32h
struct VoiceChangeMessage {  // E8h
    NativeSessionMessageStorage base;
    std::uint8_t flag_18, pad_19[7];
    std::uint32_t qword_20[2];
};
static_assert(offsetof(VoiceChangeMessage, qword_20) == 0x20);
struct ServerDlcUnlockMessage { NativeSessionMessageStorage base; std::uint16_t words_18[0xff]; };  // DEh
static_assert(sizeof(ServerDlcUnlockMessage) == 0x218);
struct TGunTurnSyncMessage {  // AEh: constructor stores only
    HeaderedMessage h;
    std::uint8_t zero_1c, zero_1d, pad_1e[2];
    std::uint32_t zero_20[5];
    std::uint8_t opaque_34[0xc];
    std::uint32_t zero_40;
};
static_assert(offsetof(TGunTurnSyncMessage, zero_40) == 0x40);
struct VehicleGunControlMessage {  // 79h
    HeaderedMessage h;
    std::uint32_t mode_1c;
    float vector_20[3];
    float angle_2c, angle_30;
    std::uint8_t flag_34, flag_35, has_target_36, pad_37;
    std::uint16_t target_id_38;
};
static_assert(offsetof(VehicleGunControlMessage, target_id_38) == 0x38);
struct VehicleValueMessage { HeaderedMessage h; std::uint32_t value_1c; };  // 7Fh party, 80h race
struct VehicleShipyardLaunchMessage { UnitMessage u; std::uint8_t flag_20; };  // 7Ah
struct HitMarkerMessage { NativeSessionMessageStorage base; std::uint8_t flag_18, flag_19; };  // E5h
struct HitWarningMessage { NativeSessionMessageStorage base; std::uint32_t value_18; };  // E6h
struct RepairAddDamageMessage {  // 9Eh
    HeaderedMessage h;
    float amount_1c;
    std::uint8_t flag_20, pad_21[3];
    std::int32_t section_24;
};
struct RepairBodyMessage { HeaderedMessage h; std::uint8_t flag_1c; };  // 9Fh
struct LandingShipsLaunchedMessage { HeaderedMessage h; float value_1c; };  // 95h
struct TorpedoGenerateMessage { HeaderedMessage h; std::int32_t side_1c; float value_20, value_24, value_28, value_2c; };  // 9Dh
struct SetTorpedoStockMessage { UnitMessage u; std::uint32_t stock_20; };  // 96h
struct ExplodeOnePartMessage { UnitMessage u; std::uint32_t part_20; };  // 99h
struct SectionFailureMessage { UnitMessage u; std::uint32_t section_20, effect_24; std::uint8_t flag_28; };  // 9Bh
struct ShipLeakMessage { HeaderedMessage h; std::uint32_t leak_1c; float vector_20[3]; };  // 90h
struct ShipAddTorqueMessage { HeaderedMessage h; float torque_1c[3]; };  // 93h
struct ShipAvoidSideMessage { HeaderedMessage h; std::uint16_t other_id_1c, pad_1e; std::int32_t side_20; };  // 8Fh
struct PlaneSyncAckMessage { NativeSessionMessageStorage base; std::uint32_t count_18; std::uint32_t ids_1c[0x7f]; };  // C2h
struct PlaneChangeStateMessage { UnitMessage u; std::uint8_t state_20, state_21; };  // C3h
struct PlaneDisappearMessage { UnitMessage u; std::uint16_t id_20; };  // C7h
struct PlaneDeadmeatRotMessage { UnitMessage u; float rotation_20[3]; };  // C8h
struct AirbaseLaunchSlotMessage { UnitMessage u; std::uint32_t slot_20; std::uint8_t flag_24; };  // 83h
struct ParatrooperLandingPosMessage { UnitMessage u; float position_20[3]; };  // 71h
struct ShipyardSpawnMessage { UnitMessage u; std::uint32_t value_20, value_24; std::uint16_t target_id_28; };  // AAh
static_assert(offsetof(ShipyardSpawnMessage, target_id_28) == 0x28);
struct SpawnBannMessage { NativeSessionMessageStorage base; std::uint8_t flag_18; };  // D7h
struct CameraMessage {  // 55h
    NativeSessionMessageStorage base;
    float position_18[3];
    float direction_24[3];
    float fov_30;
    std::uint8_t flag_34;
};
static_assert(offsetof(CameraMessage, flag_34) == 0x34);
// 8-byte native string header (DWORD length, char* data), as 0041DD40 uses.
struct NativeStringHeader { std::uint32_t length; char* data; };
struct ManageMessagesMessage { NativeSessionMessageStorage base; NativeStringHeader text_18; std::uint32_t value_20, value_24; };  // E2h
struct VoiceOverMessage { NativeSessionMessageStorage base; NativeStringHeader text_18; };  // E7h
struct ManageDialogsMessage { NativeSessionMessageStorage base; std::uint32_t zero_18, zero_1c, opaque_20, zero_24, zero_28; };  // E1h
struct CountdownMessage {  // 2Ch
    NativeSessionMessageStorage base;
    NativeStringHeader text_18;
    float value_20, value_24;
    std::uint8_t stop_28;
};
static_assert(offsetof(CountdownMessage, stop_28) == 0x28);
struct MetricsSessionMessage { NativeSessionMessageStorage base; std::uint32_t zero_18, zero_1c; };  // E9h
struct InferiorListMessage { HeaderedMessage h; std::uint32_t zero_1c, zero_20, zero_24; };  // D6h
struct SessionManagerMessage { NativeSessionMessageStorage base; std::uint8_t opaque_18[8]; void* list_head_20; std::uint32_t list_size_24; };  // E4h
static_assert(offsetof(SessionManagerMessage, list_head_20) == 0x20);
// 29h: the payload uses the semantic VoiceDataPayload (std::vector), so this
// struct is not the native layout (native vector at +20h, begin +24h).
struct VoiceDataMessage { NativeSessionMessageStorage base; seg1_cc13::VoiceDataPayload payload_18; };

// ---- host hooks ----------------------------------------------------------
struct MovieKeyframeHooks {
    virtual ~MovieKeyframeHooks() = default;
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    virtual void* construct_keyframe_00799b00(void* storage) = 0;  // ECX storage, returns it
    virtual void write_bits_00791360(void* keyframe, NativeBitCursor* cursor) = 0;
    virtual void read_bits_007a2370(void* keyframe, NativeSessionReadStream* stream) = 0;
    virtual void scalar_delete_slot0(void* keyframe) = 0;  // keyframe->vtable[0](1)
};
struct CameraHooks {
    virtual ~CameraHooks() = default;
    virtual void refresh_world_matrix_00b6db70(void* camera) = 0;
    virtual float fov_aspect_product_00b6fc30(void* camera) = 0;  // ST0 rounded to float by the caller's FSTP
};
// The std::vector<uint32_t> whose first/last words live at 00F87288/00F8728C.
struct PlaneSyncAckSource {
    const std::uint32_t* volatile& first_00f87288;
    const std::uint32_t* volatile& last_00f8728c;
    void (*invalid_parameter_noinfo_00bf6713)();
};
// The entity-id registry FindEntityByID uses (split, two 10h-byte row tables).
struct EntityIdTables {
    const volatile std::int32_t& split_00f89a10;
    const volatile std::int32_t& low_base_00f89a0c;
    const std::uint8_t* const volatile& low_rows_00f89a54;
    const volatile std::int32_t& high_base_00f89a60;
    const std::uint8_t* const volatile& high_rows_00f89aa8;
};

// ---- ConstructForRead ----------------------------------------------------
void construct_for_read_formation_join_0075a2e0(FormationMemberMessage*, const Context&);
void construct_for_read_formation_leave_0075a330(FormationMemberMessage*, const Context&);
void construct_for_read_formation_set_0075a380(FormationSetMessage*, const Context&);
void construct_for_read_kill_bullet_0075a4d0(HeaderedMessage*, const Context&);
void construct_for_read_gun_set_fire_mode_0075a550(UnitMessage*, const Context&);
void construct_for_read_r_t_gun_set_contfiring_0075a580(UnitMessage*, const Context&);
void construct_for_read_s_t_gun_fire_0075a5c0(UnitMessage*, const Context&);
void construct_for_read_ship_sync_0075a630(HeaderedMessage*, const Context&);
void construct_for_read_ship_helmsman_control_0075a690(ShipHelmsmanControlMessage*, const Context&);
void construct_for_read_path_set_0075a840(UnitMessage*, const Context&);
void construct_for_read_path_add_0075a8b0(UnitMessage*, const Context&);
void construct_for_read_path_insert_0075a920(UnitMessage*, const Context&);
void construct_for_read_path_delete_0075a990(UnitMessage*, const Context&);
void construct_for_read_plane_sync_0075aab0(PlaneSyncMessage*, const Context&);
void construct_for_read_plane_controlfactor_0075ab40(PlaneControlFactorMessage*, const Context&);
void construct_for_read_plane_hit_0075abe0(PlaneHitMessage*, const Context&);
void construct_for_read_plane_pcmdlasttime_0075ac50(PlanePcmdLastTimeMessage*, const Context&);
void construct_for_read_planesquadron_attackrun_0075ad30(UnitMessage*, const Context&);
void construct_for_read_submarine_dip_rise_0075ade0(UnitMessage*, const Context&);
void construct_for_read_submarine_dip_rise_sync_0075ae10(DipRiseSyncMessage*, const Context&);
void construct_for_read_grnd_vehicle_sync_0075aea0(HeaderedMessage*, const Context&);
void construct_for_read_landfort_effect_0075aed0(LandfortEffectMessage*, const Context&);
void construct_for_read_landingship_startlanding_0075af00(StartLandingMessage*, const Context&);
void construct_for_read_playerspawnpoint_setactive_0075b220(SpawnPointSetActiveMessage*, const Context&);
void construct_for_read_movcamnew_addposition_0075b2a0(MovieAddPositionMessage*, const Context&);

// ---- Construct (0075B430 inlined) ----------------------------------------
EntityRelocateMessage* construct_entity_relocate_0075bb30(EntityRelocateMessage*, const Context&);
PlayerInfoMessage* construct_playerinfo_0075c680(PlayerInfoMessage*, const Context&);
MusicSetLevelMessage* construct_musiccontrol_setlevel_0075ea30(MusicSetLevelMessage*, const Context&);
BlackoutMessage* construct_blackout_0075eb40(BlackoutMessage*, const Context&);
HandModeTargetedMessage* construct_movcam_handmodetargeted_0075ecb0(HandModeTargetedMessage*, const Context&);
RefPosPolarMessage* construct_movcam_refposmodepolar_0075edf0(RefPosPolarMessage*, const Context&);
RefPosRadiusMessage* construct_movcam_refposmoderadius_0075f000(RefPosRadiusMessage*, const Context&);
PathWalkPropertyMessage* construct_movcam_pathwalkproperty_0075f1a0(PathWalkPropertyMessage*, const Context&);
AddInterpolatorMessage* construct_movcam_addinterpolator_0075f350(AddInterpolatorMessage*, const Context&);
SetDeviationFactorMessage* construct_movcam_setdeviationfactor_0075f540(SetDeviationFactorMessage*, const Context&);
VoiceChangeMessage* construct_voicechange_0075f9d0(VoiceChangeMessage*, const Context&);
NativeSessionMessageStorage* construct_playerunlockunit_0075fb00(NativeSessionMessageStorage*, const Context&);
ServerDlcUnlockMessage* construct_serverdlcunlock_0075fb70(ServerDlcUnlockMessage*, const Context&);
HeaderedMessage* construct_fragile_sethp_0075fc80(HeaderedMessage*, const Context&);
TGunTurnSyncMessage* construct_t_gun_turn_sync_0075fe90(TGunTurnSyncMessage*, const Context&);
VehicleGunControlMessage* construct_vehicle_gun_control_0075ff60(VehicleGunControlMessage*, const Context&);
UnitMessage* construct_vehicle_unit_launch_007601a0(UnitMessage*, const Context&);
UnitMessage* construct_vehicle_add_launched_00760200(UnitMessage*, const Context&);
VehicleValueMessage* construct_vehicle_set_party_00760260(VehicleValueMessage*, const Context&);
VehicleValueMessage* construct_vehicle_set_race_00760390(VehicleValueMessage*, const Context&);
VehicleShipyardLaunchMessage* construct_vehicle_shipyard_launch_007604c0(VehicleShipyardLaunchMessage*, const Context&);
HitMarkerMessage* construct_hitmarker_00760680(HitMarkerMessage*, const Context&);
HitWarningMessage* construct_hitwarning_007607b0(HitWarningMessage*, const Context&);
RepairAddDamageMessage* construct_shiprepair_adddamage_007608e0(RepairAddDamageMessage*, const Context&);
RepairBodyMessage* construct_shiprepair_bodyrepair_00760a50(RepairBodyMessage*, const Context&);
HeaderedMessage* construct_shiprepair_repairpriority_00760b70(HeaderedMessage*, const Context&);
LandingShipsLaunchedMessage* construct_ship_landingshipslaunched_00760e90(LandingShipsLaunchedMessage*, const Context&);
TorpedoGenerateMessage* construct_ship_torpedogenerate_hack_00760fd0(TorpedoGenerateMessage*, const Context&);
SetTorpedoStockMessage* construct_ship_set_torpedostock_007611d0(SetTorpedoStockMessage*, const Context&);
UnitMessage* construct_ship_explodetoparts_007613b0(UnitMessage*, const Context&);
ExplodeOnePartMessage* construct_ship_explodeonepart_00761450(ExplodeOnePartMessage*, const Context&);
SectionFailureMessage* construct_ship_sectionfailureefx_00761590(SectionFailureMessage*, const Context&);
ShipLeakMessage* construct_ship_leak_00761f60(ShipLeakMessage*, const Context&);
HeaderedMessage* construct_ship_sink_00762130(HeaderedMessage*, const Context&);
HeaderedMessage* construct_ship_leakcheatwater_00762190(HeaderedMessage*, const Context&);
ShipAvoidSideMessage* construct_ship_avoidside_007623d0(ShipAvoidSideMessage*, const Context&);
PlaneSyncAckMessage* construct_plane_sync_ack_007626e0(PlaneSyncAckMessage*, const Context&);
PlaneChangeStateMessage* construct_plane_change_state_00762880(PlaneChangeStateMessage*, const Context&);
PlaneDisappearMessage* construct_plane_disappear_00762b20(PlaneDisappearMessage*, const Context&);
PlaneDeadmeatRotMessage* construct_plane_deadmeatrot_00762c80(PlaneDeadmeatRotMessage*, const Context&);
UnitMessage* construct_airbase_setslot_00763140(UnitMessage*, const Context&);
AirbaseLaunchSlotMessage* construct_airbase_launchslot_007631a0(AirbaseLaunchSlotMessage*, const Context&);
UnitMessage* construct_airbase_changeslotstatus_00763320(UnitMessage*, const Context&);
UnitMessage* construct_airbase_slotstatussync_00763380(UnitMessage*, const Context&);
UnitMessage* construct_airbase_stockitem_007633e0(UnitMessage*, const Context&);
UnitMessage* construct_airbase_appearreadyplane_00763440(UnitMessage*, const Context&);
UnitMessage* construct_airbase_setorders_007634a0(UnitMessage*, const Context&);
UnitMessage* construct_mothership_elevator_move_00763500(UnitMessage*, const Context&);
UnitMessage* construct_bombplatform_fire_00763600(UnitMessage*, const Context&);
HeaderedMessage* construct_multibombplatform_generate_00763660(HeaderedMessage*, const Context&);
UnitMessage* construct_catapult_fire_007636c0(UnitMessage*, const Context&);
ParatrooperLandingPosMessage* construct_paratrooper_landingpos_00763b40(ParatrooperLandingPosMessage*, const Context&);
UnitMessage* construct_shipyard_change_slot_class_00763cb0(UnitMessage*, const Context&);
UnitMessage* construct_shipyard_change_slot_equipment_00763d10(UnitMessage*, const Context&);
UnitMessage* construct_shipyard_launch_slot_00763d70(UnitMessage*, const Context&);
ShipyardSpawnMessage* construct_shipyard_spawn_00763dd0(ShipyardSpawnMessage*, const Context&);
HeaderedMessage* construct_commandbuilding_setowner_00764220(HeaderedMessage*, const Context&);
HeaderedMessage* construct_commandbuilding_setlevel_00764280(HeaderedMessage*, const Context&);
HeaderedMessage* construct_commandbuilding_updatecapture_007642e0(HeaderedMessage*, const Context&);
SpawnBannMessage* construct_highlvlai_spawnbann_00764490(SpawnBannMessage*, const Context&);
UnitMessage* construct_ship_erasewreck_007645b0(UnitMessage*, const Context&);
HeaderedMessage* construct_watermine_arm_007648b0(HeaderedMessage*, const Context&);
// __thiscall(this, camera, flag) RET 8.
CameraMessage* construct_camera_00764960(CameraMessage*, void* camera, std::uint8_t flag, const Context&, CameraHooks&);
ManageMessagesMessage* construct_manage_messages_00765490(ManageMessagesMessage*, const Context&);
VoiceOverMessage* construct_voiceover_00765610(VoiceOverMessage*, const Context&);
ManageDialogsMessage* construct_manage_dialogs_00765940(ManageDialogsMessage*, const Context&);
CountdownMessage* construct_countdown_007663f0(CountdownMessage*, const Context&);
MetricsSessionMessage* construct_metricssession_007665d0(MetricsSessionMessage*, const Context&);
InferiorListMessage* construct_commandbuilding_inferiorlist_00766660(InferiorListMessage*, const Context&);
// 004C1F50 allocates the std::list head node.
SessionManagerMessage* construct_session_manager_007666c0(SessionManagerMessage*, const Context&, void* (*buy_head_node_004c1f50)());
VoiceDataMessage* construct_voicedata_007673b0(VoiceDataMessage*, const Context&);

// ---- IsType --------------------------------------------------------------
bool is_control_00759e00(std::uint32_t type);
bool is_formation_join_0075a310(std::uint32_t type);
bool is_formation_leave_0075a360(std::uint32_t type);
bool is_ship_helmsman_control_0075a7d0(std::uint32_t type);
bool is_plane_sync_0075ab10(std::uint32_t type);
bool is_plane_controlfactor_0075abb0(std::uint32_t type);
bool is_plane_hit_0075ac20(std::uint32_t type);
bool is_submarine_dip_rise_sync_0075ae50(std::uint32_t type);
bool is_movcamnew_addposition_0075b330(const NativeSessionMessageStorage*, std::uint32_t type);
bool is_entity_relocate_0075bb90(std::uint32_t type);
bool is_musiccontrol_setlevel_0075eab0(std::uint32_t type);
bool is_blackout_0075ebc0(std::uint32_t type);
bool is_movcam_handmodetargeted_0075ed30(std::uint32_t type);
bool is_movcam_refposmodepolar_0075ee70(std::uint32_t type);
bool is_movcam_refposmoderadius_0075f080(std::uint32_t type);
bool is_movcam_pathwalkproperty_0075f220(std::uint32_t type);
bool is_movcam_addinterpolator_0075f3d0(std::uint32_t type);
bool is_movcam_setdeviationfactor_0075f5c0(std::uint32_t type);
bool is_voicechange_0075fa50(std::uint32_t type);
bool is_serverdlcunlock_0075fbf0(std::uint32_t type);
bool is_t_gun_turn_sync_0075ff10(std::uint32_t type);
bool is_vehicle_gun_control_0075ffd0(std::uint32_t type);
bool is_vehicle_set_party_007602c0(std::uint32_t type);
bool is_vehicle_set_race_007603f0(std::uint32_t type);
bool is_vehicle_shipyard_launch_00760520(std::uint32_t type);
bool is_hitmarker_007606f0(const NativeSessionMessageStorage*, std::uint32_t type);
bool is_hitwarning_00760830(const NativeSessionMessageStorage*, std::uint32_t type);
bool is_shiprepair_adddamage_00760940(std::uint32_t type);
bool is_shiprepair_bodyrepair_00760ab0(std::uint32_t type);
bool is_ship_guns_sync_00760c80(std::uint32_t type);
bool is_ship_landingshipslaunched_00760ef0(std::uint32_t type);
bool is_ship_torpedogenerate_hack_00761030(std::uint32_t type);
bool is_ship_set_torpedostock_00761230(std::uint32_t type);
bool is_ship_wrecked_00761370(std::uint32_t type);
bool is_ship_explodetoparts_00761410(std::uint32_t type);
bool is_ship_explodeonepart_007614b0(std::uint32_t type);
bool is_ship_sectionfailureefx_007615f0(std::uint32_t type);
bool is_ship_leak_00761fc0(std::uint32_t type);
bool is_ship_leakcheatwater_007621f0(std::uint32_t type);
bool is_ship_addtorque_00762280(std::uint32_t type);
bool is_ship_avoidside_00762430(std::uint32_t type);
bool is_plane_sync_ack_00762750(const NativeSessionMessageStorage*, std::uint32_t type);
bool is_plane_change_state_007628f0(std::uint32_t type);
bool is_plane_touchdown_00762ad0(std::uint32_t type);
bool is_plane_disappear_00762b90(std::uint32_t type);
bool is_plane_deadmeatrot_00762cf0(std::uint32_t type);
bool is_airbase_launchslot_00763210(std::uint32_t type);
bool is_paratrooper_landingpos_00763ba0(std::uint32_t type);
bool is_highlvlai_spawnbann_00764510(const NativeSessionMessageStorage*, std::uint32_t type);
bool is_ship_erasewreck_00764610(std::uint32_t type);
bool is_watermine_arm_00764920(std::uint32_t type);
bool is_manage_messages_007654f0(std::uint32_t type);
bool is_voiceover_00765670(std::uint32_t type);
bool is_countdown_00766450(std::uint32_t type);
bool is_voicedata_00767450(std::uint32_t type);

// ---- Write / Read ----------------------------------------------------------
using Numeric = NativeBitNumericContext;
void write_movcamnew_addposition_0075b350(const MovieAddPositionMessage*, NativeBitCursor*, MovieKeyframeHooks&);
void read_movcamnew_addposition_0075b380(MovieAddPositionMessage*, NativeSessionReadStream*, MovieKeyframeHooks&);
void write_entity_relocate_0075bbd0(const EntityRelocateMessage*, NativeBitCursor*, const Numeric&);
void read_entity_relocate_0075bc80(EntityRelocateMessage*, NativeSessionReadStream*, const Numeric&);
void write_formation_join_0075c7c0(const FormationMemberMessage*, NativeBitCursor*);
void read_formation_join_0075c810(FormationMemberMessage*, NativeSessionReadStream*);
void write_formation_leave_0075c860(const FormationMemberMessage*, NativeBitCursor*);
void read_formation_leave_0075c8b0(FormationMemberMessage*, NativeSessionReadStream*);
void write_musiccontrol_setlevel_0075eac0(const MusicSetLevelMessage*, NativeBitCursor*);
void read_musiccontrol_setlevel_0075eaf0(MusicSetLevelMessage*, NativeSessionReadStream*);
void write_blackout_0075ebd0(const BlackoutMessage*, NativeBitCursor*, const Numeric&);
void read_blackout_0075ec30(BlackoutMessage*, NativeSessionReadStream*, const Numeric&);
void write_movcam_handmodetargeted_0075ed40(const HandModeTargetedMessage*, NativeBitCursor*);
void read_movcam_handmodetargeted_0075ed90(HandModeTargetedMessage*, NativeSessionReadStream*);
void write_movcam_refposmodepolar_0075ee80(const RefPosPolarMessage*, NativeBitCursor*, const Numeric&);
void read_movcam_refposmodepolar_0075ef30(RefPosPolarMessage*, NativeSessionReadStream*, const Numeric&);
void write_movcam_refposmoderadius_0075f090(const RefPosRadiusMessage*, NativeBitCursor*, const Numeric&);
void read_movcam_refposmoderadius_0075f110(RefPosRadiusMessage*, NativeSessionReadStream*, const Numeric&);
void write_movcam_pathwalkproperty_0075f230(const PathWalkPropertyMessage*, NativeBitCursor*, const Numeric&);
void read_movcam_pathwalkproperty_0075f2b0(PathWalkPropertyMessage*, NativeSessionReadStream*, const Numeric&);
void write_movcam_addinterpolator_0075f3e0(const AddInterpolatorMessage*, NativeBitCursor*, const Numeric&);
void read_movcam_addinterpolator_0075f480(AddInterpolatorMessage*, NativeSessionReadStream*, const Numeric&);
void write_movcam_setdeviationfactor_0075f5d0(const SetDeviationFactorMessage*, NativeBitCursor*, const Numeric&);
void read_movcam_setdeviationfactor_0075f610(SetDeviationFactorMessage*, NativeSessionReadStream*, const Numeric&);
void write_voicechange_0075fa60(const VoiceChangeMessage*, NativeBitCursor*);
void read_voicechange_0075faa0(VoiceChangeMessage*, NativeSessionReadStream*);
void write_serverdlcunlock_0075fc00(const ServerDlcUnlockMessage*, NativeBitCursor*);
void read_serverdlcunlock_0075fc30(ServerDlcUnlockMessage*, NativeSessionReadStream*);
void write_vehicle_gun_control_0075fff0(const VehicleGunControlMessage*, NativeBitCursor*, const Numeric&);
void read_vehicle_gun_control_007600d0(VehicleGunControlMessage*, NativeSessionReadStream*, const Numeric&);
void write_vehicle_set_party_007602f0(const VehicleValueMessage*, NativeBitCursor*);
void read_vehicle_set_party_00760340(VehicleValueMessage*, NativeSessionReadStream*);
void write_vehicle_set_race_00760420(const VehicleValueMessage*, NativeBitCursor*);
void read_vehicle_set_race_00760470(VehicleValueMessage*, NativeSessionReadStream*);
void write_vehicle_shipyard_launch_00760550(const VehicleShipyardLaunchMessage*, NativeBitCursor*);
void read_vehicle_shipyard_launch_007605a0(VehicleShipyardLaunchMessage*, NativeSessionReadStream*);
void write_hitmarker_00760710(const HitMarkerMessage*, NativeBitCursor*);
void read_hitmarker_00760750(HitMarkerMessage*, NativeSessionReadStream*);
void write_hitwarning_00760850(const HitWarningMessage*, NativeBitCursor*);
void read_hitwarning_00760880(HitWarningMessage*, NativeSessionReadStream*);
void write_shiprepair_adddamage_00760970(const RepairAddDamageMessage*, NativeBitCursor*, const Numeric&);
void read_shiprepair_adddamage_007609e0(RepairAddDamageMessage*, NativeSessionReadStream*, const Numeric&);
void write_shiprepair_bodyrepair_00760ae0(const RepairBodyMessage*, NativeBitCursor*);
void read_shiprepair_bodyrepair_00760b30(RepairBodyMessage*, NativeSessionReadStream*);
void write_ship_helmsman_control_00760d00(const ShipHelmsmanControlMessage*, NativeBitCursor*, const Numeric&);
void read_ship_helmsman_control_00760d80(ShipHelmsmanControlMessage*, NativeSessionReadStream*, const Numeric&);
void write_ship_landingshipslaunched_00760f20(const LandingShipsLaunchedMessage*, NativeBitCursor*, const Numeric&);
void read_ship_landingshipslaunched_00760f80(LandingShipsLaunchedMessage*, NativeSessionReadStream*, const Numeric&);
void write_ship_torpedogenerate_hack_00761060(const TorpedoGenerateMessage*, NativeBitCursor*, const Numeric&);
void read_ship_torpedogenerate_hack_00761120(TorpedoGenerateMessage*, NativeSessionReadStream*, const Numeric&);
void write_ship_set_torpedostock_00761270(const SetTorpedoStockMessage*, NativeBitCursor*);
void read_ship_set_torpedostock_007612c0(SetTorpedoStockMessage*, NativeSessionReadStream*);
void write_ship_explodeonepart_007614f0(const ExplodeOnePartMessage*, NativeBitCursor*);
void read_ship_explodeonepart_00761540(ExplodeOnePartMessage*, NativeSessionReadStream*);
void write_ship_sectionfailureefx_00761630(const SectionFailureMessage*, NativeBitCursor*);
void read_ship_sectionfailureefx_007616a0(SectionFailureMessage*, NativeSessionReadStream*);
void write_ship_leak_00761ff0(const ShipLeakMessage*, NativeBitCursor*, const Numeric&);
void read_ship_leak_00762090(ShipLeakMessage*, NativeSessionReadStream*, const Numeric&);
void write_ship_addtorque_007622b0(const ShipAddTorqueMessage*, NativeBitCursor*, const Numeric&);
void read_ship_addtorque_00762340(ShipAddTorqueMessage*, NativeSessionReadStream*, const Numeric&);
void write_ship_avoidside_00762460(const ShipAvoidSideMessage*, NativeBitCursor*);
void read_ship_avoidside_007624b0(ShipAvoidSideMessage*, NativeSessionReadStream*);
void write_plane_sync_ack_00762770(const PlaneSyncAckMessage*, NativeBitCursor*, const PlaneSyncAckSource&);
void read_plane_sync_ack_00762800(PlaneSyncAckMessage*, NativeSessionReadStream*);
void write_plane_change_state_00762940(const PlaneChangeStateMessage*, NativeBitCursor*);
void read_plane_change_state_007629a0(PlaneChangeStateMessage*, NativeSessionReadStream*);
void write_plane_disappear_00762be0(const PlaneDisappearMessage*, NativeBitCursor*);
void read_plane_disappear_00762c30(PlaneDisappearMessage*, NativeSessionReadStream*);
void write_plane_deadmeatrot_00762d40(const PlaneDeadmeatRotMessage*, NativeBitCursor*, const Numeric&);
void read_plane_deadmeatrot_00762da0(PlaneDeadmeatRotMessage*, NativeSessionReadStream*, const Numeric&);
void write_plane_controlfactor_00762e00(const PlaneControlFactorMessage*, NativeBitCursor*, const Numeric&);
void read_plane_controlfactor_00762e90(PlaneControlFactorMessage*, NativeSessionReadStream*, const Numeric&);
void write_plane_hit_00762f20(const PlaneHitMessage*, NativeBitCursor*, const Numeric&);
void read_plane_hit_00762fb0(PlaneHitMessage*, NativeSessionReadStream*, const Numeric&);
void write_plane_pcmdlasttime_00763040(const PlanePcmdLastTimeMessage*, NativeBitCursor*, const Numeric&);
void read_plane_pcmdlasttime_007630b0(PlanePcmdLastTimeMessage*, NativeSessionReadStream*, const Numeric&);
void write_airbase_launchslot_00763260(const AirbaseLaunchSlotMessage*, NativeBitCursor*);
void read_airbase_launchslot_007632c0(AirbaseLaunchSlotMessage*, NativeSessionReadStream*);
void write_submarine_dip_rise_sync_00763560(const DipRiseSyncMessage*, NativeBitCursor*);
void read_submarine_dip_rise_sync_007635b0(DipRiseSyncMessage*, NativeSessionReadStream*);
void write_paratrooper_landingpos_00763bd0(const ParatrooperLandingPosMessage*, NativeBitCursor*, const Numeric&);
void read_paratrooper_landingpos_00763c30(ParatrooperLandingPosMessage*, NativeSessionReadStream*, const Numeric&);
void write_shipyard_spawn_00763ee0(const ShipyardSpawnMessage*, NativeBitCursor*);
void read_shipyard_spawn_00763f50(ShipyardSpawnMessage*, NativeSessionReadStream*);
void write_highlvlai_spawnbann_00764530(const SpawnBannMessage*, NativeBitCursor*);
void read_highlvlai_spawnbann_00764560(SpawnBannMessage*, NativeSessionReadStream*);
// 00E17669 is the native empty-string backing the owned-string writer uses for null data.
void write_manage_messages_00765500(const ManageMessagesMessage*, NativeBitCursor*, const char* fallback_00e17669);
void read_manage_messages_00765540(ManageMessagesMessage*, NativeSessionReadStream*, NativeStringRawPoolContext&);
void write_voiceover_00765680(const VoiceOverMessage*, NativeBitCursor*, const char* fallback_00e17669);
void read_voiceover_007656b0(VoiceOverMessage*, NativeSessionReadStream*, NativeStringRawPoolContext&);
void write_countdown_00766460(const CountdownMessage*, NativeBitCursor*, const char* fallback_00e17669, const Numeric&);
void read_countdown_007664e0(CountdownMessage*, NativeSessionReadStream*, NativeStringRawPoolContext&, const Numeric&);
void write_voicedata_00767460(const VoiceDataMessage*, NativeBitCursor*);
void read_voicedata_00767490(VoiceDataMessage*, NativeSessionReadStream*);

// ---- other ---------------------------------------------------------------
// 00763E70, slot +10h of 00D035A4 (__thiscall, ECX this, plain RET): the
// target id +28h names a live registry row. Returns AL only.
bool shipyard_spawn_target_resolves_00763e70(const ShipyardSpawnMessage*, const EntityIdTables&);
// 00762500, slot 0 of 00D02D94 (SHIP_SYNC): __thiscall(this, flags) RET 4.
// Stamps the root vtable 00CE4974, frees through _free when bit0 is set.
NativeSessionMessageStorage* delete_ship_sync_00762500(NativeSessionMessageStorage*, std::uint32_t flags, const Context&);
// 007656E0 non-deleting destructor: own vtable, 0041DD20 on +18h, root vtable.
void destruct_voiceover_007656e0(VoiceOverMessage*, const Context&, NativeStringRawPoolContext&);
// 007674C0 non-deleting destructor: own vtable, free the frame vector, root vtable.
void destruct_voicedata_007674c0(VoiceDataMessage*, const Context&);
// 0075B2D0 non-deleting destructor: own vtable 00D02ED4; when +1Ch is set and
// +18h is non-null, keyframe->vtable[0](1); root vtable.
void destruct_movcamnew_addposition_0075b2d0(MovieAddPositionMessage*, const Context&, MovieKeyframeHooks&);
// 00765590 / 00766550 non-deleting destructors: own vtable, inlined 0041DD20 on +18h, root vtable.
void destruct_manage_messages_00765590(ManageMessagesMessage*, const Context&, NativeStringRawPoolContext&);
void destruct_countdown_00766550(CountdownMessage*, const Context&, NativeStringRawPoolContext&);
// More class tests and constructors in the segment.
bool is_ship_guns_sync_category_0075a660(std::uint32_t type);  // 8Ch/48h/46h
bool is_mnettablesyncentity_sync_00768440(std::uint32_t type);  // E0h/48h/46h
NativeSessionMessageStorage* construct_movcam_blackbar_0075e4d0(NativeSessionMessageStorage*, const Context&);  // 34h, 00CF17F8
UnitMessage* construct_plane_takeoff_00762a00(UnitMessage*, const Context&);  // C6h, 00CFAA20
UnitMessage* construct_plane_touchdown_00762a60(UnitMessage*, const Context&);  // C5h, 00D03518
HeaderedMessage* construct_ship_addtorque_00762220(HeaderedMessage*, const Context&);  // 93h, 00D034C8
UnitMessage* construct_ship_wrecked_00761310(UnitMessage*, const Context&);  // 98h, 00D033C4
// 0075B500 __thiscall(ECX source, float out[16]) -> EAX out, RET 4: identity,
// 00467050(out, [src+30h], [src+34h], [src+38h]) (ECX out, RET 0Ch), the
// translation row from +24h..+2Ch, and when byte +100h is set the three basis
// rows scaled by +104h (each product of two floats, exact before the store).
float* build_transform_0075b500(const void* source, float* out16, void (*set_rotation_from_euler_00467050)(float* matrix, float x, float y, float z));
// 0076A7B0 __thiscall(ECX session, message 05h) RET 4: copies the server-info
// fields of MT 05h into the session record (strcpy of the 32-byte name).
// The raw 110h net-entity create message base (vtable 00D03720) as its
// ConstructForRead and destructors touch it. net_entity_create_cc13.hpp holds
// the semantic snapshot of the same message for the writer/reader.
struct NetEntityCreatePrefix {
    HeaderedMessage h;
    std::uint8_t opaque_1c[0x30];
    void* pairs_first_4c;
    void* pairs_last_50;
    void* pairs_end_54;
    std::uint8_t opaque_58[0x68];
    char* name_c0;
    std::uint8_t opaque_c4[0x4c];
};
static_assert(sizeof(NetEntityCreatePrefix) == 0x110);
static_assert(offsetof(NetEntityCreatePrefix, name_c0) == 0xC0);
struct PathCreatePrefix { NetEntityCreatePrefix base; std::uint32_t opaque_110; void* first_114; void* last_118; void* end_11c; };
struct AirfieldCreatePrefix {
    NetEntityCreatePrefix base;
    const std::uint32_t* sub_vtable_110;
    std::uint32_t opaque_114[2];
    std::uint32_t zero_11c[3];
    std::uint32_t opaque_128;
    std::uint32_t zero_12c[5];
};
static_assert(offsetof(AirfieldCreatePrefix, zero_12c) == 0x12C);
struct LandfortCreatePrefix { NetEntityCreatePrefix base; const std::uint32_t* sub_vtable_110; };
// E0h MNETTABLESYNCENTITY_SYNC: a std::map at +1Ch (head +20h, size +24h)
// and a native string header at +28h.
struct MNetTableSyncMessage {
    HeaderedMessage h;
    std::uint8_t map_1c[4];
    std::uint8_t* head_20;
    std::uint32_t size_24;
    NativeStringHeader text_28;
};
static_assert(offsetof(MNetTableSyncMessage, text_28) == 0x28);
struct MNetTableSyncHooks {
    virtual ~MNetTableSyncHooks() = default;
    virtual std::uint8_t* allocate_head_node_006edea0() = 0;  // the map head node (Ghidra: BSP_MissionLuaVariant_AllocateLinkRecord)
    // 006EE960 __thiscall(map, out iterator, first {map,node}, last {map,node}).
    virtual void erase_range_006ee960(void* map, void* out_iterator, void* first_map, void* first_node, void* last_map, void* last_node) = 0;
};
void construct_for_read_net_entity_create_00766f60(NetEntityCreatePrefix*, const Context&);
// 007668E0 and its four identical copies (00767920, 00767B40, 00767C70, 00767D80).
void destruct_net_entity_create_007668e0(NetEntityCreatePrefix*, const Context&);
void destruct_net_entity_create_00767920(NetEntityCreatePrefix*, const Context&);
void destruct_net_entity_create_00767b40(NetEntityCreatePrefix*, const Context&);
void destruct_net_entity_create_00767c70(NetEntityCreatePrefix*, const Context&);
void destruct_net_entity_create_00767d80(NetEntityCreatePrefix*, const Context&);
PathCreatePrefix* construct_for_read_path_create_007676c0(PathCreatePrefix*, const Context&);
void destruct_path_create_00767740(PathCreatePrefix*, const Context&);
AirfieldCreatePrefix* construct_for_read_airfield_create_007679d0(AirfieldCreatePrefix*, const Context&);
LandfortCreatePrefix* construct_for_read_landfort_create_00767a70(LandfortCreatePrefix*, const Context&);
MNetTableSyncMessage* construct_for_read_mnettablesyncentity_sync_007683c0(MNetTableSyncMessage*, const Context&, MNetTableSyncHooks&);
void destruct_mnettablesyncentity_sync_00768470(MNetTableSyncMessage*, const Context&, MNetTableSyncHooks&, NativeStringRawPoolContext&);
// Derived create-message writers: the base writer runs first through a hook
// (00765000 for planes, 006E0690 BOMB_CREATE for torpedoes and paratroopers),
// then the derived fields at +110h and above.
struct CreateWriteHooks {
    virtual ~CreateWriteHooks() = default;
    virtual void net_entity_create_write_00765000(const void* message, NativeBitCursor* cursor) = 0;
    virtual void bomb_create_write_006e0690(const void* message, NativeBitCursor* cursor) = 0;
};
struct PlaneCreateMessage {
    NetEntityCreatePrefix base;
    std::uint16_t id_110, pad_112;
    float value_114, value_118, value_11c;
    std::int32_t value_120, value_124;
    std::uint8_t byte_128, byte_129, flag_12a, pad_12b;
    std::int32_t optional_12c;  // sent as 8 bits after a presence bool when >= 0
};
static_assert(offsetof(PlaneCreateMessage, optional_12c) == 0x12C);
struct BombDerivedCreateMessage {
    NetEntityCreatePrefix base;
    std::uint8_t bomb_fields_110[0x10];  // BOMB_CREATE's +110h float[3], +11Ch bool
    float value_120;
    float value_124;
    std::uint16_t id_128;
};
static_assert(offsetof(BombDerivedCreateMessage, id_128) == 0x128);
struct ParatrooperCreateMessage { NetEntityCreatePrefix base; std::uint8_t bomb_fields_110[0x10]; float position_120[3]; };
void write_plane_create_00767810(const PlaneCreateMessage*, NativeBitCursor*, const Numeric&, CreateWriteHooks&);
// +120h FLT_MAX/32, +124h (0, 1, [00CE3D08] = 100.0f, 12 bits), +128h id 12.
void write_torpedo_create_00767ad0(const BombDerivedCreateMessage*, NativeBitCursor*, const Numeric&, CreateWriteHooks&);
// +120h float[3] through the array codec.
void write_paratrooper_create_00767c30(const ParatrooperCreateMessage*, NativeBitCursor*, const Numeric&, CreateWriteHooks&);
inline constexpr std::uint32_t kScale_00ce3d08 = 0x42C80000u;  // 100.0f
void apply_server_info_0076a7b0(std::uint8_t* session, const bsp::NativeSessionMessage05* message);

}  // namespace bsp::seg42
