// Packet cc13-powerup: the power-up grant natives and the manager routines they
// reach. See include/bsp/cc13_powerup.hpp and docs/CC13_POWERUP.md. Names are
// hypotheses, not recovered symbols. Main-thread code; the only float operation
// is 008EA0C0's compare, which needs no precision model.

#include "bsp/cc13_powerup.hpp"

#include "bsp/native_string.hpp"
#include "bsp/panel_sequence_types.hpp"

#include <Windows.h>

#include <algorithm>
#include <memory>
#include <stdexcept>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace bsp::cc13_powerup {
namespace {

// 004DC6A0's GUI services as 008E6850 / 008E7350 reach them for the PUMicon and
// mapIconPicture textures. SUBSTITUTION (labelled): the mission frame keeps no
// texture service, so the atlas lookup (00AEFB20 on 00F8C26C) misses and the
// renderer load (slot +64h) is answered null; each request is counted. The
// texture pointers (+24h of a faction, +D0h of a class) are read only by the HUD
// power-up screens, which this process does not build.
class ClassLoadHost final : public PowerupConfigHost {
public:
    ClassLoadHost(std::int32_t faction, unsigned long long& requests)
        : faction_(faction), requests_(requests) {
        callbacks_.find_atlas_item = [](std::string_view) -> const TextureAtlasItem* {
            return nullptr;
        };
        callbacks_.load_texture = [this](std::string_view, std::uint32_t) -> void* {
            ++requests_;
            return nullptr;
        };
        // Reached only on an atlas hit, which cannot happen here.
        callbacks_.width = [](void*) -> std::uint32_t {
            throw std::logic_error("power-up class texture width without an atlas hit");
        };
        callbacks_.height = [](void*) -> std::uint32_t {
            throw std::logic_error("power-up class texture height without an atlas hit");
        };
        callbacks_.retain = [](void*) {
            throw std::logic_error("power-up class texture retain without an atlas hit");
        };
    }
    const GuiTextureCallbacks& current_gui_texture_callbacks_004c12b0() override {
        return callbacks_;
    }
    const std::int32_t& current_selected_player_faction_28() override { return faction_; }
    void texture_zero_references(void*) override {
        throw std::logic_error("power-up class load released a texture it never received");
    }

private:
    std::int32_t faction_;
    unsigned long long& requests_;
    GuiTextureCallbacks callbacks_;
};

bool is_active_slot(PowerupRuntime& runtime, std::int32_t slot) {
    return slot == runtime.host->local_player_slot_18ec();
}

std::size_t slot_index(std::int32_t slot) {
    // The image indexes +20h / +14Ch with the slot unchecked; a slot outside
    // 0..7 writes past the arrays. The host cannot model that and stops.
    if (slot < 0 || slot >= static_cast<std::int32_t>(kPowerupPlayerSlots)) {
        throw std::logic_error("power-up slot outside 0..7 reached the manager");
    }
    return static_cast<std::size_t>(slot);
}

// 00617030 on manager+14Ch + slot*0Ch with the class key: lower bound by the key
// as an unsigned dword; a miss inserts the value 008E3710 zeroes (byte +0h, +4h,
// +8h, floats +0Ch and +10h, byte +14h), which is PowerUpSlot{}.
PowerUpSlot& slot_record_00617030(PowerupRuntime& runtime, std::int32_t slot,
    const PowerupClassConfig* cls) {
    const std::size_t index = slot_index(slot);
    std::vector<std::uintptr_t>& keys = runtime.record_keys[index];
    std::vector<PowerUpSlot>& values = is_active_slot(runtime, slot)
        ? runtime.host->active_slot_records() : runtime.other_records[index];
    const std::uintptr_t key = reinterpret_cast<std::uintptr_t>(cls);
    const auto at = std::lower_bound(keys.begin(), keys.end(), key);
    const std::size_t position = static_cast<std::size_t>(at - keys.begin());
    if (at == keys.end() || key < *at) {
        keys.insert(at, key);
        values.insert(values.begin() + static_cast<std::ptrdiff_t>(position), PowerUpSlot{});
        ++runtime.counts.records_inserted;
    }
    return values[position];
}

// A field of the table at `table_slot`. The reader (004425C0 over 00B66FA0's
// copy) looks the key up in that table; a non-table argument has no fields here
// (host adaptation: the image's reader over a non-table was not read).
bool push_field(lua_State* state, int table_slot, const char* key) {
    if (lua_type(state, table_slot) != LUA_TTABLE) return false;
    lua_getfield(state, table_slot, key);
    return true;
}

// Reader vtable +10h, 00BD6830 with field type 0 (string), no default: an absent
// key converts as nil, which gives an empty string.
std::string field_string(lua_State* state, int table_slot, const char* key) {
    if (!push_field(state, table_slot, key)) return {};
    std::size_t length = 0;
    const char* text = lua_tolstring(state, -1, &length);
    std::string value = text != nullptr ? std::string(text, length) : std::string();
    lua_pop(state, 1);
    return value;
}

// Reader vtable +0Ch, 00BD68D0 with field type 1 (int) and an int default: nil
// takes the default; a number is truncated as the host's other integer reads do.
std::int32_t field_integer(lua_State* state, int table_slot, const char* key,
    std::int32_t fallback) {
    if (!push_field(state, table_slot, key)) return fallback;
    const bool nil = lua_isnil(state, -1);
    const std::int32_t value = nil ? fallback : static_cast<std::int32_t>(lua_tonumber(state, -1));
    lua_pop(state, 1);
    return value;
}

// 008E7820, __thiscall(ECX = item, reader), RET 4: "classID" through 008E6CB0
// into +4h, then "useLimit" (default -1) into +0Ch.
void load_item_008e7820(PowerupRuntime& runtime, PowerupItem& item, lua_State* state,
    int table_slot) {
    const std::string name = field_string(state, table_slot, "classID");  // 00CF86E0
    item.cls = find_powerup_class_008e6cb0(runtime, name);                // 008E789D
    item.use_limit = field_integer(state, table_slot, "useLimit", -1);    // 00D16210
}

// 008EDDC0, __thiscall(ECX = manager, slot, Lua object), RET 8.
void grant_item_008eddc0(PowerupRuntime& runtime, std::int32_t slot, lua_State* state,
    int table_slot) {
    PowerupManagerHost& host = *runtime.host;
    PowerupCounts& counts = runtime.counts;
    if (!host.powerups_enabled_00e0c978()) {  // 008EDDDD
        ++counts.grants_gated;
        return;
    }
    // 008EDE04..008EDE53: a fresh 008E6A80 item appended to the slot's list.
    std::list<PowerupItem>& inventory = runtime.inventories[slot_index(slot)];
    inventory.push_back(PowerupItem{});
    PowerupItem& item = inventory.back();
    // The list size at +28h + slot*0Ch is 008EB110's notify gate for the active slot.
    if (is_active_slot(runtime, slot)) host.set_active_slot_notify_gate(!inventory.empty());
    // 008EDE82..008EDEA8: 00B66FA0 copy, 004425C0 reader, 008E7820, 00441A20.
    load_item_008e7820(runtime, item, state, table_slot);
    if (item.cls == nullptr) ++counts.grant_class_misses;
    ++counts.grants;
    // 008EDEB6..008EDED4: 009789A0(slot, item, "pup_gain" 00D16270) on [00F8A0C4].
    host.report_powerup_009789a0(slot, item, "pup_gain");
    ++counts.pup_gain_reports;
    // 008EDEFC..008EDF10: [00E198C4], its +C0h screen's byte +5, then 0061B0F0.
    if (host.powerup_screen_shown_00e198c4()) {
        host.powerup_screen_rebuild_lists_0061b0f0();
        ++counts.screen_rebuilds;
    }
    // 008EDF1E..008EDF8F: "PUM1STGET" (00D162B4); SETZ on 0068E600, then
    // 00690FD0(hint, 0) when it had not been shown.
    const std::string hint = "PUM1STGET";
    const bool not_shown = !host.hint_has_been_shown_0068e600(hint);
    if (not_shown) {
        host.trigger_hint_00690fd0(hint, 0);
        ++counts.hint_triggers;
    }
    // 008EDFB7..008EDFF7: message 67, op 0, only when [game+1FE4h] == 1.
    if (host.session_word_1fe4() == 1) {
        host.send_powerup_message67(slot, item, 0);
        ++counts.session_messages;
    }
}

} // namespace

