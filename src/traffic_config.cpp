#include "bsp/traffic_config.hpp"

#include "bsp/gui_lua_runtime.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstring>
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
static_assert(offsetof(TrafficConfig, runtime_tree_10) == 0x10);
static_assert(offsetof(TrafficConfig, runtime_tree_34) == 0x34);
static_assert(sizeof(TrafficGroupListNode) == 0x0c);
struct StringOwner {
    NativeStringStorage& strings;
    NativeString value;
    bool owns = true;
    ~StringOwner() { if (owns) destroy_native_string_header_0041dd20(&value, strings); }
};
struct RefOwner {
    GuiLua51Host& host;
    GuiLuaRef value;
    ~RefOwner() { host.release(value); }
    void assign(GuiLuaRef other) {
        host.release(value);
        value = {};
        value = host.copy_ref_00b66fa0(other);
    }
};
struct Iteration {
    GuiLua51Host& host;
    GuiLuaRef key{}, value{};
    ~Iteration() { host.release(value); host.release(key); }
    bool next(GuiLuaRef table, bool restart) {
        host.release(value); value = {};
        host.release(key); key = {};
        return host.next(table, key, value, restart);
    }
};
template<class T> void write(void* address, std::size_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(address) + offset, &value, sizeof value);
}
void construct_runtime_tree(TrafficRuntimeTreeHeader& tree,
    std::size_t bytes, std::size_t color_offset) {
    //00486070/00486100/00486180/00486210: three null links, color1,isnil0;
    //004A43C0 then publishes head, sets isnil1, parent/left/right=self,count0.
    void* head = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    write<void*>(head, 0, nullptr);
    write<void*>(head, 4, nullptr);
    write<void*>(head, 8, nullptr);
    write<std::uint8_t>(head, color_offset, 1);
    write<std::uint8_t>(head, color_offset + 1, 0);
    tree.head = head;
    write<std::uint8_t>(head, color_offset + 1, 1);
    write<void*>(tree.head, 4, tree.head);
    write<void*>(tree.head, 0, tree.head);
    write<void*>(tree.head, 8, tree.head);
    tree.count = 0;
}
TrafficGroup* construct_group(const TrafficGroupAllocationWords& words) {
    auto* object = static_cast<TrafficGroup*>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x20, sizeof(TrafficGroup)}));
    ::new (object) TrafficGroup; // preserve the native allocation's unconsumed bytes
    object->allocator_word_0c = words.allocator_word_0c;
    object->native_vtable_00 = 0x00ce6600;
    TrafficGroupListNode* head;
    try {
        head = static_cast<TrafficGroupListNode*>(singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 0x0c, sizeof(TrafficGroupListNode)}));
    } catch (...) {
        singleton_lifetime_free(object);
        throw;
    }
    ::new (head) TrafficGroupListNode;
    head->next = head; //00485270 leaves the sentinel payload untouched
    head->previous = head;
    object->head_10 = head;
    object->count_14 = 0;
    const std::uint32_t initial = 0xd01502f9u; // MOVSS from00CE4ADC; raw bits
    std::memcpy(&object->value_18, &initial, 4);
    object->word_08 = 0;
    object->word_04 = 0;
    object->value_1c = 1.0f;
    return object;
}
// Inline49D8E8..966 /49DAB6..DB3A. No old-value preservation. Return whether
// the nonzero resize arm was taken (that arm reloads the vehicle key buffer).
bool assign_value(NativeString& output, const char* captured, NativeStringStorage& strings) {
    const auto length = captured ? static_cast<std::uint32_t>(std::strlen(captured)) : 0u;
    const bool allocated = length != output.length() && length != 0;
    output.resize_0041dd40(strings, length, false);
    if (output.data() && output.length())
        std::memcpy(output.data(), captured, output.length());
    return allocated;
}
void read_pairs(TrafficStringMap& map, Iteration& iterator, GuiLuaRef table,
    NativeStringStorage& strings, bool vehicle) {
    bool restart = true;
    while (iterator.next(table, restart)) {
        restart = false;
        const char* key_text = iterator.host.to_string(iterator.key);
        // Native unconditionally scans the key. A valid string-convertible,
        // nonnull key is required; no fabricated empty replacement is added.
        StringOwner key{strings, {}};
        key.value.resize_0041dd40(strings,
            static_cast<std::uint32_t>(std::strlen(key_text)), true);
        if (key.value.data())
            std::memcpy(key.value.data(), key_text, key.value.length() + 1u);
        char* captured_key = key.value.data();
        const char* captured_value = iterator.host.to_string(iterator.value);
        auto& destination = lookup_traffic_pair_0049c9c0(map, key.value, strings);
        const bool allocated = assign_value(destination, captured_value, strings);
        if (vehicle) {
            if (allocated) captured_key = key.value.data();
            if (captured_key) strings.release(captured_key, key.value.length() + 1u);
            key.owns = false;
        }
    }
}
bool empty_runtime_tree(const TrafficRuntimeTreeHeader& tree) {
    if (!tree.head) return tree.count == 0;
    void* links[3];
    std::memcpy(links, tree.head, sizeof links);
    return tree.count == 0 && links[0] == tree.head && links[1] == tree.head && links[2] == tree.head;
}
bool empty_group(const TrafficGroup* group) {
    return !group || (group->native_vtable_00 == 0x00ce6600 && group->word_04 == 0
        && group->word_08 == 0 && group->count_14 == 0 && group->head_10
        && group->head_10->next == group->head_10 && group->head_10->previous == group->head_10);
}
void clear_pairs(std::optional<TrafficStringMap>& map, NativeStringStorage& strings) {
    if (!map) return;
    while (!map->empty()) {
        auto node = map->extract(map->begin());
        destroy_native_string_header_0041dd20(&node.mapped(), strings);
        destroy_native_string_header_0041dd20(&node.key(), strings);
    }
    map.reset();
}
void free_runtime_tree(TrafficRuntimeTreeHeader& tree) {
    singleton_lifetime_free(tree.head);
    tree.head = nullptr;
    tree.count = 0;
}
} // namespace

