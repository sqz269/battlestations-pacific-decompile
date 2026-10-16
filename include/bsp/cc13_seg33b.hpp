// Segment 33, upper half, reconstruction pass (packet cc13-seg33b; marker cc13_seg33b).
//
// Range 006EC000..006FA320: the MCatapult device (construct, debug dump, saved state, launch
// heading, launch message), the std::map<key, LuaVariant> / std::vector<LuaVariant> template
// instances the mission Lua bridge uses, the debug side swap, the fog-cloud model pool, the
// MCommandBuilding helpers (garrison and paratrooper vectors, landing pads, inferior-list
// session message, Lua class reader, repair, construct/destruct, the land-fort shared
// virtuals), the command list helpers and the convex-object stream readers.
// docs/CC13_SEG33B.md carries the evidence, the status of every routine and the adoption
// notes for the CommandBuilding routines the cc9 gameplay hosts already implement.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as
// in the image) and reaches the engine through Seg33bHost, whose methods name the image
// routine, global or virtual slot they stand for. New C++ interfaces, not drop-in binary
// replacements; nothing here is bound into bsp_game.

#ifndef BSP_CC13_SEG33B_HPP
#define BSP_CC13_SEG33B_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg33b {

static_assert(sizeof(void*) == 4, "segment 33 objects are modelled in their Win32 layout");

using Object = std::uint8_t*;

template <class T>
inline T& at(Object object, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(object + offset);
}
inline Object obj(std::uint32_t value) noexcept {
    return reinterpret_cast<Object>(static_cast<std::uintptr_t>(value));
}
inline std::uint32_t addr(const void* pointer) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer));
}

// A tagged value as the archive visitor and the Lua reader take it by value: {tag, value};
// tag 0 a string pointer, 1 an int (or int*), 3 a bool (or bool*). For a by-value bool the
// image writes only the low byte and leaves the other three as stack contents; modelled as 0.
struct TaggedValue {
    std::uint32_t tag;
    std::uint32_t value;
};

// The 8-byte checked iterator of the image's std::map: {container, node}.
struct TreeIterator {
    Object container;
    Object node;
};
// std::pair<iterator, bool> as 006EE4E0 writes it (0Ch bytes, bool at +8h).
struct TreeInsertResult {
    TreeIterator where;
    std::uint8_t inserted;
    std::uint8_t pad[3];
};

// ---------------------------------------------------------------------------
// Image addresses and constants.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kGameGlobals = 0x00E188A8;        // the game object pointer
inline constexpr std::uint32_t kIdSplit = 0x00F89A10;            // int, ids below use bank 0
inline constexpr std::uint32_t kIdBank0Base = 0x00F89A0C;        // int
inline constexpr std::uint32_t kIdBank0Records = 0x00F89A54;     // 10h-byte records, entity at +0Ch
inline constexpr std::uint32_t kIdBank1Base = 0x00F89A60;
inline constexpr std::uint32_t kIdBank1Records = 0x00F89AA8;
inline constexpr std::uint32_t kAllocatorListHead = 0x00E188B4;  // Mit::allocator_list_elem list
inline constexpr std::uint32_t kFogCloudPool = 0x00E199B0;       // the static fog-cloud pool
inline constexpr std::uint32_t kAllocatorListElemVtable = 0x00D7A0C0;
inline constexpr std::uint32_t kFogCloudPoolVtable = 0x00CFAD6C;  // slot 0 = 006F08C0
inline constexpr std::uint32_t kHudRoot = 0x00E198C4;            // [kHudRoot]+40h is ECX of 006485A0

// MCatapult (0x4D8 bytes; class id 28h).
inline constexpr std::uint32_t kCatapultVtable = 0x00CFAAB8;
inline constexpr std::uint32_t kCatapultVtable10 = 0x00CFAA9C;
inline constexpr std::uint32_t kCatapultVtable24 = 0x00CFAA94;
inline constexpr std::uint32_t kCatapultVtable170 = 0x00CFAA90;
inline constexpr std::uint32_t kCatapultVtable1E4 = 0x00CFAA88;
inline constexpr std::uint32_t kCatapultVtable310 = 0x00CFAA70;
inline constexpr std::uint32_t kObserverMemberVtable = 0x00CF6514;  // the member at +494h
inline constexpr std::size_t kCatapultSize = 0x4D8;
inline constexpr std::size_t kCatapultAmmo = 0x480;          // int, "ammo"
inline constexpr std::size_t kCatapultOrgAmmo = 0x484;       // int, "orgAmmo"
inline constexpr std::size_t kCatapultStartLaunch = 0x488;   // bool, "startLaunch"
inline constexpr std::size_t kCatapultLaunchTick = 0x490;    // int, "launchTick", -1 idle
inline constexpr std::size_t kCatapultObserver = 0x494;      // observer member; target at +14h
inline constexpr std::size_t kCatapultLaunched = 0x4A8;      // = observer +14h, the squadron
inline constexpr std::size_t kCatapultDefaultHeading = 0x4AC;  // float, 006ED000's fallback
inline constexpr float kLaunchHeadingWindow = 10.0f;         // 00CE38B8
inline constexpr double kMinusPi = -3.1415927410125732;      // double 00CE3D18 (float pi widened)
inline constexpr double kPi = 3.1415927410125732;            // double 00CE3D28
inline constexpr double kTwoPi = 6.2831854820251465;         // double 00CE3828
inline constexpr std::uint32_t kFltMaxBits = 0x7F7FFFFFu;    // float 00CFA9D8

// Fog-cloud pool block: two 194h-byte slots, a uint16 free stack at +328h and the free count
// at +32Ch (block 330h bytes); every slot keeps its block index at slot+190h.
inline constexpr std::size_t kFogBlockBytes = 0x330;
inline constexpr std::size_t kFogSlotBytes = 0x194;
inline constexpr std::uint16_t kFogSlotsPerBlock = 2;
inline constexpr std::size_t kFogFreeStack = 0x328;
inline constexpr std::size_t kFogFreeCount = 0x32C;
inline constexpr std::size_t kFogSlotIndex = 0x190;
inline constexpr float kTwoTo32 = 4294967296.0f;             // float 00CE3978 (4F800000h)

// ---------------------------------------------------------------------------
// Engine services. Each method names the image routine, global or virtual slot.
// ---------------------------------------------------------------------------
class Seg33bHost {
public:
    virtual ~Seg33bHost() = default;

    // Image globals: the 32-bit cell at an image address (BSS / .data).
    virtual std::uint32_t& global_u32(std::uint32_t address) = 0;

    // CRT. 00BF55BE / 00BF681B operator new, 00BF6989 / 00BF65AC free, 00BF79F0 memset,
    // 00BF7680 memcpy, 00BF857A _CIfmod (x87 fmod of ST1 by ST0).
    virtual Object operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual Object operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void memset_00bf79f0(void* block, int value, std::uint32_t bytes) = 0;
    virtual void memcpy_00bf7680(void* to, const void* from, std::uint32_t bytes) = 0;
    virtual double fmod_00bf857a(double x, double y) = 0;
    // Win32 imports: [00CE220C] InitializeCriticalSection, [00CE2214] DeleteCriticalSection,
    // [00CE2218] EnterCriticalSection, [00CE2210] LeaveCriticalSection.
    virtual void initialize_critical_section(void* section) = 0;
    virtual void delete_critical_section(void* section) = 0;
    virtual void enter_critical_section(void* section) = 0;
    virtual void leave_critical_section(void* section) = 0;
    // 00419CC0(data, bytes, 1) then 00BD1510 on the pool it returns: the inlined string free.
    virtual void pool_release_00419cc0_00bd1510(void* data, std::uint32_t bytes) = 0;
    // 0041DD40 BSP_NativeString_Resize, __thiscall(string, length, 1), RET 8.
    virtual void native_string_resize_0041dd40(Object string, std::uint32_t length, std::uint32_t flag) = 0;
    // 00BD2F10 __thiscall(ECX = stream)(low, high), RET 8, ST0: the process-wide RNG.
    virtual float random_uniform_00bd2f10(std::uint32_t stream, float low, float high) = 0;

