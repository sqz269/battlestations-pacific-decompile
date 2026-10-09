#pragma once
#include "bsp/native_lua_objects.hpp"

namespace bsp {

// Borrow the enclosing 0087CA80 invocation's actual stable storage. Offsets are
// from ESP after its prologue and four saved registers, not this C++ frame.
// Unique is already live in parent state 1 and stays live after this fragment,
// including on C++ failure. Its actual tracked index may change during cleanup.
struct NativeDamageableClassMeshFragmentScratch {
    NativeLuaObjectStorage& live_unique_at_parent_94;
    void* fresh_field_at_parent_44; // aligned 14h bytes; previous Name is dead
    void* fresh_string_at_parent_10; // aligned 8h bytes; no live string on entry
};

// The 0087CB44..0087CBCE Mesh fragment of 0087CA80, not an original entrypoint.
// Borrow the actual descriptor (NativeString at +38h/+3Ch), live Lua row,
// parent scratch, current Lua owner and real string-pool publication cells.
// Descriptor, row, Unique, both scratch regions and publication cells must not
// overlap; both scratch addresses stay stable. All owner/tracking and capacity
// contracts of native_lua_get_by_name_protected apply, including owner/index
// stability across callbacks and the inherited error handler's stable position.
// The enclosing owner keeps Unique registered and cleans it after subsequent
// fields, or after this fragment propagates failure. It must not copy the row,
// Unique or tracked field object, nor replay a partially completed invocation.
//
// Lookup/string/resize/copy/cleanup execute in native order. Header changes
// survive failure; state is lowered before each cleanup, preventing retries.
// Exact Lua STRING is selected; every other type uses the actual empty-string
// default. No numeric coercion or model/resource loading is added.
//
// Explicit MSVC Win32 source ABI. Lua lookup uses the existing protected C++
// status-to-exception transport. Original stack/register/FH3 ABI, Lua longjmp,
// SEH/faults and double exceptions are not reproduced. Unwind cleanup uses the
// ordinary C++ noexcept-destructor boundary; a second cleanup failure terminates.
void read_native_damageable_class_mesh_fragment_0087cb44(
    void* actual_descriptor, NativeLuaObjectStorage& actual_row,
    NativeDamageableClassMeshFragmentScratch& actual_scratch,
    NativeStringRawPoolContext& actual_strings);

} // namespace bsp
