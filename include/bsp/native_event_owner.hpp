#pragma once

#include <cstdint>

namespace bsp {

// Actual eight-byte event storage. Table words are original address identities,
// not a host C++ vptr or callable tables in this process. HANDLE occupies +04.
struct NativeEventOwnerStorage {
    volatile std::uint32_t table_00;
    void* volatile handle_04;
};

inline constexpr std::uint32_t native_event_concrete_table_00d6821c = 0x00d6821c;
inline constexpr std::uint32_t native_event_base_table_00d68208 = 0x00d68208;

// Complete BD1970. Native CL supplies the raw manual-reset byte; it is zero
// extended for CreateEventA, with null security/name and initial state FALSE.
// Allocates native8h/host8h using the existing actual operator-new boundary.
// The returned owner retains a null HANDLE if the Win32 call fails.
NativeEventOwnerStorage* create_native_event_owner_00bd1970(std::uint8_t manual_reset);

// Complete BD19B0, including returning-free tail. Capture current HANDLE, write
// concrete table, unconditionally CloseHandle, restore base table, then free
// owner only when flags bit0 is set. Return the original (possibly freed) address.
// Native: ECX owner, stack flags, RET4/EAX owner. This is a new C++ interface.
NativeEventOwnerStorage* delete_native_event_owner_00bd19b0(
    NativeEventOwnerStorage*, std::uint32_t flags) noexcept;

// Complete native leaves: each loads the current +04 HANDLE exactly once and
// returns the real API result. No null guard, retry, or successful fallback.
std::int32_t signal_native_event_owner_00bd1910(const NativeEventOwnerStorage*) noexcept;
std::uint32_t wait_native_event_owner_00bd17c0(const NativeEventOwnerStorage*) noexcept;
std::int32_t reset_native_event_owner_00bd1960(const NativeEventOwnerStorage*) noexcept;

} // namespace bsp