    // Observer pairs. 006952A0 BSP_Observer_UnregisterPair(ECX = subject, EDX = member);
    // 00694A60 BSP_Observer_RegisterPair(ECX = subject, EDX = member).
    virtual void observer_unregister_006952a0(Object subject, Object member) = 0;
    virtual void observer_register_00694a60(Object subject, Object member) = 0;
    // 00653390 BSP_ObserverMember_Cleanup(ECX = member).
    virtual void observer_member_cleanup_00653390(Object member) = 0;

    // Archive / debug visitor, ECX = visitor, 8-byte tagged values by value:
    // +4h begin_section(key), +8h end_section(), +0Ch write_field(key, value).
    virtual void visitor_begin_section_4(Object visitor, TaggedValue key) = 0;
    virtual void visitor_end_section_8(Object visitor) = 0;
    virtual void visitor_write_field_c(Object visitor, TaggedValue key, TaggedValue value) = 0;

    // The saved-entity Lua reader. 009238A0 SEntity_GetSavedEntityLuaData(ECX = entity, out
    // 14h LuaObject), RET 4; 004425C0 BSP_LuaReader_Construct(ECX = reader, LuaObject by value
    // 14h), RET 14h; 00BD8E20 EnterKey(key) RET 8; 00BD6830 ReadField(key, value) RET 10h;
    // 00BD7A20 LeaveKey(); 00441A20 the reader destructor.
    virtual void saved_entity_lua_data_009238a0(Object entity, Object out_lua_object) = 0;
    virtual void lua_reader_construct_004425c0(Object reader, Object lua_object_by_value) = 0;
    virtual void lua_reader_enter_00bd8e20(Object reader, TaggedValue key) = 0;
    virtual void lua_reader_read_00bd6830(Object reader, TaggedValue key, TaggedValue value) = 0;
    virtual void lua_reader_leave_00bd7a20(Object reader) = 0;
    virtual void lua_reader_destruct_00441a20(Object reader) = 0;

    // Gun base (MCatapult derives from it). ECX = gun throughout.
    virtual void gun_debug_dump_fields_0072adc0(Object gun, Object visitor) = 0;  // RET 4
    virtual void gun_construct_0072e510(Object gun, std::uint32_t argument) = 0;   // RET 4
    virtual void gun_destruct_0072d950(Object gun) = 0;
    virtual void gun_load_saved_state_0072f060(Object gun) = 0;
    // 0072CF00 __thiscall(gun, int barrel, float time, int set_full), RET 0Ch.
    virtual void gun_set_barrel_reload_timer_0072cf00(Object gun, std::int32_t barrel, float time,
                                                      std::int32_t set_full) = 0;
    // 006EC070 BSP_MCatapult_AfterLaunch, __thiscall(catapult, value), RET 4.
    virtual void catapult_after_launch_006ec070(Object catapult, std::uint32_t value) = 0;
    // 005471B0 BSP_RefCountedPointerArray_Resize, __thiscall(array, n), RET 4.
    virtual void pointer_array_resize_005471b0(Object array, std::int32_t count) = 0;
    // 007F5960 BSP_GunArc_HorizontalAngleInWindow, __thiscall(arc, float angle), RET 4 -> AL.
    virtual bool gun_arc_angle_in_window_007f5960(Object arc, float angle) = 0;
    // 00438B10 BSP_Math_SubtractWrappedAngle, __stdcall(float a, float b), RET 8, ST0.
    virtual float subtract_wrapped_angle_00438b10(float a, float b) = 0;

    // std::vector<LuaVariant> / std::map<key, LuaVariant> element routines (outside the range).
    // 00884460 key less, __thiscall(ECX = a)(b), RET 4 -> AL.
    virtual bool lua_key_less_00884460(Object a, Object b) = 0;
    // 00884EF0 key copy-construct, __thiscall(ECX = to)(from), RET 4.
    virtual void lua_key_copy_construct_00884ef0(Object to, Object from) = 0;
    // 00886DA0 LuaVariant copy-construct, __thiscall(ECX = to)(from), RET 4.
    virtual void lua_variant_copy_construct_00886da0(Object to, Object from) = 0;
    // 00884AF0 LuaVariant destructor, __thiscall(ECX = variant).
    virtual void lua_variant_destruct_00884af0(Object variant) = 0;
    // 006EDA20 / 006EDD80 tree iterator ++ / -- on {container, node}, __thiscall(ECX = it).
    virtual void tree_iterator_increment_006eda20(TreeIterator* it) = 0;
    virtual void tree_iterator_decrement_006edd80(TreeIterator* it) = 0;
    // 006EDEA0 allocate the 34h-byte head node, __thiscall(ECX = map) -> EAX.
    virtual Object tree_allocate_head_006edea0(Object map) = 0;
    // 006EE2E0 _Insert, __thiscall(map)(out iterator*, bool add_left, node where, value*),
    // RET 10h -> EAX = out.
    virtual TreeIterator* tree_insert_006ee2e0(Object map, TreeIterator* out, bool add_left, Object where,
                                               Object value) = 0;
    // 006B87F0 vector reserve(n), 006B88E0 vector resize(n), __thiscall(ECX = vector), RET 4.
    virtual void lua_variant_vector_reserve_006b87f0(Object vector, std::int32_t count) = 0;
    virtual void lua_variant_vector_resize_006b88e0(Object vector, std::int32_t count) = 0;

    // Debug side swap (006EDB40) callees.
    // 008053C0 BSP_Recon_EnsureSlot, __fastcall(ECX = side) -> EAX.
    virtual std::uint32_t recon_ensure_slot_008053c0(std::uint32_t side) = 0;
    // 008073C0 BSP_Recon_RebuildSlotLists, __thiscall(ECX = slot).
    virtual void recon_rebuild_slot_lists_008073c0(std::uint32_t slot) = 0;
    // Slot 0 of the sub-object at unit+1E4h, __thiscall(ECX = unit+1E4h)(side, a, b), RET 0Ch.
    virtual void unit_sub1e4_slot0(Object sub_object, std::uint32_t side, std::uint32_t a, std::uint32_t b) = 0;
    // 004C3CB0 BSP_Game_BuildLocalPlayerUnitLists, __thiscall(ECX = game).
    virtual void game_build_local_player_unit_lists_004c3cb0(Object game) = 0;
    // 006485A0 BSP_InGameHudRoot_ForceSelectUnit, __thiscall(ECX = [[00E198C4]+40h]), tail jump.
    virtual void hud_force_select_unit_006485a0(Object hud) = 0;

