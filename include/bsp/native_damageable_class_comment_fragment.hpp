#pragma once
#include "bsp/native_lua_objects.hpp"

namespace bsp {

// Borrow the enclosing 0087CA80 invocation's actual stable storage. Offsets are
// from ESP after its prologue and four saved registers, not this C++ frame.
// Unique is already live in parent state 1 and stays live after this fragment,
// including on C++ failure. Its actual tracked index may change during cleanup.
struct NativeDamageableClassCommentFragmentScratch {
    NativeLuaObjectStorage& live_unique_at_parent_94;
    void* fresh_field_at_parent_44; // aligned 14h bytes; previous Mesh is dead
    void* fresh_string_at_parent_10; // aligned 8h bytes; no live string on entry
};

// The 0087CBCF..0087CC65 Comment fragment of 0087CA80, including lookup setup;
// CBCF is an interior instruction, not an original entrypoint. Borrow the actual
// descriptor (NativeString at +58h/+5Ch), live Lua row, parent scratch, current
// Lua owner and real string-pool publication cells. Descriptor, row, Unique,
// both scratch regions and publication cells must not overlap; scratch addresses
// stay stable. All owner/tracking and capacity contracts of the existing
// native_lua_get_by_name_protected apply, including stable owner/index across
// callbacks and the inherited error handler's stable position. Unique remains
// registered with the actual owner of this parent invocation; its enclosing
// owner cleans it after subsequent fields or after propagated fragment failure.
// Do not copy row/Unique/tracked field or replay a partly completed invocation.
//
// Lookup/string/resize/copy/cleanup execute in native order. Header changes
// survive failure; state 5->4->1 is lowered before each cleanup, preventing
// retries. Exact Lua STRING is selected; every other type uses the original
// empty-string default. Embedded NUL ends the C string. No numeric coercion,
// descriptor rollback or model/resource loading is added.
//
// Explicit MSVC Win32 source ABI. Lookup uses the existing protected C++ error
// transport. Original stack/register/FH3 ABI, Lua longjmp, SEH/faults and double
// exceptions are not reproduced. Unwind cleanup has the ordinary C++ noexcept
// destructor boundary; a second cleanup failure terminates.
void read_native_damageable_class_comment_fragment_0087cbcf(
    void* actual_descriptor, NativeLuaObjectStorage& actual_row,
    NativeDamageableClassCommentFragmentScratch& actual_scratch,
    NativeStringRawPoolContext& actual_strings);

} // namespace bsp
