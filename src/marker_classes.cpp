#include "bsp/marker_classes.hpp"

#include "bsp/gui_lua_runtime.hpp"
#include "bsp/lua_numeric.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstddef>
#include <iterator>
#include <new>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Marker classes require Win32 intrusive reference offsets.
#endif

namespace bsp {
namespace {
static_assert(sizeof(MarkerClassDescriptor) == 0x20);
static_assert(offsetof(MarkerClassDescriptor, name_04) == 4);
static_assert(offsetof(MarkerClassDescriptor, mesh_0c) == 0x0c);
static_assert(offsetof(MarkerClassDescriptor, resource_14) == 0x14);
static_assert(offsetof(MarkerClassDescriptor, animation_18) == 0x18);
static_assert(offsetof(MarkerClassDescriptor, synchronized_1c) == 0x1c);

struct OwnedString {
    NativeStringStorage& storage;
    NativeString value;
    ~OwnedString() { destroy_native_string_header_0041dd20(&value, storage); }
};
struct CapturedString {
    NativeStringStorage& storage;
    NativeString value;
    char* captured_data{};
    std::uint32_t captured_size{};
    void capture() noexcept { captured_data = value.data(); captured_size = value.length() + 1u; }
    ~CapturedString() { if (captured_data) storage.release(captured_data, captured_size); }
};
struct PairKeyCleanup {
    NativeStringStorage& storage;
    NativeString& key;
    char* captured_data;
    ~PairKeyCleanup() { if (captured_data) storage.release(captured_data, key.length() + 1u); }
};
struct OwnedRef {
    GuiLua51Host& host;
    GuiLuaRef value;
    ~OwnedRef() { host.release(value); }
};
struct Iteration {
    GuiLua51Host& host;
    GuiLuaRef table;
    GuiLuaRef key{}, value{};
    ~Iteration() { host.release(value); host.release(key); }
    bool next(bool restart) {
        host.release(value); value = {};
        host.release(key); key = {};
        return host.next(table, key, value, restart);
    }
};

void clear_marker_registry_keys(MarkerClassRegistry& registry, NativeStringStorage& strings) {
    //006DA6B0: right recursion, capture left, release key, free node, loop left.
    // Standard containers own tree links/free; their size is not count_08.
    while (!registry.entries.empty()) {
        const auto current = std::prev(registry.entries.end());
        destroy_native_string_header_0041dd20(const_cast<NativeString*>(&current->first), strings);
        registry.entries.erase(current);
    }
}
} // namespace

void* load_marker_resource_007188a0(NativeString& name, MarkerClassHost& host) {
    auto* factory = host.current_game_resource_factory_007175d0();
    auto* manager = host.current_resource_manager_004c1400();
    return host.load_and_cache_resource_00b80720(manager, name, factory);
}

MarkerClassDescriptor& construct_marker_class_006d8fc0(MarkerClassDescriptor& descriptor,
    const NativeString& name, const NativeString& mesh, std::int32_t animation,
    std::uint8_t synchronized, MarkerClassContext& context)
{
    descriptor.native_vtable_00 = 0x00cf8f78;
    copy_construct_native_string_header_00426060(&descriptor.name_04, &name, context.strings);
    bool mesh_complete = false;
    try {
        copy_construct_native_string_header_00426060(&descriptor.mesh_0c, &mesh, context.strings);
        mesh_complete = true;
        descriptor.animation_18 = animation;
        descriptor.synchronized_1c = synchronized;
        OwnedString resource_name{context.strings, {}};
        const auto* text = descriptor.mesh_0c.data();
        //00E19980 is the verified empty literal. Only a null POINTER falls back.
        resource_name.value.assign_0041e870(context.strings, text ? text : "");
        descriptor.resource_14 = load_marker_resource_007188a0(resource_name.value, context.host);
        // Resource is published to+14 before the temporary string release.
    } catch (...) {
        if (mesh_complete) destroy_native_string_header_0041dd20(&descriptor.mesh_0c, context.strings);
        destroy_native_string_header_0041dd20(&descriptor.name_04, context.strings);
        throw;
    }
    return descriptor;
}

void destroy_marker_class_006d8bc0(MarkerClassDescriptor& descriptor,
    MarkerClassContext& context)
{
    descriptor.native_vtable_00 = 0x00cf8f78;
    void* const captured = descriptor.resource_14;
    if (captured) {
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(
                static_cast<unsigned char*>(captured) + 4)) == 0)
            context.host.resource_zero_references(captured);
        descriptor.resource_14 = nullptr;
    }
    destroy_native_string_header_0041dd20(&descriptor.mesh_0c, context.strings);
    destroy_native_string_header_0041dd20(&descriptor.name_04, context.strings);
}