    // Fog-cloud model. 00B748E0 BSP_NativeGeneratedModel_Render, __thiscall(model)(a1, float a2,
    // float visibility, 0), RET 10h. 00B750C0 BSP_Model_Destroy, __thiscall(ECX = model).
    virtual void generated_model_render_00b748e0(Object model, std::uint32_t a1, float a2, float visibility,
                                                 std::uint32_t zero) = 0;
    virtual void model_destroy_00b750c0(Object model) = 0;

    // ---- MCommandBuilding callees ----
    // Object virtuals, ECX = object: slot 0 the deleting destructor (flags), RET 4; +5Ch
    // IsKindOf(kind) -> AL; +198h(name*, 0, out bool*), RET 0Ch; +12Ch() -> EAX;
    // +128h(value), RET 4.
    virtual void object_deleting_destructor_0(Object object, std::uint32_t flags) = 0;
    virtual bool object_is_kind_of_5c(Object object, std::int32_t kind) = 0;
    virtual void object_virtual_198(Object object, Object name, std::uint32_t zero, std::uint8_t* out) = 0;
    virtual std::uint32_t object_virtual_12c(Object object) = 0;
    virtual void object_virtual_128(Object object, std::uint32_t value) = 0;
    // 00BD1860 BSP_CriticalSection_Create -> EAX (a {CRITICAL_SECTION, depth +18h} block);
    // 0041CC80 BSP_CriticalSection_DestroyOwned(ECX = &cell).
    virtual Object critical_section_create_00bd1860() = 0;
    virtual void critical_section_destroy_owned_0041cc80(Object cell) = 0;
    // 0059FD60 the std::list head node, __thiscall(ECX = list) -> EAX; 005A0B80 list tidy.
    virtual Object list_buy_head_0059fd60(Object list) = 0;
    virtual void list_tidy_005a0b80(Object list) = 0;
    // Landing pad: 006AC220 occupant (ECX = pad) -> EAX; 006AC4D0 refresh (ECX = pad).
    virtual Object pad_occupant_006ac220(Object pad) = 0;
    virtual void pad_refresh_006ac4d0(Object pad) = 0;
    // Garrison entry (64h bytes) parts: 00545F50 observer-handle copy, __thiscall(ECX = to)
    // (from), RET 4; 008F41F0 BSP_ScenePropertyBag_Clone(ECX = bag) -> EAX; 00545600 the
    // entry's base destructor (ECX); 00695870 BSP_CallbackOwner_Destroy (ECX).
    virtual void observer_handle_copy_00545f50(Object to, Object from) = 0;
    virtual std::uint32_t property_bag_clone_008f41f0(Object bag) = 0;
    virtual void garrison_entry_base_destruct_00545600(Object entry) = 0;
    virtual void callback_owner_destroy_00695870(Object owner) = 0;
    // 0048CBB0 __thiscall(ECX = [[00E188A8]+21D0h])(unit+1E4h or null), RET 4 -> AL.
    virtual bool pad_group_query_0048cbb0(Object service, Object unit_sub) = 0;
    // uint16 array {data, size, capacity}: 005296A0 reserve(n), 00529980 resize(n), RET 4.
    virtual void word_array_reserve_005296a0(Object array, std::int32_t count) = 0;
    virtual void word_array_resize_00529980(Object array, std::int32_t count) = 0;
    // Session messages: 0075B430 construct base(ECX = message)(id), RET 4; 0075B480 write /
    // 0075B4C0 read the extended header(ECX = message)(stream), RET 4.
    virtual void session_message_construct_0075b430(Object message, std::uint32_t id) = 0;
    virtual void session_message_write_header_0075b480(Object message, Object stream) = 0;
    virtual void session_message_read_header_0075b4c0(Object message, Object stream) = 0;
    // Bit cursor: 00428FF0 write byte(value, bits), 00429120 write word(value, bits),
    // 00428C70 read byte(out*, bits), 00428E30 read word(out*, bits); ECX = cursor, RET 8.
    virtual void bit_write_byte_00428ff0(Object cursor, std::uint8_t value, std::uint32_t bits) = 0;
    virtual void bit_write_word_00429120(Object cursor, std::uint16_t value, std::uint32_t bits) = 0;
    virtual void bit_read_byte_00428c70(Object cursor, std::uint8_t* out, std::uint32_t bits) = 0;
    virtual void bit_read_word_00428e30(Object cursor, std::uint16_t* out, std::uint32_t bits) = 0;
    // Lua class reading. 00749210 BSP_StructureClass_ReadLuaFields(ECX = class)(lua), RET 4.
    // LuaObject (14h bytes): 00B67800 GetByName(ECX = object)(out, name), RET 8 -> EAX = out;
    // 00B66380 GetIntegerOrDefault(ECX)(int), RET 4 -> EAX; 00B66330 GetFloatOrDefault(ECX)
    // (float), RET 4 -> ST0; 00B66270 GetNumber -> ST0; 00B66050 IsNumber -> AL; 00B660A0
    // IsString -> AL; 00B662B0 GetString -> EAX; 00B67700 the destructor.
    virtual void structure_class_read_lua_00749210(Object klass, Object lua) = 0;
    virtual Object lua_get_by_name_00b67800(Object object, Object out, const char* name) = 0;
    virtual std::int32_t lua_get_int_or_default_00b66380(Object object, std::int32_t fallback) = 0;
    virtual float lua_get_float_or_default_00b66330(Object object, float fallback) = 0;
    virtual float lua_get_number_00b66270(Object object) = 0;
    virtual bool lua_is_number_00b66050(Object object) = 0;
    virtual bool lua_is_string_00b660a0(Object object) = 0;
    virtual const char* lua_get_string_00b662b0(Object object) = 0;
    virtual void lua_object_destruct_00b67700(Object object) = 0;
    // 004C1D10 BSP_GameSingleton_00e19a64_GetOrCreate(ECX = [00E188A8]) -> EAX.
    virtual Object game_tuning_singleton_004c1d10(Object game) = 0;
    // Engine strings {length, data}: 0041E870 assign(ECX = string)(cstr), RET 4;
    // 00425850 equals-insensitive(ECX = string)(cstr), RET 4 -> AL.
    virtual void native_string_assign_0041e870(Object string, const char* text) = 0;
    virtual bool native_string_equals_insensitive_00425850(Object string, const char* text) = 0;
    // 00745BE0 BSP_LandFort_FrameUpdate(ECX = fort)(float dt), RET 4; 0051F460
    // BSP_DebugFeatureOwner_GetSingleton -> EAX.
    virtual void land_fort_frame_update_00745be0(Object fort, float dt) = 0;
    virtual Object debug_feature_owner_0051f460() = 0;
    // Message handlers, ECX = building: 006F4D10 D3h(slot, byte), RET 8; 006F38E0 D4h(a, b),
    // RET 8; 006F29D0 D5h(float progress, value, side), RET 0Ch; 00744BE0
    // BSP_LandFort_HandleMessage(message), RET 4 -> AL.
    virtual void command_building_d3_006f4d10(Object building, std::uint32_t slot, std::uint8_t flag) = 0;
    virtual void command_building_d4_006f38e0(Object building, std::uint32_t a, std::uint32_t b) = 0;
    virtual void command_building_d5_006f29d0(Object building, float progress, std::uint32_t value,
                                              std::uint32_t side) = 0;
    virtual bool land_fort_handle_message_00744be0(Object fort, Object message) = 0;
    // 00521E30 BSP_EntityHandleTable_Resolve(ECX = id) -> EAX.
    virtual Object entity_handle_resolve_00521e30(std::uint32_t id) = 0;
    // HUD: 00644A60 selected list unit(ECX = [[00E198C4]+40h]) -> EAX; 00549430(ECX =
    // [[00E198C4]+50h]) the panel refresh.
    virtual Object hud_selected_unit_00644a60(Object list) = 0;
    virtual void hud_panel_refresh_00549430(Object panel) = 0;
    // Construction chain: 00745940 BSP_LandFort_Construct(ECX)(argument), RET 4; 008E35D0
    // powerup binding construct(ECX = +758h)(owner), RET 4; 00875A80 fixed-step register
    // (ECX = +310h)(1), RET 4; 00875AE0 unregister(ECX = +310h); 008E9A70 powerup binding
    // destructor(ECX = +758h); 00745AA0 BSP_LandFort_Destruct(ECX); 009553D0
    // BSP_Unit_SetVehicleClass(ECX = unit)(descriptor), RET 4; 00748CC0
    // BSP_LandFort_LoadSavedState(ECX); 006F5CC0 BSP_CommandBuilding_AdoptNearbyGarrison(ECX).
    virtual void land_fort_construct_00745940(Object fort, std::uint32_t argument) = 0;
    virtual void powerup_binding_construct_008e35d0(Object binding, Object owner) = 0;
    virtual void fixed_step_register_00875a80(Object node, std::uint32_t enable) = 0;
    virtual void fixed_step_unregister_00875ae0(Object node) = 0;
    virtual void powerup_binding_destruct_008e9a70(Object binding) = 0;
    virtual void land_fort_destruct_00745aa0(Object fort) = 0;
    virtual void unit_set_vehicle_class_009553d0(Object unit, Object descriptor) = 0;
    virtual void land_fort_load_saved_state_00748cc0(Object fort) = 0;
    virtual void command_building_adopt_nearby_garrison_006f5cc0(Object building) = 0;
    // World lists: 006D3620 BSP_UnitInstance_DetachFromEarlyWorldLists(ECX); 004837D0
    // BSP_UnitList_EraseNode(ECX = list)(node), RET 4.
    virtual void unit_detach_early_world_lists_006d3620(Object unit) = 0;
    virtual void unit_list_erase_node_004837d0(Object list, Object node) = 0;
    // 00BF6713 the checked-iterator failure (_invalid_parameter_noinfo path).
    virtual void invalid_iterator_00bf6713() = 0;
    // 006F2780 callees: 007482B0 BSP_LandFort_InitSceneAndCreateStaticBody(ECX); 008ED720
    // powerup binding init(ECX = +758h)(creation record), RET 4; 008F2260
    // BSP_ScenePropertyBag_Find(ECX = bag)(name), RET 4 -> EAX (value dword at entry+0Ch);
    // 004BCA50 BSP_Game_GetEffectiveGameMode(ECX = [00E188A8]) -> EAX.
    virtual void land_fort_init_scene_007482b0(Object fort) = 0;
    virtual void powerup_binding_init_008ed720(Object binding, Object record) = 0;
    virtual Object property_bag_find_008f2260(Object bag, const char* name) = 0;
    virtual std::int32_t effective_game_mode_004bca50(Object game) = 0;
    // Lua state owner (4DCh bytes in 006F7670's frame), ECX = owner: 00B66BD0 construct;
    // 00B6A020 open(libraries), RET 4; 00B69D40 run script(string*, 0), RET 8; 00B67980
    // globals(out LuaObject), RET 4 -> EAX = out; 00B669A0 close/destroy.
    virtual void lua_state_construct_00b66bd0(Object owner) = 0;
    virtual void lua_state_open_00b6a020(Object owner, std::uint32_t libraries) = 0;
    virtual void lua_state_run_script_00b69d40(Object owner, Object path, std::uint32_t zero) = 0;
    virtual Object lua_state_globals_00b67980(Object owner, Object out) = 0;
    virtual void lua_state_close_00b669a0(Object owner) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld(ECX = entity); 004142E0
    // BSP_Vector3f_TransformAffinePoint(ECX = point)(out, 4x4), RET 8 -> EAX = out.
    virtual void entity_pose_refresh_00414db0(Object entity) = 0;
    virtual float* transform_affine_point_004142e0(const float* point, float* out, const float* matrix) = 0;
    // 00467170 BSP_CommandRegistry_FindByName(ECX = name) -> EAX.
    virtual Object command_registry_find_00467170(const char* name) = 0;
    // Structured stream reading: vtable +10h(key, value) by value (ECX = stream), RET 10h;
    // 00BE9A80 read u32(ECX = node)(out*), RET 4; 00BE9A60 read float(ECX = node)(out*), RET 4
    // -> EAX = the node to chain on.
    virtual void stream_read_field_10(Object stream, TaggedValue key, TaggedValue value) = 0;
    virtual void node_read_u32_00be9a80(Object node, void* out) = 0;
    virtual Object node_read_float_00be9a60(Object node, float* out) = 0;
    // Convex object: 00B868B0 / 00B86890 resource-item base construct / destroy (ECX); 00C32D50
    // / 00C37450 the embedded hull at +0Ch construct / destroy (ECX = +0Ch); 00C356D0 the hull's
    // vertex list(ECX = hull)(vector*) and 00C35630 its neighbour index list(vector*), RET 4;
    // 0085FB30 debug line(ECX = from, EDX = to)(float colour[4] by value), RET 10h.
    virtual void resource_item_construct_00b868b0(Object item) = 0;
    virtual void resource_item_destroy_00b86890(Object item) = 0;
    virtual void convex_hull_construct_00c32d50(Object hull) = 0;
    virtual void convex_hull_destroy_00c37450(Object hull) = 0;
    virtual void convex_hull_vertices_00c356d0(Object hull, Object vector) = 0;
    virtual void convex_hull_neighbours_00c35630(Object hull, Object vector) = 0;
    virtual void debug_line_0085fb30(const float* from, const float* to, const float* colour) = 0;
    // 006EEB20 callees: 00B65FB0 LuaObject IsNil(ECX) -> AL; 00B662D0 ToUserdata(ECX) -> EAX;
    // 00884D90 key from a C string(ECX = key)(text), RET 4; 00884A00 LuaVariant from a dword
    // (ECX = variant)(value), RET 4 -> EAX = variant; 00886840 LuaVariant from a map(ECX =
    // variant)(map*), RET 4; 006EE960 map erase(first, last)(ECX = map)(out*, first iterator,
    // last iterator by value), RET 14h; 009291D0 the mission object's message call(ECX =
    // userdata)(argument vector*), RET 4.
    virtual bool lua_is_nil_00b65fb0(Object object) = 0;
    virtual Object lua_to_userdata_00b662d0(Object object) = 0;
    virtual void lua_key_from_text_00884d90(Object key, const char* text) = 0;
    virtual Object lua_variant_from_dword_00884a00(Object variant, std::uint32_t value) = 0;
    virtual void lua_variant_from_map_00886840(Object variant, Object map) = 0;
    virtual void lua_map_erase_range_006ee960(Object map, TreeIterator* out, TreeIterator first,
                                              TreeIterator last) = 0;
    virtual void mission_message_009291d0(Object userdata, Object arguments) = 0;
    // 006ECBE0 callees, ECX = catapult unless noted: 0043F7B0 BSP_DeviceEntity_InitHealthAndParts;
    // 0072C6A0 BSP_Gun_CreateAiBots; 0072DD20 BSP_Gun_PlatformMountFrame(out 4x4), RET 4 -> EAX;
    // 00729700 BSP_Gun_ApplyDescriptorNodeFlag; vtable +1BCh(value), RET 4; vtable +194h(name*,
    // 0), RET 8. Node: vtable +38h set local matrix(ECX = node)(4x4*), RET 4; 00B6E680
    // BSP_Node_SetParent(ECX = node)(parent), RET 4; 00B6ED70 BSP_NativeNode_AllocateRawSlot
    // (ECX = size) -> EAX; 00B6F5A0 BSP_Node_Construct(ECX = memory)(name*), RET 4 -> EAX;
    // 0071AD50 BSP_Model_FindNoteNode(ECX = model)(name), RET 4 -> EAX; 0042ED50
    // BSP_SceneNode_InvalidateSubtreePose(ECX = child); 004134F0 matrix copy(ECX = to)(from),
    // RET 4. +3Ch's vtable +18h() -> EAX, the parent node.
    virtual void device_init_health_and_parts_0043f7b0(Object device) = 0;
    virtual void gun_create_ai_bots_0072c6a0(Object gun) = 0;
    virtual float* gun_platform_mount_frame_0072dd20(Object gun, float* out) = 0;
    virtual void gun_apply_descriptor_node_flag_00729700(Object gun) = 0;
    virtual void unit_virtual_1bc(Object unit, std::uint32_t value) = 0;
    virtual void unit_virtual_194(Object unit, Object name, std::uint32_t zero) = 0;
    virtual void node_set_local_matrix_38(Object node, const float* matrix) = 0;
    virtual void node_set_parent_00b6e680(Object node, Object parent) = 0;
    virtual Object node_allocate_00b6ed70(std::uint32_t size) = 0;
    virtual Object node_construct_00b6f5a0(Object memory, Object name) = 0;
    virtual Object model_find_note_node_0071ad50(Object model, const char* name) = 0;
    virtual void scene_node_invalidate_0042ed50(Object node) = 0;
    virtual void matrix_copy_004134f0(float* to, const float* from) = 0;
    virtual Object object_virtual_18(Object object) = 0;
    // 006F1440 callees. Generic COM-style virtual calls on render objects, ECX = object:
    // render_virtual_3(object, slot, a, b, c) -> EAX and render_virtual_0(object, slot) -> EAX.
    // The renderer [00F8D394]: +38h vertex format(string*), +5Ch vertex buffer(count, flags,
    // format), +60h index buffer(count, flags, format), +48h effect(string*), +64h texture
    // (string*, 0). [00CE2220] InterlockedDecrement. 00B75030 BSP_GeneratedModel_Construct
    // (ECX = model)(name*), RET 4; 00B75170 BSP_GeneratedModel_SetGeometry(ECX = model)(0,
    // mesh, float, float), RET 10h; 00B73B60 / 00B18780 / 00B85EE0 raw-slot allocators (ECX =
    // size) -> EAX; 00B73D70 BSP_MeshObject_Construct(ECX); 00B18900
    // BSP_Material_ConstructWithEffect(ECX)(effect), RET 4; 00B857F0 BSP_DrawSection_Construct
    // (ECX); 00B73BB0 vertex stream(ECX = mesh)(0, buffer), RET 8; 00B73B70 index stream(ECX =
    // mesh)(buffer), RET 4; 00B189F0 texture slot(ECX = material)(0, texture), RET 8; 00B864C0
    // section material(ECX = section)(material), RET 4; 00B73C60 append section(ECX = mesh)
    // (section), RET 4; 00B732C0 element(ECX = mesh)(index), RET 4 -> EAX; 00B73C10 clear the
    // section owners(ECX = mesh); 00B6D890 root registration(ECX = node)(root), RET 4.
    virtual std::uint32_t render_virtual_3(Object object, std::uint32_t slot, std::uint32_t a, std::uint32_t b,
                                           std::uint32_t c) = 0;
    virtual std::uint32_t render_virtual_1(Object object, std::uint32_t slot, std::uint32_t a) = 0;
    virtual std::uint32_t render_virtual_2(Object object, std::uint32_t slot, std::uint32_t a, std::uint32_t b) = 0;
    virtual std::uint32_t render_virtual_0(Object object, std::uint32_t slot) = 0;
    virtual std::int32_t interlocked_decrement(std::int32_t* cell) = 0;
    virtual void generated_model_construct_00b75030(Object model, Object name) = 0;
    virtual void generated_model_set_geometry_00b75170(Object model, std::uint32_t zero, Object mesh, float a,
                                                       float b) = 0;
    virtual Object mesh_allocate_00b73b60(std::uint32_t size) = 0;
    virtual Object mesh_construct_00b73d70(Object memory) = 0;
    virtual Object material_allocate_00b18780(std::uint32_t size) = 0;
    virtual Object material_construct_00b18900(Object memory, Object effect) = 0;
    virtual Object draw_section_allocate_00b85ee0(std::uint32_t size) = 0;
    virtual Object draw_section_construct_00b857f0(Object memory) = 0;
    virtual void mesh_set_vertex_stream_00b73bb0(Object mesh, std::uint32_t stream, Object buffer) = 0;
    virtual void mesh_set_index_stream_00b73b70(Object mesh, Object buffer) = 0;
    virtual void material_set_texture_00b189f0(Object material, std::uint32_t slot, Object texture) = 0;
    virtual void draw_section_set_material_00b864c0(Object section, Object material) = 0;
    virtual void mesh_append_section_00b73c60(Object mesh, Object section) = 0;
    virtual Object mesh_element_00b732c0(Object mesh, std::uint32_t index) = 0;
    virtual void mesh_clear_section_owners_00b73c10(Object mesh) = 0;
    virtual void node_propagate_root_00b6d890(Object node, std::uint32_t root) = 0;
    // 006EC240 callees: 00B6DB60 BSP_Node_GetLocalMatrix(ECX = node) -> EAX (4x4, row 3 the
    // position); 00B646E0 / 00B64640 BSP_Matrix_BuildRotationY / X(ECX = out 4x4, EDX = &angle)
    // -> EAX = out; 00413920 multiply(ECX = a)(out, b), RET 8 -> EAX = out (out = a * b); node
    // vtable +2Ch set position(float[3]*), RET 4; the plane's vtable +88h(4x4*), RET 4.
    virtual const float* node_local_matrix_00b6db60(Object node) = 0;
    virtual float* matrix_rotation_y_00b646e0(float* out, const float* angle) = 0;
    virtual float* matrix_rotation_x_00b64640(float* out, const float* angle) = 0;
    virtual float* matrix_multiply_00413920(const float* a, float* out, const float* b) = 0;
    virtual void node_set_position_2c(Object node, const float* position) = 0;
    virtual void plane_set_pose_88(Object plane, const float* matrix) = 0;
};

