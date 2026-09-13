#pragma once
#include "bsp/native_string.hpp"

namespace bsp {
// Source storage contracts for actual10h checked vectors
// {opaque,begin,end,capacity_end}, containing actual8h pooled string headers.
// These own no parallel vector and borrow the caller's string allocation domain.
// 450540 append preserves aliased source elements through a temporary copy on
// growth, uses max(size+1,capacity+capacity/2), and publishes capacity/end/begin
// after forward old-string destruction and backing release. Header0 is retained.
void append_checked_native_string_storage(void* actual_header,
    const void* actual_source_string, NativeStringStorage&);

// 48DA50's forward assignment contract; identical element addresses skip the
// assignment. The existing actual-header string copy reloads after resizing.
void* copy_native_string_range_storage(const void* first, const void* last,
    void* destination, NativeStringStorage&);

// 4954F0: validate only nonnull/equal iterator owners, copy the captured suffix,
// destroy its trailing strings, then publish end and output {first_owner,first}.
// The receiver need not equal either iterator owner. Erasing an empty range
// still validates and writes the output; backing capacity and header0 survive.
void* erase_checked_native_string_storage(void* actual_header, void* output,
    const void* first_owner, void* first, const void* last_owner, void* last,
    NativeStringStorage&);

// Valid, consistent ranges and pooled ownership are required. Append's growth
// contract is the native end-insertion call, not general44F410 insertion.
// Allocation/string callbacks may throw but must not structurally mutate the
// vector or source range. Completed new strings unwind forward on copy failure;
// failed string construction retains the existing string primitive's limits.
// Original library ABI/FH3, arbitrary stack aliases and malformed storage are
// not supplied. Byte-identical native empty-copy calls are omitted as before.
} // namespace bsp
