#pragma once

#include <cstdint>

namespace bsp {
class ActualNativeStringPoolStorage;

// Borrow the application's actual publications; no semantic lifetime or pool.
struct NativeMpakFactoryContext {
    void* volatile& actual_manager_publication_01090aa0;
    void* volatile& actual_factory_publication_010904d4;
};

// Complete736B60[206]: raw8h CFEA20 primary/CFEA1C secondary+4. Capture the
// first manager's section; recheck publication under that section; capture
// published factory+4 BEFORE the second manager getter/registration. RET0.
void* get_native_mpak_factory_00736b60(NativeMpakFactoryContext&);
// Complete7370D0[58], ECX primary, flags stacked, EAX captured owner, RET4.
// Clear publication, write secondary CE3818 then primary CFE9F4; bit0 frees.
void* delete_native_mpak_factory_007370d0(void* actual_primary,
    std::uint32_t flags, NativeMpakFactoryContext&) noexcept;
// Complete735D30[8], SUB ECX,4/JMP7370D0, inherited RET4.
void* delete_native_mpak_factory_secondary_00735d30(void* actual_secondary,
    std::uint32_t flags, NativeMpakFactoryContext&) noexcept;
// Complete735840[41]: raw base input, no adjustment. Clear publication,
// write only CE3818, flags bit0 frees, EAX captured base, RET4.
void* delete_native_mpak_factory_base_00735840(void* actual_base,
    std::uint32_t flags, NativeMpakFactoryContext&) noexcept;

// Explicit actual-provider boundaries. Construction takes raw44h in ECX and
// the ORIGINAL actual8h name header on stack, returns EAX, RET4. Cache
// replacement takes ONLY the actual8h name header in ECX and returns RET0;
// it does not receive the cached owner captured by BB83A0. Implement with
// the complete actual provider/parser operations, never a semantic fallback.
class NativeMpakProviderOperations {
public:
    virtual ~NativeMpakProviderOperations() = default;
    virtual void* construct_00bb8240(void* actual_owner,
        const void* actual_system_name) = 0;
    virtual void replace_cached_00bb82f0(const void* actual_system_name) = 0;
};
struct NativeMpakCreateContext {
    ActualNativeStringPoolStorage& strings;
    NativeMpakProviderOperations& providers;
    void* volatile& actual_lock_publication_010904e0;
    void* volatile& actual_cached_provider_010904dc;
};

// CompleteBB83A0[375], ECX unused factory, system/unused virtual headers
// stacked, EAX provider/null, RET8. Unsigned length>4 gates native substring
// and case-insensitive .mpak comparison; suffix cleanup precedes lock reads.
// Enter/increment captured lock, reload current lock, capture cache, decrement
// and leave reloaded lock. A cached owner causes BB82F0(empty temporary header)
// BEFORE returning that captured owner. Otherwise allocate44h/construct using
// the original header. No added reference, lock null check or cache repair.
void* create_native_mpak_provider_00bb83a0(void* actual_factory,
    const void* actual_system_name, const void* unused_virtual_name,
    NativeMpakCreateContext&);

// New source ABI; original FH3/SEH, compiler-spill aliases and game execution
// remain unvalidated. Existing actual string release is noexcept, excluding
// throwing native lazy-pool cleanup. See docs/NATIVE_MPAK_FACTORY.md.
} // namespace bsp
