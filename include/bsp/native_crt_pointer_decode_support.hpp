#pragma once

#include <cstdint>

namespace bsp {
struct LegacyCrtMathRuntime;
struct SingletonLifetimeCallbacks;

// These four borrowed bindings stay valid and stable during every call. The
// referenced OS words and owning CRT services retain their current state.
// No OS/TLS initialization, errno storage or handler population is supplied.
struct NativeCrtPointerDecodeSupportContext {
    const volatile std::uint32_t& osplatform_0109dd84;
    const volatile std::uint32_t& winmajor_0109dd90;
    const LegacyCrtMathRuntime& owning_crt;
    const SingletonLifetimeCallbacks& invalid_parameters;
};

// Full C05920[136]. Native cdecl two pointers, EAX -1/0/+1, plain RET.
// Naked Win32 body preserves first-string byte/WORD/aligned-DWORD reads,
// second-string byte comparisons, flags and nonvolatile registers. Both
// inputs must support all original reads, including wider reads before NUL
// decisions. No null, range or page validation and no host strcmp dispatch.
std::int32_t __cdecl native_crt_strcmp_00c05920(
    const char* first, const char* second) noexcept;

// Full BFBB61[60], with an explicit additional source context argument.
// Capture output first. Null output or current OS-platform zero calls owning
// errno, writes22, then the established returning invalid-parameter service.
// This composes native BF66EF(five zero arguments) through that service;
// it does not reproduce the original outgoing stack layout. Failure returns
// 22 and makes no output store, subject to actual service side effects.
// Success copies current winmajor to output and returns0.
std::int32_t get_native_crt_winmajor_00bfbb61(
    std::uint32_t* output, const NativeCrtPointerDecodeSupportContext&);

// Full C04EFB[108]. Calls the complete getter and ignores its status; signed
// major>5 returns1. Otherwise uses actual GetModuleHandleA(nullptr) and its
// current raw PE section headers, comparing against aligned ".mixcrt" with
// the complete raw comparator. Matching section returns0, otherwise1.
// Direct reads require the actual current module and all traversed header
// bytes to be readable. No synthetic-module provider or extra validation.
std::int32_t native_crt_pointer_decode_module_gate_00c04efb(
    const NativeCrtPointerDecodeSupportContext&);

// Getter/gate are qualified C++ interfaces: native private-frame layout,
// incidental volatile registers/EFLAGS, exact fault sites and native SEH
// continuation are not reproduced. Services may return or propagate their
// actual C++ behavior; no catch, synthetic exception or rollback is added.
// Pointer decoder C04FDE, libm service and SSE2 pow remain separate packets.
} // namespace bsp
