#include "bsp/native_stream_copy_range.hpp"

#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_retained_memory_owners.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual stream range copy requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::uint32_t word(const void* owner, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(owner) + offset);
}
void* pointer(const void* owner, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(owner, offset));
}
} // namespace

void* copy_native_stream_range_00bef840(void* source,
    std::uint32_t offset_low, std::uint32_t offset_high,
    std::uint32_t length_low, std::uint32_t unused_length_high,
    NativeRetainedMemoryOwnerContext& memory, NativeAdoptedSubstreamDispatch& streams) {
    (void)unused_length_high;
    const auto seek_entry = word(pointer(source), 0x1c);
    (void)streams.source_seek(seek_entry, source, offset_low, offset_high, 0);

    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x10, 0x10});
    void* backing = nullptr;
    try {
        if (allocation) backing = construct_native_memory_backing_008d43c0(
            allocation, static_cast<std::int32_t>(length_low), memory);
    } catch (...) {
        // CC7670..CC767A: free the captured allocation from native [EBP+4].
        singleton_lifetime_free(allocation);
        throw;
    }
    const void* const read_table = pointer(source);
    void* const destination = pointer(backing, 8);
    const auto read_entry = word(read_table, 0x24);
    streams.source_read(read_entry, source, destination, length_low, nullptr);

    void* const result = create_native_memory_stream_from_backing_00bef6d0(backing, memory);
    auto* const references = reinterpret_cast<volatile LONG*>(
        reinterpret_cast<std::uintptr_t>(backing) + 4u);
    if (InterlockedDecrement(references) == 0) {
        const auto table = word(backing);
        const auto entry = word(reinterpret_cast<const void*>(table));
        streams.source_zero_reference(entry, backing, table);
    }
    return result;
}
} // namespace bsp
