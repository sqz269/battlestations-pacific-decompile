// Segment 59 gun bots (008FB000..00905300), reconstruction pass (packet cc13-seg59A).
//
// The torpedo bot, the gunner bots (lead / turret / ballistic / torpedo / depth-charge gun-bot
// classes) and the robot-config descriptor rows that still had no body or ledger record.
// docs/CC13_SEG59B.md carries the evidence and the status of every routine, including the ones
// recorded ledger-only because a cc9 gameplay host (or an earlier cc13 file) already implements
// them.
//
// cc9 coverage of the gun-bot machinery: docs/GUN_BOT_TICKS.md (:83, :124-140 and the coverage
// table, where 008FBC80, 008FBDC0, 008FBEC0 and 008FEAC0 are read in full) and the constants in
// include/bsp/gun_bot_ticks.hpp (kGunBotAttachAddress, kAAGunnerBotOffGun 5Ch, kTailGunnerBotOffGun
// 68h, kEntityKindUnit 5, the 008FEAC0 property layout), which the bodies here reuse.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in the
// image) and reaches the engine through Seg59bHost, whose methods name the image routine or the
// virtual slot they stand for. New C++ interfaces, not drop-in binary replacements; nothing here is
// bound into bsp_game. Names are hypotheses, not recovered symbols. Marker: cc13_seg59b.

#ifndef BSP_CC13_SEG59B_HPP
#define BSP_CC13_SEG59B_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg59b {

template <class T>
inline T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
inline void* ptr(void* base, std::size_t off) noexcept { return at<void*>(base, off); }
inline void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}

// The property archive's tagged by-value argument (8 bytes on the stack): kind 0 = C string key,
// 1 = int (an index key, or the value / int* of an int field), 2 = float (value bits when written,
// float* when read), 3 = byte (low byte when written), 4 = object reference, 5 / 6 = by-address
// aggregates (5 is the vec3 "pos"; 6 is a record whose size this file does not establish); 7 = a 4x4 matrix by address.
struct ArchiveArg {
    std::uint32_t kind;
    std::uintptr_t value;
};
inline ArchiveArg key(const char* name) noexcept {
    return ArchiveArg{0u, reinterpret_cast<std::uintptr_t>(name)};
}

// ---------------------------------------------------------------------------
// Image constants.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kTorpedoBotVtable = 0x00D182B0;          // primary
inline constexpr std::uint32_t kTorpedoBotObserverVtable = 0x00D18294;  // +1Ch
inline constexpr std::uint32_t kTorpedoBotInitialRangeBits = 0x461C4000; // 00CE3D64, 10000.0f
inline constexpr std::uint32_t kTorpedoDescriptorGlobal = 0x00E1998C;   // TorpedoBot descriptor
inline constexpr const char* kBotSectionKey = "_bot";                   // 00CF904C
inline constexpr const char* kThinkTimeLeftKey = "thinkTimeLeft";       // 00D1830C
inline constexpr const char* kTargetAngleKey = "targetAngle";           // 00D18300

inline constexpr std::uint32_t kAAGunnerBotVtable = 0x00D180A0;
inline constexpr std::uint32_t kAAGunnerBotObserverVtable = 0x00D18088;
inline constexpr std::uint32_t kAAGunnerDescriptorGlobal = 0x00E19998;
inline constexpr std::uint32_t kTailGunnerBotVtable = 0x00D18140;
inline constexpr std::uint32_t kTailGunnerBotObserverVtable = 0x00D18124;
inline constexpr std::uint32_t kTailGunnerDescriptorGlobal = 0x00E199A0;
inline constexpr std::uint32_t kAAFlakBotVtable = 0x00D18238;
inline constexpr std::uint32_t kAAFlakBotObserverVtable = 0x00D18220;
inline constexpr std::uint32_t kAAFlakDescriptorGlobal = 0x00E1999C;
inline constexpr std::uint32_t kDepthChargeBotVtable = 0x00D18338;
inline constexpr std::uint32_t kDepthChargeBotObserverVtable = 0x00D1831C;
inline constexpr std::uint32_t kDepthChargeDescriptorGlobal = 0x00E19988;
inline constexpr std::uint32_t kMinusOneBits = 0xBF800000;                // 00D7A260, -1.0f

inline constexpr const char* kProcessSectionKey = "_process";           // 00D1807C
inline constexpr const char* kLevelKey = "level";                       // 00CFE9B8
inline constexpr const char* kTargetTypeSectionKey = "trgType";         // 00D18074
inline constexpr const char* kEntityIdKey = "eId";                      // 00D18070
inline constexpr const char* kEntityKey = "e";                          // 00D1806C
inline constexpr const char* kPositionKey = "pos";                      // 00CF8FA4
inline constexpr const char* kRangeKey = "range";                       // 00D18064
inline constexpr const char* kCalcTickKey = "calcTick";                 // 00D18118
inline constexpr const char* kAngleErrTickKey = "angleErrTick";         // 00D18108
inline constexpr const char* kActAngleErrKey = "actAngleErr";           // 00D180FC
inline constexpr const char* kDAngleErrKey = "dAngleErr";               // 00D180F0
inline constexpr const char* kActFireStateKey = "actFireState";         // 00D18210
inline constexpr const char* kNextFireStateKey = "nextFireState";       // 00D18200
inline constexpr const char* kFireDelayKey = "fireDelay";               // 00D181F4
inline constexpr const char* kHorzAngleErrorKey = "horzAngleError";     // 00D181E4
inline constexpr const char* kVertAngleErrorKey = "vertAngleError";     // 00D181D4
inline constexpr const char* kAimTimeKey = "aimTime";                   // 00D181CC
inline constexpr const char* kTrKey = "tr";                             // 00D181C8
inline constexpr const char* kAngleErrorKey = "angleError";             // 00D181BC
inline constexpr const char* kAimPeriodMinKey = "aimPeriodMin";         // 00D181AC
inline constexpr const char* kAimPeriodMaxKey = "aimPeriodMax";         // 00D1819C
inline constexpr const char* kShootRangeKey = "shootRange";             // 00D18190
inline constexpr const char* kDistErrorKey = "distError";               // 00D18288
inline constexpr const char* kMatrixInterpolatorsKey = "matrixInterpolators";  // 00D184C0
inline constexpr const char* kStationariesKey = "stationaries";         // 00D184B0
inline constexpr const char* kEntityFieldKey = "entity";                // 00CE5818
inline constexpr const char* kTranslationKey = "translation";           // 00D184A4
inline constexpr const char* kRotationKey = "rotation";                 // 00D18498
inline constexpr const char* kTimeKey = "time";                         // 00CE37A8
inline constexpr const char* kInitialMatrixKey = "initialMatrix";       // 00D18488
inline constexpr const char* kInitialTimeKey = "initialTime";           // 00D1847C

