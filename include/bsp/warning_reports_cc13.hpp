#pragma once

// WarningManager report builders, segment 64 (cc13-warnings1). These map
// game events to the message ids the WarningManager ([00F8A0C4] = game+21E0h)
// resolves into voice and text reports. They only READ unit state.
// Reconstruct-only: no host calls these yet. docs/CC13_WARNING_REPORTS.md.
//
// None of the code here draws a random number: a rel32/absolute-dword census
// of 0096C000..0096F100, 00973A90..00979730, 0097B400..0097B9C0 and
// 00980380..00980E50 finds no reference to 00BD2F10 or 00BD2FC0
// (docs/CC13_WARNING_REPORTS.md, "RNG").

#include "bsp/options_hardware_cc13.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {

// 00803CE0 relation codes (ECX = viewer side id, EDX = entity, fastcall):
// recon level below 2 for the viewer side -> 3; else side +54h compared with
// the viewer side, party 2 being neutral (00803CE0..00803D30, read whole).
enum class WarningRelation : std::int32_t { Own = 0, Enemy = 1, Neutral = 2, Unknown = 3 };

struct WarningUnitQuery {
    virtual ~WarningUnitQuery() = default;
    virtual bool is_kind_5c(WarningEntity, std::int32_t kind) = 0;   // vtable +5Ch
    virtual WarningEntity plane_owner_9d4(WarningEntity) = 0;        // entity+9D4h
    virtual WarningEntity controlled_unit_e188d8() = 0;
    virtual std::int32_t local_slot_18ec() = 0;                      // game+18ECh
    virtual std::int32_t local_side_id() = 0;   // [game+18CCh+slot*4]+28h
    virtual std::int32_t side_54(WarningEntity) = 0;                 // entity+54h
    // 00803CE0 with ECX = local_side_id(), EDX = entity.
    virtual WarningRelation relation_00803ce0(WarningEntity) = 0;
    // 00965810: the controlled unit, or a kind-0Fh plane whose +9D4h is it.
    virtual bool is_player_controlled_00965810(WarningEntity) = 0;
};

// 00968400 (__fastcall ECX = entity): nonnull and entity+54h is the local side.
bool is_local_side_unit_00968400(WarningUnitQuery&, WarningEntity entity);
// 00968370 (__fastcall ECX = entity): null -> Unknown, else 00803CE0.
WarningRelation local_relation_00968370(WarningUnitQuery&, WarningEntity entity);

// The three watched-entity slots of a Warning. Names are the archive keys of
// the Warning load 009768F0 / save 0096CF10 ("shortcut", "ambient",
// "reporter"); each slot is an observer handle whose pointer sits at +14h, set
// by 005B7F60 (from a raw pointer) or 00523B30 (from another handle). Both
// unlink the old entity (006952A0) and link the new one (00694A60) when it
// changes. The constructors below build the native object in place through
// cc13_seg49_64_78::WarningObject (+24h shortcut, +3Ch ambient, +54h reporter).

// ---------------------------------------------------------------------------
// 00974C70: hit warning constructor. __thiscall(ECX = warning (88h),
// target, hit, unused), RET 0Ch; vtable 00D1B308, secondary 00D1B2F0. The
// unused third argument goes to 00973BA0, which ignores its second argument.
// Caller: the hit channel 00988510 (0098997C).
// ---------------------------------------------------------------------------
struct HitWarningQuery : WarningUnitQuery {
    virtual std::int32_t slot_1c(WarningEntity attacker) = 0;          // attacker+1Ch
    virtual WarningEntity carrier_ac(WarningEntity attacker) = 0;      // attacker+ACh
    virtual WarningEntity weapon_cc(WarningEntity attacker) = 0;       // attacker+CCh
    virtual bool is_base_78c(WarningEntity) = 0;                       // entity+78Ch != 0
    virtual std::int32_t owner_slot_180(WarningEntity) = 0;            // entity+180h
    // Entries of [game+19CCh]+16Ch (command buildings) whose +180h == slot.
    virtual std::int32_t count_command_buildings_of_slot(std::int32_t slot) = 0;
};

inline constexpr std::int32_t kWarningKindUnit = 2;            // attacker+ACh test
inline constexpr std::int32_t kWarningKindShip = 6;
inline constexpr std::int32_t kWarningKindPlane = 0x0F;
inline constexpr std::int32_t kWarningKindSquadron = 0x18;
inline constexpr std::int32_t kWarningKindCommandBuilding = 0x1C;
inline constexpr std::int32_t kWarningKindPayloadWeapon = 0x2A; // attacker+CCh test
inline constexpr std::int32_t kWarningKindAirfield = 0x45;
inline constexpr std::int32_t kWarningKindCaptureExcluded = 0x0C; // 00977DA0 list filter