// ---------------------------------------------------------------------------
// MCatapult.
// ---------------------------------------------------------------------------
// 006EC140 vtable slot A8h, __thiscall(ECX = catapult)(visitor), RET 4.
void catapult_debug_dump_006ec140(Seg33bHost& host, Object catapult, Object visitor);
// 006EC240 __fastcall(ECX = catapult), reached only from 006EC632: pose the catapult's base /
// barrel nodes and a plane sitting on it.
void catapult_pose_006ec240(Seg33bHost& host, Object catapult);
// 006EC640 __thiscall(ECX = memory)(argument), RET 4 -> EAX = catapult.
Object catapult_construct_006ec640(Seg33bHost& host, Object memory, std::uint32_t argument);
// 006EC7B0 vtable IsKindOf, __thiscall(ECX = catapult)(kind), RET 4 -> AL.
bool catapult_is_kind_of_006ec7b0(Object catapult, std::int32_t kind);
// 006EC7F0 destructor body, __fastcall(ECX = catapult).
void catapult_destruct_body_006ec7f0(Seg33bHost& host, Object catapult);
// 006EC860 __thiscall(ECX = owner)(argument), RET 4 -> EAX = the new catapult.
Object catapult_create_006ec860(Seg33bHost& host, Object owner, std::uint32_t argument);
// 006ECB50 vtable +1F0h, __thiscall(ECX = catapult)(message), RET 4.
void catapult_apply_launch_message_006ecb50(Seg33bHost& host, Object catapult, Object message);
// 006ECBE0 vtable init, __fastcall(ECX = catapult).
void catapult_init_device_006ecbe0(Seg33bHost& host, Object catapult);
// 006ED000 __thiscall(ECX = catapult)(float heading), RET 4 -> ST0.
float catapult_launch_heading_006ed000(Seg33bHost& host, Object catapult, float heading);
// 006ED690 __fastcall(ECX = catapult).
void catapult_load_saved_state_006ed690(Seg33bHost& host, Object catapult);

