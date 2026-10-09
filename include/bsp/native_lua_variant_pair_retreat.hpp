#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua variant pair retreat requires MSVC Win32.
#endif

namespace bsp {

// Complete 006EDD80..006EDE08[137] through a new naked Source interface.
// The actual adjacent pair occupies ECX; no dummy EDX or public stack word.
// The provisional name recovers neither a tree type nor predecessor validity.
// Void defines no semantic EAX result; path-dependent register values remain.
//
// Read full pair+0 as a zero guard, call the admitted validation provider if
// zero, then load current pair+4 even if validation returned without repair.
// A nonzero initial node byte+31h selects node+8 and stores it to pair+4 BEFORE
// testing its byte+31h. The other paths traverse current +0/+4/+8 links,
// preserving each intermediate pair+4 store and subsequent raw reload.
// After ascent, test current pair+4's byte+31h before the final candidate
// store. Every flag test distinguishes zero from any nonzero byte.
//
// Borrow live readable pair+0, readable/writable pair+4, and every selected
// node DWORD/byte span, through byte+31h (50-byte highest-address span).
// These are access extents, not allocations or recovered node/header types.
// No copied pair, backing initialization, ownership or lifetime is supplied.
// Raw accesses add no alignment validation or stronger C++ struct-alignment
// promise; callers satisfy the selected x86 accesses and child requirements.
//
// Pair writes may alias later node reads, saved ESI or return backing. Each
// exit pops the CURRENT saved word; completed stores are never rolled back.
// Caller supplies valid control backing and required callee preservation.
// The ordinary provider call reaches entry ESP-20h before its CRT callee;
// either restored-frame tail reaches entry ESP-18h, plus provider frames.
// No null/cycle/range guard, traversal bound or fault/unwind recovery is added.
//
// One CALL and two restored-ESI tail JMPs use the actual admitted Source
// invoke_native_invalid_parameter_00bf6713. Current CRT handler, return and
// failure behavior is qualified Source policy, not Native BF6713/BF66EF
// runtime equivalence. No noexcept, noreturn, Original ABI equivalence,
// production consumer or forced retention is supplied by this interface.
void __fastcall retreat_native_lua_variant_pair_006edd80(void* actual_pair);

} // namespace bsp
