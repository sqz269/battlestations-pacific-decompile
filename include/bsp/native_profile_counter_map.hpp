#pragma once
#include "bsp/native_profile_collections.hpp"

namespace bsp {
// 5070C0 normal raw map indexing contract: ECX map, stack NativeString* key,
// EAX mapped DWORD*, RET4. Existing equivalent keys retain their stored value;
// missing keys copy the supplied header and insert mapped0 into a1Ch tree node.
// Reuses existing source tree mechanics/string/CRT storage. Node flags18/19;
// header allocator/head/count0/4/8. Valid owned topology and stable key ordering
// required. Original private-stack aliases, validation-fault behavior and FH3
// exception ABI are not exposed by this C++ interface.
std::uint32_t* index_native_profile_counter_005070c0(void* actual_tree,
    const void* key_header,NativeStringStorage&,NativeProfileCollectionCalls&);
} // namespace bsp
