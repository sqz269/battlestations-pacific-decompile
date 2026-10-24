#include "bsp/native_gui_line_storage.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/gui_widgets_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
using gui_widgets_cc13::GuiTableReader;
using gui_widgets_cc13::GuiReaderHost;
using gui_widgets_cc13::GuiTaggedValue;
using gui_widgets_cc13::gui_line_read_properties_00ac2de0;
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeGuiLineTailStorage) == 0x18);
static_assert(offsetof(NativeGuiLineTailStorage, capacity_100) == 0x14);
void require_storage(void* raw) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % 4)
        throw std::invalid_argument("native Line requires its aligned actual104h storage");
}
volatile Word& word(void* raw, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(raw) + offset);
}
void return_failed_slot(void* slot) noexcept {
    try { free_gui_widget_pool_slot<GuiLinePoolTraits>(slot); }
    catch (...) { std::terminate(); }
}
constexpr Word max_points = 0x1fffffff;
struct PointRange {
    std::byte* first;
    std::byte* last;
    std::byte* end;
    Word size;
    Word capacity;
};
PointRange point_range(const void* raw_vector) {
    if (!raw_vector || reinterpret_cast<std::uintptr_t>(raw_vector) % 4)
        throw std::invalid_argument("Line vector requires its aligned actual EC header");
    auto* const values = static_cast<const volatile Word*>(raw_vector);
    const Word first = values[1], last = values[2], end = values[3];
    if ((!first && (last || end)) || (first && (last < first || end < last)) ||
        ((last - first) & 7) || ((end - first) & 7))
        throw std::invalid_argument("Line stock vector requires its valid actual pointer range");
    const Word size = first ? static_cast<Word>(static_cast<std::int32_t>(last - first) >> 3) : 0;
    const Word capacity = first ? static_cast<Word>(static_cast<std::int32_t>(end - first) >> 3) : 0;
    if (size > max_points || capacity > max_points)
        throw std::length_error("Line stock vector exceeds the native point limit");
    return {reinterpret_cast<std::byte*>(first), reinterpret_cast<std::byte*>(last),
        reinterpret_cast<std::byte*>(end), size, capacity};
}
// 0041C760/00419930/00793500 use FLD/FSTP, not SSE moves or memcpy.
// The pair is POD but native floating exceptions/NaN quieting remain observable.
void copy_pair_x87(void* destination, const void* source) {
    __asm {
        mov eax, destination
        mov edx, source
        fld dword ptr [edx]
        fstp dword ptr [eax]
        fld dword ptr [edx + 4]
        fstp dword ptr [eax + 4]
    }
}
void* allocate_points(Word count) {
    const std::size_t bytes = static_cast<std::size_t>(count) * 8;
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
void store_pointer(void* raw_vector, std::size_t offset, const void* pointer) noexcept {
    word(raw_vector, offset) = reinterpret_cast<Word>(pointer);
}
} // namespace
void NativeGuiLineStockVectorCalls::copy_construct_0079ee20(void* destination,
    const void* source) {
    if (destination == source)
        throw std::invalid_argument("Line vector copy construction requires distinct actual headers");
    const auto range = point_range(source);
    if (!destination || reinterpret_cast<std::uintptr_t>(destination) % 4)
        throw std::invalid_argument("Line vector copy requires aligned actual destination EC");
    word(destination, 4) = 0;
    word(destination, 8) = 0;
    word(destination, 12) = 0;
    if (!range.size) return;
    auto* const allocated = static_cast<std::byte*>(allocate_points(range.size));
    store_pointer(destination, 4, allocated);
    store_pointer(destination, 8, allocated);
    store_pointer(destination, 12, allocated + range.size * 8);
    try {
        for (Word i = 0; i != range.size; ++i)
            copy_pair_x87(allocated + i * 8, range.first + i * 8);
    } catch (...) {
        // 0079EEE4 -> 0041F250 frees current first and clears all three slots.
        void* const current = reinterpret_cast<void*>(word(destination, 4));
        if (current) singleton_lifetime_free(current);
        word(destination, 4) = 0;
        word(destination, 8) = 0;
        word(destination, 12) = 0;
        throw;
    }
    store_pointer(destination, 8, allocated + range.size * 8);
}
void NativeGuiLineStockVectorCalls::append_004222c0(void* actual_vector,
    const void* pair) {
    if (!pair) throw std::invalid_argument("Line append requires its actual two-float source");
    const auto range = point_range(actual_vector);
    if (range.first && range.size < range.capacity) {
        copy_pair_x87(range.last, pair);
        store_pointer(actual_vector, 8, range.last + 8);
        return;
    }
    // 00420B60 snapshots both floats with MOVSS before growth/allocation. Keep
    // the bitwise snapshot so a source alias into the old allocation survives.
    Word captured_pair[2];
    std::memcpy(captured_pair, pair, sizeof(captured_pair));
    if (range.size == max_points)
        throw std::length_error("vector<T> too long");
    Word capacity = max_points - (range.capacity >> 1) < range.capacity
        ? 0 : range.capacity + (range.capacity >> 1);
    if (capacity < range.size + 1) capacity = range.size + 1;
    auto* const allocated = static_cast<std::byte*>(allocate_points(capacity));
    try {
        for (Word i = 0; i != range.size; ++i)
            copy_pair_x87(allocated + i * 8, range.first + i * 8);
        copy_pair_x87(allocated + range.size * 8, captured_pair);
    } catch (...) {
        singleton_lifetime_free(allocated);
        throw;
    }
    if (range.first) singleton_lifetime_free(range.first);
    store_pointer(actual_vector, 12, allocated + capacity * 8);
    store_pointer(actual_vector, 8, allocated + (range.size + 1) * 8);
    store_pointer(actual_vector, 4, allocated);
}
NativeGuiLineTailStorage& native_gui_line_tail(void* raw) {
    require_storage(raw);
    return *reinterpret_cast<NativeGuiLineTailStorage*>(static_cast<std::byte*>(raw) + 0xec);
}
void* construct_native_gui_line_default_00ac2d00(void* raw, const volatile Word& one) {
    require_storage(raw);
    construct_native_gui_widget_base_00aa9390(raw, 4, one);
    word(raw, 0) = 0x00d5c9e8;
    word(raw, 0xf0) = 0;
    word(raw, 0xf4) = 0;
    word(raw, 0xf8) = 0;
    word(raw, 0xfc) = 0;
    word(raw, 0x100) = 4;
    return raw;
}
void* construct_native_gui_line_copy_00ac2d60(void* raw, const void* source,
    NativeGuiLineCopyContext& context, NativeGuiLineCopyAcquired& acquired) {
    require_storage(raw);
    if (!source || acquired.base_returned || acquired.vector_entered || acquired.complete)
        throw std::invalid_argument("native Line copy requires its fresh retained operation and live source");
    construct_native_gui_widget_copy_00aa9520(raw, source, context.base, acquired.base);
    acquired.base_returned = true;
    word(raw, 0) = 0x00d5c9e8;
    acquired.vector_entered = true;
    try {
        context.vectors.copy_construct_0079ee20(static_cast<std::byte*>(raw) + 0xec,
            static_cast<const std::byte*>(source) + 0xec);
    } catch (...) {
        acquired.base_cleanup_entered = true;
        try { destroy_native_gui_widget_base_00aa9730(raw, context.lifetime); }
        catch (...) { std::terminate(); }
        throw;
    }
    acquired.complete = true;
    return raw;
}
void* create_native_gui_line_default_00aa1530(const volatile Word& one) {
    void* const slot = allocate_gui_widget_pool_slot<GuiLinePoolTraits>();
    if (!slot) return nullptr;
    try { return construct_native_gui_line_default_00ac2d00(slot, one); }
    catch (...) { return_failed_slot(slot); throw; }
}
void* create_native_gui_line_copy_00aa1530(const void* source,
    NativeGuiLineCopyContext& context, NativeGuiLineCopyAcquired& acquired) {
    if (!source) throw std::invalid_argument("Line copy arm requires its captured nonnull native source");
    const void* const captured_source = source;
    void* const slot = allocate_gui_widget_pool_slot<GuiLinePoolTraits>();
    if (!slot) return nullptr;
    try { return construct_native_gui_line_copy_00ac2d60(slot, captured_source, context, acquired); }
    catch (...) { return_failed_slot(slot); throw; }
}
void destroy_native_gui_line_00ac28d0(void* raw, NativeGuiWidgetLifetimeContext& context) {
    require_storage(raw);
    word(raw, 0) = 0x00d5c9e8;
    void* const current_points = reinterpret_cast<void*>(word(raw, 0xf0));
    if (current_points) singleton_lifetime_free(current_points);
    word(raw, 0xf0) = 0;
    word(raw, 0xf4) = 0;
    word(raw, 0xf8) = 0;
    destroy_native_gui_widget_base_00aa9730(raw, context);
}
void* delete_native_gui_line_00ac2ca0(void* raw, const volatile Word& flags,
    NativeGuiWidgetLifetimeContext& context) {
    destroy_native_gui_line_00ac28d0(raw, context);
    const auto low = *reinterpret_cast<const volatile unsigned char*>(&flags);
    if ((low & 1u) != 0) free_gui_widget_pool_slot<GuiLinePoolTraits>(raw);
    return raw;
}
bool native_gui_line_is_kind_of_00ac2920(Word descriptor,
    const volatile Word (&lineage)[3]) noexcept {
    for (const volatile Word* current = lineage; current != lineage + 3; ++current)
        if (*current == descriptor) return true;
    return false;
}
void append_native_gui_line_point_00ac2dc0(void* raw, const void* pair,
    NativeGuiLineVectorCalls& vectors) {
    require_storage(raw);
    vectors.append_004222c0(static_cast<std::byte*>(raw) + 0xec, pair);
}
void read_native_gui_line_properties_00ac2de0(void* raw,
    GuiTableReader& reader, GuiReaderHost& host) {
    gui_line_read_properties_00ac2de0(raw, reader, host);
}
void read_native_gui_line_points_after_base_00ac2de0(void* raw,
    GuiTableReader& reader, NativeGuiLineVectorCalls& vectors) {
    require_storage(raw);
    if (!reader.has("Points")) return;
    reader.open("Points");
    for (std::int32_t key = 1; reader.has_index(key); ++key) {
        GuiTaggedValue value;
        value.tag = GuiValueTag::Vec2;
        reader.read_index(key, value);
        Word pair[2];
        std::memcpy(pair, value.f.data(), sizeof(pair)); // Native MOVSS pair spills, no x87 conversion.
        vectors.append_004222c0(static_cast<std::byte*>(raw) + 0xec, pair);
    }
    reader.close();
}
} // namespace bsp
