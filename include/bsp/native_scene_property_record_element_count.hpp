#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene property record element count requires MSVC Win32.
#endif

namespace bsp {

// Hypothetical name. Whole [008EF7F0,008EF81A): 42 bytes / 17 instructions.
// ECX is the actual readable record root; EDX is an unused register formal.
// No stack arguments; every exit is plain RET. The tag DWORD at +0x04 must
// be readable and stable. Tags 9/10/11 also require the byte-size DWORD at
// +0x24. This leaf writes nothing and never reads DATA +0x20 or child bytes.
// Tags 9/10 return unsigned byte_size >> 2. Tag 11 returns unsigned
// byte_size / 12 through MUL 0xAAAAAAAB, high DWORD, then SHR 3. Other tags
// return zero without reading byte_size. All DWORD inputs are interpreted
// as unsigned; no positivity, divisibility, saturation or overflow guard.
// EAX is the count. ECX, nonvolatiles, DF and ES remain unchanged. EDX is
// unchanged except on tag 11, where it retains the MUL high DWORD.
// Default XOR: CF0/PF1/ZF1/SF0/OF0 (mask 0x8C5 = 0x44), AF undefined.
// SHR 2/3: CF/PF/ZF/SF defined (mask 0xC5); AF and OF are undefined.
// No allocation, dispatch, ownership, numeric payload or class lifetime
// is established. Constructor/clone round-trip preconditions are separate.
std::uint32_t __fastcall native_scene_property_record_element_count_008ef7f0(
    const void* actual_record_ecx, std::uint32_t unused_edx) noexcept;

} // namespace bsp
