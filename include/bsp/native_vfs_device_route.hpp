#pragma once

#include <cstdint>

namespace bsp {
class ActualNativeStringPoolStorage;
struct NativeVfsLookupRouteContext;

// The original provider+24h call boundary. Both BDBC70 branches capture the
// current provider and table entry, then pass suffix/output (RET8, raw AL).
// Integration must invoke that captured target; no successful fallback exists.
class NativeVfsProviderResolveDispatch {
public:
    virtual ~NativeVfsProviderResolveDispatch() = default;
    virtual std::uint8_t invoke_resolve(std::uintptr_t captured_entry,
        void* captured_provider, const void* suffix, void* actual_output) = 0;
};

struct NativeVfsDeviceRouteContext {
    NativeVfsLookupRouteContext& lookup;
    const void* actual_device_profile_00d683f4;
    NativeVfsProviderResolveDispatch& providers;
};

// Full BDBC70[396]: ECX actual14h visitor; stack mount payload/name; RET8.
// Empty mount sends visitor+8 directly to current provider+24. Nonempty mount
// always assigns mount + slash + provider output, even when raw AL is zero.
// Afterwards CURRENT result byte decides whether CURRENT payload+8 provider's
// device+10 is stored at visitor+10. Actual raw headers and current pool only.
void read_native_vfs_device_provider_00bdbc70(void* actual_visitor,
    const void* actual_mount_payload, const void* actual_name,
    NativeVfsDeviceRouteContext&);

// Full BDB670[4]: ECX actual visitor; RET; AL=current byte4, not normalized.
std::uint8_t read_native_vfs_device_result_00bdb670(const void* actual_visitor) noexcept;
// Full BDB680[88]: ECX actual visitor; RET. Return current name data+0C with
// current length+8 plus one, then D68380 base reset. Keep name/result/device.
void destroy_native_vfs_device_visitor_00bdb680(void* actual_visitor,
    ActualNativeStringPoolStorage&);
// Full BDBE00[30]: ECX visitor; stack flags; RET4; EAX captured visitor.
// Destroy, then free only for bit0. Existing compiler-generated name retained.
void* delete_native_vfs_device_visitor_00bdbe00(void* actual_visitor,
    std::uint32_t flags, ActualNativeStringPoolStorage&);

// Full BDD850[305]: ECX captured manager; stack name/unused_name; RET8;
// EAX device or -1. Construct actual14h stack visitor, copy and normalize a
// separate name, use existing actual BDD0A0 traversal, capture result/device,
// release copied name then visitor. Second stack argument is never read.
std::int32_t select_native_vfs_device_00bdd850(void* actual_manager,
    const void* actual_name, const void* unused_name, NativeVfsDeviceRouteContext&);

// Full BF0FB0[75]: ECX actual provider; stack suffix/output; RET8; AL0/1.
// Capture current provider+10 contains target, call with original suffix.
// False leaves output; true copies CURRENT original suffix header with an
// identity guard and overlap-capable original BF7680 copy behavior.
bool resolve_native_provider_logical_name_00bf0fb0(void* actual_provider,
    const void* actual_suffix, void* actual_output, NativeVfsLookupRouteContext&);

// Source APIs, not original x86 ABI/FH3/SEH replacements. Contexts borrow the
// existing actual manager, lookup tree, string pool and profile storage.
// Runtime wiring and native/source execution are separate validation domains.
} // namespace bsp
