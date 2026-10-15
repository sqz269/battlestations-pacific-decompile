// Segment 70 (00A16050..00A371A0), reconstruction pass (packet cc13-seg70).
//
// The high-level AI coordinator's leaves: the party brain and planner object lifetimes, the
// planner constructors, the AI group's command installers, member removal, the Lua info fill
// behind AIGetGroupInfo, the stripped Capture/Defend debug passes, the forced-target-weight rule
// insert, the coordinator's party-state broadcast and its small dynamic arrays.
// docs/CC13_SEG70.md carries the evidence and the status of every routine.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in the
// image) and reaches the engine through Seg70Host, whose methods name the image routine, virtual
// slot or global they stand for. Routines the cc9 gameplay hosts already implement by behaviour
// (00A1E210, 00A2C8D0, 00A2D440, 00A2DB50, 00A2DDE0, 00A2DFA0) and the table-driven class test
// 00A31C60 are recorded ledger-only and are not reconstructed again here. New C++ interfaces, not
// drop-in binary replacements; nothing here is bound into bsp_game.

#ifndef BSP_CC13_SEG70_HPP
#define BSP_CC13_SEG70_HPP

#include <cstddef>
#include <cstdint>

#include "bsp/ingame_gui_cc13.hpp"

namespace bsp::cc13_seg70 {

using bsp::ingame_gui_cc13::Raw;

// ---------------------------------------------------------------------------
// Image addresses and layouts the routines touch.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kGameGlobal = 0x00E188A8;          // *: game; +19CCh the world lists
inline constexpr std::uint32_t kPartyBrainSlots = 0x00F8A89C;     // eight brain pointers ..00F8A8BC
inline constexpr std::uint32_t kPartyBrainSlotsEnd = 0x00F8A8BC;
inline constexpr std::uint32_t kPartyRecords = 0x00F8A8C8;        // 1Ch per party
inline constexpr std::size_t kPartyRecordStride = 0x1C;
inline constexpr std::uint32_t kPartyAggressiveRatio = 0x00F8A8D0; // record +8h, float
inline constexpr std::uint32_t kPartyHasSpawnPoint = 0x00F8A8D4;   // record +0Ch, byte
inline constexpr std::uint32_t kPartySpawnPoint = 0x00F8A8D8;      // record +10h, float[3]
inline constexpr std::uint32_t kCurrentPartyIndex = 0x00E0E344;   // dword
inline constexpr std::uint32_t kDebugTeam = 0x00E0E354;           // dword, the team 00A16CC0 asks
inline constexpr std::uint32_t kSpawnDisabled = 0x00F8AB6A;       // byte, 00A25B90 returns when set
inline constexpr std::uint32_t kSpawnRequestManager = 0x00F89B3C; // *: 00946970's ECX
inline constexpr std::uint32_t kEmptiedGroupList = 0x00F8AA78;    // std::list<group*>, head at +4h
inline constexpr std::uint32_t kCaptureDebugColourGuard = 0x00F8A9B8;  // bit 0; colour 00F8A9A8..B4
inline constexpr std::uint32_t kCaptureDebugColour = 0x00F8A9A8;
inline constexpr std::uint32_t kPlannerDebugColourGuard = 0x00F8A9D0;  // bit 0; colour 00F8A9C0..CC
inline constexpr std::uint32_t kPlannerDebugColour = 0x00F8A9C0;
inline constexpr std::uint32_t kDefaultTextStyleGuard = 0x00F8AAF8;    // bit 0
inline constexpr std::uint32_t kDefaultTextStyle = 0x00F8AAE8;         // float[4]
inline constexpr std::uint32_t kCoordinatorSingleton = 0x00F8AAFC;
inline constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818;
inline constexpr std::uint32_t kCoordinatorLiveFlag = 0x00F8AB04;      // dword, 00A32460 clears
inline constexpr std::uint32_t kGlobalForcedRules = 0x00F8AB5C;        // {data, count, capacity}
inline constexpr std::uint32_t kGlobalForcedRuleCount = 0x00F8AB60;
inline constexpr std::uint32_t kModeForcedRules = 0x00F8AB08;          // + mode * 0Ch
inline constexpr std::size_t kModeForcedRuleStride = 0x0C;
inline constexpr std::size_t kForcedRuleSize = 0x20;
inline constexpr std::uint32_t kUnitTypeNames = 0x00E0B590;            // const char*[]
inline constexpr float kOne = 1.0f;          // 00D7A24C (00 00 80 3F)
inline constexpr float kMinusOne = -1.0f;    // 00D7A260 (00 00 80 BF)
inline constexpr float kHalf = 0.5f;         // 00CE3800 (00 00 00 3F)

// Entity (MissionEntity) fields read here.
inline constexpr std::size_t kEntityParentPose = 0x3C;   // pose object, null at the root
inline constexpr std::size_t kEntitySide = 0x54;         // int
inline constexpr std::size_t kEntityLocalMatrix = 0x74;  // float[16]
inline constexpr std::size_t kEntityPoseClean = 0xC8;    // byte
inline constexpr std::size_t kEntityWorldMatrix = 0xCC;  // float[16]
inline constexpr std::size_t kEntityPoseChanged = 0x10C; // byte
inline constexpr std::size_t kEntityAiGroup = 0x16C;     // AI group pointer
inline constexpr std::size_t kEntityPlayerSlot = 0x180;  // int, 0..7 or another value
inline constexpr std::size_t kUnitMessageSlot = 0x528;   // int, 00A31C90 stores -1
inline constexpr std::size_t kShipClass = 0x538;         // ship class pointer

// AI group (5660h bytes).
inline constexpr std::size_t kGroupSize = 0x5660;
inline constexpr std::size_t kGroupObserver = 0x10;      // observer sub-object
inline constexpr std::size_t kGroupPartySlot = 0x5634;
inline constexpr std::size_t kGroupTeam = 0x5638;
inline constexpr std::size_t kGroupMembers = 0x563C;     // std::list<entity*>, head at +5640h
inline constexpr std::size_t kGroupMemberHead = 0x5640;
inline constexpr std::size_t kGroupPopulation = 0x5644;
inline constexpr std::size_t kGroupAutoGrouping = 0x5648; // byte
inline constexpr std::size_t kGroupCommand = 0x564C;
inline constexpr std::size_t kGroupOwnerPlanner = 0x5654;

// AI command object virtuals (docs/AI_COMMAND_OBJECT.md).
inline constexpr std::uint32_t kSellingCommandVtable = 0x00D229B8;
inline constexpr std::uint32_t kRetreatCommandVtable = 0x00D22A60;
inline constexpr std::uint32_t kMoveCommandBaseVtable = 0x00D22A94;
inline constexpr std::uint32_t kRegroupingMoveVtable = 0x00D22B14;
inline constexpr int kCommandTypeAttack = 0x06;
inline constexpr int kCommandTypeRetreat = 0x0D;
inline constexpr int kCommandTypeSelling = 0x0E;

// Planner object (38h bytes, 44h for Capture). The std::list<group*> sits at +20h, head +24h,
// size +28h; the name {length, data} at +14h.
inline constexpr std::uint32_t kPlannerBaseVtable = 0x00D22D08;
inline constexpr std::size_t kPlannerName = 0x14;
inline constexpr std::size_t kPlannerBrain = 0x1C;
inline constexpr std::size_t kPlannerGroups = 0x20;
inline constexpr std::size_t kPlannerGroupHead = 0x24;
inline constexpr std::size_t kPlannerGroupCount = 0x28;
inline constexpr std::size_t kPlannerChanged = 0x2C;     // byte
inline constexpr std::size_t kPlannerSide = 0x30;
inline constexpr std::size_t kPlannerEnemySide = 0x34;
inline constexpr std::size_t kBrainSide = 0x24;
inline constexpr std::size_t kBrainSpawnKey = 0x20;

// AI controller (the coordinator entity).
inline constexpr std::size_t kControllerTickElement = 0x170;

// The engine's 8-byte string {length, data}; storage returns to the sized pool as
// (data, length + 1).
struct NativeStringRaw {
    std::uint32_t length;
    char* data;
};

// A LuaObject is 14h bytes (the image keeps them 14h apart on the stack).
struct LuaObjectRaw {
    std::uint8_t bytes[0x14];
};

// The std::vector<record> 00A08420 fills: an allocator dword, then first/last/end.
struct VectorRaw {
    std::uint32_t allocator;
    void* first;
    void* last;
    void* end;
};

// A std::list object: an allocator dword, the head node, the size.
struct ListRaw {
    std::uint32_t allocator;
    void* head;
    std::uint32_t size;
};

// A std::map/set object: an allocator dword, the head node, the size. The head node holds
// left +0h, parent +4h, right +8h, the colour +14h and the is-nil byte +15h.
struct TreeRaw {
    std::uint32_t allocator;
    void* head;
    std::uint32_t size;
};
// A checked tree iterator: {container, node}.
struct TreeIterRaw {
    void* container;
    void* node;
};

// ---------------------------------------------------------------------------
// Engine services. Each method names the image routine, virtual slot or global it stands for.
// ---------------------------------------------------------------------------
struct Seg70Host {
    virtual ~Seg70Host() = default;

