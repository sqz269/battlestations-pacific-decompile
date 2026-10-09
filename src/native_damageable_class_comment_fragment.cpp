#include "bsp/native_damageable_class_comment_fragment.hpp"
#include <cstring>

namespace bsp {
namespace {
template<class T> T read_header(const void* header, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(header) + offset, sizeof value);
    return value;
}

// State 1 belongs to the parent. This guard owns only states 4 and 5; it never
// destroys borrowed Unique or restores a copied Lua index/string header.
struct CommentCleanup {
    NativeDamageableClassCommentFragmentScratch& scratch;
    NativeStringRawPoolContext& strings;
    unsigned state = 1;

    void release_string() {
        state = 4; // 0087CC34, before getter/return can throw
        destroy_native_string_header_0041dd20(scratch.fresh_string_at_parent_10, strings);
    }
    void release_field() {
        state = 1; // 0087CC59, before Lua tracked-index cleanup
        destroy_native_lua_object_00b67700(
            *static_cast<NativeLuaObjectStorage*>(scratch.fresh_field_at_parent_44));
    }
    ~CommentCleanup() noexcept {
        if (state == 5) release_string();
        if (state == 4) release_field();
    }
};
} // namespace

void read_native_damageable_class_comment_fragment_0087cbcf(
    void* descriptor, NativeLuaObjectStorage& row,
    NativeDamageableClassCommentFragmentScratch& scratch,
    NativeStringRawPoolContext& strings) {
    // The real owner table retains Unique and updates any shifted indices.
    CommentCleanup cleanup{scratch, strings};
    auto* field = native_lua_get_by_name_protected(row, scratch.fresh_field_at_parent_44, "Comment");
    cleanup.state = 4; // 0087CBEC

    // Compose the existing B685C0 exact-string predicate/getter and raw41E870;
    // avoid the older NativeStringStorage overload's noexcept release boundary.
    const char* text = "";
    if (native_lua_is_string_00b660a0(*field)) text = native_lua_string_00b662b0(*field);
    void* const temporary = construct_native_string_header_0041e870(
        scratch.fresh_string_at_parent_10, strings, text);
    auto* const destination = static_cast<std::byte*>(descriptor) + 0x58;
    cleanup.state = 5; // 0087CC00
    if (destination != temporary) {
        resize_native_string_header_0041dd40(destination, strings,
            read_header<std::uint32_t>(temporary, 0), true);
        if (read_header<std::uint32_t>(temporary, 0) != 0) {
            const auto bytes = read_header<std::uint32_t>(destination, 0);
            void* const target = read_header<void*>(destination, 4);
            const void* const source = read_header<const void*>(temporary, 4);
            // Existing BF7680 boundary: allow overlap; omit a zero-byte call
            // rather than passing possibly-null pointers to the host CRT.
            if (bytes != 0) std::memmove(target, source, bytes);
        }
    }
    cleanup.release_string();
    cleanup.release_field();
}
} // namespace bsp
