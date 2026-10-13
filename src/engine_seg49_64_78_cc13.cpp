// Packet cc13-sys6: segments 49, 64 and 78. See docs/CC13_SYS_6.md and the header for
// addresses, ABI and evidence. Comments cite the instruction each statement stands for.
#include "bsp/engine_seg49_64_78_cc13.hpp"

#include <cstdint>
#include <cstring>
#include <utility>

#include "bsp/band_0067_cc13.hpp"

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

void gui_font_registry_list_construct_00ac3660(GuiFontRegistryList& list, void* sentinel)
{
    list.head_04 = sentinel;
    list.size_08 = 0;
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

} // namespace bsp::cc13_seg49_64_78
