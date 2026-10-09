#include "bsp/native_damageable_class_mesh_fragment.hpp"
#include <cstring>

namespace bsp {
namespace {
template<class T> T read_header(const void* header, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(header) + offset, sizeof value);
    return value;
}

// State 1 belongs to the parent. This guard owns only states 2 and 3; it never
// destroys the borrowed Unique object or restores a copied Lua index/header.
struct MeshCleanup {
    NativeDamageableClassMeshFragmentScratch& scratch;
    NativeStringRawPoolContext& strings;
    unsigned state = 1;

    void release_string() {
        state = 2; // 0087CB9D, before getter/return can throw
        destroy_native_string_header_0041dd20(scratch.fresh_string_at_parent_10, strings);
    }
    void release_field() {
        state = 1; // 0087CBC2, before Lua tracked-index cleanup
        destroy_native_lua_object_00b67700(
            *static_cast<NativeLuaObjectStorage*>(scratch.fresh_field_at_parent_44));
    }
    ~MeshCleanup() noexcept {
        if (state == 3) release_string();
        if (state == 2) release_field();
    }
};
} // namespace

void read_native_damageable_class_mesh_fragment_0087cb44(
    void* descriptor, NativeLuaObjectStorage& row,
    NativeDamageableClassMeshFragmentScratch& scratch,
    NativeStringRawPoolContext& strings) {
    // Unique remains the parent's real registered object. The Lua provider's
    // actual owner table, not a cached copy here, updates any shifted index.
    MeshCleanup cleanup{scratch, strings};
    auto* field = native_lua_get_by_name_protected(row, scratch.fresh_field_at_parent_44, "Mesh");
    cleanup.state = 2; // 0087CB55

    // Compose B685C0's existing exact-string predicate/getter and raw 41E870.
    // Its old NativeStringStorage overload imposes noexcept release; the raw
    // header constructor preserves current-pool getter exceptions instead.
    const char* text = "";
    if (native_lua_is_string_00b660a0(*field)) text = native_lua_string_00b662b0(*field);
    void* const temporary = construct_native_string_header_0041e870(
        scratch.fresh_string_at_parent_10, strings, text);
    auto* const destination = static_cast<std::byte*>(descriptor) + 0x38;
    cleanup.state = 3; // 0087CB69
    if (destination != temporary) {
        resize_native_string_header_0041dd40(destination, strings,
            read_header<std::uint32_t>(temporary, 0), true);
        if (read_header<std::uint32_t>(temporary, 0) != 0) {
            const auto bytes = read_header<std::uint32_t>(destination, 0);
            const void* const source = read_header<const void*>(temporary, 4);
            void* const target = read_header<void*>(destination, 4);
            // Existing BF7680 source boundary: overlap admitted; a zero-byte
            // call is omitted instead of passing possibly-null C++ pointers.
            if (bytes != 0) std::memmove(target, source, bytes);
        }
    }
    cleanup.release_string();
    cleanup.release_field();
}
} // namespace bsp
