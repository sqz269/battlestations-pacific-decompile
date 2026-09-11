#include "bsp/warning_owner.hpp"

#include <cstring>
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
template<class T> T read(const void* base, std::size_t offset) noexcept {
    T result;
    std::memcpy(&result, static_cast<const std::byte*>(base) + offset, sizeof(result));
    return result;
}
template<class T> void write(void* base, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(base) + offset, &value, sizeof(value));
}
float float_word(std::uint32_t word) noexcept {
    float result;
    std::memcpy(&result, &word, sizeof(result));
    return result;
}

// Leaf native allocation routines, not tree insertion/balancing algorithms.
// Their payload and trailing padding remain the allocator's actual bytes.
void* blank_tree_node(std::size_t bytes, std::size_t color_offset) {
    void* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    write<void*>(node, 0, nullptr);
    write<void*>(node, 4, nullptr);
    write<void*>(node, 8, nullptr);
    write<std::uint8_t>(node, color_offset, 1);
    write<std::uint8_t>(node, color_offset + 1, 0);
    return node;
}
void* allocate_node_0096b960() { return blank_tree_node(0x18, 0x14); }
void* allocate_node_0096b9b0() { return blank_tree_node(0x24, 0x20); }
void* allocate_node_005826b0() { return blank_tree_node(0x1c, 0x18); }
void* allocate_node_00443e20() { return blank_tree_node(0x1c, 0x18); }
void* allocate_node_0096baf0() { return blank_tree_node(0x18, 0x14); }
void* allocate_node_0096bb80() { return blank_tree_node(0x20, 0x1c); }
void* allocate_node_0096bbd0() { return blank_tree_node(0x20, 0x1c); }
void* allocate_node_005a0220() { return blank_tree_node(0x14, 0x10); }

void install_sentinel(WarningOpaqueTreeHeader& tree, void* node, std::size_t nil_offset) {
    tree.head_04 = node;
    write<std::uint8_t>(node, nil_offset, 1);
    write<void*>(tree.head_04, 4, tree.head_04);
    write<void*>(tree.head_04, 0, tree.head_04);
    write<void*>(tree.head_04, 8, tree.head_04);
    tree.count_08 = 0;
}

struct OwnedRef {
    GuiLua51Host& host;
    GuiLuaRef ref;
    OwnedRef(GuiLua51Host& h, GuiLuaRef r) : host(h), ref(r) {}
    ~OwnedRef() { host.release(ref); }
    OwnedRef(const OwnedRef&) = delete;
    OwnedRef& operator=(const OwnedRef&) = delete;
};
struct OwnedString {
    NativeStringStorage& storage;
    NativeString value;
    ~OwnedString() { destroy_native_string_header_0041dd20(&value, storage); }
};
void literal_string(NativeString& value, NativeStringStorage& storage,
    const char* literal, std::uint32_t length) {
    value.resize_0041dd40(storage, length, true);
    if (value.data()) std::memcpy(value.data(), literal, value.length() + 1u);
}
void assign_ref(OwnedRef& destination, GuiLuaRef parent, const char* key) {
    OwnedRef temporary(destination.host, destination.host.get_by_name(parent, key));
    destination.host.release(destination.ref);
    destination.ref = {};
    destination.ref = destination.host.copy_ref_00b66fa0(temporary.ref);
}
void invalid_parameter(const WarningOwnerContext& context) {
    if (!context.invalid_parameters.invalid_parameter)
        throw std::invalid_argument("warning source iterator requires native invalid-parameter binding");
    context.invalid_parameters.invalid_parameter(context.invalid_parameters.context);
}
void increment_effect(void* object) noexcept {
    auto* refs = reinterpret_cast<volatile LONG*>(static_cast<std::byte*>(object) + 4);
    ::InterlockedIncrement(refs);
}
void release_effect(void* object, WarningOwnerServices& services) {
    auto* refs = reinterpret_cast<volatile LONG*>(static_cast<std::byte*>(object) + 4);
    if (::InterlockedDecrement(refs) == 0) services.effect_zero_references(object);
}
void load_effect(void*& destination, const char* literal, std::uint32_t length,
    WarningOwnerContext& context) {
    OwnedString name{context.strings, {}};
    literal_string(name.value, context.strings, literal, length);
    void* temporary; //00871BA0's actual output, not an incoming handle value
    context.services.acquire_effect_00871ba0(temporary, name.value, 1);
    void* const next = temporary;
    void* const previous = destination;
    if (previous != next) {
        destination = next;
        if (next) increment_effect(next);
        if (previous) release_effect(previous, context.services);
    }
    // Reload the native output slot after the previous object's virtual call.
    // The resource service may have retained the address of that output slot.
    void* const release = temporary;
    if (release) {
        release_effect(release, context.services);
        temporary = nullptr;
    }
    // Native destroys the temporary handle before this name's storage.
}
} // namespace

