#include "bsp/native_input_configuration_modifiers.hpp"
#include "bsp/native_input_backend_bindings.hpp"
#include "bsp/native_input_configuration_storage.hpp"
#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>
#include <new>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = std::uint32_t;
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word& word(Word p, Word offset = 0) noexcept { return *reinterpret_cast<Word*>(p + offset); }
Word read(Word p, Word offset = 0) noexcept { return *reinterpret_cast<volatile Word*>(p + offset); }
Word distance(Word first, Word last, unsigned shift) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(last - first) >> shift);
}
Word count(Word first, Word last, unsigned shift) noexcept { return first ? distance(first, last, shift) : 0; }
void invalid() { _invalid_parameter_noinfo(); }
void empty_row(Word row) noexcept { word(row, 4) = 0; word(row, 8) = 0; word(row, 0xc) = 0; }
void release_row(Word row) noexcept {
    if (const auto data = read(row, 4)) singleton_lifetime_free(pointer(data));
}
void copy_row(Word output, Word source) {
    //557590 contract: count is captured before zeroing the fresh header.
    const auto source_begin = read(source, 4);
    const auto required = count(source_begin, read(source, 8), 2);
    empty_row(output);
    if (!required) return;
    if (required > 0x3fffffffu) native_singleton_length_error_00bd0590();
    const auto bytes = required * 4u;
    const auto data = address(singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes}));
    word(output, 4) = data;
    word(output, 8) = data;
    word(output, 0xc) = data + bytes;
    try {
        const auto last = read(source, 8);
        if (read(source, 4) > last) invalid();
        const auto first = read(source, 4);
        if (first > read(source, 8)) invalid();
        const auto copy_bytes = distance(first, last, 2) * 4u;
        if (copy_bytes) (void)memmove_s(pointer(data), copy_bytes, pointer(first), copy_bytes);
        word(output, 8) = data + copy_bytes;
    } catch (...) { release_row(output); throw; }
}
void release_rows(Word first, Word last) noexcept {
    for (; first != last; first += 0x10u) { release_row(first); empty_row(first); }
}
struct RowCopy {
    Word storage[4];
    explicit RowCopy(Word source) { copy_row(address(storage), source); }
    ~RowCopy() { release_row(address(storage)); }
};
struct EmptyRow {
    Word storage[4];
    EmptyRow() { empty_row(address(storage)); }
    ~EmptyRow() { release_row(address(storage)); }
};
void insert_row(Word header, Word position, Word value) {
    // Existing library insertion contract specialized to a single owning row.
    // Snapshot the value before allocation/relocation to support aliases.
    RowCopy retained(value);
    const auto first = read(header, 4);
    const auto capacity = count(first, read(header, 0xc), 4);
    constexpr Word maximum = 0x0fffffffu;
    if (maximum - count(first, read(header, 8), 4) < 1u)
        native_singleton_length_error_00bd0590();
    if (capacity < count(first, read(header, 8), 4) + 1u) {
        auto grown = maximum - (capacity >> 1) < capacity ? 0u : capacity + (capacity >> 1);
        const auto minimum = count(first, read(header, 8), 4) + 1u;
        if (grown < minimum) grown = count(first, read(header, 8), 4) + 1u;
        if (grown > maximum) throw std::bad_alloc();
        const auto bytes = grown * 0x10u;
        const auto replacement = address(singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes}));
        auto end = replacement;
        try {
            for (auto row = read(header, 4); row != position; row += 0x10u, end += 0x10u) copy_row(end, row);
            copy_row(end, address(retained.storage));
            end += 0x10u;
            const auto last = read(header, 8);
            for (auto row = position; row != last; row += 0x10u, end += 0x10u) copy_row(end, row);
        } catch (...) {
            release_rows(replacement, end);
            singleton_lifetime_free(pointer(replacement));
            throw;
        }
        const auto old_first = read(header, 4);
        const auto resulting_count = count(old_first, read(header, 8), 4) + 1u;
        if (old_first) {
            release_rows(old_first, read(header, 8));
            singleton_lifetime_free(pointer(read(header, 4)));
        }
        word(header, 0xc) = replacement + bytes;
        word(header, 8) = replacement + resulting_count * 0x10u;
        word(header, 4) = replacement;
    } else {
        // The returning diagnostic may have repaired capacity after the
        // push_back wrapper captured its insertion position.
        const auto end = read(header, 8);
        if (position == end) {
            copy_row(end, address(retained.storage));
            word(header, 8) = read(header, 8) + 0x10u;
        } else {
            copy_row(end, end - 0x10u);
            word(header, 8) = end + 0x10u;
            for (auto row = end - 0x10u; row != position; row -= 0x10u)
                assign_input_checked_word_storage(pointer(row), pointer(row - 0x10u));
            assign_input_checked_word_storage(pointer(position), retained.storage);
        }
    }
}
// Ownership begins only after each native constructor returns. Release clears
// the source guard first, matching native normal-path EH state transitions.
struct LuaObject {
    NativeLuaObjectStorage value;
    bool owned = false;
    void release() { if (owned) { owned = false; destroy_native_lua_object_00b67700(value); } }
    ~LuaObject() noexcept(false) { release(); }
    void construct() { construct_native_lua_object_00b65f50(&value); owned = true; }
    void named(NativeLuaObjectStorage& table, const char* name) {
        native_lua_get_by_name_00b67800(table, &value, name); owned = true;
    }
    void indexed(NativeLuaObjectStorage& table, std::int32_t index) {
        native_lua_get_by_index_00b67720(table, &value, index); owned = true;
    }
};
Word checked_last_row(Word header) {
    const auto captured_end = read(header, 8);
    if (read(header, 4) > captured_end) invalid();
    const auto row = captured_end - 0x10u;
    if (row > read(header, 8) || row < read(header, 4)) invalid();
    if (row >= read(header, 8)) invalid();
    return row;
}
} // namespace