// The 64-bucket property-hash iterator (12 bytes on the stack): table, node, bucket index.
struct Hash64Iterator {
    void* table;
    void* node;
    std::uint32_t bucket;
};

inline constexpr std::uint32_t kEmptyString = 0x00CE3A0C;       // ""
inline constexpr std::uint32_t kEnumNameEmpty = 0x00F89450;     // .bss byte, a "" for a null name
inline constexpr std::uint32_t kValueTextBuffer = 0x00F89458;   // .bss text buffer of 008F0E20
inline constexpr std::uint32_t kNoneText = 0x00D16350;          // "(none)"
inline constexpr std::uint32_t kTrueTextGlobal = 0x00E0C980;    // -> "true" (00CE4378)
inline constexpr std::uint32_t kFalseTextGlobal = 0x00E0C97C;   // -> "false" (00CE4370)
inline constexpr std::uint32_t kSpawnRefFrameVtable = 0x00D16AA4;
inline constexpr std::uint32_t kScenePropertyRecordVtable = 0x00CE89D4;

// ---------------------------------------------------------------------------
// Engine boundary. Each method stands for one image routine or one virtual slot, named by
// address; the comment gives the original register ABI.
// ---------------------------------------------------------------------------
struct Seg59bHost {
    virtual ~Seg59bHost() = default;

    // The dword at an image global (the robot descriptor aliases at 00E19988..00E199A0).
    virtual std::uint32_t image_dword(std::uint32_t address) = 0;

    // 0072BBD0, __thiscall(bot, descriptor), RET 4: the gun-bot base constructor.
    virtual void gun_bot_construct_base_0072bbd0(void* bot, std::uint32_t descriptor) = 0;

    // The property archive's virtual slots, __thiscall(archive, ...): +04h open section(key)
    // RET 8, +08h close section RET, +0Ch write field(key, value) RET 10h, +10h read field(key,
    // dest) RET 10h.
    virtual void archive_open_section_vt04(void* archive, ArchiveArg key) = 0;
    virtual void archive_close_section_vt08(void* archive) = 0;
    virtual void archive_write_field_vt0c(void* archive, ArchiveArg key, ArchiveArg value) = 0;
    virtual void archive_read_field_vt10(void* archive, ArchiveArg key, ArchiveArg dest) = 0;

    // 00876020 BSP_TickSubnode_Reparent, __thiscall(ECX = parent, node), RET 4.
    virtual void tick_subnode_reparent_00876020(void* parent, void* node) = 0;
    // entity->vtable[5Ch](kind), __thiscall, RET 4: IsKindOf.
    virtual bool entity_is_kind_of_vt5c(void* entity, std::int32_t kind) = 0;
    // unit->vtable[12Ch](), __thiscall, RET: the unit's skill level.
    virtual std::int32_t unit_skill_level_vt12c(void* unit) = 0;
    // bot->vtable[18h](skill), __thiscall, RET 4: set the skill index (base 0072BC80 stores +34h).
    virtual void bot_set_skill_vt18(void* bot, std::int32_t skill) = 0;
    // 0072BD50 BSP_GunBot_ClearFireTarget, __fastcall(ECX = bot), RET.
    virtual void gun_bot_clear_fire_target_0072bd50(void* bot) = 0;
    // 00927F10, __stdcall(slot), RET 4: the party slot is AI-held.
    virtual bool party_slot_is_ai_held_00927f10(std::int32_t slot) = 0;
    // gun->vtable[1E8h](arg), __thiscall, RET 4: the gun's trigger; (0) releases it
    // (docs/GUNNERY_OPEN_ITEMS.md ~10906 and ~14140).
    virtual void gun_vt1e8(void* gun, std::int32_t arg) = 0;
    // 008746F0 / 00874700, __thiscall(object, archive), RET 4: the tick-node property visitors
    // (both empty bodies in this image, TRIV_body_*).
    virtual void tick_node_read_properties_008746f0(void* object, void* archive) = 0;
    virtual void tick_node_write_properties_00874700(void* object, void* archive) = 0;