// `attacker` is [hit+4]->vtable+108h(), or null when hit+4 is null.
// 00973BA0(target) base, vtable 00D1B308, +6Ch observer vtable 00D1B2F0,
// id "hit", +84h (archive "areaexclusion") = 0, then the id tree.
cc13_seg49_64_78::WarningObject& construct_hit_warning_00974c70(cc13_seg49_64_78::WarningObject& w,
    cc13_seg49_64_78::WarningHost& host, HitWarningQuery&, WarningEntity target,
    WarningEntity attacker);

// ---------------------------------------------------------------------------
// 009744C0: kill warning constructor. __thiscall(ECX = warning (70h),
// victim), RET 4; vtable 00D1B2CC. Caller: the kill channel 009813A0.
// ---------------------------------------------------------------------------
struct KillWarningQuery : WarningUnitQuery {
    virtual WarningEntity killer_2c4(WarningEntity victim) = 0;   // victim+2C4h
    virtual std::int32_t slot_2d8(WarningEntity victim) = 0;      // victim+2D8h
};

// 00973A90 base, vtable 00D1B2CC, +6Ch = victim (raw pointer, no observer),
// id "kill", then the id tree.
cc13_seg49_64_78::WarningObject& construct_kill_warning_009744c0(cc13_seg49_64_78::WarningObject& w,
    cc13_seg49_64_78::WarningHost& host, KillWarningQuery&, WarningEntity victim);

// ---------------------------------------------------------------------------
// Capture and command-building reports. Each reports one entity-less 6Ch
// MessageWarning (operator_new(6Ch), 00975AC0(id), 009763E0) or, for the
// capture point, an 84h EntityWarning (00974150(point, id)).
// ---------------------------------------------------------------------------
struct WarningVec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

struct CaptureReportHost : WarningUnitQuery {
    virtual void lock_24() = 0;
    virtual void unlock_24() noexcept = 0;
    virtual bool network_session_1fe4() = 0;           // game+1FE4h != 0
    virtual bool suppressed_d0() = 0;                  // manager+D0h
    virtual float mission_clock_f876a4() = 0;
    virtual bool is_base_78c(WarningEntity) = 0;       // entity+78Ch != 0 ("base")
    virtual std::int32_t owner_slot_180(WarningEntity) = 0;
    virtual std::int32_t side_7b0(WarningEntity) = 0;  // entity+7B0h
    // +FCh/+100h/+104h after 00414DB0 when +C8h is clear.
    virtual WarningVec3 world_position(WarningEntity) = 0;
    // 00427EB0 position read, used by 009781E0's player range test.
    virtual WarningVec3 world_position_00427eb0(WarningEntity) = 0;
    virtual float capture_range_218(WarningEntity) = 0;   // vtable +218h, ST0
    virtual std::vector<WarningEntity> unit_list_64() = 0; // [game+19CCh]+64h
    virtual std::int32_t count_command_buildings_of_slot(std::int32_t slot) = 0;
    // 009761D0 inputs.
    virtual WarningEntityFlags entity_flags(WarningEntity) = 0;
    virtual bool entity_suppressed_0077edf0(WarningEntity) = 0;
    // 0096DBB0 with ECX = [00F8A0C4].
    virtual bool is_message_id_known_0096dbb0(std::string_view id) = 0;
    // 004254B0 debug print.
    virtual void debug_print_004254b0(std::string_view text) = 0;
    virtual void report_message_warning_00975ac0(std::string_view id) = 0;
    virtual std::int32_t record_state_18(WarningEntity record) = 0; // 00978A70 record+18h
    virtual void report_entity_warning_00974150(WarningEntity, std::string_view id) = 0;
};

// 009761D0: __thiscall(ECX = manager, entity), RET 4. Entity +5Ch set,
// +5Dh/+60h/+5Eh clear, and the manager+D4h map byte (0077EDF0, inserts on
// a miss) clear. Same rule as raise_entity_warning steps 4 and 5.
bool is_entity_reportable_009761d0(CaptureReportHost&, WarningEntity entity);

inline constexpr std::string_view kCaptureShipRangeId = "captureshiprange"; // 00D1B5D0
inline constexpr std::string_view kCaptureShipId = "captureship";           // 00D1B5C4

