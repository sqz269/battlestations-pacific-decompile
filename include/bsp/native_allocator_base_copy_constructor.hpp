#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native allocator base-copy construction requires MSVC Win32.
#endif

namespace bsp {

// Complete owned schedule 00BF63A6..00BF63FD: 88 bytes / 38 instructions in
// Original. Descriptive Source name is provisional; saved library name is
// exception. Naked fastcall binds ECX receiver, unused incoming EDX word and
// actual source address at ESP+4; EAX returns captured receiver, RET 4.
// The explicit EDX argument only fixes Source argument placement. CRT calls
// may clobber volatile registers; no extra EDX preservation promise is made.
//
// Caller supplies actual raw 12-byte destination/source backing. Destination
// DWORDs 0/4/8 are writable; source DWORDs 4/8 are readable. The nonzero-control,
// null-message path also reads destination+4 through a real AND DWORD RMW.
// Publish raw 00D69370h, copy the full source+8 control word, then read source+4.
// Zero control shallow-copies that pointer. Nonzero control with a null pointer
// uses the RMW. Otherwise call current Source CRT strlen, malloc(length+1 with
// 32-bit wrap), publish the nullable allocation, and on success reload source+4
// before strcpy_s. The copy result is ignored; control is not cleared on failure.
//
// Preserve alias/partial-fault order. No receiver/source null check, self-copy
// guard, prior free, catch, extra EH frame or throwing allocator is added.
// Child pointer validity, allocation ownership/lifetime and CRT failure policy
// remain caller/provider contracts. Raw profile data creates no callable Source
// vtable, exception type, RTTI, static owner or Native slot binding. This Source
// CRT composition does not establish Original placement, callers, flags/fault
// equivalence, exception/runtime identity, drop-in ABI or gameplay behavior.
void* __fastcall construct_native_allocator_base_copy_00bf63a6(
    void* actual_receiver,
    std::uint32_t unused_edx,
    const void* actual_raw_source);

} // namespace bsp
