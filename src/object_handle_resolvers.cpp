#include "bsp/object_handle_resolvers.hpp"

#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Object handle reconstruction requires MSVC Win32 pointers.
#endif

namespace bsp {
void* object_from_handle_006ad080(std::uint32_t handle, const ObjectHandleTables& tables) noexcept {
    if (handle == 0) return nullptr;
    const std::uint32_t id = static_cast<std::uint16_t>(handle);
    std::uint32_t index;
    const void* entries;
    if (static_cast<std::int32_t>(id) < tables.first_end_00f89a10) {
        index = id - static_cast<std::uint32_t>(tables.first_begin_00f89a0c);
        entries = tables.first_entries_00f89a54;
    } else {
        index = id - static_cast<std::uint32_t>(tables.second_begin_00f89a60);
        entries = tables.second_entries_00f89aa8;
    }
    const std::uint32_t address = reinterpret_cast<std::uint32_t>(entries) + (index << 4) + 0xc;
    void* object;
    std::memcpy(&object, reinterpret_cast<const void*>(address), sizeof(object));
    return object;
}

std::uint16_t handle_from_object_006ad0c0(const void* object) noexcept {
    if (object == nullptr) return 0;
    std::uint16_t result;
    std::memcpy(&result, static_cast<const std::byte*>(object) + 0x174, sizeof(result));
    return result;
}

void* lua_object_to_userdata_00b662d0(GuiLuaHost& host, GuiLuaRef object) {
    return host.to_userdata(object);
}

void* object_from_lua_table_00888aa0(GuiLuaHost& host, GuiLuaRef table) {
    struct Temporary {
        GuiLuaHost& host;
        GuiLuaRef object;
        ~Temporary() { host.release(object); }
    } temporary{host, host.get_by_name(table, "Ptr")};
    // Capture before the temporary's destructor; release may have side effects.
    void* result = lua_object_to_userdata_00b662d0(host, temporary.object);
    return result;
}

void set_object_handle_resolvers_00bd4fc0(ObjectHandleResolverSlots& slots,
    HandleToObjectCallback from_handle, ContextValueCallback from_table,
    ObjectToHandleCallback from_object) noexcept {
    slots.handle_to_object = from_handle;
    slots.context_value = from_table;
    slots.object_to_handle = from_object;
}

void install_object_handle_resolvers_006ad0d0(ObjectHandleResolverSlots& slots) noexcept {
    set_object_handle_resolvers_00bd4fc0(slots, object_from_handle_006ad080,
        object_from_lua_table_00888aa0, handle_from_object_006ad0c0);
}

namespace {
std::int32_t pointer_bits(void* object) noexcept {
    std::int32_t bits;
    static_assert(sizeof(bits) == sizeof(object));
    std::memcpy(&bits, &object, sizeof(bits));
    return bits;
}
}
std::int32_t ObjectHandleReaderResolver::resolve_by_number(std::int32_t id) {
    return pointer_bits(slots_.handle_to_object(static_cast<std::uint32_t>(id), tables_));
}
std::int32_t ObjectHandleReaderResolver::resolve_by_table(const GuiTable*) {
    throw std::invalid_argument("Object Handle resolution requires a live Lua table with native userdata");
}
std::int32_t ObjectHandleReaderResolver::resolve_by_live_table(GuiLuaHost& host, GuiLuaRef table) {
    return pointer_bits(slots_.context_value(host, table));
}
} // namespace bsp
