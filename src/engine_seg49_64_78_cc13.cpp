// Packet cc13-sys6: segments 49, 64 and 78. See docs/CC13_SYS_6.md and the header for
// addresses, ABI and evidence. Comments cite the instruction each statement stands for.
#include "bsp/engine_seg49_64_78_cc13.hpp"

namespace bsp::cc13_seg49_64_78 {
ReconStyleRecord* recon_default_style_008036f0(ReconStyleFallback& state) noexcept {
    if ((state.flags & 1u) == 0) {
        state.flags |= 1u;
        state.record = {{0.0f, 0.0f, 0.0f, 1.0f}};
    }
    return &state.record;
}
const ReconStyleRecord* recon_relation_style_00803c20(int own, int other, std::uint32_t index,
    const ReconStyleRecord* same, const ReconStyleRecord* enemy,
    const ReconStyleRecord* neutral, const ReconStyleRecord* unknown, ReconStyleFallback& state) noexcept {
    if (other == 3) return unknown + index;
    if (other == 2 || own == 2) return neutral + index;
    if (own == 0) {
        if (other == 0) return same + index;
        if (other == 1) return enemy + index;
    } else if (own == 1) {
        if (other == 0) return enemy + index;
        if (other == 1) return same + index;
    }
    return recon_default_style_008036f0(state);
}
}

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <utility>

#include "bsp/band_0067_cc13.hpp"
#include "bsp/gui_widgets_cc13.hpp"

namespace bsp::cc13_seg49_64_78 {
namespace {

GuiLuaVariant name_key(const char* name) noexcept
{
    return gui_lua_key_by_name(name);
}

GuiLuaVariant index_key(std::int32_t index) noexcept
{
    return gui_lua_key_by_index(index);
}

GuiLuaVariant field(GuiLuaFieldType type, void* dest) noexcept
{
    return gui_lua_field(type, dest);
}

GuiLuaVariant int_value(std::int32_t value) noexcept
{
    GuiLuaVariant v{};
    v.tag = static_cast<std::int32_t>(GuiLuaFieldType::Int);
    v.value.integer = value;
    return v;
}

GuiLuaVariant float_value(float value) noexcept
{
    GuiLuaVariant v{};
    v.tag = static_cast<std::int32_t>(GuiLuaFieldType::Float);
    v.value.number = value;
    return v;
}

GuiLuaVariant bool_value(bool value) noexcept
{
    GuiLuaVariant v{};
    v.tag = static_cast<std::int32_t>(GuiLuaFieldType::Bool);
    // 0096E58B stores only the low byte of the word; the upper three bytes are stale stack.
    v.value.integer = value ? 1 : 0;
    return v;
}

GuiLuaVariant string_value(const char* text) noexcept
{
    GuiLuaVariant v{};
    v.tag = static_cast<std::int32_t>(GuiLuaFieldType::String);
    v.value.text = text;
    return v;
}

// The 250-pair key array every set loader stages: each tag FFFFFFFFh, count zero
// (009721EB..009721FC, then [ESP+800h] = 0 before the +18h call).
void stage_keys(GuiLuaKeyArray& keys) noexcept
{
    gui_lua_clear_key_array_00aaa710(keys);
}

const EventParam& param_at(const EventParams& params, std::size_t index,
                           EventSubscriptionHost& host)
{
    // 00968E89..00968E95: begin non-null and (end - begin) / 4 > index, else 00BF6713.
    if (params.size() <= index) host.invalid_parameter_00bf6713();
    return *params[index];
}

} // namespace

bool StricmpLess::operator()(const std::string& left, const std::string& right) const noexcept
{
    // 00443D00: an empty string compares as "", otherwise CRT _stricmp on the two C strings.
#if defined(_MSC_VER)
    return _stricmp(left.c_str(), right.c_str()) < 0;
#else
    return strcasecmp(left.c_str(), right.c_str()) < 0;
#endif
}

// 00979140: size +0Ch zero -> true; else find(param+4h) (009790D0) and false when it is end.
bool EntityIdFilter::matches(const EventParam& param) const
{
    if (ids.empty()) return true;
    return ids.count(static_cast<std::uint16_t>(param.word04)) != 0;
}

// 00979320: the same over the dword at param+4h (find 009792B0).
bool IntFilter::matches(const EventParam& param) const
{
    if (values.empty()) return true;
    return values.count(static_cast<std::int32_t>(param.word04)) != 0;
}

// 00979510: the key is (dword at param+4h != 0) (find 00979490).
bool BoolFilter::matches(const EventParam& param) const
{
    if (values.empty()) return true;
    return values.count(param.word04 != 0) != 0;
}

// 0097BC70: the NativeString at param+4h through BSP_NativeStringSet_Find 004C7F10.
bool TextFilter::matches(const EventParam& param) const
{
    if (values.empty()) return true;
    return values.count(param.text04) != 0;
}

// 00968440: FLD [param+4]; FCOMI against min, JB -> false; FLD max; FCOMIP, JB -> false. An
// unordered compare sets CF, so a NaN on either side is false.
bool RangeFilter::matches(const EventParam& param) const
{
    const float value = param.number04;
    if (!(value >= min)) return false;
    if (!(max >= value)) return false;
    return true;
}

void load_callback_0096aa90(EventSubscription& subscription, EventArchiveReader& reader)
{
    // (**(+10h))({0, "callback"}, {0, this+4})
    reader.read_10(name_key("callback"), field(GuiLuaFieldType::String, &subscription.callback));
}

void save_callback_0096d2b0(const EventSubscription& subscription, EventArchiveWriter& writer)
{
    // data pointer +8h, or 00F8A0C8 ("") when null; written as a String under "callback".
    writer.write_0c(name_key("callback"), string_value(subscription.callback.c_str()));
}

void load_entity_set_009721c0(EventArchiveReader& reader, EventSubscriptionHost& host,
                              const char* key, EntityIdFilter& filter)
{
    reader.enter_04(name_key(key));                         // 009721E5
    GuiLuaKeyArray keys{};
    stage_keys(keys);
    reader.enumerate_keys_18(keys);                         // 00972213
    // Neither destination is cleared per key ([ESP+1Ch] handle, [ESP+14h] int): a key whose
    // value the reader leaves alone keeps the previous key's result (on the first key, whatever
    // the frame held; zero here).
    std::int32_t handle = 0;
    std::int32_t number = 0;
    for (std::int32_t i = 0; i < keys.count; ++i) {
        reader.read_10(keys.keys[i], field(GuiLuaFieldType::Handle, &handle));   // 00972254
        std::uint16_t id;
        if (handle != 0) {
            id = host.entity_id_174(handle);                 // 0097225E MOVZX [EAX+174h]
        } else {
            reader.read_10(keys.keys[i], field(GuiLuaFieldType::Int, &number));  // 00972287
            id = static_cast<std::uint16_t>(number);         // 00972289 MOVZX word
        }
        filter.ids.insert(id);                               // 009722A0 00971190
    }
    reader.leave_08();
}

void load_int_set_009722d0(EventArchiveReader& reader, const char* key, IntFilter& filter)
{
    reader.enter_04(name_key(key));
    GuiLuaKeyArray keys{};
    stage_keys(keys);
    reader.enumerate_keys_18(keys);
    std::int32_t number = 0;  // not cleared per key, as in 009721C0
    for (std::int32_t i = 0; i < keys.count; ++i) {
        reader.read_10(keys.keys[i], field(GuiLuaFieldType::Int, &number));
        filter.values.insert(number);                        // 00971250
    }
    reader.leave_08();
}

void load_bool_set_00972390(EventArchiveReader& reader, const char* key, BoolFilter& filter)
{
    reader.enter_04(name_key(key));
    GuiLuaKeyArray keys{};
    stage_keys(keys);
    reader.enumerate_keys_18(keys);
    bool flag = false;  // not cleared per key
    for (std::int32_t i = 0; i < keys.count; ++i) {
        reader.read_10(keys.keys[i], field(GuiLuaFieldType::Bool, &flag));
        filter.values.insert(flag);                          // 00971310
    }
    reader.leave_08();
}

void load_string_set_00970ff0(EventArchiveReader& reader, const char* key, TextFilter& filter)
{
    reader.enter_04(name_key(key));
    GuiLuaKeyArray keys{};
    stage_keys(keys);
    reader.enumerate_keys_18(keys);
    for (std::int32_t i = 0; i < keys.count; ++i) {
        std::string text;  // a fresh NativeString per key, released after the insert (00419CC0)
        reader.read_10(keys.keys[i], field(GuiLuaFieldType::String, &text));
        filter.values.insert(text);                          // 009710BD 004D0640
    }
    reader.leave_08();
}

void load_range_0096aad0(EventArchiveReader& reader, const char* key, RangeFilter& filter)
{
    reader.enter_04(name_key(key));
    reader.read_or_default_0c(name_key("min"), field(GuiLuaFieldType::Float, &filter.min),
                              float_value(kRangeFilterDefaultMin));
    reader.read_or_default_0c(name_key("max"), field(GuiLuaFieldType::Float, &filter.max),
                              float_value(kRangeFilterDefaultMax));
    reader.leave_08();
}

void save_entity_set_0096e3a0(EventArchiveWriter& writer, const char* key,
                              const EntityIdFilter& filter)
{
    writer.begin_04(name_key(key));
    std::int32_t index = 0;
    for (const std::uint16_t id : filter.ids) {
        writer.write_0c(index_key(index), int_value(id));   // 0096E403 MOVZX word, tag 1
        ++index;
    }
    writer.end_08();
}

void save_int_set_0096e460(EventArchiveWriter& writer, const char* key, const IntFilter& filter)
{
    writer.begin_04(name_key(key));
    std::int32_t index = 0;
    for (const std::int32_t value : filter.values) {
        writer.write_0c(index_key(index), int_value(value));
        ++index;
    }
    writer.end_08();
}

void save_bool_set_0096e520(EventArchiveWriter& writer, const char* key, const BoolFilter& filter)
{
    writer.begin_04(name_key(key));
    std::int32_t index = 0;
    for (const bool value : filter.values) {
        writer.write_0c(index_key(index), bool_value(value));
        ++index;
    }
    writer.end_08();
}

// --- Load slots ----------------------------------------------------------------------------------

void load_recon_00972450(ReconSubscription& s, EventArchiveReader& r, EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_int_set_009722d0(r, "oldLevel", s.old_level);
    load_int_set_009722d0(r, "newLevel", s.new_level);
    // has_key_14("party") first: older archives may lack the key.
    if (r.has_key_14(name_key("party"))) load_int_set_009722d0(r, "party", s.party);
}

void load_kill_00972520(KillSubscription& s, EventArchiveReader& r, EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_entity_set_009721c0(r, h, "lastAttacker", s.last_attacker);
    load_int_set_009722d0(r, "lastAttackerPlayerIndex", s.last_attacker_player_index);
}

void load_hit_009725b0(HitSubscription& s, EventArchiveReader& r, EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "target", s.target);
    load_entity_set_009721c0(r, h, "targetDevice", s.target_device);
    load_entity_set_009721c0(r, h, "attacker", s.attacker);
    load_string_set_00970ff0(r, "attackType", s.attack_type);
    load_int_set_009722d0(r, "attackerPlayerIndex", s.attacker_player_index);
    load_range_0096aad0(r, "damageCaused", s.damage_caused);
    load_range_0096aad0(r, "fireCaused", s.fire_caused);
    load_range_0096aad0(r, "leakCaused", s.leak_caused);
}

void load_exit_zone_009726d0(ExitZoneSubscription& s, EventArchiveReader& r,
                             EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_bool_set_00972390(r, "exited", s.exited);
}

void load_input_00972750(InputSubscription& s, EventArchiveReader& r)
{
    load_callback_0096aa90(s, r);
    load_int_set_009722d0(r, "inputID", s.input_id);
}

void load_surrender_009727b0(SurrenderSubscription& s, EventArchiveReader& r)
{
    load_callback_0096aa90(s, r);
    load_int_set_009722d0(r, "party", s.party);
}

void load_failure_00972810(FailureSubscription& s, EventArchiveReader& r, EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_string_set_00970ff0(r, "failureType", s.failure_type);
    load_range_0096aad0(r, "duration", s.duration);
}

void load_leak_009728a0(LeakSubscription& s, EventArchiveReader& r, EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_range_0096aad0(r, "duration", s.duration);
}

void load_fire_00972920(FireSubscription& s, EventArchiveReader& r, EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_range_0096aad0(r, "duration", s.duration);
}

void load_repair_009729a0(RepairSubscription& s, EventArchiveReader& r, EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_int_set_009722d0(r, "repairSetting", s.repair_setting);
}

void load_zone_00972a20(ZoneSubscription& s, EventArchiveReader& r, EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_string_set_00970ff0(r, "zoneName", s.zone_name);
    load_bool_set_00972390(r, "enter", s.enter);
}

void load_command_00972ab0(CommandEventSubscription& s, EventArchiveReader& r,
                           EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_entity_set_009721c0(r, h, "target", s.target);
    load_string_set_00970ff0(r, "command", s.command);
    load_string_set_00970ff0(r, "status", s.status);
}

void load_target_00972b60(TargetSubscription& s, EventArchiveReader& r, EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_entity_set_009721c0(r, h, "target", s.target);
}

void load_ammo_type_00972be0(AmmoTypeSubscription& s, EventArchiveReader& r,
                             EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_int_set_009722d0(r, "ammoType", s.ammo_type);
}

void load_stock_00972c60(StockSubscription& s, EventArchiveReader& r, EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_int_set_009722d0(r, "vehicleClass", s.vehicle_class);
    load_int_set_009722d0(r, "stock", s.stock);
}

void load_gui_00972cf0(GuiSubscription& s, EventArchiveReader& r)
{
    load_callback_0096aa90(s, r);
    load_string_set_00970ff0(r, "guiName", s.gui_name);
    load_bool_set_00972390(r, "enter", s.enter);
}

void load_generate_00972d70(GenerateSubscription& s, EventArchiveReader& r)
{
    load_callback_0096aa90(s, r);
    load_string_set_00970ff0(r, "entityName", s.entity_name);
    load_int_set_009722d0(r, "entityType", s.entity_type);
}

