#pragma once

#include <cstdint>

namespace bsp {
class NativeStringStorage;
class ActualNativeStringPoolStorage;
struct NativeStringRawPoolContext;
struct SingletonLifetimeCallbacks;

// Raw eight-byte headers; no wrapper object or copied std::string is required.
// ECX left, stack right, RET4, AL Boolean for both original comparators.
bool equal_native_string_headers_00435c40(const void* left, const void* right);
bool unequal_native_string_headers_00449af0(const void* left, const void* right);
// ECX source, stack needle/limit, RET8, EAX signed position. Position zero is
// never tested. The limit is unsigned, but the subtraction is tested signed.
std::int32_t reverse_find_native_string_header_00467cf0(
    const void* source, const void* needle, std::uint32_t limit);

// Full raw-header adapters. Each returns the supplied output/destination.
// 4261A0: ECX left, stack output/right, RET8; clears output before copying.
// Cleanup becomes armed only after the initial copy has returned.
void* concatenate_native_string_headers_004261a0(const void* left, void* output,
    const void* right, NativeStringStorage&);
// 425F40: ECX destination, stack source, RET4; preserves identity.
void* assign_native_string_header_00425f40(void* destination, const void* source,
    NativeStringStorage&);
// Actual-pool overloads retain the original header identities and current
// reloads. Assignment reads count, source.data, destination.data after resize;
// concatenation's initial copy reads count, output.data, left.data instead.
// BF7680 permits overlap: these paths use memmove, not the older typed helper.
// Concatenation clears output even when it aliases left, and does not own it
// during initial resize/copy. Append failure consumes one raw output cleanup;
// a cleanup failure propagates the newer exception without retry. This source
// policy does not claim native FH3/double-exception or private-frame alias parity.
// Valid actual headers/data and the existing zero-byte-copy omission apply.
void* assign_native_string_header_00425f40(void* destination, const void* source,
    NativeStringRawPoolContext&);
void* concatenate_native_string_headers_004261a0(const void* left, void* output,
    const void* right, NativeStringRawPoolContext&);
// 41E350: nullable source, preserve=false, copies current length, RET4.
void* assign_native_string_cstring_0041e350(void* destination, const char* source,
    NativeStringStorage&);
// 41E870: nonnull source, clears header first, copies current length+1, RET4.
void* construct_native_string_cstring_0041e870(void* destination, const char* source,
    NativeStringStorage&);
// Same complete 41E870..41E8C2 body over the application's actual pool domain.
// Clear length/data BEFORE the byte scan, including source/header aliases.
// Raw41DD40 preserve1 resolves the current pool and permits getter exceptions;
// read current data then current length+1 afterward and use overlap-safe copy.
// Empty text takes the equal-zero resize path without acquiring a pool.
// Original ECX header, stack nonnull text, RET4, EAX header; this overload's
// borrowed context is a new C++ interface, not original binary/EH ABI proof.
void* construct_native_string_cstring_0041e870(void* destination, const char* source,
    NativeStringRawPoolContext&);

// Actual index: tree+4=head; node links +0/+4/+8, basename +C/+10,
// full name +14/+18, nil byte+1D. Iterator is {owner,node}, two DWORDs.
// No construction, population, ownership or normalization is performed.
// BDA260: ECX tree, stack key, RET4, EAX node.
void* lower_bound_native_physical_index_00bda260(void* tree, const void* key);
// BF36D0: ECX tree, stack iterator-output/key, RET8, EAX output.
void* find_native_physical_index_00bf36d0(void* tree, void* iterator_output,
    const void* key, const SingletonLifetimeCallbacks&);
// BD9860: ECX iterator, RET. Invalid sentinel is a tail call to returning CRT.
void advance_native_physical_index_00bd9860(void* iterator,
    const SingletonLifetimeCallbacks&);
// BD92C0: ECX left iterator, stack right iterator, RET4, AL equality.
bool equal_native_physical_iterators_00bd92c0(const void* left, const void* right,
    const SingletonLifetimeCallbacks&);

// Bind the application's actual publication slot (0109CEEC), owning pooled
// string bridge, and established returning CRT boundary. No cached manager.
struct NativePhysicalFileDateContext {
    void* volatile& manager_0109ceec;
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& invalid_parameters;
};

// Established D69168 physical profile only. Each virtual call reads the current
// table and slot at the native call site: +10 BF3F70, +18 BF39C0, +1C BF3970.
// An unimplemented slot throws std::invalid_argument as an explicit SOURCE
// boundary, not as recovered native behavior. There is no provider callback.
// BF3970: ECX provider; stack output/suffix; RET8; EAX output.
void* build_native_physical_path_00bf3970(void* provider, void* output,
    const void* suffix, NativePhysicalFileDateContext&);
// BF3F70: ECX provider; stack suffix; RET4; AL existence (all index branches).
bool exists_native_physical_path_00bf3f70(void* provider, const void* suffix,
    NativePhysicalFileDateContext&);
// BF39C0: ECX provider; stack mutable name; RET4; AL existence.
bool replace_native_physical_path_00bf39c0(void* provider, void* name,
    NativePhysicalFileDateContext&);
// BF3A80: ECX provider; stack output/name; RET8; EAX output. Current manager+78
// nonzero gives five descending zeros. Enabled path uses real Win32 APIs,
// ignores FileTimeToSystemTime BOOL, and publishes before string cleanup.
// Conversion failure consumes native unspecified SYSTEMTIME words: no added
// exception or defined zero tuple. These interfaces do not preserve SEH/ABI.
void* query_native_physical_file_date_00bf3a80(void* provider, void* output,
    const void* name, NativePhysicalFileDateContext&);
} // namespace bsp
