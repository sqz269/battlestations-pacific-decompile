#include "bsp/native_resource_reader_owner.hpp"
#include "bsp/native_resource_reader_buffer.hpp"
#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>
#include <exception>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource reader ownership requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(LONG) == 4);

std::uint32_t word(const void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
void* at(void* base, std::uint32_t byte_offset) noexcept {
    return pointer(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(base)) + byte_offset);
}

int unwind_exception_filter(unsigned long code) noexcept {
    // BF7C44..BF7C57 and C0696D..C06980: secondary C++ exception -> terminate.
    // The original CRT's TLS ProcessingThrow/CLR bookkeeping is not this ABI.
    if (code == 0xe06d7363UL) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}

void unwind_strings(void* current, std::int32_t remaining,
    NativeStringRawPoolContext& pool) {
    // BF7C10 receives the cursor at the failing/unconstructed element and
    // the number of preceding completed elements. Decrement BEFORE each call.
    __try {
        while (--remaining >= 0) {
            current = at(current, 0xfffffff8u);
            destroy_native_string_header_0041dd20(current, pool);
        }
    } __except (unwind_exception_filter(GetExceptionCode())) {
        // Both evidenced filters continue search or terminate, never handle.
    }
}

void destroy_ten_strings(void* first, NativeStringRawPoolContext& pool) {
    void* current = at(first, 0x50);
    std::int32_t remaining = 10;
    bool completed = false;
    __try {
        while (--remaining >= 0) {
            current = at(current, 0xfffffff8u);
            destroy_native_string_header_0041dd20(current, pool);
        }
        completed = true;
    } __finally {
        if (!completed) unwind_strings(current, remaining, pool);
    }
}

void construct_ten_strings(void* first, NativeStringRawPoolContext& pool) {
    void* current = first;
    std::int32_t count = 0;
    bool completed = false;
    __try {
        while (count < 10) {
            // 415270 is two DWORD stores. An unfinished entry is not counted.
            put(current, 0, 0);
            put(current, 4, 0);
            current = at(current, 8);
            ++count;
        }
        completed = true;
    } __finally {
        if (!completed) unwind_strings(current, count, pool);
    }
}

void unwind_buffer(void* header, NativeStringRawPoolContext& pool) {
    __try { destroy_native_resource_reader_buffer_00bf0980(header, pool); }
    __except (unwind_exception_filter(GetExceptionCode())) {}
}
void unwind_base(void* reader, NativeStringRawPoolContext& pool,
    NativeAdoptedSubstreamDispatch& streams) {
    __try { destroy_native_resource_reader_00bf09b0(reader, pool, streams); }
    __except (unwind_exception_filter(GetExceptionCode())) {}
}
void unwind_ten_strings(void* first, NativeStringRawPoolContext& pool) {
    __try { destroy_ten_strings(first, pool); }
    __except (unwind_exception_filter(GetExceptionCode())) {}
}
} // namespace

void destroy_native_resource_reader_buffer_00bf0980(void* header,
    NativeStringRawPoolContext& pool) {
    resize_native_resource_reader_buffer_00bf0700(header, 0, pool);
    singleton_lifetime_free(pointer(word(header)));
}

void destroy_native_resource_reader_00bf09b0(void* reader,
    NativeStringRawPoolContext& pool, NativeAdoptedSubstreamDispatch& streams) {
    void* const source = pointer(word(reader)); // Before native state0 is armed.
    bool buffer_armed = true;
    __try {
        if (source) {
            auto* const refs = reinterpret_cast<volatile LONG*>(at(source, 4));
            if (InterlockedDecrement(refs) == 0) {
                const auto table = word(source);
                const auto entry = word(pointer(table));
                streams.source_zero_reference(entry, source, table);
            }
            put(reader, 0, 0);
        }
        buffer_armed = false;
        destroy_native_resource_reader_buffer_00bf0980(at(reader, 4), pool);
    } __finally {
        if (buffer_armed) unwind_buffer(at(reader, 4), pool);
    }
}

void* construct_native_structured_resource_reader_00bea150(void* reader,
    NativeStringRawPoolContext& pool, NativeAdoptedSubstreamDispatch& streams) {
    construct_native_resource_reader_00bf09a0(reader);
    bool base_armed = true;
    __try {
        construct_ten_strings(at(reader, 0x10), pool);
        put(reader, 0x60, 0);
        put(reader, 0x64, 0xffffffffu);
        put(reader, 0x68, 0);
        put(reader, 0x6c, 0);
        base_armed = false;
    } __finally {
        if (base_armed) unwind_base(reader, pool, streams);
    }
    return reader;
}

void destroy_native_structured_resource_reader_00be9f10(void* reader,
    NativeStringRawPoolContext& pool, NativeAdoptedSubstreamDispatch& streams) {
    const auto name_data = word(reader, 0x6c); // Before native state1 is armed.
    std::int32_t state = 1;
    __try {
        if (name_data != 0) {
            const auto size = word(reader, 0x68) + 1u;
            auto* const current_pool = native_string_pool_get_or_create_00419cc0(
                pool.actual_published_01090aa8, pool.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(current_pool, pointer(name_data), size,
                pool.actual_small_returns_disabled_01090aa4);
        }
        state = 0;
        destroy_ten_strings(at(reader, 0x10), pool);
        state = -1;
        destroy_native_resource_reader_00bf09b0(reader, pool, streams);
    } __finally {
        if (state == 1) {
            state = 0; // Native FrameUnwindToState advances before calling action.
            unwind_ten_strings(at(reader, 0x10), pool);
        }
        if (state == 0) {
            state = -1;
            unwind_base(reader, pool, streams);
        }
    }
}

} // namespace bsp
