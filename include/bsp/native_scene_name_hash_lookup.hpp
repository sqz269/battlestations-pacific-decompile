#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scene name-hash lookup requires MSVC Win32.
#endif

namespace bsp {

// Whole 00468660 [84B/30 instructions] and 00467EE0 [74B/27 instructions].
// Original ECX database, unused EDX, one DWORD at entry ESP+4, RET4; ESI saved.
// These ordinary interfaces bind actual input4 and private iterator12 backing;
// neither the Original nor the compiler's implicit argument/private stack is
// reproduced. The descriptive names are hypotheses, not recovered symbols.
//
// Only iterator.table0 is initialized to database+34h modulo32 before real
// Begin. Do not zero node4/index8. Reload node4 after Begin and every Next;
// read the input4 word only after the first nonnull node, then capture it once
// across all advances. Compare arbitrary DWORD bits at captured(node+8)+0.
//
// Keep bindings/Source metadata stable and separate from reached storage.
// Actual input4/private12 backing and all reached table/node/payload accesses
// must stay valid. Payload aliases with those actual cells are admitted; no
// implicit native/helper/compiler-frame, return-slot, saved-register or async
// aliases are asserted. No added null-payload/count/ownership guard is supplied.

// A match returns the already captured node+8 pointer, not the node and not a
// fresh node+8 read. Exhaustion returns raw zero. No nominal payload type or
// lifetime/ownership follows from this borrowed raw pointer result.
void* find_native_scene_name_hash_value_00467ee0(
    void* actual_database, const void* actual_parameter4, void* actual_iterator12);

// A match reads node+4 once; null selects the bound Original E18560 role.
// Borrow the existing GameNativeStringProcess::library_empty_00e18560() cell
// through actual_empty_00e18560; no owner/provider is created here. This body
// never reads that cell's byte. Exhaustion returns raw zero. A nonnull result
// asserts neither CString validity nor lifetime/ownership. Original writable
// DATA is not claimed universally immutable, nor are physical addresses equal.
void* find_native_scene_database_class_name_00468660(
    void* actual_database, const void* actual_parameter4, void* actual_iterator12,
    const void* actual_empty_00e18560);

// Current ordinary storage/provider composition only. Original register/stack,
// fault/SEH/exception transport, binary ABI and game identity remain unproved.

} // namespace bsp