    // An image address (a string literal, a global buffer) as host memory.
    virtual void* image_pointer(std::uint32_t address) = 0;
    // 008EEB10 / 008EEB60, __fastcall(ECX = iterator {table, node, bucket}): the 64-bucket
    // property-hash iterator's begin / next (next returns node != 0 in EAX).
    virtual void hash64_iterator_begin_008eeb10(Hash64Iterator* iterator) = 0;
    virtual std::uint32_t hash64_iterator_next_008eeb60(Hash64Iterator* iterator) = 0;
    // 008EF7F0, __fastcall(ECX = property): the array property's element count.
    virtual std::uint32_t scene_property_array_count_008ef7f0(void* property) = 0;
    // 00438E10, __fastcall(ECX = a, EDX = b): case-insensitive compare, EAX = ordering.
    virtual std::int32_t compare_insensitive_00438e10(const char* a, const char* b) = 0;
    // 004B7F70, __cdecl(dst, fmt, ...): _vsprintf_s(dst, 200h, fmt, ...). The two shapes the
    // callers here use: one dword, or `count` doubles.
    virtual void format_string_512_004b7f70_u32(void* dst, const char* fmt, std::uint32_t value) = 0;
    virtual void format_string_512_004b7f70_f64(void* dst, const char* fmt, const double* values,
                                                 int count) = 0;
    // 00694A60, __fastcall(ECX = subject, EDX = member): register an observer pair.
    virtual void observer_register_pair_00694a60(void* subject, void* member) = 0;
    // 004134F0, __thiscall(ECX = dst, src), RET 4: copy a 4x4 float matrix.
    virtual void matrix_copy_004134f0(void* dst, const void* src) = 0;
    // 00414DB0, __thiscall(entity): refresh the cached world pose.
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;

    // archive->vtable[14h](key), __thiscall, RET 8: AL = the keyed section exists.
    virtual bool archive_has_section_vt14(void* archive, ArchiveArg key) = 0;
    // 00BF6713, __cdecl(): the checked-iterator failure report.
    virtual void checked_iterator_failure_00bf6713() = 0;
    // 009045C0, __thiscall(ECX = list, next, prev, const value*), RET 0Ch: allocate a list node
    // (operator new + the 64h-byte element copy 00904250); EAX = node.
    virtual void* list_buy_node_009045c0(void* list, void* next, void* prev, const void* value) = 0;
    // 00904E50, __thiscall(ECX = list, count), RET 4: grow the size, throwing on overflow.
    virtual void list_inc_size_00904e50(void* list, std::uint32_t count) = 0;
    // 0084F080 / 0084F1A0, __fastcall(ECX = archive): the stationaries reader / writer.
    virtual void stationaries_read_archive_0084f080(void* archive) = 0;
    virtual void stationaries_write_archive_0084f1a0(void* archive) = 0;


    // 00BF681B, __cdecl(size): operator new; EAX = block or null.
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    // 008F33F0 BSP_ScenePropertyBag_Insert, __thiscall(ECX = bag, key, record), RET 8; EAX.
    virtual void* scene_property_bag_insert_008f33f0(void* bag, const char* key, void* record) = 0;
    // 008F3510 BSP_ScenePropertyBag_AddFloatArrayProperty, __thiscall(ECX = bag, key, data,
    // count, flag), RET 10h; EAX.
    virtual void* scene_property_bag_add_float_array_008f3510(void* bag, const char* key,
                                                              const void* data, std::uint32_t count,
                                                              std::uint32_t flag) = 0;

