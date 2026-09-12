#pragma once

#include "bsp/native_retained_memory_owners.hpp"

#include <cstdint>

namespace bsp {
struct SingletonLifetimeCallbacks;
struct NativePhysicalStreamOpenContext;

// Borrow the three actual type descriptor DWORDs at 0109DBA0..0109DBA8.
// They are initialized by CD8FC0 and remain current reads, not synthesized IDs.
struct NativeStoredStreamConversionContext {
    NativeRetainedMemoryOwnerContext& memory_owners;
    const volatile std::uint32_t* actual_memory_type_ids_0109dba0;
    // Required together when converting an actual D691B0 physical stream.
    // The original D691B0 table bytes must remain readable and unchanged.
    NativePhysicalStreamOpenContext* physical = nullptr;
    const volatile std::uint32_t* actual_physical_type_ids_0109dc30 = nullptr;
};

// Complete BB8F60: native ECX ignored, token stack, AL result, RET4.
// This C++ interface adds explicit borrowed descriptor storage.
bool query_native_memory_stream_type_00bb8f60(std::uint32_t token,
    const volatile std::uint32_t* actual_three_type_ids) noexcept;

// Complete original BEF540 machine interface. Ignore high DWORD. Origin0
// starts at backing data, 1 at current cursor, every other value at end.
// Low DWORD addition wraps and is unchecked, including an out-of-range cursor.
void __fastcall native_memory_stream_seek_00bef540(void* actual_stream,
    void* unused_edx, std::uint32_t low, std::uint32_t high,
    std::uint32_t origin) noexcept; // RET0C

// Explicit current D642C0/+0C BB8F60 and +1C BEF540 profile dispatch.
// Other numeric profiles or changed borrowed slot identities are out of domain.
bool dispatch_native_memory_stream_type_query(void* actual_stream,
    std::uint32_t token, NativeStoredStreamConversionContext&) noexcept;
void dispatch_native_memory_stream_seek(void* actual_stream,
    std::uint32_t low, std::uint32_t high, std::uint32_t origin,
    NativeRetainedMemoryOwnerContext&) noexcept;

// Complete BEF750 control flow over actual owners. Native ECX source, EAX
// new14h stream, RET. This new C++ interface adds an explicit ownership/type
// context. Null -> null. Type-query true shares backing+8 and resets only the
// new cursor. Otherwise seek(0,0,0), read current size low DWORD, allocate10h,
// construct backing, read once without an actual-count output, wrap and release
// the temporary backing. No short-read trim, source release or cursor restore.
// Input domain: numeric D642C0 owners; numeric D691B0 owners with explicit
// physical services/type descriptors; or externally supplied actual callable
// original-ABI tables (+0C type, +1C seek, +30 size, +24 read). No table is
// rewritten. Physical conversion uses current type/seek/cached-size/read slots,
// ignores seek/read status and leaves the source cursor changed. Other numeric
// owner classes require their own concrete binding.
void* convert_native_stored_stream_00bef750(void* actual_source,
    NativeStoredStreamConversionContext&);

// Complete BE5E90: ECX tree, output/name stack, EAX output, RET8. Uses existing
// actual lower-bound/comparator and eight-byte NativeFileStoreNameIterator.
// Lower-bound precedes null-tree validation; fallback reloads current head.
void* find_native_file_store_open_name_00be5e90(void* actual_tree,
    void* actual_iterator_output, const void* actual_name_header,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Complete BE5FA0: ECX FileStore, name/full flags stack, EAX stream, RET8.
// Bit0 returns null before reading any owner/name storage. Otherwise search
// actual tree+14 without normalization, validate iterator with returning CRT
// binding, then convert current hit node+14. FileStore tree population and its
// own lifetime remain external. No competing FileStore/stream owner is created.
void* open_native_file_store_00be5fa0(void* actual_file_store,
    const void* actual_name_header, std::uint32_t flags,
    const SingletonLifetimeCallbacks& invalid_parameters,
    NativeStoredStreamConversionContext&);
} // namespace bsp