void load_player_00972df0(PlayerSubscription& s, EventArchiveReader& r)
{
    load_callback_0096aa90(s, r);
    load_int_set_009722d0(r, "playerIndex", s.player_index);
    load_string_set_00970ff0(r, "event", s.event);
}

void load_music_over_00971130(MusicOverSubscription& s, EventArchiveReader& r)
{
    load_callback_0096aa90(s, r);
    load_string_set_00970ff0(r, "music", s.music);
}

void load_chat_00972e70(ChatSubscription& s, EventArchiveReader& r)
{
    load_callback_0096aa90(s, r);
    load_int_set_009722d0(r, "playerIndex", s.player_index);
    load_string_set_00970ff0(r, "message", s.message);
    load_int_set_009722d0(r, "messageType", s.message_type);
}

void load_entity_killed_00972f00(EntityKilledSubscription& s, EventArchiveReader& r,
                                 EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_string_set_00970ff0(r, "killReason", s.kill_reason);
}

void load_ship_landed_00972f80(ShipLandedSubscription& s, EventArchiveReader& r,
                               EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
}

void load_hp_event_00972fe0(HpEventSubscription& s, EventArchiveReader& r, EventSubscriptionHost& h)
{
    load_callback_0096aa90(s, r);
    load_entity_set_009721c0(r, h, "entity", s.entity);
    load_string_set_00970ff0(r, "reason", s.reason);
    r.read_10(name_key("hp"), field(GuiLuaFieldType::Float, &s.hp));  // 00D1B20C, +2Ch
}

// --- Save slots ----------------------------------------------------------------------------------

void save_recon_0096e790(const ReconSubscription& s, EventArchiveWriter& w)
{
    save_callback_0096d2b0(s, w);
    save_entity_set_0096e3a0(w, "entity", s.entity);
    save_int_set_0096e460(w, "oldLevel", s.old_level);
    save_int_set_0096e460(w, "newLevel", s.new_level);
    save_int_set_0096e460(w, "party", s.party);  // unconditional, unlike the load
}

void save_kill_0096e850(const KillSubscription& s, EventArchiveWriter& w)
{
    save_callback_0096d2b0(s, w);
    save_entity_set_0096e3a0(w, "entity", s.entity);
    save_entity_set_0096e3a0(w, "lastAttacker", s.last_attacker);
    save_int_set_0096e460(w, "lastAttackerPlayerIndex", s.last_attacker_player_index);
}

void save_exit_zone_0096e9f0(const ExitZoneSubscription& s, EventArchiveWriter& w)
{
    save_callback_0096d2b0(s, w);
    save_entity_set_0096e3a0(w, "entity", s.entity);
    save_bool_set_0096e520(w, "exited", s.exited);
}

void save_input_0096ea70(const InputSubscription& s, EventArchiveWriter& w)
{
    save_callback_0096d2b0(s, w);
    save_int_set_0096e460(w, "inputID", s.input_id);
}

void save_surrender_0096ead0(const SurrenderSubscription& s, EventArchiveWriter& w)
{
    save_callback_0096d2b0(s, w);
    save_int_set_0096e460(w, "party", s.party);
}

// --- Matches slots -------------------------------------------------------------------------------

bool player_matches_00968e80(const PlayerSubscription& s, const EventParams& p,
                             EventSubscriptionHost& h)
{
    if (!s.player_index.matches(param_at(p, 0, h))) return false;  // +0Ch slot 0
    return s.event.matches(param_at(p, 1, h));                     // +1Ch slot 0
}

bool chat_matches_00968f40(const ChatSubscription& s, const EventParams& p,
                           EventSubscriptionHost& h)
{
    if (!s.player_index.matches(param_at(p, 0, h))) return false;
    if (!s.message.matches(param_at(p, 1, h))) return false;
    return s.message_type.matches(param_at(p, 2, h));              // +2Ch slot 0
}

bool entity_killed_matches_00968ff0(const EntityKilledSubscription& s, const EventParams& p,
                                    EventSubscriptionHost& h)
{
    if (!s.entity.matches(param_at(p, 0, h))) return false;
    return s.kill_reason.matches(param_at(p, 1, h));
}

bool hp_event_matches_009690b0(const HpEventSubscription& s, const EventParams& p,
                               EventSubscriptionHost& h)
{
    // +2Ch (hp) is not tested here.
    if (!s.entity.matches(param_at(p, 0, h))) return false;
    return s.reason.matches(param_at(p, 1, h));
}

bool kill_matches_0096ace0(const KillSubscription& s, const EventParams& p,
                           EventSubscriptionHost& h)
{
    // All three filters run first (BL, [ESP+10h], AL), then 0096AD6E..0096AD7B combine them.
    const bool entity = s.entity.matches(param_at(p, 0, h));
    const bool attacker = s.last_attacker.matches(param_at(p, 1, h));
    const bool player = s.last_attacker_player_index.matches(param_at(p, 2, h));
    // A match passes "LuaListener Kill matched, '%s' callback function called" (00D1AF4C) to
    // 004254B0, whose body is empty in this build.
    return entity && attacker && player;
}



// --- Warning objects ------------------------------------------------------------------------------

namespace {

// The three handles of the base, in the order the save and load visit them.
struct SlotRef {
    void* WarningObject::*member;
    WarningSlot slot;
    const char* key;
};
constexpr SlotRef kWarningSlots[3] = {
    {&WarningObject::shortcut, WarningSlot::Shortcut24, "shortcut"},  // 00CF0DF8, entity +38h
    {&WarningObject::ambient, WarningSlot::Ambient3C, "ambient"},      // 00D1AFA4, entity +50h
    {&WarningObject::reporter, WarningSlot::Reporter54, "reporter"},   // 00D1AF98, entity +68h
};

// The handle assignment 009768F0 and 009795B0 inline: nothing when unchanged; unlink the old
// entity, store, link the new one.
void assign_slot(WarningObject& w, WarningHost& host, const SlotRef& ref, void* entity)
{
    void*& current = w.*(ref.member);
    if (current == entity) return;
    if (current != nullptr) host.observer_unlink_006952a0(current, ref.slot);
    current = entity;
    if (entity != nullptr) host.observer_link_00694a60(entity, ref.slot);
}

// The +6Ch observer the entity warnings build (vtable 00CE3CD4 then the class's own, +70h..+78h
// zero, +7Ch byte 0); only its vtable differs between the classes.
void construct_entity_observer(WarningObject& w, std::uint32_t observer_vtable)
{
    w.word_6c = observer_vtable;
}

} // namespace

void construct_warning_base_00973a90(WarningObject& w, WarningHost& host)
{
    w = WarningObject{};
    w.vtable = 0x00D1B104;
    // Each handle: vtable 00CE75CC, +4h..+0Ch zero, +10h byte 1, entity +14h zero.
    w.state = 0;                                      // +0Ch
    w.validity_ref_time = host.clock_00f876a4();      // +10h = [00F876A4]
}

void construct_entity_warning_default_00973b20(WarningObject& w, WarningHost& host)
{
    construct_warning_base_00973a90(w, host);
    construct_entity_observer(w, 0x00D1B218);
    w.vtable = 0x00D1B230;  // +80h is left as the allocation had it
}

void construct_entity_warning_base_00973ba0(WarningObject& w, WarningHost& host, void* entity)
{
    construct_warning_base_00973a90(w, host);
    construct_entity_observer(w, 0x00D1B218);
    w.vtable = 0x00D1B230;
    w.entity = entity;                                 // 00973BF2
    host.observer_link_00694a60(entity, WarningSlot::Entity6C);  // 00973BF8, no null test
}

void construct_entity_warning_for_archive_00973c20(WarningObject& w, WarningHost& host)
{
    construct_warning_base_00973a90(w, host);
    construct_entity_observer(w, 0x00D1B254);
    w.vtable = 0x00D1B26C;
    w.type = 1;
}

void construct_recon_warning_for_archive_00973ca0(WarningObject& w, WarningHost& host)
{
    construct_warning_base_00973a90(w, host);
    construct_entity_observer(w, 0x00D1B290);
    w.vtable = 0x00D1B2A8;
    w.type = 2;
}

void construct_kill_warning_default_00973d30(WarningObject& w, WarningHost& host)
{
    construct_warning_base_00973a90(w, host);
    w.vtable = 0x00D1B2CC;
    w.type = 3;
}

void construct_hit_warning_for_archive_00973d90(WarningObject& w, WarningHost& host)
{
    construct_warning_base_00973a90(w, host);
    construct_entity_observer(w, 0x00D1B2F0);
    w.vtable = 0x00D1B308;
    w.type = 4;
}

void construct_exit_zone_warning_for_archive_00973e10(WarningObject& w, WarningHost& host)
{
    construct_warning_base_00973a90(w, host);
    construct_entity_observer(w, 0x00D1B32C);
    w.vtable = 0x00D1B344;
    w.type = 5;
}

void construct_confirm_warning_for_archive_00973e90(WarningObject& w, WarningHost& host)
{
    construct_warning_base_00973a90(w, host);
    construct_entity_observer(w, 0x00D1B368);
    w.vtable = 0x00D1B380;
    w.type = 6;
}

void construct_entity_warning_with_id_00974150(WarningObject& w, WarningHost& host, void* entity,
                                               const std::string& id)
{
    construct_entity_warning_base_00973ba0(w, host, entity);
    w.vtable = 0x00D1B26C;
    w.word_6c = 0x00D1B254;
    w.type = 1;
    w.id = id;  // 0041DD40 resize then memcpy, skipped when the source is the field itself
}

void construct_recon_warning_009741e0(WarningObject& w, WarningHost& host, void* entity,
                                      std::int32_t old_level, std::int32_t new_level)
{
    construct_entity_warning_base_00973ba0(w, host, entity);
    w.vtable = 0x00D1B2A8;
    w.word_6c = 0x00D1B290;
    w.type = 2;
    w.old_level = old_level;                           // +84h
    w.new_level = new_level;                           // +88h
    host.entity_world_position(entity, w.position);   // 00974249..00974267 FLD/FSTP copies
    const std::int32_t side = host.local_side_28();   // 0097427F..00974281
    const std::int32_t relation = host.relation_00803ce0(side, entity);  // 0097428B
    void* detector = host.detector_for_side_210(entity, side);          // 00974293
    bool player = false;
    if (detector != nullptr) {
        // A plane compares its squadron (+9D4h) with the local player unit [00E188D8].
        void* unit = host.is_kind_of_5c(detector, 0x0F) ? host.plane_squadron_9d4(detector)
                                                        : detector;
        player = unit == host.local_player_unit_00e188d8();
    }
    w.player = player;                                 // 009742D6
    w.id = "recon";                                    // 0041DD40(5, 0) + memcpy
    // Only a change from unidentified (<= 1) to identified (>= 2) gets the long id.
    if (old_level > 1) return;
    if (new_level < 2) return;
    static const char* const kRelationNames[4] = {"OWN", "ENEMY", "NEUTRAL", "UNKNOWN"};  // 00E0B080
    w.id += "identified";
    w.id += kRelationNames[relation];
    if (detector != nullptr && host.is_kind_of_5c(detector, 6)) {
        w.id += "ship";
    } else if (detector != nullptr &&
               (host.is_kind_of_5c(detector, 0x18) || host.is_kind_of_5c(detector, 0x0F))) {
        w.id += "plane";
    } else {
        w.id += "other";
    }
}

void construct_warning_type9_00975750(WarningObject& w, WarningHost& host, std::uint32_t word)
{
    construct_warning_base_00973a90(w, host);
    w.word_6c = word;                                  // +6Ch = argument
    w.vtable = 0x00D1B484;
}

void construct_message_warning_00975ac0(WarningObject& w, WarningHost& host, const std::string& id)
{
    construct_warning_base_00973a90(w, host);
    w.vtable = 0x00D1B4D4;
    w.id = id;
}

void construct_string_list_warning_for_archive_00976230(WarningObject& w, WarningHost& host)
{
    construct_warning_base_00973a90(w, host);
    w.vtable = 0x00D1B4F8;
    w.type = 0;
    w.strings.clear();                                 // +70h..+78h zero
}

void construct_string_list_warning_009795b0(WarningObject& w, WarningHost& host,
                                            const std::vector<std::string>& strings,
                                            void* shortcut, void* ambient)
{
    construct_string_list_warning_for_archive_00976230(w, host);
    w.strings = strings;                               // BSP_StringVector_Assign
    assign_slot(w, host, kWarningSlots[0], shortcut);  // +38h
    assign_slot(w, host, kWarningSlots[1], ambient);   // +50h
}

void destroy_warning_0096f170(WarningObject& w, WarningHost& host)
{
    w.vtable = 0x00D1B104;
    // EH states 3, 2, 1: the handles at +54h, +3Ch, +24h, then the message vector, then the id
    // (returned to the sized storage pool).
    host.observer_destroy(WarningSlot::Reporter54);
    host.observer_destroy(WarningSlot::Ambient3C);
    host.observer_destroy(WarningSlot::Shortcut24);
    w.messages.clear();
    w.messages.shrink_to_fit();
    w.id.clear();
}

void destroy_entity_warning_0096f270(WarningObject& w, WarningHost& host)
{
    host.observer_destroy(WarningSlot::Entity6C);      // BSP_CallbackOwner_Destroy on +6Ch
    destroy_warning_0096f170(w, host);
}

void destroy_string_list_warning_009762a0(WarningObject& w, WarningHost& host)
{
    w.strings.clear();                                 // 00432050 then _free, +70h..+78h zero
    w.strings.shrink_to_fit();
    destroy_warning_0096f170(w, host);
}

bool warning_is_duplicate_0096aa40(const WarningObject& self, const WarningObject& other,
                                   WarningHost& host)
{
    // vtable +10h on the other first, then on this; then 00435C40.
    if (other.type != self.type) return false;
    return host.equals_insensitive_00435c40(self.id, other.id);
}