    // 00BF55BE operator new, __cdecl(size); 00BF65AC / 00BF6989 _free thunks, __cdecl(p);
    // 00BF7680 memcpy, __cdecl(dst, src, count).
    virtual void* operator_new_00bf55be(std::uint32_t size) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t count) = 0;
    // 008D8A70 BSP_SceneTokenizer_PeekToken, __thiscall(ECX = tokenizer).
    virtual void scene_tokenizer_peek_token_008d8a70(void* tokenizer) = 0;
    // 0094FF10, __thiscall(ECX = writer, text), RET 4: append a C string to the scene text.
    virtual void* scene_text_write_0094ff10(void* writer, const char* text) = 0;
    // 0094FFA0, __thiscall(ECX = writer, code), RET 4; EAX = writer: 0 newline, 1 one indent.
    virtual void* scene_text_writer_manipulator_0094ffa0(void* writer, std::int32_t code) = 0;
    // 008EE5F0 BSP_SceneText_EscapeToStaticBuffer, __fastcall(ECX = text, EDX = length); EAX.
    virtual const char* scene_text_escape_008ee5f0(const char* text, std::uint32_t length) = 0;
    // 00BF86F0 _strchr, __cdecl(text, ch).
    virtual const char* strchr_00bf86f0(const char* text, std::int32_t ch) = 0;
    // 00480690 / 0047E480 BSP_ScenePropertyBag_IteratorBegin / Next, __thiscall(ECX = iterator).
    virtual void scene_property_bag_iterator_begin_00480690(Hash64Iterator* iterator) = 0;
    virtual void scene_property_bag_iterator_next_0047e480(Hash64Iterator* iterator) = 0;
    // The 008F08E0 resolver callback, __fastcall(ECX = reference name, EDX = context); EAX.
    virtual void* call_reference_resolver(std::uintptr_t callback, void* name, void* context) = 0;
    // 00435C40 BSP_NativeString_EqualsInsensitive, __thiscall(ECX = string, other), RET 4; AL.
    virtual bool native_string_equals_insensitive_00435c40(const void* string, const void* other) = 0;
    // 0043B8B0 BSP_PropertyMap_FindBorrowedKeyNode, __thiscall(ECX = map, key, uint* bucket), RET 8.
    virtual void* property_map_find_node_0043b8b0(void* map, const void* key, std::uint32_t* bucket) = 0;
    // 0043B760 BSP_PropertyKey_ComputeBucket, __thiscall(ECX = map, key), RET 4; EAX.
    virtual std::uint32_t property_key_compute_bucket_0043b760(void* map, const void* key) = 0;
    // 00425F40 BSP_NativeString_AssignHeader, __thiscall(ECX = string, other), RET 4.
    virtual void native_string_assign_header_00425f40(void* string, const void* other) = 0;
    // 0041E870 BSP_NativeString_Assign, __thiscall(ECX = string, text), RET 4; EAX = string.
    virtual void* native_string_assign_0041e870(void* string, const char* text) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton (no args); 00BD1510 ReturnBlock,
    // __thiscall(ECX = pool, data, bytes, flag), RET 0Ch.
    virtual void* sized_pool_singleton_00419cc0() = 0;
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
                                                  std::uint32_t flag) = 0;
    // 0043B0A0 BSP_EnumNodePool_Return14h, __thiscall(ECX = pool, node), RET 4.
    virtual void enum_node_pool_return_0043b0a0(void* pool, void* node) = 0;
    // 0094F270 BSP_StringKeyList_AppendOwnedHeader, __thiscall(ECX = list, header, flag), RET 8.
    virtual std::uint32_t string_key_list_append_owned_header_0094f270(void* list, void* header,
                                                                      std::uint32_t flag) = 0;
    // 0043D2D0 BSP_PropertyMapNode_Destroy, __thiscall(ECX = map, node), RET 4.
    virtual void property_map_node_destroy_0043d2d0(void* map, void* node) = 0;
    // 00BD2FE0 / 00BD3050 BSP_RandomThreads_Register/UnregisterCurrent, ECX passed through
    // unread; 00BD0400 BSP_SingletonLifetime_Destroy, __thiscall(ECX = object).
    virtual void random_threads_register_current_00bd2fe0(void* ecx) = 0;
    virtual void random_threads_unregister_current_00bd3050(void* ecx) = 0;
    virtual void singleton_lifetime_destroy_00bd0400(void* object) = 0;
    // 004254B0, __cdecl(format, ...): a trivial body (the release build's compiled-out trace).
    // The pushed argument dwords are handed over in stack order (a double is two dwords, low
    // first), so the call site's arguments stay exact.
    virtual void trace_printf_004254b0(const char* format, const std::uint32_t* args,
                                       std::uint32_t count) = 0;
    // 0041DD40 BSP_NativeString_Resize, __thiscall(ECX = string, length, flag), RET 8.
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t flag) = 0;
    // 00950130 / 00950000 / 00950050 BSP_SceneTextWriter_WriteFloat / WriteInt / WriteFormatted
    // (unsigned), __thiscall(ECX = writer, value), RET 4; EAX = writer.
    virtual void* scene_text_write_float_00950130(void* writer, float value) = 0;
    virtual void* scene_text_write_int_00950000(void* writer, std::int32_t value) = 0;
    virtual void* scene_text_write_unsigned_00950050(void* writer, std::uint32_t value) = 0;
    // 00439AC0 BSP_String_FormatStatic, __cdecl(format, ...); EAX = static text. Arguments as
    // pushed dwords, as for 004254B0.
    virtual const char* string_format_static_00439ac0(const char* format, const std::uint32_t* args,
                                                     std::uint32_t count) = 0;
    // 008F1960 BSP_SceneReference_GetTypeLetter, __thiscall(ECX = record, string*), RET 4
    // (hosted as scene_reference_kind_letter, src/scene_property_bag.cpp:70).
    virtual void scene_reference_get_type_letter_008f1960(void* record, void* string) = 0;
    // 0041DD20 BSP_NativeString_DestroyStorage, __thiscall(ECX = string).
    virtual void native_string_destroy_storage_0041dd20(void* string) = 0;
    // 00887E50 BSP_MissionLuaHost_CallNamedThreadSafe, __thiscall(ECX = runner, a, name*, args*,
    // b, c), RET 14h.
    virtual void lua_call_named_00887e50(void* runner, std::uint32_t a, const void* name, void* args,
                                         std::uint32_t b, std::int32_t c) = 0;
    // 004C3020 BSP_PooledStringList_AllocateSentinel, __thiscall(ECX = list); EAX = head.
    virtual void* pooled_string_list_allocate_sentinel_004c3020(void* list) = 0;
    // 004D05E0 BSP_PooledStringList_ClearStorage, __thiscall(ECX = list).
    virtual void pooled_string_list_clear_storage_004d05e0(void* list) = 0;
    // 00441210 BSP_LuaObjectVector_Tidy, __thiscall(ECX = vector); 00442220 PushBack,
    // __thiscall(ECX = vector, const LuaObject*), RET 4; 00B67700 BSP_LuaObject_Destruct,
    // __thiscall(ECX = object).
    virtual void lua_object_vector_tidy_00441210(void* vector) = 0;
    virtual void lua_object_vector_push_back_00442220(void* vector, const void* object) = 0;
    virtual void lua_object_destruct_00b67700(void* object) = 0;
    // 005FE8B0 (std::list<NativeString>::remove), __thiscall(ECX = list, const value*), RET 4.
    virtual void string_list_remove_005fe8b0(void* list, const void* value) = 0;
    // 004CE6F0 BSP_NativeRenderResourceAliasNode_AllocateCopy, __thiscall(ECX = list, next,
    // prev, const value*), RET 0Ch; EAX = node. 004CE780 BSP_NativeAliasList_GrowCount,
    // __thiscall(ECX = list, n), RET 4.
    virtual void* alias_node_allocate_copy_004ce6f0(void* list, void* next, void* prev, const void* value) = 0;
    virtual void alias_list_grow_count_004ce780(void* list, std::uint32_t count) = 0;
    // The std::_Tree::_Erase instantiation at `callee` (0058B520, 0058BCD0, 005914B0, 00582B00,
    // 00592A20, 00592EA0, 00582C00, 00582C80, 00582D00, 00582B80), __thiscall(ECX = tree, root),
    // RET 4: frees the subtree. Bound library code; the caller resets the head itself.
    virtual void stl_tree_erase_subtree(std::uint32_t callee, void* tree, void* root) = 0;
    // 00BF67A7 _memmove_s, __cdecl(dst, dst_size, src, count).
    virtual void memmove_s_00bf67a7(void* dst, std::uint32_t dst_size, const void* src, std::uint32_t count) = 0;
    // entity->vtable[108h](), __thiscall: the entity's crew / player record (byte +21h, slot +1Ch).
    virtual void* entity_player_record_vt108(void* entity) = 0;
    // 00779A00 BSP_OrdnanceKind_ToCategory, __thiscall(ECX = value); EAX.
    virtual std::uint32_t ordnance_kind_to_category_00779a00(void* value) = 0;
    // 005070C0 STL_CaseInsensitiveStringIntMap_Index, __thiscall(ECX = map, key*), RET 4; EAX = &int.
    virtual std::int32_t* string_int_map_index_005070c0(void* map, const void* key) = 0;
    // 0050FC30 BSP_AwardTable_FindRecord, __thiscall(ECX = [00E19900]+10h, key*), RET 4.
    virtual void* award_table_find_record_0050fc30(void* table, const void* key) = 0;
    // 00481750 (checked vector<int>::operator[]), __thiscall(ECX = vector, index), RET 4; EAX.
    virtual std::int32_t* int_vector_at_00481750(void* vector, std::uint32_t index) = 0;
    // 0090EDE0 BSP_MissionScoring_GrantAward, __thiscall(ECX = scoring, slot, key*, value, flag),
    // RET 10h (reconstructed in src/cc13_seg59a.cpp).
    virtual void scoring_grant_award_0090ede0(void* scoring, std::int32_t slot, const void* key,
                                              std::int32_t value, std::int32_t flag) = 0;
    // 00939F70 / 00939F80 / 00939FC0 / 00939FB0, __thiscall(ECX = unit+A20h); ST0: trivial float
    // getters of the repair / damage block.
    virtual float unit_block_float_00939f70(void* block) = 0;
    virtual float unit_block_float_00939f80(void* block) = 0;
    virtual float unit_block_float_00939fc0(void* block) = 0;
    virtual float unit_block_float_00939fb0(void* block) = 0;
    // 007265E0 / 0090E7C0 vector push_back instantiations, __thiscall(ECX = vector, value*), RET 4.
    virtual void short_vector_push_back_007265e0(void* vector, const void* value) = 0;
    virtual void repair_event_vector_push_back_0090e7c0(void* vector, const void* value) = 0;
    // 00923BE0 BSP_UnitInstance_GetHealth, __thiscall(ECX = unit); ST0.
    virtual float unit_get_health_00923be0(void* unit) = 0;
    // 00914390 BSP_MissionScoreRecord_RecomputeCategoryTotals, __thiscall(ECX = record);
    // 0090EA00 (the record's Lua publish, open row), __thiscall(ECX = record).
    virtual void score_record_recompute_category_totals_00914390(void* record) = 0;
    virtual void score_record_publish_0090ea00(void* record) = 0;
    // 00B67980 BSP_LuaStateOwner_GetGlobals, __thiscall(ECX = owner, LuaObject* out), RET 4;
    // 00B67800 BSP_LuaObject_GetByName, __thiscall(ECX = table, LuaObject* out, name), RET 8;
    // 00B67460 BSP_LuaObject_SetNativeStringInteger, __thiscall(ECX = table, key*, value), RET 8.
    // Each returns its out object (EAX).
    virtual void* lua_owner_get_globals_00b67980(void* owner, void* out) = 0;
    virtual void* lua_object_get_by_name_00b67800(void* table, void* out, const char* name) = 0;
    virtual void lua_object_set_native_string_integer_00b67460(void* table, const void* key,
                                                              std::int32_t value) = 0;
    // 0094F3E0 BSP_StringKeyList_ClearOwnedHeaders, __thiscall(ECX = list); 0094F1B0 (qsort of
    // the list with comparator 0094F130 / 0094F180 by the flag), __thiscall(ECX = list, flag),
    // RET 4.
    virtual void string_key_list_clear_owned_headers_0094f3e0(void* list) = 0;
    virtual void string_key_list_sort_0094f1b0(void* list, std::uint32_t flag) = 0;
    // 008F5900 (std::vector<void*>::insert), __thiscall(ECX = vector, ret*, container, where,
    // const value*), RET 10h; 008F3C90 (std::sort), __fastcall(ECX = first, EDX = last, count,
    // predicate), RET 8.
    virtual void pointer_vector_insert_008f5900(void* vector, void* ret, void* container, void* where,
                                                const void* value) = 0;
    virtual void sort_by_predicate_008f3c90(void* first, void* last, std::int32_t count,
                                            std::uint32_t predicate) = 0;
    // 00BF79F0 _memset, __cdecl(dst, value, count).
    virtual void memset_00bf79f0(void* dst, std::int32_t value, std::uint32_t count) = 0;
    // 00950320 BSP_SceneTextChunkList_Clear, __thiscall(ECX = list).
    virtual void scene_text_chunk_list_clear_00950320(void* list) = 0;
    // object->vtable[0](flags), __thiscall, RET 4: the scalar deleting destructor.
    virtual void object_scalar_delete_vt00(void* object, std::uint32_t flags) = 0;
    // 008EF650 / 008EEA80, __fastcall(ECX = iterator): the hash iterator's begin / next copies.
    virtual void hash64_iterator_begin_008ef650(Hash64Iterator* iterator) = 0;
    virtual std::uint32_t hash64_iterator_next_008eea80(Hash64Iterator* iterator) = 0;
};

