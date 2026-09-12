#pragma once

#include <cstdint>

namespace bsp {
class ActualNativeStringPoolStorage;
class NativeAdoptedSubstreamDispatch;
struct SingletonLifetimeCallbacks;

// Borrow the application's actual pool and stream dispatch. The raw provider
// is 2Ch bytes: provider base0..13, resident tree14..1F, pending tree20..2B.
// This creates no separate string-pool publication or lifetime-manager domain.
struct NativeFileStoreProviderLifetimeContext {
    ActualNativeStringPoolStorage& strings;
    NativeAdoptedSubstreamDispatch& streams;
    const SingletonLifetimeCallbacks& invalid_parameters;
};

// Complete BE7FA0[238]. Native ECX raw2Ch owner, EAX original owner, RET0.
// Construct the empty-name BB5590 base, install D689E8, and allocate/form the
// resident then pending sentinels. Tree+0 words remain untouched. On pending
// allocation failure, BE7BB0 destroys the resident tree, then BB5380 the base;
// the caller still owns the raw provider allocation. Native FH3/SEH is separate.
void* construct_native_file_store_provider_00be7fa0(
    void* actual_owner, NativeFileStoreProviderLifetimeContext&);

// Complete BE80B0[105]. Native ECX factory0Ch, EAX cached/new provider, RET0.
// Return captured cache+8 when nonnull. Otherwise allocate2Ch and construct;
// publish the returned pointer only after success. Constructor failure frees
// the captured allocation. No lock, reference increment or registration exists.
void* get_native_file_store_provider_00be80b0(
    void* actual_factory, NativeFileStoreProviderLifetimeContext&);

// Complete BE8120[43]. Native ECX factory, stack actual8h system/virtual name
// headers, RET8, EAX provider/null. A zero system length fails; otherwise compare
// against the verified CFF1FC literal "filestore" through425850, consuming AL.
// The virtual-name argument is unused. No null-header validation is added.
void* create_native_file_store_provider_00be8120(void* actual_factory,
    const void* actual_system_name, const void* unused_virtual_name,
    NativeFileStoreProviderLifetimeContext&);

// Complete BE7BF0[180], including raw BE7C4A..BE7CA3 originally outside Ghidra's
// stored body (repaired in AV). ECX owner, RET0. Clear resident entries, erase/free
// pending then resident heads, zero their head/count words, and destroy base.
// Native state2 cleans pending BE7A70, resident BE7BB0 and base on Clear failure;
// lower the state BEFORE each member's normal cleanup. Never free owner here.
void destroy_native_file_store_provider_00be7bf0(
    void* actual_owner, NativeFileStoreProviderLifetimeContext&);

// Complete BE8090[30]. Native ECX owner, stacked flags, RET4, EAX original
// owner. Destroy first; only successful destruction and flags bit0 free owner.
void* delete_native_file_store_provider_00be8090(void* actual_owner,
    std::uint32_t flags, NativeFileStoreProviderLifetimeContext&);

// These explicit-service C++ interfaces are not drop-in native ABI replacements.
// Normal raw-storage behavior and C++ unwind ordering are reconstructed. Native
// CRT/FH3 exception identity, SEH cleanup and double-failure cleanup are unproven;
// inherited noexcept string release excludes throwing lazy-pool recreation.
// Descriptive names are hypotheses. No game/runtime validation is claimed.
} // namespace bsp
