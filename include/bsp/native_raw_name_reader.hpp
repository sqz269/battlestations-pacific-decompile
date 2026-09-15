#pragma once
#include <cstdint>

namespace bsp {
struct NativeRawScalarReaderContext;
struct NativeStringRawPoolContext;

// Borrow existing real stream services and actual string-pool publications.
// Supported profiles are D642C0 and D691B0. Physical numeric table words and
// the native null-data fallback0109DB64 must be accessible when consumed.
struct NativeRawNameReaderContext {
    NativeRawScalarReaderContext& streams;
    NativeStringRawPoolContext& strings;
};

// BE4300 is byte-identical to BE42E0. Native ECX stream, stack optional count,
// RET4; short-read tails retain the incoming count-pointer word's bytes.
std::uint32_t read_native_raw_name_length_00be4300(void* actual_stream,
    std::uint32_t* optional_actual, NativeRawScalarReaderContext&);

// Full BE4620 in the qualified provider domain. Native ECX stream, stack
// actual8h output/optional count, RET8, EAX output. Fresh output semantics:
// zero header, resize/copy; never release an old output value. Payload short
// reads retain space20h fill. Publish actual count BEFORE output construction.
// Preserve captured temporary data/length and the native cleanup state order.
void* read_native_raw_name_00be4620(void* actual_stream, void* actual_output8h,
    std::uint32_t* optional_actual, NativeRawNameReaderContext&);

// Native ECX reader, stack output/non-null budget, RET8. Reader+0 is stream;
// debit actual bytes modulo32 only after normal provider return.
void* read_native_raw_name_and_debit_00bf0510(void* actual_reader,
    void* actual_output8h, std::uint32_t* budget, NativeRawNameReaderContext&);
// Native ECX node / wrapper, one output argument, RET4. No validity guards.
void* read_native_raw_node_name_00be9fe0(void* actual_node,
    void* actual_output8h, NativeRawNameReaderContext&);
void* read_native_raw_node_handle_name_00bea010(const void* actual_wrapper,
    void* actual_output8h, NativeRawNameReaderContext&);

// All five are context-bearing C++ interfaces, not original ABI thunks.
// New source call frames change numeric pointer seeds. Unsupported profiles
// or slots throw source-domain exceptions, not native read errors. No native
// FH3 metadata, caller-stack address or asynchronous-fault identity claim.
// Evidence: docs/NATIVE_RAW_NAME_READER_BI.md.
} // namespace bsp
