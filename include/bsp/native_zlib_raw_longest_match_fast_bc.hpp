#pragma once

#include <cstdint>

#if defined(_MSC_VER) && defined(_M_IX86)
// 00BCBB40 longest_match_fast: new Source control-transfer interface.
// Two genuine inputs select ESI=raw backing and the actual kernel-entry ESP.
// Current_match remains the actual DWORD at entry ESP+4, read once by the
// complete private body. The frame supplies a real continuation and protocol;
// actual argument/saved/control cells may alias input data and the result store.
// MOV ESI/MOV ESP/JMP adds two ordered reads, preserving flags and all EDX bits
// before kernel entry. The private body has EAX result and two actual RET4 exits.
// Source-only public noreturn is not Original no_return, which remains false.
// Formal Original ABI/layout, ordinary C++ return, callers, SEH and game behavior
// are unproved; see docs/GOAL16Y_ZLIB_RAW_LONGEST_MATCH_FAST.md.
extern "C" __declspec(noreturn) void __cdecl
bsp_native_zlib_longest_match_fast_frame_00bcbb40(
    void* backing, void* kernel_entry_stack) noexcept;
#endif