// --- Torpedo bot (vtable 00D182B0) ---------------------------------------------------------
// 008FF260, __thiscall(ECX = bot), RET; EAX = bot. Base 0072BBD0([00E1998C]), vtables, +68h.
void* torpedo_bot_construct_008ff260(Seg59bHost& host, void* bot);
// 008FF360, __thiscall(bot, archive), RET 4; slot +48h. Save/load reader.
void torpedo_bot_read_properties_008ff360(Seg59bHost& host, void* bot, void* archive);
// 008FF400, __thiscall(bot, archive), RET 4; slot +4Ch. Save/load writer.
void torpedo_bot_write_properties_008ff400(Seg59bHost& host, void* bot, void* archive);

// --- Gun-bot base (descriptor-independent) -------------------------------------------------
// 008FBC80, __thiscall(bot, parent), RET 4; base slot +04h. Reparent, +50h = [parent+28h], skill.
void gun_bot_attach_008fbc80(Seg59bHost& host, void* bot, void* parent);
// 008FE170 / 008FE2A0, __thiscall(ECX = record (bot+38h), archive), RET 4: section "trgType".
void gun_bot_read_target_type_properties_008fe170(Seg59bHost& host, void* record, void* archive);
void gun_bot_write_target_type_properties_008fe2a0(Seg59bHost& host, void* record, void* archive);
// 008FE660 / 008FE6D0, __thiscall(bot, archive), RET 4: section "_process", "level", trgType.
void gun_bot_read_base_properties_008fe660(Seg59bHost& host, void* bot, void* archive);
void gun_bot_write_base_properties_008fe6d0(Seg59bHost& host, void* bot, void* archive);