PowerupRuntime& powerup_runtime() {
    static PowerupRuntime runtime;
    return runtime;
}

bool load_powerup_classes_004dc6a0(PowerupRuntime& runtime, PowerupManagerHost& host,
    std::string_view script_text, std::int32_t selected_player_faction) {
    // 004DC6A0: operator_new(1C4h) at 004DC882, 008EDC60 at 004DC899 (publishes
    // 00F88C30), then 008ECEC0. The allocator words are the uninitialised bytes
    // the constructor leaves; zero is a host choice. Every mission load builds a
    // new manager: 004DFB70 calls 004DC6A0 once per load, and 004D2BB0 deletes the
    // previous one first (destroy_powerup_manager_004d2d84). So each load starts
    // with empty inventories and empty +14Ch maps.
    static const PowerupOwnerAllocationWords allocation{};
    auto* owner = new PowerupConfigOwner(allocation);
    construct_powerup_config_008edc60(*owner, runtime.manager);
    runtime.host = &host;
    // A failed run or walk leaves the image with a manager whose registry is
    // partly filled. The host does not publish one: it releases the owner, and
    // the natives stay unimplemented records for this load.
    const auto fail = [&](std::string message) {
        runtime.load_error = std::move(message);
        destroy_powerup_manager_004d2d84(runtime);
        return false;
    };

    // 008ECEC0 opens a fresh state (mask 4) and runs the file; the text sets
    // only globals from constructors and arithmetic, so a bare state runs it the
    // same way.
    std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(), &lua_close);
    if (!state) return fail("cannot open a Lua state");
    int status = luaL_loadbuffer(state.get(), script_text.data(), script_text.size(),
        "@Scripts/datatables/PowerupClasses.lua");
    if (status == 0) status = lua_pcall(state.get(), 0, 0, 0);
    if (status != 0) {
        const char* message = lua_tostring(state.get(), -1);
        return fail(message != nullptr ? message : "PowerupClasses.lua failed");
    }
    ClassLoadHost load_host(selected_player_faction, runtime.counts.texture_requests);
    // Same CRT conversion source as the game host's (C27B7C's __get_sse2_info).
    const bool sse2 = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE;
    // Uninitialised stack words in the image; zero is a host choice.
    static const PowerupDescriptorAllocationWords descriptor_scratch{};
    static const std::array<std::uint32_t, 2> texture_size_words{};
    PowerupConfigContext context{crt_string_storage(), load_host, sse2, descriptor_scratch,
        texture_size_words};
    try {
        load_powerup_config_tables_008ecec0(*owner, *state, context);
    } catch (const std::exception& failure) {
        return fail(failure.what());
    }
    runtime.counts.class_records = owner->descriptors_14 ? owner->descriptors_14->size() : 0u;
    return true;
}

