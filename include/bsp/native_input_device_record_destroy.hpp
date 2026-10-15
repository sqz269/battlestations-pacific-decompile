#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual input-device record destruction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete0055B690..0055B8A5 (534B/195 instructions): actual84h record.
// Native ECX record, EDX unused, no stacked input, plain RET. EAX is residue,
// not a recovered result. Source retains dead EDX and adds stacked RawCtx.
// Normal reverse member order:78,6C,5C,50,40,30,24,18,0C,00. Each initial
// head/head.left or begin capture precedes the next state arm; that state
// remains active through child/free/clears and the following initial captures.
// Tree calls consume only accepted full-selection prefixes, with private8B
// output and actual owner/endpoints. Matching finite exclusive owned storage
// must remain stable through each selection. Free CURRENT head/begin; preserve
// each opaque+0; no null-head, self, retry or repeated-destroy guard is added.
void __fastcall destroy_native_input_device_record_0055b690(
    void* record, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);

// Complete0055B8B0..0055B910 (97B/29 instructions): ECX actual pair, EDX
// unused, no stacked input, RET. Record at+8 is destroyed with key unwind
// armed. Capture CURRENT key data/test at+4, disarm, then read length only for
// nonnull data and return that captured block/length+1 through the actual pool.
// Key fields are not cleared. EAX has no stable semantic result, even when
// data is null. The extra context/private Source frames change the native ABI.
void __fastcall destroy_native_input_device_pair_0055b8b0(
    void* pair, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);

// Complete006A6EB0..006A6ED0 (33B/12 instructions). Native ECX allocation,
// unused EDX, one stacked flags DWORD, RET4, EAX original allocation. Destroy
// pair at+C first; only AFTER it returns test the flags LOW BYTE bit0, and free
// the original allocation when set. Source keeps the byte read/order and adds
// stacked context. An exception skips the flag read, free and return.
void* __fastcall scalar_delete_native_input_device_pair_node_006a6eb0(
    void* node, std::uint32_t unused_edx, std::uint32_t flags,
    NativeStringRawPoolContext& strings);

// Parent-bound logical action services, not original binary funclet entries.
// Native record actions recover [runtime EBP-18h], pair action[EBP-10h], add
// the listed member offset and tail-JMP the actual child. Source takes that
// raw saved parent receiver explicitly; it never reads a fabricated EBP/frame.
// All are ordinary propagating services when called directly. The private
// CurrentCpp unwind guards update state BEFORE each action and are noexcept,
// so a secondary C++ exception during unwinding terminates in the current CRT.
// Normal child calls are outside those noexcept guards and can propagate.
void unwind_native_input_device_record_00c6dc30(void* record,
    NativeStringRawPoolContext& strings); // +00 ->55A3F0
void unwind_native_input_device_record_00c6dc38(void* record,
    NativeStringRawPoolContext& strings); // +0C ->55AAB0
void unwind_native_input_device_record_00c6dc43(void* record,
    NativeStringRawPoolContext& strings); // +18 ->55AAF0
void unwind_native_input_device_record_00c6dc4e(void* record,
    NativeStringRawPoolContext& strings); // +24 ->445900
void unwind_native_input_device_record_00c6dc59(void* record,
    NativeStringRawPoolContext& strings); // +30 ->4324A0
void unwind_native_input_device_record_00c6dc64(void* record,
    NativeStringRawPoolContext& strings); // +40 ->4324A0
void unwind_native_input_device_record_00c6dc6f(void* record,
    NativeStringRawPoolContext& strings); // +50 ->55B490
void unwind_native_input_device_record_00c6dc7a(void* record,
    NativeStringRawPoolContext& strings); // +5C ->55B110
void unwind_native_input_device_record_00c6dc85(void* record,
    NativeStringRawPoolContext& strings); // +6C ->55A430
void unwind_native_input_device_pair_00c6dca0(void* pair,
    NativeStringRawPoolContext& strings); // +00 ->41DD20

// Native FH3 maps/actions and secondary-C++-failure termination routing are
// recovered evidence. These CurrentCpp guards do not reproduce original FH3
// registration, RTTI/throw matching, foreign SEH/managed exceptions, fault or
// stack-exhaustion behavior, private aliases, register/flag residue, original
// heap/CRT/terminate-handler identity, binary ABI, runtime or game validation.
// RawCtx borrows the actual pool publication/gate/manager; allocation ownership
// must match current CRT frees. No callbacks, providers, host tree model or
// NativeStringStorage noexcept bridge is introduced. Older AV domains remain.
} // namespace bsp
