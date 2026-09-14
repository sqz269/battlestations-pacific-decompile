#pragma once

#include <cstdint>

namespace bsp {
class ActualNativeStringPoolStorage;
struct NativeVfsLookupRouteContext;

// Original provider boundaries: submit at +0Ch (ECX provider, four stack
// arguments, RET10h, raw AL) and tick at +28h (ECX provider, RET). Invoke the
// captured entry on the captured actual provider. No callback ownership,
// normalization, provider retention, queue or success policy is supplied here.
class NativeVfsProviderPendingDispatch {
public:
    virtual ~NativeVfsProviderPendingDispatch() = default;
    virtual std::uint8_t invoke_submit(std::uintptr_t captured_entry,
        void* actual_provider, const void* first_header, const void* second_header,
        std::uintptr_t opaque_callback, std::uint32_t flags) = 0;
    virtual void invoke_tick(std::uintptr_t captured_entry, void* actual_provider) = 0;
};

struct NativeVfsPendingRouteContext {
    NativeVfsLookupRouteContext& lookup;
    const void* actual_pending_profile_00d68478;
    NativeVfsProviderPendingDispatch& providers;
};

// Full FileStore leaves: BE7CB0[5] XOR AL,AL / RET10h; BE7CC0[1] RET.
// Read no provider/header/callback/flags and perform no side effects.
std::uint8_t decline_native_filestore_pending_00be7cb0(void* unused_provider,
    const void* unused_first, const void* unused_second,
    std::uintptr_t unused_callback, std::uint32_t unused_flags) noexcept;
void tick_native_filestore_pending_00be7cc0(void* unused_provider) noexcept;
// Complete MPKG leaves with the same no-read contracts: BB9D30[5] RET10h,
// raw AL0; BB9D40[1] RET. These are actual archive provider defaults.
std::uint8_t decline_native_mpkg_pending_00bb9d30(void* unused_provider,
    const void* unused_first, const void* unused_second,
    std::uintptr_t unused_callback, std::uint32_t unused_flags) noexcept;
void tick_native_mpkg_pending_00bb9d40(void* unused_provider) noexcept;

// Full BDC100[195]. ECX actual20h visitor; stack first/second/callback/flags;
// RET10h, EAX captured visitor. Zero/copy first+4 then second+0C as actual8h
// headers, install D68478, then flags+18, zero raw acceptance+14, callback+1C.
// Neither input is normalized. Identity comparisons occur after each header
// has been cleared. On failure only completed members are destroyed.
void* construct_native_vfs_pending_visitor_00bdc100(void* actual_visitor,
    const void* first_header, const void* second_header,
    std::uintptr_t opaque_callback, std::uint32_t flags,
    ActualNativeStringPoolStorage&);

// Full BDB740[122]. ECX visitor, RET. Release current second then current
// first header through the actual pool, then install base D68380. Headers,
// flags, raw result and callback remain otherwise untouched. No cancellation.
void destroy_native_vfs_pending_visitor_00bdb740(void* actual_visitor,
    ActualNativeStringPoolStorage&);

// Full BDC1D0[4]. ECX visitor, RET, AL=current raw byte+14.
std::uint8_t read_native_vfs_pending_result_00bdc1d0(const void* actual_visitor) noexcept;
// Full BDC1E0[40]. ECX visitor; stack mount payload/unused traversal suffix;
// RET8. Capture payload+8 provider and its current +0C entry. Pass complete
// visitor headers+4/+0C, callback+1C and flags+18; publish returned raw AL+14
// after the call, even when provider code changes that byte or the visitor.
void read_native_vfs_pending_provider_00bdc1e0(void* actual_visitor,
    const void* actual_mount_payload, const void* unused_suffix,
    NativeVfsPendingRouteContext&);

// Full BDDA10[135]. ECX captured manager; stack first/second/callback/flags;
// RET10h, AL acceptance. Construct actual stack visitor, traverse with the
// original SECOND header through existing BDD0A0, capture raw+14, destroy.
// lookup.pending must bind this context; the traversal rereads D68478 profile
// slots at each visit/stop boundary. No callback-null or name validation added.
std::uint8_t submit_native_vfs_pending_00bdda10(void* actual_manager,
    const void* first_header, const void* second_header,
    std::uintptr_t opaque_callback, std::uint32_t flags,
    NativeVfsPendingRouteContext&);

// Full BDB0B0[108]. ECX manager, RET. Walk actual manager tree+3C/head+40
// beginning at head.left; call current node+18 provider's current slot+28,
// then advance the existing iterator. Repeated providers and tree mutations
// remain observable. No snapshot, deduplication, reentrancy guard or retention.
void pump_native_vfs_pending_00bdb0b0(void* actual_manager,
    NativeVfsPendingRouteContext&);

// Complete source bodies in the existing actual string bridge's returning
// getter domain. Its noexcept release excludes throwing-getter unwind parity.
// Explicit C++ contexts are not original x86 ABI/FH3/SEH replacements. Concrete
// provider dispatch and executable wiring are separate integration contracts.
} // namespace bsp
