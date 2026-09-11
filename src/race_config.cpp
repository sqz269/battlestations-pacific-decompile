#include "bsp/race_config.hpp"

#include "bsp/gui_lua_runtime.hpp"
#include "bsp/lua_numeric.hpp"

#include <cstddef>
#include <cstring>
#include <new>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Race configuration reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
constexpr std::uint32_t maximum_count = 0x3fffffff;
static_assert(sizeof(RaceRecord) == 0x20 && offsetof(RaceRecord, name_08) == 8
    && offsetof(RaceRecord, color_10) == 0x10, "Native Win32 race record fields");
std::uintptr_t address(const void* pointer) noexcept {
    return reinterpret_cast<std::uintptr_t>(pointer);
}
std::uint32_t distance(void* const* first, void* const* last) noexcept {
    return static_cast<std::uint32_t>(
        static_cast<std::int32_t>(address(last) - address(first)) >> 2);
}
void** offset(void** pointer, std::uint32_t count) noexcept {
    return reinterpret_cast<void**>(address(pointer) + count * 4u);
}
std::uint32_t count(const RaceRecordTable& table) noexcept {
    return table.begin ? distance(table.begin, table.end) : 0;
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}
//007FFAB0 uses memmove_s for nonzero byte count, including overlapping ranges.
// These plain pointer cells have no pointee copy/destructor/refcount semantics.
void** copy_slots(void** first, void** last, void** destination) {
    const auto bytes = distance(first, last) * 4u;
    if (bytes) (void)memmove_s(destination, bytes, first, bytes);
    return reinterpret_cast<void**>(address(destination) + bytes);
}
void** fill_slots(void** first, std::uint32_t number, void* const* fill) {
    auto** current = first;
    for (std::uint32_t i = 0; i != number; ++i) {
        *current = *fill; //007FFDD0 reloads the source cell each iteration
        current = offset(current, 1);
    }
    return offset(first, number);
}
void fill_range(void** first, void** last, void* const* fill) {
    for (auto** current = first; current != last; current = offset(current, 1))
        *current = *fill; //007FF7F0
}
// Pointer-container behavior reached by00800090. The standard CRT owns
// allocation/memmove/length_error; no replacement CRT or object ownership.
void insert_slots(RaceRecordTable& table, void** position,
    std::uint32_t number, void* const* source)
{
    void* captured = *source; //007FFEC0 captures before inspecting the container
    const auto capacity = table.begin ? distance(table.begin, table.capacity_end) : 0;
    if (!number) return;
    if (maximum_count - count(table) < number)
        throw std::length_error("vector<T> too long"); //007FFE00
    if (capacity < count(table) + number) {
        auto grown = maximum_count - (capacity >> 1u) < capacity
            ? 0u : capacity + (capacity >> 1u);
        if (grown < count(table) + number) grown = count(table) + number;
        //007FF340 rejects multiplication overflow, then allocates4*count.
        if (grown > maximum_count) throw std::bad_alloc();
        auto** replacement = static_cast<void**>(singleton_lifetime_allocate({
            SingletonAllocationKind::pointer_slots, grown * 4u, grown * sizeof(void*)}));
        // Allocation/new-handler can have changed the global header. Preserve
        // the native rereads and the already-captured insertion position.
        auto** after_prefix = copy_slots(table.begin, position, replacement);
        auto** after_fill = fill_slots(after_prefix, number, &captured);
        (void)copy_slots(position, table.end, after_fill);
        auto** old_begin = table.begin;
        const auto new_count = number + (old_begin ? distance(old_begin, table.end) : 0);
        if (old_begin) singleton_lifetime_free(old_begin);
        // Recovered continuation after007FFFBC, despite its saved false
        // CALL_RETURN annotation: free precedes all three header stores.
        table.begin = replacement;
        table.capacity_end = offset(replacement, grown);
        table.end = offset(replacement, new_count);
        return;
    }
    auto** captured_end = table.end;
    const auto tail = distance(position, captured_end);
    if (tail < number) {
        (void)copy_slots(position, captured_end, offset(position, number));
        (void)fill_slots(table.end, number - distance(position, table.end), &captured);
        table.end = offset(table.end, number);
        fill_range(position, offset(table.end, 0u - number), &captured);
    } else {
        auto** split = offset(captured_end, 0u - number);
        table.end = copy_slots(split, captured_end, captured_end);
        //007FF810 is copy_backward via memmove_s, destination=end-size.
        const auto moving = distance(position, split);
        if (static_cast<std::int32_t>(moving) > 0)
            (void)copy_slots(position, split, offset(captured_end, 0u - moving));
        fill_range(position, offset(position, number), &captured);
    }
}
struct OwnedString {
    NativeStringStorage& strings;
    NativeString value;
    ~OwnedString() { destroy_native_string_header_0041dd20(&value, strings); }
};
struct OwnedRef {
    GuiLuaHost& host;
    GuiLuaRef ref;
    ~OwnedRef() { host.release(ref); }
};
struct Iteration {
    GuiLua51Host& host;
    GuiLuaRef table;
    GuiLuaRef key{};
    GuiLuaRef value{};
    ~Iteration() { host.release(value); host.release(key); }
    bool next(bool restart) {
        host.release(value); value = {};
        host.release(key); key = {};
        return host.next(table, key, value, restart);
    }
};
} // namespace

