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

#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "bsp/gui_lua_reader.hpp"
#include "bsp/options_hardware_cc13.hpp"

namespace bsp::cc13_seg49_64_78 {

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

}  // namespace bsp::cc13_seg49_64_78