// ---------------------------------------------------------------------------
// std::map<key, LuaVariant> / std::vector<LuaVariant> instances (node 34h bytes: left +0h,
// parent +4h, right +8h, key +0Ch (10h bytes), value +1Ch (14h), color +30h, is-nil +31h).
// ---------------------------------------------------------------------------
// 006ED890 the map's key compare, __thiscall(ECX unused)(a, b), RET 8 -> AL.
bool lua_map_key_less_006ed890(Seg33bHost& host, Object a, Object b);
// 006EDE40 and 006EDEF0 (identical bodies), __fastcall(ECX = it) -> EAX = it.
TreeIterator* lua_map_iterator_increment_006ede40(Seg33bHost& host, TreeIterator* it);
TreeIterator* lua_map_iterator_increment_006edef0(Seg33bHost& host, TreeIterator* it);
// 006EDFB0 __fastcall(ECX = it) -> EAX = it.
TreeIterator* lua_map_iterator_decrement_006edfb0(Seg33bHost& host, TreeIterator* it);
// 006EDF00 push_back, __thiscall(ECX = vector)(const variant*), RET 4.
void lua_variant_vector_push_back_006edf00(Seg33bHost& host, Object vector, Object variant);
// 006EDFC0 map init, __fastcall(ECX = map); 006EE120 the same with RET 8 -> EAX = map.
void lua_map_init_head_006edfc0(Seg33bHost& host, Object map);
Object lua_map_construct_006ee120(Seg33bHost& host, Object map);
// 006EE040 pair destructor, __fastcall(ECX = pair).
void lua_map_pair_destruct_006ee040(Seg33bHost& host, Object pair);
// 006EE0B0 vector tidy, __fastcall(ECX = vector).
void lua_variant_vector_tidy_006ee0b0(Seg33bHost& host, Object vector);
// 006EE0D0 pair(key, value), __thiscall(ECX = pair)(key*, value*), RET 8 -> EAX = pair.
Object lua_map_pair_construct_006ee0d0(Seg33bHost& host, Object pair, Object key, Object value);
// 006EE150 pair copy, __thiscall(ECX = pair)(from), RET 4 -> EAX = pair.
Object lua_map_pair_copy_construct_006ee150(Seg33bHost& host, Object pair, Object from);
// 006EE230 _Buynode, __thiscall(ECX unused)(left, parent, right, value*, color), RET 14h.
Object lua_map_buy_node_006ee230(Seg33bHost& host, Object left, Object parent, Object right, Object value,
                                 std::uint8_t color);
