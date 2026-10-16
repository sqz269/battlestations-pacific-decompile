#pragma once

#include <cstdint>

namespace bsp {

// Names describe observed operations; Original class/semantic types are unknown.
std::uint32_t __fastcall native_tagged_b3_read_dword_14_00b33e50(const void* base) noexcept;

// Original ECX=base, stack+4 supplies byte/dword, RET4. These meaningful new
// ECX/EDX Source interfaces expose concrete AL/EAX after the single store.
std::uint8_t __fastcall native_tagged_b3_write_byte_74_00b343b0(void* base, std::uint8_t value) noexcept;
std::uint32_t __fastcall native_tagged_b3_write_dword_84_00b343e0(void* base, std::uint32_t value) noexcept;

std::uint32_t __fastcall native_tagged_b3_identity_00b3ff30(std::uint32_t bits) noexcept;
std::uint32_t __fastcall native_tagged_b3_address_10_00b3ccf0(std::uint32_t bits) noexcept;

// Three four-byte zero stores, then concrete EAX=0. Only the touched twelve
// bytes need writable backing; no typed/aligned owner record is invented.
// Whole emitted widths/order and fault-register qualifications are in report.
std::uint32_t __fastcall native_tagged_b3_zero_three_dwords_00b344c0(void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_zero_three_dwords_00b344d0(void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_zero_three_dwords_00b344e0(void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_zero_three_dwords_00b344f0(void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_zero_three_dwords_00b3d110(void* base) noexcept;

void __cdecl native_tagged_b3_noop_00b3c4f0() noexcept;
void __cdecl native_tagged_b3_noop_00b3c500() noexcept;
void __cdecl native_tagged_b3_noop_00b3c510() noexcept;
void __cdecl native_tagged_b3_noop_00b3dc80() noexcept;

// RET4 describes cleanup, not a consumed argument value or recovered receiver.
void __stdcall native_tagged_b3_noop_pop04_00b3d4c0(std::uint32_t opaque_word) noexcept;
void __stdcall native_tagged_b3_noop_pop04_00b3d4d0(std::uint32_t opaque_word) noexcept;
void __stdcall native_tagged_b3_noop_pop04_00b3dc90(std::uint32_t opaque_word) noexcept;

// Original XOR AL,AL preserves incoming upper EAX; only AL is the semantic result.
std::uint8_t __cdecl native_tagged_b3_zero_byte_00b3d4e0() noexcept;
std::uint8_t __cdecl native_tagged_b3_zero_byte_00b3d500() noexcept;

// Actual ECX backing input; complete named x86 ADD-memory/RET bodies retain
// one unlocked four-byte RMW and its ADD(-1) EFLAGS, including CF and AF.
// No return value, extra EDX parameter, helper/provider or atomic claim.
void __fastcall native_tagged_b3_add_minus_one_dword_04_00b3e380(void* base) noexcept;
void __fastcall native_tagged_b3_add_minus_one_dword_04_00b3e500(void* base) noexcept;
void __fastcall native_tagged_b3_add_minus_one_dword_04_00b3e680(void* base) noexcept;

} // namespace bsp
