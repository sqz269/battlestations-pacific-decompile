#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native entity intrusive unlink requires MSVC Win32.
#endif

namespace bsp {

inline constexpr std::size_t native_entity_intrusive_header_bytes = 0x0c;

// Hypothetical names for the complete 83-byte / 29-instruction raw leaves.
// ECX is the actual 12-byte {first+0, last+4, count+8} header. EDX is a padding
// formal, not an input. The actual entity occupies one stack word, consumed
// by RET4, and is returned unchanged in EAX. ECX and nonvolatiles survive;
// EDX is volatile. No allocation, callback, projection or membership check.
//
// Caller supplies physical storage/lifetime for every access actually reached.
// These entries do not produce World/entity objects or establish their phase.
// With both entity links zero, only signed count > 1 skips mutation. Otherwise
// preserve the physical reload/overlap schedule, clear next before previous,
// and ADD the count DWORD by -1 with native wrapping and final ADD flags.
// A skipped call retains the flags of CMP count,1. DF/x87/SSE are untouched.
// No Native SEH/hardware-fault compatibility or executable-patching ABI claim.

// [00903F30,00903F83): entity previous+34h, next+38h.
void* __fastcall unlink_native_entity_world_chain_00903f30(
    void* actual_header, std::uint32_t unused_edx, void* actual_entity) noexcept;

// [00924710,00924763): entity previous+40h, next+44h.
void* __fastcall unlink_native_entity_sibling_chain_00924710(
    void* actual_header, std::uint32_t unused_edx, void* actual_entity) noexcept;

} // namespace bsp