    // Image globals: the memory at that image address.
    virtual Raw global(std::uint32_t image_address) = 0;

    // CRT.
    virtual void* operator_new_00bf681b(std::size_t size) = 0;
    virtual void* operator_new_00bf55be(std::size_t size) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void memcpy_00bf7680(void* destination, const void* source, std::size_t size) = 0;

    // Native strings and the sized pool.
    virtual void native_string_resize_0041dd40(NativeStringRaw& string, std::uint32_t length, int keep) = 0;
    virtual void native_string_construct_0041e870(NativeStringRaw& string, const char* text) = 0;
    // 00419CC0 (pool singleton) then its 00BD1510(data, size, 1).
    virtual void pool_return_00bd1510(void* data, std::uint32_t size) = 0;

    // Object virtuals, ECX = object.
    virtual void scalar_deleting_destructor_0(void* object, int flags) = 0;   // vtable +0
    virtual bool is_kind_of_5c(void* entity, int class_id) = 0;              // entity vtable +5Ch
    virtual bool command_is_type_08(void* command, int type) = 0;            // command vtable +8h
    virtual void* command_clone_for_group_20(void* command, void* group) = 0; // command vtable +20h
    virtual void command_forward_24(void* command, std::uint32_t a, std::uint32_t b) = 0;  // +24h, RET 8
    virtual void command_fill_lua_18(void* command, LuaObjectRaw* table) = 0; // command vtable +18h

