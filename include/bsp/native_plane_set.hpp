#pragma once
#include <cstddef>
#include <cstdint>

namespace bsp {
struct NativePlaneRecord {
    float coefficients_00[4];
    std::uint32_t flags_10;
};
struct NativePlaneSetStorage {
    NativePlaneRecord planes_00[16];
    std::uint32_t count_140;
};
static_assert(sizeof(NativePlaneRecord) == 0x14);
static_assert(offsetof(NativePlaneRecord, flags_10) == 0x10);
static_assert(offsetof(NativePlaneSetStorage, count_140) == 0x140);
static_assert(sizeof(NativePlaneSetStorage) == 0x144);

// Original B250B0: ECX destination, stack source, RET4. Copy all 16 records,
// not count+140. Each coefficient is FLD m32 / FSTP m32, then flags are MOV.
// Preserve ordered overlap, current x87 control/status and exception effects;
// masked signaling NaNs quiet through x87. No snapshot, memcpy or SSE copy.
// Raw pointers deliberately permit unaligned byte views. The explicit unused
// EDX argument keeps the third argument in the original stack position.
void __fastcall copy_native_plane_set_records_00b250b0(
    void* actual_destination, void* unused_edx, const void* actual_source);

// B65080: ECX actual set, EAX raw count+140, RET. No validation.
std::uint32_t __fastcall native_plane_set_count_00b65080(const void*) noexcept;
// B656F0/B65700: ECX actual base, stack raw index, EAX address/flags, RET4.
// Address arithmetic wraps at 32 bits: base + (index*20), flags at +10.
// The plane-address entry does not dereference; flags/count require readable
// actual memory. EDX is unused. These are explicit C++ entry interfaces, not
// a claim that arbitrary original game callers can use the new library.
const NativePlaneRecord* __fastcall native_plane_set_plane_00b656f0(
    const void* actual_base, void* unused_edx, std::uint32_t index) noexcept;
std::uint32_t __fastcall native_plane_set_flags_00b65700(
    const void* actual_base, void* unused_edx, std::uint32_t index) noexcept;
} // namespace bsp
