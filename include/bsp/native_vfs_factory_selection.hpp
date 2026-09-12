#pragma once

#include <cstdint>

namespace bsp {
struct SingletonLifetimeCallbacks;

// BDB078 supplies the current factory table's captured slot4, actual factory
// ECX and the original system/virtual raw8h headers. The target's native RET8
// consumes both arguments. Bind real reconstructed factories or actual ABI
// entries; a null result means that this factory declined this input.
class NativeVfsFactoryCreateDispatch {
public:
    virtual ~NativeVfsFactoryCreateDispatch() = default;
    virtual void* factory_create(std::uintptr_t captured_entry,
        void* actual_factory, const void* system_header,
        const void* virtual_header) = 0;
};

// Complete BDB040..BDB094 (85 bytes). Native ECX actual manager, stack
// system/virtual headers, EAX first nonnull result or zero, RET8.
// Actual list is manager+30: head+4/count+8, nodes next/previous/factory +0/+4/+8.
// Capture initial head->next once; reload current head for every comparison,
// and after a null callback advance from the captured cursor's CURRENT next.
// Returning validation resumes without replacing that cursor. The CMP EDI,EDI
// validation at BDB057 is unreachable; no corresponding source call is added.
// No factory retain/release, allocation, list mutation or input normalization.
// Explicit-service C++ interface; no original ABI/FH3/SEH or game proof.
void* select_native_vfs_factory_00bdb040(void* manager,
    const void* system_header, const void* virtual_header,
    NativeVfsFactoryCreateDispatch&, const SingletonLifetimeCallbacks&);
} // namespace bsp