bool hit_warning_is_duplicate_0096adb0(const WarningObject& self, const WarningObject& other,
                                       WarningHost& host)
{
    if (other.type != self.type) return false;             // 0096ADD0
    // 0096ADD8..0096ADFE: the type test again, then the id compare (ECX = other.id).
    if (other.type == self.type && host.equals_insensitive_00435c40(other.id, self.id)) return true;
    // Two area-excluded hits on live entities closer than 5000 (00D1AF84) are duplicates.
    if (!self.area_exclusion || !other.area_exclusion) return false;
    if (self.entity == nullptr || other.entity == nullptr) return false;
    const float distance = host.entity_distance_0042b2f0(self.entity, other.entity);
    return 5000.0f > distance;                             // FCOMIP; JBE false, NaN false
}

void warning_add_message_00976b00(WarningObject& w, WarningHost& host, const std::string& name)
{
    // 007018A0 on [[game+18CCh+4*[game+18ECh]]+28h], then 00705E00(name, 0) on [game+21DCh].
    VoiceClipRef clip{};
    clip.speaker = band_0067_cc13::warning_remap_kind_007018a0(host.local_side_28());
    clip.record = host.resolve_message_00705e00(name);
    w.messages.push_back(clip);                            // 005BB020 on +14h
}

void save_warning_0096cf10(const WarningObject& w, EventArchiveWriter& writer, WarningHost& host)
{
    writer.write_0c(name_key("type"), int_value(w.type));             // 0096CF3A vtable +10h
    writer.write_0c(name_key("id"), string_value(w.id.c_str()));      // data or 00F8A0C8
    writer.write_0c(name_key("state"), int_value(w.state));
    writer.write_0c(name_key("validityRefTime"), float_value(w.validity_ref_time));
    writer.begin_04(name_key("message"));
    for (std::size_t i = 0; i < w.messages.size(); ++i) {
        writer.begin_04(index_key(static_cast<std::int32_t>(i)));    // 0096D062
        host.write_message_ref_00703380(w.messages[i], writer);      // element slot 0
        writer.end_08();
    }
    writer.end_08();
    // Each handle is written as field type 4 through a temporary observer handle (vtable
    // 00CE74FC/00CE75CC) that links (00694A60) and unlinks (006952A0) around the write; the
    // archive sees only the entity pointer.
    for (const SlotRef& ref : kWarningSlots) {
        GuiLuaVariant value{};
        value.tag = static_cast<std::int32_t>(GuiLuaFieldType::Handle);
        value.value.pointer = w.*(ref.member);
        writer.write_0c(name_key(ref.key), value);
    }
}

void save_entity_warning_0096d2f0(const WarningObject& w, EventArchiveWriter& writer,
                                  WarningHost& host)
{
    save_warning_0096cf10(w, writer, host);
    GuiLuaVariant value{};
    value.tag = static_cast<std::int32_t>(GuiLuaFieldType::Handle);
    value.value.pointer = w.entity;                                  // +80h
    writer.write_0c(name_key("entity"), value);                      // 00CE5818
}

void save_recon_warning_0096d3e0(const WarningObject& w, EventArchiveWriter& writer,
                                 WarningHost& host)
{
    save_entity_warning_0096d2f0(w, writer, host);
    writer.write_0c(name_key("oldLevel"), int_value(w.old_level));
    writer.write_0c(name_key("newLevel"), int_value(w.new_level));
    writer.write_0c(name_key("position"),
                    field(GuiLuaFieldType::Vec3, const_cast<float*>(w.position)));  // +8Ch
    writer.write_0c(name_key("player"), bool_value(w.player));
}

void save_hit_warning_0096d4d0(const WarningObject& w, EventArchiveWriter& writer,
                               WarningHost& host)
{
    save_entity_warning_0096d2f0(w, writer, host);
    writer.write_0c(name_key("areaexclusion"), bool_value(w.area_exclusion));  // +84h
}

void load_warning_009768f0(WarningObject& w, EventArchiveReader& reader, WarningHost& host)
{
    reader.read_10(name_key("id"), field(GuiLuaFieldType::String, &w.id));
    // One local ([ESP+20h] after each call) takes "state" and all three handles in turn and is
    // never cleared: a key the reader leaves alone inherits the previous key's value.
    std::int32_t scratch = 0;
    reader.read_10(name_key("state"), field(GuiLuaFieldType::Int, &scratch));
    w.state = scratch;                                                // 00976951
    reader.read_10(name_key("validityRefTime"),
                   field(GuiLuaFieldType::Float, &w.validity_ref_time));
    reader.enter_04(name_key("message"));
    for (std::int32_t i = 0; reader.has_key_14(index_key(i)); ++i) {
        reader.enter_04(index_key(i));
        VoiceClipRef clip{};
        host.read_message_ref_00706060(reader, clip);                 // 009769D8
        w.messages.push_back(clip);                                   // 005BB020
        reader.leave_08();
    }
    reader.leave_08();
    for (const SlotRef& ref : kWarningSlots) {
        reader.read_10(name_key(ref.key), field(GuiLuaFieldType::Handle, &scratch));
        assign_slot(w, host, ref, reinterpret_cast<void*>(static_cast<std::intptr_t>(scratch)));
    }
}

void load_entity_warning_00976c60(WarningObject& w, EventArchiveReader& reader, WarningHost& host)
{
    load_warning_009768f0(w, reader, host);
    // The destination is pre-loaded with a stack address (00976C74) that survives when the key
    // is absent; here it starts null. The link is unconditional.
    std::int32_t handle = 0;
    reader.read_10(name_key("entity"), field(GuiLuaFieldType::Handle, &handle));
    w.entity = reinterpret_cast<void*>(static_cast<std::intptr_t>(handle));  // 00976C9E
    host.observer_link_00694a60(w.entity, WarningSlot::Entity6C);           // 00976CA4
}

void load_recon_warning_00976e40(WarningObject& w, EventArchiveReader& reader, WarningHost& host)
{
    load_entity_warning_00976c60(w, reader, host);
    std::int32_t scratch = 0;  // shared by both levels, as at 00976E73 / 00976EA1
    reader.read_10(name_key("oldLevel"), field(GuiLuaFieldType::Int, &scratch));
    w.old_level = scratch;
    reader.read_10(name_key("newLevel"), field(GuiLuaFieldType::Int, &scratch));
    w.new_level = scratch;
    reader.read_10(name_key("position"), field(GuiLuaFieldType::Vec3, w.position));
    reader.read_10(name_key("player"), field(GuiLuaFieldType::Bool, &w.player));
}

void load_hit_warning_00977000(WarningObject& w, EventArchiveReader& reader, WarningHost& host)
{
    load_entity_warning_00976c60(w, reader, host);
    reader.read_10(name_key("areaexclusion"), field(GuiLuaFieldType::Bool, &w.area_exclusion));
}

void warning_manager_withdraw_0096ce70(WarningManagerHost& host, WarningObject* warning)
{
    host.lock_24();
    if (warning->state == 1) {
        host.remove_from_list_0096c280(0xE0, warning);  // the queued list
    } else if (warning->state == 2) {
        host.remove_from_list_0096c280(0xEC, warning);  // the applied list
    }
    host.delete_warning(warning);                        // 0096CEE9 slot 0 (1)
    host.unlock_24();
}

void entity_warning_on_released_00973b60(WarningManagerHost& host, WarningObject& warning,
                                         void* released)
{
    if (released != warning.entity) return;              // 00973B64 CMP [ECX+14h] (= +80h)
    warning_manager_withdraw_0096ce70(host, &warning);
}

void warning_manager_clear_waiting_0096bf70(WarningManagerHost& host)
{
    host.lock_24();
    for (void* payload : host.waiting_payloads_e4()) {
        if (payload != nullptr) host.delete_waiting_payload(payload);  // slot 0 (1), node+8 = 0
    }
    host.clear_waiting_list_e4();                        // head relinked, +E8h = 0, nodes freed
    if (void* gui = host.ingame_gui_00e198c4()) {
        host.voice_stop_005b83e0(gui);                   // the host passes ECX = [gui+A4h]
    }
    host.unlock_24();
}

void ship_noise_stop_effect_0096ae90(ShipNoiseEffectHost& host, void*& effect_08)
{
    void* effect = effect_08;
    if (effect == nullptr) return;
    host.effect_stop_children_00867b10(effect);
    host.effect_set_byte_9(effect);
    host.release(effect_08);                             // 0096AEBC InterlockedDecrement
    effect_08 = nullptr;
}

void ship_noise_start_effect_0096c070(ShipNoiseEffectHost& host, void*& effect_08, void* ship)
{
    if (effect_08 != nullptr) return;
    // Identity 4x4 (1.0f at 00D7A24C on the diagonal), transform word 1, options 0, 0.
    static constexpr float kIdentity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    void* parent = host.ship_node_4a4(ship);
    void* definition = host.manager_effect_definition_194();
    if (definition != nullptr) host.add_ref(definition);  // the reference 008687C0 consumes
    void* created = host.create_point_effect_008687c0(parent, definition, kIdentity);
    if (effect_08 != created) {
        void* old = effect_08;
        effect_08 = created;
        if (created != nullptr) host.add_ref(created);
        if (old != nullptr) host.release(old);
    }
    if (created != nullptr) host.release(created);       // the temporary's reference
}

WarningEventRecord construct_warning_event_record_0096dc00(std::string name)
{
    WarningEventRecord record;
    record.name = std::move(name);  // copy then the by-value argument's block is returned
    return record;
}


// --- reports -------------------------------------------------------------------------------------

bool warning_report_gate(WarningReportHost& host)
{
    if (host.suppressed_d0()) return false;              // CMP byte [manager+D0h], 0
    const std::int32_t slot = host.local_player_slot_18ec();
    if (slot < 0 || slot > 7) return false;              // JL / JG 7
    return host.clock_00f876a4() > 4.0f;                 // COMISS [00CE3D34]; JBE out
}

void report_confirm_00977ba0(WarningReportHost& host, void* entity, std::uint32_t arg)
{
    host.lock_24();
    if (warning_report_gate(host) && host.entity_byte(entity, 0x5C) &&
        !host.entity_byte(entity, 0x5D) && !host.entity_byte(entity, 0x60) &&
        !host.entity_byte(entity, 0x5E) && !host.entity_disabled_0077edf0(entity) &&
        host.entity_side_54(entity) == host.local_side_28()) {
        host.report_009763e0(host.new_confirm_warning_00975540(entity, arg));  // 84h block
    }
    host.unlock_24();
}

void report_new_unit_00977cd0(WarningReportHost& host, std::int32_t slot)
{
    host.lock_24();
    if (warning_report_gate(host) && slot == host.local_player_slot_18ec()) {
        host.report_009763e0(host.new_new_unit_warning_009756a0());              // 6Ch block
    }
    host.unlock_24();
}

void report_powerup_009789a0(WarningReportHost& host, std::int32_t slot, const std::string* name)
{
    host.lock_24();
    if (warning_report_gate(host) && slot == host.local_player_slot_18ec()) {
        host.report_009763e0(host.new_powerup_warning_009758e0(name));           // 6Ch block
    }
    host.unlock_24();
}

void report_string_list_00979670(WarningReportHost& host, const std::vector<std::string>& strings,
                                 void* shortcut, void* ambient)
{
    if (!warning_report_gate(host)) return;
    host.set_byte_104();
    if (strings.empty()) return;                          // (end - begin) >> 3 == 0
    host.report_009763e0(host.new_string_list_warning_009795b0(strings, shortcut, ambient));
}

void session_hit_feedback_0096f090(WarningHitFeedbackState& state, float clock,
                                   const HitFeedbackMessage& message, WarningTextHost& text)
{
    // 0096F094/0096F098 MOVZX the two bytes over the stack arguments, then JMP 0096EF60.
    report_hit_feedback_0096ef60(state, clock, message.damaging_18 != 0, message.friendly_19 != 0,
                                 text);
}

// --- the variant table ---------------------------------------------------------------------------

namespace {

std::int32_t use_count_of(const WarningVariantTable& table, const std::string& name)
{
    // 004BF370 lower_bound, then the __stricmp test (00BF7FBF); end -> 0, else the node's int.
    const auto it = table.use_count_144.find(name);
    return it == table.use_count_144.end() ? 0 : it->second;
}

} // namespace

std::size_t pick_message_variant_00979b50(WarningVariantTable& table,
                                          const std::vector<std::string>& variants,
                                          WarningVariantHost& host)
{
    std::size_t picked = 0;
    if (variants.size() == 1) {
        picked = 0;                                       // the only element, no draw
    } else {
        std::int32_t best_count = 0x7FFFFFFF;
        std::int32_t ties = 0;
        float best_time = -1.0e10f;                       // 00CE4ADC
        for (std::size_t i = 0; i < variants.size(); ++i) {
            const std::int32_t count = use_count_of(table, variants[i]);
            if (count < best_count) {                     // 00979C70 JLE
                ties = 1;
                best_count = count;
                // A new lowest count with uses takes its time unconditionally; with no uses the
                // previous best_time is kept.
                if (count > 0) best_time = table.last_use_150[variants[i]];   // 00444BE0 inserts
            } else if (count == best_count) {
                ++ties;
                if (count > 0) {
                    const float time = table.last_use_150[variants[i]];
                    if (time > best_time) best_time = time;                  // FCOMIP; JBE skip
                }
            }
        }
        // With several tied variants that have been used, one fewer slot is drawn, so the
        // last of them in vector order is never picked.
        const std::int32_t adjust = (ties < 2 || best_count < 1) ? 0 : -1;
        const std::uint32_t divisor = static_cast<std::uint32_t>(ties + adjust);
        if (divisor == 0) host.divide_error();
        std::uint32_t draw = host.random_word_00bd2fc0_stream1() % divisor;  // 00979D11, DIV
        std::size_t chosen = static_cast<std::size_t>(-1);  // [ESP+28h] = FFFFFFFFh
        for (std::size_t i = 0; i < variants.size(); ++i) {
            const std::int32_t count = use_count_of(table, variants[i]);
            if (count != best_count) continue;
            if (ties != 1 && count != 0) {
                const float time = table.last_use_150[variants[i]];
                if (best_time < time) continue;           // FCOMIP; JC skip (NaN qualifies)
            }
            if (static_cast<std::int32_t>(draw) < 1) {
                chosen = i;
                break;
            }
            --draw;
        }
        if (chosen >= variants.size()) host.invalid_parameter_00bf6713();
        picked = chosen;
    }
    // 005070C0 operator[] increments the use count; 00444BE0 operator[] stores the clock.
    ++table.use_count_144[variants[picked]];
    table.last_use_150[variants[picked]] = host.clock_00f876a4();
    return picked;
}

