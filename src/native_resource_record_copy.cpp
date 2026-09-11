#include "bsp/native_resource_record_copy.hpp"

#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_string.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource-record copy requires MSVC Win32 pointer widths.
#endif

namespace bsp {
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);
static_assert(offsetof(NativeRenderResourceRecord, sentinel_0c) == 0x0c);
static_assert(offsetof(NativeRenderResourceRecord, payload_14_24) == 0x14);
static_assert(offsetof(NativeRenderResourceRecord, resource_28) == 0x28);

NativeRenderResourceRecord& copy_construct_native_resource_record_004d6f70(
    NativeRenderResourceRecord& actual_destination,
    const NativeRenderResourceRecord& actual_source,
    ActualNativeStringPoolStorage& actual_string_pool,
    const SingletonLifetimeCallbacks& actual_validation) {
    volatile auto& destination = actual_destination;
    const volatile auto& source = actual_source;
    const bool identical = &actual_destination == &actual_source;
    destination.name_length_00 = 0;
    destination.name_data_04 = nullptr;
    if (!identical) {
        resize_native_string_header_0041dd40(&actual_destination,
            actual_string_pool, source.name_length_00, true);
        if (source.name_length_00 != 0) {
            const auto copied = destination.name_length_00;
            auto* const data = destination.name_data_04;
            const auto* const source_data = source.name_data_04;
            if (copied != 0) std::memcpy(data, source_data, copied);
        }
    }
    // State0 is written at004D6FCC, after initial name copying and before
    // entering the alias-list copy. No name cleanup precedes that point.
    try {
        copy_construct_native_render_alias_list_004d48a0(
            reinterpret_cast<unsigned char*>(&actual_destination) + 8,
            reinterpret_cast<const unsigned char*>(&actual_source) + 8,
            actual_string_pool, actual_validation);
        for (unsigned index = 0; index != 5; ++index)
            destination.payload_14_24[index] = source.payload_14_24[index];
        destination.resource_28 = source.resource_28;
    } catch (...) {
        // D8F084 -> C66220: destroy only the current actual name header.
        destroy_native_string_header_0041dd20(
            &actual_destination, actual_string_pool);
        throw;
    }
    return actual_destination;
}

void destroy_native_resource_record_004d45a0(
    NativeRenderResourceRecord& actual_record,
    ActualNativeStringPoolStorage& actual_string_pool) {
    volatile auto& record = actual_record;
    char* captured_data;
    try {
        // The complete004D0A10 algorithm matches the inlined clear/free/null
        // sequence at004D45BE..004D45DF, including current-sentinel reload.
        destroy_native_render_alias_list_004d0a10(
            reinterpret_cast<unsigned char*>(&actual_record) + 8,
            actual_string_pool);
        captured_data = record.name_data_04; // Before state is disarmed.
    } catch (...) {
        // D8ED6C -> C65FD0; the current name remains armed during list cleanup.
        destroy_native_string_header_0041dd20(&actual_record, actual_string_pool);
        throw;
    }
    // Native state=-1 at004D45E8 precedes the length read and pool getter.
    if (captured_data != nullptr)
        actual_string_pool.release(captured_data, record.name_length_00 + 1u);
}

} // namespace bsp