WarningOwner::WarningOwner(const WarningOwnerAllocationWords& allocation) noexcept
    : unconsumed_34(allocation.unconsumed_34),
      tree_d4{allocation.allocator_words[1], nullptr, 0},
      channels_f8{allocation.allocator_words[4], nullptr, 0},
      channel_flags_104(allocation.channel_flags_104),
      tree_144{allocation.allocator_words[8], nullptr, 0},
      tree_150{allocation.allocator_words[9], nullptr, 0},
      tree_15c{allocation.allocator_words[10], nullptr, 0},
      tree_168{allocation.allocator_words[11], nullptr, 0},
      tree_184{allocation.allocator_words[12], nullptr, 0},
      tree_1a4{allocation.allocator_words[13], nullptr, 0},
      canonical_allocator_words{allocation.allocator_words[0], allocation.allocator_words[2],
          allocation.allocator_words[3], allocation.allocator_words[5],
          allocation.allocator_words[6], allocation.allocator_words[7]} {
    for (std::size_t i = 0; i != clocks_04_20.size(); ++i)
        clocks_04_20[i] = float_word(allocation.clock_words_04_20[i]);
    state.suppressed = (allocation.suppression_word_d0 & 0xffu) != 0;
    for (std::size_t i = 0; i != unconsumed_d1_d3.size(); ++i)
        unconsumed_d1_d3[i] = static_cast<std::byte>(allocation.suppression_word_d0 >> (8u * (i + 1u)));
    state.air_raid_deadline = float_word(allocation.air_raid_deadline_174);
    state.air_raid_active = allocation.air_raid_active_178 != 0;
    state.collision_deadline = float_word(allocation.collision_deadline_17c);
    state.collision_active = allocation.collision_active_180 != 0;
    state.periodic_accumulator = float_word(allocation.periodic_accumulator_198);
}

WarningOwner& construct_warning_owner_0098a020(WarningOwner& owner) {
    owner.native_vtable_00 = kWarningOwnerNativeVtable;
    owner.roster_28.emplace(); //0096B260 creates a C-byte list head; payload unconsumed
    for (auto& string : owner.strings_38) ::new (&string) NativeString;
    install_sentinel(owner.tree_d4, allocate_node_0096b960(), 0x15);
    owner.state.pending.clear(); //0096B2D0: empty canonical pending queue atE0
    owner.state.applied.clear(); //0096B2D0: empty canonical applied queue atEC
    install_sentinel(owner.channels_f8, allocate_node_0096b9b0(), 0x21);
    owner.tables_108.emplace(); //008E5CF0: existing canonical message map
    //0097BBC0 initializes each escape section's empty string and replacement
    //map. Those are members of the same canonical WarningMessageTables object;
    //no duplicate native map or string storage is constructed alongside it.
    install_sentinel(owner.tree_144, allocate_node_005826b0(), 0x19);
    install_sentinel(owner.tree_150, allocate_node_00443e20(), 0x19);
    install_sentinel(owner.tree_15c, allocate_node_0096baf0(), 0x15);
    install_sentinel(owner.tree_168, allocate_node_0096bb80(), 0x1d);
    install_sentinel(owner.tree_184, allocate_node_0096bbd0(), 0x1d);
    owner.environment_us_horn_190 = nullptr;
    owner.environment_air_raid_194 = nullptr;
    owner.state.prompt_callback.clear();
    install_sentinel(owner.tree_1a4, allocate_node_005a0220(), 0x11);
    owner.critical_section_24 = critical_section_create_00bd1860();
    return owner;
}

void rebuild_warning_roster_00973f20(WarningOwner& owner, WarningOwnerContext& context) {
    owner.roster_28->clear(); //list clear/allocate/count are standard-container boundaries
    void* outer_owner = read<void*>(context.world_e188a8, 0x6b4);
    void* outer_node = read<void*>(read<void*>(outer_owner, 4), 0);
    WarningRosterIterator outer{outer_owner, outer_node};
    for (;;) {
        void* const current_owner = read<void*>(context.world_e188a8, 0x6b4);
        void* const outer_end = read<void*>(current_owner, 4);
        if (!outer_owner || outer_owner != current_owner) invalid_parameter(context);
        if (outer_node == outer_end) break;
        if (!outer_owner) invalid_parameter(context);
        if (outer_node == read<void*>(outer_owner, 4)) invalid_parameter(context);
        void* const inner_tree = static_cast<std::byte*>(outer_node) + 0x128;
        void* inner_owner = inner_tree;
        void* inner_node = read<void*>(read<void*>(outer_node, 0x12c), 0);
        WarningRosterIterator inner{inner_owner, inner_node};
        for (;;) {
            if (outer.node == read<void*>(outer.owner, 4)) invalid_parameter(context);
            void* const inner_end = read<void*>(inner_tree, 4);
            if (!inner_owner || inner_owner != inner_tree) invalid_parameter(context);
            if (inner_node == inner_end) break;
            if (!inner_owner) invalid_parameter(context);
            if (inner_node == read<void*>(inner_owner, 4)) invalid_parameter(context);
            //0096B4C0 copies only the DWORD at source+C into destination node+8.
            //0096F8C0's native count limit is3FFFFFFF; host std::list owns its
            //own node allocation, insertion and exception ABI.
            const auto value = read<std::uint32_t>(inner_node, 0x0c);
            owner.roster_28->push_back(value);
            context.services.increment_inner_0057f510(inner);
            inner_node = inner.node;
            inner_owner = inner.owner;
        }
        context.services.increment_outer_00581290(outer);
        outer_node = outer.node;
        outer_owner = outer.owner;
    }
}

