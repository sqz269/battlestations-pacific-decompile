#include "bsp/native_mpak_entry.hpp"

#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_filestore_open.hpp"
#include "bsp/native_mpak_provider.hpp"
#include "bsp/native_raw_inflate_owner.hpp"
#include "bsp/native_stream_copy_range.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual MPAK entry materialization requires MSVC Win32.
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
void put(void* owner, std::uint32_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(owner) + offset) = value;
}
} // namespace

void* materialize_native_mpak_entry_00bb5080(void* owner,
    std::int32_t file_index, NativeMpakEntryContext& context) {
    if (file_index < 0) return nullptr;
    const auto index = static_cast<std::uint32_t>(file_index);
    const auto begin = word(owner, 0x20);
    if (!begin || index >= static_cast<std::uint32_t>(
            static_cast<std::int32_t>(word(owner, 0x24) - begin) / 0x24))
        context.containers.invalid_parameter_00bf6713();
    void* const record = reinterpret_cast<void*>(word(owner, 0x20) + index * 0x24u);
    void* const position_source = pointer(owner, 0x14);
    put(owner, 0x40, index);
    const auto position_entry = word(pointer(position_source), 0x20);
    const auto position = static_cast<std::uint32_t>(
        context.position.source_position(position_entry, position_source));

    const auto offsets_begin = word(record, 0x18);
    if (!offsets_begin ||
        (static_cast<std::int32_t>(word(record, 0x1c) - offsets_begin) >> 2) == 0)
        context.containers.invalid_parameter_00bf6713();
    auto iterator = word(record, 0x18);
    const auto first_offset = word(reinterpret_cast<const void*>(iterator));
    if (iterator > word(record, 0x1c))
        context.containers.invalid_parameter_00bf6713();
    auto selected = first_offset;
    for (;;) {
        const auto captured_end = word(record, 0x1c);
        if (word(record, 0x18) > captured_end)
            context.containers.invalid_parameter_00bf6713();
        // BB5141 CMP ESI,ESI makes the BB5145 validation call unreachable.
        if (iterator == captured_end) break;
        if (iterator >= word(record, 0x1c))
            context.containers.invalid_parameter_00bf6713();
        if (word(reinterpret_cast<const void*>(iterator)) >= position) {
            if (iterator >= word(record, 0x1c))
                context.containers.invalid_parameter_00bf6713();
            selected = word(reinterpret_cast<const void*>(iterator));
            break;
        }
        if (iterator >= word(record, 0x1c))
            context.containers.invalid_parameter_00bf6713();
        iterator += 4;
    }
    const auto compressed = *reinterpret_cast<const volatile std::uint8_t*>(
        reinterpret_cast<std::uintptr_t>(record) + 0x10u);
    const auto decoded_length = word(record, 8);
    if (!compressed) {
        return copy_native_stream_range_00bef840(pointer(owner, 0x14),
            selected, 0, decoded_length, 0, context.conversion.memory_owners,
            context.streams);
    }
    const std::uint32_t descriptor[3] = {selected, word(record, 0xc), decoded_length};
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x34, 0x34});
    void* inflater = nullptr;
    try {
        if (allocation) inflater = construct_native_raw_inflate_owner_00bbc1d0(
            allocation, pointer(owner, 0x14), descriptor, 0x4000, 0x10000,
            context.streams);
    } catch (...) {
        // CC4360..CC436A: state0 frees only the raw34h allocation.
        singleton_lifetime_free(allocation);
        throw;
    }
    void* const result = convert_native_stored_stream_00bef750(inflater, context.conversion);
    auto* const references = reinterpret_cast<volatile LONG*>(
        reinterpret_cast<std::uintptr_t>(inflater) + 4u);
    if (InterlockedDecrement(references) == 0) {
        const auto table = word(inflater);
        const auto entry = word(reinterpret_cast<const void*>(table));
        context.streams.source_zero_reference(entry, inflater, table);
    }
    return result;
}
} // namespace bsp