// --- the manager archive -------------------------------------------------------------------------

void save_warning_manager_0096dd30(const WarningManagerArchiveView& view, EventArchiveWriter& writer,
                                   WarningManagerSaveHost& host)
{
    writer.write_0c(name_key("disableAll"), bool_value(view.disable_all));
    writer.begin_04(name_key("disableEntity"));
    std::int32_t index = 0;
    for (const auto& [entity, disabled] : view.disable_entity) {
        if (!disabled) continue;                          // node +10h byte
        GuiLuaVariant value{};
        value.tag = static_cast<std::int32_t>(GuiLuaFieldType::Handle);
        value.value.pointer = entity;                     // node +0Ch
        writer.write_0c(index_key(index), value);
        ++index;
    }
    writer.end_08();
    writer.begin_04(name_key("waitingQueue"));
    index = 0;
    for (void* payload : view.waiting_queue) {
        writer.begin_04(index_key(index));
        host.save_payload_slot2(payload, writer);
        writer.end_08();
        ++index;
    }
    writer.end_08();
    writer.begin_04(name_key("disableList"));
    index = 0;
    for (void* payload : view.disable_list) {
        writer.begin_04(index_key(index));
        host.save_payload_slot2(payload, writer);
        writer.end_08();
        ++index;
    }
    writer.end_08();
    writer.begin_04(name_key("luaListeners"));
    for (const WarningListenerChannel& channel : view.listeners) {
        writer.begin_04(name_key(channel.name.c_str()));  // data, or 00F8A0C8 when null
        for (const auto& [callback, subscription] : channel.subscriptions) {
            writer.begin_04(name_key(callback.c_str()));
            host.save_subscription_slot2(subscription, writer);
            writer.end_08();
        }
        writer.end_08();
    }
    writer.end_08();
    writer.begin_04(name_key("useCount"));
    if (view.variants != nullptr) {
        for (const auto& [name, count] : view.variants->use_count_144) {
            writer.write_0c(name_key(name.c_str()), int_value(count));
        }
    }
    writer.end_08();
    writer.begin_04(name_key("lastUse"));
    if (view.variants != nullptr) {
        for (const auto& [name, time] : view.variants->last_use_150) {
            writer.write_0c(name_key(name.c_str()), float_value(time));
        }
    }
    writer.end_08();
    writer.begin_04(name_key("shipNoiseInfo"));
    for (const ShipNoiseArchiveRow& row : view.ship_noise) {
        writer.begin_04(index_key(row.ship_id_174));      // {1, word [ship+174h]}
        writer.write_0c(name_key("collisionSoundDisableTime"),
                        float_value(row.collision_sound_disable_time));
        if (row.effect != nullptr) {
            writer.write_0c(name_key("airRaidSoundExpires"), float_value(row.air_raid_sound_expires));
        }
        writer.end_08();
    }
    writer.end_08();
}


// --- segment 78: GUI leaves -----------------------------------------------------------------------

GuiTypeDescriptor<3>& gui_depth3_type_init(GuiTypeDescriptor<3>& target, std::uint8_t& guard,
                                           const GuiDepth3TypeInit& init,
                                           GuiTypeDescriptor<2>& element,
                                           void (*initialize_element_00aa8200)(GuiTypeDescriptor<2>&),
                                           GuiTypeCounterHost& host)
{
    // CMP byte [guard], 0; MOV byte [guard], 1; [ECX+0Ch] = name; 00AA8200(00F8BC88); the two
    // Element ids into [ECX+4h]/[ECX+8h]; 006FAC20 counter +4h post-increment into [ECX].
    gui_widgets_cc13::gui_type_initialize<3>(
        target, guard, init.name, element.ids,
        [&]() { initialize_element_00aa8200(element); }, host);
    return target;
}

void gui_singleton_destruct(void*& global, std::uint32_t& vtable)
{
    global = nullptr;          // MOV [00F8BD98 / 00F8BF44], 0
    vtable = 0x00CE3818;       // BSP_SingletonBase_ScalarDelete's vtable
}

void gui_icon_state_release_00ab3280(GuiIconState& state, GuiIconStateHost& host)
{
    if (state.texture != nullptr) {
        host.release_texture(state.texture);   // InterlockedDecrement, slot 0 at zero
        state.texture = nullptr;
    }
    state.texture_name.clear();                 // the name's block back to the sized pool
}

void gui_icon_state_copy_00ab3a70(GuiIconState& out, const GuiIconState& in, GuiIconStateHost& host)
{
    out = in;                                   // +0h, the string (0041DD40 + memcpy), +0Ch..+3Dh
    if (out.texture != nullptr) host.retain_texture(out.texture);  // 00AB3B1D InterlockedIncrement
}

void gui_icon_state_assign_00ab3c10(GuiIconState& out, const GuiIconState& in)
{
    // `*param_1 = *param_2` on the texture word with no Interlocked call either way.
    out.texture = in.texture;
    if (&out != &in) out.texture_name = in.texture_name;
    out.resolved_uv = in.resolved_uv;
    out.authored_uv = in.authored_uv;
    out.pivot_x = in.pivot_x;
    out.pivot_y = in.pivot_y;
    out.size = in.size;
    out.load_pending = in.load_pending;
    out.flag_3d = in.flag_3d;
}

void gui_icon_set_pivot_00ab1f60(GuiWidgetTransform& widget, GuiIconWidget& icon,
                                 const float (&pivot)[2], GuiWidgetTransformHost& transform,
                                 GuiLeafHost& host)
{
    widget.pivot_x = pivot[0];                  // FLD/FSTP [ESI+18h]
    widget.pivot_y = pivot[1];                  // FLD/FSTP [ESI+1Ch]
    recompose_local_transform(widget, transform);  // 00AA7220
    if (static_cast<std::uint16_t>(icon.current_state) == 0xFFFF) return;  // CMP AX, FFFFh
    // MOVSX index, then an unsigned compare against the 40h-element count.
    const auto index = static_cast<std::uint32_t>(static_cast<std::int32_t>(icon.current_state));
    if (index >= icon.states.size()) host.invalid_parameter_00bf6713();
    GuiIconState& state = icon.states[index];
    state.pivot_x = pivot[0];                   // +2Ch
    state.pivot_y = pivot[1];                   // +30h
    host.widget_virtual_8c(&widget);
}

void gui_icon_set_shader_name_00ab4630(void* widget, GuiIconWidget& icon, const std::string& name,
                                       GuiLeafHost& host)
{
    if (&icon.shader_name != &name) icon.shader_name = name;  // 0041DD40 + memcpy at +104h/+108h
    host.clear_draw_section_owners_00b73c10(widget);
    host.widget_virtual_8c(widget);
}

GuiNamedValue gui_named_value_copy_00ab4690(const GuiNamedValue& in)
{
    GuiNamedValue out;
    out.name = in.name;
    out.value = in.value;
    return out;
}

void gui_named_value_assign_00ab46e0(GuiNamedValue& out, const GuiNamedValue& in)
{
    if (&out != &in) out.name = in.name;
    out.value = in.value;
}

void gui_named_value_array_reserve_00ab4720(GuiNamedValueArray& a, std::int32_t wanted)
{
    if (wanted < 1) wanted = 1;
    if (a.capacity >= wanted) return;
    // operator_new(wanted * 0Ch), copy-construct the live elements, release the old names,
    // _free the old block.
    std::vector<GuiNamedValue> block(static_cast<std::size_t>(wanted));
    for (std::int32_t i = 0; i < a.count; ++i) {
        block[static_cast<std::size_t>(i)] = gui_named_value_copy_00ab4690(a.slots[static_cast<std::size_t>(i)]);
    }
    a.slots = std::move(block);
    a.capacity = wanted;
}

void gui_named_value_array_resize_00ab4850(GuiNamedValueArray& a, std::int32_t size)
{
    if (a.capacity < size) gui_named_value_array_reserve_00ab4720(a, size);
    for (std::int32_t i = a.count; i < size; ++i) {
        a.slots[static_cast<std::size_t>(i)] = GuiNamedValue{};  // three zero words
    }
    while (size < a.count) {
        --a.count;
        a.slots[static_cast<std::size_t>(a.count)].name.clear();  // name block released
    }
    a.count = size;
}

void gui_named_value_array_push_back_00ab4990(GuiNamedValueArray& a, const GuiNamedValue& v)
{
    if (a.count == a.capacity) {
        std::int32_t grown = a.capacity * 2;
        if (grown < 2) grown = 1;               // 0 -> 1, 1 -> 2
        gui_named_value_array_reserve_00ab4720(a, grown);
    }
    a.slots[static_cast<std::size_t>(a.count)] = gui_named_value_copy_00ab4690(v);
    ++a.count;
}

void gui_named_value_array_swap_remove_00ab4a10(GuiNamedValueArray& a, std::int32_t index)
{
    if (index == -1) return;
    const auto last = static_cast<std::size_t>(a.count - 1);
    if (index != a.count - 1) {
        gui_named_value_assign_00ab46e0(a.slots[static_cast<std::size_t>(index)], a.slots[last]);
    }
    a.slots[last].name.clear();
    --a.count;
}

void gui_named_value_array_destroy_00ab53b0(GuiNamedValueArray& a)
{
    gui_named_value_array_resize_00ab4850(a, 0);
    a.slots.clear();                            // _free of the block (capacity is left)
    a.slots.shrink_to_fit();
}

void gui_framebox_border_icon_layout_00ab6cb0(const GuiFrameBoxBorderIconLayout& in,
                                              std::uint8_t& flag, std::uint32_t& a,
                                              std::uint32_t& b, float (&rect)[4])
{
    flag = in.flag_15c;
    a = in.word_160;
    b = in.word_164;
    for (int i = 0; i < 4; ++i) rect[i] = in.rect_168[i];
}

bool text_accepts_input_char_00ab6d30(char c, GuiLeafHost& host)
{
    // 00AB6D34 MOVSX EAX, DL: the six compares against A2h..B2h test a value in -128..127 and
    // never match, so the rejection list is dead and the tail jump always runs.
    const std::int32_t widened = static_cast<std::int32_t>(static_cast<signed char>(c));
    switch (widened) {
    case 0xA2: case 0xA3: case 0xA5: case 0xA7: case 0xAA: case 0xB2:
        return false;                           // unreachable for a sign-extended byte
    default:
        return host.accepts_glyph_byte_00ab6d00(c);
    }
}

void gui_text_set_state_color_00ab7290(GuiTextStateColors& text, std::int32_t index,
                                       const float (&rgba)[4])
{
    for (int i = 0; i < 4; ++i) text.colors_11c[index][i] = rgba[i];  // no index check
    text.has_mis_colors_118 = true;
}

void gui_text_set_cursor_mode_00ab95f0(void* text, GuiTextCursor& cursor, std::int32_t mode,
                                       GuiLeafHost& host)
{
    cursor.mode_17c = mode;
    host.text_ensure_cursor_model_00ab8910(text);
    // SETNZ CL; CVTSI2SS: 1.0 unless the mode is 2; flag word 0.
    host.node_set_visibility_00b6da70(cursor.node_184, cursor.mode_17c != 2 ? 1.0f : 0.0f, 0);
    if (cursor.mode_17c == 2) return;
    host.text_position_cursor_00ab9240(text, cursor.index_180);
    host.widget_virtual_50(text, static_cast<std::uint8_t*>(text) + 0x50);
}

void* gui_first_vertex_stream(void* widget, GuiLeafHost& host)
{
    return host.first_vertex_stream(widget);
}

// 00ABE640..00ABE66E: ECX = section, plain RET, EAX result. Each null/zero
// branch precedes the next native call. Offset +4Ch is the established widget
// generated-model field. Callee contracts are retained as address boundaries;
// this routine claims no additional geometry implementation or binary ABI.
void* gui_section_get_first_vertex_stream_00abe640(void* section, GuiSectionVertexStreamHost& host)
{
    void* model;
    std::memcpy(&model, static_cast<unsigned char*>(section) + 0x4C, sizeof model);
    if (!model) return nullptr;
    void* const geometry = host.call_00b74640(model, 0); // 00ABE64A
    if (!geometry) return nullptr;
    if (host.call_00b72b20(geometry) == 0) return nullptr; // 00ABE657
    return host.call_00b73260(geometry, 0); // 00ABE664
}

void gui_section_set_size_00abe670(void* section, const float (&size)[2], GuiLeafHost& host)
{
    host.widget_set_size_00aa7970(section, size);
    host.widget_virtual_7c(section);
}

void hud_ticker_set_range_00ac2f20(GuiHudTickerRange& ticker, std::uint32_t from, std::uint32_t to)
{
    ticker.from_0c = from;
    ticker.to_10 = to;
}

GuiFontRegistryList& gui_font_registry_list_construct_00ac3660(GuiFontRegistryList& list)
{
    // 00AC3663: the sentinel comes from 00AC3400 inside the constructor.
    list.head_04 = gui_widgets_cc13::msvc9_containers::list_buy_sentinel_00aac180();
    list.size_08 = 0;
    return list;
}

void gui_write_int_unless_default_00ac4ad0(GuiPropertyWriter& writer, const char* key,
                                           std::int32_t value, std::int32_t fallback)
{
    GuiTaggedValue v{};
    v.tag = static_cast<GuiValueTag>(1);
    v.i = value;
    GuiTaggedValue d{};
    d.tag = static_cast<GuiValueTag>(1);
    d.i = fallback;
    if (writer.equals_00bd5680(v, d)) return;
    writer.write(key, v);
}