void destroy_powerup_manager_004d2d84(PowerupRuntime& runtime) {
    // 004D2D84..004D2D96 in 004D2BB0: when [00F88C30] is set, its vtable slot 0
    // (008EDAD0) with 1, then [00F88C30] = 0. The registry goes through
    // release_powerup_configuration_storage; the inventories and +14Ch maps are
    // the runtime's, cleared with it, so the next load's keys, the frame's
    // record values and its notify gate all start empty together.
    if (runtime.manager != nullptr) {
        PowerupConfigOwner* owner = runtime.manager;
        // The release reaches no texture (every load answered null) and no
        // faction read.
        unsigned long long unused_requests = 0;
        ClassLoadHost release_host(0, unused_requests);
        const bool sse2 = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE;
        static const PowerupDescriptorAllocationWords descriptor_scratch{};
        static const std::array<std::uint32_t, 2> texture_size_words{};
        PowerupConfigContext context{crt_string_storage(), release_host, sse2,
            descriptor_scratch, texture_size_words};
        runtime.manager = nullptr;
        release_powerup_configuration_storage(*owner, context);
        delete owner;
    }
    runtime.host = nullptr;
    for (std::size_t slot = 0; slot < kPowerupPlayerSlots; ++slot) {
        runtime.inventories[slot].clear();
        runtime.record_keys[slot].clear();
        runtime.other_records[slot].clear();
    }
    for (auto& list : runtime.category_lists) list.clear();
    runtime.counts = PowerupCounts{};
}

const PowerupClassConfig* find_powerup_class_008e6cb0(PowerupRuntime& runtime,
    std::string_view name) {
    // 008E6CB0, __thiscall(ECX = manager, NativeString* name), RET 4: 008E6530
    // finds the name in the +14h tree (case-insensitive 00443D00 order); a hit
    // runs 008E34F0 on the class and returns node+14h, a miss returns 0.
    PowerupClassMap& values = *runtime.manager->descriptors_14;
    NativeStringStorage& strings = crt_string_storage();
    NativeString key;
    key.assign_0041e870(strings, std::string(name).c_str());
    const auto found = lower_bound_powerup_class_008e5b10(values, key);
    const bool hit = found != values.end()
        && !native_string_less_case_insensitive_00443d00(key, found->first);
    key.release_to(strings);
    if (!hit) return nullptr;
    runtime.host->resolve_air_support_classes_008e34f0(found->second);  // 008E6CFE
    if (found->second.group_08 == 1) ++runtime.counts.air_support_resolves;
    return &found->second;
}