TrafficConfig::TrafficConfig(const TrafficConfigAllocationWords& words) noexcept
    : group_08(reinterpret_cast<TrafficGroup*>(static_cast<std::uintptr_t>(words.group_pointer_word_08))),
      word_0c(words.word_0c),
      runtime_tree_10{words.allocator_words[0], nullptr, 0},
      runtime_tree_1c{words.allocator_words[1], nullptr, 0},
      runtime_tree_28{words.allocator_words[2], nullptr, 0},
      runtime_tree_34{words.allocator_words[3], nullptr, 0},
      soldiers_allocator_word_40(words.allocator_words[4]),
      vehicles_allocator_word_4c(words.allocator_words[5]) {}

TrafficConfig& construct_traffic_config_004a43c0(TrafficConfig& owner) {
    owner.native_vtable_00 = 0x00ce68d0;
    try {
        construct_runtime_tree(owner.runtime_tree_10, 0x18, 0x14);
        construct_runtime_tree(owner.runtime_tree_1c, 0x24, 0x20);
        construct_runtime_tree(owner.runtime_tree_28, 0x18, 0x14);
        construct_runtime_tree(owner.runtime_tree_34, 0x1c, 0x18);
        owner.soldiers_40.emplace();
        owner.vehicles_4c.emplace();
        owner.section_04 = critical_section_create_00bd1860();
    } catch (...) {
        // Host exception cleanup only; no native SEH/allocator timing claim.
        owner.vehicles_4c.reset();
        owner.soldiers_40.reset();
        free_runtime_tree(owner.runtime_tree_34);
        free_runtime_tree(owner.runtime_tree_28);
        free_runtime_tree(owner.runtime_tree_1c);
        free_runtime_tree(owner.runtime_tree_10);
        throw;
    }
    return owner;
}

