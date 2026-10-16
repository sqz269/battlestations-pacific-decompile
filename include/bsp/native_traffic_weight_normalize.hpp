#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native traffic weight normalization requires MSVC Win32.
#endif

namespace bsp {

// Whole 0043EC30..0043EC42. Chosen Source interface: ECX borrows raw header
// storage, with begin at +4 and end at +8; no class/template is recovered.
// No EDX or incoming stack argument. Plain RET; EAX is zero for null begin,
// otherwise the DWORD-wrapped (end-begin) arithmetic-shifted right by two.
// EDX receives begin; ECX is unchanged. No header-null/range/alignment guard.
std::uint32_t __fastcall size_native_traffic_dwords_0043ec30(const void*);

// Whole 004809F0..00480A89. ECX borrows actual mutable header/data storage;
// no EDX or incoming stack argument, plain RET. EAX exposes the final physical
// size-leaf result, not a recovered semantic return type or success flag.
//
// Entry ESP=S: PUSH ECX creates the actual S-4 sum cell, then PUSH ESI/EDI.
// MOVSS overwrites S-4 with +0. Each x87 FLD/FADD accumulation spills FSTP32
// to this cell before the next size call. The second pass uses FLD/FDIV/FSTP32
// per element, with no zero-sum or exceptional-value guard. Header bounds and
// begin are reloaded at their actual instructions, including after a returning
// diagnostic. Actual storage may alias stack cells/header/data; no snapshots.
// POP ECX loads the CURRENT S-4 word, not necessarily entry ECX or an unchanged
// sum. POP EDI/ESI similarly read their actual saved cells. No FP environment,
// x87 stack, status, rounding mode or MXCSR is initialized or repaired.
//
// The existing genuine current-SDK invalid-parameter provider may return.
// Its policy is not Original private CRT/FH3 identity. Current C++ exception
// propagation through this naked entry, complete Original ABI integration,
// external header producers, physical parents and game adoption are unproved.
std::uint32_t __fastcall normalize_native_traffic_weights_004809f0(void*);

} // namespace bsp
