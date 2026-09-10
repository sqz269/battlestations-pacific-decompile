#pragma once

#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {

// Raw 14h row, with no initialization/destructor. The three words at +8 are
// copied by native x87 FLD/FSTP, not as integer bits. This layout declaration
// does not overlay an object on the caller's existing row/header storage.
struct NativeRenderQueueRowStorage {
    std::uint32_t string_length_00;
    char* string_data_04;
    std::uint32_t float_words_08[3];
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderQueueRowStorage) == 20);
static_assert(offsetof(NativeRenderQueueRowStorage, string_data_04) == 4);
static_assert(offsetof(NativeRenderQueueRowStorage, float_words_08) == 8);
static_assert(std::is_trivially_default_constructible_v<NativeRenderQueueRowStorage>);

// actual_header is the existing three-DWORD data/count/capacity header, such
// as the actual queue's +24h address. No new header object, owner or count is
// created. Native ECX=header, stack signed request, RET4; these C++ interfaces
// add the existing explicit string-storage boundary and return void.
void reserve_native_render_queue_rows_00b1db30(void* actual_header,
    NativeStringStorage&, std::int32_t capacity);
void resize_native_render_queue_rows_00b1e7f0(void* actual_header,
    NativeStringStorage&, std::int32_t count);

// Native ECX=header, RET. Resize0, reload data, free; leave dangling data and
// capacity. Reserve likewise publishes new data/capacity only after old free.
void destroy_native_render_queue_rows_00b1f150(void* actual_header,
    NativeStringStorage&);

// Caller supplies valid readable/writable spans and nonnegative live header
// count/capacity. Native DWORD address/product arithmetic and null computed-row
// branches remain; no overflow or corrupt-header repair is added. Reserve's
// native exception helper 00401130 is RET: replacement storage and completed
// strings are not rolled back on failure. NativeStringStorage retains its
// explicit pool boundary, noexcept release and zero-byte memcpy omission.
// Full behavior, evidence and verification: docs/NATIVE_RENDER_QUEUE_ROWS.md.
} // namespace bsp
