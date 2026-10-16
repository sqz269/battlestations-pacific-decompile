#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native traffic range erase requires MSVC Win32.
#endif

namespace bsp {

// Whole 0048E400..0048E443 and 00489110..00489169. Chosen Source envelopes:
// ECX actual borrowed raw header; Source-only unused EDX binds FIVE actual
// caller stack DWORDs {out,first_metadata,first,last_metadata,last}. Original
// has no incoming EDX input. Both return physical EAX=late actual out, RET14h.
// No Original class/template/declaration/semantic metadata type is recovered.
//
// First metadata is cached BEFORE a genuine potentially returning diagnostic;
// first/last positions are loaded AFTER it. Metadata validity is first!=0 and
// first==actual second, without a receiver comparison. Equal positions skip
// all header-end access/copy/publication. Otherwise header+8 is read late.
// After copy/publication, load CURRENT actual out cell; POP EDI; store out+4
// first; POP ESI; store out+0 cached metadata; restore EBX/EBP; RET14h.
// Actual argument, saved-register, header and data cells may overlap. No guard,
// snapshot, repaired bounds, output temporary or storage-domain narrowing.
//
// POD18 calls actual cdecl 00486850 with three actual words; that complete
// existing provider preserves partial tag preimages and all six words passed
// to the real six-word x87 0047FC30 child. Child EAX is published directly.
void* __fastcall erase_native_traffic_pod18_range_0048e400(
    void*, std::uint32_t, void*, std::uint32_t, void*, std::uint32_t, const void*);

// DWORD suffix is wrapped(end-last), signed SAR2. Capture newend before copy;
// count<=0 still publishes wrapped newend. Positive count calls actual current
// SDK memmove_s with four cdecl words and publishes even after an error return.
void* __fastcall erase_native_traffic_dword_range_00489110(
    void*, std::uint32_t, void*, std::uint32_t, void*, std::uint32_t, const void*);

// The whole owned schedules are retained with genuine qualified Source
// providers. Code/return/fault addresses, private frames, saved x87 FIP, current
// SDK error policy and exception propagation through naked entries do not
// establish Original private CRT/FH3/RTTI/full physical ABI or game identity.
// No FP control/status/stack reset, NaN conversion policy or noexcept promise.
// Physical parents, other insertion/resize entries and header producers remain
// separate OPEN contracts; an abstract Host method is not a physical provider.

} // namespace bsp