void* gui_layer_compose_script_path_00ac5770(const std::string& layer_name_100, void* out,
                                             GuiLeafHost& host)
{
    const std::string copy = layer_name_100;  // 0041DD40 + memcpy into a stack NativeString
    return host.compose_script_path_00ac5600(out, copy);
}

void gui_scroll_area_set_offset_00ac8ee0(void* area, GuiScrollAreaFields& f, float x, float y,
                                         GuiLeafHost& host)
{
    f.offset_110[0] = x;
    f.offset_110[1] = y;
    const float values[2] = {x, y};
    for (int axis = 0; axis < 2; ++axis) {
        if (values[axis] > 0.0f) {              // COMISS against 0; JBE to the clamp
            f.offset_110[axis] = 0.0f;
            continue;
        }
        // FLD content; FSUB size; FCHS; FCOMI with the argument; JBE keeps the argument. The
        // difference of two floats is exact in double, as it is on the x87 stack here.
        const double bound = -(static_cast<double>(f.content_11c[axis]) -
                               static_cast<double>(f.size_20[axis]));
        if (bound > static_cast<double>(values[axis])) {
            f.offset_110[axis] = static_cast<float>(bound);  // FSTP float
        }
    }
    host.widget_set_local_xy_00aa7d00(f.content_118, f.offset_110[0], f.offset_110[1]);
    host.widget_virtual_7c(area);
}

void gui_movie_construct_00aafe20(GuiMovieFields& f)
{
    f = GuiMovieFields{};  // +ECh, +F0h, +F8h..+100h, +104h/+105h, +108h, both vectors zero
}

void gui_movie_copy_construct_00aafff0(GuiMovieFields& f, const GuiMovieFields& source)
{
    f = GuiMovieFields{};
    f.movie_name_fc = source.movie_name_fc;     // the only field copied (+FCh)
}

void gui_movie_destroy_00aafef0(GuiMovieFields& f, void (*delete_player)(void*))
{
    if (f.player_f0 != nullptr) {
        delete_player(f.player_f0);             // vtable +8h with 1
        f.player_f0 = nullptr;
    }
    f.vector_11c.clear();
    f.vector_11c.shrink_to_fit();
    f.vector_10c.clear();
    f.vector_10c.shrink_to_fit();
    f.movie_name_fc.clear();
}

void gui_movie_read_properties_00aad560(void* widget, const GuiWidgetTransform& transform,
                                        GuiMovieFields& f,
                                        gui_widgets_cc13::GuiPropertyReader& reader,
                                        GuiLeafHost& host)
{
    host.read_base_00aaa710(widget, reader);
    GuiTaggedValue field{};
    field.tag = GuiValueTag::String;
    field.s = f.movie_name_fc.c_str();
    GuiTaggedValue fallback{};
    fallback.tag = GuiValueTag::String;
    fallback.s = "";                            // 00CE3A0C
    reader.read_with_default("MovieName", field, fallback);
    if (field.s != nullptr) f.movie_name_fc = field.s;
    f.unit_size_f4 = transform.size.width == 1.0f && transform.size.height == 1.0f;
}

void gui_movie_set_size_00aacf10(void* widget, const GuiWidgetTransform& transform,
                                 const float (&size)[2], GuiLeafHost& host)
{
    host.widget_set_size_00aa7970(widget, size);
    void* stream = host.vertex_stream_00b73260(widget);
    host.stream_lock_10(stream, 4, 0, 0);
    const float w = transform.size.width;       // +20h after the resize
    const float h = 0.75f * transform.size.height;  // [00E12FCC] * +24h
    host.stream_write_position(stream, 0, 0.0f, 0.0f, 0.0f);
    host.stream_write_uv0(stream, 0, 0.0f, 0.0f);
    host.stream_write_position(stream, 1, w, 0.0f, 0.0f);
    host.stream_write_uv0(stream, 1, 1.0f, 0.0f);
    host.stream_write_position(stream, 2, w, h, 0.0f);
    host.stream_write_uv0(stream, 2, 1.0f, 1.0f);
    host.stream_write_position(stream, 3, 0.0f, h, 0.0f);
    host.stream_write_uv0(stream, 3, 0.0f, 1.0f);
    host.stream_unlock_14(stream);
}

namespace {

GuiTaggedValue float_tagged(float value)
{
    GuiTaggedValue v{};
    v.tag = GuiValueTag::Float;
    v.f[0] = value;
    return v;
}

void write_float_unless(GuiPropertyWriter& writer, const char* key, float value, float fallback)
{
    const GuiTaggedValue v = float_tagged(value);
    if (!writer.equals_00bd5680(v, float_tagged(fallback))) writer.write(key, v);
}

} // namespace

void gui_section_describe_properties_00abf090(const void* section, const GuiSectionFields& f,
                                              GuiPropertyWriter& writer, GuiDescribeHost& host)
{
    gui_widgets_cc13::gui_widget_describe_properties_00aaaed0(section, writer, host);
    write_float_unless(writer, "Value", f.value_f4, 0.0f);
    write_float_unless(writer, "StartAngle", f.start_angle_f8, 0.0f);
    GuiTaggedValue texture{};
    texture.tag = GuiValueTag::String;
    texture.s = f.texture_ec.c_str();           // data +F0h, or 00F8BEF4 when null
    GuiTaggedValue empty{};
    empty.tag = GuiValueTag::String;
    empty.s = "";                               // 00CE3A0C
    if (!writer.equals_00bd5680(texture, empty)) writer.write("Texture", texture);
    GuiTaggedValue clockwise{};
    clockwise.tag = GuiValueTag::Bool;
    clockwise.b = f.clockwise_10c;
    GuiTaggedValue clockwise_default{};
    clockwise_default.tag = GuiValueTag::Bool;
    clockwise_default.b = true;
    if (!writer.equals_00bd5680(clockwise, clockwise_default)) writer.write("ClockWise", clockwise);
    write_float_unless(writer, "TextureAngle", f.texture_angle_fc, 0.0f);
    write_float_unless(writer, "U0", f.u0_100, 0.0f);                  // 00D5C8A4
    write_float_unless(writer, "U1", f.u1_104, 1.0f);                  // 00D5C8A0, 00D7A24C
    write_float_unless(writer, "RimWidthPercent", f.rim_width_percent_108, 0.5f);  // 00CE3800
    gui_write_int_unless_default_00ac4ad0(writer, "TextureMode", f.texture_mode_110, 0);
}


// --- segment 49: profile, unlocks and nose art ---------------------------------------------------

namespace {

// The inlined NativeString equality the profile walks use: equal lengths, then both empty or
// __stricmp == 0.
bool native_equals_insensitive(const std::string& a, const std::string& b)
{
    if (a.size() != b.size()) return false;
    if (a.empty()) return true;
#if defined(_MSC_VER)
    return _stricmp(a.c_str(), b.c_str()) == 0;
#else
    return strcasecmp(a.c_str(), b.c_str()) == 0;
#endif
}

} // namespace

bool has_unseen_unlock_007f93f0(const ProfileUnlockState& profile)
{
    // For each pending name (+7Ch, head +80h): lower_bound in +70h (004C2140) plus the
    // __stricmp test; the first one missing returns true.
    for (const std::string& name : profile.pending_unlocks) {
        if (profile.unlocks.find(name) == profile.unlocks.end()) return true;
    }
    return false;
}

bool stored_hint_list_contains_007f8e00(const std::vector<std::string>& hints_08,
                                        const std::string& name)
{
    // Each node's string is first copied to a stack NativeString, compared, then released.
    for (const std::string& hint : hints_08) {
        const std::string copy = hint;
        if (native_equals_insensitive(copy, name)) return true;
    }
    return false;
}

void stored_hint_list_push_back_007fae20(std::vector<std::string>& hints_08, const std::string& name)
{
    hints_08.push_back(name);  // node copy (005FE8B0 / alias-node allocate), size + 1, link at end
}

bool checkpoint_vehicle_class_valid_007fc370(ProfileCheckpointHost& host, const std::string& key)
{
    if (!host.checkpoint_contains_007f8980(key)) return false;     // +94h map, end = +98h
    const std::int32_t vehicle_class = host.checkpoint_vehicle_class_8(key);  // operator[] +8h
    if (vehicle_class == -1) return true;
    return host.lua_vehicle_class_entry_is_table(vehicle_class);   // globals.VehicleClass[n]
}

void mission_progress_ptr_reset_007fd870(void*& progress, ProfileCheckpointHost& host)
{
    if (progress == nullptr) return;
    host.mission_progress_destroy(progress);  // BSP_MissionProgress_Destroy on the block
    host.free_00bf65ac(progress);
    progress = nullptr;
}

ScoringBonusRecord construct_profile_bonus_record_007f8b40(std::string a, std::string b,
                                                           std::string c, std::int32_t value)
{
    // Three NativeStrings by value (consumed: their blocks go back to the pool) and an int.
    ScoringBonusRecord record;
    record.text_00 = std::move(a);
    record.text_08 = std::move(b);
    record.text_10 = std::move(c);
    record.value_18 = value;
    return record;
}

void grant_bonus_text_007fcbc0(std::vector<std::string>& granted_ac,
                               std::vector<ScoringBonusRecord>& records_bc,
                               const std::string& key, const std::string& text_a,
                               const std::string& text_b, const std::string& text_c)
{
    for (const std::string& granted : granted_ac) {
        if (native_equals_insensitive(granted, key)) return;  // the walk stops on the next pass
    }
    granted_ac.push_back(key);                                // 00450540 on +ACh
    // 007F8B40(text_a, text_b, text_c, 0): EBX was zeroed at 007FCC90.
    records_bc.push_back(construct_profile_bonus_record_007f8b40(text_a, text_b, text_c, 0));
}

std::string nose_art_texture_path_007f9cb0(std::int32_t index, ProfileCheckpointHost& host)
{
    // "NoseArt." + decimal index (004263B0) + ".Texture", resolved from the globals
    // (00B67980, 00B68D70).
    const std::string path = "NoseArt." + std::to_string(index) + ".Texture";
    std::string value;
    if (!host.lua_resolve_string(path, value)) return std::string();  // not a string -> ""
    return "models/Textures/noseart/" + value;
}

void nose_art_load_textures_007fa2d0(void* (&textures_104)[3], ProfileCheckpointHost& host)
{
    for (std::int32_t i = 0; i < 3; ++i) {
        if (textures_104[i] != nullptr) {
            host.release_texture(textures_104[i]);            // InterlockedDecrement, slot 0
            textures_104[i] = nullptr;
        }
        const std::string path = nose_art_texture_path_007f9cb0(i, host);  // 007F9F70
        if (!path.empty()) textures_104[i] = host.load_texture_00f8d394_64(path);
    }
}

void send_nose_art_selection_007f8690(const std::uint32_t (&record)[3], ProfileCheckpointHost& host)
{
    // Message E3h (base 0075B430, vtable 00D02FA8) carrying the three words and [game+18C8h].
    NoseArtSelectionMessage message{};
    for (int i = 0; i < 3; ++i) message.words[i] = record[i];
    message.slot = host.local_slot_18c8();
    if (host.session_mode_1fe4() == 1) {
        // The server applies it to its own player record (game+748h + slot*118h, 007F81C0)
        // and hands the message to 0076A9F0 on game+1EF0h with 0.
        host.apply_nose_art_007f81c0(message);
        host.session_dispatch_0076a9f0(message);
    } else {
        host.session_send_to_server_00770af0(message);       // game+1EF0h
    }
}


// --- segment 49: the race record table (00F87464/68) ----------------------------------------------

RaceRecord* race_records_find_by_name_007ffb50(const RaceRecordTable& table, const std::string& name)
{
    for (void** slot = table.begin; slot != table.end; ++slot) {
        auto* record = static_cast<RaceRecord*>(*slot);   // dereferenced without a null test
        const std::uint32_t length = record->name_08.length();
        if (length != name.size()) continue;
        if (length == 0) return record;                   // both empty
        const char* data = record->name_08.data();
#if defined(_MSC_VER)
        if (_stricmp(data, name.c_str()) == 0) return record;
#else
        if (strcasecmp(data, name.c_str()) == 0) return record;
#endif
    }
    return nullptr;
}

void race_records_delete_all_007ffd60(RaceRecordTable& table, NativeStringStorage& strings)
{
    for (void** slot = table.begin; slot != table.end; ++slot) {
        if (*slot == nullptr) continue;
        // vtable slot 0 with 1: 007FFB10, the scalar deleting destructor.
        scalar_delete_race_record_007ffb10(static_cast<RaceRecord*>(*slot), 1, strings);
        *slot = nullptr;
    }
}


// --- small leaves -----------------------------------------------------------------------------------

void owned_object_ptr_reset_007ff320(void*& object, void (*deleting_destructor)(void*, std::uint32_t))
{
    if (object == nullptr) return;
    deleting_destructor(object, 1);                     // vtable slot 0 with 1
    object = nullptr;
}

void rain_set_drop_num_percent_008003b0(RainDropSettingHost& host, float fraction)
{
    host.store_drop_num_percent_00f87470(fraction);     // one store to [00F87470]
}

GuiStringRecord14 gui_string_record14_copy_00aae390(const GuiStringRecord14& in)
{
    GuiStringRecord14 out;
    out.text = in.text;                                 // 0041DD40 + memcpy (skipped for self)
    out.word_08 = in.word_08;
    out.word_0c = in.word_0c;
    out.byte_10 = in.byte_10;
    out.byte_11 = in.byte_11;
    return out;
}

GuiStringRecord24 gui_string_record24_copy_00aae3f0(const GuiStringRecord24& in)
{
    GuiStringRecord24 out;
    out.text = in.text;
    for (int i = 0; i < 5; ++i) out.words_08[i] = in.words_08[i];
    out.byte_1c = in.byte_1c;
    out.word_20 = in.word_20;
    return out;
}

void gui_section_copy_00abf5b0(GuiSectionFields& out, const GuiSectionFields& in,
                               GuiIconStateHost& host)
{
    // After the base copy (00AAxxxx) and vtable 00D5C8E8: the texture name, +F4h..+108h, the
    // byte +10Ch, +110h..+124h, then InterlockedIncrement on a non-null +114h.
    out = in;
    if (out.texture_ref_114 != nullptr) host.retain_texture(out.texture_ref_114);
}

