#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource-cache erase requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;
struct SingletonLifetimeCallbacks;

// Actual 8-byte iterator, passed by value to native B7FA60.
struct NativeResourceCacheIteratorStorage { void* owner; void* node; };
static_assert(sizeof(NativeResourceCacheIteratorStorage) == 8);

// Complete B7CB70[28]/B7CB90[27]: ECX node, EAX maximum/minimum, RET.
// Follow current right/left links until the child's actual nil byte +19 is set.
void* maximum_native_resource_cache_node_00b7cb70(void* node) noexcept;
void* minimum_native_resource_cache_node_00b7cb90(void* node) noexcept;

// Complete B7CE80[99]: ECX actual owner/node iterator, RET; no uniform EAX
// result. Null-owner CRT validation can return; reload node afterward. A nil
// node invokes the second CRT boundary and returns without a later node store.
void increment_native_resource_cache_iterator_00b7ce80(void* actual_iterator,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Complete B7FA60[716]: ECX tree; stack(output, input owner, input node);
// EAX output, RET0Ch. Tree+4=head,+8=count; actual nodes are 1Ch bytes with
// links0/4/8, keyC/10, borrowed resource14, color18,nil19. Sentinel input throws
// owning NativeHardwareLayoutInvalidIterator with the native 27-byte message.
// Advance the local input, transplant/rebalance, return original key storage,
// free original node, decrement current nonzero count, publish owner then node.
// No owner==tree requirement, mapped-resource release, or failure rollback.
void* erase_native_resource_cache_iterator_00b7fa60(void* actual_tree,
    void* actual_iterator_output, NativeResourceCacheIteratorStorage input,
    NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Complete B801C0[77]: ECX actual manager; stack(name header), RET4; no EAX
// result contract. The actual tree is manager+14h. Find one equivalent key,
// validate the captured owner, compare against the captured current head, and
// erase if present. No resource-pointer match precondition or release policy.
void erase_native_resource_manager_name_00b801c0(void* actual_manager,
    const void* actual_name_header, NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& invalid_parameters);

// These source interfaces add explicit services and host C++ error transport;
// they are not original register/stack/FH3/SEH ABI bridges. Native exception
// payload ownership is reused; no projected tree or manager lifetime is added.
// Full evidence, current-read details and limits: NATIVE_RESOURCE_CACHE_ERASE_BP.md.
} // namespace bsp