// --- AAGunnerBot (the "lead class", vtable 00D180A0, descriptor [00E19998]) -----------------
void* aa_gunner_bot_construct_008fe740(Seg59bHost& host, void* bot);                 // 008FE740
void aa_gunner_bot_attach_008fbdc0(Seg59bHost& host, void* bot, void* parent);       // +04h
void aa_gunner_bot_clear_fire_target_008fe7a0(Seg59bHost& host, void* bot);          // +30h
void aa_gunner_bot_read_properties_008fe800(Seg59bHost& host, void* bot, void* archive);   // +48h
void aa_gunner_bot_write_properties_008fe8f0(Seg59bHost& host, void* bot, void* archive);  // +4Ch

// --- TailGunnerBot (the "turret class", vtable 00D18140, descriptor [00E199A0]) -------------
void* tail_gunner_bot_construct_008fe9f0(Seg59bHost& host, void* bot);               // 008FE9F0
void tail_gunner_bot_attach_008fbec0(Seg59bHost& host, void* bot, void* parent);     // +04h
void tail_gunner_bot_clear_fire_target_008fea60(Seg59bHost& host, void* bot);        // +30h
void tail_gunner_bot_read_properties_008feac0(Seg59bHost& host, void* bot, void* archive);  // +48h
void tail_gunner_bot_write_properties_008fecf0(Seg59bHost& host, void* bot, void* archive); // +4Ch

// --- AAFlakBot (the "ballistic class", vtable 00D18238, descriptor [00E1999C]) --------------
void* aa_flak_bot_construct_008fefd0(Seg59bHost& host, void* bot);                   // 008FEFD0
void aa_flak_bot_clear_fire_target_008fdba0(Seg59bHost& host, void* bot);            // +30h
void aa_flak_bot_read_properties_008ff060(Seg59bHost& host, void* bot, void* archive);      // +48h
void aa_flak_bot_write_properties_008ff130(Seg59bHost& host, void* bot, void* archive);     // +4Ch

// --- DepthChargeBot (vtable 00D18338, descriptor [00E19988]) --------------------------------
void* depth_charge_bot_construct_008ff4a0(Seg59bHost& host, void* bot);              // 008FF4A0
void depth_charge_bot_read_properties_008ff560(Seg59bHost& host, void* bot, void* archive); // +48h
void depth_charge_bot_write_properties_008ff5d0(Seg59bHost& host, void* bot, void* archive);// +4Ch

// --- Robot descriptor base ---------------------------------------------------------------
// 008FB770, base descriptor vtable 00D17DC0 slot +04h, __thiscall(descriptor, table, level),
// RET 8: the base parameter reader reads nothing.
void robot_config_read_parameters_base_008fb770(void* descriptor, void* level_table, std::int32_t level);

// --- Checked entity casts ---------------------------------------------------------------------
// 008FE120 / 008FE140, __fastcall(ECX = entity), RET: entity when non-null and vt[5Ch](5 / 6)
// accepts, else null.
void* entity_as_kind05_or_null_008fe120(Seg59bHost& host, void* entity);
void* entity_as_kind06_or_null_008fe140(Seg59bHost& host, void* entity);

// --- Scene property records (+04h type, +0Ch value, +10h/+14h vec3 tail, +18h/+1Ch reference
// object / name, +20h/+24h array data / byte size, +28h enum declaration) -------------------
// 008EF7C0, __thiscall(ECX = property, value), RET 4; EAX = property: +0Ch = value.
void* scene_property_set_field0c_008ef7c0(void* property, std::uint32_t value);
// 008EF820, __thiscall(ECX = property, other), RET 4; AL = the two values are equal.
bool scene_property_value_equals_008ef820(Seg59bHost& host, void* property, void* other);
// 008F0B00, __thiscall(ECX = enum table, value), RET 4; EAX = name, the empty name, or null.
const char* scene_enum_table_find_name_008f0b00(Seg59bHost& host, void* table, std::uint32_t value);
// 008F0D80, __thiscall(ECX = enum declaration, value), RET 4; the same through the iterator.
const char* scene_enum_decl_find_name_008f0d80(Seg59bHost& host, void* decl, std::uint32_t value);
// 008F0DF0, __fastcall(ECX = property): the string value (string, enum name, reference name).
const char* scene_property_string_value_008f0df0(Seg59bHost& host, void* property);
// 008F0E20, __fastcall(ECX = property): the value as display text (the shared buffer F89458).
const char* scene_property_value_text_008f0e20(Seg59bHost& host, void* property);