RaceRecord::RaceRecord(const RaceRecordAllocationWords& words) noexcept
    : index_04(words.index_word_04)
{
    for (std::size_t i = 0; i != 4; ++i)
        std::memcpy(&color_10[i], &words.color_words_10[i], sizeof(float));
}
RaceRecord& construct_race_record_007ff9d0(RaceRecord& record) {
    record.native_vtable_00 = 0x00d08d20;
    new (&record.name_08) NativeString;
    return record;
}
void read_race_record_007ffc00(RaceRecord& record, GuiLuaHost& host,
    GuiLuaRef row, NativeStringStorage& strings)
{
    {
        OwnedRef name{host, host.get_by_name(row, "Name")};
        const char* captured = host.to_string(name.ref);
        const auto length = captured ? static_cast<std::uint32_t>(std::strlen(captured)) : 0;
        record.name_08.resize_0041dd40(strings, length, false);
        if (record.name_08.data() && record.name_08.length())
            std::memcpy(record.name_08.data(), captured, record.name_08.length());
    }
    OwnedRef color{host, host.get_by_name(row, "Color")};
    for (std::int32_t lane = 1; lane <= 3; ++lane) {
        OwnedRef value{host, host.get_by_index(color.ref, lane)};
        record.color_10[static_cast<std::size_t>(lane - 1)] =
            lua_object_number_00b66270(host, value.ref);
    }
    record.color_10[3] = 1.0f; //00D7A24C; after lane3 release, before Color release
}
void destroy_race_record_007ff9f0(RaceRecord& record, NativeStringStorage& strings) {
    record.native_vtable_00 = 0x00d08d20;
    destroy_native_string_header_0041dd20(&record.name_08, strings);
}
RaceRecord* scalar_delete_race_record_007ffb10(RaceRecord* record,
    std::uint32_t flags, NativeStringStorage& strings)
{
    destroy_race_record_007ff9f0(*record, strings);
    if (flags & 1u) {
        record->~RaceRecord(); // members have no implicit NativeString cleanup
        singleton_lifetime_free(record);
    }
    return record;
}
void resize_race_record_table_00800090(RaceRecordTable& table,
    std::uint32_t requested, RaceRecord* fill, const SingletonLifetimeCallbacks& callbacks)
{
    auto** initial_begin = table.begin;
    const auto initial_count = initial_begin ? distance(initial_begin, table.end) : 0;
    if (initial_count < requested) {
        const auto retained_count = initial_begin ? distance(initial_begin, table.end) : 0;
        auto** position = table.end;
        if (address(initial_begin) > address(position)) invalid(callbacks);
        void* value = fill;
        insert_slots(table, position, requested - retained_count, &value);
        return;
    }
    if (!initial_begin) return;
    auto** captured_end = table.end;
    if (requested >= distance(initial_begin, captured_end)) return;
    if (address(initial_begin) > address(captured_end)) invalid(callbacks);
    auto** retained_begin = table.begin;
    if (address(retained_begin) > address(table.end)) invalid(callbacks);
    auto** first = offset(retained_begin, requested);
    if (address(first) > address(table.end) || address(first) < address(table.begin))
        invalid(callbacks);
    //007FF910 receives this owner's two checked iterators. Erase copies any
    // suffix after captured_end (normally empty), updates end, never deletes.
    if (first != captured_end) {
        const auto remaining = distance(captured_end, table.end);
        if (static_cast<std::int32_t>(remaining) > 0)
            (void)copy_slots(captured_end, table.end, first);
        table.end = offset(first, remaining);
    }
}
void load_race_config_00800160(RaceRecordTable& table, LuaStateOwnerEnvironment environment,
    LuaScriptRuntime& scripts, RaceConfigContext& context)
{
    PcStorageLuaOwner lua(std::move(environment));
    lua.open_storage_archive_00b6a020(1);
    {
        OwnedString path{context.strings, {}};
        path.value.assign_0041e870(context.strings, "Scripts\\datatables\\Races.lua");
        (void)scripts.run_file(lua.storage_lua_38(), path.value.data(), false);
    }
    {
        GuiLua51Host host(*lua.storage_lua_38());
        OwnedRef globals{host, host.globals()};
        OwnedRef races{host, host.get_by_name(globals.ref, "Races")};
        Iteration iterator{host, races.ref};
        for (bool more = iterator.next(true); more; more = iterator.next(false)) {
            const auto index = static_cast<std::uint32_t>(lua_object_integer_00b66290(
                host, iterator.key, context.crt_sse2_conversion));
            if (!table.begin || count(table) <= index)
                resize_race_record_table_00800090(table, index + 1u, nullptr, context.validation);
            void* allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x20, sizeof(RaceRecord)});
            auto* record = new (allocation) RaceRecord(context.allocation_words);
            construct_race_record_007ff9d0(*record); // inlined008002C7..D0
            record->index_04 = index;
            // Native reloads the global header after allocation, and again
            // after a returning invalid-parameter callback. Do not use at().
            auto** current_begin = table.begin;
            if (!current_begin || index >= distance(current_begin, table.end)) {
                invalid(context.validation);
                current_begin = table.begin;
            }
            *offset(current_begin, index) = record; // no disposal of old pointer
            // Fresh canonical vtable00D08D20 slot+4 is007FFC00. No callback can
            // replace it on the valid path before this native dispatch.
            read_race_record_007ffc00(*record, host, iterator.value, context.strings);
        }
    } // iterator value/key, Races, globals; then close owner
    lua.close_storage_archive_00b65e80();
}
void release_race_record_table_storage(RaceRecordTable& table) noexcept {
    singleton_lifetime_free(table.begin);
    table.begin = table.end = table.capacity_end = nullptr;
}
} // namespace bsp