// 00977DA0: __thiscall(ECX = manager, subject, point), RET 8. Multiplayer
// only (game+1FE4h). Returns the ids reported, in order.
std::vector<std::string> report_capture_ship_00977da0(CaptureReportHost&,
    WarningEntity subject, WarningEntity point);

// 009781E0: __thiscall(ECX = manager, point, slot_arg, party_arg), RET 0Ch.
// Multiplayer only. Returns the id built, or nullopt when a guard fails;
// `reported` tells whether 0096DBB0 accepted it.
struct CapturePointReport {
    std::string id;
    bool reported{false};
};
std::optional<CapturePointReport> report_capture_point_009781e0(CaptureReportHost&,
    WarningEntity point, std::int32_t slot_arg, std::int32_t party_arg);

// 00978A70: __thiscall(ECX = manager, record, unused), RET 8. No lock and no
// guard. "hitweareunderattack" + by record+18h: 0 "commandbuildingplayer",
// 1 "commandbuildinglast", 2 "navalsupply", other values nothing. Then reports
// the id as one MessageWarning (00978BC7..00978BF9: operator_new(6Ch),
// 00975AC0(id), 009763E0(manager, warning)). Returns the reported id.
std::string report_command_building_00978a70(CaptureReportHost&, WarningEntity record);

// 0096E5E0: one stack argument (an object whose vtable +10h returns a name),
// RET 4. "Message from |." (00D1B088) + name + "|.: " (00D1B098) through two
// 004261A0 concatenations, then returns 00705E00(text, 1) on the message
// manager game+21DCh. No reference to the function exists (Ghidra xrefs,
// absolute-dword scan).
struct MessageFromHost {
    virtual ~MessageFromHost() = default;
    virtual std::string name_10(WarningEntity speaker) = 0; // vtable +10h, char*
    // 00705E00 (ECX = [game+21DCh]), stack (text, flag); returns the record.
    virtual const void* resolve_message_00705e00(const std::string& text, std::int32_t flag) = 0;
};
const void* message_from_0096e5e0(MessageFromHost&, WarningEntity speaker);

// ---------------------------------------------------------------------------
// Persistence. 009766D0: __thiscall(ECX = archive), RET. Reads key "type"
// (field type 1, int) and constructs the matching Warning, then calls its
// vtable +4 (load) with the archive. An unknown type leaves the pointer null
// and the native dereferences it (no guard at 009768C7).
// ---------------------------------------------------------------------------
struct WarningArchiveClass {
    std::int32_t type;
    std::uint32_t size;
    std::uint32_t constructor;   // native default constructor
    std::uint32_t vtable;
    std::string_view role;       // which builder fills this class
};

std::optional<WarningArchiveClass> warning_archive_class_009766d0(std::int32_t type);

// Archive keys of the Warning base load (009768F0) / save (0096CF10), with
// the reader's field type codes (0 string, 1 int, 2 float, 4 entity).
inline constexpr std::string_view kWarningKeyType = "type";                 // 00CE4AD0, save only
inline constexpr std::string_view kWarningKeyId = "id";                     // 00CF16BC, +4h
inline constexpr std::string_view kWarningKeyState = "state";               // 00CE7324, +0Ch
inline constexpr std::string_view kWarningKeyValidityRefTime = "validityRefTime"; // 00D1AFAC, +10h
inline constexpr std::string_view kWarningKeyMessage = "message";           // 00CE49A4, table
inline constexpr std::string_view kWarningKeyShortcut = "shortcut";         // +24h handle
inline constexpr std::string_view kWarningKeyAmbient = "ambient";           // +3Ch handle
inline constexpr std::string_view kWarningKeyReporter = "reporter";         // +54h handle
inline constexpr std::string_view kEntityWarningKeyEntity = "entity";       // 00976C60, +80h

// The manager map at +184h, keyed by ship, saved by 0096DD30 under
// "shipNoiseInfo" (one table per entity, named by its word +174h) and read
// back by 00980380 (00980ABC..00980BF0). This is where the archive keys
// "collisionSoundDisableTime" and "airRaidSoundExpires" land.
struct ShipNoiseInfo {
    float collision_sound_disable_time_00{0.0f}; // 00977820: one-shot sound, re-armed after this
    float air_raid_sound_expires_04{-1.0e10f};   // 00CE4ADC default; 00977990 holds 2.0 s
    bool effect_live_08{false};                  // effect handle; save writes +4h only when set
};
inline constexpr float kShipNoiseNoDeadline = -1.0e10f; // 00CE4ADC

} // namespace bsp
