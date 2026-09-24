#pragma once
#include "bsp/native_resource_registry_insertion.hpp"
#include <cstdint>

namespace bsp {
// Stable borrowed bindings, not copied publications/profile contents. Only the
// reached registry profile view must be valid. Resolve the numeric identity
// without side effects, then read its CURRENT +4; qualified target is B1B3A0.
// The genuine getter may construct/register through this SAME raw manager.
struct NativeProceduralFactoryStorageContext {
    void* volatile& actual_manager_publication_01090aa0;
    void* volatile& actual_registry_publication_00f8d41c;
    NativeResourceRegistryInsertionContext& insertion;
    const void* const actual_registry_profile_00d5e594;
    const void* const actual_registry_profile_00d5e59c;
};

// Immutable views over initialized, genuinely live caller DWORD cells. Native
// name is entry S+4, cleanup_self is S-10 (EH EBP-10). PUSH ECX seeds that local;
// the later MOV repeats the captured destination before state0/profile stores.
// Nested registration owns separate native argument/local sites. Metadata and
// diagnostics must be disjoint; no aggregate overlay on reused scratch.
struct NativeProceduralFactoryStorageFrame {
    volatile std::uint32_t& name_argument;
    volatile std::uint32_t& cleanup_self;
    const NativeResourceRegistryRegistrationFrame& registration;
};
struct NativeProceduralFactoryStorageAcquired {
    bool started{};
    int native_eh_state{-1};
    bool getter_entered{};
    bool getter_returned{};
    void* returned_registry{};
    std::uint32_t captured_name{};
    std::uint32_t captured_registry_profile{};
    std::uint32_t captured_registration_target{};
    bool registration_entered{};
    bool constructor_completed{};
    bool source_failed{};
    bool unsupported_binding{};
    bool cleanup_started{};
    bool cleanup_completed{};
    void* cleanup_receiver{};
    NativeResourceRegistryRegistrationAcquired registration;
};

// BBC5F0/BBC740, each83B: ECX actual4B factory, stack current raw-name pointer,
// RET4/EAX captured factory. Stamp D64470/D644AC, genuine B1B730, then read
// CURRENT name, returned registry profile and captured table+4, in that order.
// Seed nested factory then name at native push points; compose real B1B3A0.
// Registry node+14 borrows factory bits; no count/retain/unregister/admission.
void* construct_native_caustics_factory_00bbc5f0(void* actual_factory,
    const NativeProceduralFactoryStorageFrame&, NativeProceduralFactoryStorageContext&,
    NativeProceduralFactoryStorageAcquired&);
void* construct_native_shore_wave_factory_00bbc740(void* actual_factory,
    const NativeProceduralFactoryStorageFrame&, NativeProceduralFactoryStorageContext&,
    NativeProceduralFactoryStorageAcquired&);

// BBC440[7]: exact actual4B base-profile stamp D64468. No allocation, free,
// field+4, unregister, callback or ownership operation.
void destroy_native_procedural_factory_base_00bbc440(void* actual_factory) noexcept;

// Distinct BBC650/BBC7A0[31 each]. Test CURRENT flags LOW BYTE bit0 BEFORE the
// base stamp; genuine existing CRT free only if that captured test was set.
// Return captured pointer bits even after free, with no post-free payload read.
// Flags0 leaves caller-owned backing; it does not establish a second lifetime.
void* delete_native_caustics_factory_00bbc650(void* actual_factory,
    const volatile std::uint32_t& flags);
void* delete_native_shore_wave_factory_00bbc7a0(void* actual_factory,
    const volatile std::uint32_t& flags);

// Valid actual4B live DWORD payload (not a host wrapper), raw names/tree, genuine
// same pool/manager/current publication/CRT bindings and fresh persistent nested
// acquisitions are caller requirements. Factory backing outlives all lookups,
// or callers quiesce lookups before its disposal. No automatic lifetime binding.
// Constructor failure consumes state0 once and stamps CURRENT cleanup_self via
// BBC440, then rethrows. Never free/unregister/rollback a factory, tree mutation,
// publication or residual provider credit. Getter booleans mark entry/return,
// not private getter unwind progress; its established cleanup contract remains.
// Unknown profile/selector raises a source diagnostic and uses that same source
// cleanup policy; it is outside the qualified native binding domain. No fallback.
// Keep frames/backing/diagnostics alive for explicit failure disposition. Source
// C++ cleanup is not native FH3/SEH; private stack coincidence, saved registers,
// volatile register parity, hardware faults, table/derived scalar/resource34h
// admission, global startup and application behavior are not established.
} // namespace bsp