void initialize_warning_owner_009870a0(WarningOwner& owner, LuaStateOwnerEnvironment environment,
    LuaScriptRuntime& scripts, WarningOwnerContext& context) {
    context.singleton_f8a0c4 = &owner;
    owner.state.air_raid_deadline = 0.0f;
    owner.state.air_raid_active = false;
    owner.state.collision_deadline = 0.0f;
    owner.state.collision_active = false;
    PcStorageLuaOwner lua(std::move(environment));
    lua.open_storage_archive_00b6a020(1);
    {
        OwnedString path{context.strings, {}};
        literal_string(path.value, context.strings, kWarningDataScript, 0x1f);
        (void)scripts.run_file(lua.storage_lua_38(), path.value.data(), false);
    }
    {
        GuiLua51Host host(*lua.storage_lua_38());
        const auto globals = host.globals();
        OwnedRef warnings(host, host.get_by_name(globals, "Warnings"));
        host.release(globals); //009871AF, before roster construction
        rebuild_warning_roster_00973f20(owner, context);
        owner.state.suppressed = false;
        OwnedRef escape(host, host.get_by_name(warnings.ref, "escapecharacters"));
        OwnedRef selected(host, {});
        auto& tables = *owner.tables_108;
        assign_ref(selected, escape.ref, "entity");
        load_warning_escape_table_00979990(tables, tables.entity, host, selected.ref);
        assign_ref(selected, escape.ref, "playerunit_section");
        load_warning_escape_table_00979990(tables, tables.playerunit_section, host, selected.ref);
        assign_ref(selected, warnings.ref, "messages");
        {
            OwnedString prefix{context.strings, {}};
            literal_string(prefix.value, context.strings, "", 0);
            load_warning_message_tree_0097f570(tables.messages, host, selected.ref, "");
        }
        //Keep selected/messages, escapecharacters and Warnings references alive
        //through both effect callbacks, hook publication and the final resets.
        load_effect(owner.environment_us_horn_190, "EnvironmentUSHorn", 0x11, context);
        load_effect(owner.environment_air_raid_194, "EnvironmentAirRaid", 0x12, context);
        owner.state.periodic_accumulator = 0.0f;
        write<std::uint32_t>(context.hook_owner_f8bbcc, 0x230, 0x00987080u);
        context.hook_f8bf4c = 0x00987090u;
        for (auto& clock : owner.clocks_04_20) clock = 0.0f;
    }
    lua.close_storage_archive_00b65e80();
}

void warning_hook_00987080(WarningOwnerContext& context, std::uint32_t ecx_argument) {
    context.services.call_00985c50(*context.singleton_f8a0c4, ecx_argument);
}
void warning_hook_00987090(WarningOwnerContext& context, std::uint32_t ecx_argument,
    std::uint32_t edx_argument) {
    context.services.call_00985250(*context.singleton_f8a0c4, ecx_argument, edx_argument);
}

void release_warning_owner_host_storage(WarningOwner& owner, WarningOwnerContext& context) {
    WarningOpaqueTreeHeader* trees[]{&owner.tree_d4, &owner.channels_f8, &owner.tree_144,
        &owner.tree_150, &owner.tree_15c, &owner.tree_168, &owner.tree_184, &owner.tree_1a4};
    for (auto* tree : trees)
        if (tree->count_08 != 0)
            throw std::invalid_argument("populated warning tree requires its native owning destructor");
    if (owner.environment_us_horn_190) {
        release_effect(owner.environment_us_horn_190, context.services);
        owner.environment_us_horn_190 = nullptr;
    }
    if (owner.environment_air_raid_194) {
        release_effect(owner.environment_air_raid_194, context.services);
        owner.environment_air_raid_194 = nullptr;
    }
    for (auto& string : owner.strings_38) string.release_to(context.strings);
    for (auto* tree : trees) {
        singleton_lifetime_free(tree->head_04);
        tree->head_04 = nullptr;
    }
    critical_section_destroy_owned_0041cc80(owner.critical_section_24);
}
} // namespace bsp