// 008F37E0, __thiscall(ECX = bag, key, x, y, z), RET 10h; EAX = 008F33F0's result: a new type-7
// (vec3) record. The coordinates are passed as their float bits.
void* scene_property_bag_add_vec3_008f37e0(Seg59bHost& host, void* bag, const char* key,
                                           std::uint32_t x_bits, std::uint32_t y_bits,
                                           std::uint32_t z_bits);
// 008F3880, __thiscall(ECX = bag, key, 64-byte matrix by value), RET 44h; EAX: the matrix as a
// 16-float array property through 008F3510.
void* scene_property_bag_add_matrix_008f3880(Seg59bHost& host, void* bag, const char* key,
                                             const float (&matrix)[16]);

// --- Scene text and small containers ---------------------------------------------------------
// 008EEBD0, __thiscall(ECX = array {data, count, capacity, grow}, capacity), RET 4; EAX = array.
void* dword_array_construct_capacity_008eebd0(Seg59bHost& host, void* array, std::uint32_t capacity);
// 008EEC10 (00BF65AC) and 008EEC30 / 008EF630 / 008EFC30 (00BF6989), __thiscall(ECX = slot),
// RET: free the owned block and clear the slot.
void owned_buffer_free_008eec10(Seg59bHost& host, void* slot);
void owned_buffer_free_008eec30(Seg59bHost& host, void* slot);
void owned_buffer_free_008ef630(Seg59bHost& host, void* slot);
void owned_buffer_free_008efc30(Seg59bHost& host, void* slot);
// 008EEFD0, __thiscall(ECX = tokenizer), RET; AL = byte +4 after 008D8A70.
std::uint8_t scene_tokenizer_peek_flag_008eefd0(Seg59bHost& host, void* tokenizer);
// 008EEFE0, __thiscall(ECX = writer, value), RET 4: "true" / "false" through 0094FF10.
void scene_property_write_bool_text_008eefe0(Seg59bHost& host, void* writer, bool value);
// 008EF030, __thiscall(ECX = reader), RET: vtable 00D162CC, 00950320, base vtable 00CE4DFC.
void scene_property_reader_destruct_008ef030(Seg59bHost& host, void* reader);
// 008EF730, __thiscall(ECX = buffer {data, size}, data, size, copy), RET 0Ch.
void byte_buffer_assign_008ef730(Seg59bHost& host, void* buffer, void* data, std::uint32_t size,
                                 bool copy);
// 008EFBC0, __cdecl(const void* a, const void* b): [*a+34h] - [*b+34h] (a sort comparator).
std::int32_t scene_property_ptr_compare_order_008efbc0(const void* a, const void* b);
// 008F0010, __thiscall(ECX = array, const dword* value), RET 4; EAX = the new count.
std::uint32_t pointer_array_push_back_grow_008f0010(Seg59bHost& host, void* array,
                                                    const std::uint32_t* value);
// 008F0600, __thiscall(ECX = slot), RET: delete the owned object through vtable[0](1).
void owned_polymorphic_reset_008f0600(Seg59bHost& host, void* slot);
// 008F1010, __thiscall(ECX = hash owner), RET: walk table +4 with the iterator copies, no body.
void property_hash64_for_each_008f1010(Seg59bHost& host, void* owner);

// 008EFA30, __stdcall(text, writer), RET 8: one quoted, escaped line per newline-split piece.
void scene_text_write_multiline_008efa30(Seg59bHost& host, const char* text, void* writer);
// 008F08E0, __thiscall(ECX = bag, callback, context), RET 8; AL = every callback succeeded.
bool scene_property_bag_resolve_references_008f08e0(Seg59bHost& host, void* bag, std::uintptr_t callback,
                                                    void* context);
// 008F18B0, __thiscall(ECX = map, old key, new key), RET 8; AL.
bool property_map_rename_key_008f18b0(Seg59bHost& host, void* map, void* old_key, void* new_key);
// 008F2AA0 / 008F2AE0, __thiscall(ECX = node, flags), RET 4; EAX = node: release the name,
// return the node to pool 00E17578 / 00E17540 when flags bit 0 is set.
void* scene_name_node_scalar_deleting_dtor_008f2aa0(Seg59bHost& host, void* node, std::uint32_t flags);
void* scene_name_node_scalar_deleting_dtor_008f2ae0(Seg59bHost& host, void* node, std::uint32_t flags);
// 008F2B20, __thiscall(ECX = list, name), RET 4; EAX = 0094F270's result.
std::uint32_t scene_symbol_create_from_name_008f2b20(Seg59bHost& host, void* list, const char* name);
// 008F2CF0, __thiscall(ECX = owner, old name, new name), RET 8.
void property_map_rename_key_cstring_008f2cf0(Seg59bHost& host, void* owner, const char* old_name,
                                              const char* new_name);
// 008F4030, __thiscall(ECX = map, key), RET 4; AL = removed.
bool property_map_unlink_key_008f4030(Seg59bHost& host, void* map, const void* key);

// 008F1CA0, __thiscall(ECX = never read, name, record, writer), RET 0Ch: one "name = <letter>
// value ;" line of a scene property text file.
void scene_property_write_text_008f1ca0(Seg59bHost& host, const char* name, void* record,
                                        void* writer);
