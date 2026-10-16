// cc13 leftover sweep C2 (packet cc13-sweepC2): the functions cc13-sweepC deferred
// (docs/CC13_SWEEPC.md, "Skipped or deferred").
//
// Model as in cc13_sweepC.hpp: every routine works on the original object as raw bytes (Win32
// layout, offsets as in the image) and reaches the engine through SweepC2Host, whose methods name
// the image routine or virtual slot they stand for and give the register inputs. Constants carry
// their image address and width. New C++ interfaces, not drop-in binary replacements; nothing
// here is bound into bsp_game. Names are hypotheses, not recovered symbols.

#ifndef BSP_CC13_SWEEPC2_HPP
#define BSP_CC13_SWEEPC2_HPP

#include <cstddef>
#include <cstdint>

#include "bsp/cc13_sweepC.hpp"

namespace bsp::cc13_sweepC2 {

using cc13_sweepC::add;
using cc13_sweepC::at;
using cc13_sweepC::ptr;

// ---------------------------------------------------------------------------
// Image constants.
// ---------------------------------------------------------------------------
inline constexpr double kAiLaunchInterval = 2.0;          // 00D7A308, double
inline constexpr std::uint32_t kAiLaunchClassKind = 0x13; // 006CD8CC, vslot +1Ch answer
inline constexpr std::uint32_t kAiLaunchMaxFill = 3;      // 006CD9D3, the fill count clamp
inline constexpr std::uint32_t kAirOpsRandomStream = 1;   // 006CDAA0, ECX into 00BD2F40
inline constexpr std::uint32_t kMotherShipSiteVtable = 0x00CF8A58;      // 006CFB22
inline constexpr std::uint32_t kSiteObserverMemberVtable = 0x00CF89B4;  // 006CFB4C
inline constexpr std::uint32_t kElevatorOwnerLinkVtable = 0x00CF8928;   // 006CFD7D
inline constexpr float kUnitScale = 1.0f;                               // 00D7A24C, float
inline constexpr char kSiteEffectChildName[] = "Sound";                 // 00CEA3FC
inline constexpr double kParkSpotRadiusSq = 225.0;                      // 00CF8EB0, double
inline constexpr std::uint32_t kSessionGlobals = 0x00E188A8;            // pointer global
inline constexpr std::uint32_t kRelocateMessageKind = 0x97;             // 00819710
inline constexpr std::uint32_t kRelocateMessageVtable = 0x00D02F24;     // 00819737
inline constexpr std::uint32_t kSessionMessageBaseVtable = 0x00CE4974;  // 0081976B
inline constexpr float kRelocateSettleValue = 1.25f;                    // 00CF29A8, float
inline constexpr std::uint32_t kRelocateMemberArgument = 0x00F87574;    // 0081980A, address
inline constexpr std::uint32_t kInGameInterface = 0x00E198C4;           // pointer global
inline constexpr std::uint32_t kPlayerSlotManager = 0x00F89B3C;         // pointer global
inline constexpr std::uint32_t kSupportManagerBanned = 0x00F8AB6A;      // byte global
inline constexpr char kEmptyText[] = "";                                // 00CE3A0C
inline constexpr std::uint32_t kXenonSystemManager = 0x00F8ABE8;        // pointer global
inline constexpr char kNewMusicFormat[] = "New music requested %s at %d";  // 00D58ED0
inline constexpr char kMusicNotFoundFormat[] = "MUSIC NOT FOUND %d %d";    // 00D58EF0
inline constexpr std::uint32_t kEmptyNativeText = 0x00F8BBD0;           // char buffer global
inline constexpr char kDefExtension[] = ".def";                         // 00CF779C
inline constexpr float kNegativeZero = -0.0f;                           // 00D7A208, float
inline constexpr std::uint32_t kTextListenerBaseVtable = 0x00D5B708;    // 00A9727C / 00A97160
inline constexpr float kClipBoxDefaultBorder = 0.1f;                    // 00D7A2F0, float
inline constexpr char kBorderWidthName[] = "BorderWidth";               // 00D5D0D8
inline constexpr std::uint32_t kTaggedVector2Kind = 6;                  // 00ACE6EE
inline constexpr float kTerrainVerticalEpsilon = 0.001f;                // 00D7A23C, float
inline constexpr std::uint32_t kTerrainHeightFieldVtable = 0x00D5D350;  // 00ADD2C9
inline constexpr std::uint32_t kTerrainNodeBytes = 0x174;               // 00ADE880
inline constexpr char kTerrainVisibilityGroupName[] = "Terrain Visibility Group";  // 00CF00D8
inline constexpr char kTerrainPathPrefix[] = "terrain/";                // 00D0E69C
inline constexpr char kHeightmapSuffix[] = "_heightmap.tdt";            // 00D0E6A8 (14 chars)

// A 32-bit std::_Tree iterator {container, node} as the debug STL keeps it on the stack.
struct TreeIter {
    void* container;
    void* node;
};

// ---------------------------------------------------------------------------
// Engine boundary. Every call the reconstructed bodies make goes through here, at the point the
// image makes it.
// ---------------------------------------------------------------------------
struct SweepC2Host {
    virtual ~SweepC2Host() = default;

    // [00CE2218] EnterCriticalSection / [00CE2210] LeaveCriticalSection. The +18h recursion
    // counter of the engine's tracked lock is maintained by the bodies here.
    virtual void enter_critical_section(void* section) = 0;
    virtual void leave_critical_section(void* section) = 0;
    // 00BF6713, the checked-iterator failure report.
    virtual void invalid_iterator_00bf6713() = 0;
    // [00F876A4], the fixed-step clock (float).
    virtual float fixed_step_clock_00f876a4() = 0;
    // 00BF65AC free.
    virtual void free_00bf65ac(void* block) = 0;