// 006EE4E0 insert(value), __thiscall(ECX = map)(out pair<iterator,bool>*, value*), RET 8.
TreeInsertResult* lua_map_insert_unique_006ee4e0(Seg33bHost& host, Object map, TreeInsertResult* out,
                                                 Object value);
// 006EE890 _Erase(subtree), __thiscall(ECX = map)(node), RET 4.
void lua_map_erase_subtree_006ee890(Seg33bHost& host, Object map, Object node);

// 006EEB20 __stdcall(value, shift, ctrl, alt), RET 10h: Mission.Ptr's message call with
// {{F = value, Message = 4, Shift, Ctrl, Alt}}.
void mission_publish_input_event_006eeb20(Seg33bHost& host, std::uint32_t value, std::uint8_t shift,
                                          std::uint8_t ctrl, std::uint8_t alt);
// 006EDB40 debug side swap, __cdecl(), no arguments.
void debug_swap_local_side_006edb40(Seg33bHost& host);

// ---------------------------------------------------------------------------
// Fog-cloud model pool (static instance 00E199B0; same template as the fish/flag pools of
// engine_seg11_35_66_cc13.hpp with this geometry and its own CRT entry points).
// Pool layout: +0h vtable, +4h/+8h allocator-list links, +0Ch CRITICAL_SECTION, +24h lock
// depth, +28h block array {data, count +2Ch, capacity +30h}, +34h first block with a free slot.
// ---------------------------------------------------------------------------
// 006EF2A0 vtable slot, __thiscall(ECX = model)(a1, float a2, unused a3, unsigned a4), RET 10h.
void fog_cloud_render_006ef2a0(Seg33bHost& host, Object model, std::uint32_t a1, float a2, std::uint32_t a3,
                               std::uint32_t a4);
// 006EF3F0 __thiscall(ECX = block)(pool index), RET 4 -> EAX = block.
Object fog_pool_init_block_006ef3f0(Object block, std::uint32_t pool_index);
// 006F03A0 __thiscall(ECX = pool)(slot), RET 4.
void fog_pool_free_slot_006f03a0(Seg33bHost& host, Object pool, Object slot);
// 006F06F0 class operator delete, slot in ECX; frees into the static pool 00E199B0.
void fog_cloud_operator_delete_006f06f0(Seg33bHost& host, Object slot);
// 006F0700 __fastcall(ECX = pool) -> EAX = pool.
Object fog_pool_construct_006f0700(Seg33bHost& host, Object pool);
// 006F0830 __fastcall(ECX = pool).
void fog_pool_destruct_006f0830(Seg33bHost& host, Object pool);
// 006F08C0 vtable slot 0, __fastcall(ECX = pool).
void fog_pool_trim_006f08c0(Seg33bHost& host, Object pool);
// 006F0960 __fastcall(ECX = pool) -> EAX = slot.
Object fog_pool_allocate_slot_006f0960(Seg33bHost& host, Object pool);
// 006F0C30 scalar deleting destructor, __thiscall(ECX = model)(flags), RET 4 -> EAX = model.
Object fog_cloud_deleting_destructor_006f0c30(Seg33bHost& host, Object model, std::uint8_t flags);
// 006F1440 __stdcall(float z, colour), RET 8 -> EAX: a pooled "Tej" generated model holding a
// 20 x 20 textured quad at height z (cloud.dds through alphachannel.mshd).
Object fog_cloud_create_006f1440(Seg33bHost& host, float z, std::uint32_t colour);
inline constexpr std::uint32_t kRenderer = 0x00F8D394;
inline constexpr std::uint32_t kFogCloudModelVtable = 0x00CFAD10;
// 006F13E0 record assignment, __thiscall(ECX = to)(from), RET 4 -> EAX = to. Record: string
// {length +0h, data +4h}, floats +8h/+0Ch, dword +10h, floats +14h/+18h, dword +1Ch.
Object fog_cloud_record_assign_006f13e0(Seg33bHost& host, Object to, Object from);