void gui_layer_save_backup_00ac5820(void* layer, const std::string& layer_name_100,
                                    GuiLayerBackupHost& host)
{
    std::string path;
    host.compose_script_path(layer_name_100, path);     // 00AC5770
    const std::string backup = path + ".guiedbak";      // 0041DD40 + 004261A0
    host.remove_file_00bffd99(backup);                  // DeleteFileA through IAT 00CE2234
    host.rename_file_00bffdc8(path, backup);            // MoveFileA through IAT 00CE20DC
    // 00BD5CE0 opens a stdio stream writer on `path`; +4h ("GuiScreen"), the layer's describe
    // (vtable +1Ch) into it, +8h, then the stream is destroyed.
    host.write_layer_archive(layer, path, "GuiScreen");
}


// --- segment 49: the rain model (vtable 00D08D58) -------------------------------------------------

void rain_model_construct_00802250(RainModelFields& f, RainModelHost& host)
{
    // After the generated-model base (BSP_GeneratedModel_Construct): the +188h vector, the six
    // counted references +1ACh..+1C0h and the name +1C4h zeroed, colour +1F4h = 80D0D0FFh.
    f = RainModelFields{};
    f.color_1f4 = 0x80D0D0FFu;
    // The +1B0h/+1B8h/+1C0h resets that follow release nothing (just zeroed); then the global
    // drop fraction [00F87470] goes back to 1.0 (00D7A24C).
    host.store_drop_num_percent_00f87470(1.0f);
}

void rain_model_destroy_00801b70(RainModelFields& f, RainModelHost& host)
{
    // Live effects at +1B8h and +1C0h are stopped first (00867B10, byte +9 = 1).
    for (void* effect : {f.refs_1ac[3], f.refs_1ac[5]}) {
        if (effect != nullptr) {
            host.effect_stop_children_00867b10(effect);
            host.effect_set_byte_9(effect);
        }
    }
    f.name_1c4.clear();                                  // returned to the sized pool
    for (int i = 5; i >= 0; --i) {                       // +1C0h down to +1ACh
        if (f.refs_1ac[i] != nullptr) {
            host.release(f.refs_1ac[i]);
            f.refs_1ac[i] = nullptr;
        }
    }
    f.vector_188.clear();                                // _free of the block, three words zero
    f.vector_188.shrink_to_fit();
}

RainModelParams& rain_model_params_assign_008019a0(RainModelParams& out, const RainModelParams& in)
{
    if (&out != &in) out.name = in.name;
    for (int i = 0; i < 11; ++i) out.words_08[i] = in.words_08[i];
    return out;
}

void rain_model_submit_00800b20(RainModelFields& f, void* self, void* context, float a, float b,
                                std::uint32_t c, RainModelHost& host)
{
    void* node = host.context_node_8(context);           // [context+8h]
    if (host.node_hidden_198(node)) return;              // [node+198h] != 0
    f.offset_1f8[0] = f.offset_1f8[1] = f.offset_1f8[2] = 0.0f;
    f.clock_210 = host.clock_00f876a4();
    host.node_refresh_world_if_stale(node);              // byte +5Ch bit 1 clear -> 00B6DB70
    const float k = f.drop_1ec;
    float axis[3];
    host.node_world_row_110(node, axis);                 // +110h..+118h
    // Each product is spilled as a float (FSTP f), each sum too: plain float arithmetic.
    const float scaled[3] = {k * axis[0], axis[1] * k, k * axis[2]};
    host.node_refresh_world_if_stale(node);
    float origin[3];
    host.node_world_translation_120(node, origin);       // +120h..+128h
    for (int i = 0; i < 3; ++i) f.position_204[i] = scaled[i] + origin[i];
    host.node_refresh_world_if_stale(node);
    host.model_virtual_34(self, node);                   // vtable +34h with node+F0h
    host.render_00b748e0(self, context, a, b, c);        // the arguments passed through
}


// --- segment 49: the rain model's pool (532-byte slots, two per page) -----------------------------

void rain_pool_page_construct_00800450(RainPoolPage& page, std::uint32_t page_index)
{
    page.free_count_434 = 2;
    for (std::uint16_t i = 0; i < 2; ++i) {
        page.free_stack_430[i] = static_cast<std::uint16_t>(1 - i);  // slot 0 is popped first
        page.slot_page_index_214[i] = page_index;
    }
}

void rain_pool_construct_00800c60(RainPool& pool, RainPoolHost& host)
{
    host.registry_link_head_00e188b4(pool);             // prev +4h = 0, next +8h = old head
    host.initialize_critical_section_0c(pool);
    pool.pages.clear();
    pool.capacity_30 = 32;                              // the array grows to 20h entries (80h bytes)
    pool.pages.reserve(32);
    pool.lowest_free_34 = -1;
}

void* rain_pool_allocate_00800ec0(RainPool& pool, RainPoolHost& host)
{
    host.lock_0c(pool);                                 // EnterCriticalSection, +24h + 1
    if (pool.lowest_free_34 == -1) {
        pool.lowest_free_34 = static_cast<std::int32_t>(pool.pages.size());
        // operator_new(438h); a failed allocation stores a null page that the pop below
        // dereferences. make_unique throws instead.
        auto page = std::make_unique<RainPoolPage>();
        rain_pool_page_construct_00800450(*page, static_cast<std::uint32_t>(pool.lowest_free_34));
        if (pool.pages.size() == pool.capacity_30) {
            pool.capacity_30 = pool.capacity_30 * 2 + 2;  // copy into operator_new(cap*4), _free old
            pool.pages.reserve(pool.capacity_30);
        }
        pool.pages.push_back(std::move(page));
    }
    RainPoolPage& page = *pool.pages[static_cast<std::size_t>(pool.lowest_free_34)];
    --page.free_count_434;
    const std::uint16_t slot = page.free_stack_430[page.free_count_434];
    void* result = page.slots[slot].data();
    if (page.free_count_434 == 0) {
        // Look for the next page with a free slot after this one; none leaves -1.
        std::uint32_t next = static_cast<std::uint32_t>(pool.lowest_free_34) + 1;
        pool.lowest_free_34 = -1;
        for (; next < pool.pages.size(); ++next) {
            if (pool.pages[next]->free_count_434 != 0) {
                pool.lowest_free_34 = static_cast<std::int32_t>(next);
                break;
            }
        }
    }
    host.unlock_0c(pool);
    return result;
}

void rain_pool_free_008007a0(RainPool& pool, RainPoolHost& host, std::uint32_t page_index,
                             std::uint16_t slot)
{
    host.lock_0c(pool);
    // The page comes from the slot's +214h word, the slot from (pointer - page) / 218h.
    RainPoolPage& page = *pool.pages[page_index];
    page.free_stack_430[page.free_count_434] = slot;
    ++page.free_count_434;
    // Unsigned compare: -1 (no free page) is above every index.
    if (page_index < static_cast<std::uint32_t>(pool.lowest_free_34)) {
        pool.lowest_free_34 = static_cast<std::int32_t>(page_index);
    }
    host.unlock_0c(pool);
}

void rain_pool_trim_00800e20(RainPool& pool)
{
    // No lock. Each fully free page is freed, the last page moves into its slot and has its two
    // slot index words renumbered, and the same index is examined again.
    for (std::size_t i = 0; i < pool.pages.size(); ++i) {
        if (pool.pages[i]->free_count_434 != 2) continue;
        pool.pages[i] = std::move(pool.pages.back());
        pool.pages.pop_back();
        if (i < pool.pages.size()) {
            for (auto& index : pool.pages[i]->slot_page_index_214) index = static_cast<std::uint32_t>(i);
        }
        --i;  // unsigned wrap at 0 followed by ++, as in the native loop
    }
    pool.lowest_free_34 = -1;
    for (std::size_t i = 0; i < pool.pages.size(); ++i) {
        if (pool.pages[i]->free_count_434 != 0) {
            pool.lowest_free_34 = static_cast<std::int32_t>(i);
            break;
        }
    }
}

void rain_pool_destroy_00800d90(RainPool& pool, RainPoolHost& host)
{
    pool.pages.clear();                                 // each page _free, then the array
    host.release_lock_fully_and_delete_0c(pool);        // LeaveCriticalSection while +24h > 0
    host.registry_unlink_00e188b4(pool);                // prev/next relink, head when prev is 0
}


// --- segment 49: the rain emitter step ------------------------------------------------------------

namespace {

void* spawn_rain_effect(RainEmitterHost& host, void* definition, float translate_y, float translate_z)
{
    // Identity with an optional translation (row 3: m13, m14), transform word 1, options 0, 0,
    // parented to the camera [game+19FCh]; 008687C0 consumes one reference to the definition.
    float matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, translate_y, translate_z, 1};
    if (definition != nullptr) host.add_ref(definition);  // IAT 00CE221C
    return host.create_point_effect_008687c0(host.camera_node_19fc(), definition, matrix);
}

void assign_retained(RainEmitterHost& host, void*& slot, void* created)
{
    // 004845D0 then the temporary's release.
    if (slot != created) {
        if (created != nullptr) host.add_ref(created);
        if (slot != nullptr) host.release(slot);
        slot = created;
    }
    if (created != nullptr) host.release(created);
}

} // namespace

void rain_emitter_step_00801400(RainModelFields& f, float dt, float camera_altitude_4,
                                RainEmitterHost& host)
{
    // k = [00F87470] * 100.0 (00D7A220), spilled as a float.
    const float k = static_cast<float>(static_cast<double>(host.drop_num_percent_00f87470()) * 100.0);
    void* splash = nullptr;  // [ESP+14h], released on exit
    // 00BD2F10 with ECX = 1: U(0, k*dt) on stream 1. A presentation draw; the host owns it.
    const float hi = k * dt;
    const float draw = host.random_uniform_00bd2f10_stream1(0.0f, hi);
    // The threshold dt * 0.3 (00D04368) * k stays on the x87 stack; double stands in.
    const double threshold = static_cast<double>(dt) * 0.3 * static_cast<double>(k);
    if (static_cast<double>(draw) > threshold) {         // FCOMIP; JBE skips
        void* camera = host.camera_node_19fc();
        host.node_refresh_world_if_stale(camera);
        float origin[3];
        host.node_world_translation_120(camera, origin);
        if (origin[1] > 0.0f) {                          // COMISS [camera+124h], 0
            void* created = spawn_rain_effect(host, f.refs_1ac[0], 0.0f, 0.0f);  // +1ACh
            assign_retained(host, splash, created);
            host.effect_set_byte_9(splash);              // 008015A6, no null test
        }
    }
    if (f.refs_1ac[3] == nullptr && static_cast<double>(k) > 25.0) {  // +1B8h, 00CE3880
        // Definition +1B4h, translation (0, 40, 150) (00CE685C, 00CE3808).
        void* created = spawn_rain_effect(host, f.refs_1ac[2], 40.0f, 150.0f);
        assign_retained(host, f.refs_1ac[3], created);
        // Velocity +5Ch..+64h = 100 * the rain direction +1E0h..+1E8h (DC C9 at 008016D6), +28h = 0.
        float velocity[3];
        for (int i = 0; i < 3; ++i) {
            velocity[i] = static_cast<float>(static_cast<double>(f.direction_1e0[i]) * 100.0);
        }
        host.effect_set_velocity_5c(f.refs_1ac[3], velocity);
        host.effect_set_word_28(f.refs_1ac[3], 0);
    }
    const double altitude = static_cast<double>(camera_altitude_4);
    if (200.0 > altitude && f.refs_1ac[5] == nullptr && static_cast<double>(k) > 25.0) {
        // Definition +1BCh into +1C0h, translation (0, -0.0 - altitude, 150) (00D7A208 SUBSS).
        void* created = spawn_rain_effect(host, f.refs_1ac[4], -0.0f - camera_altitude_4, 150.0f);
        assign_retained(host, f.refs_1ac[5], created);
    }
    if ((altitude > 200.0 || 25.0 > static_cast<double>(k)) && f.refs_1ac[5] != nullptr) {
        host.effect_stop_children_00867b10(f.refs_1ac[5]);
        host.effect_set_byte_9(f.refs_1ac[5]);
        host.release(f.refs_1ac[5]);
        f.refs_1ac[5] = nullptr;
    }
    if (splash != nullptr) host.release(splash);
}

namespace {

float descriptor_float(const RainModelParams& d, std::uint32_t offset)
{
    // Descriptor words are read as floats (FLD dword [EBX+offset]); +8h is words_08[0].
    float value;
    std::memcpy(&value, &d.words_08[(offset - 8u) / 4u], sizeof value);
    return value;
}

// U(0, 2r) - 2r * 0.5 (00D7A280, a double): 2r is spilled to a float before both uses.
float centred_draw(RainBuildHost& host, float r)
{
    const float two_r = r + r;
    const float draw = host.random_uniform_00bd2f10_stream0(0.0f, two_r);
    return draw - two_r * 0.5f;
}

} // namespace