    // --- 006CC800 -------------------------------------------------------------------------
    // 006CA010: vector<12-byte record>::resize(count, value) (ECX = vector object, RET 10h).
    virtual void vector12_resize_006ca010(void* vector, std::uint32_t count,
                                          const std::uint32_t (&value)[3]) = 0;
    // 006CA0F0: vector<20-byte record>::resize(count, value) (ECX = vector object, RET 18h).
    virtual void vector20_resize_006ca0f0(void* vector, std::uint32_t count,
                                          const std::uint32_t (&value)[5]) = 0;
    // 0095B9C0 BSP_VehicleClass_GetRegistryId (ECX = vehicle class).
    virtual std::uint32_t vehicle_class_registry_id_0095b9c0(void* vehicle_class) = 0;

    // --- 006CD810 -------------------------------------------------------------------------
    // vehicle class vtable +1Ch (ECX = class): the class kind (13h for the AI launch type).
    virtual std::int32_t class_kind_vslot_1c(void* vehicle_class) = 0;
    // 006BD230 BSP_AirOps_SlotStateFromSquadronCommand (ECX = block, slot, command), RET 8.
    virtual void air_ops_slot_state_from_command_006bd230(void* block, std::uint32_t slot,
                                                          std::uint32_t command) = 0;
    // 006C0F00 BSP_AirOps_FillSlot (ECX = block, slot, class, count), RET 0Ch.
    virtual void air_ops_fill_slot_006c0f00(void* block, std::uint32_t slot, void* vehicle_class,
                                            std::int32_t count) = 0;
    // 00BD2F40 BSP_RandomThreads_DrawUnsignedFraction (ECX = stream), float in ST0. One draw
    // from the shared generator 00BD2F10.
    virtual float random_unsigned_fraction_00bd2f40(std::uint32_t stream) = 0;
    // 006BFDB0 STL_Tree_BuyHeadNode (ECX = tree), returns the head node.
    virtual void* tree_buy_head_node_006bfdb0(void* tree) = 0;
    // 006CAB40 map<int,int>::operator[] (ECX = map, key*), RET 4, returns the mapped int.
    virtual std::int32_t* int_map_subscript_006cab40(void* map, const std::int32_t* key) = 0;
    // 006C9DC0 tree erase(first, last) (ECX = tree, out iterator*, first, last), RET 14h.
    virtual void tree_erase_range_006c9dc0(void* tree, TreeIter* out, TreeIter first,
                                           TreeIter last) = 0;
    // 006BAE90 tree iterator ++ (ECX = iterator).
    virtual void tree_iter_inc_006bae90(TreeIter* iter) = 0;