MarkerClassDescriptor* scalar_delete_marker_class_006d90e0(MarkerClassDescriptor* descriptor,
    std::uint32_t flags, MarkerClassContext& context)
{
    destroy_marker_class_006d8bc0(*descriptor, context);
    if (flags & 1u) {
        descriptor->~MarkerClassDescriptor();
        singleton_lifetime_free(descriptor);
    }
    return descriptor;
}

MarkerClassMap::iterator lower_bound_marker_class_006d8460(MarkerClassRegistry& registry,
    const NativeString& name)
{
    return registry.entries.lower_bound(name);
}

MarkerClassDescriptor*& lookup_marker_class_006dbc10(MarkerClassRegistry& registry,
    const NativeString& name, NativeStringStorage& strings)
{
    const auto lower = lower_bound_marker_class_006d8460(registry, name);
    if (lower != registry.entries.end()
        && !native_string_less_case_insensitive_00443d00(name, lower->first))
        return lower->second;
    NativeString pair_key;
    copy_construct_native_string_header_00426060(&pair_key, &name, strings);
    const PairKeyCleanup pair_cleanup{strings, pair_key, pair_key.data()};
    OwnedString node_key{strings, {}};
    copy_construct_native_string_header_00426060(&node_key.value, &pair_key, strings);
    auto where = registry.entries.emplace_hint(lower, std::move(node_key.value), nullptr);
    ++registry.count_08;
    return where->second; // cleanup captured pair data before caller publication
}

void load_marker_classes_006dbeb0(MarkerClassContext& context)
{
    GuiLua51Host lua(context.host.current_game_lua_1a0c());
    GuiLuaRef classes_ref;
    {
        OwnedRef root{lua, lua.globals()};
        classes_ref = lua.get_by_name(root.value, "MarkerClasses");
    }
    OwnedRef classes{lua, classes_ref};
    Iteration iterator{lua, classes.value};
    for (bool more = iterator.next(true); more; more = iterator.next(false)) {
        OwnedString name{context.strings, {}};
        name.value.assign_0041e870(context.strings, lua.to_string(iterator.key));
        CapturedString mesh{context.strings, {}};
        {
            OwnedRef field{lua, lua.get_by_name(iterator.value, "Mesh")};
            mesh.value.assign_0041e870(context.strings, lua.to_string(field.value));
            mesh.capture(); //006DBFF9/FFF: before Lua field-reference release
        }
        std::int32_t animation;
        {
            OwnedRef field{lua, lua.get_by_name(iterator.value, "Anim")};
            animation = lua_object_integer_00b66290(lua, field.value, context.crt_sse2_conversion);
        }
        std::uint8_t synchronized;
        {
            OwnedRef field{lua, lua.get_by_name(iterator.value, "Synchronized")};
            synchronized = static_cast<std::uint8_t>(lua.to_boolean(field.value));
        }
        void* storage = singleton_lifetime_allocate({SingletonAllocationKind::object,
            0x20, sizeof(MarkerClassDescriptor)});
        MarkerClassDescriptor* descriptor = nullptr;
        if (storage) {
            try {
                descriptor = ::new (storage) MarkerClassDescriptor;
                construct_marker_class_006d8fc0(*descriptor, name.value, mesh.value,
                    animation, synchronized, context);
            } catch (...) {
                if (descriptor) descriptor->~MarkerClassDescriptor();
                singleton_lifetime_free(storage);
                throw;
            }
        }
        //006DC0E3 is a raw overwrite. There is no release of a previous value,
        // and no descriptor cleanup if registry insertion throws after creation.
        lookup_marker_class_006dbc10(context.registry_00e19974, name.value, context.strings) = descriptor;
        // Captured mesh data/size release precedes current name-header release.
    }
}

void clear_marker_classes_006db2f0(MarkerClassContext& context)
{
    auto& registry = context.registry_00e19974;
    for (auto current = registry.entries.begin(); current != registry.entries.end(); ++current) {
        auto& captured_cell = current->second;
        auto* descriptor = captured_cell;
        if (descriptor) {
            // All live typed entries are the descriptor constructed above.
            scalar_delete_marker_class_006d90e0(descriptor, 1, context);
            captured_cell = nullptr; //006DB34D follows deleting virtual dispatch
        }
    }
    clear_marker_registry_keys(registry, context.strings);
    registry.count_08 = 0;
}

} // namespace bsp