RainModelRef rain_emitter_build_008023b0(const RainModelParams& descriptor, RainBuildHost& host)
{
    // 008023E6 / 00802427: the pool slot and its constructor; the name temporary is released.
    RainModelRef model = host.allocate_rain_model_00800ec0("RainModel");
    RainModelFields& f = *model.fields;   // no null test in the image (00802471 uses it)

    // 00802478: 008019A0 copies the descriptor into +1C4h..+1F7h.
    f.name_1c4 = descriptor.name;
    for (std::uint32_t i = 0; i < 5; ++i) f.descriptor_1cc[i] = descriptor_float(descriptor, 8u + 4u * i);
    for (std::uint32_t i = 0; i < 3; ++i) f.direction_1e0[i] = descriptor_float(descriptor, 0x1Cu + 4u * i);
    f.drop_1ec = descriptor_float(descriptor, 0x28);
    f.box_radius_1f0 = descriptor_float(descriptor, 0x2C);
    f.color_1f4 = descriptor.words_08[10];

    // 0080247D: CVTTSS2SI of +8h, then 00802170 resizes the +184h vector to it.
    const std::int32_t count = static_cast<std::int32_t>(descriptor_float(descriptor, 0x08));
    f.vector_188.resize(static_cast<std::size_t>(count));

    // 008024A0..00802692: five stream-0 draws per column, in this order.
    const float radius = descriptor_float(descriptor, 0x2C);
    for (RainColumn& c : f.vector_188) {
        const float h = host.random_uniform_00bd2f10_stream0(0.0f, descriptor_float(descriptor, 0x18));
        c.height_10 = h + descriptor_float(descriptor, 0x14);
        const float w = host.random_uniform_00bd2f10_stream0(0.0f, descriptor_float(descriptor, 0x10));
        c.width_0c = w + descriptor_float(descriptor, 0x0C);
        c.y_04 = centred_draw(host, radius);
        c.x_00 = centred_draw(host, radius);
        c.z_08 = centred_draw(host, radius);
    }

    // 00802697: bounding sphere (0, 0, 0) radius 300000.0 (00D08E2C).
    const float sphere[4] = {0.0f, 0.0f, 0.0f, 300000.0f};
    void* mesh = host.mesh_allocate_construct_00b73b60_bc();
    host.generated_model_set_geometry_00b75170(model.object, 0, mesh, -1.0f, -1.0f);  // 00D7A260
    host.geometry_set_bounds_00b72650(mesh, sphere);

    // 0080272F..008027AD: the vertex format by name; the name temporary is released.
    void* format = host.renderer_vertex_format_38("pf43ccuf42uf42uf43.mvfm");
    const std::uint32_t quads = static_cast<std::uint32_t>(count);
    host.store_vertex_count_00f87474(quads * 4u);         // 008027C7
    host.store_drop_num_percent_00f87470(1.0f);           // 008027D9, 00D7A24C
    void* vertices = host.renderer_vertex_buffer_5c(host.vertex_count_00f87474(), 1, format);
    host.geometry_set_vertex_stream_00b73bb0(mesh, 0, vertices);
    void* indices = host.renderer_index_buffer_60(quads * 6u, 1, 0x65);
    host.geometry_set_index_stream_00b73b70(mesh, indices);

    // 00802840: two triangles per quad, (b, b+1, b+2) and (b, b+2, b+3), b in 16 bits.
    std::uint16_t* index = host.index_buffer_lock_0c(indices);
    std::uint32_t base = 0;
    for (std::int32_t i = 0; i < count; ++i, base += 4u) {
        const std::uint16_t b = static_cast<std::uint16_t>(base);
        const std::uint16_t quad[6] = {b, static_cast<std::uint16_t>(b + 1), static_cast<std::uint16_t>(b + 2),
                                       b, static_cast<std::uint16_t>(b + 2), static_cast<std::uint16_t>(b + 3)};
        for (std::uint16_t v : quad) *index++ = v;
    }
    host.index_buffer_unlock_10(indices);

    float* vertex = host.vertex_buffer_lock_10(vertices);
    // 00802894: the height scale is +18h + +14h; the drift speed is |direction| / 5.0 (00D7A370).
    const float height_max = descriptor_float(descriptor, 0x18) + descriptor_float(descriptor, 0x14);
    const float dx = descriptor_float(descriptor, 0x1C);
    const float dy = descriptor_float(descriptor, 0x20);
    const float dz = descriptor_float(descriptor, 0x24);
    // Each square is spilled to a float; the first two are added on the x87 stack, then the third.
    const float xx = dx * dx;
    const float yy = dy * dy;
    const float zz = dz * dz;
    const float length_sq = (xx + yy) + zz;
    const float length = static_cast<float>(std::sqrt(static_cast<double>(length_sq)));  // 00BF7030
    const float spread = length / 5.0f;
    const float low = -0.0f - spread;                      // 00D7A208 SUBSS
    // 00802916: texture coordinates of the four corners.
    const float uv[4][2] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    for (std::int32_t i = 0; i < count; ++i) {
        const RainColumn& c = f.vector_188[static_cast<std::size_t>(i)];
        // 008029B0 / 008029D1: two stream-0 draws per column.
        const float vx = host.random_uniform_00bd2f10_stream0(low, spread) + dx;
        const float vz = host.random_uniform_00bd2f10_stream0(low, spread) + dz;
        for (int corner = 0; corner < 4; ++corner, vertex += 11) {
            vertex[0] = c.x_00;
            vertex[1] = c.y_04;
            vertex[2] = c.z_08;
            std::memcpy(&vertex[3], &descriptor.words_08[10], sizeof(float));  // +30h, the colour
            const float s = c.height_10 / height_max;      // FDIV, spilled to a float
            vertex[8] = s * vx;
            vertex[9] = dy * s;                            // D8 C9 at 00802A97: ST(0) = ST(0) * ST(1)
            vertex[10] = s * vz;
            vertex[4] = uv[corner][0];
            vertex[5] = uv[corner][1];
            vertex[6] = c.width_0c * uv[corner][0];
            vertex[7] = c.height_10 * -uv[corner][1];      // FCHS then FMULP
        }
    }
    host.vertex_buffer_unlock_14(vertices);
    host.release(indices);
    host.release(vertices);

    // 00802BB5..00802E90: the material and its parameters; each name temporary is released.
    void* material = host.material_create_00535320("rain.mshd");
    host.material_register_float_00b18b20(material, "cLightningIntensity", host.lightning_intensity_010909b4());
    host.material_register_float3_00b18ae0(material, "unitspeed", f.offset_1f8);
    host.material_register_float_00b18b20(material, "time", &f.clock_210);
    host.material_register_float3_00b18ae0(material, "box_center", f.position_204);
    host.material_register_float_00b18b20(material, "box_radius", &f.box_radius_1f0);
    if (!descriptor.name.empty()) {                        // CMP [EBX], 0; JBE
        void* texture = host.renderer_texture_by_name_64(descriptor.name, 0);
        host.material_set_texture_slot_00b189f0(material, 0, texture);
        if (texture != nullptr) host.release(texture);
    }

    // 00802EC8..00803039: one draw section over the vertex stream.
    void* section = host.draw_section_allocate_construct_00b85ee0();
    void* declaration = host.renderer_vertex_declaration_40(format, 1);
    host.draw_section_append_vertex_stream_00b85b80(section, host.geometry_vertex_stream_00b73260(mesh, 0));
    host.draw_section_set_vertex_layout_00b86650(section, declaration);
    host.release(declaration);
    host.release(format);
    host.draw_section_set_material_00b864c0(section, material);
    if (material != nullptr) host.release(material);
    host.geometry_append_draw_section_00b73c60(mesh, section);
    // 00802FA8: +10h = trunc((([00F87474] >> 2) * [00F87470]) * 4.0) under a chop control word.
    const std::uint32_t drawn_quads = host.vertex_count_00f87474() >> 2;
    const float drawn = static_cast<float>(drawn_quads) * host.drop_num_percent_00f87470() * 4.0f;
    host.draw_section_store_words(section, 4, 0, static_cast<std::uint32_t>(static_cast<std::int64_t>(drawn)),
                                  0, quads * 2u);
    host.mesh_element_set_bounds_00b855b0(section, sphere);
    host.mesh_clear_draw_section_owners_00b73c10(mesh);
    host.geometry_append_draw_section_00b73c60(mesh, section);
    host.release(section);
    host.release(mesh);

    // 0080303D: attach under [game+19ECh], zero +1A0h..+1A8h, copy the camera translation.
    host.node_attach_root_00b6d890(model.object, host.scene_root_19ec());
    f.vector_1a0[0] = f.vector_1a0[1] = f.vector_1a0[2] = 0.0f;
    void* camera = host.camera_node_19fc();
    host.node_refresh_world_if_stale(camera);
    host.node_world_translation_120(camera, f.position_194);

    // 008030A0..00803304: effect id 4Bh into +1ACh, "RainCloud" into +1B4h, "RainSplashes" into
    // +1BCh, each acquired with flag 1.
    assign_retained(host, f.refs_1ac[0], host.effect_acquire_00870cd0(0x4B, 1));
    assign_retained(host, f.refs_1ac[2], host.effect_acquire_by_name_00871ba0("RainCloud", 1));
    assign_retained(host, f.refs_1ac[4], host.effect_acquire_by_name_00871ba0("RainSplashes", 1));
    return model;
}


void byte_list_record_assign_007fba30(ByteListRecord& record, std::uint32_t a, std::uint32_t b,
                                      std::uint32_t c, const std::uint8_t* bytes, std::int32_t count)
{
    record.words_20[0] = a;
    record.words_20[1] = b;
    record.words_20[2] = c;
    record.bytes_14.clear();                              // head relinked, size 0, nodes freed
    for (std::int32_t i = 0; i < count; ++i) record.bytes_14.push_back(bytes[i]);  // node + 007FA3A0
    record.count_2c = count;
}


// --- segment 49: content ids, checkpoint copy and the rank award (cc13-left3) ---------------------

void collect_content_ids_007fb520(const ProfileUnlockState& profile, std::vector<std::int32_t>& out)
{
    const std::string prefix = "DL_Content_";
    // Two copies of one loop: +70h through 00442190 (push_back), +7Ch with the push_back inlined
    // (00441F30 when full).
    for (const UnlockNameSet* set : {&profile.unlocks, &profile.pending_unlocks}) {
        for (const std::string& name : *set) {
            // 00469840 substring(0, 11), then length and __stricmp against the prefix.
            if (!native_equals_insensitive(name.substr(0, prefix.size()), prefix)) continue;
            // substring(11, size - 11); its buffer is null when empty -> 00F8745C ("").
            const std::string rest = name.substr(prefix.size());
            out.push_back(static_cast<std::int32_t>(std::atol(rest.c_str())));  // 00BF8417
        }
    }
}

ProfileCheckpointRecord checkpoint_get_copy_007fc490(ProfileCheckpointMapHost& host,
                                                     const std::string& name)
{
    return host.checkpoint_find_or_insert_007fbed0(name);   // 007FBED0 (ECX = profile+94h), 00437490
}

void evaluate_rank_award_007fcd90(UnlockCounterMap& awards_a0,
                                  const std::vector<std::pair<std::string, std::int32_t>>& record_6c,
                                  bool keep_earned, RankAwardHost& host)
{
    // Both "RANK" tests are __stricmp on the C strings with no length test; the first treats an
    // empty key as unequal (NEG/SBB), the second tests the buffer for null.
    const auto is_rank = [](const std::string& key) {
#if defined(_MSC_VER)
        return !key.empty() && _stricmp(key.c_str(), "RANK") == 0;
#else
        return !key.empty() && strcasecmp(key.c_str(), "RANK") == 0;
#endif
    };
    for (const auto& [key, value] : record_6c) {
        const bool earned = host.badge_earned_0090c000(key);          // 007FCE73
        if (!keep_earned && earned) {
            awards_a0[key] += value;                                  // 005070C0, ADD [ESI], EDX
            continue;
        }
        if (!is_rank(key) && !earned) {
            awards_a0[key] = value;                                   // 007FCF94
            continue;
        }
        if (is_rank(key) && value > awards_a0["RANK"]) {              // 007FD048 JLE
            awards_a0["RANK"] = value;                                // 007FD0F8
        }
    }
}


// --- segment 78: the movie widget's loaded hook (cc13-left3) -------------------------------------

void gui_movie_loaded_hook_00aae770(void* widget, void*& player_f0, bool flag_74,
                                    std::uint32_t source_size_fc, std::uint8_t index_f4,
                                    GuiMovieLoadedHookHost& host)
{
    host.deactivate_and_refresh_bounds_00aa7170(widget);
    player_f0 = host.movie_player_00f8aefc_4(index_f4);              // 00AAE7A7, stored at +F0h
    if (!flag_74 && !host.movie_player_virtual_44(player_f0)) {
        void* model = host.generated_model_4c(widget);
        void* mesh = host.mesh_allocate_construct_00b73b60_bc();
        host.generated_model_set_geometry_00b75170(model, 0, mesh, -1.0f, -1.0f);  // 00D7A260
        host.release(mesh);                                          // 00AAE81D
        void* format = host.renderer_vertex_format_38("SimpleColor.mvfm");
        void* vertices = host.renderer_vertex_buffer_5c(4, 1, format);
        host.geometry_set_vertex_stream_00b73bb0(mesh, 0, vertices);
        void* indices = host.renderer_index_buffer_60(6, 1, 0x65);
        host.geometry_set_index_stream_00b73b70(mesh, indices);
        std::uint16_t* index = host.index_buffer_lock_0c(indices, 6);
        void* section = host.draw_section_create_00533fa0();
        void* material = host.material_create_00535320("guidefault.mshd");
        host.draw_section_store_words(section, 4, 0, 4, 0, 2);      // 00AAE91F..00AAE936
        host.widget_virtual_58(widget);
        // 00AAE948: colour bytes FFh x4; the buffer is locked for four vertices.
        host.vertex_buffer_lock_10(vertices, 4);
        const GuiQuadVertexColorLayout layout = host.vertex_color_layout(vertices);
        for (std::uint32_t v = 0; v < 4; ++v) {
            if (layout.packed_offset_34 >= 0) {
                host.vertex_store_dword(vertices, v, layout.packed_offset_34, 0xFFFFFFFFu);
            } else {
                for (std::int32_t channel : layout.channel_offsets_38) {
                    host.vertex_store_float(vertices, v, channel, 1.0f);   // 00D7A24C
                }
            }
        }
        host.vertex_buffer_unlock_14(vertices);
        const std::uint16_t quad[6] = {0, 1, 2, 0, 2, 3};
        for (int i = 0; i < 6; ++i) index[i] = quad[i];
        host.index_buffer_unlock_10(indices);
        host.draw_section_set_material_00b864c0(section, material);
        host.release(material);
        host.draw_section_rebuild_vertex_layout_00b865a0(section, mesh);
        host.geometry_append_draw_section_00b73c60(mesh, section);
        host.release(section);
        host.widget_virtual_50(widget);
        host.release(indices);
        host.release(vertices);
        // 00AAEB0F: [ESP+1Ch] was overwritten by the format at 00AAE863, so the last release is
        // the format, not the mesh allocation slot first stored there.
        host.release(format);
    }
    if (source_size_fc != 0) host.set_source_00aae320(widget, 1);   // CMP [ESI+FCh], 0
}

