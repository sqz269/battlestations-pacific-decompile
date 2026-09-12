#include "bsp/native_physical_pending_records.hpp"

#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <initializer_list>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

void* at(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
std::uint32_t word(const void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(base, offset));
}
void put(void* base, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(base, offset)) = value;
}
void* pointer(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof result);
    return result;
}
std::int32_t signed_word(const void* base, std::uint32_t offset) noexcept {
    return signed_bits(word(base, offset));
}
void construct_name(void* destination, const void* source,
    NativeStringStorage& strings) {
    const bool identical = destination == source;
    put(destination, 0, 0);
    put(destination, 4, 0);
    if (identical) return;
    resize_native_string_header_0041dd40(destination, strings, word(source), true);
    if (word(source) != 0) {
        const auto length = word(destination);
        const auto* const source_data = pointer(source, 4);
        auto* const destination_data = pointer(destination, 4);
        // BF7680 copies backward for overlap. Zero bytes perform no access;
        // retain the native header read order before omitting that CRT call.
        if (length != 0) std::memmove(destination_data, source_data, length);
    }
}
} // namespace

void destroy_native_physical_pending_names_00bf3880(void* record,
    NativeStringStorage& strings) noexcept {
    destroy_native_string_header_0041dd20(at(record, 0x28), strings);
    destroy_native_string_header_0041dd20(at(record, 0x20), strings);
}

void* copy_construct_native_physical_pending_record_00bf3c10(
    void* destination, const void* source, NativeStringStorage& strings) {
    for (const auto offset : {0u, 4u, 8u, 0xcu, 0x10u, 0x18u, 0x1cu})
        put(destination, offset, word(source, offset));
    construct_name(at(destination, 0x20), at(source, 0x20), strings);
    try {
        construct_name(at(destination, 0x28), at(source, 0x28), strings);
    } catch (...) {
        // E02880 state0 -> CC7B10 ->41DD20(destination+20h).
        destroy_native_string_header_0041dd20(at(destination, 0x20), strings);
        throw;
    }
    put(destination, 0x30, word(source, 0x30));
    return destination;
}

void reserve_native_physical_pending_records_00bf3da0(void* queue,
    std::int32_t capacity, NativeStringStorage& strings) {
    if (capacity < 1) capacity = 1;
    if (signed_word(queue, 8) >= capacity) return;
    const std::uint32_t bytes = static_cast<std::uint32_t>(capacity) * 0x38u;
    void* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes});
    for (std::uint32_t index = 0; signed_bits(index) < signed_word(queue, 4); ++index) {
        const auto offset = index * 0x38u;
        void* const destination = at(replacement, offset);
        if (destination != nullptr)
            copy_construct_native_physical_pending_record_00bf3c10(
                destination, at(pointer(queue), offset), strings);
        // E028AC state0 -> CC7B30 calls RET401130: no replacement cleanup.
    }
    for (std::uint32_t index = 0; signed_bits(index) < signed_word(queue, 4); ++index)
        destroy_native_physical_pending_names_00bf3880(
            at(pointer(queue), index * 0x38u), strings);
    singleton_lifetime_free(pointer(queue));
    put(queue, 0, reinterpret_cast<std::uintptr_t>(replacement));
    put(queue, 8, static_cast<std::uint32_t>(capacity));
}

void resize_native_physical_pending_records_00bf3ed0(void* queue,
    std::int32_t count, NativeStringStorage& strings) {
    if (count > signed_word(queue, 8))
        reserve_native_physical_pending_records_00bf3da0(queue, count, strings);
    const auto initial_count = word(queue, 4);
    if (signed_bits(initial_count) < count) {
        auto offset = initial_count * 0x38u;
        auto remaining = static_cast<std::uint32_t>(count) - initial_count;
        do {
            void* const record = at(pointer(queue), offset);
            if (record != nullptr) {
                for (const auto field : {0u, 4u, 8u, 0xcu, 0x10u, 0x18u, 0x1cu,
                        0x20u, 0x24u, 0x28u, 0x2cu, 0x30u})
                    put(record, field, 0);
            }
            offset += 0x38u;
        } while (--remaining != 0);
    }
    while (count < signed_word(queue, 4)) {
        put(queue, 4, word(queue, 4) - 1u);
        const auto index = word(queue, 4);
        auto* const data = pointer(queue);
        destroy_native_physical_pending_names_00bf3880(at(data, index * 0x38u), strings);
    }
    put(queue, 4, static_cast<std::uint32_t>(count));
}

void destroy_native_physical_pending_records_00bf4b80(void* queue,
    NativeStringStorage& strings) {
    resize_native_physical_pending_records_00bf3ed0(queue, 0, strings);
    singleton_lifetime_free(pointer(queue));
}
} // namespace bsp