NativeString& lookup_traffic_pair_0049c9c0(TrafficStringMap& map,
    const NativeString& name, NativeStringStorage& strings) {
    const auto lower = map.lower_bound(name); //00485790, same existing comparator
    if (lower != map.end() && !native_string_less_case_insensitive_00443d00(name, lower->first))
        return lower->second;
    //0048E1F0 constructs key then empty mapped string. Node construction copies
    // both again;00489D50 releases the temporary value then key after insertion.
    StringOwner pair_key{strings, {}};
    copy_construct_native_string_header_00426060(&pair_key.value, &name, strings);
    StringOwner pair_value{strings, {}};
    StringOwner node_key{strings, {}};
    copy_construct_native_string_header_00426060(&node_key.value, &pair_key.value, strings);
    StringOwner node_value{strings, {}};
    copy_construct_native_string_header_00426060(&node_value.value, &pair_value.value, strings);
    const auto inserted = map.emplace_hint(lower, std::move(node_key.value), std::move(node_value.value));
    return inserted->second;
}

void load_traffic_config_0049d690(TrafficConfig& owner, LuaStateOwnerEnvironment environment,
    LuaScriptRuntime& scripts, TrafficConfigContext& context) {
    TrafficGroup* group = construct_group(context.group_allocation_words);
    owner.group_08 = group; // native overwrites old ownership without deleting it
    owner.word_0c = 0;
    PcStorageLuaOwner lua(std::move(environment));
    lua.open_storage_archive_00b6a020(1);
    {
        StringOwner path{context.strings, {}};
        path.value.resize_0041dd40(context.strings, 0x25, true);
        if (path.value.data()) std::memcpy(path.value.data(),
            "Scripts\\datatables\\TrafficGlobals.lua", path.value.length() + 1u);
        (void)scripts.run_file(lua.storage_lua_38(), path.value.data(), false);
    }
    {
        GuiLua51Host host(*lua.storage_lua_38());
        RefOwner pairs{host, {}};
        {
            RefOwner globals{host, host.globals()};
            pairs.value = host.get_by_name(globals.value, "PartyPairs");
        }
        Iteration iterator{host};
        RefOwner table{host, host.get_by_name(pairs.value, "Soldiers")};
        read_pairs(owner.soldiers_40.value(), iterator, table.value, context.strings, false);
        {
            RefOwner vehicles{host, host.get_by_name(pairs.value, "Vehicles")};
            table.assign(vehicles.value);
        }
        read_pairs(owner.vehicles_4c.value(), iterator, table.value, context.strings, true);
    }
    lua.close_storage_archive_00b65e80();
}

void release_empty_traffic_group(TrafficGroup* group) {
    if (!empty_group(group)) throw std::logic_error("traffic group requires its actual runtime destructor");
    if (group) {
        singleton_lifetime_free(group->head_10);
        singleton_lifetime_free(group);
    }
}
void release_traffic_config_startup_storage(TrafficConfig& owner, NativeStringStorage& strings) {
    if (!empty_runtime_tree(owner.runtime_tree_10) || !empty_runtime_tree(owner.runtime_tree_1c)
        || !empty_runtime_tree(owner.runtime_tree_28) || !empty_runtime_tree(owner.runtime_tree_34)
        || !empty_group(owner.group_08))
        throw std::logic_error("traffic owner requires populated runtime container destruction");
    critical_section_destroy_owned_0041cc80(owner.section_04);
    release_empty_traffic_group(owner.group_08);
    owner.group_08 = nullptr;
    clear_pairs(owner.vehicles_4c, strings);
    clear_pairs(owner.soldiers_40, strings);
    free_runtime_tree(owner.runtime_tree_34);
    free_runtime_tree(owner.runtime_tree_28);
    free_runtime_tree(owner.runtime_tree_1c);
    free_runtime_tree(owner.runtime_tree_10);
}
} // namespace bsp