    // std::list helpers (ECX = the list object).
    virtual void* list_buy_head_node_00a168a0(void* list) = 0;
    virtual void* list_buy_head_node_004c1630(void* list) = 0;
    virtual void* list_buy_node_00a16ab0(void* list, void* next, void* prev, void* const* value) = 0;
    virtual void* list_buy_node_004c2220(void* list, void* next, void* prev, void* const* value) = 0;
    virtual void list_incsize_00a172a0(void* list, std::uint32_t count) = 0;  // throws past max_size
    virtual void list_incsize_004cee30(void* list, std::uint32_t count) = 0;
    virtual void list_remove_00a1d1d0(void* list, void* const* value) = 0;
    virtual void list_remove_0077bea0(void* list, void* const* value) = 0;

    // std::map helpers.
    virtual void* tree_buy_head_node_005827f0(TreeRaw* tree) = 0;
    virtual void tree_iter_inc_0057f4a0(TreeIterRaw* iterator) = 0;
    virtual void tree_erase_range_0058ad40(TreeRaw* tree, TreeIterRaw* out, TreeIterRaw first,
                                           TreeIterRaw last) = 0;

    // Observer pairs (ECX = subject, EDX = observer).
    virtual void observer_register_pair_00694a60(void* subject, void* observer) = 0;
    virtual void observer_unregister_pair_006952a0(void* subject, void* observer) = 0;
    virtual void observer_unregister_if_registered_006956a0(void* subject, void* observer) = 0;

    // Pose.
    virtual void entity_pose_refresh_world_00414db0(void* pose) = 0;              // ECX = pose
    virtual float* matrix_multiply_00413920(float* lhs, float* out, const float* rhs) = 0;  // ECX = lhs
    virtual void matrix_copy_004134f0(float* destination, const float* source) = 0;          // ECX = dst