// ---------------------------------------------------------------------------
// MCommandBuilding (7E8h bytes, class id 1Ch, derives from MLandFort 758h). Fields used here:
// +764h the {CRITICAL_SECTION, depth +18h} block, +778h garrison vector {data, size, cap} of
// 64h-byte entries (observer target at entry+14h), +784h std::list, +794h landing-pad
// pointer vector, +7DCh paratrooper pointer vector, +7BCh/+7C0h a random offset pair.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kCommandBuildingVtable = 0x00CFB028;
inline constexpr std::uint32_t kCommandBuildingVtable10 = 0x00CFB00C;
inline constexpr std::uint32_t kCommandBuildingVtable24 = 0x00CFB004;
inline constexpr std::uint32_t kCommandBuildingVtable170 = 0x00CFB000;
inline constexpr std::uint32_t kCommandBuildingVtable1E4 = 0x00CFAFF8;
inline constexpr std::uint32_t kCommandBuildingVtable310 = 0x00CFAFE0;  // fixed-step node, slot 4 = 006F7360
inline constexpr std::uint32_t kCommandBuildingVtable38C = 0x00CFAFDC;
inline constexpr std::uint32_t kCommandBuildingVtable72C = 0x00CFAFD8;
inline constexpr std::uint32_t kCommandBuildingVtable758 = 0x00CFAFD4;
inline constexpr std::size_t kCommandBuildingSize = 0x7E8;
inline constexpr std::uint32_t kGarrisonEntryVtable = 0x00CEDDA0;      // observer handle, derived
inline constexpr std::uint32_t kGarrisonEntryBaseVtable = 0x00CEDD88;  // its base
inline constexpr std::uint32_t kInferiorListMessageVtable = 0x00CFAE84;
inline constexpr std::uint32_t kSessionMessageRootVtable = 0x00CE4974;
inline constexpr std::uint32_t kLandFortEmptyName = 0x00E19A60;
inline constexpr std::uint32_t kWarningManager = 0x00F8A0C4;
inline constexpr float kOne = 1.0f;                                    // float 00D7A24C
inline constexpr float kRepairTimeDefault = 200.0f;                    // float 00CE386C
inline constexpr float kCameraDistanceDefault = 150.0f;                // float 00CE3808
inline constexpr float kMinCameraAngleDefault = -30.0f;                // float 00CF8F3C
inline constexpr float kMaxCameraAngleDefault = 89.0f;                 // float 00CFAEB4
inline constexpr double kDegreesPerTurn = 360.0;                       // double 00CF1448

// 006F1EE0 / 006F1F00 __fastcall(ECX = building): enter / leave the +764h section with depth.
void command_building_lock_006f1ee0(Seg33bHost& host, Object building);
void command_building_unlock_006f1f00(Seg33bHost& host, Object building);
// 006F1F80 vtable +1A8h, an empty body (RET).
void command_building_die_006f1f80(Object building);
// 006F2250 __fastcall(ECX = cell): free the block at +0h (00BF6989) and clear it when set.
void free_and_null_006f2250(Seg33bHost& host, Object cell);
// 006F2330 dword vector reserve, __thiscall(ECX = vector)(n), RET 4.
void dword_vector_reserve_006f2330(Seg33bHost& host, Object vector, std::uint32_t count);
// 006F23E0 __thiscall(ECX = pair)(float range), RET 4 -> EAX = pair: {range, -U(0, range)}.
Object random_offset_pair_init_006f23e0(Seg33bHost& host, Object pair, float range);
// 006F3010 __fastcall(ECX = building): 006AC4D0 on every landing pad.
void command_building_refresh_pads_006f3010(Seg33bHost& host, Object building);
// 006F30A0 / 006F3320 / 006F34F0 (identical bodies) observer member target swap,
// __thiscall(ECX = member)(target), RET 4.
void observer_member_set_target_006f30a0(Seg33bHost& host, Object member, Object target);
void observer_member_set_target_006f3320(Seg33bHost& host, Object member, Object target);
void observer_member_set_target_006f34f0(Seg33bHost& host, Object member, Object target);
// 006F30D0 pointer vector reserve, __thiscall(ECX = vector)(n), RET 4.
void pointer_vector_reserve_006f30d0(Seg33bHost& host, Object vector, std::int32_t count);
// 006F3140 dword vector grow to 2n+2, __fastcall(ECX = vector).
void dword_vector_grow_006f3140(Seg33bHost& host, Object vector);
// 006F31B0 garrison entry destructor, __fastcall(ECX = entry).
void garrison_entry_destruct_006f31b0(Seg33bHost& host, Object entry);
// 006F32B0 swap-erase one value, __thiscall(ECX = vector)(const value*), RET 4 -> AL.
bool pointer_vector_erase_value_006f32b0(Object vector, const std::uint32_t* value);
// 006F3360 / 006F3530 (identical bodies) pointer push_back, __thiscall(ECX = vector)(value*).
void pointer_vector_push_back_006f3360(Seg33bHost& host, Object vector, const std::uint32_t* value);
void pointer_vector_push_back_006f3530(Seg33bHost& host, Object vector, const std::uint32_t* value);
// 006F33A0 pointer vector resize, __thiscall(ECX = vector)(n), RET 4.
void pointer_vector_resize_006f33a0(Seg33bHost& host, Object vector, std::int32_t count);
// 006F3410 dword push_back with inline 2n+2 growth, __thiscall(ECX = vector)(value), RET 4.
void dword_vector_push_back_006f3410(Seg33bHost& host, Object vector, std::uint32_t value);
// 006F3490 __fastcall(ECX = entity) -> EAX: the entity when IsKindOf(0Ch), else null.
Object entity_as_kind_0c_006f3490(Seg33bHost& host, Object entity);
// 006F34B0 / 006F39B0 paratrooper list +7DCh remove / add under the lock, RET 4.
void command_building_remove_paratrooper_006f34b0(Seg33bHost& host, Object building, std::uint32_t unit);
void command_building_add_paratrooper_006f39b0(Seg33bHost& host, Object building, std::uint32_t unit);
// 006F3570 uint16 push_back, __thiscall(ECX = array)(value*), RET 4.
void word_array_push_back_006f3570(Seg33bHost& host, Object array, const std::uint16_t* value);
// 006F35E0 garrison entry copy, __thiscall(ECX = to)(from), RET 4 -> EAX = to.
Object garrison_entry_copy_construct_006f35e0(Seg33bHost& host, Object to, Object from);
// 006F3A20 vtable +210h, __fastcall(ECX = building) -> AL.
bool command_building_any_pad_group_006f3a20(Seg33bHost& host, Object building);
// Garrison vector (64h-byte entries): 006F3FC0 / 006F55F0 tidy (resize 0 via 006F33A0 resp.
// 006F4BA0, then free), 006F3FE0 reserve, 006F4B30 push_back, 006F4BA0 resize.
void pointer_vector_tidy_006f3fc0(Seg33bHost& host, Object vector);
void garrison_vector_reserve_006f3fe0(Seg33bHost& host, Object vector, std::int32_t count);
void garrison_vector_push_back_006f4b30(Seg33bHost& host, Object vector, Object entry);
void garrison_vector_resize_006f4ba0(Seg33bHost& host, Object vector, std::int32_t count);
void garrison_vector_tidy_006f55f0(Seg33bHost& host, Object vector);
// COMMANDBUILDING_INFERIORLIST (D6h) session message: 006F4110 construct(ECX = message)
// (garrison vector*), RET 4; 006F4220 write(stream), RET 4; 006F4270 read(stream), RET 4;
// 006F4300 destructor body.
Object inferior_list_message_construct_006f4110(Seg33bHost& host, Object message, Object garrison);
void inferior_list_message_write_006f4220(Seg33bHost& host, Object message, Object stream);
void inferior_list_message_read_006f4270(Seg33bHost& host, Object message, Object stream);
void inferior_list_message_destruct_006f4300(Seg33bHost& host, Object message);
// 006F4360 class vtable +8h, __thiscall(ECX = class)(lua), RET 4.
void command_building_class_read_lua_006f4360(Seg33bHost& host, Object klass, Object lua);
// 006F4A00 __thiscall(ECX = building)(kind), RET 4 -> AL.
bool command_building_garrison_inferior_failure_006f4a00(Seg33bHost& host, Object building, std::int32_t kind);
// 006F4CC0 frame update, __thiscall(ECX = building)(float dt), RET 4.
void command_building_update_006f4cc0(Seg33bHost& host, Object building, float dt);
// 006F5460 message handler, __thiscall(ECX = building)(message), RET 4 -> AL.
bool command_building_handle_message_006f5460(Seg33bHost& host, Object building, Object message);
// 006F5610 __thiscall(ECX = memory)(argument), RET 4 -> EAX.
Object command_building_construct_006f5610(Seg33bHost& host, Object memory, std::uint32_t argument);
// Land-fort shared virtuals, __fastcall(ECX = fort): 006F5790 the +72Ch sub-object (null in,
// null out); 006F57C0 +738h is null; 006F57F0 (kind) RET 4: the master's IsKindOf(kind)
// (tail jump) or false; 006F5980 the display name.
Object land_fort_sub72c_006f5790(Object fort);
bool land_fort_has_no_master_006f57c0(Object fort);
bool land_fort_master_is_kind_of_006f57f0(Seg33bHost& host, Object fort, std::int32_t kind);
Object land_fort_name_006f5980(Object fort);
// 006F59F0 / 006F5AA0 detach from the world lists, __fastcall(ECX = fort / building).
void land_fort_detach_world_lists_006f59f0(Seg33bHost& host, Object fort);
void command_building_detach_world_lists_006f5aa0(Seg33bHost& host, Object building);
// 006F5AE0 destructor body, __fastcall(ECX = building).
void command_building_destruct_006f5ae0(Seg33bHost& host, Object building);
// 006F5C10 class allocator, __thiscall(ECX = descriptor)(argument), RET 4 -> EAX.
Object command_building_allocate_006f5c10(Seg33bHost& host, Object descriptor, std::uint32_t argument);
// 006F6740 __fastcall(ECX = building).
void command_building_init_all_006f6740(Seg33bHost& host, Object building);
// 006F2780 __fastcall(ECX = building): scene init and the scene property fields.
void command_building_init_from_properties_006f2780(Seg33bHost& host, Object building);
// 006F7670 __fastcall(ECX = the tuning singleton 00E19A64): the first call runs
// luaMW_init.lua and CommandBuildingGlobals.lua and reads the CommandBuildingGlobals table.
void command_building_globals_load_006f7670(Seg33bHost& host, Object tuning);

