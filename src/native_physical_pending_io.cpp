#include "bsp/native_physical_pending_io.hpp"

#include "bsp/native_memory_stream.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_physical_pending_records.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <Windows.h>
#include <cstring>
#include <initializer_list>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(OVERLAPPED) == 0x14);

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
std::uint32_t bits(const void* value) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}
std::int32_t signed_bits(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof result);
    return result;
}
void diagnostic_name(const void* header) noexcept {
    // 4254B0 is a single RET: retain its current argument load, not formatting.
    (void)word(header, 4);
}
void assign_name(void* destination, const void* source, NativeStringStorage& strings) {
    if (destination == source) return;
    resize_native_string_header_0041dd40(destination, strings, word(source), true);
    if (word(source) != 0) {
        const auto length = word(destination);
        const auto* data = pointer(source, 4);
        auto* output = pointer(destination, 4);
        // BF7680 includes backward overlap copying; zero bytes access no data.
        if (length != 0) std::memmove(output, data, length);
    }
}
void require_path_slot(void* provider) {
    const auto table = word(provider);
    if (table != 0x00d69168u || word(reinterpret_cast<void*>(table), 0x1c) != 0x00bf3970u)
        throw std::invalid_argument("Unimplemented current native physical pending path slot");
}
void* allocate(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
} // namespace

void* assign_native_physical_pending_record_00bf3d00(void* destination,
    const void* source, NativeStringStorage& strings) {
    for (const auto offset : {0u, 4u, 8u, 0xcu, 0x10u, 0x18u, 0x1cu})
        put(destination, offset, word(source, offset));
    assign_name(at(destination, 0x20), at(source, 0x20), strings);
    assign_name(at(destination, 0x28), at(source, 0x28), strings);
    put(destination, 0x30, word(source, 0x30));
    return destination;
}

void append_native_physical_pending_record_00bf41c0(void* queue,
    const void* source, NativeStringStorage& strings) {
    const auto capacity = word(queue, 8);
    if (word(queue, 4) == capacity) {
        auto doubled = capacity * 2u;
        if (signed_bits(doubled) <= 1) doubled = 1;
        reserve_native_physical_pending_records_00bf3da0(queue, signed_bits(doubled), strings);
    }
    const auto index = word(queue, 4);
    void* const destination = at(pointer(queue), index * 0x38u);
    if (destination != nullptr)
        copy_construct_native_physical_pending_record_00bf3c10(destination, source, strings);
    // State0 unwind CC7B80 calls RET401130: no additional construction cleanup.
    put(queue, 4, word(queue, 4) + 1u);
}

void erase_native_physical_pending_record_00bf4240(void* queue,
    std::int32_t index, NativeStringStorage& strings) {
    auto current = static_cast<std::uint32_t>(index);
    auto offset = current * 0x38u;
    while (signed_bits(current) < signed_bits(word(queue, 4) - 1u)) {
        auto* const destination = at(pointer(queue), offset);
        assign_native_physical_pending_record_00bf3d00(
            destination, at(destination, 0x38), strings);
        ++current;
        offset += 0x38u;
    }
    const auto count = word(queue, 4);
    auto* const data = pointer(queue);
    destroy_native_physical_pending_names_00bf3880(at(data, count * 0x38u - 0x38u), strings);
    put(queue, 4, word(queue, 4) - 1u);
}

