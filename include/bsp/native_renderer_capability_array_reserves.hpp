#pragma once

#include <cstdint>

namespace bsp {

// Full B22B30[130] / B236B0[95], MSVC Win32 only. actual_header addresses
// exactly three native DWORDs: data+0, signed count+4, signed capacity+8.
// The backing allocation belongs to the existing singleton_lifetime shared
// heap. All reached raw reads/writes must have valid extents under the native
// wrapping 32-bit arithmetic; these entries add no overflow or bounds checks.
// They preserve count, including its current value after allocator reentry.
//
// Original ABI: ECX header, one DWORD request in the actual callee argument
// slot, RET4, no semantic result. The explicit ignored EDX argument provides a
// new C++ fastcall interface. It is not a dependency context. B22B30 overwrites
// that actual request slot with its fresh allocation, then reloads the slot;
// the slot is caller-prepared argument storage, not an owner field pointer.
//
// Calls bind directly to fixed cdecl bridges for singleton_lifetime_allocate
// and singleton_lifetime_free. The allocation bridge passes the same wrapping
// byte count as native_bytes and host_bytes and retains the service's current
// malloc/new-handler/retry/throw behavior. Bridges preserve the Win32 cdecl
// nonvolatile registers; caller-save registers/flags are not promised.
// Allocation may throw. Neither naked entry adds an EH frame or rollback;
// original CRT exception objects, native SEH/fault unwinding, incidental
// register results and binary caller compatibility are not established.
void __fastcall reserve_native_capability_records_00b22b30(
    void* actual_header, std::uint32_t unused_edx,
    std::int32_t requested_capacity);

void __fastcall reserve_native_capability_dwords_00b236b0(
    void* actual_header, std::uint32_t unused_edx,
    std::int32_t requested_capacity);

} // namespace bsp