// ---------------------------------------------------------------------------
// Command list, target descriptor, command stream and convex object helpers.
// ---------------------------------------------------------------------------
// 006F7D10 node {0, 0, value}, __thiscall(ECX unused)(value), RET 4 -> EAX.
Object command_list_new_node_006f7d10(Seg33bHost& host, std::uint32_t value);
// 006F7D40 append at the tail, __thiscall(ECX = list {count, head, tail})(value), RET 4.
void command_list_append_006f7d40(Seg33bHost& host, Object list, std::uint32_t value);
// 006F7DD0 __thiscall(ECX = descriptor)(out float[3]), RET 4. Descriptor: byte +0h has an
// entity, byte +1h has a point, +4h entity, +8h point.
void target_descriptor_resolve_position_006f7dd0(Seg33bHost& host, Object descriptor, float* out);
// 006F99C0 __fastcall(ECX = stream) -> EAX: the command class named by the "name" field.
Object command_stream_read_class_006f99c0(Seg33bHost& host, Object stream);
// 006F9AC0 __fastcall(ECX = node, EDX = out uint16*)(count*), RET 4.
void convex_read_u16_array_006f9ac0(Seg33bHost& host, Object node, Object out_data, std::uint32_t* count);
// 006F9B20 __fastcall(ECX = node, EDX = uint16[4] out) -> EAX = node.
Object convex_read_record_006f9b20(Seg33bHost& host, Object node, std::uint16_t* out);
// 006F9BF0 __fastcall(ECX = node, EDX = float[3] out) -> EAX = node.
Object node_read_vector3_006f9bf0(Seg33bHost& host, Object node, float* out);
// 006F9C20 / 006F9C40, 006F9C80 / 006F9CA0 the two array-holder constructors and destructors
// (__fastcall(ECX = holder)).
Object convex_arrays_construct_006f9c20(Object holder);
void convex_arrays_destruct_006f9c40(Seg33bHost& host, Object holder);
Object convex_arrays2_construct_006f9c80(Object holder);
void convex_arrays2_destruct_006f9ca0(Seg33bHost& host, Object holder);
// 006F9CD0 / 006F9D70 ConvexObject constructor / destructor body, __fastcall(ECX = object).
Object convex_object_construct_006f9cd0(Seg33bHost& host, Object object);
void convex_object_destruct_006f9d70(Seg33bHost& host, Object object);
// 006F9D20 (RET 8) and 006F9D30 (RET): EAX = 0.
std::uint32_t return_zero_006f9d20(std::uint32_t a, std::uint32_t b);
std::uint32_t return_zero_006f9d30();
// 006F9D40 __thiscall(ECX unused)(token), RET 4 -> AL: the token is one of the three at
// 00E19A98..00E19AA4.
bool convex_object_is_kind_of_006f9d40(Seg33bHost& host, std::uint32_t token);
// 006FA320 __fastcall(ECX = hull, EDX = 4x4 matrix): draw every hull edge once.
void convex_hull_debug_draw_006fa320(Seg33bHost& host, Object hull, const float* matrix);
inline constexpr std::uint32_t kConvexTokens = 0x00E19A98;     // three dwords
inline constexpr std::uint32_t kConvexTokensEnd = 0x00E19AA4;
inline constexpr std::uint32_t kEmptyCString = 0x00E19A78;     // "" for a null string
inline constexpr std::uint32_t kZeroVector = 0x00F87574;       // float[3]
inline constexpr std::uint32_t kConvexArraysVtable = 0x00CFB69C;
inline constexpr std::uint32_t kConvexArrays2Vtable = 0x00CFB6A0;
inline constexpr std::uint32_t kConvexObjectVtable = 0x00CFB6A4;

}  // namespace bsp::cc13_seg33b

#endif  // BSP_CC13_SEG33B_HPP