std::vector<std::string> gui_anim_icon_visit_child_paths_00ab4ce0(
    const std::vector<GuiAnimIconFrame>& frames_f4, const std::string& prefix,
    GuiAnimIconVisitHost& host)
{
    std::vector<std::string> dropped;
    std::int32_t i = 0;
    for (const GuiAnimIconFrame& frame : frames_f4) {   // 40h records
        // 004261A0 / 004263B0 left to right; the result (ESP+20h) is only released.
        dropped.push_back(prefix + "." + std::to_string(i) + ":" + frame.name_04 + "(" +
                          host.texture_name_00b33e40(frame.texture_00) + ")");
        ++i;
    }
    host.visit_child_paths_00aaa0f0(prefix);
    return dropped;
}


// --- segment 78: frame-box layout and the text cursor (cc13-left3) -------------------------------
// x87: every DC-form operand order below was read from the bytes (tools/x87_dc_form_scan.py);
// spills to float are FSTP dword. These bodies run on the main thread, whose x87 precision
// control is 24 bits during gameplay (docs/CC13_FPU_PRECISION.md), so every x87 step rounds to
// float precision and float arithmetic models them, values kept on the stack included.

namespace {

bool ucomiss_equal(float a, float b)
{
    return a == b;   // LAHF; TEST AH,44h; JP/JNP: equal and ordered
}

// x87 FUCOMIP; LAHF; TEST AH,44h; JNP: the store is skipped only when the values are equal.
bool fucomip_equal(double a, double b)
{
    return a == b;
}

} // namespace

void gui_frame_box_set_icon_extent_00ac0970(void* widget, float w, float h, GuiLayoutHost& host)
{
    // COMISS 0, w; JBE: only w < 0 (ordered) is tested against -1.0.
    const bool bad_w = (0.0f > w) && !ucomiss_equal(w, -1.0f);
    const bool bad_h = (0.0f > h) && !ucomiss_equal(h, -1.0f);
    if (bad_w || bad_h) {
        host.widget_virtual_34(widget, 0);
        return;
    }
    host.widget_virtual_34(widget, 1);
    void* stream = host.widget_vertex_stream_a0(widget);
    host.stream_lock_10(stream);
    if (w > 0.0f) {                                         // COMISS w, [00D7A218] = 0.0
        if (!fucomip_equal(w, host.stream_read_position_004768d0(stream, 1)[0])) {
            for (std::uint32_t v : {1u, 3u}) {
                const std::array<float, 3> p = host.stream_read_position_004768d0(stream, v);
                const float xyz[3] = {w, p[1], p[2]};
                host.stream_store_position(stream, v, xyz);
            }
        }
    }
    if (h > 0.0f) {
        // The test reads vertex 2's y against h itself (spilled as a double), not 0.75 * h.
        if (!fucomip_equal(static_cast<double>(h), host.stream_read_position_004768d0(stream, 2)[1])) {
            const float y = 0.75f * h;                      // FLD [00E12FF0]; FMUL h; FSTP
            for (std::uint32_t v : {2u, 3u}) {
                const std::array<float, 3> p = host.stream_read_position_004768d0(stream, v);
                const float xyz[3] = {p[0], y, p[2]};
                host.stream_store_position(stream, v, xyz);
            }
        }
    }
    host.stream_unlock_14(stream);
    void* model = host.widget_generated_model_4c(widget);
    void* section = host.mesh_element_00b732c0(host.generated_model_geometry_00b74640(model, 0), 0);
    host.draw_section_rebuild_vertex_layout_00b865a0(section, host.generated_model_geometry_00b74640(model, 0));
}

void gui_frame_box_place_border_icon_00ac0610(const GuiFrameBoxLayout& layout, void* source,
                                              GuiLayoutHost& host)
{
    const std::array<float, 3> parent = host.widget_resolved_position_00aa6750(host.widget_parent_70(layout.icon_04));
    const std::array<float, 3> at = host.widget_resolved_position_00aa6750(source);
    const float dx = at[0] - parent[0];                     // FSUB, FSTP
    const float dy = at[1] - parent[1];
    host.widget_virtual_58(layout.icon_04, host.widget_size_00aa6740(source));
    host.widget_virtual_30(layout.icon_04, source);
    // DC C9 at 00AC067B: ST(1) = +10h * 0.5; then FSUBR from dy (likewise x with +0Ch).
    const float y = dy - layout.extra_10 * 0.5f;
    const float x = dx - layout.extra_0c * 0.5f;
    host.widget_set_local_xy_00aa7d00(layout.icon_04, x, y);
}

void gui_text_icon_pair_layout_00ac0bd0(const GuiFrameBoxLayout& layout, void* widget,
                                        GuiLayoutHost& host)
{
    float h = host.widget_size_00aa6740(widget)[1] + layout.extra_10;
    if (host.widget_virtual_5c(widget) == 3) {
        h = host.text_normalized_height_00ab6bd0(widget) + layout.extra_10;
    }
    const float extent = h * 1.33333337f;   // 00CF5750, the double of float(4/3)
    gui_frame_box_set_icon_extent_00ac0970(layout.icon_04, -1.0f, extent, host);
    const float half = (host.widget_size_00aa6740(layout.icon_04)[1] - h) * 0.5f;   // FSTP qword, exact
    const float py = host.widget_local_position_00aa67f0(widget)[1];
    const float y = (py + half) - h / 6.0f;   // FSUBP ST(1)
    host.widget_set_local_y_00aa78f0(layout.icon_04, y);
}

namespace {

// The twelve steps both halves of 00AC10A0 repeat, over pieces UL U UR L R BL B BR. right and
// bottom are kept as doubles (FSTP qword), every passed coordinate is spilled to a float.
void lay_out_nine_slice(void* const (&p)[8], float ox, float oy, float sw, float sh, GuiLayoutHost& host)
{
    void* const ul = p[0];
    void* const u = p[1];
    void* const ur = p[2];
    void* const l = p[3];
    void* const r = p[4];
    void* const bl = p[5];
    void* const b = p[6];
    void* const br = p[7];
    const auto size = [&host](void* w) { return host.widget_size_00aa6740(w); };
    const auto to_f = [](float v) { return v; };

    host.widget_set_local_xy_00aa7d00(ul, ox, oy);
    {
        const float a = size(ur)[0];
        gui_frame_box_set_icon_extent_00ac0970(u, to_f(sw - (size(ul)[0] + a)), -1.0f, host);
    }
    host.widget_set_local_xy_00aa7d00(u, to_f(ox + size(ul)[0]), oy);
    const float right = sw + ox;
    host.widget_set_local_xy_00aa7d00(ur, to_f(right - size(ur)[0]), oy);
    {
        const float bly = size(bl)[1];
        gui_frame_box_set_icon_extent_00ac0970(l, -1.0f, to_f(sh - (size(ul)[1] + bly)), host);
    }
    host.widget_set_local_xy_00aa7d00(l, ox, to_f(size(ul)[1] + oy));
    {
        const float bry = size(br)[1];
        gui_frame_box_set_icon_extent_00ac0970(r, -1.0f, to_f(sh - (size(ur)[1] + bry)), host);
    }
    {
        const std::array<float, 2> rs = size(r);
        host.widget_set_local_xy_00aa7d00(r, to_f(right - rs[0]), to_f(size(ur)[1] + oy));
    }
    const float bottom = sh + oy;
    host.widget_set_local_xy_00aa7d00(bl, ox, to_f(bottom - size(bl)[1]));
    {
        const float brx = size(br)[0];
        gui_frame_box_set_icon_extent_00ac0970(b, to_f(sw - (size(bl)[0] + brx)), -1.0f, host);
    }
    {
        const std::array<float, 2> bls = size(bl);
        host.widget_set_local_xy_00aa7d00(b, to_f(bls[0] + ox), to_f(bottom - size(b)[1]));
    }
    {
        const std::array<float, 2> brs = size(br);
        host.widget_set_local_xy_00aa7d00(br, to_f(right - brs[0]), to_f(bottom - size(br)[1]));
    }
}

} // namespace

void gui_frame_box_layout_border_icons_00ac10a0(const GuiFrameBoxLayout& layout, void* widget,
                                                GuiLayoutHost& host)
{
    float size[2] = {0.0f, 0.0f};                           // [ESP+30h], [ESP+34h]
    float offset[2] = {0.0f, 0.0f};                         // [ESP+14h], [ESP+18h]
    const float padding[2] = {0.04f, 0.02f};                // 00CE3910, 00CE9BAC
    host.measure_padded_relative_bounds_00ac06c0(widget, size, offset, padding, 0.0f);
    lay_out_nine_slice(layout.pieces_14, offset[0], offset[1], size[0], size[1], host);
    gui_frame_box_place_border_icon_00ac0610(layout, widget, host);   // second argument unused

    if (host.widget_virtual_5c(widget) != 3) {
        host.widget_virtual_34(layout.container_08, 0);
        return;
    }
    const GuiFrameBoxBorderIconLayout border = host.border_icon_layout_00ab6cb0(widget);
    host.widget_virtual_34(layout.container_08, border.flag_15c);
    host.widget_virtual_58(layout.container_08, host.widget_size_00aa6740(layout.icon_04));
    host.widget_virtual_30(layout.container_08, layout.icon_04);
    // z offset 0.1 (00D7A2F0) when +160h is 0, else -0.1 (00CE3CB4); x and y pass through x87
    // additions of 0.0 (DC C1 at 00AC1477).
    const float dz = border.word_160 == 0 ? 0.1f : -0.1f;
    const std::array<float, 3> at = host.widget_local_position_00aa67f0(layout.icon_04);
    const float xyz[3] = {at[0] + 0.0f, 0.0f + at[1], at[2] + dz};
    host.widget_set_local_position_00aa7dc0(layout.container_08, xyz);
    if (border.flag_15c == 0) return;

    // 00AC14BD: shift = (short)font+14h / 720.0 * (+164h as a float).
    float scale;
    std::memcpy(&scale, &border.word_164, sizeof scale);
    const float shift = static_cast<float>(host.text_font_height_108(widget)) / 720.0f * scale;
    const float ox = shift + offset[0];
    const float oy = shift + offset[1];
    static const char* const kNames[8] = {"UL_Icon", "U_Icon", "UR_Icon", "L_Icon",
                                          "R_Icon",  "BL_Icon", "B_Icon", "BR_Icon"};
    void* pieces[8];
    for (int i = 0; i < 8; ++i) {
        pieces[i] = host.widget_find_child_00aa7e00(layout.container_08, kNames[i], 1);
    }
    lay_out_nine_slice(pieces, ox, oy, size[0], size[1], host);
    for (void* piece : pieces) host.widget_virtual_50(piece, border.rect_168);
}

void gui_text_position_cursor_00ab9240(void* text, GuiTextCursorFields& f, std::uint32_t index,
                                       GuiTextCursorHost& host)
{
    host.ensure_cursor_model_00ab8910(text);
    f.index_180 = index < f.length_ec ? index : f.length_ec;   // unsigned, JAE
    float x = f.x_start_18c;
    const float font_height = static_cast<float>(host.font_height_14(f.font_108));  // CVTSI2SS
    const std::uint16_t* glyphs = f.glyphs_f0;
    const std::uint16_t first = glyphs != nullptr ? glyphs[0] : host.empty_glyph_00f8be50();
    // The glyph of the first character is selected before the loop; after it, EAX holds the
    // glyph of the last character visited (index - 1), or this one when the index is 0.
    GuiGlyphMetrics glyph = host.font_select_glyph_00ad4480(f.font_108, first);
    for (std::uint32_t i = 0; i < f.index_180; ++i) {
        const std::uint16_t ch = glyphs != nullptr ? glyphs[i] : host.empty_glyph_00f8be50();
        glyph = host.font_select_glyph_00ad4480(f.font_108, ch);
        x = static_cast<float>(glyph.advance_12) * f.advance_scale_1d8 + x;
    }
    f.x_190 = x / 960.0f;   // DC F9 at 00AB92FB
    const float left = x + static_cast<float>(glyph.bearing_10);
    const float x0 = static_cast<float>(left);                       // FST [ESP+14h]
    const float right = f.mode_17c == 1 ? left + static_cast<float>(glyph.width_14) : left + 2.0f;  // 00D7A308
    const float x1 = static_cast<float>(right);
    // 00AB933B..00AB9399: (fh + 0) and ((fh + 1) - 1) on the x87 stack, then the divisions.
    const float h = (font_height + 1.0f) - 1.0f;
    const float v = host.vertical_scale_00e12fd4();
    const float u0 = x0 / 960.0f;
    const float u1 = x1 / 960.0f;
    const float y0 = (0.0f / 720.0f) * v;
    const float y1 = (h / 720.0f) * v;

    void* stream = host.cursor_vertex_stream(f.cursor_model_184);
    host.stream_lock_10(stream);
    const GuiQuadVertexColorLayout layout = host.vertex_color_layout(stream);
    // Vertex order (x0, y1), (x0, y0), (x1, y1), (x1, y0); uv +28h = (0, 0); colour white.
    const float corners[4][2] = {{u0, y1}, {u0, y0}, {u1, y1}, {u1, y0}};
    const float uv[2] = {0.0f, 0.0f};
    for (std::uint32_t vtx = 0; vtx < 4; ++vtx) {
        const float xyz[3] = {corners[vtx][0], corners[vtx][1], 0.0f};
        host.stream_store_position(stream, vtx, xyz);
        host.stream_store_uv_28(stream, vtx, uv);
        if (layout.packed_offset_34 >= 0) {
            host.vertex_store_dword(stream, vtx, layout.packed_offset_34, 0xFFFFFFFFu);
        } else {
            for (std::int32_t channel : layout.channel_offsets_38) host.vertex_store_float(stream, vtx, channel, 1.0f);
        }
    }
    host.stream_unlock_14(stream);
}

} // namespace bsp::cc13_seg49_64_78