    // --- 006CFAF0 -------------------------------------------------------------------------
    // 006CF100 BSP_AirOpsSite_Construct (ECX = site).
    virtual void air_ops_site_construct_006cf100(void* site) = 0;
    // 006FC2A0 BSP_ElevatorPlatform_Construct (ECX = platform).
    virtual void elevator_platform_construct_006fc2a0(void* platform) = 0;
    // [00CE221C] InterlockedIncrement / [00CE2220] InterlockedDecrement -> the new count.
    virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* counter) = 0;
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* counter) = 0;
    // [object]->vtable[0](ECX = object): the ref-counted object's destroy slot (no argument).
    virtual void refcounted_destroy_vslot_00(void* object) = 0;
    // 008687C0 BSP_PointEffect_CreateWithParentMatrix, __fastcall(ECX = out slot, EDX = parent,
    // consumed definition (by-value ref, already add-ref'd), matrix*, word, word, word), RET 14h;
    // returns the out slot.
    virtual void** point_effect_create_with_parent_matrix_008687c0(
        void** out_slot, void* parent, void* consumed_definition, float* matrix,
        std::uint32_t transform_word, std::uint32_t option_word, std::uint32_t tail_word) = 0;
    // 0041DD40, __thiscall(ECX = NativeString, length, keep), RET 8: resize.
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length,
                                               std::uint32_t keep) = 0;
    // 00BF7680 memcpy (cdecl).
    virtual void memcpy_00bf7680(void* dest, const void* src, std::uint32_t bytes) = 0;
    // 00419CC0(data, size, 1) answers the pool, then 00BD1510(ECX = pool) returns the block with
    // the same three stack arguments (as cc13_sweepC models it).
    virtual void string_pool_release_00419cc0_00bd1510(void* data, std::uint32_t size) = 0;
    // 00866CD0 BSP_PointEffect_FindOrCreateNamedChild, __thiscall(ECX = effect, out slot*,
    // NativeString* name), RET 8; returns the out slot.
    virtual void** point_effect_find_or_create_named_child_00866cd0(void* effect, void** out_slot,
                                                                    void* name) = 0;
    // 00424C40 BSP_GameSettings_GetSingleton.
    virtual void* game_settings_singleton_00424c40() = 0;
    // 006FC380 BSP_ElevatorPlatform_Setup (ECX = platform, const float* lift point, float speed,
    // float depth, consumed effect ref by value), RET 10h.
    virtual void elevator_platform_setup_006fc380(void* platform, const float* lift_point,
                                                  float first, float second,
                                                  void* consumed_effect) = 0;
    // 00BF681B operator new.
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    // 006FBC20 BSP_OwnedPtrWithDefault_Assign (ECX = platform, object), RET 4.
    virtual void owned_ptr_assign_006fbc20(void* platform, void* object) = 0;

    // --- 006D3F20 -------------------------------------------------------------------------
    // 006D32F0 BSP_AirFieldLockSingleton_Get (ECX = airfield): the holder whose +4 is the lock.
    virtual void* airfield_lock_singleton_006d32f0(void* airfield) = 0;
    // 006D2980 BSP_AirField_RebuildAimPoints (ECX = airfield).
    virtual void airfield_rebuild_aim_points_006d2980(void* airfield) = 0;

    // --- shared ---------------------------------------------------------------------------
    // The host location of an image global (for globals read as dwords, or passed by address).
    virtual void* image_address(std::uint32_t address) = 0;

    // --- 008196B0 -------------------------------------------------------------------------
    // 007788B0 BSP_Unit_IsFormationFollower (ECX = unit) -> AL.
    virtual bool unit_is_formation_follower_007788b0(void* unit) = 0;
    // 0075B430 BSP_SessionMessage_ConstructBase (ECX = message, kind), RET 4.
    virtual void session_message_construct_base_0075b430(void* message, std::uint32_t kind) = 0;
    // 0077C7B0 BSP_SessionOwner_BroadcastMessage (ECX = unit, message*, 0), RET 8.
    virtual void session_owner_broadcast_0077c7b0(void* unit, void* message,
                                                  std::uint32_t flags) = 0;
    // 009584D0 BSP_UnitInstance_SetHeading (ECX = unit, float), RET 4.
    virtual void unit_set_heading_009584d0(void* unit, float heading) = 0;
    // 00414DB0 entity pose refresh (ECX = unit).
    virtual void entity_pose_refresh_world_00414db0(void* unit) = 0;
    // 0092D620 (ECX = motion object unit+1018h, unit+0CCh), RET 4.
    virtual void motion_sync_pose_0092d620(void* motion, void* pose) = 0;
    // 00778890 BSP_UnitInstance_IsOccupantOwner (ECX = unit) -> AL.
    virtual bool unit_is_occupant_owner_00778890(void* unit) = 0;
    // 0070D060 BSP_UnitGroup_MemberAt (ECX = group, index), RET 4.
    virtual void* unit_group_member_at_0070d060(void* group, std::int32_t index) = 0;
    // unit vtable +118h (ECX = unit, const void* position) and +D8h (ECX = unit).
    virtual void unit_vslot_118(void* unit, void* argument) = 0;
    virtual void unit_vslot_d8(void* unit) = 0;

    // --- 0095A880 -------------------------------------------------------------------------
    // 005471B0 BSP_RefCountedPointerArray_Resize (ECX = array {data, size}, count), RET 4.
    virtual void pointer_array_resize_005471b0(void* array, std::int32_t count) = 0;
    // 009596A0 std::list<int> contains (ECX = list object, EDX = value*) -> AL.
    virtual bool int_list_contains_009596a0(void* list, const std::uint32_t* value) = 0;
    // 00443490 BSP_DeviceClass_ResolveAndActivate (ECX = id, DL = flag) -> class.
    virtual void* device_class_resolve_00443490(std::uint32_t id, std::uint8_t flag) = 0;
    // class vtable +28h (ECX = class, 0): create the device instance.
    virtual void* device_class_create_vslot_28(void* device_class, std::uint32_t argument) = 0;
    // object vtable +5Ch (ECX = object, kind) -> AL: is-kind-of.
    virtual bool object_is_kind_of_vslot_5c(void* object, std::uint32_t kind) = 0;
    // device vtable +1BCh (ECX = device, dword), +1C0h (ECX = device, float), +98h (ECX =
    // device, unit, (unit+30h) value, const float matrix[16]).
    virtual void device_vslot_1bc(void* device, std::uint32_t value) = 0;
    virtual void device_vslot_1c0(void* device, float value) = 0;
    virtual void device_attach_vslot_98(void* device, void* unit, std::uint32_t unit_30,
                                        const float* matrix) = 0;
    // 0048F930 vector<bool>::operator[] (ECX = vector, out iterator {container, word*, bit},
    // index), RET 8; returns the out iterator.
    virtual void* bool_vector_at_0048f930(void* vector, void* out_iterator,
                                          std::uint32_t index) = 0;

    // --- 0096EDD0 -------------------------------------------------------------------------
    // 00BF9A50 strcspn (cdecl).
    virtual std::uint32_t strcspn_00bf9a50(const char* text, const char* set) = 0;
    // 00469840 BSP_NativeString_Substring (ECX = string, out, start, count), RET 0Ch -> out.
    virtual void* native_string_substring_00469840(void* string, void* out, std::uint32_t start,
                                                   std::uint32_t count) = 0;
    // 004261A0 BSP_NativeString_Concat (ECX = left, out, right), RET 8 -> out.
    virtual void* native_string_concat_004261a0(void* left, void* out, const void* right) = 0;

    // --- 00A38DA0 -------------------------------------------------------------------------
    // 00652A50 / 00652A40 (ECX = (interface [00E198C4])+BCh, flag), RET 4: one-store setters.
    virtual void interface_flag_a_00652a50(void* object, std::uint32_t flag) = 0;
    virtual void interface_flag_b_00652a40(void* object, std::uint32_t flag) = 0;
    // 0068B3F0 BSP_InGameInterface_CollapseLevel3WhenSpectating (ECX = interface).
    virtual void ingame_interface_collapse_0068b3f0(void* interface) = 0;
    // 004BCA50 BSP_Game_GetEffectiveGameMode.
    virtual std::int32_t game_effective_mode_004bca50() = 0;
    // 00946970 (ECX = [00F89B3C], slot), RET 4, float in ST0.
    virtual float player_slot_value_00946970(void* manager, std::int32_t slot) = 0;
    // 009FFD20 BSP_Unit_LossCountingSlot (ECX = player object).
    virtual std::uint32_t unit_loss_counting_slot_009ffd20(void* player) = 0;
    // 006599C0 STL_List_BuyHeadNode (ECX = list) -> head.
    virtual void* list_buy_head_node_006599c0(void* list) = 0;
    // 0066E590 (ECX = record+28h value, EDX = list*, slot), RET 4: fills the slot's list.
    virtual void slot_fill_list_0066e590(void* source, void* list, std::int32_t slot) = 0;
    // 0084D560 BSP_SupportSite_BuildSupplyClassList (ECX = class, out list*, source, 0), RET 0Ch.
    virtual void support_site_build_supply_class_list_0084d560(void* vehicle_class, void* out_list,
                                                               void* source,
                                                               std::uint32_t flags) = 0;
    // 00964790 (ECX = value, DL = 1) -> object.
    virtual void* resolve_00964790(void* value, std::uint8_t flag) = 0;
    // 0041E870 BSP_NativeString_Assign (ECX = string, const char*), RET 4.
    virtual void native_string_assign_0041e870(void* string, const char* text) = 0;
    // 009FFF60 (ECX = object, 3, 0, NativeString*), RET 0Ch.
    virtual std::uint32_t object_query_009fff60(void* object, std::uint32_t a, std::uint32_t b,
                                                void* name) = 0;
    // 0094C900 (ECX = [00F89B3C], slot, object, class, 3, value), RET 14h: the spawn request.
    virtual void player_spawn_0094c900(void* manager, std::int32_t slot, void* object,
                                       void* vehicle_class, std::uint32_t mode,
                                       std::uint32_t value) = 0;
    // 004E8150 STL_List_Clear (ECX = list) / 006654C0 STL_List_ClearNested (ECX = list).
    virtual void list_clear_004e8150(void* list) = 0;
    virtual void list_clear_nested_006654c0(void* list) = 0;

    // --- 00A46930 -------------------------------------------------------------------------
    // XLive system vtable +19Ch (ECX = system) -> AL.
    virtual bool xlive_vslot_19c(void* system) = 0;
    // 004BEB60 std::string::compare(pos, n, const char*, count) (ECX = string), RET 10h.
    virtual std::int32_t string_compare_004beb60(void* string, std::uint32_t pos, std::uint32_t n,
                                                 const char* text, std::uint32_t count) = 0;
    // 00A456D0 list buy node (ECX = list, next, prev, const value*), RET 0Ch -> node.
    virtual void* player_list_buy_node_00a456d0(void* list, void* next, void* prev,
                                                const void* value) = 0;
    // 00A45150 list incsize (ECX = list, count), RET 4.
    virtual void player_list_incsize_00a45150(void* list, std::uint32_t count) = 0;
    // 00A45C60 BSP_XLiveSystem_RefreshFriends (ECX = system).
    virtual void xlive_refresh_friends_00a45c60(void* system) = 0;
    // 00A3EB00 BSP_XenonSystemManager_GetActiveXuid (ECX = manager) -> EDX:EAX.
    virtual std::uint64_t xenon_active_xuid_00a3eb00(void* manager) = 0;
    // 00A49B90 BSP_XLiveSession_JoinRemotePlayer (ECX = system+2098h, xuid*, invited), RET 8.
    virtual void xlive_session_join_remote_00a49b90(void* session, const std::uint64_t* xuid,
                                                    std::uint32_t invited) = 0;
    // 00A39C50 BSP_cNetworkClient_JoinLocalPlayer (ECX = client+C8h, invited), RET 4.
    virtual void network_client_join_local_00a39c50(void* client, std::uint32_t invited) = 0;
    // 00A396E0 BSP_cNetworkClient_JoinRemotePlayer (ECX = client+C8h, xuid*, invited), RET 8.
    virtual void network_client_join_remote_00a396e0(void* client, const std::uint64_t* xuid,
                                                     std::uint32_t invited) = 0;
    // 00A3E510 HasSelectedUser / 00A3EAD0 GetSelectedSignInState / 00A3F500
    // RequestDownloadStorage (ECX = [00F8ABE8]).
    virtual bool xenon_has_selected_user_00a3e510(void* manager) = 0;
    virtual std::int32_t xenon_selected_sign_in_state_00a3ead0(void* manager) = 0;
    virtual void xlive_request_download_storage_00a3f500(void* manager) = 0;

    // --- seg 73 dialog manager ------------------------------------------------------------
    // 00A77FF0 BSP_SoundStream_CopyName (ECX = stream, out NativeString*), RET 4 -> out.
    virtual void* sound_stream_copy_name_00a77ff0(void* stream, void* out) = 0;
    // 00435C40 BSP_NativeString_EqualsInsensitive (ECX = string, other*), RET 4 -> AL.
    virtual bool native_string_equals_insensitive_00435c40(void* string, const void* other) = 0;
    // channel vtable +0 (ECX = channel) -> float in ST0: the channel's base volume.
    virtual float channel_volume_vslot_00(void* channel) = 0;
    // 00A864F0 BSP_SoundStream_SetGain (ECX = stream, float), RET 4.
    virtual void sound_stream_set_gain_00a864f0(void* stream, float gain) = 0;
    // 004254B0 the log printf (cdecl, format + arguments), with the two shapes used here.
    virtual void log_printf_text_int_004254b0(const char* format, const char* text,
                                              std::int32_t value) = 0;
    virtual void log_printf_int_int_004254b0(const char* format, std::int32_t first,
                                             std::int32_t second) = 0;
    // 00A78230 BSP_LogicalDialogRecord_Assign (ECX = record, const NativeString*), RET 4.
    virtual void logical_dialog_record_assign_00a78230(void* record, const void* name) = 0;
    // 00A77BB0 BSP_LogicalDialogConfiguration_ClearReferences (ECX = configuration).
    virtual void logical_dialog_configuration_clear_00a77bb0(void* configuration) = 0;
    // 00A782E0 BSP_MusicDirector_SelectRowAndStoreName (ECX = director, record+8), RET 4.
    virtual void music_director_select_row_00a782e0(void* director, void* field) = 0;
    // 004C87F0 BSP_StreamedDialogRecord_Destroy (ECX = record).
    virtual void streamed_dialog_record_destroy_004c87f0(void* record) = 0;
    // 00A78DC0 BSP_DialogChannelRecords_Reserve (ECX = records, count), RET 4.
    virtual void dialog_channel_records_reserve_00a78dc0(void* records, std::int32_t count) = 0;
    // 00A77F80 BSP_DialogChannelRecord_Construct (ECX = record).
    virtual void dialog_channel_record_construct_00a77f80(void* record) = 0;
    // 00A79150 BSP_SoundStreamTable_Construct (ECX = memory, NativeString* filename), RET 4.
    virtual void* sound_stream_table_construct_00a79150(void* memory, const void* filename) = 0;

    // --- seg 74 input / text listener -----------------------------------------------------
    // input device vtable +30h (ECX = device) -> the active control or -1; +24h (ECX = device,
    // control), RET 4 -> float in ST0: that control's value.
    virtual std::int32_t input_device_active_control_vslot_30(void* device) = 0;
    virtual float input_device_control_value_vslot_24(void* device, std::int32_t control) = 0;
    // 0042BE90 |*value| (ECX = const float*), ST0.
    virtual float fabs_0042be90(const float* value) = 0;
    // 00A92EE0 BSP_NativeInputModifierVector_Assign (ECX = dest, const src*), RET 4.
    virtual void input_modifier_vector_assign_00a92ee0(void* dest, const void* src) = 0;
    // 00A93070 BSP_InputBinding_Destruct (ECX = binding).
    virtual void input_binding_destruct_00a93070(void* binding) = 0;
    // 00A96FB0 tree erase(first, last) (ECX = tree, out iterator*, first, last), RET 14h.
    virtual void tree_erase_range_00a96fb0(void* tree, TreeIter* out, TreeIter first,
                                           TreeIter last) = 0;
    // 00A964D0 tree buy head node (ECX = tree) -> head.
    virtual void* tree_buy_head_node_00a964d0(void* tree) = 0;

    // --- seg 79 gui ------------------------------------------------------------------------
    // 00AC9F10 grid cell destroy (ECX = 14h-byte cell).
    virtual void gui_grid_cell_destroy_00ac9f10(void* cell) = 0;
    // 00ACB870 vector<cell>::resize (ECX = vector, count, 14h-byte cell by value), RET 18h.
    virtual void gui_grid_cells_resize_00acb870(void* vector, std::int32_t count,
                                                const void* value) = 0;
    // 005A5A80 vector<dword>::resize (ECX = vector, count, value), RET 8.
    virtual void dword_vector_resize_005a5a80(void* vector, std::uint32_t count,
                                              std::uint32_t value) = 0;
    // widget vtable +84h (ECX = widget): relayout.
    virtual void gui_widget_vslot_84(void* widget) = 0;
    // 00AAAED0 BSP_GuiWidget_DescribeProperties (ECX = widget, sink), RET 4.
    virtual void gui_widget_describe_properties_00aaaed0(void* widget, void* sink) = 0;
    // 00BD5680 BSP_TaggedValue_Equals (ECX = left*, right*), RET 4 -> AL.
    virtual bool tagged_value_equals_00bd5680(const cc13_sweepC::ArchiveArg* left,
                                              const cc13_sweepC::ArchiveArg* right) = 0;
    // property sink vtable +0Ch (ECX = sink, flags, const char* name, {kind, value} by value),
    // RET 10h.
    virtual void property_sink_vslot_0c(void* sink, std::uint32_t flags, const char* name,
                                        cc13_sweepC::ArchiveArg value) = 0;
    // 00AD40D0 tree iterator ++ (ECX = iterator) / 00AD41F0 tree _Erase(subtree) (ECX = tree,
    // node), RET 4.
    virtual void tree_iter_inc_00ad40d0(TreeIter* iter) = 0;
    virtual void tree_erase_subtree_00ad41f0(void* tree, void* node) = 0;
    // 00AD4C30 BSP_Font_LoadGlyphDataAndTextures (ECX = font, root, font+20h, font+30h,
    // font+28h, extra), RET 14h.
    virtual void font_load_glyph_data_00ad4c30(void* font, const void* root, void* a, void* b,
                                               void* c, const void* extra) = 0;

    // --- seg 80/81 terrain -----------------------------------------------------------------
    // terrain vtable +40h (ECX = terrain) -> its scene node.
    virtual void* terrain_node_vslot_40(void* terrain) = 0;
    // 00B6E0D0 BSP_Node_GetInverseWorldMatrix (ECX = node) -> matrix.
    virtual void* node_inverse_world_matrix_00b6e0d0(void* node) = 0;
    // 004142E0 BSP_Vector3f_TransformAffinePoint (ECX = point, out, matrix), RET 8 -> out.
    virtual float* vector3_transform_affine_point_004142e0(const float* point, float* out,
                                                           const void* matrix) = 0;
    // 00AECC40 (ECX = terrain, EDX = from, to, out), RET 8 -> AL: the vertical-segment case.
    virtual bool terrain_vertical_segment_hit_00aecc40(void* terrain, const float* from,
                                                       const float* to, float* out) = 0;
    // 00AEA2B0 BSP_TerrainQuadtree_RayWalk (ECX = terrain+88h, from, to, hit), RET 0Ch -> AL.
    virtual bool terrain_quadtree_ray_walk_00aea2b0(void* quadtree, const float* from,
                                                    const float* to, float* hit) = 0;
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix (ECX = node).
    virtual void transform_refresh_world_matrix_00b6db70(void* node) = 0;
    // 00AE9870 BSP_TerrainWeakLink_Construct, the terrain base (ECX = terrain, manager), RET 4.
    virtual void terrain_base_construct_00ae9870(void* terrain, void* manager) = 0;
    // 00AEA8C0 BSP_TerrainQuadtree_Construct (ECX = terrain+88h).
    virtual void terrain_quadtree_construct_00aea8c0(void* quadtree) = 0;
    // 00ADBE60 pointer array resize (ECX = array, count), RET 4.
    virtual void pointer_array_resize_00adbe60(void* array, std::uint32_t count) = 0;
    // 00B6ED70 BSP_NativeNode_AllocateRawSlot (ECX = byte count) -> memory.
    virtual void* native_node_allocate_00b6ed70(std::uint32_t bytes) = 0;
    // 00B6F5A0 BSP_Node_Construct (ECX = memory, const NativeString* name), RET 4 -> node.
    virtual void* node_construct_00b6f5a0(void* memory, const void* name) = 0;
    // node vtable +38h (ECX = node, dword), RET 4.
    virtual void node_vslot_38(void* node, std::uint32_t value) = 0;
    // 00B6D890 BSP_Node_PropagateRootRegistration (ECX = node, root), RET 4.
    virtual void node_propagate_root_00b6d890(void* node, std::uint32_t root) = 0;
    // 00B6E680 BSP_Node_SetParent (ECX = (model+0Ch) node, child node), RET 4.
    virtual void node_set_parent_00b6e680(void* node, void* other) = 0;
    // 0043C130 BSP_NativeString_ConstructPrefixedHeader (ECX = out, EDX = const char* prefix,
    // const NativeString* right), RET 4 -> out.
    virtual void* native_string_construct_prefixed_0043c130(void* out, const char* prefix,
                                                             const void* right) = 0;
    // 00ADE500 BSP_TerrainHeightField_LoadForModel (ECX = terrain, const NativeString* path,
    // float, float, 0), RET 10h.
    virtual void terrain_height_field_load_00ade500(void* terrain, const void* path, float first,
                                                    float second, std::uint32_t flags) = 0;

    // --- seg 81/82 --------------------------------------------------------------------------
    // 00BE99D0 BSP_StructuredNode_ReadFloat (ECX = reader) -> float in ST0.
    virtual float structured_node_read_float_00be99d0(void* reader) = 0;
    // grid vtable +1Ch (ECX = grid, float value, column, row), RET 0Ch.
    virtual void grid_set_cell_vslot_1c(void* grid, float value, std::int32_t column,
                                        std::int32_t row) = 0;
    // 004BCC00 BSP_String_LowercaseAscii (ECX = NativeString).
    virtual void string_lowercase_ascii_004bcc00(void* string) = 0;
    // 00B1A4F0 load_native_sampler_cache (ECX = loader+4, NativeString*, options, 1, 1), RET 10h.
    virtual std::uint32_t sampler_cache_load_00b1a4f0(void* cache, const void* name,
                                                      std::uint32_t options, std::uint32_t a,
                                                      std::uint32_t b) = 0;
    // Lua object wrappers (each a 14h-byte LuaObject; ECX = the object).
    // 00B67980 BSP_LuaStateOwner_GetGlobals (ECX = owner+4, out object*), RET 4.
    virtual void lua_state_get_globals_00b67980(void* owner, void* out) = 0;
    virtual void lua_object_construct_00b65f50(void* object) = 0;
    // 00B67080 IterateFirst / 00B67190 IterateNext (ECX = table, key*, value*), RET 8.
    virtual void lua_object_iterate_first_00b67080(void* table, void* key, void* value) = 0;
    virtual void lua_object_iterate_next_00b67190(void* table, void* key, void* value) = 0;
    // 00B66420 BSP_LuaObject_IsUnbound (ECX = table, value*), RET 4 -> AL.
    virtual bool lua_object_is_unbound_00b66420(void* table, void* value) = 0;
    virtual bool lua_object_is_string_00b660a0(void* object) = 0;
    virtual const char* lua_object_get_string_00b662b0(void* object) = 0;
    // 00B674C0 BSP_LuaObject_SetStringField (ECX = table, const NativeString* name, value), RET 8.
    virtual void lua_object_set_string_field_00b674c0(void* table, const void* name,
                                                      void* value) = 0;
    virtual void lua_object_destruct_00b67700(void* object) = 0;
    // 004CDC20 BSP_NativeStringVector_Append (ECX = vector, const NativeString*), RET 4.
    virtual void native_string_vector_append_004cdc20(void* vector, const void* string) = 0;
    // 00427110 BSP_NativeStringVector_Resize (ECX = vector, count), RET 4.
    virtual void native_string_vector_resize_00427110(void* vector, std::uint32_t count) = 0;
    // 00BF6989 operator delete.
    virtual void operator_delete_00bf6989(void* block) = 0;

    // --- seg 75 ------------------------------------------------------------------------------
    // 00BF55BE operator new (the array allocation).
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    // 00A9E970 node block construct (ECX = 7804h-byte block, block index), RET 4 -> block.
    virtual void* string_map_node_block_construct_00a9e970(void* memory, std::int32_t index) = 0;

    // --- 00818EA0 -------------------------------------------------------------------------
    // 00BF701A _CIatan2: ST1 = y, ST0 = x.
    virtual double crt_atan2_00bf701a(double y, double x) = 0;
    // 00867B10 BSP_PointEffect_StopChildren (ECX = effect).
    virtual void point_effect_stop_children_00867b10(void* effect) = 0;
    // 00868420 BSP_PointEffect_CreateFromMatrix, __fastcall(ECX = out slot, EDX = [globals+19ECh],
    // consumed handle by value, const float* matrix, 0, 0), RET 10h -> out slot.
    virtual void** point_effect_create_from_matrix_00868420(void** out_slot, std::uint32_t owner,
                                                            void* consumed_handle,
                                                            const float* matrix, std::uint32_t a,
                                                            std::uint32_t b) = 0;
    // 00413920 BSP_Matrix_Multiply4x4 (ECX = left, out, right), RET 8 -> out.
    virtual float* matrix_multiply_00413920(const void* left, float* out, const void* right) = 0;
    // 004134F0 BSP_Matrix_Copy4x4X87 (ECX = dest, src), RET 4.
    virtual void matrix_copy_004134f0(void* dest, const void* src) = 0;
    // unit vtable +50h (ECX = unit) -> float in ST0.
    virtual float unit_vslot_50_float(void* unit) = 0;
    // 00810020 BSP_UnitPoseHistoryRing_Fill (ECX = unit+BD0h, unit+FCh, float), RET 8.
    virtual void pose_history_ring_fill_00810020(void* ring, const void* pose, float value) = 0;
};

