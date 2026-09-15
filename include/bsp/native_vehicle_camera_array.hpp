#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Borrow the four actual constant cells; no competing global/default state.
// Observed image bits: C0490FDB, 40490FDB, BFC90FDB, 3FC90FDB.
struct NativeVehicleCameraDefaults {
    const volatile std::uint32_t& word_00ce684c;
    const volatile std::uint32_t& word_00d7a264;
    const volatile std::uint32_t& word_00ce3ccc;
    const volatile std::uint32_t& word_00ce3c64;
};

// Complete 0078E230..0078E265. Native ECX points to four words, EAX returns
// that pointer. Ordered raw MOVSS transfers; no arithmetic or NaN conversion.
void* initialize_native_vehicle_camera_limits_0078e230(
    void* actual_four_words, const NativeVehicleCameraDefaults& defaults) noexcept;

// Complete 005CD070..005CD0E2. Native ECX=destination, stacked source, RET4,
// EAX=destination. Row stride5Ch: string header+0/+4, raw matrix+8..47,
// four words+48..57, byte+58. Padding+59..5B is never copied. Self-copy still
// clears its string header. String bytes use the existing host memmove boundary.
void* copy_native_vehicle_camera_row_005cd070(
    void* actual_destination, const void* actual_source,
    NativeStringRawPoolContext& strings);

// Complete ordinary bodies 005CD260..005CD354 and005CD640..005CD703.
// Native ECX={DWORD data,signed count,signed capacity}, stacked signed request,
// RET4; no stable result. Reserve copies rows, releases old strings forwards,
// frees current backing, then publishes data/capacity. Resize growth initializes
// only the string header and four limit words; shrink publishes each decrement
// before release. Matrix/byte/padding of new rows remain untouched.
void reserve_native_vehicle_camera_array_005cd260(
    void* actual_header, std::int32_t requested, NativeStringRawPoolContext& strings);
void resize_native_vehicle_camera_array_005cd640(
    void* actual_header, std::int32_t requested, NativeStringRawPoolContext& strings,
    const NativeVehicleCameraDefaults& defaults);

// New Win32 source interfaces, not original ABI/FH3 bridges. Borrow valid
// reached storage and canonical pool publications; context objects must remain
// outside modified storage. Preserve DWORD wrap and signed comparisons, without
// validation, rollback or fault recovery. Native reserve unwind is a RET-only
// helper: completed copies/replacement storage are not reclaimed on failure.
// Host CRT allocation/memmove, DF=0, and native pool interfaces are explicit
// dependencies; original heap, hardware-fault and exception identity are not.
} // namespace bsp
