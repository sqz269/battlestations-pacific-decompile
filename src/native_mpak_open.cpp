#include "bsp/native_mpak_open.hpp"

#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native MPAK open requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
using I = std::int32_t;
static_assert(sizeof(void*) == 4, "Actual MPAK headers contain Win32 pointers.");

const void* at(const void* p, U offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* at(void* p, U offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
U word(const void* p, U offset = 0) noexcept {
    return *static_cast<const volatile U*>(at(p, offset));
}
void put(void* p, U offset, U value) noexcept {
    *static_cast<volatile U*>(at(p, offset)) = value;
}
void* pointer(U value) noexcept { return reinterpret_cast<void*>(value); }
U address(const void* p) noexcept { return reinterpret_cast<U>(p); }

U file_count_from_current_end(const void* provider, U captured_begin) noexcept {
    // IMUL/SAR/add-sign sequence computes signed byte-difference / 24h,
    // then CMP treats that quotient as unsigned. Preserve DWORD subtraction.
    return static_cast<U>(static_cast<I>(word(provider, 0x24) - captured_begin) / 0x24);
}

// The installed 004254B0 body is exactly RET, not an unresolved logging stub.
void trace_004254b0(const char*, const char*, I) noexcept {}
} // namespace

I find_native_mpak_file_index_00bb4a60(void* provider, const void* name,
    const SingletonLifetimeCallbacks& invalid_parameters) {
    U index = 0;
    U record_offset = 0;
    for (;;) {
        const auto first_begin = word(provider, 0x20);
        if (first_begin == 0 || index >= file_count_from_current_end(provider, first_begin))
            return -1;

        const auto checked_begin = word(provider, 0x20);
        if (checked_begin == 0 || index >= file_count_from_current_end(provider, checked_begin))
            invalid_parameters.invalid_parameter(invalid_parameters.context); // May return.

        // The validation callback can replace backing storage. BB4ABB reloads.
        const auto* record = pointer(word(provider, 0x20) + record_offset);
        const auto record_length = word(record);
        const auto requested_length = word(name);
        if (record_length == requested_length) {
            if (record_length == 0) return static_cast<I>(index);
            const auto* requested_data = static_cast<const char*>(pointer(word(name, 4)));
            const auto* record_data = static_cast<const char*>(pointer(word(record, 4)));
            if (_stricmp(record_data, requested_data) == 0) return static_cast<I>(index);
        }
        ++index;
        record_offset += 0x24;
    }
}

bool contains_native_mpak_file_00bb4b20(void* provider, const void* name,
    const SingletonLifetimeCallbacks& invalid_parameters) {
    return find_native_mpak_file_index_00bb4a60(provider, name, invalid_parameters) >= 0;
}

void* open_native_mpak_file_00bb5bb0(void* provider, const void* name,
    U flags, NativeMpakOpenContext& context) {
    if ((flags & 1u) != 0) return nullptr;

    const auto* initial_name_data = static_cast<const char*>(pointer(word(name, 4)));
    I device = static_cast<I>(word(provider, 0x10));
    if (initial_name_data != nullptr) {
        const auto* match = std::strstr(initial_name_data, "_0000");
        if (match != nullptr && address(match) - word(name, 4) != 0xffffffffu && device == -1) {
            U temporary[2];
            // Construction has no cleanup owner until the original call returns.
            copy_construct_native_string_header_00426060(temporary, name, context.strings);
            try {
                auto* const captured_manager = context.actual_manager_publication_0109ceec;
                device = context.dispatch.select_device_00bdd850(captured_manager,
                    temporary, temporary);
            } catch (...) {
                destroy_native_string_header_0041dd20(temporary, context.strings);
                throw;
            }
            // Native state is -1 before the getter/return pair. Release uses
            // current fields and leaves the now-dead temporary words unchanged.
            destroy_native_string_header_0041dd20(temporary, context.strings);
        }
    }

    auto* const current_manager = context.actual_manager_publication_0109ceec;
    put(current_manager, 0x18, static_cast<U>(device));
    const auto* current_name_data = static_cast<const char*>(pointer(word(name, 4)));
    trace_004254b0(">PAK open %s deviceid:%d", current_name_data ? current_name_data : "", device);

    I index;
    const auto cached_index = word(provider, 0x40);
    if (static_cast<I>(cached_index) > 0 &&
        equal_native_string_headers_00435c40(
            context.library.file_at_00bb4140(at(provider, 0x1c), cached_index), name)) {
        index = static_cast<I>(word(provider, 0x40)); // Original reload after comparison.
    } else {
        index = find_native_mpak_file_index_00bb4a60(provider, name, context.invalid_parameters);
    }
    return context.dispatch.materialize_entry_00bb5080(provider, index);
}
} // namespace bsp