// 006CC800 BSP_AirOpsStockSnapshot_Capture. __thiscall(ECX = snapshot, block), RET 4.
void air_ops_stock_snapshot_capture_006cc800(SweepC2Host& host, void* snapshot, void* block);

// 006CD810 BSP_AirOps_AiLaunchTick. __thiscall(ECX = block, float dt unused), RET 4.
void air_ops_ai_launch_tick_006cd810(SweepC2Host& host, void* block, float dt_unused);

// 006CFAF0 BSP_MotherShipSite_Construct. __thiscall(ECX = site, mothership), RET 4; returns site.
void* mothership_site_construct_006cfaf0(SweepC2Host& host, void* site, void* mothership);

// 006D3F20 BSP_AirField_ParkSpotTest. __thiscall(ECX = airfield, float x, float y, float z),
// RET 0Ch; AL = 1 when an aim point lies within the radius.
bool airfield_park_spot_test_006d3f20(SweepC2Host& host, void* airfield, float x, float y,
                                      float z);

// 008196B0 BSP_UnitInstance_BroadcastRelocate (unit vtable +11Ch). __thiscall(ECX = unit,
// float heading), RET 4.
void unit_broadcast_relocate_008196b0(SweepC2Host& host, void* unit, float heading);

// 00818EA0 BSP_UnitInstance_ResetPoseDependentState. __thiscall(ECX = unit), RET.
void unit_reset_pose_dependent_state_00818ea0(SweepC2Host& host, void* unit);