    // AI routines outside this packet (existing reconstructions or hosts).
    virtual bool planner_owns_group_00a1c8b0(void* planner, void* group) = 0;
    virtual bool group_any_member_matches_00a2c600(void* group) = 0;
    virtual void group_add_entity_00a2d8e0(void* group, void* entity) = 0;
    virtual void* group_construct_00a2dfa0(void* block, void* first_entity) = 0;
    virtual bool group_is_regrouping_to_00a2c1c0(void* group, const float* point) = 0;
    virtual float group_recon_ratio_00a2c3d0(void* group) = 0;
    virtual void group_collect_members_in_world_set_00a2def0(void* group, int team, ListRaw* out) = 0;
    virtual void group_count_unit_types_00a2e1e0(void* group, TreeRaw* out) = 0;
    // __fastcall(ECX = side, EDX = entity)(float* out, NativeStringRaw* explain), RET 8, ST0.
    virtual float planner_strategic_gain_00a243d0(int side, void* entity, float* out,
                                                  NativeStringRaw* explain) = 0;
    virtual float planner_capture_target_score_00a1e250(int side, void* entity, float* out,
                                                        NativeStringRaw* explain) = 0;
    // ECX = [00F89B3C], (brain+20h), ST0.
    virtual float spawn_queued_requests_00946970(void* manager, std::uint32_t key) = 0;
    // __fastcall(ECX = side, DL = flag)(a, VectorRaw* out, 0, b).
    virtual void collect_spawn_candidates_00a08420(int side, std::uint8_t flag, std::uint32_t a,
                                                   VectorRaw* out, std::uint32_t zero, std::uint32_t b) = 0;
    // __thiscall(planner)(position, amount, VectorRaw* candidates, name, flag).
    virtual void planner_try_spawn_group_00a25a30(void* planner, const float* position, float amount,
                                                  VectorRaw* candidates, std::uint32_t name,
                                                  std::uint32_t flag) = 0;
    virtual int ai_effective_game_mode_index_009ffc80() = 0;
    virtual void* coordinator_get_or_create_004c1c50() = 0;
    virtual unsigned game_effective_game_mode_004bca50(void* game) = 0;  // ECX = [00E188A8]
    virtual bool party_ai_enabled_009ffe50(int slot) = 0;                 // ECX = slot
    virtual void forced_target_weight_clear_00a038c0() = 0;

    // Lua.
    virtual LuaObjectRaw* entity_lua_self_object_00927b40(void* entity, LuaObjectRaw* out) = 0;
    virtual void lua_destruct_00b67700(LuaObjectRaw* object) = 0;
    virtual void lua_set_object_00b675d0(void* table, NativeStringRaw* name, LuaObjectRaw* value) = 0;
    virtual void lua_set_new_table_00b67580(void* table, NativeStringRaw* name) = 0;
    virtual void lua_get_by_name_00b67800(void* table, LuaObjectRaw* out, const char* name) = 0;
    virtual void lua_set_object_at_index_00b666c0(LuaObjectRaw* table, int index, LuaObjectRaw* value) = 0;
    virtual void lua_set_boolean_00b673a0(void* table, NativeStringRaw* name, std::uint32_t value) = 0;
    virtual void lua_set_integer_00b67460(void* table, NativeStringRaw* name, std::int32_t value) = 0;

    // Session messages and units.
    virtual void session_message_construct_base_0075b430(void* message, int class_id) = 0;  // ECX = msg
    virtual void unit_route_message_0077c2a0(void* unit, void* message, int a, int b) = 0;   // ECX = unit

