// Segment 59 gun bots (008FB000..00905300), reconstruction pass (packet cc13-seg59A).
//
// The torpedo bot, the gunner bots (lead / turret / ballistic / torpedo / depth-charge gun-bot
// classes) and the robot-config descriptor rows that still had no body or ledger record.
// docs/CC13_SEG59B.md carries the evidence and the status of every routine, including the ones
// recorded ledger-only because a cc9 gameplay host (or an earlier cc13 file) already implements
// them.
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
    // gun->vtable[1E8h](arg), __thiscall, RET 4 (the gun's own release; meaning not read).
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

    // 0042E630, __cdecl(): the spatial index singleton.
    virtual void* spatial_index_singleton_0042e630() = 0;
    // entity->vtable[B0h](), __thiscall, RET: the object a segment query excludes for it.
    virtual void* entity_query_exclude_vtb0(void* entity) = 0;
    // 0098ADD0 BSP_SpatialIndex_QuerySegment, __thiscall(index; from, to, exclude, record,
    // kind_filter), RET 14h; EAX = result.
    virtual std::uint32_t spatial_index_query_segment_0098add0(void* index, void* from, void* to,
                                                               void* exclude, void* record,
                                                               std::uint32_t kind_filter) = 0;
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

// 009043A0, __stdcall(from, to, record, entity, kind_filter), RET 14h; EAX = the query result.
// A segment query on the spatial index that excludes entity->vt[B0h]() (or nothing).
std::uint32_t spatial_query_segment_excluding_009043a0(Seg59bHost& host, void* from, void* to,
                                                       void* record, void* entity,
                                                       std::uint32_t kind_filter);

}  // namespace bsp::cc13_seg59b

#endif  // BSP_CC13_SEG59B_HPP