// 0095A880 BSP_Unit_InstantiateClassDevices. __thiscall(ECX = unit, list, byte all, vector<bool>*
// mask), RET 0Ch; AL = 1 always.
bool unit_instantiate_class_devices_0095a880(SweepC2Host& host, void* unit, void* list,
                                             std::uint8_t instantiate_class_devices, void* mask);

// 0096EDD0 native-string token replace. __thiscall(ECX = unused, NativeString* text, const
// NativeString* token, const NativeString* insert), RET 0Ch.
void native_string_replace_token_0096edd0(SweepC2Host& host, void* unused_this, void* text,
                                          const void* token, const void* insert);

// 00A38DA0 BSP_Session_SpawnPlayerVehicles (support-manager ban switch). __fastcall(CL =
// banned), RET.
void session_spawn_player_vehicles_00a38da0(SweepC2Host& host, bool banned);

// 00A46930 BSP_XLiveSystem_JoinPlayer (vtable 00D249D0 slot +158h). __thiscall(ECX = system,
// player record*, invited), RET 8.
void xlive_system_join_player_00a46930(SweepC2Host& host, void* system, void* player,
                                       std::uint32_t invited);

// Segment 73, the streamed dialog / music manager.
// 00A78330 BSP_StreamedDialogManager_SetChannelVolumeByName. __thiscall(ECX = channel,
// const NativeString* name, float volume), RET 8.
void dialog_channel_set_volume_by_name_00a78330(SweepC2Host& host, void* channel,
                                                const void* name, float volume);