void append_input_checked_word_storage(void* header, const void* value) {
    const auto h = address(header), first = read(h, 4);
    const auto size = count(first, read(h, 8), 2);
    if (first && size < distance(first, read(h, 0xc), 2)) {
        const auto end = read(h, 8);
        if (end) word(end) = read(address(value));
        word(h, 8) = end + 4u;
        return;
    }
    const auto end = read(h, 8);
    if (first > end) invalid();
    Word iterator[2];
    insert_input_active_pointer_storage(iterator, header, pointer(end), value);
}

void append_input_checked_row_storage(void* header, const void* value) {
    const auto h = address(header), first = read(h, 4);
    const auto size = count(first, read(h, 8), 4);
    if (first && size < distance(first, read(h, 0xc), 4)) {
        const auto end = read(h, 8);
        if (end) copy_row(end, address(value));
        word(h, 8) = end + 0x10u;
        return;
    }
    const auto end = read(h, 8);
    if (first > end) invalid();
    const auto initial_first = read(h, 4);
    Word index = 0;
    if (count(initial_first, read(h, 8), 4)) {
        if (initial_first > read(h, 8)) invalid();
        if (!h) invalid();
        index = distance(initial_first, end, 4);
    }
    insert_row(h, end, address(value));
    const auto result_begin = read(h, 4);
    if (result_begin > read(h, 8)) invalid();
    const auto result = result_begin + index * 0x10u;
    if (result > read(h, 8) || result < read(h, 4)) invalid();
}

void populate_native_input_configuration_modifiers_00698b64(
    void* configuration, NativeLuaObjectStorage& globals, const bool& conversion) {
    const auto base = address(configuration), pairs = base + 0x4d0u;
    const auto first = read(pairs, 4);
    if (first && distance(first, read(pairs, 8), 4) != 0) return;
    LuaObject modifiers; modifiers.named(globals, "InputModifiers");
    LuaObject table; table.named(modifiers.value, "SwapStickPairs");
    LuaObject key; key.construct();
    LuaObject value; value.construct();
    native_lua_iterate_first_00b67080(table.value, key.value, value.value);
    while (!native_lua_is_unbound_00b66420(key.value)) {
        {
            EmptyRow empty;
            append_input_checked_row_storage(pointer(pairs), empty.storage);
        }
        for (std::int32_t index = 1; index != 3; ++index) {
            LuaObject item; item.indexed(value.value, index);
            const auto integer = native_lua_integer_00b66290(item.value, conversion);
            append_input_checked_word_storage(pointer(checked_last_row(pairs)), &integer);
            item.release();
        }
        native_lua_iterate_next_00b67190(table.value, key.value, value.value);
    }
    const char* names[] = {"SwapStickGeneral", "SwapStickMap", "InvertCameraY", "InvertPlaneY"};
    const Word offsets[] = {0x500, 0x510, 0x4e0, 0x4f0};
    for (unsigned group = 0; group != 4; ++group) {
        LuaObject next; next.named(modifiers.value, names[group]);
        assign_native_lua_object_00b67690(table.value, next.value);
        next.release();
        native_lua_iterate_first_00b67080(table.value, key.value, value.value);
        while (!native_lua_is_unbound_00b66420(key.value)) {
            const auto integer = native_lua_integer_00b66290(value.value, conversion);
            append_input_checked_word_storage(pointer(base + offsets[group]), &integer);
            native_lua_iterate_next_00b67190(table.value, key.value, value.value);
        }
    }
    value.release(); key.release(); table.release(); modifiers.release();
}

NativeLuaObjectStorage* load_native_input_configuration_modifiers_prefix_00698a10(
    void* configuration, void* fresh_globals, NativeInputConfigurationLoadServices& services, const bool& conversion) {
    load_native_input_configuration_script_prefix_00698a10(configuration, services);
    auto* globals = native_lua_globals_00b67980(*static_cast<NativeLuaStateStorage*>(configuration), fresh_globals);
    try { populate_native_input_configuration_modifiers_00698b64(configuration, *globals, conversion); }
    catch (...) { destroy_native_lua_object_00b67700(*globals); throw; }
    return globals;
}
} // namespace bsp
