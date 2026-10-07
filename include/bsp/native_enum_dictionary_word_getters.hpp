#pragma once

#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {

// Complete normal 0048E960/0048E840 getters, with new Source interfaces.
// Borrow an actual Win32 owner whose dictionary receiver begins at +4, and a
// closed ASCII CString. Construct one owning temporary eight-byte key header,
// find its node, capture the opaque DWORD at node+8, then release the temporary
// through the supplied real raw-pool context. Dictionary/map ownership is external.
//
// Admission additionally requires a known-found key, C-locale providers and the
// stable valid storage/empty-cell bindings of native_enum_dictionary_lookup.hpp.
// There is deliberately no missing-key/default/insert/type-conversion policy:
// native dereferences null+8 on a miss. Invalid/null CString, missing keys,
// allocation failure, alias/reentry/concurrency, native EH/SEH and original ABI
// are outside this Source normal-success binding.
std::uint32_t read_native_enum_table_word_0048e960(const void* actual_owner,
    const char* key, NativeStringRawPoolContext& strings,
    const char* empty_00e186ed, const char* empty_00e17654);
std::uint32_t read_native_enum_symbol_word_0048e840(const void* actual_owner,
    const char* key, NativeStringRawPoolContext& strings,
    const char* empty_00e186ed, const char* empty_00e17654);

} // namespace bsp
