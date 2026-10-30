#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Actual0Ch {data0,signed count4,signed capacity8}, with actual8h {length0,
// data4} elements. NOT the checked10h begin4/end8/capC vector. Caller supplies
// genuine initialized backing and SAME live AA8/AA4/AA0 context; these APIs
// neither construct a vector/manager nor cast a NativeString/container owner.
// All reached header/buffer/provider storage must stay valid and externally
// serialized, including across callbacks and partial failure. No native ABI,
// private-frame/fault/FH3/SEH, recovery, loading or startup claim is supplied.

// 426520: native ECX vector, signed capacity stack, RET4. Minimum1; captured
// new backing, CURRENT count/data copy and forward return, CURRENT old backing
// free BEFORE publishing new data/capacity; count untouched. Bare-RET unwind
// adds no allocated-copy rollback or element/header cleanup.
void reserve_native_string_vector_00426520(void* actual_vector0c,
    std::int32_t requested_capacity, NativeStringRawPoolContext&);

// 427110: native ECX vector, signed requested count stack, RET4. Grow through
// reserve and zero only new headers using CURRENT data each iteration. Shrink
// count DWORD-RMW-- BEFORE each reverse return; then publish requested count.
void resize_native_string_vector_00427110(void* actual_vector0c,
    std::int32_t requested_count, NativeStringRawPoolContext&);

// 4CDC20: native ECX vector, borrowed raw8h source stack, RET4. Capture cap,
// grow ONLY on count==cap to signed max(wrapped2cap,1); CURRENT count/data form
// destination after reserve. Self-CMP precedes zeroing/copy; count RMW++ only
// after copy returns (or computed-null destination skips it). Source must
// remain genuinely readable across reserve; no protective source copy exists.
void append_native_string_vector_004cdc20(void* actual_vector0c,
    const void* actual_source_header8, NativeStringRawPoolContext&);

// 4D0FA0 full23B: resize0, then free CURRENT backing even when null. Leave data
// and capacity stale; no retry/dedup guard or implicit C++ destructor follows.
void destroy_native_string_vector_004d0fa0(
    void* actual_vector0c, NativeStringRawPoolContext&);

// Ordinary MSVC Win32 overloads; old typed interfaces remain unchanged. Real
// matched singleton heap and raw pool services only. All C++ escapes propagate
// with partial/unpublished custody retained, no rollback/free/clear/retry or
// manufactured completion/receipt. See NATIVE_STRING_VECTOR_RAW_STORAGE.md.
} // namespace bsp
