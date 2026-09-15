#pragma once

#include <cstdint>

namespace bsp {
struct NativeRetainedMemoryOwnerContext;
struct NativePhysicalStreamOpenContext;

// Borrow existing services, without adding ownership or a substitute provider.
// The context selected by the current profile must be nonnull; the other may
// be null. Memory profile words are borrowed through its existing context;
// physical D691B0 words must be readable at their actual numeric address.
struct NativeRawScalarReaderContext {
    NativeRetainedMemoryOwnerContext* memory;
    NativePhysicalStreamOpenContext* physical;
};

// Complete source bodies within current D642C0/BEF590 and D691B0/BF5030
// provider domains. All SIX interfaces below are context-bearing __cdecl,
// NOT native ABI thunks. Original scalar ABI: ECX stream, stack optional
// actual-count pointer, EAX DWORD / ST0 float, RET4. Reuses the incoming
// actual-count ARGUMENT SLOT as the four-byte read buffer: short-read tail
// retains pointer-address bits, including null-pointer zero bits. One read.
std::uint32_t __cdecl read_native_raw_dword_00be42e0(void* actual_stream,
    std::uint32_t* optional_actual, NativeRawScalarReaderContext&);
float __cdecl read_native_raw_float_00be4360(void* actual_stream,
    std::uint32_t* optional_actual, NativeRawScalarReaderContext&);

// Original ABI: ECX actual reader, stack nonnull budget pointer, RET4.
// Reader+0 is actual stream. Reload current slot34/44 and dispatch the two
// established profiles. Always pass a nonnull actual-count local and debit
// *budget modulo32 after normal provider return; no short-read rejection.
// BF02C0 retains FSTP32/FLD32 after BE4360's FLD32. No FP-mode changes.
std::uint32_t __cdecl read_native_raw_dword_and_debit_00bf0280(
    void* actual_reader, std::uint32_t* budget, NativeRawScalarReaderContext&);
float __cdecl read_native_raw_float_and_debit_00bf02c0(
    void* actual_reader, std::uint32_t* budget, NativeRawScalarReaderContext&);

// Original ABI: ECX wrapper, no stack args, RET; EAX DWORD or ST0 float.
// Actual wrapper -> node; node+8 -> reader; node+20 -> remaining budget.
// No null-node, attached/leaf, readiness, full-read, or ownership policy.
std::uint32_t __cdecl read_native_raw_node_dword_00be9a00(
    const void* actual_wrapper, NativeRawScalarReaderContext&);
float __cdecl read_native_raw_node_float_00be99d0(
    const void* actual_wrapper, NativeRawScalarReaderContext&);

// Unexpected current profiles/slots throw a SOURCE-domain boundary exception,
// not a native read error. Native invalid wrapper/node pointers still fault.
// Different source call frames change the numeric short-read pointer seed;
// exact original caller-stack-address parity and arbitrary profiles are not
// claimed. See NATIVE_RAW_SCALAR_READER_BH.md for evidence and source bindings.
} // namespace bsp
