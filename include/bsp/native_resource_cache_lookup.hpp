#pragma once

namespace bsp {
struct SingletonLifetimeCallbacks;

// Actual caller-owned resource cache: tree+4=head, tree+8=count, head+4=root.
// Native1Ch nodes: links+0/+4/+8, key length/data+C/+10, borrowed resource+14,
// color+18, nil byte+19. Allocator word+0, count, colors and resources are not
// read by lookup. Input headers are actual8h length/data pairs, not projections.

// Complete B7DFA0..B7DFF2[83]. ECX tree, stack query, EAX node, RET4. Capture
// initial head/root; walk current links and nil bytes. Empty stored length sorts
// first; nonempty C strings compare through CRT _stricmp with no length bound
// or tie-break. Return the first key not less than query, or captured head.
void* lower_bound_native_resource_cache_name_00b7dfa0(void* actual_tree,
    const void* actual_query_header);

// Complete B7E7B0..B7E815[102]. ECX tree, stack(output,query), EAX output,
// RET8. Lower-bound runs BEFORE the null-tree BF6713 check, whose handler may
// return. Retain the candidate, compare it against CURRENT tree+4, then reject
// query<candidate through443D00. A miss rereads CURRENT head. Capture selected
// owner/node before publishing output+0 then output+4, including output aliases.
void* find_native_resource_cache_name_00b7e7b0(void* actual_tree,
    void* actual_iterator_output, const void* actual_query_header,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Explicit-service MSVC Win32 C++ APIs, not original stack/SEH ABI replacements.
// These bodies borrow all storage and perform no allocation, normalization,
// insertion, removal, resource reference operation or manager publication.
// Valid raw memory/current CRT locale are required; no new pointer validation.
// Arbitrary original stack aliases, concurrent mutation and gameplay unproved.
// Similar B19B90/B19D60 source is not used as an address alias.
} // namespace bsp
