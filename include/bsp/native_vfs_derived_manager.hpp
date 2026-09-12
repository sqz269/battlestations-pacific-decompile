#pragma once

#include <cstdint>

namespace bsp {
struct NativeVfsManagerLifetimeContext;
struct NativePhysicalFactoryContext;

// Bind both contexts to the same application's actual singleton manager and
// publications. The physical factory getter uses its existing raw-manager
// transport; this wrapper creates no separate lifetime domain or dispatch table.
// Complete BEDA60[79]: ECX A0h owner, RET0, EAX original owner. Build the complete
// base manager, write D68D04, get the actual physical factory and append it to the
// actual factory list. Getter/registration failure invokes BE1F60 on this owner.
void* construct_native_vfs_derived_manager_00beda60(void* actual_owner,
    NativeVfsManagerLifetimeContext&, NativePhysicalFactoryContext&);

// Complete BEDAC0[30]: ECX owner, stack DWORDflags, RET4, EAX original owner.
// Always run BE1F60; free only if bit0 set; return the original pointer after free.
void* delete_native_vfs_derived_manager_00bedac0(void* actual_owner,
    std::uint32_t flags, NativeVfsManagerLifetimeContext&);

// Descriptive source interfaces, not the original ABI/FH3/SEH implementation.
// The EH cleanup schedule is established from E02044/E0203C and CC74E0; original
// handler execution, simultaneous cleanup failures and game behavior unvalidated.
} // namespace bsp
