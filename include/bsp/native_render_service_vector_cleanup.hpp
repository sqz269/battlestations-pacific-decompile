#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Complete B14500..B14548: native ECX actual vector, no stack arguments,
// plain RET, no semantic result. The service embeds this vector at +68C;
// prefix+0 is untouched, begin/end/capacity are +4/+8/+C. Capture the end,
// destroy ascending 12Ch records through full B10740, then free the CURRENT
// begin pointer and clear the three fields in order. Exceptions escape before
// that free/clear sequence. Null begin still clears all three fields.
void destroy_native_renderer_record_vector_00b14500(void* actual_vector,
    NativeStringRawPoolContext&);

// Complete B14590..B14594: forwarding JMP to B14500, same original interface.
void destroy_native_renderer_record_vector_thunk_00b14590(void* actual_vector,
    NativeStringRawPoolContext&);

// Complete 432050..432087: native ECX begin, EDX end, two unused DWORD stack
// arguments, RET8. Ascending actual8h headers, pointer equality and DWORD wrap.
// Capture data then current length+1 before EACH actual419CC0/BD1510 return;
// a null data pointer skips the length read and the getter. Headers stay as-is.
// The existing typed global_config fragment and its STL name remain separate.
void destroy_native_string_header_range_00432050(void* begin, void* end,
    NativeStringRawPoolContext&);

// Complete 4324A0..4324DC: native ECX actual vector, no stack arguments,
// plain RET. Service member +69C uses the same +4/+8/+C raw layout. Capture
// endpoints, destroy the range, reread begin for free, then clear the fields.
void destroy_native_string_header_vector_004324a0(void* actual_vector,
    NativeStringRawPoolContext&);

// Borrow the application's actual string-pool publication/gate/manager domain.
// All reached storage and CRT allocations must be valid. No semantic owner,
// resolver, callback, general STL port or replacement allocator is introduced.
// These C++ interfaces do not preserve native entry/stack/FH3 binary ABI;
// unrestricted SEH, private stack aliases, concurrency and gameplay are open.
} // namespace bsp
