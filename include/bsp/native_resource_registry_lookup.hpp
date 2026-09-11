#pragma once

namespace bsp {
struct SingletonLifetimeCallbacks;

// Borrow the actual read-only eight-byte factory profiles. Raw factory objects
// retain their native identities; every dispatch reads current object+0 and
// then current profile+4. Only the reached profile view must be usable.
// This supplies no factory or registry population.
struct NativeResourceRegistryLookupContext {
    const void* actual_factory_profile_00d64470;
    const void* actual_factory_profile_00d644ac;
    const void* actual_factory_profile_00d644e8;
    const void* actual_factory_profile_00d644f0;
    const SingletonLifetimeCallbacks& invalid_parameters;
};

// Full B19E90 (108 bytes): ECX actual registry; stack native key header;
// EAX created resource or zero on miss; RET4. Registry+4 is the actual tree,
// tree+4 its current head; native 1Ch nodes hold the factory pointer at+14.
// Invoke complete B19D60, capture iterator owner then current tree head BEFORE
// returning CRT validation, read node AFTER it, and compare with captured head.
// The later check uses captured owner and its freshly read head. Read current
// node+14, factory+0 and profile+4; forward the creator's result unchanged.
void* create_native_registered_resource_00b19e90(void* actual_registry,
    const void* actual_key, const NativeResourceRegistryLookupContext&);

// Caller supplies live native registry/tree/nodes/factories, actual profile
// storage and the established returning CRT service. The four identities
// above and current BBC6F0/BBC810 selectors are the qualified source domain.
// Unknown/base pure-virtual identities or selectors raise invalid_argument
// only as SOURCE boundaries, not recovered native exception behavior.
// No profile copy, resource retain/cleanup, registry lifetime or construction.
// New C++ API; original binary ABI/SEH and game behavior remain unproved.
} // namespace bsp