// 00A786F0 BSP_StreamedDialogManager_RequestMusic. __thiscall(ECX = director, const
// NativeString* name, ref handle by value (released here), float fade), RET 0Ch.
void dialog_request_music_00a786f0(SweepC2Host& host, void* director, const void* name,
                                   void* handle, float fade);
// 00A788A0 BSP_MusicDirector_SetLevel. __thiscall(ECX = manager, level), RET 4.
void music_director_set_level_00a788a0(SweepC2Host& host, void* manager, std::int32_t level);
// 00A78CE0 BSP_StreamedDialogManager_PlayDialogStream. __thiscall(ECX = manager, const
// NativeString* name, float unused), RET 8.
void dialog_play_stream_00a78ce0(SweepC2Host& host, void* manager, const void* name,
                                 float unused);
// 00A78EC0 BSP_DialogChannelRecords_Resize. __thiscall(ECX = records, count), RET 4.
void dialog_channel_records_resize_00a78ec0(SweepC2Host& host, void* records,
                                            std::int32_t count);
// 00A79480 BSP_StreamedDialogManager_SetSlotRecord. __thiscall(ECX = manager, const
// NativeString* path, party, state), RET 0Ch.
void dialog_set_slot_record_00a79480(SweepC2Host& host, void* manager, const void* path,
                                     std::int32_t party, std::int32_t state);

