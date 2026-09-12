#pragma once
#include <cstdint>

namespace bsp {
// Actual Win32 {base,count,capacity} header. Counts are signed, not end pointers.
// Borrow existing native storage; do not place an independently owning std::vector
// beside it. The routines below require valid, addressable native container ranges.
struct NativeInputArrayHeader {
    void* base;
    std::int32_t count;
    std::int32_t capacity;
};
static_assert(sizeof(NativeInputArrayHeader) == 12);
struct NativeInputBindingStorageContext {
    const volatile std::uint32_t& one_00d7a24c;
};

// Native ECX header, signed count/capacity on stack, RET4; no result. The two
// DWORD specializations have identical behavior but distinct native entrypoints.
void reserve_native_input_dwords_0086a220(void*, std::int32_t);
void resize_native_input_dwords_0086a430(void*, std::int32_t);
void reserve_native_input_context_words_00696d80(void*, std::int32_t);
void resize_native_input_context_words_00696e70(void*, std::int32_t);
void reserve_native_input_modifiers_00696de0(void*, std::int32_t);
void resize_native_input_modifiers_00697220(void*, std::int32_t);

// Native ECX destination header, borrowed source header on stack, EAX dest,
// RET4. These clear destination count first, even when source aliases it.
void* assign_native_input_context_words_00a92e70(void*, const void*);
void* assign_native_input_modifiers_00a92ee0(void*, const void*);

// Actual34h records. Constructor/copy writes preserve untouched padding bytes;
// modifier records are actual14h, including their otherwise unmodeled fifth word.
// Copy: ECX destination, stack source, EAX destination, RET4. Scale uses FLD/FSTP.
void* copy_native_input_binding_00a93100(void*, const void*);
void reserve_native_input_bindings_00a93220(void*, std::int32_t);
void resize_native_input_bindings_00a93500(void*, std::int32_t,
    const NativeInputBindingStorageContext&);
void append_native_input_binding_00a93440(void*, const void*);
void* assign_native_input_bindings_00a937e0(void*, const void*,
    const NativeInputBindingStorageContext&);

// These are explicit-service source APIs, not original stack/SEH replacements.
// Allocation/free use the existing CRT boundary. Placement cleanup00401130 is
// an actual no-op: a failed reserve copy does not reclaim the unpublished new
// array or its completed elements. Do not add a stronger rollback guarantee.
} // namespace bsp
