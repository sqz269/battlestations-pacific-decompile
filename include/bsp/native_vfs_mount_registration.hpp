#pragma once

#include <cstdint>

namespace bsp {
class NativeVfsFactoryCreateDispatch;
struct NativePathCanonicalizerContext;
struct SingletonLifetimeCallbacks;

// BE18B8 calls the captured manager+90h target with no stacked arguments.
// At that call EAX is the CURRENT 0109CEEC publication and ECX is the target
// itself. This is an application callback boundary, not a conventional thiscall.
// Supply its established implementation; null/unknown targets have no fallback.
class NativeVfsMountFailureDispatch {
public:
    virtual ~NativeVfsMountFailureDispatch() = default;
    virtual void mount_failure_00be18b8(std::uintptr_t captured_target,
        void* captured_current_manager) = 0;
};

struct NativeVfsMountRegistrationContext {
    void* volatile& manager_0109ceec;
    NativePathCanonicalizerContext& canonicalizer;
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativeVfsFactoryCreateDispatch& factories;
    NativeVfsMountFailureDispatch& failure;
};

// Complete BE1740[329]. Native ECX manager; stacked provider, actual8h prefix,
// signed-priority bits, flags DWORD; RET10h. Canonicalize, trim '/', consume a
// constructed actual10h payload, copy actual14h record, insert at manager+3Ch,
// then release the three completed string owners in reverse construction order.
// Only flags' low byte is stored. No provider reference operation or rollback.
void register_native_vfs_mount_00be1740(void* actual_manager, void* provider,
    const void* actual_prefix, std::uint32_t priority, std::uint32_t flags,
    NativePathCanonicalizerContext&);

// Complete BE1890[127] in the finite current-slot18h=BDB040 dispatch domain.
// Native ECX manager; stacked system/virtual actual8h headers, priority, flags,
// device-id; RET14h; EAX selected provider or null. Failure reloads the global
// manager and invokes its +90h callback. Success writes provider+10h, performs
// the proven RET-only diagnostic's argument reads, then calls BE1740 on the
// originally captured manager. Mount failure does not destroy the provider.
void* mount_native_vfs_system_path_00be1890(void* actual_manager,
    const void* actual_system, const void* actual_virtual,
    std::uint32_t priority, std::uint32_t flags, std::uint32_t device_id,
    NativeVfsMountRegistrationContext&);

// New C++ interfaces, not native binary/FH3 replacements. Actual string storage
// is borrowed. Explicit canonicalizer services preserve wrapper cleanup-getter
// ordering; nested NativeStringStorage::release remains noexcept. Arbitrary
// aliases into compiler stack spills and simultaneous unwind failures are not
// validated. Names are descriptive hypotheses. See the accompanying evidence.
} // namespace bsp
