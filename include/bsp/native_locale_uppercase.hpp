#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw locale uppercase helpers require MSVC Win32.
#endif

namespace bsp {

inline constexpr std::size_t kNativeLocaleUppercaseManagerBytes = 0x4040;
inline constexpr std::size_t kNativeLocaleWideHeaderBytes = 8;

// New ordinary C++ interface, NOT Native ECX/stack/AX/RET4 ABI.
// The input domain is a zero-extended WORD, as supplied by 00A9EC30.
// Arbitrary high bits of Native 00A9EBA0's incoming stack DWORD are not modeled.
// Caller supplies the genuine loaded raw manager, readable through +403F,
// and the actual backing for every CURRENT table address/WORD reached.
// No semantic LocaleTables/NativeWideStringHeader layout is accepted here.
//
// A range rejection calls the existing qualified Source 00BF6713 entry. Its
// actual current-SDK invalid-parameter handler may return: the table pointer
// is then reloaded and normal continuation remains. No exception conversion.
// A sidecar miss calls the selected Source std::towupper, using its current
// CRT/locale/PTD policy; Original CRT/handler equivalence remains unproved.
// Providers may change manager fields; all reached backing must stay valid.
std::uint16_t uppercase_native_locale_code_unit_00a9eba0(
    const void* actual_manager_4040, std::uint16_t code_unit);

// Raw header {CURRENT uint32 length, CURRENT uint32 data address}, not a host
// string/layout adapter. Zero length reads neither data nor manager/provider.
// Each iteration reloads data, reads a WORD and captures its destination BEFORE
// mapping; increments the uint32 index, stores AX to that captured destination,
// then compares CURRENT length. Callback edits must preserve that destination
// through its store, plus every later reached header/data/table extent.
// Serialized accessible-memory/provider/lifetime contracts are caller proofs;
// no allocation, validation repair, publication, ownership or cleanup is added.
// Genuine manager loading/publication is unbound; this grants no startup/raw
// Text admission, Native ABI, retail CRT equivalence or source_ready status.
void uppercase_native_locale_wide_header_00a9ec30(
    const void* actual_manager_4040, void* actual_wide_header8);

} // namespace bsp
