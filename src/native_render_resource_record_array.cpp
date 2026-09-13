#include "bsp/native_render_resource_record_array.hpp"

#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource-record arrays require MSVC Win32 pointer widths.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);

template<class T> volatile T& field(void* header, std::size_t offset) {
    return *reinterpret_cast<volatile T*>(static_cast<unsigned char*>(header) + offset);
}
std::int32_t signed_bits(std::uint32_t value) {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
NativeRenderResourceRecord* record_at(void* begin, std::uint32_t index) {
    return reinterpret_cast<NativeRenderResourceRecord*>(
        reinterpret_cast<std::uintptr_t>(begin) + index * 0x2cu);
}
NativeRenderResourceRecord* current_begin(void* header) {
    return field<NativeRenderResourceRecord*>(header, 0);
}
std::uint32_t current_count(void* header) {
    return field<std::uint32_t>(header, 4);
}
} // namespace

void reserve_native_render_resource_record_array_00b2ff00(
    void* actual_header, std::uint32_t requested_capacity,
    SizedStoragePool& actual_string_pool, const SingletonLifetimeCallbacks& callbacks) {
    if (signed_bits(requested_capacity) < 64) requested_capacity = 64;
    if (signed_bits(field<std::uint32_t>(actual_header, 8)) >=
        signed_bits(requested_capacity)) return;

    const auto allocated_bytes = requested_capacity * 0x2cu;
    auto* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::object, allocated_bytes, allocated_bytes});
    std::uint32_t index = 0;
    while (signed_bits(index) < signed_bits(current_count(actual_header))) {
        auto* destination = record_at(replacement, index);
        if (destination != nullptr) {
            // Start this raw record's lifetime without initializing its fields.
            destination = ::new (destination) NativeRenderResourceRecord;
            auto* const source = record_at(current_begin(actual_header), index);
            copy_construct_native_render_resource_record_00b2fc60(
                *destination, *source, actual_string_pool, callbacks);
        }
        ++index;
    }
    // CBD970 only computes two pointer arguments and calls RET leaf 00401130.
    // There is no replacement free or completed-record unwind on copy failure.
    index = 0;
    while (signed_bits(index) < signed_bits(current_count(actual_header))) {
        auto* const record = record_at(current_begin(actual_header), index);
        destroy_native_render_resource_record_00b2f990(*record, actual_string_pool);
        ++index;
    }
    singleton_lifetime_free(current_begin(actual_header));
    field<NativeRenderResourceRecord*>(actual_header, 0) =
        static_cast<NativeRenderResourceRecord*>(replacement);
    field<std::uint32_t>(actual_header, 8) = requested_capacity;
}

void resize_native_render_resource_record_array_00b30340(
    void* actual_header, std::uint32_t requested_count,
    SizedStoragePool& actual_string_pool, const SingletonLifetimeCallbacks& callbacks) {
    if (signed_bits(requested_count) > signed_bits(field<std::uint32_t>(actual_header, 8)))
        reserve_native_render_resource_record_array_00b2ff00(
            actual_header, requested_count, actual_string_pool, callbacks);

    auto index = current_count(actual_header);
    while (signed_bits(index) < signed_bits(requested_count)) {
        auto* record = record_at(current_begin(actual_header), index);
        if (record != nullptr) {
            record = ::new (record) NativeRenderResourceRecord;
            volatile auto* const current = record;
            current->name_length_00 = 0;
            current->name_data_04 = nullptr;
            PooledStringStorage storage(actual_string_pool);
            try { // State 1 at B3039B, after both empty-name stores.
                auto* const sentinel = allocate_native_render_alias_sentinel_004c3020();
                current->sentinel_0c = sentinel;
                current->alias_count_10 = 0;
                for (unsigned tail = 5; tail != 0; --tail)
                    current->payload_14_24[tail - 1] = 0;
            } catch (...) {
                destroy_native_string_header_0041dd20(record, storage);
                // State 0 CBDA90 reloads current begin for a no-op 00401130 call.
                // Preserve that header read; it performs no allocation cleanup.
                (void)record_at(current_begin(actual_header), index);
                throw;
            }
        }
        ++index;
    }
    while (signed_bits(requested_count) < signed_bits(current_count(actual_header))) {
        field<std::uint32_t>(actual_header, 4) = current_count(actual_header) - 1u;
        const auto retained = current_count(actual_header);
        auto* const record = record_at(current_begin(actual_header), retained);
        destroy_native_render_resource_record_00b2f990(*record, actual_string_pool);
    }
    field<std::uint32_t>(actual_header, 4) = requested_count;
}


void reserve_native_render_resource_record_array_00b2ff00(
    void* actual_header, std::uint32_t requested_capacity,
    ActualNativeStringPoolStorage& actual_string_pool, const SingletonLifetimeCallbacks& callbacks) {
    if (signed_bits(requested_capacity) < 64) requested_capacity = 64;
    if (signed_bits(field<std::uint32_t>(actual_header, 8)) >=
        signed_bits(requested_capacity)) return;

    const auto allocated_bytes = requested_capacity * 0x2cu;
    auto* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::object, allocated_bytes, allocated_bytes});
    std::uint32_t index = 0;
    while (signed_bits(index) < signed_bits(current_count(actual_header))) {
        auto* destination = record_at(replacement, index);
        if (destination != nullptr) {
            // Start this raw record's lifetime without initializing its fields.
            destination = ::new (destination) NativeRenderResourceRecord;
            auto* const source = record_at(current_begin(actual_header), index);
            copy_construct_native_render_resource_record_00b2fc60(
                *destination, *source, actual_string_pool, callbacks);
        }
        ++index;
    }
    // CBD970 only computes two pointer arguments and calls RET leaf 00401130.
    // There is no replacement free or completed-record unwind on copy failure.
    index = 0;
    while (signed_bits(index) < signed_bits(current_count(actual_header))) {
        auto* const record = record_at(current_begin(actual_header), index);
        destroy_native_render_resource_record_00b2f990(*record, actual_string_pool);
        ++index;
    }
    singleton_lifetime_free(current_begin(actual_header));
    field<NativeRenderResourceRecord*>(actual_header, 0) =
        static_cast<NativeRenderResourceRecord*>(replacement);
    field<std::uint32_t>(actual_header, 8) = requested_capacity;
}

} // namespace bsp