// Segment 74.
// 00A90530 BSP_InputBackend_ReadEvent. __thiscall(ECX = backend, int* device_group, int* zero,
// int* control, float* value), RET 10h; AL = (*control != -1).
bool input_backend_read_event_00a90530(SweepC2Host& host, void* backend, std::int32_t* group,
                                       std::int32_t* zero, std::int32_t* control, float* value);
// 00A93690 BSP_InputAction_RemoveMatchingBinding. __thiscall(ECX = action, const descriptor*),
// RET 4.
void input_action_remove_matching_binding_00a93690(SweepC2Host& host, void* action,
                                                   const void* descriptor);
// 00A97140 BSP_TextListenerBase_Destroy. __thiscall(ECX = listener), RET.
void text_listener_base_destroy_00a97140(SweepC2Host& host, void* listener);
// 00A97260 BSP_TextListenerBase_Construct. __thiscall(ECX = listener), RET; returns listener.
void* text_listener_base_construct_00a97260(SweepC2Host& host, void* listener);

// Segment 79.
// 00ACB9B0 BSP_GuiGrid_SetCellCount. __thiscall(ECX = grid, count), RET 4.
void gui_grid_set_cell_count_00acb9b0(SweepC2Host& host, void* grid, std::int32_t count);
// 00ACD8E0 BSP_GuiSoundRefPtr_CopyAssign and 00ACD9E0 BSP_GuiSound_Assign: the same body for two
// handle types. __fastcall(ECX = dest slot, EDX = source slot), RET; returns dest.
void** gui_sound_ref_copy_assign_00acd8e0(SweepC2Host& host, void** dest, void* const* source);
void** gui_sound_assign_00acd9e0(SweepC2Host& host, void** dest, void* const* source);
// 00ACE6D0 BSP_GuiClipBox_DescribeProperties (vtable 00D5D074). __thiscall(ECX = clip box,
// sink), RET 4.
void gui_clip_box_describe_properties_00ace6d0(SweepC2Host& host, void* box, void* sink);
// 00AD51D0 BSP_FontRegistry_ReplacePath. __thiscall(ECX = font, const NativeString* root, const
// NativeString* extra), RET 8. EAX is left as the last callee set it; the one caller (00AC3643)
// does not read it.
void font_registry_replace_path_00ad51d0(SweepC2Host& host, void* font, const void* root,
                                         const void* extra);

