#pragma once
#include "bsp/native_mesh_owner.hpp"
namespace bsp {
// Reuse the actual 0Ch vector / 8-byte length+pointer element declarations.
using NativeStringVectorStorage = NativeMeshWeightNamesStorage;
// ECX vector; stack signed capacity; RET4. Minimum1, pooled deep copies,
// forward old-name cleanup, old backing free then publication. The original
// one-state exception action calls a bare RET: no allocated-copy rollback.
void reserve_native_string_vector_00426520(NativeStringVectorStorage&,
    std::int32_t capacity,NativeStringStorage&);
// ECX vector; stack signed count; RET4. Grow via426520, zero new headers;
// shrink backwards, decrementing live count before each string release.
void resize_native_string_vector_00427110(NativeStringVectorStorage&,
    std::int32_t count,NativeStringStorage&);
// Full 4D0FA0 including disk tail: resize0 then free current backing; RET.
void destroy_native_string_vector_004d0fa0(NativeStringVectorStorage&,NativeStringStorage&);
// Full4CDC20: ECX vector, stack source string, RET4. Only count==capacity
// grows to signed max(2*capacity,1); zero fresh header, deep-copy source,
// increment live count after copy. Sole native unwind action is bare RET.
void append_native_string_vector_004cdc20(NativeStringVectorStorage&,const NativeString&,NativeStringStorage&);
} // namespace bsp
