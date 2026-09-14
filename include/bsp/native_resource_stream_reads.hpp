#pragma once
#include <cstdint>
namespace bsp {
class NativeAdoptedSubstreamDispatch;
struct NativeStringRawPoolContext;
struct NativeResourceStreamReadContext {
    NativeStringRawPoolContext& strings;
    NativeAdoptedSubstreamDispatch& streams;
    void* actual_empty_string_storage_0109db64;
};

// Actual ECX stream, stacked optional actual-count pointer, EAX DWORD, RET4.
// Each body uses the incoming pointer's bits as the initial read-buffer word.
// One CURRENT slot24 read4 overwrites only bytes the stream supplies. Partial
// reads preserve the remaining pointer bytes; null actual supplies a zero seed.
std::uint32_t read_native_stream_dword_slot34_00be42e0(void* stream,
    std::uint32_t* actual, NativeAdoptedSubstreamDispatch&);
std::uint32_t read_native_stream_dword_slot38_00be4300(void* stream,
    std::uint32_t* actual, NativeAdoptedSubstreamDispatch&);

// ECX stream; stacked output8h/optional actual pointer; EAX output; RET8.
// Read length through CURRENT slot38; make a space-filled temporary, read once,
// report prefix+payload actual before constructing output, copy captured temp
// fields, then return captured temp buffer. Zero length constructs empty output.
// Failure while returning the temp destroys completed output, not temp again.
void* read_native_stream_string_00be4620(void* stream, void* output,
    std::uint32_t* actual, NativeResourceStreamReadContext&);

// ECX actual reader (stream+0), stack budget, EAX DWORD, RET4. Local actual
// starts with reader-address bits; invoke CURRENT slot34, then wrap-debit budget.
std::uint32_t read_native_resource_dword_00bf0280(void* reader,
    std::uint32_t* actual_budget, NativeResourceStreamReadContext&);
// ECX reader, stacked output/budget, EAX captured output, RET8. The actual-count
// slot initially holds output-address bits. Invoke CURRENT slot48, ignore its
// return value and subtract reported count; no transfer/budget validation.
void* read_native_resource_string_00bf0510(void* reader, void* output,
    std::uint32_t* actual_budget, NativeResourceStreamReadContext&);

// Known scalar targets42E0/4300 and string4620 dispatch to complete source;
// unsupported reached targets throw. Raw reads use the borrowed existing
// dispatch, never numeric native code. No owner, pool or stream clone is made.
// New source ABI: original private stack aliases, FH3/SEH/CRT identity and
// hardware faults remain outside this interface. Storage must remain live.
}
