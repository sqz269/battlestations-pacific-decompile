#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource-manager tree helpers require MSVC Win32.
#endif

namespace bsp {
struct SingletonLifetimeCallbacks;

// Complete B7E000..B7E036[55] and B7E050..B7E086[55]. No native input is
// consumed (caller ECX is ignored); EAX actual1Ch allocation; RET. Allocate
// through canonical BF681B semantics, then conditionally zero each computed
// link address0/4/8 separately, write color+18=1 and normal nil+19=0. KeyC/10,
// mapped pointer14 and tail1A/1B retain allocation bytes. There is no vtable.
// These prepare raw head storage; caller later sets nil=1 and self-links.
// No local EH frame, cleanup, null-allocation return policy or resource retain.
void* allocate_native_resource_parser_head_storage_00b7e000();
void* allocate_native_resource_cache_head_storage_00b7e050();

// Complete B7DF40..B7DF92[83]: ECX actual parser tree; stack(name header);
// EAX candidate node, RET4. Tree+4=head; head+4=root. Actual nodes1Ch use
// links0/4/8, keyC/10, borrowed parser14, color18,nil19. Empty stored length
// sorts before nonempty; otherwise use CRT case-insensitive C-string order.
// No length tie-break, pointer normalization, tree validation or parser retain.
void* lower_bound_native_resource_parser_name_00b7df40(void* actual_tree,
    const void* actual_name_header);

// Complete B7E740..B7E7A5[102]: ECX parser tree; stack(output iterator,name);
// EAX output, RET8. Lower-bound precedes the returning null-tree CRT boundary.
// Compare against CURRENT head, check candidate equivalence, reload head when
// selecting end, capture owner/node before output owner0 then node4 writes.
void* find_native_resource_parser_name_00b7e740(void* actual_tree,
    void* actual_iterator_output, const void* actual_name_header,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Complete B7CEF0..B7CF78[137]: ECX actual owner/node iterator; RET, no uniform
// EAX result. Reload node after returning null-owner CRT; sentinel selects its
// maximum before a possible returning CRT tail. Normal traversal preserves
// current iterator writes/reads and the final current-node nil check/tail.
void decrement_native_resource_parser_iterator_00b7cef0(void* actual_iterator,
    const SingletonLifetimeCallbacks& invalid_parameters);

// New source interfaces over actual caller storage; no std::map projection,
// manager construction/registration, string ownership or mapped-object policy.
// No original register/stack/FH3/SEH or native CRT exception ABI bridge.
// Evidence and fixture limits: NATIVE_RESOURCE_MANAGER_TREES_BQ.md.
} // namespace bsp
