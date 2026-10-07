#pragma once

#include "bsp/native_string.hpp"

namespace bsp {

// Complete ordinary normal paths of 0048E8D0, with a new Source interface.
// Borrow an actual Win32 owner whose dictionary receiver begins at +4. Build
// one owning temporary eight-byte string header, find the key, capture whether
// its node is nonnull, and release the temporary through the real raw pool.
// The mapped word at node+8 is never read. Found and missing both return normally.
// Native returns Boolean AL; this interface does not assert canonical full EAX.
//
// Admission: valid closed ASCII CString (including empty), C-locale providers,
// a returning raw pool, and stable borrowed storage/actual distinct empty cells
// as required by native_enum_dictionary_lookup.hpp. Dictionary ownership,
// insertion, declaration identity, invalid/null inputs, allocation failures,
// alias/reentry/concurrency, native EH/SEH and original ABI remain external.
bool contains_native_enum_symbol_0048e8d0(const void* actual_owner,
    const char* key, NativeStringRawPoolContext& strings,
    const char* empty_00e186ed, const char* empty_00e17654);

} // namespace bsp
