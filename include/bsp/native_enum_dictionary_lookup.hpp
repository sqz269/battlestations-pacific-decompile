#pragma once

#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {

// Complete normal readers at 004895B0/00489610 and 0048D480/0048D4E0,
// with new Source interfaces rather than original stack/register ABI bindings.
// Borrow actual Win32 string headers and dictionary storage: bucket heads at
// receiver+8, node string at +0/+4, next at +0C. The mapped word at +8 is opaque;
// these readers neither allocate nor insert nor own any dictionary storage.
// The observed native node allocator uses 14h slots, not a 10h node-size claim.
//
// Admission: stable valid headers, closed NUL-free ASCII keys, C-locale CRT,
// null data only for empty strings, matching lengths, finite consistent chains,
// and nonaliasing bucket output. Supply the distinct actual empty-cell bindings
// (00E186ED and 00E17654); no global/default provider is manufactured here.
// Faults, non-ASCII/other locales, allocator/EH behavior and game ABI are external.
// Whole 0043B760 as a distinct borrowed raw property-key service. Read actual
// Win32 header data+4 before length+0; sample consecutive prefix bytes while
// subtracting (length|20h)>>5 from a separate unsigned counter. Preserve signed
// byte promotion into genuine current CRT toupper and modulo-DWORD arithmetic.
// Supply the distinct property fallback role, Original 00E177E4; no default
// cell/global is created. Valid null/empty input selects it without dereference.
// Admit stable actual8h headers and closed NUL-free ASCII buffers whose lengths
// match storage, null data only when empty, and stable current CRT C locale.
// No allocation/ownership/map access. Original stack-header/RET4 is not this
// two-argument C++ interface; no non-ASCII/locale, fault/SEH, ABI or game claim.
std::uint32_t native_property_key_bucket_0043b760(const void* actual_key_header,
    const char* actual_empty_00e177e4);

std::uint32_t native_enum_symbol_bucket_004895b0(const NativeString& key,
    const char* empty_00e186ed);
std::uint32_t native_enum_table_bucket_00489610(const NativeString& key,
    const char* empty_00e186ed);

// Write the bucket before reading its head. Capture query length only after a
// nonnull first head, then return the first length-and-case-insensitive match.
// Return a borrowed node (or null), without reading its mapped word.
const void* find_native_enum_symbol_node_0048d480(const void* actual_receiver,
    const NativeString& key, std::uint32_t& bucket,
    const char* empty_00e186ed, const char* empty_00e17654);
const void* find_native_enum_table_node_0048d4e0(const void* actual_receiver,
    const NativeString& key, std::uint32_t& bucket,
    const char* empty_00e186ed, const char* empty_00e17654);

// Whole 0043B8B0 as an independent ordinary C++ reader of actual Win32 storage.
// Hash the actual eight-byte query header unconditionally, store bucket before
// loading actual_map+8+4*bucket, and capture length only after a nonnull head.
// Node header +0/+4 and next +0C are borrowed; mapped +8 and allocator +10 are
// untouched. Return the first matching mutable borrowed node, or null.
// Supply the distinct actual property-node (00E177E4) and query (00E17654)
// fallback roles. Pointer equality succeeds before null/real CRT stricmp gates.
// Admit live actual 108h maps, finite consistent correctly bucketed chains,
// matching closed NUL-free ASCII headers/buffers (null data only when empty),
// stable nonnull NUL fallback cells and current CRT C locale. Bucket output is
// writable and nonaliasing; external synchronization keeps all borrowed storage
// and bindings stable. No allocation, ownership, populated-map producer,
// replacement lifetime, class/Original ABI, fault/SEH or game claim is made.
void* find_native_property_map_node_0043b8b0(const void* actual_map,
    const void* actual_query_header, std::uint32_t& bucket,
    const char* actual_empty_00e177e4, const char* actual_empty_00e17654);

// Partial Original 008F28F0: only genuine miss tail [008F2930,008F299C).
// Consume a fresh NULL result/bucket from whole property lookup for the SAME
// synchronized actual map/query. Borrow genuine raw bag+4, actual8h owning query,
// same initialized E175B0 38h pool/canonical E188B4 domain and concrete raw string
// context. Allocate a real14h slot; zero only key0/4, preserve allocator+10,
// own the copied key, then publish mapped8/nextC/head/count and return count.
// Admit successful nonoverflowing allocator/getter/buffer domains, valid closed
// ASCII/C locale and predecessor fallback bindings, unique slots, no reentry or
// concurrent mutation/trim, and stable live owners/storage throughout the call.
// Caller retains the published node/key and any opaque mapped pointee; release
// the owning key before returning its slot or destroying the same pool. This
// fragment supplies no map cleanup owner, record preimage/profile dispatcher,
// replacement, outer wrapper, null-allocation/unwind, Original ABI or game proof.
std::uint32_t publish_native_property_map_new_node_008f2930_fragment(
    void* actual_map, const void* actual_query_header,
    std::uint32_t lookup_bucket, std::uint32_t incoming_mapped_record_word,
    void* actual_initialized_property_pool_00e175b0,
    NativeStringRawPoolContext& actual_key_context);

} // namespace bsp
