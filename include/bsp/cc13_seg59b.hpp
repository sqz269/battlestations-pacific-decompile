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
// aggregates (5 is the vec3 "pos"; 6 is a record whose size this file does not establish).
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

}  // namespace bsp::cc13_seg59b

#endif  // BSP_CC13_SEG59B_HPP