// Segments 80/81.
// 00ADA240 BSP_TerrainHeightField_SegmentHit (terrain vtable slot 3Ch). __thiscall(ECX =
// terrain, const float* from, const float* to, float* out), RET 0Ch; AL = hit.
bool terrain_height_field_segment_hit_00ada240(SweepC2Host& host, void* terrain,
                                               const float* from, const float* to, float* out);
// 00ADD290 BSP_TerrainHeightField_Construct. __thiscall(ECX = terrain, manager, NativeString
// name BY VALUE {length, data}, released here), RET 0Ch; returns terrain.
void* terrain_height_field_construct_00add290(SweepC2Host& host, void* terrain, void* manager,
                                              std::uint32_t name_length, char* name_data);
// 00ADE820 BSP_TerrainHeightField_AttachModel. __thiscall(ECX = terrain, const NativeString*
// name, model handle, float first, float second, dword node_value), RET 14h.
void terrain_height_field_attach_model_00ade820(SweepC2Host& host, void* terrain, const void* name,
                                                void* model, float first, float second,
                                                std::uint32_t node_value);
// 00A9F0B0 BSP_StringMapNodePool_Allocate. __thiscall(ECX = pool), RET; returns a 1Ch-byte node.
void* string_map_node_pool_allocate_00a9f0b0(SweepC2Host& host, void* pool);
// 00AED100 grid fill from a structured reader. __thiscall(ECX = grid, reader), RET 4.
void grid_read_cells_00aed100(SweepC2Host& host, void* grid, void* reader);

// Segment 82.
// 00B1B400 BSP_SamplerLoader_LoadWithOptions. __thiscall(ECX = loader, const NativeString* name,
// options), RET 8 -> the cache answer.
std::uint32_t sampler_loader_load_with_options_00b1b400(SweepC2Host& host, void* loader,
                                                        const void* name, std::uint32_t options);
// 00B1BD20 BSP_LuaGlobals_SetFieldsNamedByStringEntries. __thiscall(ECX = owner, value), RET 4.
void lua_globals_set_fields_named_by_strings_00b1bd20(SweepC2Host& host, void* owner,
                                                      void* value);

}  // namespace bsp::cc13_sweepC2

#endif  // BSP_CC13_SWEEPC2_HPP
