#include "bsp/native_resource_record_vector.hpp"

#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource-record vectors require MSVC Win32 pointer widths.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeResourceRecordVectorStorage) == 0x0c);
static_assert(offsetof(NativeResourceRecordVectorStorage, count_04) == 4);
static_assert(offsetof(NativeResourceRecordVectorStorage, capacity_08) == 8);
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);

std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
NativeRenderResourceRecord* record_at(void* base, std::uint32_t byte_offset) noexcept {
    return reinterpret_cast<NativeRenderResourceRecord*>(
        reinterpret_cast<std::uintptr_t>(base) + byte_offset);
}
} // namespace

void reserve_native_resource_record_vector_004da180(
    NativeResourceRecordVectorStorage& actual_header,
    std::int32_t requested_capacity,
    ActualNativeStringPoolStorage& actual_string_pool,
    const SingletonLifetimeCallbacks& actual_validation) {
    volatile auto& header = actual_header;
    volatile auto& current_request = requested_capacity;
    auto capacity = static_cast<std::uint32_t>(current_request);
    if (signed_bits(capacity) < 64) {
        current_request = 64;
        capacity = static_cast<std::uint32_t>(current_request);
    }
    if (signed_bits(header.capacity_08) >= signed_bits(capacity)) return;

    const auto bytes = capacity * 0x2cu;
    void* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes}); // BF55BE -> BF681B.
    std::uint32_t index = 0;
    while (signed_bits(index) < signed_bits(header.count_04)) {
        const auto offset = index * 0x2cu;
        auto* destination = record_at(replacement, offset);
        if (destination != nullptr) {
            destination = ::new (destination) NativeRenderResourceRecord;
            auto* const source = record_at(header.data_00, offset);
            copy_construct_native_resource_record_004d6f70(
                *destination, *source, actual_string_pool, actual_validation);
        }
        ++index;
    }
    // C66680 computes only captured-local arguments and calls RET00401130.
    // No new-array free or completed-prefix destruction occurs on copy failure.
    index = 0;
    if (signed_bits(header.count_04) > 0) {
        std::uint32_t offset = 0;
        do {
            auto* const record = record_at(header.data_00, offset);
            destroy_native_resource_record_004d45a0(*record, actual_string_pool);
            ++index;
            offset += 0x2cu;
        } while (signed_bits(index) < signed_bits(header.count_04));
        // EBX held the running offset; native restores the current argument
        // word after a nonempty destruction loop at004DA234.
        capacity = static_cast<std::uint32_t>(current_request);
    }
    singleton_lifetime_free(header.data_00); // BF6989 -> BF65AC, current base.
    header.data_00 = static_cast<NativeRenderResourceRecord*>(replacement);
    header.capacity_08 = capacity; // Both publications follow returning free.
}

void append_native_resource_record_00b1a3c0(
    NativeResourceRecordVectorStorage& actual_header,
    const NativeRenderResourceRecord* actual_source,
    ActualNativeStringPoolStorage& actual_string_pool,
    const SingletonLifetimeCallbacks& actual_validation) {
    volatile auto& header = actual_header;
    const auto capacity = header.capacity_08;
    if (header.count_04 == capacity) {
        auto doubled = capacity + capacity;
        if (signed_bits(doubled) <= 64) doubled = 64;
        reserve_native_resource_record_vector_004da180(actual_header,
            signed_bits(doubled), actual_string_pool, actual_validation);
    }
    const auto offset = header.count_04 * 0x2cu;
    auto* destination = record_at(header.data_00, offset);
    try { // State0 at00B1A405; reserve above is outside this armed region.
        if (destination != nullptr) {
            destination = ::new (destination) NativeRenderResourceRecord;
            copy_construct_native_resource_record_004d6f70(
                *destination, *actual_source, actual_string_pool, actual_validation);
        }
    } catch (...) {
        // CBC640 reloads current count and then current base before calling
        // RET-only00401130. Retain the volatile reads, without adding cleanup.
        const auto current_offset = header.count_04 * 0x2cu;
        (void)record_at(header.data_00, current_offset);
        throw;
    }
    header.count_04 = header.count_04 + 1u;
}

} // namespace bsp