    // Coordinator teardown.
    virtual void unit_tick_element_destruct_00875490(void* element) = 0;
    virtual void entity_destruct_base_00925780(void* entity) = 0;
};

// ---------------------------------------------------------------------------
// Party brains (the brain is eight planner pointers, +0h..+1Ch).
// ---------------------------------------------------------------------------
// 00A16490 BSP_AiPartyBrain_DestructBody, __fastcall(ECX = brain).
void party_brain_destruct_body_00a16490(Seg70Host& host, Raw brain);
// 00A16990 BSP_AiPartyBrain_Delete, __fastcall(ECX = void** slot).
void party_brain_delete_00a16990(Seg70Host& host, void** slot);
// 00A16B20 BSP_Ai_DestroyPartyBrains, void().
void destroy_party_brains_00a16b20(Seg70Host& host);
// 00A184F0 BSP_Ai_PartyAggressiveRatio, __fastcall(ECX = party slot), ST0.
float party_aggressive_ratio_00a184f0(Seg70Host& host, int party);

// ---------------------------------------------------------------------------
// Planner objects.
// ---------------------------------------------------------------------------
// 00A1EE50 BSP_AiPlanner_ConstructBase, __thiscall(planner)(brain), RET 4. Returns the planner.
void* planner_construct_base_00a1ee50(Seg70Host& host, Raw planner, Raw brain);

enum class PlannerConstructKind { Defend, Attack, Sell, Capture, Duel, Escort, Siege, Competitive };
// 00A1EFC0 / 00A1F0F0 / 00A1F220 / 00A1F390 / 00A1F500 / 00A1F630 / 00A1F760 / 00A1F890, the eight
// planner constructors, __thiscall(planner)(brain), RET 4. Each runs 00A1EE50, installs its vtable
// and copies its literal name into planner+14h; Capture also clears +38h/+3Ch and sets +40h = -1.0.
void* planner_construct_00a1efc0_family(Seg70Host& host, PlannerConstructKind kind, Raw planner, Raw brain);
std::uint32_t planner_construct_address(PlannerConstructKind kind) noexcept;
std::uint32_t planner_vtable(PlannerConstructKind kind) noexcept;
const char* planner_literal_name(PlannerConstructKind kind) noexcept;

// 00A1F310 BSP_AiPlanner_Sell_ReleaseGroup, __thiscall(planner)(group), RET 4 (the Sell vtable's
// +24h; the same body as the hosted 00A1E210).
void planner_sell_release_group_00a1f310(Seg70Host& host, Raw planner, void* group);

// 00A22920 BSP_AiPlanner_SplitGroupsByClass, __fastcall(ECX = planner). Unreferenced.
void planner_split_groups_by_class_00a22920(Seg70Host& host, Raw planner);

// 00A25B90 BSP_AiPlanner_QuickSpawn, __thiscall(planner)(name, flag, a, b), RET 10h.
void planner_quick_spawn_00a25b90(Seg70Host& host, Raw planner, std::uint32_t name, std::uint8_t flag,
                                  std::uint32_t a, std::uint32_t b);

// The stripped debug passes: the scores are computed and the strings freed; nothing is drawn.
// 00A16CC0 BSP_AiDebug_CaptureScoresStripped, void(). Unreferenced.
void debug_capture_scores_stripped_00a16cc0(Seg70Host& host);
// 00A1FC30 BSP_AiPlanner_Capture_DebugDrawStripped, __fastcall(ECX = planner) (Capture slot +28h).
void planner_capture_debug_draw_stripped_00a1fc30(Seg70Host& host, Raw planner);
// 00A25CB0 BSP_AiPlanner_Defend_DebugDrawStripped, __fastcall(ECX = planner) (Defend slot +28h).
void planner_defend_debug_draw_stripped_00a25cb0(Seg70Host& host, Raw planner);

// ---------------------------------------------------------------------------
// AI group commands (group+564Ch) and membership.
// ---------------------------------------------------------------------------
// 00A2BD00 BSP_AiGroup_SetCommand, __thiscall(group)(command), RET 4.
void group_set_command_00a2bd00(Seg70Host& host, Raw group, void* command);
// 00A2BD50 BSP_AiGroup_CopyCommandFrom, __thiscall(group)(other), RET 4.
void group_copy_command_from_00a2bd50(Seg70Host& host, Raw group, Raw other);
// 00A2BD90 BSP_AiGroup_ForwardCommand, __thiscall(group)(a, b), RET 8.
void group_forward_command_00a2bd90(Seg70Host& host, Raw group, std::uint32_t a, std::uint32_t b);
// 00A2BDB0 BSP_AiGroup_IsAttackingTarget, __thiscall(group)(target_group), RET 4.
bool group_is_attacking_target_00a2bdb0(Seg70Host& host, Raw group, void* target_group);
// 00A2BE10 BSP_AiGroup_IsSelling, __thiscall(group), AL.
bool group_is_selling_00a2be10(Seg70Host& host, Raw group);
// 00A2BE70 BSP_AiGroup_OrderRetreat, __fastcall(ECX = group). Unreferenced.
void group_order_retreat_00a2be70(Seg70Host& host, Raw group);
// 00A185E0 BSP_AiGroup_OrderSelling, (group), RET 4; ECX is not read. Unreferenced.
void group_order_selling_00a185e0(Seg70Host& host, Raw group);
// 00A2C2A0 BSP_AiGroup_OrderRegroupingMove, __thiscall(group)(point, unused), RET 8. Unreferenced.
void group_order_regrouping_move_00a2c2a0(Seg70Host& host, Raw group, const float* point, std::uint32_t unused);
// 00A2D9D0 BSP_AiGroup_RemoveEntity, __thiscall(group)(entity, queue_if_empty), RET 8.
void group_remove_entity_00a2d9d0(Seg70Host& host, Raw group, void* entity, std::uint8_t queue_if_empty);
// 00A2EEE0 BSP_AiGroup_FillLuaInfo, __thiscall(group)(LuaObject* table), RET 4.
void group_fill_lua_info_00a2eee0(Seg70Host& host, Raw group, void* table);
// 00A2C050 BSP_AiDebug_DefaultTextStyle, void(), returns 00F8AAE8.
Raw debug_default_text_style_00a2c050(Seg70Host& host);

// ---------------------------------------------------------------------------
// The coordinator (AI controller) and its tables.
// ---------------------------------------------------------------------------
// 00A317C0 BSP_AiController_TickElement, __fastcall(ECX = controller): this+170h or null.
void* controller_tick_element_00a317c0(void* controller);
// 00A31800 BSP_GameSingleton_00f8aafc_ResetSingleton, __fastcall(ECX = object).
void coordinator_reset_singleton_00a31800(Seg70Host& host, Raw object);
// 00A31C20 BSP_AiControllerMessage_Construct, __thiscall(message)(value), RET 4.
void* controller_message_construct_00a31c20(Seg70Host& host, Raw message, std::uint32_t value);
// 00A31C90 BSP_AiController_SendPartyMessage, __thiscall(controller, unread)(unit, value), RET 8.
void controller_send_party_message_00a31c90(Seg70Host& host, Raw unit, std::uint32_t value);
// 00A329F0 BSP_AiController_BroadcastPartyState, __thiscall(controller). Unreferenced.
void controller_broadcast_party_state_00a329f0(Seg70Host& host);
// 00A32460 BSP_AiController_DestructBody, __fastcall(ECX = controller).
void controller_destruct_body_00a32460(Seg70Host& host, Raw controller);
// 00A32500 BSP_Ai_ForcedTargetWeightInsert, __fastcall(ECX = 20h-byte rule, DL = global)(mode), RET 4.
void forced_target_weight_insert_00a32500(Seg70Host& host, const std::uint8_t* rule, bool global_table,
                                          int mode);
// 00A371A0 BSP_Ai_ActiveModeTuning, void(): coordinator + 4 + mode * 23Ch.
void* active_mode_tuning_00a371a0(Seg70Host& host);

// The coordinator's {data, count, capacity} arrays (operator new 00BF55BE, free 00BF6989).
struct ArrayRaw {
    void* data;
    std::uint32_t count;
    std::uint32_t capacity;
};
// 00A31A40 BSP_Array32_Grow, __fastcall(ECX = array): 20h-byte elements, capacity 2n+2.
void array32_grow_00a31a40(Seg70Host& host, ArrayRaw& array);
// 00A31B10 BSP_ArrayDword_Reserve, __thiscall(array)(n), RET 4.
void array_dword_reserve_00a31b10(Seg70Host& host, ArrayRaw& array, std::uint32_t capacity);
// 00A32120 BSP_Array32_PushBack, __thiscall(array)(const element*), RET 4.
void array32_push_back_00a32120(Seg70Host& host, ArrayRaw& array, const void* element);
// 00A32200 BSP_ArrayDword_Grow, __fastcall(ECX = array).
void array_dword_grow_00a32200(Seg70Host& host, ArrayRaw& array);
// 00A32880 BSP_ArrayDword_PushBack, __thiscall(array)(value), RET 4.
void array_dword_push_back_00a32880(Seg70Host& host, ArrayRaw& array, std::uint32_t value);

}  // namespace bsp::cc13_seg70

#endif
