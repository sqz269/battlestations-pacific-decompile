#pragma once

#include <cstdint>

namespace bsp {

// Descriptive hypotheses for whole Original bodies, not recovered class names.
// The stdcall words below describe RET cleanup only; no value is consumed.
std::uint32_t __stdcall native_tagged_b3_zero_pop04_00b32910(std::uint32_t opaque_word) noexcept;

// Original: ECX=base, [ESP+4]=value, RET 4, EAX=value after the store.
// New Source: meaningful ECX/EDX arguments, RET; not an Original ABI adapter.
std::uint32_t __fastcall native_tagged_b3_write_dword_08_00b33a90(void* base, std::uint32_t value) noexcept;

// Original ECX supplies the backing address. Only the indicated four bytes
// must be readable; memcpy avoids inventing a typed/aligned record layout.
std::uint32_t __fastcall native_tagged_b3_read_dword_18_00b33e60(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_1c_00b33e70(const void* base) noexcept;

// Original XOR AL,AL defines only the byte result; upper EAX is not a result.
std::uint8_t __cdecl native_tagged_b3_zero_byte_00b33e80() noexcept;
void __cdecl native_tagged_b3_noop_00b33f30() noexcept;
void __cdecl native_tagged_b3_noop_00b33f40() noexcept;
void __stdcall native_tagged_b3_noop_pop08_00b34070(std::uint32_t opaque_word0, std::uint32_t opaque_word1) noexcept;
std::uint32_t __stdcall native_tagged_b3_zero_pop08_00b34080(std::uint32_t opaque_word0, std::uint32_t opaque_word1) noexcept;

std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b34550(const void* base) noexcept;

// Read child address bits at owner+24h, then the child's dword at +4h.
// Child address addition is unsigned modulo 2^32, with no alignment assumption.
std::uint32_t __fastcall native_tagged_b3_read_child_dword_04_00b34560(const void* owner) noexcept;

// Original reads owner+24h, child+0, then [ESP+4]; returns child_word+index*8
// modulo 2^32 and RET 4. New Source takes index meaningfully in EDX, returns
// through RET, and has qualified scratch-register/flags/fault-context effects.
std::uint32_t __fastcall native_tagged_b3_child_scaled_address_00b347c0(const void* owner, std::uint32_t index) noexcept;

} // namespace bsp
