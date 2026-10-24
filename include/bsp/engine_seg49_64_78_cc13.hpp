#pragma once

// Packet cc13-sys6: code in segments 49 (007F8400..0080D9B0), 64 (00968E80..0097AAF0) and 78
// (00AACF10..00AC9640) that earlier passes named but did not reconstruct. Evidence:
// docs/CC13_SYS_6.md.
//
// Every name is a descriptive hypothesis, not a recovered symbol. Nothing here is ABI
// compatible: the host takes field values and callbacks where native code works on raw
// objects, and host methods are named by the callee they stand for. Nothing is
// game-validated. Gameplay code of these segments (recon, sensors, rockets, repair zones,
// the turn-path planner) is left to the cc9 lanes; the doc lists it.

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "bsp/gui_icon.hpp"
#include "bsp/gui_lua_reader.hpp"
#include "bsp/gui_widget.hpp"
#include "bsp/gui_widgets_cc13.hpp"
#include "bsp/options_hardware_cc13.hpp"
#include "bsp/profile_unlock.hpp"
#include "bsp/race_config.hpp"
#include "bsp/scoring_bodies.hpp"

namespace bsp::cc13_seg49_64_78 {

// oct18b-nB-high: 8036F0 RET; 803C20 ECX/EDX + stack index, RET4.
// Four float words at F874CC, guarded by bit0 at F874DC. Relation tables
// E0B090/E0B0F0/E0B150/E0B1B0 have 10h stride; index bounds are native preconditions.
struct ReconStyleRecord { float value[4]; };
struct ReconStyleFallback { std::uint32_t flags; ReconStyleRecord record; };
ReconStyleRecord* recon_default_style_008036f0(ReconStyleFallback&) noexcept;
const ReconStyleRecord* recon_relation_style_00803c20(int own, int other, std::uint32_t index,
    const ReconStyleRecord* same, const ReconStyleRecord* enemy,
    const ReconStyleRecord* neutral, const ReconStyleRecord* unknown, ReconStyleFallback&) noexcept;

// ===================================================================================================
// Segment 64: Lua mission event subscriptions (the "LuaListener" objects 0097E360 builds).
// ===================================================================================================

// The archive visitor the Load slots receive: the six slots of the abstract base 00CE374C
// (bsp/gui_lua_reader.hpp describes the Lua implementation 00CE44FC). Every argument is an
// eight-byte {tag, word} pair.
class EventArchiveReader {
public:
    virtual ~EventArchiveReader() = default;
    virtual void enter_04(const GuiLuaVariant& key) = 0;                                // +4h, RET 8
    virtual void leave_08() = 0;                                                        // +8h
    virtual bool read_or_default_0c(const GuiLuaVariant& key, const GuiLuaVariant& field,
                                    const GuiLuaVariant& fallback) = 0;                 // +0Ch, RET 18h
    virtual bool read_10(const GuiLuaVariant& key, const GuiLuaVariant& field) = 0;     // +10h, RET 10h
    virtual bool has_key_14(const GuiLuaVariant& key) = 0;                              // +14h, RET 8
    virtual std::size_t enumerate_keys_18(GuiLuaKeyArray& keys) = 0;                    // +18h, RET 4
};

// The archive writer the Save slots receive: +4h opens a section under a key, +8h closes it,
// +0Ch writes a key/value pair (the value tag is a GuiLuaFieldType).
class EventArchiveWriter {
public:
    virtual ~EventArchiveWriter() = default;
    virtual void begin_04(const GuiLuaVariant& key) = 0;
    virtual void end_08() = 0;
    virtual void write_0c(const GuiLuaVariant& key, const GuiLuaVariant& value) = 0;
};

// Calls the bodies make that are not archive traffic.
struct EventSubscriptionHost {
    virtual ~EventSubscriptionHost() = default;
    // 009721C0: an entity handle resolved by the reader (field type 4) -> word [entity+174h].
    virtual std::uint16_t entity_id_174(std::int32_t entity_handle) = 0;
    // 00BF6713, the CRT invalid-parameter report the checked iterators and vectors make.
    virtual void invalid_parameter_00bf6713() = 0;
};

// One event parameter as a filter sees it: the parameter object's word at +4h. Entity filters
// compare its low 16 bits, integer filters the dword, boolean filters (dword != 0), text filters
// the NativeString at +4h and range filters the float at +4h. The parameter vector the Matches
// slots receive is a std::vector of pointers to such objects (begin +4h, end +8h).
struct EventParam {
    std::uint32_t word04{0};
    float number04{0.0f};
    std::string text04{};
};

// _stricmp order, the comparator BSP_NativeString_LessCaseInsensitive 00443D00 applies.
struct StricmpLess {
    bool operator()(const std::string& left, const std::string& right) const noexcept;
};

// The 10h-byte set filters {vtable, allocator, head, size}. Slot 0 of each vtable is the
// predicate: true when the set is empty or holds the value.
struct EntityIdFilter {                  // vtable 00D1B670, predicate 00979140
    std::set<std::uint16_t> ids{};
    bool matches(const EventParam& param) const;
};
struct IntFilter {                       // vtable 00D1B674, predicate 00979320
    std::set<std::int32_t> values{};
    bool matches(const EventParam& param) const;
};
struct BoolFilter {                      // vtable 00D1B678, predicate 00979510
    std::set<bool> values{};
    bool matches(const EventParam& param) const;
};
struct TextFilter {                      // vtable 00D1B73C, predicate 0097BC70 (find 004C7F10)
    std::set<std::string, StricmpLess> values{};
    bool matches(const EventParam& param) const;
};
// The 0Ch-byte range {vtable 00D1AF44, min +4h, max +8h}; predicate 00968440. The constructors
// leave both floats unset; Load always writes them (defaults -1e10 and 1e10).
struct RangeFilter {
    float min{0.0f};
    float max{0.0f};
    bool matches(const EventParam& param) const;
};

inline constexpr float kRangeFilterDefaultMin = -1.0e10f;  // 00CE4ADC (D01502F9h)
inline constexpr float kRangeFilterDefaultMax = 1.0e10f;   // 00CE4970 (501502F9h)

// The subscription base: vtable, the callback name (a NativeString {length +4h, data +8h}).
// Base vtable 00D1AF88 (scalar deleting destructor 0096CDC0); every class destructor ends by
// restoring it and returning the callback's block to the sized storage pool.
struct EventSubscription {
    std::string callback{};
};

struct ReconSubscription : EventSubscription {      // 4Ch, vtable 00D1B67C
    EntityIdFilter entity{};                         // +0Ch
    IntFilter old_level{};                           // +1Ch
    IntFilter new_level{};                           // +2Ch
    IntFilter party{};                               // +3Ch
};
struct KillSubscription : EventSubscription {       // 3Ch, vtable 00D1B68C
    EntityIdFilter entity{};                         // +0Ch
    EntityIdFilter last_attacker{};                  // +1Ch
    IntFilter last_attacker_player_index{};          // +2Ch
};
struct HitSubscription : EventSubscription {        // 80h, vtable 00D1B740
    EntityIdFilter target{};                         // +0Ch
    EntityIdFilter target_device{};                  // +1Ch
    EntityIdFilter attacker{};                       // +2Ch
    TextFilter attack_type{};                        // +3Ch
    IntFilter attacker_player_index{};               // +4Ch
    RangeFilter damage_caused{};                     // +5Ch
    RangeFilter fire_caused{};                       // +68h
    RangeFilter leak_caused{};                       // +74h
};
struct ExitZoneSubscription : EventSubscription {   // 2Ch, vtable 00D1B69C
    EntityIdFilter entity{};                         // +0Ch
    BoolFilter exited{};                             // +1Ch
};
struct InputSubscription : EventSubscription {      // 1Ch, vtable 00D1B6AC
    IntFilter input_id{};                            // +0Ch
};
struct SurrenderSubscription : EventSubscription {  // 1Ch, vtable 00D1B6BC
    IntFilter party{};                               // +0Ch
};
struct FailureSubscription : EventSubscription {    // 38h, vtable 00D1B750
    EntityIdFilter entity{};                         // +0Ch
    TextFilter failure_type{};                       // +1Ch
    RangeFilter duration{};                          // +2Ch
};
struct LeakSubscription : EventSubscription {       // 28h, vtable 00D1B6CC
    EntityIdFilter entity{};                         // +0Ch
    RangeFilter duration{};                          // +1Ch
};
using FireSubscription = LeakSubscription;           // 28h, vtable 00D1B6DC, same fields

// Subscription lifecycle bodies. Native ABI: ECX=this, no stack arguments,
// plain RET; constructors return this in EAX. These value-model constructors
// return fresh empty filters/callback. Destructors release filters in native
// reverse order, then callback storage. std::set/std::string are qualified
// storage substitutions: native sentinels, vtables, pool allocation and EH are
// not binary-compatible with these existing model types.
ReconSubscription construct_recon_0097a220();
void destruct_recon_0097a330(ReconSubscription&);
KillSubscription construct_kill_0097a450();
void destruct_kill_0097a530(KillSubscription&);
ExitZoneSubscription construct_exit_zone_0097a620();
void destruct_exit_zone_0097a6d0(ExitZoneSubscription&);
InputSubscription construct_input_0097a790();
void destruct_input_0097a810(InputSubscription&);
SurrenderSubscription construct_surrender_0097a8b0();
void destruct_surrender_0097a930(SurrenderSubscription&);
// Native0097A9D0 leaves duration min/max untouched. Supply the allocation's
// existing range words explicitly; archive Load later writes their values.
LeakSubscription construct_leak_0097a9d0(RangeFilter untouched_duration);
void destruct_leak_0097aa50(LeakSubscription&);

struct RepairSubscription : EventSubscription {     // 2Ch, vtable 00D1B6EC
    EntityIdFilter entity{};                         // +0Ch
    IntFilter repair_setting{};                      // +1Ch
};
struct ZoneSubscription : EventSubscription {       // 3Ch, vtable 00D1B760
    EntityIdFilter entity{};                         // +0Ch
    TextFilter zone_name{};                          // +1Ch
    BoolFilter enter{};                              // +2Ch
};
struct CommandEventSubscription : EventSubscription {  // 4Ch, vtable 00D1B770
    EntityIdFilter entity{};                         // +0Ch
    EntityIdFilter target{};                         // +1Ch
    TextFilter command{};                            // +2Ch
    TextFilter status{};                             // +3Ch
};
struct TargetSubscription : EventSubscription {     // 2Ch, vtable 00D1B6FC
    EntityIdFilter entity{};                         // +0Ch
    EntityIdFilter target{};                         // +1Ch
};
struct AmmoTypeSubscription : EventSubscription {   // 2Ch, vtable 00D1B70C
    EntityIdFilter entity{};                         // +0Ch
    IntFilter ammo_type{};                           // +1Ch
};
struct StockSubscription : EventSubscription {      // 3Ch, vtable 00D1B71C
    EntityIdFilter entity{};                         // +0Ch
    IntFilter vehicle_class{};                       // +1Ch
    IntFilter stock{};                               // +2Ch
};
struct GuiSubscription : EventSubscription {        // 2Ch, vtable 00D1B780
    TextFilter gui_name{};                           // +0Ch
    BoolFilter enter{};                              // +1Ch
};
struct GenerateSubscription : EventSubscription {   // 2Ch, vtable 00D1B790
    TextFilter entity_name{};                        // +0Ch
    IntFilter entity_type{};                         // +1Ch
};
struct PlayerSubscription : EventSubscription {     // 2Ch, vtable 00D1B7A0
    IntFilter player_index{};                        // +0Ch
    TextFilter event{};                              // +1Ch
};
struct MusicOverSubscription : EventSubscription {  // 1Ch, vtable 00D1B7B0
    TextFilter music{};                              // +0Ch
};
struct ChatSubscription : EventSubscription {       // 3Ch, vtable 00D1B7C0
    IntFilter player_index{};                        // +0Ch
    TextFilter message{};                            // +1Ch
    IntFilter message_type{};                        // +2Ch
};
struct EntityKilledSubscription : EventSubscription {  // 2Ch, vtable 00D1B7D0
    EntityIdFilter entity{};                         // +0Ch
    TextFilter kill_reason{};                        // +1Ch
};
struct ShipLandedSubscription : EventSubscription { // 1Ch, vtable 00D1B72C
    EntityIdFilter entity{};                         // +0Ch
};
struct HpEventSubscription : EventSubscription {    // 30h, vtable 00D1B7E0
    EntityIdFilter entity{};                         // +0Ch
    TextFilter reason{};                             // +1Ch
    float hp{0.0f};                                  // +2Ch, "hp" (00D1B20C) read as a float
};

// --- field loaders and savers, __fastcall(ECX = archive, EDX = filter) + key pair, RET 8 ------

// 0096AA90, slot 1 of the classes without extra fields beyond the callback: read_10("callback",
// String -> +4h).
void load_callback_0096aa90(EventSubscription& subscription, EventArchiveReader& reader);
// 0096D2B0, the shared Save slot (18 classes): write_0c("callback", String data or "" when the
// data pointer is null, 00F8A0C8).
void save_callback_0096d2b0(const EventSubscription& subscription, EventArchiveWriter& writer);

// 009721C0: enter(key), clear a 250-pair key array, enumerate, then per key read it as a handle
// (field 4); a non-null entity gives word [+174h], otherwise the key is read again as an int and
// its low word is used; insert; leave.
void load_entity_set_009721c0(EventArchiveReader& reader, EventSubscriptionHost& host,
                              const char* key, EntityIdFilter& filter);
// 009722D0 / 00972390 / 00970FF0: the same walk reading each value as Int (1), Bool (3) and
// String (0) and inserting it.
void load_int_set_009722d0(EventArchiveReader& reader, const char* key, IntFilter& filter);
void load_bool_set_00972390(EventArchiveReader& reader, const char* key, BoolFilter& filter);
void load_string_set_00970ff0(EventArchiveReader& reader, const char* key, TextFilter& filter);
// 0096AAD0: enter(key), "min" (00D15F48) and "max" (00D1AF48) as floats with the two defaults,
// leave.
void load_range_0096aad0(EventArchiveReader& reader, const char* key, RangeFilter& filter);
// 0096E3A0 / 0096E460 / 0096E520: begin(key), each element in tree order under index keys 0..n-1
// (as an Int for entity ids and ints, as a Bool for booleans), end.
void save_entity_set_0096e3a0(EventArchiveWriter& writer, const char* key, const EntityIdFilter& filter);
void save_int_set_0096e460(EventArchiveWriter& writer, const char* key, const IntFilter& filter);
void save_bool_set_0096e520(EventArchiveWriter& writer, const char* key, const BoolFilter& filter);

// --- Load slots (vtable slot 1), __thiscall(ECX = subscription, reader), RET 4 ------------------
void load_recon_00972450(ReconSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_kill_00972520(KillSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_hit_009725b0(HitSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_exit_zone_009726d0(ExitZoneSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_input_00972750(InputSubscription&, EventArchiveReader&);
void load_surrender_009727b0(SurrenderSubscription&, EventArchiveReader&);
void load_failure_00972810(FailureSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_leak_009728a0(LeakSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_fire_00972920(FireSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_repair_009729a0(RepairSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_zone_00972a20(ZoneSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_command_00972ab0(CommandEventSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_target_00972b60(TargetSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_ammo_type_00972be0(AmmoTypeSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_stock_00972c60(StockSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_gui_00972cf0(GuiSubscription&, EventArchiveReader&);
void load_generate_00972d70(GenerateSubscription&, EventArchiveReader&);
void load_player_00972df0(PlayerSubscription&, EventArchiveReader&);
void load_music_over_00971130(MusicOverSubscription&, EventArchiveReader&);
void load_chat_00972e70(ChatSubscription&, EventArchiveReader&);
void load_entity_killed_00972f00(EntityKilledSubscription&, EventArchiveReader&,
                                 EventSubscriptionHost&);
void load_ship_landed_00972f80(ShipLandedSubscription&, EventArchiveReader&, EventSubscriptionHost&);
void load_hp_event_00972fe0(HpEventSubscription&, EventArchiveReader&, EventSubscriptionHost&);

// --- Save slots (vtable slot 2) with fields, __thiscall(ECX = subscription, writer), RET 4 -------
void save_recon_0096e790(const ReconSubscription&, EventArchiveWriter&);
void save_kill_0096e850(const KillSubscription&, EventArchiveWriter&);
void save_exit_zone_0096e9f0(const ExitZoneSubscription&, EventArchiveWriter&);
void save_input_0096ea70(const InputSubscription&, EventArchiveWriter&);
void save_surrender_0096ead0(const SurrenderSubscription&, EventArchiveWriter&);

// --- Matches slots (vtable slot 3), __thiscall(ECX = subscription, params), RET 4 ----------------
// Each filter i is tested against params[i]; the vector size is checked before each index
// (00BF6713 when it is short). Player, chat, entityKilled and hpEvent stop at the first filter
// that fails; kill evaluates all three before combining them.
using EventParams = std::vector<const EventParam*>;
bool player_matches_00968e80(const PlayerSubscription&, const EventParams&, EventSubscriptionHost&);
bool chat_matches_00968f40(const ChatSubscription&, const EventParams&, EventSubscriptionHost&);
bool entity_killed_matches_00968ff0(const EntityKilledSubscription&, const EventParams&,
                                    EventSubscriptionHost&);
bool hp_event_matches_009690b0(const HpEventSubscription&, const EventParams&,
                               EventSubscriptionHost&);
bool kill_matches_0096ace0(const KillSubscription&, const EventParams&, EventSubscriptionHost&);

// ===================================================================================================
// Segment 64: Warning objects (the voice/HUD warnings the WarningManager [00F8A0C4] queues).
// ===================================================================================================

// The 18h observer handle the base keeps three of: {vtable 00CE75CC, +4h..+0Ch zero, +10h byte 1,
// +14h entity}. 00694A60 (ECX = entity, EDX = handle) links it to the entity, 006952A0 unlinks
// it, BSP_ObserverLink_DestroyBase tears it down.
enum class WarningSlot : std::uint8_t { Shortcut24, Ambient3C, Reporter54, Entity6C };

struct VoiceClipRef {                 // the 0Ch message element {vtable 00CF0DD0, record, speaker}
    const void* record{nullptr};
    std::uint32_t speaker{0};
};

// One projection of every Warning class this packet touches. The base is 6Ch bytes; the entity
// warnings add the observer at +6Ch and the entity at +80h; recon (9Ch) and hit (88h) put their
// own fields after it, so `old_level` and `area_exclusion` share +84h in the native objects.
struct WarningObject {
    std::uint32_t vtable{0x00D1B104};          // +0h, the class
    std::int32_t type{-1};                     // vtable +10h; the archive "type" (009766D0)
    std::string id{};                          // +4h NativeString
    std::int32_t state{0};                     // +0Ch, 1 queued (+E0h list), 2 applied (+ECh)
    float validity_ref_time{0.0f};             // +10h, the clock [00F876A4] at construction
    std::vector<VoiceClipRef> messages{};      // +14h (begin +18h, end +1Ch, cap +20h)
    void* shortcut{nullptr};                   // +24h handle, entity at +38h
    void* ambient{nullptr};                    // +3Ch handle, entity at +50h
    void* reporter{nullptr};                   // +54h handle, entity at +68h
    std::uint32_t word_6c{0};                  // +6Ch: observer vtable (entity warnings), or the
                                               // constructor argument (00975750)
    void* entity{nullptr};                     // +80h, entity warnings
    std::int32_t old_level{0};                 // +84h, recon
    std::int32_t new_level{0};                 // +88h, recon
    float position[3]{0.0f, 0.0f, 0.0f};       // +8Ch, recon
    bool player{false};                        // +98h, recon
    bool area_exclusion{false};                // +84h, hit
    std::vector<std::string> strings{};        // +70h, string-list warning (allocator +6Ch)
};

// Calls the Warning bodies make outside the object.
struct WarningHost {
    virtual ~WarningHost() = default;
    virtual float clock_00f876a4() = 0;
    virtual void observer_link_00694a60(void* entity, WarningSlot slot) = 0;
    virtual void observer_unlink_006952a0(void* entity, WarningSlot slot) = 0;
    // BSP_ObserverLink_DestroyBase on a +24h/+3Ch/+54h handle; BSP_CallbackOwner_Destroy on +6Ch.
    virtual void observer_destroy(WarningSlot slot) = 0;
    // 00435C40: NativeString equality, length then __stricmp.
    virtual bool equals_insensitive_00435c40(const std::string& left, const std::string& right) = 0;
    // 00427EB0 on an entity (its refreshed world position), then 00427C90 (subtract) and 0042B2F0
    // (length) on the difference.
    virtual float entity_distance_0042b2f0(void* from, void* to) = 0;
    // 009741E0's reads: byte entity+C8h, 0042xxxx refresh when clear, then +FCh..+104h.
    virtual void entity_world_position(void* entity, float out[3]) = 0;
    virtual std::int32_t local_side_28() = 0;  // [[game+18CCh+4*[game+18ECh]]+28h]
    // 00803CE0 (ECX = side, EDX = entity): 0 own, 1 enemy, 2 neutral, 3 unknown.
    virtual std::int32_t relation_00803ce0(std::int32_t side, void* entity) = 0;
    virtual void* detector_for_side_210(void* entity, std::int32_t side) = 0;  // [+210h+side*34h]
    virtual bool is_kind_of_5c(void* entity, std::int32_t kind) = 0;           // vtable +5Ch
    virtual void* plane_squadron_9d4(void* plane) = 0;                         // [plane+9D4h]
    virtual void* local_player_unit_00e188d8() = 0;
    // 00705E00 on the message manager [game+21DCh], second word 0.
    virtual const void* resolve_message_00705e00(const std::string& name) = 0;
    virtual void read_message_ref_00706060(EventArchiveReader& reader, VoiceClipRef& out) = 0;
    virtual void write_message_ref_00703380(const VoiceClipRef& clip, EventArchiveWriter& writer) = 0;
};

// Constructors, __fastcall/__thiscall(ECX = new block), returning ECX.
void construct_warning_base_00973a90(WarningObject& w, WarningHost& host);
void construct_entity_warning_default_00973b20(WarningObject& w, WarningHost& host);
void construct_entity_warning_base_00973ba0(WarningObject& w, WarningHost& host, void* entity);
void construct_entity_warning_for_archive_00973c20(WarningObject& w, WarningHost& host);
void construct_recon_warning_for_archive_00973ca0(WarningObject& w, WarningHost& host);
void construct_kill_warning_default_00973d30(WarningObject& w, WarningHost& host);
void construct_hit_warning_for_archive_00973d90(WarningObject& w, WarningHost& host);
void construct_exit_zone_warning_for_archive_00973e10(WarningObject& w, WarningHost& host);
void construct_confirm_warning_for_archive_00973e90(WarningObject& w, WarningHost& host);
void construct_entity_warning_with_id_00974150(WarningObject& w, WarningHost& host, void* entity,
                                               const std::string& id);
void construct_recon_warning_009741e0(WarningObject& w, WarningHost& host, void* entity,
                                      std::int32_t old_level, std::int32_t new_level);
void construct_warning_type9_00975750(WarningObject& w, WarningHost& host, std::uint32_t word);
void construct_message_warning_00975ac0(WarningObject& w, WarningHost& host, const std::string& id);
void construct_string_list_warning_for_archive_00976230(WarningObject& w, WarningHost& host);
void construct_string_list_warning_009795b0(WarningObject& w, WarningHost& host,
                                            const std::vector<std::string>& strings,
                                            void* shortcut, void* ambient);

// Destructor bodies (the scalar deleting destructors call them).
void destroy_warning_0096f170(WarningObject& w, WarningHost& host);
void destroy_entity_warning_0096f270(WarningObject& w, WarningHost& host);
void destroy_string_list_warning_009762a0(WarningObject& w, WarningHost& host);

// Duplicate tests (vtable slot used by the manager's report walk), __thiscall(ECX, other), RET 4.
bool warning_is_duplicate_0096aa40(const WarningObject& self, const WarningObject& other,
                                   WarningHost& host);
bool hit_warning_is_duplicate_0096adb0(const WarningObject& self, const WarningObject& other,
                                       WarningHost& host);

// 00976B00, __thiscall(ECX = warning, const NativeString* name), RET 4: one message element.
void warning_add_message_00976b00(WarningObject& w, WarningHost& host, const std::string& name);

// Archive slots, __thiscall(ECX = warning, archive), RET 4.
void save_warning_0096cf10(const WarningObject& w, EventArchiveWriter& writer, WarningHost& host);
void save_entity_warning_0096d2f0(const WarningObject& w, EventArchiveWriter& writer,
                                  WarningHost& host);
void save_recon_warning_0096d3e0(const WarningObject& w, EventArchiveWriter& writer,
                                 WarningHost& host);
void save_hit_warning_0096d4d0(const WarningObject& w, EventArchiveWriter& writer,
                               WarningHost& host);
void load_warning_009768f0(WarningObject& w, EventArchiveReader& reader, WarningHost& host);
void load_entity_warning_00976c60(WarningObject& w, EventArchiveReader& reader, WarningHost& host);
void load_recon_warning_00976e40(WarningObject& w, EventArchiveReader& reader, WarningHost& host);
void load_hit_warning_00977000(WarningObject& w, EventArchiveReader& reader, WarningHost& host);

// --- the manager calls -----------------------------------------------------------------------------

struct WarningManagerHost {
    virtual ~WarningManagerHost() = default;
    // The critical section pointer at manager+24h may be null; the lock also counts +18h.
    virtual void lock_24() = 0;
    virtual void unlock_24() = 0;
    virtual void remove_from_list_0096c280(std::size_t list_offset, WarningObject* warning) = 0;
    virtual void delete_warning(WarningObject* warning) = 0;  // vtable slot 0 with flag 1
    // The waiting queue at +E4h: each node's payload (+8h) deleted through slot 0 (1), then the
    // list emptied and its nodes freed.
    virtual std::vector<void*> waiting_payloads_e4() = 0;
    virtual void delete_waiting_payload(void* payload) = 0;
    virtual void clear_waiting_list_e4() = 0;
    // [00E198C4] (the in-game GUI) and 005B83E0 on its +A4h voice player.
    virtual void* ingame_gui_00e198c4() = 0;
    virtual void voice_stop_005b83e0(void* ingame_gui) = 0;  // ECX = [ingame_gui+A4h]
};

// 0096CE70, __thiscall(ECX = manager, Warning*), RET 4.
void warning_manager_withdraw_0096ce70(WarningManagerHost& host, WarningObject* warning);
// 00973B60, the +6Ch observer's release slot: when the released entity is +80h, withdraw the
// warning through the manager [00F8A0C4] (tail jump to 0096CE70).
void entity_warning_on_released_00973b60(WarningManagerHost& host, WarningObject& warning,
                                         void* released);
// 0096BF70, __fastcall(ECX = manager).
void warning_manager_clear_waiting_0096bf70(WarningManagerHost& host);

// A per-ship record of the manager map +184h (docs/CC13_WARNING_REPORTS.md); +8h is the
// effect, an intrusively counted object (+4h count, slot 0 destroys).
struct ShipNoiseEffectHost {
    virtual ~ShipNoiseEffectHost() = default;
    virtual void effect_stop_children_00867b10(void* effect) = 0;
    virtual void effect_set_byte_9(void* effect) = 0;           // effect+9 = 1
    virtual void add_ref(void* object) = 0;                     // InterlockedIncrement(+4h)
    virtual void release(void* object) = 0;                     // decrement, slot 0 at zero
    virtual void* ship_node_4a4(void* ship) = 0;
    virtual void* manager_effect_definition_194() = 0;
    // 008687C0: (out, parent, definition (consumed reference), matrix, 1, 0, 0) -> out.
    virtual void* create_point_effect_008687c0(void* parent, void* definition,
                                               const float matrix[16]) = 0;
};
// 0096AE90, __cdecl(record): stop and drop a live effect.
void ship_noise_stop_effect_0096ae90(ShipNoiseEffectHost& host, void*& effect_08);
// 0096C070, __thiscall(ECX = manager, record, ship), RET 8: create the effect when none is live.
void ship_noise_start_effect_0096c070(ShipNoiseEffectHost& host, void*& effect_08, void* ship);

// 0096DC00 / 0096DCB0: a 0Ch {vtable 00D1AFE4, NativeString} record built from a NativeString
// passed by value (consumed); the destructor frees it and restores vtable 00D1AF18.
struct WarningEventRecord {
    std::string name{};
};
WarningEventRecord construct_warning_event_record_0096dc00(std::string name);
//0096DCB0 ECX=this, RET0096DD07; release name storage, base vtable00D1AF18.
// Value model uses std::string storage and has no native vtable field.
void destruct_warning_event_record_0096dcb0(WarningEventRecord&);

// --- reports ---------------------------------------------------------------------------------------

// The gate the report entry points share: manager+D0h clear (not suppressed), the local player
// slot [game+18ECh] in 0..7, and the clock [00F876A4] above 4.0 (00CE3D34, COMISS/JBE).
struct WarningReportHost : WarningManagerHost {
    virtual bool suppressed_d0() = 0;
    virtual std::int32_t local_player_slot_18ec() = 0;
    virtual float clock_00f876a4() = 0;
    virtual std::int32_t local_side_28() = 0;
    // 0077EDF0: operator[] of the manager's entity -> byte map at +D4h (inserts a zero byte).
    virtual bool entity_disabled_0077edf0(void* entity) = 0;
    // Entity bytes the confirm report tests: +5Ch live, +5Dh, +5Eh, +60h, and the side +54h.
    virtual bool entity_byte(void* entity, std::uint32_t offset) = 0;
    virtual std::int32_t entity_side_54(void* entity) = 0;
    // operator_new + constructor; null when the allocation fails (Report then gets null).
    virtual WarningObject* new_confirm_warning_00975540(void* entity, std::uint32_t arg) = 0;
    virtual WarningObject* new_new_unit_warning_009756a0() = 0;
    virtual WarningObject* new_powerup_warning_009758e0(const std::string* name) = 0;
    virtual WarningObject* new_string_list_warning_009795b0(const std::vector<std::string>& strings,
                                                            void* shortcut, void* ambient) = 0;
    virtual void report_009763e0(WarningObject* warning) = 0;
    virtual void set_byte_104() = 0;  // manager+104h = 1
};

bool warning_report_gate(WarningReportHost& host);
// 00977BA0, __thiscall(ECX = manager, entity, arg), RET 8. Under the +24h lock.
void report_confirm_00977ba0(WarningReportHost& host, void* entity, std::uint32_t arg);
// 00977CD0, __thiscall(ECX = manager, slot), RET 4. Under the lock; slot must be the local one.
void report_new_unit_00977cd0(WarningReportHost& host, std::int32_t slot);
// 009789A0, __thiscall(ECX = manager, slot, unused, const NativeString* name), RET 0Ch.
void report_powerup_009789a0(WarningReportHost& host, std::int32_t slot, const std::string* name);
// 00979670, __thiscall(ECX = manager, strings, shortcut, ambient), RET 0Ch. No lock; sets +104h
// once the gate passes, and builds nothing for an empty vector.
void report_string_list_00979670(WarningReportHost& host, const std::vector<std::string>& strings,
                                 void* shortcut, void* ambient);

// 0096F090, the session handler: __thiscall(ECX = manager, message), RET 8 by tail jump. The
// message bytes +18h (damaging) and +19h (friendly) replace the stack arguments of 0096EF60.
struct HitFeedbackMessage {
    std::uint8_t damaging_18{0};
    std::uint8_t friendly_19{0};
};
void session_hit_feedback_0096f090(WarningHitFeedbackState& state, float clock,
                                   const HitFeedbackMessage& message, WarningTextHost& text);

// --- the variant table (manager +144h use counts, +150h last use times) ----------------------------

struct WarningVariantTable {
    std::map<std::string, std::int32_t, StricmpLess> use_count_144{};
    std::map<std::string, float, StricmpLess> last_use_150{};
};

struct WarningVariantHost {
    virtual ~WarningVariantHost() = default;
    // 00BD2FC0 with ECX = 1: a raw word of the thread generator's stream 1 (not the gameplay
    // stream 0 of 00BD2F10). A presentation draw; the host chooses its source.
    virtual std::uint32_t random_word_00bd2fc0_stream1() = 0;
    virtual float clock_00f876a4() = 0;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void divide_error() = 0;  // DIV by zero at 00979D1A (an empty variant list)
};

// 00979B50, __thiscall(ECX = table owner, const std::vector<NativeString>* variants), RET 4.
// Returns the index of the variant picked (the native returns its address).
std::size_t pick_message_variant_00979b50(WarningVariantTable& table,
                                          const std::vector<std::string>& variants,
                                          WarningVariantHost& host);

// --- the manager archive ---------------------------------------------------------------------------

struct ShipNoiseArchiveRow {
    std::uint16_t ship_id_174{0};          // the key's entity word +174h
    float collision_sound_disable_time{0.0f};  // record +0h (node +10h)
    float air_raid_sound_expires{0.0f};        // record +4h (node +14h)
    void* effect{nullptr};                     // record +8h (node +18h)
};

struct WarningListenerChannel {
    std::string name{};                                       // map +F8h key
    std::vector<std::pair<std::string, void*>> subscriptions; // inner map: callback -> object
};

struct WarningManagerArchiveView {
    bool disable_all{false};                                  // +D0h
    std::vector<std::pair<void*, bool>> disable_entity{};     // map +D4h, in tree order
    std::vector<void*> waiting_queue{};                       // list +E4h payloads
    std::vector<void*> disable_list{};                        // list +F0h payloads
    std::vector<WarningListenerChannel> listeners{};          // map +F8h
    const WarningVariantTable* variants{nullptr};             // +144h / +150h
    std::vector<ShipNoiseArchiveRow> ship_noise{};            // map +184h
};

struct WarningManagerSaveHost {
    virtual ~WarningManagerSaveHost() = default;
    virtual void save_payload_slot2(void* payload, EventArchiveWriter& writer) = 0;      // +8h
    virtual void save_subscription_slot2(void* subscription, EventArchiveWriter& writer) = 0;
};

// 0096DD30, __thiscall(ECX = manager, writer), RET 4.
void save_warning_manager_0096dd30(const WarningManagerArchiveView& view, EventArchiveWriter& writer,
                                   WarningManagerSaveHost& host);

// ===================================================================================================
// Segment 78: GUI widget leaves left over from cc13-gui1/gui2 (see bsp/gui_widgets_cc13.hpp).
// ===================================================================================================

using gui_widgets_cc13::GuiPropertyWriter;
using gui_widgets_cc13::GuiTaggedValue;
using gui_widgets_cc13::GuiTypeCounterHost;
using gui_widgets_cc13::GuiTypeDescriptor;
using gui_widgets_cc13::GuiDescribeHost;
using gui_widgets_cc13::GuiIconStateHost;

// Engine calls the segment-78 leaves make on raw widgets.
struct GuiLeafHost {
    virtual ~GuiLeafHost() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void widget_virtual_7c(void* widget) = 0;
    virtual void widget_virtual_8c(void* widget) = 0;       // Icon: 00AB10D0 rebuild current state
    virtual void widget_virtual_50(void* widget, void* arg) = 0;
    // 00AA7970 BSP_GuiWidget_SetSize, __thiscall(widget, const float size[2]).
    virtual void widget_set_size_00aa7970(void* widget, const float (&size)[2]) = 0;
    // 00AA7D00, __thiscall(widget, float x, float y): local X/Y, recompose and bounds.
    virtual void widget_set_local_xy_00aa7d00(void* widget, float x, float y) = 0;
    // [widget+4Ch] -> 00B74640(0) -> 00B73C10: drop the draw sections' owners.
    virtual void clear_draw_section_owners_00b73c10(void* widget) = 0;
    // [widget+4Ch] -> 00B74640(0), 00B72B20 count, 00B73260(0): null when there is no node,
    // model or stream.
    virtual void* first_vertex_stream(void* widget) = 0;
    // The vertex stream of the widget's model: vtable +10h lock(a, b, c), +14h unlock, and the
    // element writes at data +8h + i*stride(+0Ch) + offset (+10h positions, +28h UV0).
    virtual void* vertex_stream_00b73260(void* widget) = 0;
    virtual void stream_lock_10(void* stream, std::int32_t a, std::int32_t b, std::int32_t c) = 0;
    virtual void stream_unlock_14(void* stream) = 0;
    virtual void stream_write_position(void* stream, std::size_t vertex, float x, float y, float z) = 0;
    virtual void stream_write_uv0(void* stream, std::size_t vertex, float u, float v) = 0;
    // Text: 00AB8910 (ensure the cursor model), 00B6DA70 on the cursor node +184h, 00AB9240.
    virtual void text_ensure_cursor_model_00ab8910(void* text) = 0;
    virtual void node_set_visibility_00b6da70(void* node, float factor, std::int32_t flag) = 0;
    virtual void text_position_cursor_00ab9240(void* text, std::uint32_t index) = 0;
    // 00AB6D00, the glyph test 00AB6D30 forwards to.
    virtual bool accepts_glyph_byte_00ab6d00(char c) = 0;
    // 00AC5600 BSP_GuiScreen_ComposeScriptPath(out, name copy) as 00AC5770 calls it.
    virtual void* compose_script_path_00ac5600(void* out, const std::string& name) = 0;
    // 00AAA710, the base widget reader 00AAD560 starts with.
    virtual void read_base_00aaa710(void* widget, gui_widgets_cc13::GuiPropertyReader& reader) = 0;
};

// Type-id initializers (see gui_widgets_cc13::gui_type_initialize): guard, name, parent Element
// (00AA8200 on 00F8BC88), its two ids, then the counter. The *Return variants also return ECX.
//   Icon      00AB3180 / 00AB31D0  guard 00F8BD9C  name 00D5C448 "cGuiIcon"
//   Line      00AC2390 / 00AC23E0  guard 00F8BEF8  name 00D5C9DC "cGuiLine"
//   Scrollbar 00AC8500 / 00AC8550  guard 00F8C000  name 00D5CD7C "cGuiScrollbar"
struct GuiDepth3TypeInit {
    std::uint32_t guard;
    std::uint32_t name;
};
inline constexpr GuiDepth3TypeInit kGuiIconTypeInit{0x00F8BD9C, 0x00D5C448};
inline constexpr GuiDepth3TypeInit kGuiLineTypeInit{0x00F8BEF8, 0x00D5C9DC};
inline constexpr GuiDepth3TypeInit kGuiScrollbarTypeInit{0x00F8C000, 0x00D5CD7C};
GuiTypeDescriptor<3>& gui_depth3_type_init(GuiTypeDescriptor<3>& target, std::uint8_t& guard,
                                           const GuiDepth3TypeInit& init,
                                           GuiTypeDescriptor<2>& element,
                                           void (*initialize_element_00aa8200)(GuiTypeDescriptor<2>&),
                                           GuiTypeCounterHost& host);

// 00AB21C0 / 00AC31F0, __fastcall(ECX = singleton): clear the global (00F8BD98 / 00F8BF44) and
// restore the singleton-base vtable 00CE3818.
void gui_singleton_destruct(void*& global, std::uint32_t& vtable);

// Icon state records (bsp::GuiIconState, 40h bytes).
// 00AB3280: release the texture reference (+0h) and the name (+4h).
void gui_icon_state_release_00ab3280(GuiIconState& state, GuiIconStateHost& host);
// 00AB3A70: member-wise copy, then retain the copied texture.
void gui_icon_state_copy_00ab3a70(GuiIconState& out, const GuiIconState& in, GuiIconStateHost& host);
// 00AB3C10: member-wise assignment with NO reference traffic on the texture pointer.
void gui_icon_state_assign_00ab3c10(GuiIconState& out, const GuiIconState& in);

// 00AB1F60, __thiscall(icon, const float pivot[2]), RET 4: the widget's Pivot (+18h/+1Ch),
// recompose (00AA7220), and when the current state (+ECh, MOVSX) is not -1 the state's Pivot
// (+2Ch) too, then vtable +8Ch. The ledger calls it SetPosition; it sets the pivot.
void gui_icon_set_pivot_00ab1f60(GuiWidgetTransform& widget, GuiIconWidget& icon,
                                 const float (&pivot)[2], GuiWidgetTransformHost& transform,
                                 GuiLeafHost& host);

// 00AB4630, __thiscall(icon, const NativeString*), RET 4: ShaderName (+104h) assigned, the
// draw sections' owners cleared, vtable +8Ch. The ledger calls it SetTexturePath.
void gui_icon_set_shader_name_00ab4630(void* widget, GuiIconWidget& icon, const std::string& name,
                                       GuiLeafHost& host);

// The {data, count, capacity} array of 0Ch {NativeString name, word} elements the delayed-load
// registry keeps (00F8BD98 +4h). `slots` has `capacity` elements.
struct GuiNamedValue {
    std::string name{};
    std::uint32_t value{0};
};
struct GuiNamedValueArray {
    std::vector<GuiNamedValue> slots{};
    std::int32_t count{0};
    std::int32_t capacity{0};
};
GuiNamedValue gui_named_value_copy_00ab4690(const GuiNamedValue& in);
void gui_named_value_assign_00ab46e0(GuiNamedValue& out, const GuiNamedValue& in);
void gui_named_value_array_reserve_00ab4720(GuiNamedValueArray& a, std::int32_t wanted);
void gui_named_value_array_resize_00ab4850(GuiNamedValueArray& a, std::int32_t size);
void gui_named_value_array_push_back_00ab4990(GuiNamedValueArray& a, const GuiNamedValue& v);
void gui_named_value_array_swap_remove_00ab4a10(GuiNamedValueArray& a, std::int32_t index);
void gui_named_value_array_destroy_00ab53b0(GuiNamedValueArray& a);

// FrameBox +15Ch..+174h, read out by 00AB6CB0 __thiscall(framebox, out byte, out word, out word,
// out float[4]), RET 10h.
struct GuiFrameBoxBorderIconLayout {
    std::uint8_t flag_15c{0};
    std::uint32_t word_160{0};
    std::uint32_t word_164{0};
    float rect_168[4]{};
};
void gui_framebox_border_icon_layout_00ab6cb0(const GuiFrameBoxBorderIconLayout& in,
                                              std::uint8_t& flag, std::uint32_t& a,
                                              std::uint32_t& b, float (&rect)[4]);

// 00AB6D30, __stdcall(char), RET 4. MOVSX then compares with A2h, A3h, A5h, A7h, AAh and B2h; a
// sign-extended char never equals those, so every byte goes on to 00AB6D00.
bool text_accepts_input_char_00ab6d30(char c, GuiLeafHost& host);

// 00AB7290, __thiscall(text, state index, const float rgba[4]), RET 8: MISColors entry
// +11Ch + 10h*index, then +118h (has-MISColors) = 1.
struct GuiTextStateColors {
    bool has_mis_colors_118{false};
    float colors_11c[4][4]{};
};
void gui_text_set_state_color_00ab7290(GuiTextStateColors& text, std::int32_t index,
                                       const float (&rgba)[4]);

// 00AB95F0, __thiscall(text, mode), RET 4: +17Ch = mode, 00AB8910, the cursor node (+184h)
// visibility = (mode != 2) as a float, and unless mode is 2: 00AB9240(+180h) then vtable +50h
// with this+50h.
struct GuiTextCursor {
    std::int32_t mode_17c{0};
    std::uint32_t index_180{0};
    void* node_184{nullptr};
};
void gui_text_set_cursor_mode_00ab95f0(void* text, GuiTextCursor& cursor, std::int32_t mode,
                                       GuiLeafHost& host);

// 00AB10A0 (Icon) / 00ABE640 (Section), __fastcall(ECX = widget): the first vertex stream of
// the widget's model, or null.
void* gui_first_vertex_stream(void* widget, GuiLeafHost& host);

struct GuiSectionVertexStreamHost {
    virtual ~GuiSectionVertexStreamHost() = default;
    // Exact native call boundaries; callee internals are not reconstructed here.
    virtual void* call_00b74640(void* model, std::int32_t index) = 0;
    virtual std::uint32_t call_00b72b20(void* geometry) = 0;
    virtual void* call_00b73260(void* geometry, std::int32_t index) = 0;
};
// 00ABE640, native ECX = section, RET -> EAX stream or null. Complete normal
// body, unlike the existing higher-level first_vertex_stream hook. Valid raw
// Win32 widget storage required; no native binary calling-convention claim.
void* gui_section_get_first_vertex_stream_00abe640(void* section, GuiSectionVertexStreamHost&);

// 00ABE670, __thiscall(section, const float size[2]), RET 4: 00AA7970 then vtable +7Ch.
void gui_section_set_size_00abe670(void* section, const float (&size)[2], GuiLeafHost& host);

// 00AC2F20, __thiscall(ticker, a, b), RET 8: +0Ch = a, +10h = b.
struct GuiHudTickerRange {
    std::uint32_t from_0c{0};
    std::uint32_t to_10{0};
};
void hud_ticker_set_range_00ac2f20(GuiHudTickerRange& ticker, std::uint32_t from, std::uint32_t to);

// 00AC3660, __thiscall(ECX = list), RET, EAX = list: 00AC3663 calls 00AC3400 (operator new
// 0Ch, next = prev = the node; instruction-identical to 00AAC180, reconstructed as
// gui_widgets_cc13::msvc9_containers::list_buy_sentinel_00aac180), +4h = that sentinel, +8h = 0.
struct GuiFontRegistryList {
    void* head_04{nullptr};
    std::uint32_t size_08{0};
};
GuiFontRegistryList& gui_font_registry_list_construct_00ac3660(GuiFontRegistryList& list);

// 00AC4AD0, __thiscall(writer, key pair, value, default), RET 10h: both values carry tag 1 (an
// integer); the key/value pair is written unless 00BD5680 finds them equal. The ledger calls it
// a bool property; the tag is the integer one.
void gui_write_int_unless_default_00ac4ad0(GuiPropertyWriter& writer, const char* key,
                                           std::int32_t value, std::int32_t fallback);

// 00AC5770, __thiscall(layer, out), RET 4: copies the layer name (+100h) and returns
// 00AC5600(out, copy).
void* gui_layer_compose_script_path_00ac5770(const std::string& layer_name_100, void* out,
                                             GuiLeafHost& host);

// 00AC8EE0, __thiscall(scroll area, float x, float y), RET 8. Clamps each offset into
// [-(content - size), 0] (+11Ch/+120h content, +20h/+24h size; the bound computed on the x87
// stack and compared with FCOMI), stores them at +110h/+114h, moves the content widget [+118h]
// through 00AA7D00 and calls this vtable +7Ch.
struct GuiScrollAreaFields {
    float size_20[2]{};
    float offset_110[2]{};
    void* content_118{nullptr};
    float content_11c[2]{};
};
void gui_scroll_area_set_offset_00ac8ee0(void* area, GuiScrollAreaFields& f, float x, float y,
                                         GuiLeafHost& host);

// Movie widget (cGuiMovie, vtable 00D5C270) fields past the 0ECh base.
struct GuiMovieFields {
    std::uint32_t word_ec{0};
    void* player_f0{nullptr};          // deleted through vtable +8h (1) by the destructor
    bool unit_size_f4{false};          // ReadProperties: Size is exactly (1,1)
    std::uint32_t word_f8{0};
    std::string movie_name_fc{};       // "MovieName" (+FCh length, +100h data)
    bool flag_104{false};
    bool flag_105{false};
    std::uint32_t word_108{0};
    std::vector<std::uint8_t> vector_10c{};  // +110h..+118h, elements destroyed by 00AAF0E0
    std::vector<std::uint8_t> vector_11c{};  // +124h..+12Ch, elements destroyed by 00AAF120
};
// 00AAFE20: base 00AA9xxx(10), vtable, and the stores above (+F4h/+104h/+105h zero).
void gui_movie_construct_00aafe20(GuiMovieFields& f);
// 00AAFFF0: base copy, then the same stores except that MovieName is copied from the source.
void gui_movie_copy_construct_00aafff0(GuiMovieFields& f, const GuiMovieFields& source);
// 00AAFEF0 destructor body: player deleted, both vectors freed, name released, base destroyed.
void gui_movie_destroy_00aafef0(GuiMovieFields& f, void (*delete_player)(void*));
// 00AAD560, __thiscall(movie, reader), RET 4: 00AAA710, "MovieName" (String, default "") into
// +FCh, then +F4h = (Size == (1.0, 1.0)).
void gui_movie_read_properties_00aad560(void* widget, const GuiWidgetTransform& transform,
                                        GuiMovieFields& f,
                                        gui_widgets_cc13::GuiPropertyReader& reader,
                                        GuiLeafHost& host);
// 00AACF10, __thiscall(movie, const float size[2]), RET 4: 00AA7970, then the four vertices of
// the model's stream (lock 4,0,0): positions (0,0,0), (w,0,0), (w,0.75h,0), (0,0.75h,0) with
// 0.75 from 00E12FCC and UV0 (0,0), (1,0), (1,1), (0,1); unlock.
void gui_movie_set_size_00aacf10(void* widget, const GuiWidgetTransform& transform,
                                 const float (&size)[2], GuiLeafHost& host);

// 00ABF090, __thiscall(section, writer), RET 4: the base describer, then each unless default.
struct GuiSectionFields {
    std::string texture_ec{};          // +ECh length, +F0h data (empty literal 00F8BEF4)
    float value_f4{0.0f};
    float start_angle_f8{0.0f};
    float texture_angle_fc{0.0f};
    float u0_100{0.0f};
    float u1_104{1.0f};
    float rim_width_percent_108{0.5f};
    bool clockwise_10c{true};
    std::int32_t texture_mode_110{0};
    void* texture_ref_114{nullptr};    // counted reference (copy retains it)
    std::uint32_t words_118[4]{};      // +118h..+124h, copied verbatim
};
void gui_section_describe_properties_00abf090(const void* section, const GuiSectionFields& f,
                                              GuiPropertyWriter& writer, GuiDescribeHost& host);

// ===================================================================================================
// Segment 49: the profile (game+650h), unlocks, bonus texts and nose art.
// ===================================================================================================

// The nose-art selection message (type E3h): the record's three words and the sender slot.
struct NoseArtSelectionMessage {
    std::uint32_t words[3]{};
    std::uint32_t slot{0};       // [game+18C8h]
};

struct ProfileCheckpointHost {
    virtual ~ProfileCheckpointHost() = default;
    // The checkpoint map at profile+94h: 007F8980 find (end = +98h) and 007F8xxx operator[]
    // reading the record's +8h vehicle class.
    virtual bool checkpoint_contains_007f8980(const std::string& key) = 0;
    virtual std::int32_t checkpoint_vehicle_class_8(const std::string& key) = 0;
    // globals["VehicleClass"][n] through 00B67980 / 00B67800 / 00B67720, then lua_type == table.
    virtual bool lua_vehicle_class_entry_is_table(std::int32_t vehicle_class) = 0;
    // A dotted path from the globals (00B68D70); false when the object is not a string.
    virtual bool lua_resolve_string(const std::string& dotted_path, std::string& out) = 0;
    virtual void mission_progress_destroy(void* progress) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void release_texture(void* texture) = 0;
    // [00F8D394] vtable +64h (path, 0): the texture manager's load.
    virtual void* load_texture_00f8d394_64(const std::string& path) = 0;
    virtual std::uint32_t local_slot_18c8() = 0;
    virtual std::int32_t session_mode_1fe4() = 0;
    virtual void apply_nose_art_007f81c0(const NoseArtSelectionMessage& message) = 0;
    virtual void session_dispatch_0076a9f0(const NoseArtSelectionMessage& message) = 0;
    virtual void session_send_to_server_00770af0(const NoseArtSelectionMessage& message) = 0;
};

// 007F93F0, __fastcall(ECX = profile): true when a name of the pending set (+7Ch) is missing
// from the unlock set (+70h).
bool has_unseen_unlock_007f93f0(const ProfileUnlockState& profile);
// 007F8E00, __thiscall(ECX = profile, const NativeString*), RET 4: the stored-hint list at
// +08h (head +0Ch) holds the name (equal length, then __stricmp).
bool stored_hint_list_contains_007f8e00(const std::vector<std::string>& hints_08,
                                        const std::string& name);
// 007FAE20, __thiscall(ECX = profile+8h list, const NativeString*), RET 4: push_back.
void stored_hint_list_push_back_007fae20(std::vector<std::string>& hints_08, const std::string& name);
// 007FC370, __thiscall(ECX = profile, key), RET 4: false when the checkpoint is unknown; true
// when its vehicle class is -1; else whether globals.VehicleClass[class] is a table.
bool checkpoint_vehicle_class_valid_007fc370(ProfileCheckpointHost& host, const std::string& key);
// 007FD870, __fastcall(ECX = &profile+64h): destroy and free the mission-progress block.
void mission_progress_ptr_reset_007fd870(void*& progress, ProfileCheckpointHost& host);
// 007F8B40, __thiscall(record, NativeString a, b, c by value, int), RET 1Ch.
ScoringBonusRecord construct_profile_bonus_record_007f8b40(std::string a, std::string b,
                                                           std::string c, std::int32_t value);
// 007FCBC0, __fastcall(ECX = profile, EDX unused, key*, a*, b*, c*), RET 10h: unless the key is
// already in +ACh (equal length then __stricmp), append it and push the record {a, b, c, 0} onto
// +BCh.
void grant_bonus_text_007fcbc0(std::vector<std::string>& granted_ac,
                               std::vector<ScoringBonusRecord>& records_bc,
                               const std::string& key, const std::string& text_a,
                               const std::string& text_b, const std::string& text_c);
// 007F9CB0 / 007F9F70 (which returns its out argument), (out, index): the texture path of nose
// art `index`, or "".
std::string nose_art_texture_path_007f9cb0(std::int32_t index, ProfileCheckpointHost& host);
// 007FA2D0, __fastcall(ECX = owner): the three textures at +104h..+10Ch released and reloaded.
void nose_art_load_textures_007fa2d0(void* (&textures_104)[3], ProfileCheckpointHost& host);
// 007F8690, __cdecl(const uint32 record[3]): message E3h, applied locally on the server or sent.
void send_nose_art_selection_007f8690(const std::uint32_t (&record)[3], ProfileCheckpointHost& host);

// 007FB520, __thiscall(ECX = profile, std::vector<long>* out), RET 4 (no caller found by xref):
// for the unlock set +70h, then the pending set +7Ch, in tree order, every name whose first 11
// characters equal "DL_Content_" (__stricmp, equal length) pushes _atol of the rest. A null
// substring buffer falls back to 00F8745C (.bss, zero-fill, so "").
void collect_content_ids_007fb520(const ProfileUnlockState& profile, std::vector<std::int32_t>& out);

// 007FC490, __thiscall(ECX = profile, out, const NativeString* name), RET 8: 007FBED0 finds or
// inserts the name in the checkpoint map +94h, 00437490 copy-constructs the 14h record into out.
struct ProfileCheckpointRecord {
    std::uint32_t words[3]{};
    std::string text_0c{};
};
struct ProfileCheckpointMapHost {
    virtual ~ProfileCheckpointMapHost() = default;
    virtual ProfileCheckpointRecord& checkpoint_find_or_insert_007fbed0(const std::string& name) = 0;
};
ProfileCheckpointRecord checkpoint_get_copy_007fc490(ProfileCheckpointMapHost& host,
                                                     const std::string& name);

// 007FCD90, __thiscall(ECX = profile = game+650h, bool keep_earned), RET 4; sole caller
// BSP_RewardsScreen_Enter (0062C5A7, pushes 0). Walks a copy of the commit record's (004B4750,
// 00593CA0) +6Ch string->int tree and updates the profile's case-insensitive map +A0h:
//   - keep_earned false and 0090C000 says the key's badge is earned: map[key] += value;
//   - otherwise, a key other than "RANK" whose badge is not earned: map[key] = value;
//   - "RANK" (non-null buffer, __stricmp): map["RANK"] = value when value > map["RANK"];
//   - anything else is left alone.
struct RankAwardHost {
    virtual ~RankAwardHost() = default;
    // 0090C000 on [game+21A0h] with the key by value.
    virtual bool badge_earned_0090c000(const std::string& key) = 0;
};
void evaluate_rank_award_007fcd90(UnlockCounterMap& awards_a0,
                                  const std::vector<std::pair<std::string, std::int32_t>>& record_6c,
                                  bool keep_earned, RankAwardHost& host);

// 007FFB50, __fastcall(ECX = const NativeString*): the first record of the race table (00F87464/68)
// whose name +8h equals it (equal length, then empty or __stricmp), or null. Checked iterators.
RaceRecord* race_records_find_by_name_007ffb50(const RaceRecordTable& table, const std::string& name);
// 007FFD60: every non-null record deleted through vtable slot 0 (007FFB10) with 1, slot cleared.
void race_records_delete_all_007ffd60(RaceRecordTable& table, NativeStringStorage& strings);

// ===================================================================================================
// Small leaves.
// ===================================================================================================

// 007FF320, __fastcall(ECX = &owned): a non-null object is deleted through vtable slot 0 with 1
// and the pointer cleared.
void owned_object_ptr_reset_007ff320(void*& object, void (*deleting_destructor)(void*, std::uint32_t));

// 008003B0, __cdecl(float): the rain drop-count fraction, one store to [00F87470].
struct RainDropSettingHost {
    virtual ~RainDropSettingHost() = default;
    virtual void store_drop_num_percent_00f87470(float fraction) = 0;
};
void rain_set_drop_num_percent_008003b0(RainDropSettingHost& host, float fraction);

// 00AAE390 / 00AAE3F0: copy constructors of two GUI records that start with a NativeString.
struct GuiStringRecord14 {          // 14h: string, +08h, +0Ch, bytes +10h/+11h
    std::string text{};
    std::uint32_t word_08{0};
    std::uint32_t word_0c{0};
    std::uint8_t byte_10{0};
    std::uint8_t byte_11{0};
};
struct GuiStringRecord24 {          // 24h: string, +08h..+18h, byte +1Ch, +20h
    std::string text{};
    std::uint32_t words_08[5]{};
    std::uint8_t byte_1c{0};
    std::uint32_t word_20{0};
};
GuiStringRecord14 gui_string_record14_copy_00aae390(const GuiStringRecord14& in);
GuiStringRecord24 gui_string_record24_copy_00aae3f0(const GuiStringRecord24& in);

// 00ABF5B0, the Section copy constructor's own part (after the base copy).
void gui_section_copy_00abf5b0(GuiSectionFields& out, const GuiSectionFields& in,
                               GuiIconStateHost& host);

// 00AC5820, __fastcall(ECX = layer): the GUI editor backup. The script path (00AC5770), the
// backup name path + ".guiedbak", remove(backup), rename(path, backup), then the layer written
// back to `path` under "GuiScreen".
struct GuiLayerBackupHost {
    virtual ~GuiLayerBackupHost() = default;
    virtual void compose_script_path(const std::string& layer_name, std::string& out) = 0;
    virtual void remove_file_00bffd99(const std::string& path) = 0;
    virtual void rename_file_00bffdc8(const std::string& from, const std::string& to) = 0;
    virtual void write_layer_archive(void* layer, const std::string& path, const char* section) = 0;
};
void gui_layer_save_backup_00ac5820(void* layer, const std::string& layer_name_100,
                                    GuiLayerBackupHost& host);

// ===================================================================================================
// Segment 49: the rain model (vtable 00D08D58, allocated from the 532-byte pool 00800EC0).
// ===================================================================================================

// One 14h element of the +184h vector, written by the build (008023B0): a column centre and the
// two quad extents.
struct RainColumn {
    float x_00{0.0f};
    float y_04{0.0f};
    float z_08{0.0f};
    float width_0c{0.0f};
    float height_10{0.0f};
};

// The part past the generated-model base. refs_1ac are six counted references (+1ACh..+1C0h);
// +1B8h and +1C0h are point effects. +1C4h..+1F7h is a copy of the 34h descriptor (008019A0):
// name, then words +1CCh..+1F4h.
struct RainModelFields {
    std::vector<RainColumn> vector_188{};    // +184h object, +188h..+190h, POD (only _free on destruction)
    float position_194[3]{};                  // +194h..+19Ch, set by the build from the camera
    float vector_1a0[3]{};                    // +1A0h..+1A8h, zeroed by the build
    void* refs_1ac[6]{};
    std::string name_1c4{};
    float descriptor_1cc[5]{};                // +1CCh..+1DCh: count, +0Ch..+18h of the descriptor
    float direction_1e0[3]{};                 // +1E0h..+1E8h, the rain direction
    float drop_1ec{0.0f};                     // +1ECh, scales the node's +110h row
    float box_radius_1f0{0.0f};               // +1F0h, the shader's "box_radius"
    std::uint32_t color_1f4{0};
    float offset_1f8[3]{};
    float position_204[3]{};
    float clock_210{0.0f};
};

// The 34h parameter record 008019A0 assigns: a NativeString and 11 words.
struct RainModelParams {
    std::string name{};
    std::uint32_t words_08[11]{};
};

struct RainModelHost {
    virtual ~RainModelHost() = default;
    virtual void store_drop_num_percent_00f87470(float fraction) = 0;
    virtual void effect_stop_children_00867b10(void* effect) = 0;
    virtual void effect_set_byte_9(void* effect) = 0;
    virtual void release(void* object) = 0;           // InterlockedDecrement, slot 0 at zero
    virtual float clock_00f876a4() = 0;
    virtual void* context_node_8(void* context) = 0;
    virtual bool node_hidden_198(void* node) = 0;
    virtual void node_refresh_world_if_stale(void* node) = 0;
    virtual void node_world_row_110(void* node, float out[3]) = 0;
    virtual void node_world_translation_120(void* node, float out[3]) = 0;
    virtual void model_virtual_34(void* model, void* node) = 0;
    virtual void render_00b748e0(void* model, void* context, float a, float b, std::uint32_t c) = 0;
};

// 00802250, __thiscall(model, arg), returning ECX: the base, then the stores above; it also resets
// the global drop fraction [00F87470] to 1.0.
void rain_model_construct_00802250(RainModelFields& f, RainModelHost& host);
// 00801B70, the destructor body: effects stopped, name released, references released in reverse,
// the vector freed, then BSP_Model_Destroy. 00801DB0 adds the pool free (008007A0) for flag 1.
void rain_model_destroy_00801b70(RainModelFields& f, RainModelHost& host);
// 008019A0: NativeString assignment (skipped for self) and 11 words.
RainModelParams& rain_model_params_assign_008019a0(RainModelParams& out, const RainModelParams& in);
// 00800B20, __thiscall(model, context, float a, float b, uint c), RET 10h: unless the node is
// hidden, clear +1F8h..+200h, stamp the clock, position +204h = drop * row(+110h) + origin(+120h)
// in float, vtable +34h(node+F0h), then 00B748E0(context, a, b, c).
void rain_model_submit_00800b20(RainModelFields& f, void* self, void* context, float a, float b,
                                std::uint32_t c, RainModelHost& host);

// The rain model's pool allocator (Mit_PoolAllocator<532, 2>, vtable 00D08D54, registered on the
// allocator list 00E188B4). A page is 438h bytes: two 218h slots (532-byte payload, then the page
// index at +214h), a free-slot stack of two uint16 at +430h and the free count at +434h.
struct RainPoolPage {
    std::array<std::array<std::uint8_t, 0x214>, 2> slots{};
    std::uint32_t slot_page_index_214[2]{};
    std::uint16_t free_stack_430[2]{};
    std::uint16_t free_count_434{0};
};
struct RainPool {
    std::vector<std::unique_ptr<RainPoolPage>> pages{};  // +28h data, +2Ch count
    std::size_t capacity_30{0};
    std::int32_t lowest_free_34{-1};                     // lowest page with a free slot, or -1
};
struct RainPoolHost {
    virtual ~RainPoolHost() = default;
    virtual void registry_link_head_00e188b4(RainPool& pool) = 0;
    virtual void registry_unlink_00e188b4(RainPool& pool) = 0;
    virtual void initialize_critical_section_0c(RainPool& pool) = 0;
    virtual void lock_0c(RainPool& pool) = 0;      // EnterCriticalSection(+0Ch), +24h + 1
    virtual void unlock_0c(RainPool& pool) = 0;    // +24h - 1, LeaveCriticalSection
    virtual void release_lock_fully_and_delete_0c(RainPool& pool) = 0;
};
void rain_pool_page_construct_00800450(RainPoolPage& page, std::uint32_t page_index);  // RET 4
void rain_pool_construct_00800c60(RainPool& pool, RainPoolHost& host);
void* rain_pool_allocate_00800ec0(RainPool& pool, RainPoolHost& host);
// 008007A0, __thiscall(pool, slot pointer), RET 4; 00800B10 is a thunk to it. The model takes
// the page index (+214h word) and slot number the native derives from the pointer.
void rain_pool_free_008007a0(RainPool& pool, RainPoolHost& host, std::uint32_t page_index,
                             std::uint16_t slot);
void rain_pool_trim_00800e20(RainPool& pool);  // vtable slot 0 (00D08D54)
void rain_pool_destroy_00800d90(RainPool& pool, RainPoolHost& host);

// The emitter step's calls beyond the model host.
struct RainEmitterHost : RainModelHost {
    virtual float drop_num_percent_00f87470() = 0;
    // 00BD2F10 with ECX = 1 (stream 1 of the thread generator, not gameplay stream 0): U(lo, hi).
    virtual float random_uniform_00bd2f10_stream1(float lo, float hi) = 0;
    virtual void* camera_node_19fc() = 0;                       // [game+19FCh]
    virtual void add_ref(void* object) = 0;                     // InterlockedIncrement(+4h)
    virtual void* create_point_effect_008687c0(void* parent, void* definition,
                                               const float matrix[16]) = 0;
    virtual void effect_set_velocity_5c(void* effect, const float velocity[3]) = 0;
    virtual void effect_set_word_28(void* effect, std::uint32_t value) = 0;
};
// 00801400, __thiscall(model, float dt, const camera* (altitude at +4h), unused), RET 0Ch. With
// k = 100 * [00F87470]: a stream-1 draw U(0, k*dt) above dt*0.3*k spawns a one-shot splash
// (+1ACh) when the camera is above 0; k > 25 starts the sheet effect +1B8h (from +1B4h, velocity
// 100 * direction) and, below altitude 200, the near effect +1C0h (from +1BCh); altitude above 200
// or k below 25 stops +1C0h.
void rain_emitter_step_00801400(RainModelFields& f, float dt, float camera_altitude_4,
                                RainEmitterHost& host);

// The calls of the rain build (008023B0) beyond the emitter host. Renderer calls are the vtable
// slots of [00F8D394]; buffer and stream objects are refcounted (release = InterlockedDecrement
// of +4h, slot 0 at zero).
struct RainModelRef {
    void* object{nullptr};             // the pool slot (vtable 00D08D58)
    RainModelFields* fields{nullptr};  // its +184h.. part
};
struct RainBuildHost : RainEmitterHost {
    // 00800EC0 on the pool 00F87478, then 00802250 with the name "RainModel" when the slot is
    // non-null. The native does not test the result before 008019A0.
    virtual RainModelRef allocate_rain_model_00800ec0(const std::string& name) = 0;
    // 00BD2F10 with ECX = 0: U(lo, hi) on stream 0, the shared gameplay stream (census sites
    // 00802507..0080266D, 008029B0, 008029D1). The image shares stream 0 here; a host that wires
    // the build in must give it a separate presentation stream or keep it behind an OFF switch.
    virtual float random_uniform_00bd2f10_stream0(float lo, float hi) = 0;
    virtual void* mesh_allocate_construct_00b73b60_bc() = 0;      // 00B73B60(BCh), 00B73D70
    virtual void generated_model_set_geometry_00b75170(void* model, std::uint32_t slot, void* mesh,
                                                       float a, float b) = 0;
    virtual void geometry_set_bounds_00b72650(void* mesh, const float sphere[4]) = 0;
    virtual void* renderer_vertex_format_38(const std::string& name) = 0;
    virtual void* renderer_vertex_buffer_5c(std::uint32_t vertices, std::uint32_t flag, void* format) = 0;
    virtual void* renderer_index_buffer_60(std::uint32_t indices, std::uint32_t flag,
                                           std::uint32_t format) = 0;
    virtual void* renderer_vertex_declaration_40(void* format, std::uint32_t word_10) = 0;
    virtual void* renderer_texture_by_name_64(const std::string& name, std::uint32_t flag) = 0;
    virtual void geometry_set_vertex_stream_00b73bb0(void* mesh, std::uint32_t index, void* stream) = 0;
    virtual void geometry_set_index_stream_00b73b70(void* mesh, void* stream) = 0;
    virtual std::uint16_t* index_buffer_lock_0c(void* buffer) = 0;    // vtable +0Ch(0, 0, 0)
    virtual void index_buffer_unlock_10(void* buffer) = 0;            // vtable +10h
    virtual float* vertex_buffer_lock_10(void* buffer) = 0;           // vtable +10h(0, 0, 0)
    virtual void vertex_buffer_unlock_14(void* buffer) = 0;           // vtable +14h
    virtual void store_vertex_count_00f87474(std::uint32_t count) = 0;
    virtual std::uint32_t vertex_count_00f87474() = 0;
    virtual void* material_create_00535320(const std::string& shader) = 0;
    virtual float* lightning_intensity_010909b4() = 0;
    virtual void material_register_float_00b18b20(void* material, const std::string& name,
                                                  float* value) = 0;
    virtual void material_register_float3_00b18ae0(void* material, const std::string& name,
                                                   float* value) = 0;
    virtual void material_set_texture_slot_00b189f0(void* material, std::uint32_t slot,
                                                    void* texture) = 0;
    virtual void* draw_section_allocate_construct_00b85ee0() = 0;     // 60h, 00B857F0
    virtual void* geometry_vertex_stream_00b73260(void* mesh, std::uint32_t index) = 0;
    virtual void draw_section_append_vertex_stream_00b85b80(void* section, void* stream) = 0;
    virtual void draw_section_set_vertex_layout_00b86650(void* section, void* declaration) = 0;
    virtual void draw_section_set_material_00b864c0(void* section, void* material) = 0;
    virtual void geometry_append_draw_section_00b73c60(void* mesh, void* section) = 0;
    // Plain stores to the section: +8h, +0Ch, +10h, +14h, +18h.
    virtual void draw_section_store_words(void* section, std::uint32_t word_08, std::uint32_t word_0c,
                                          std::uint32_t word_10, std::uint32_t word_14,
                                          std::uint32_t word_18) = 0;
    virtual void mesh_element_set_bounds_00b855b0(void* section, const float sphere[4]) = 0;
    virtual void mesh_clear_draw_section_owners_00b73c10(void* mesh) = 0;
    virtual void* scene_root_19ec() = 0;                              // [game+19ECh]
    virtual void node_attach_root_00b6d890(void* model, void* root) = 0;
    // 00870CD0 / 00871BA0: a retained temporary handle (the caller releases it).
    virtual void* effect_acquire_00870cd0(std::uint32_t id, std::uint32_t flag) = 0;
    virtual void* effect_acquire_by_name_00871ba0(const std::string& name, std::uint32_t flag) = 0;
};
// 008023B0, __fastcall(ECX = the 34h descriptor), returning the model (sole caller
// BSP_WorldAtmosphere_ApplyPreset 00865900). Descriptor floats as RainModelParams words: count +8h,
// height base +14h and spread +18h, width base +0Ch and spread +10h, direction +1Ch..+24h, box
// radius +2Ch, colour +30h. Builds `count` quads (four vertices of 2Ch bytes,
// "pf43ccuf42uf42uf43.mvfm", six 16-bit indices), the "rain.mshd" material and the three effect
// references. Seven draws on shared stream 0, routed through the host.
RainModelRef rain_emitter_build_008023b0(const RainModelParams& descriptor, RainBuildHost& host);

// 007FBA30, __thiscall(record, a, b, c, const uint8* bytes, int count), RET 14h (caller: the
// session browser filter edit 0057A910): three words at +20h..+28h, the byte list +14h (head
// +18h, size +1Ch) emptied and refilled with `count` bytes, +2Ch = count (also when below 1).
struct ByteListRecord {
    std::vector<std::uint8_t> bytes_14{};
    std::uint32_t words_20[3]{};
    std::int32_t count_2c{0};
};
void byte_list_record_assign_007fba30(ByteListRecord& record, std::uint32_t a, std::uint32_t b,
                                      std::uint32_t c, const std::uint8_t* bytes, std::int32_t count);

// ===================================================================================================
// Segment 78, continued (cc13-left3): the movie widget's loaded hook.
// ===================================================================================================

// A vertex buffer's colour element as 00AAE770 writes it: base +8h, stride +0Ch, and either a
// packed colour offset +34h (>= 0) or four float channel offsets +38h..+44h.
struct GuiQuadVertexColorLayout {
    std::int32_t packed_offset_34{-1};
    std::int32_t channel_offsets_38[4]{};
    std::uint32_t stride_0c{0};
};
struct GuiMovieLoadedHookHost {
    virtual ~GuiMovieLoadedHookHost() = default;
    virtual void deactivate_and_refresh_bounds_00aa7170(void* widget) = 0;
    // [00F8AEFC] vtable +4h with the byte widget+F4h: the movie player, stored at +F0h.
    virtual void* movie_player_00f8aefc_4(std::uint8_t index_f4) = 0;
    virtual bool movie_player_virtual_44(void* player) = 0;
    virtual void* generated_model_4c(void* widget) = 0;
    virtual void* mesh_allocate_construct_00b73b60_bc() = 0;
    virtual void generated_model_set_geometry_00b75170(void* model, std::uint32_t slot, void* mesh,
                                                       float a, float b) = 0;
    virtual void release(void* object) = 0;                // InterlockedDecrement(+4h), slot 0 at zero
    virtual void* renderer_vertex_format_38(const std::string& name) = 0;
    virtual void* renderer_vertex_buffer_5c(std::uint32_t vertices, std::uint32_t flag, void* format) = 0;
    virtual void* renderer_index_buffer_60(std::uint32_t indices, std::uint32_t flag,
                                           std::uint32_t format) = 0;
    virtual void geometry_set_vertex_stream_00b73bb0(void* mesh, std::uint32_t index, void* stream) = 0;
    virtual void geometry_set_index_stream_00b73b70(void* mesh, void* stream) = 0;
    virtual std::uint16_t* index_buffer_lock_0c(void* buffer, std::uint32_t count) = 0;  // (count, 0, 0)
    virtual void index_buffer_unlock_10(void* buffer) = 0;
    virtual void vertex_buffer_lock_10(void* buffer, std::uint32_t count) = 0;          // (count, 0, 0)
    virtual void vertex_buffer_unlock_14(void* buffer) = 0;
    virtual GuiQuadVertexColorLayout vertex_color_layout(void* buffer) = 0;
    virtual void vertex_store_dword(void* buffer, std::uint32_t vertex, std::int32_t offset,
                                    std::uint32_t value) = 0;
    virtual void vertex_store_float(void* buffer, std::uint32_t vertex, std::int32_t offset, float value) = 0;
    virtual void* draw_section_create_00533fa0() = 0;
    virtual void* material_create_00535320(const std::string& shader) = 0;
    virtual void draw_section_store_words(void* section, std::uint32_t word_08, std::uint32_t word_0c,
                                          std::uint32_t word_10, std::uint32_t word_14,
                                          std::uint32_t word_18) = 0;
    virtual void widget_virtual_58(void* widget) = 0;      // vtable +58h with widget+20h
    virtual void widget_virtual_50(void* widget) = 0;      // vtable +50h with widget+50h
    virtual void draw_section_set_material_00b864c0(void* section, void* material) = 0;
    virtual void draw_section_rebuild_vertex_layout_00b865a0(void* section, void* mesh) = 0;
    virtual void geometry_append_draw_section_00b73c60(void* mesh, void* section) = 0;
    virtual void set_source_00aae320(void* widget, std::uint32_t loop) = 0;  // (&widget+FCh, 1)
};
// 00AAE770, __fastcall(ECX = widget), plain RET; vtable 00D5C270 slot 30. Stores the player at
// +F0h; unless +74h is set or the player's vtable +44h answers true, it builds a one-quad mesh
// ("SimpleColor.mvfm", four white vertices, indices 0 1 2 0 2 3, "guidefault.mshd"). Then, when
// the source string +FCh is non-empty (size word non-zero), 00AAE320 with loop 1.
void gui_movie_loaded_hook_00aae770(void* widget, void*& player_f0, bool flag_74,
                                    std::uint32_t source_size_fc, std::uint8_t index_f4,
                                    GuiMovieLoadedHookHost& host);

// 00AB4CE0, __thiscall(ECX = anim icon, const NativeString* prefix), RET 4; vtable 00D5BCE0 slot
// 27. For each 40h frame record of +F4h..+F8h it builds prefix + "." + i + ":" + name(+4h) + "(" +
// texture name(+0h, 00B33E40) + ")" and destroys it unused; then the base 00AAA0F0(prefix). The
// returned list is the strings the image builds and drops.
struct GuiAnimIconFrame {
    void* texture_00{nullptr};
    std::string name_04{};
};
struct GuiAnimIconVisitHost {
    virtual ~GuiAnimIconVisitHost() = default;
    virtual std::string texture_name_00b33e40(void* texture) = 0;
    virtual void visit_child_paths_00aaa0f0(const std::string& prefix) = 0;
};
std::vector<std::string> gui_anim_icon_visit_child_paths_00ab4ce0(
    const std::vector<GuiAnimIconFrame>& frames_f4, const std::string& prefix,
    GuiAnimIconVisitHost& host);

// The widget calls the segment-78 layout bodies make. Sizes and positions come back as the floats
// the native reads through the returned pointer.
struct GuiLayoutHost {
    virtual ~GuiLayoutHost() = default;
    virtual std::array<float, 2> widget_size_00aa6740(void* widget) = 0;
    virtual std::array<float, 3> widget_resolved_position_00aa6750(void* widget) = 0;
    virtual std::array<float, 3> widget_local_position_00aa67f0(void* widget) = 0;
    virtual void widget_set_local_xy_00aa7d00(void* widget, float x, float y) = 0;
    virtual void widget_set_local_y_00aa78f0(void* widget, float y) = 0;
    virtual void widget_set_local_position_00aa7dc0(void* widget, const float xyz[3]) = 0;
    virtual void* widget_parent_70(void* widget) = 0;                  // [widget+70h]
    virtual void widget_virtual_58(void* widget, const std::array<float, 2>& size) = 0;
    virtual void widget_virtual_30(void* widget, void* source) = 0;    // with source+18h
    virtual std::int32_t widget_virtual_5c(void* widget) = 0;          // 3 = text
    virtual void widget_virtual_34(void* widget, std::uint32_t visible) = 0;
    virtual void widget_virtual_50(void* widget, const float rect[4]) = 0;
    virtual float text_normalized_height_00ab6bd0(void* text) = 0;
    // 00AC06C0, ECX widget, EDX size out, stack (offset out, padding float[2], float).
    virtual void measure_padded_relative_bounds_00ac06c0(void* widget, float size[2], float offset[2],
                                                         const float padding[2], float extra) = 0;
    virtual GuiFrameBoxBorderIconLayout border_icon_layout_00ab6cb0(void* widget) = 0;
    // (short)[[widget+108h]+14h], or 0 when the font is null.
    virtual std::int32_t text_font_height_108(void* widget) = 0;
    virtual void* widget_find_child_00aa7e00(void* widget, const std::string& name, std::uint32_t flag) = 0;
    // 00AC0970's vertex work: vtable +A0h gives the stream; +10h(0,0,0) locks, +14h unlocks.
    virtual void* widget_vertex_stream_a0(void* widget) = 0;
    virtual void stream_lock_10(void* stream) = 0;
    virtual void stream_unlock_14(void* stream) = 0;
    virtual std::array<float, 3> stream_read_position_004768d0(void* stream, std::uint32_t index) = 0;
    // A plain store at base +8h + index * stride +0Ch + position offset +10h.
    virtual void stream_store_position(void* stream, std::uint32_t index, const float xyz[3]) = 0;
    virtual void* widget_generated_model_4c(void* widget) = 0;
    virtual void* generated_model_geometry_00b74640(void* model, std::uint32_t index) = 0;
    virtual void* mesh_element_00b732c0(void* geometry, std::uint32_t index) = 0;
    virtual void draw_section_rebuild_vertex_layout_00b865a0(void* section, void* geometry) = 0;
};

// 00AC0970, __stdcall(widget, float w, float h), RET 0Ch (ECX unused; callers load the layout
// object into it). A size below 0 other than -1.0 (00D7A260, UCOMISS) hides the widget (vtable
// +34h(0)). Otherwise it shows it (+34h(1)), locks the vtable +A0h stream and, for w > 0, moves
// vertices 1 and 3 to x = w unless vertex 1 already has it; for h > 0, vertices 2 and 3 get
// y = 0.75 * h ([00E12FF0], .data, only two literal readers and no literal writer) unless the
// y of vertex 2 equals h. Then unlock and rebuild the first draw section's vertex layout.
void gui_frame_box_set_icon_extent_00ac0970(void* widget, float w, float h, GuiLayoutHost& host);

// The layout object of 00AC0610/00AC0BD0/00AC10A0 (vtables 00D5C980/00D5C984): +4h the icon
// widget, +8h the text-icon container, +0Ch/+10h the extra size, +14h..+30h the eight border
// pieces UL, U, UR, L, R, BL, B, BR.
struct GuiFrameBoxLayout {
    void* icon_04{nullptr};
    void* container_08{nullptr};
    float extra_0c{0.0f};
    float extra_10{0.0f};
    void* pieces_14[8]{};
};
// 00AC0610, __thiscall(layout, widget source, unused), RET 8: the icon goes to source's
// resolved position relative to the icon's parent, takes source's size (vtable +58h) and
// +18h (vtable +30h), and is placed at that offset minus half of +0Ch/+10h.
void gui_frame_box_place_border_icon_00ac0610(const GuiFrameBoxLayout& layout, void* source,
                                              GuiLayoutHost& host);
// 00AC0BD0, __thiscall(layout, widget), RET 4; vtable 00D5C980 slot 1: the icon's height is
// h = (the widget's height, or for text 00AB6BD0) + extra_10; extent (-1, h * 4/3 (00CF5750)); then
// icon y = widget y + (icon height - h) * 0.5 - h / 6.0 (00CE6628).
void gui_text_icon_pair_layout_00ac0bd0(const GuiFrameBoxLayout& layout, void* widget,
                                        GuiLayoutHost& host);
// 00AC10A0, __thiscall(layout, widget), RET 4: measures the widget with padding (0.04, 0.02)
// (00CE3910, 00CE9BAC) and lays out the eight pieces around it, then places the icon (00AC0610).
// For a text widget (vtable +5Ch == 3) the container follows the icon at z +-0.1 and, when the
// widget's border flag +15Ch is set, the container's eight named children are laid out the same
// way, shifted by the font height / 720 * +164h, and given the +168h rect (vtable +50h).
void gui_frame_box_layout_border_icons_00ac10a0(const GuiFrameBoxLayout& layout, void* widget,
                                                GuiLayoutHost& host);

// 00AB9240, __thiscall(ECX = text, uint index), RET 4: the text cursor quad.
struct GuiGlyphMetrics {
    std::int16_t bearing_10{0};
    std::uint16_t advance_12{0};
    std::uint16_t width_14{0};
};
struct GuiTextCursorFields {
    std::uint32_t length_ec{0};
    const std::uint16_t* glyphs_f0{nullptr};  // null -> 00F8BE50
    void* font_108{nullptr};
    std::uint32_t mode_17c{0};
    std::uint32_t index_180{0};
    void* cursor_model_184{nullptr};
    float x_start_18c{0.0f};
    float x_190{0.0f};
    float advance_scale_1d8{0.0f};
};
struct GuiTextCursorHost {
    virtual ~GuiTextCursorHost() = default;
    virtual void ensure_cursor_model_00ab8910(void* text) = 0;
    virtual std::uint16_t empty_glyph_00f8be50() = 0;
    virtual GuiGlyphMetrics font_select_glyph_00ad4480(void* font, std::uint16_t ch) = 0;
    virtual std::int16_t font_height_14(void* font) = 0;
    virtual float vertical_scale_00e12fd4() = 0;          // .data, image value 0.75
    virtual void* cursor_vertex_stream(void* model) = 0;  // 00B74640(0), 00B73260(0)
    virtual void stream_lock_10(void* stream) = 0;
    virtual void stream_unlock_14(void* stream) = 0;
    virtual GuiQuadVertexColorLayout vertex_color_layout(void* stream) = 0;
    virtual void stream_store_position(void* stream, std::uint32_t index, const float xyz[3]) = 0;
    virtual void stream_store_uv_28(void* stream, std::uint32_t index, const float uv[2]) = 0;
    virtual void vertex_store_dword(void* stream, std::uint32_t index, std::int32_t offset,
                                    std::uint32_t value) = 0;
    virtual void vertex_store_float(void* stream, std::uint32_t index, std::int32_t offset, float value) = 0;
};
void gui_text_position_cursor_00ab9240(void* text, GuiTextCursorFields& f, std::uint32_t index,
                                       GuiTextCursorHost& host);

}  // namespace bsp::cc13_seg49_64_78
