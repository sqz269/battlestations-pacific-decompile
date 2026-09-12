#pragma once

#include <cstdint>

namespace bsp {
class ActualNativeStringPoolStorage;

// Borrow the application's actual raw singleton manager and factory cells.
// Factory storage is exactly8h: primary CFEA14, lifetime secondary+4 CFEA10.
struct NativeMpkgFactoryContext {
    void* volatile& actual_manager_publication_01090aa0;
    void* volatile& actual_factory_publication_010904f4;
};

// Complete736A90[206]: first-publication fast return; otherwise capture the
// first manager's actual section+10, enter/recheck/allocate/publish, capture the
// factory+4 argument BEFORE the second manager getter, then register it. A
// registration failure retains publication/allocation and releases the lock.
void* get_native_mpkg_factory_00736a90(NativeMpkgFactoryContext&);

// Complete737090[58], including raw7370C1..C3 ADD ESP,4. Original ECX primary,
// stacked flags, EAX original address, RET4. Clear publication, write secondary
// CE3818 then primary CFE9F4; bit0 frees. No unregister or provider ownership.
void* delete_native_mpkg_factory_00737090(void* actual_primary,
    std::uint32_t flags, NativeMpkgFactoryContext&) noexcept;
// Complete735D00[8]: SUB ECX,4/JMP737090, inherited RET4.
void* delete_native_mpkg_factory_secondary_00735d00(void* actual_secondary,
    std::uint32_t flags, NativeMpkgFactoryContext&) noexcept;
// Complete raw735800..735828[41], temporary CFE9FC scalar deleter. ECX is
// the base allocation itself; no -4 adjustment. Clear publication, write only
// CE3818 at that owner, optionally free it, return captured owner with RET4.
void* delete_native_mpkg_factory_base_00735800(void* actual_base,
    std::uint32_t flags, NativeMpkgFactoryContext&) noexcept;

// Explicit archive boundary. Supply complete actual BB9920/BB9C10 operations;
// this interface does not substitute the bounded semantic MPKG parser. Native
// construction receives actual34h storage in ECX and the ORIGINAL actual8h
// system-name header on stack (RET4), returns the stored archive pointer in EAX.
// Destruction receives the captured actual archive in ECX, consumes no stack
// arguments and does not free its allocation. Original FH3/SEH is separate.
class NativeMpkgArchiveOperations {
public:
    virtual ~NativeMpkgArchiveOperations() = default;
    virtual void* construct_00bb9920(void* actual_archive,
        const void* actual_system_name) = 0;
    virtual void destroy_00bb9c10(void* actual_archive) = 0;
};
struct NativeMpkgProviderContext {
    ActualNativeStringPoolStorage& strings;
    NativeMpkgArchiveOperations& archives;
};

// Complete BB9CB0[113]: raw18h owner. BB5590 copies the original name, then
// D64390; allocate34h/call archive construction/store returned pointer+14.
// Archive construction failure frees its captured allocation then destroys
// the completed base. Caller still owns the provider allocation. RET4/EAX owner.
void* construct_native_mpkg_provider_00bb9cb0(void* actual_owner,
    const void* actual_system_name, NativeMpkgProviderContext&);
// Complete BB9D90[211]: factory and virtual-name inputs unused. Unsigned length
// >5 gates the actual469840 suffix(start=length-5,count=7FFFFFFF), then425850
// compares against D1D818 ".mpkg". Release suffix BEFORE allocating18h and
// constructing. Forward the original header; do not normalize or validate it.
// Native ECX factory, system/virtual headers stacked, RET8, EAX provider/null.
void* create_native_mpkg_provider_00bb9d90(void* actual_factory,
    const void* actual_system_name, const void* unused_virtual_name,
    NativeMpkgProviderContext&);
// Complete BB9E70[99]: write D64390; capture+14, destroy/free if nonnull, then
// BB5380 base cleanup. Preserve stale+14 and name words. Archive failure still
// destroys the base; it does not free the archive or provider. Native RET0.
void destroy_native_mpkg_provider_00bb9e70(void* actual_owner,
    NativeMpkgProviderContext&);
// Complete BB9EE0[30]: destroy first; successful flags bit0 frees captured owner.
// Native ECX owner, stacked flags, EAX original address even when freed, RET4.
void* delete_native_mpkg_provider_00bb9ee0(void* actual_owner,
    std::uint32_t flags, NativeMpkgProviderContext&);

// New C++ interfaces, not drop-in ABI/FH3 replacements. Existing string release
// is noexcept: throwing native lazy-pool cleanup remains outside that contract.
// No private pool/domain, implicit shutdown dispatch or archive-load proof.
// Descriptive names are hypotheses; see docs/NATIVE_MPKG_PROVIDER.md.
} // namespace bsp
