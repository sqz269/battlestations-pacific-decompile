#pragma once
#include <cstdint>

namespace bsp {
struct NativeGuiIconStateRangeConstructionStorageFrame;
struct NativeGuiIconStorageContext;

// NORMAL AB58A0..AB58C4,37B/14ops. Original entry has three stacked address
// DWORDs and entryECX bits whose upper24 become a local dispatch tag; RET0Ch.
// This six-argument Source ABI explicitly requires those real input bits,
// ordinary live tag backing and separate four-home callee storage. No default
// zero/tag, owner, Frame, callback, type conversion or input filter is supplied.
//
// Copy entryECX into tag HOME; read outerHOME2 FIRST. Clear only tag LOWBYTE,
// read its FULL DWORD and write calleeHOME3. Reread outerHOME2 independently,
// then write calleeHOME2=first capture. Read outerHOME1 end, write calleeHOME1
// with ORIGINAL entryECX (not masked tag), read outerHOME0 begin, then write
// calleeHOME0=second destination capture. These volatile reaches must not be
// coalesced or replaced by a single destination/tag snapshot. Forward captured
// source begin/end and SAME four mutable homes/real RangeFrame/context directly
// to the existing AB4C50 NORMAL supplier. Return its numeric destination bits.
// No outer HOME copyback follows. Original POP-ECX is private ABI, not a Source
// authority/owner/result. Caller supplies genuine entry bits, never invented
// register state or a claim that this new ordinary ABI captures Native ECX.
//
// Tag and homes are genuine retained C++ operand objects; their legal aliases
// and genuine callback writes retain the reached load/store schedule. Private
// Source parameters/controls/spills remain separate from operand/record and
// provider-write extents. The real existing callee's individual-state birth,
// SAME raw-copy/pool/string/texture domains, mutable end HOME and UNKNOWN graph
// retention contracts are inherited unchanged. Keep all input/output homes,
// tag, frame, current/partial/completed/remaining records and providers through
// every entered escape; distinct active entries need distinct retained backing.
//
// No wrapper EH or cleanup is added. AB4C50's13Nativecatch rows remain held;
// this wrapper cannot convert arbitrary Source escape into Native permission,
// prefix destruction, retry/rollback/noexcept or normal-disposition authority.
// Existing HIGH STL/vendor name/tag and all prior credits remain; zero new
// Native/Original/vendor/helper/CG/duplicate-body credit. Private stack/FLAGS/
// fault/FH3/SEH/Original ABI/full growth/caller/runtime/activation remain unproved.
std::uint32_t copy_native_gui_icon_state_range_wrapper_storage_00ab58a0(
    std::uint32_t actual_entry_ecx_bits,
    const volatile std::uint32_t (&actual_outer_stack_homes)[3],
    std::uint32_t& actual_tag_home,
    volatile std::uint32_t (&actual_callee_stack_homes)[4],
    NativeGuiIconStateRangeConstructionStorageFrame& frame,
    NativeGuiIconStorageContext& context);
} // namespace bsp
