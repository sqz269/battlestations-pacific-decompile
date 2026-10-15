#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Complete normal admitted-range effects of 006A29C0..006A29F8. Original
// cdecl(first,last,destination), three physical pointer words, plain RET.
// Headers are actual live 10h-byte checked string vectors: opaque/proxy+0,
// begin+4,end+8,capacity_end+C; inner strings occupy8 bytes each. This is
// assignment into existing owning headers, not construction into raw backing.
//
// Forward native32 cursor+10h/equality loop; call complete actual00506C80 on
// destination=source_cursor+(captured destination-first), then advance source.
// No source snapshot or overlap repair: prior writes may affect later reads.
// Cache and return native32 destination+((SAR4(last-first))<<4) before calls.
// Empty returns destination and touches no pointed storage. All arithmetic is
// DWORD arithmetic, including wrap, without C++ out-of-array pointer arithmetic.
void* __cdecl copy_native_checked_string_vector_range_006a29c0(
    const void* first, const void* last, void* destination,
    NativeStringRawPoolContext& strings);

// Complete normal admitted-range effects of006A2A00..006A2A25. Original
// cdecl(first,last,value), three physical pointer words, plain RET. Capture
// value POINTER once on nonempty entry; each assignment reads its CURRENT
// actual owning header through00506C80, so aliases retain forward behavior.
// Empty touches no pointed storage; native32 cursor+10h/equality termination.
//
// Source void follows observed callers, which do not consume a semantic result.
// Native never writes EAX itself: empty preserves incoming ambient EAX;
// nonempty leaves last00506C80 destination result. Those private register bits
// are documented, not promoted to an extra Source semantic input or promised
// by this compiler-generated void entry.
void __cdecl fill_native_checked_string_vector_range_006a2a00(
    void* first, const void* last, const void* value,
    NativeStringRawPoolContext& strings);

// Both new Source entries have an extra explicit actual pooled-string context
// and a different physical ABI. Native16-byte stepping must reach last, with
// every executed assignment admitted by actual header/string/pool and backing
// ownership contracts. Source/value may alias destinations where those actual
// accesses remain valid; no host vector, callback or ownership stand-in exists.
// Current child CRT/EH/diagnostic/zero-copy limits are inherited. No local EH
// rollback/cleanup is added: earlier assignments remain if a child throws.
// Private native/Source frame aliases, original FH3/SEH/CRT identity, ambient
// flags and full original binary ABI are outside these interfaces.
} // namespace bsp