std::vector<const PowerupItem*> list_ready_items_008ea0c0(PowerupRuntime& runtime,
    std::int32_t slot) {
    // 008EA0C0, __thiscall(ECX = manager, slot, vector*), RET 8. The vector is
    // emptied first (005A0BE0), then filled only when 00E0C978 is set.
    std::vector<const PowerupItem*> ready;
    PowerupManagerHost& host = *runtime.host;
    if (!host.powerups_enabled_00e0c978()) return ready;
    for (const PowerupItem& item : runtime.inventories[slot_index(slot)]) {
        // 008EA133: the record is looked up (and inserted) for every item.
        const PowerUpSlot& record = slot_record_00617030(runtime, slot, item.cls);
        // 008EA13C: byte +14h set skips.
        if (record.relative_deadline) continue;
        // 008EA142..008EA14F: FLD [rec+10h], FLD [00F876A4], FCOMIP, JBE: the
        // clock must be above the stored end; unordered skips.
        const float now = host.clock_00f876a4();
        if (!(now > record.absolute_expiry)) continue;
        // 008EA163..008EA1B1: the last entry of the same class is replaced,
        // else the item is pushed.
        std::size_t match = ready.size();
        for (std::size_t index = 0; index < ready.size(); ++index) {
            if (ready[index]->cls == item.cls) match = index;
        }
        if (match != ready.size()) {
            ready[match] = &item;
        } else {
            ready.push_back(&item);
        }
    }
    return ready;
}

int lua_add_powerup_008ee410(PowerupRuntime& runtime, lua_State* state) {
    // 008EE410. The function-local profiler name (00F88C4C bit 0, 00F88C48) has
    // no effect beyond itself. 00B663F0 is the argument count.
    ++runtime.counts.add_calls;
    if (lua_gettop(state) == 1) {
        // 008EE4C4..008EE4F0: argument 0 to [game+18ECh].
        grant_item_008eddc0(runtime, runtime.host->local_player_slot_18ec(), state, 1);
    } else {
        // 008EE4FB..008EE53C: argument 1 is the table, argument 0 (00B66290) the slot.
        ++runtime.counts.add_two_argument;
        const std::int32_t slot = static_cast<std::int32_t>(lua_tonumber(state, 1));
        grant_item_008eddc0(runtime, slot, state, 2);
    }
    return 0;  // 00B66400: nothing pushed
}

int lua_prepare_powerup_008ea610(PowerupRuntime& runtime, lua_State* state) {
    // 008EA610: only with exactly one argument, 008E9FD0(argument 0).
    ++runtime.counts.prepare_calls;
    if (lua_gettop(state) == 1) {
        // 008E9FD0, __stdcall(Lua object), RET 4: a temporary item loaded by
        // 008E7820, message 67 op 3 for slot 0 when [game+1FE4h] == 1, then freed.
        PowerupItem item{};
        load_item_008e7820(runtime, item, state, 1);
        ++runtime.counts.prepare_loads;
        if (runtime.host->session_word_1fe4() == 1) {
            runtime.host->send_powerup_message67(0, item, 3);  // 008EA06B..008EA079
            ++runtime.counts.session_messages;
        }
    }
    return 0;
}

int lua_get_available_powerups_008eb350(PowerupRuntime& runtime, lua_State* state) {
    // 008EB350: the slot is [game+18ECh] unless an argument is given (008EB40B
    // JL), then 008EAB00(slot, call frame).
    ++runtime.counts.available_calls;
    std::int32_t slot = runtime.host->local_player_slot_18ec();
    if (lua_gettop(state) >= 1) slot = static_cast<std::int32_t>(lua_tonumber(state, 1));
    // 008EAB00, __thiscall(ECX = manager, slot, frame), RET 8: 008EA0C0, a new
    // table (00B67930), and the class name of each entry at 1..n (00B672F0:
    // pushnumber, pushlstring, settable).
    const std::vector<const PowerupItem*> ready = list_ready_items_008ea0c0(runtime, slot);
    lua_newtable(state);
    for (std::size_t index = 0; index < ready.size(); ++index) {
        const PowerupClassConfig* cls = ready[index]->cls;
        // 008EAB69 reads [entry+4] as the name with no test; a granted item whose
        // class missed would fault there.
        if (cls == nullptr) throw std::logic_error("008EAB00 reached an item with no class");
        lua_pushnumber(state, static_cast<lua_Number>(index + 1));
        lua_pushlstring(state, cls->name_00.data() != nullptr ? cls->name_00.data() : "",
            cls->name_00.length());
        lua_settable(state, -3);
        ++runtime.counts.available_entries;
    }
    return 1;
}

} // namespace bsp::cc13_powerup