// 008F2540, __thiscall(ECX = bag, depth), RET 4: the "PropTree {" debug dump through 004254B0.
void scene_property_bag_write_text_008f2540(Seg59bHost& host, void* bag, std::int32_t depth);
// 00903790 / 009037B0, __thiscall(ECX = slot), RET: _free 00BF65AC([slot]) and clear it.
void owned_buffer_free_00903790(Seg59bHost& host, void* slot);
void owned_buffer_free_009037b0(Seg59bHost& host, void* slot);
// 00905300, __thiscall(ECX = scoring, slot), RET 4; EAX = &counter: ++[scoring+slot*284h+170h].
std::uint32_t* score_record_increment_field170_00905300(void* scoring, std::int32_t slot);
// 00906430, __thiscall(ECX = result owner), RET: call the Lua function named at +147Ch.
void mission_result_call_named_script_00906430(Seg59bHost& host, void* owner);
// 0090D2F0, __thiscall(ECX = holder), RET: vtable 00D18784, 00441210(+4), vtable 00CE3784.
void lua_object_holder_destruct_0090d2f0(Seg59bHost& host, void* holder);
// 0090E920, __thiscall(ECX = holder, LuaObject by value), RET 14h; EAX = holder.
void* lua_object_holder_construct_0090e920(Seg59bHost& host, void* holder, void* object);
// 009221E0 / 00922210, __thiscall(ECX = watch list owner), RET: construct / destruct.
void* watch_list_construct_009221e0(Seg59bHost& host, void* owner);
void watch_list_destruct_00922210(Seg59bHost& host, void* owner);
// 00922240, __thiscall(ECX = owner, NativeString by value), RET 8: move the name to the back.
void watch_list_add_00922240(Seg59bHost& host, void* owner, void* name);
// 00915760, __thiscall(ECX = record = scoring + slot*284h + 4), RET: zero every scalar of one
// mission score record, empty its trees and vectors and reset its three strings.
void mission_score_record_clear_00915760(Seg59bHost& host, void* record);
// 00913D80, __thiscall(ECX = scoring, unit, hit, owner_kind), RET 0Ch: the component-failure
// statistics and the RUA_DBU award (called from 0093BED0 at 0093C0DC).
void scoring_record_component_failure_00913d80(Seg59bHost& host, void* scoring, void* unit, void* hit,
                                                std::int32_t owner_kind);
// 00914100, __thiscall(ECX = scoring, unit), RET 4: the GA_PL repair event (JMP from 0093CAF1).
void scoring_record_repair_completed_00914100(Seg59bHost& host, void* scoring, void* unit);
// 009150A0, __thiscall(ECX = scoring, slot), RET 4: recompute and publish one record, then
// scoring.playerIndex = slot in Lua when game+1FE4h != 0.
void scoring_force_refresh_009150a0(Seg59bHost& host, void* scoring, std::int32_t slot);
// 008F3270, __thiscall(ECX = bag, list, sort), RET 8: copy every key name of the bag into the
// string list, then sort it when the flag byte is set.
void scene_property_bag_collect_key_names_008f3270(Seg59bHost& host, void* bag, void* list, bool sort);
// 008F6E20, __thiscall(ECX = bag, out array), RET 4; EAX = count: the bag's records sorted by
// their ordinal (+34h) into the 008F0010 pointer array.
std::int32_t scene_property_bag_collect_records_sorted_008f6e20(Seg59bHost& host, void* bag, void* out);
// 008F7CF0, __thiscall(ECX = scope), RET; EAX = scope: 00BD2FE0.
void* random_threads_scope_construct_008f7cf0(Seg59bHost& host, void* scope);
// 008F7D20, no arguments: destroy and free the object at [01090AA0], then clear it.
void singleton_lifetime_destroy_global_008f7d20(Seg59bHost& host);
// 008F7D80, __thiscall(ECX = slot), RET: 00BD3050, free [slot], clear it.
void random_threads_scope_release_008f7d80(Seg59bHost& host, void* slot);

// --- Spawn reference frame (vtable 00D16AA4: +14h observed entity, +18h 4x4 matrix) ---------
void* spawn_ref_frame_assign_008f84a0(Seg59bHost& host, void* frame, void* other);           // RET 4
void* spawn_ref_frame_construct_from_matrix_008f84d0(Seg59bHost& host, void* frame,
                                                      const void* matrix);                    // RET 4
void* spawn_ref_frame_construct_from_entity_008f8530(Seg59bHost& host, void* frame,
                                                      void* entity);                          // RET 4
void* spawn_ref_frame_copy_construct_008f8610(Seg59bHost& host, void* frame, void* other);   // RET 4
void spawn_ref_frame_freeze_to_matrix_008f86b0(Seg59bHost& host, void* frame);              // RET
void spawn_ref_frame_on_observed_released_008f8700(Seg59bHost& host, void* frame,
                                                   void* entity);                             // RET 4

// --- World matrix interpolators (std::list at world+4B0h; 64h-byte elements: +0 entity,
// +4 translation, +10h rotation, +1Ch time, +20h initial matrix, +60h initial time) ----------
// 00903C60 / 00903D80, __thiscall(ECX = element, archive), RET 4.
void matrix_interpolator_read_archive_00903c60(Seg59bHost& host, void* element, void* archive);
void matrix_interpolator_write_archive_00903d80(Seg59bHost& host, void* element, void* archive);
// 00904F70 / 009042B0, __thiscall(ECX = world, archive), RET 4: "matrixInterpolators", then
// "stationaries".
void world_read_archive_00904f70(Seg59bHost& host, void* world, void* archive);
void world_write_archive_009042b0(Seg59bHost& host, void* world, void* archive);

// 009052E0, __thiscall(ECX = scoring, slot, delta), RET 8: the 284h-byte player record's float
// +18h += delta; EAX = its address.
float* score_record_add_field18_009052e0(void* scoring, std::int32_t slot, float delta);


}  // namespace bsp::cc13_seg59b

#endif  // BSP_CC13_SEG59B_HPP
