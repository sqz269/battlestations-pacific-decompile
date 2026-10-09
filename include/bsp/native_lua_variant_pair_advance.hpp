#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua variant pair advance requires MSVC Win32.
#endif

namespace bsp {

// Complete 006EDA20..006EDA82[99] through a new naked Source interface. The
// single pointer occupies ECX; there is no EDX dummy or public stack argument.
// The provisional name does not recover a tree type or successor guarantee.
// Void exposes no semantic result; physical EAX/ECX/EDX residuals are retained.
//
// Borrow the actual adjacent pair DWORDs: read +0 as a full zero guard, then
// read/write the current +4 word. A returning invalid-parameter call precedes
// a fresh +4 load, without retrying +0. A nonzero initial node byte+31h pops
// the saved ESI word BEFORE tail-jumping to the same actual Source provider.
// Other paths traverse current raw node links +0/+4/+8 and byte+31h. Preserve
// every intermediate pair+4 store and subsequent link/pair reload. Tests use
// zero versus any nonzero byte, not an assumed flag value of one.
//
// Supply live readable pair+0, readable/writable pair+4 and every selected
// node DWORD/byte span, including byte+31h (50-byte highest-address span).
// These are access extents, not allocation sizes. No copied pair, private
// node/header, owner, allocation or initialized backing is supplied. Raw
// accesses add no alignment check or stronger C++ struct-alignment promise;
// the caller must satisfy the selected x86 accesses and child requirements.
//
// Pair writes may alias later node reads, saved ESI and return backing. The
// body pops the CURRENT save word and does not roll back completed stores.
// Caller supplies valid control backing and any required callee preservation.
// The ordinary Source provider call reaches entry ESP-20h before its CRT
// callee; the restored-frame tail reaches entry ESP-18h, plus provider frames.
// No null/cycle/range check, progress bound or fault/unwind recovery is added.
//
// CALL and tail JMP use admitted invoke_native_invalid_parameter_00bf6713.
// Its current CRT/handler/return/failure policy is explicit Source policy,
// not Native BF6713/BF66EF runtime equivalence. No noexcept or noreturn,
// Native ABI equivalence, production consumer or forced retention is supplied.
void __fastcall advance_native_lua_variant_pair_006eda20(void* actual_pair);

} // namespace bsp