bool submit_native_physical_pending_io_00bf43b0(void* provider,
    const void* first, const void* second, std::uint32_t callback,
    std::uint32_t flags, NativePhysicalPendingIoContext& context) {
    if ((flags & 1u) != 0 || (flags & 0xeu) != 2) return false;
    alignas(4) std::uint32_t path[2];
    require_path_slot(provider);
    const auto* result = build_native_physical_path_00bf3970(provider, path, first, context.physical);
    const auto* path_data = static_cast<const char*>(pointer(result, 4));
    HANDLE const handle = CreateFileA(path_data ? path_data : "", GENERIC_READ,
        FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, nullptr);
    // The native temporary path is not armed in this function's FH3 state map.
    destroy_native_string_header_0041dd20(path, context.physical.strings);
    if (handle == INVALID_HANDLE_VALUE) {
        (void)GetLastError();
        diagnostic_name(first);
        return false;
    }

    alignas(4) std::uint32_t record[0x38 / 4];
    for (const auto offset : {4u, 0xcu, 0x10u, 0x18u, 0x1cu,
            0x20u, 0x24u, 0x28u, 0x2cu, 0x30u})
        put(record, offset, 0);
    put(record, 0, bits(handle));
    put(record, 8, 1);
    // Native state0 owns only record names. It never owns the handle or either
    // allocation, including when assignment/append throws after ReadFile starts.
    __try {
        if (GetFileSizeEx(handle, static_cast<LARGE_INTEGER*>(at(record, 0x18))) == FALSE &&
                GetLastError() != ERROR_SUCCESS) {
            diagnostic_name(first);
            (void)word(record, 0x1c);
            (void)word(record, 0x18);
            CloseHandle(handle);
            return false;
        }
        void* const allocation = allocate(word(record, 0x18) + 0x20000u);
        put(record, 0xc, bits(allocation));
        put(record, 0x10, bits(allocation));
        const auto low_address = bits(allocation) & 0xffffu;
        if (low_address != 0)
            put(record, 0x10, bits(allocation) - low_address + 0x10000u);
        const auto length = word(record, 0x18);
        auto read_count = length;
        if ((length & 0xffffu) != 0) read_count += 0x10000u - (length & 0xffffu);
        // The native rounded high DWORD is a dead local. Stored size stays the
        // original GetFileSizeEx low/high pair; only rounded low reaches ReadFile.
        auto* const overlapped = static_cast<OVERLAPPED*>(allocate(0x14));
        put(record, 4, bits(overlapped));
        for (std::uint32_t offset = 0; offset < 0x14; offset += 4)
            put(overlapped, offset, 0);
        put(record, 0x30, callback);
        if (ReadFile(handle, pointer(record, 0x10), read_count, nullptr, overlapped) == FALSE &&
                GetLastError() != ERROR_IO_PENDING) {
            diagnostic_name(first);
            // BF460D loads [ESP+40] with 12 diagnostic argument bytes pending:
            // local record+10h, the ALIGNED pointer, not record+Ch allocation.
            singleton_lifetime_free(pointer(record, 0x10));
            CloseHandle(handle);
            return false;
        }
        assign_native_string_header_00425f40(at(record, 0x20), first, context.physical.strings);
        assign_native_string_header_00425f40(at(record, 0x28), second, context.physical.strings);
        append_native_physical_pending_record_00bf41c0(at(provider, 0x14), record,
            context.physical.strings);
        return true;
    } __finally {
        destroy_native_physical_pending_names_00bf3880(record, context.physical.strings);
    }
}

void pump_native_physical_pending_io_00bf46b0(void* provider,
    NativePhysicalPendingIoContext& context) {
    auto* const queue = at(provider, 0x14);
    std::uint32_t index = 0;
    std::uint32_t offset = 0;
    while (signed_bits(index) < signed_bits(word(provider, 0x18))) {
        auto* record = at(pointer(queue), offset);
        auto* const overlapped = static_cast<OVERLAPPED*>(pointer(record, 4));
        if (word(overlapped) == 0x103u) {
            ++index;
            offset += 0x38u;
            continue;
        }
        DWORD actual;
        if (GetOverlappedResult(pointer(record), overlapped, &actual, FALSE) == FALSE) {
            (void)GetLastError();
            diagnostic_name(at(pointer(queue), offset + 0x20u));
        } else {
            record = at(pointer(queue), offset);
            if (word(record, 8) == 1) {
                const auto high = word(record, 0x1c);
                const auto low = word(record, 0x18);
                auto* const stream = create_native_memory_stream_from_copy_00befa40(
                    pointer(record, 0x10), low, high, context.memory);
                record = at(pointer(queue), offset);
                context.completion.invoke_00bf476d(word(record, 0x30), stream,
                    at(record, 0x20), at(record, 0x28));
                if (InterlockedDecrement(static_cast<volatile LONG*>(at(stream, 4))) == 0)
                    dispatch_native_memory_owner_zero_reference(stream, context.memory);
            }
        }
        // A callback can append and relocate all record/name storage. The native
        // pump reloads this pointer once, then keeps it through both CRT frees.
        record = at(pointer(queue), offset);
        singleton_lifetime_free(pointer(record, 0xc));
        void* const completed_overlapped = pointer(record, 4);
        if (completed_overlapped != nullptr) {
            singleton_lifetime_free(completed_overlapped);
            put(record, 4, 0);
        }
        CloseHandle(pointer(record));
        erase_native_physical_pending_record_00bf4240(queue, signed_bits(index),
            context.physical.strings);
    }
}
} // namespace bsp
