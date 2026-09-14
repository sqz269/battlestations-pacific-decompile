#pragma once
#include "bsp/native_sampler_loader_context.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_string_pool_storage.hpp"

namespace bsp {
struct NativeSamplerOwnerLifetimeContext {
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& validation;
    NativeRenderActualOwners& owners;
    void* volatile& actual_owner_00f8d420;
    const void* actual_cache_profile_00ce7d08;
    const void* actual_cache_profile_00ce7d24;
};
struct NativeSamplerOwnerLifetimeOperation final {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    std::uint32_t function{},native_site{},flags{},resize_index{};
    void* owner{};
    void* secondary{};
    void* array{};
    void* resource{};
    void* record{};
    void* array_allocation{};
    NativeSamplerOwnerLifetimeContext* context{};
    bool release_started{},release_returned{},array_free_started{},array_free_returned{};
    bool owner_free_started{},owner_free_returned{},publication_cleared{},resize_name_armed{};
    NativeSamplerOwnerLifetimeOperation()=default;
    ~NativeSamplerOwnerLifetimeOperation();
    NativeSamplerOwnerLifetimeOperation(const NativeSamplerOwnerLifetimeOperation&)=delete;
    NativeSamplerOwnerLifetimeOperation& operator=(const NativeSamplerOwnerLifetimeOperation&)=delete;
    // Caller resolves retained storage/canonical-owner obligations first.
    // Frees nothing and never reverses an already-completed native cleanup.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// Full4DC410: ECX actual0Ch record vector; stack signed requested count; RET4.
// Reserve only when requested>current capacity signed; preserve raw32 indices,
// constructor's unspecified08/28, current data rereads, decrement-before-shrink
// destruction, final requested count. Growth failure cleans only current name;
// the placement unwind reads its arguments but owns no rollback operation.
void resize_native_sampler_records_004dc410(NativeResourceRecordVectorStorage&,
    std::int32_t requested,NativeSamplerOwnerLifetimeContext&,NativeSamplerOwnerLifetimeOperation&);
// Full4DDB40: unused ECX; stack nonnull actual resource; RET4. Real+04 decrement
// and zero-only canonical terminal dispatch through existing actual owner domain.
void release_native_sampler_resource_004ddb40(void*,NativeSamplerOwnerLifetimeContext&,
    NativeSamplerOwnerLifetimeOperation&);
// Full4DDA40: ECX actual secondary cache; RET. Current last resource release,
// then current last record destruction, current count decrement, resize0.
void clear_native_sampler_cache_004dda40(void*,NativeSamplerOwnerLifetimeContext&,
    NativeSamplerOwnerLifetimeOperation&);
// Full4DDAA0: ECX actual vector; RET. Resize0, free CURRENT data, leave stale
// data/capacity fields. This is also4DE290's state0 unwind cleanup.
void destroy_native_sampler_record_array_004ddaa0(NativeResourceRecordVectorStorage&,
    NativeSamplerOwnerLifetimeContext&,NativeSamplerOwnerLifetimeOperation&);
// Full4DE290: ECX secondary; RET. StampCE7D08, clear, disarm array cleanup,
// resize0 again and free current array. Failed clear performs the array cleanup.
void destroy_native_sampler_cache_004de290(void*,NativeSamplerOwnerLifetimeContext&,
    NativeSamplerOwnerLifetimeOperation&);
// Full4B4F10: ECX actual1Ch owner; RET. Unconditionally clear F8D420 then write
// CE3818. Same concrete effect on B1B680 normal and state0 unwind paths.
void destroy_native_sampler_owner_base_004b4f10(void*,NativeSamplerOwnerLifetimeContext&,
    NativeSamplerOwnerLifetimeOperation&);
// FullB1B680: ECX primary1Ch owner; RET. CE7D38/CE7D24, secondary destruction,
// unconditional current publication clear, CE3818; unwind also clears/stamps.
void destroy_native_sampler_owner_00b1b680(NativeSamplerLoaderSingletonStorage&,
    NativeSamplerOwnerLifetimeContext&,NativeSamplerOwnerLifetimeOperation&);
// Full4DE340: ECX primary; stack flags; EAX captured address; RET4. Free only
// after successful destruction and bit0. Full4DE360 adjusts secondary-4 then
// tail-forwards, retaining the same stacked flags and returned primary address.
void* delete_native_sampler_owner_004de340(void*,std::uint32_t flags,
    NativeSamplerOwnerLifetimeContext&,NativeSamplerOwnerLifetimeOperation&);
void* delete_native_sampler_owner_secondary_004de360(void*,std::uint32_t flags,
    NativeSamplerOwnerLifetimeContext&,NativeSamplerOwnerLifetimeOperation&);

// Actual raw owners/records and canonical companions only. A terminal must
// already be admitted to the SAME NativeRenderActualOwners domain and borrow
// the actual+04 atomic. Fresh BBC6F0/BBC810 sources still need concrete terminal
// companions; this module supplies no resource map, shadow count or fake delete.
// Null record resources are NOT skipped: the native release accesses raw+04.
// Exclude external owner retirement and diagnostic-frame mutation while running
// or failed; cleanup may free arrays/clear publication before recording failure.
// Existing pool noexcept and child orphan limits remain. New C++ ABIs, not
// native FH3/hardware-fault identities, full singleton-manager dispatch or game proof.
} // namespace bsp
