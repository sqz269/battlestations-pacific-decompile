#pragma once
#include "bsp/native_resource_registry_lookup.hpp"
#include <cstdint>

namespace bsp {
// Immutable views over genuinely live caller DWORD cells. The normal table
// calls reuse one flags cell (entry S-1C). Compiler cleanup calls have explicit
// helper frames; no native FH3 private-stack alias relation is invented.
struct NativeProceduralFactorySlotFrame {
    volatile std::uint32_t& flags_argument;
};
struct NativeProceduralFactoryTableFrame {
    volatile std::uint32_t& cleanup_self; // entry S-10, native EH EBP-10
    const NativeProceduralFactorySlotFrame& normal;
    const NativeProceduralFactorySlotFrame (&cleanup)[3]; // self+0/+4/+8
};

// Fresh, persistent, disjoint metadata per invocation. This does not retain,
// register or automatically dispose any native backing, including on failure.
struct NativeProceduralFactorySlotAcquired {
    bool started{};
    void* captured_slot{};
    void* captured_owner{};
    std::uint32_t captured_profile{};
    std::uint32_t captured_target{};
    bool flags_written{};
    bool terminal_entered{};
    bool terminal_returned{};
    bool slot_cleared{};
    bool completed{};
    bool source_failed{};
    bool unsupported_binding{};
};
struct NativeProceduralFactoryTableAcquired {
    bool started{};
    void* captured_table{};
    int native_eh_state{-1};
    unsigned normal_index{}; // 0/1/2/3 mean +C/+8/+4/+0
    NativeProceduralFactorySlotAcquired normal[4];
    bool completed{};
    bool source_failed{};
    bool cleanup_started{};
    bool cleanup_completed{};
    bool cleanup_failed{};
    int cleanup_action{-1};
    void* cleanup_receiver[3]{};
    NativeProceduralFactorySlotAcquired cleanup[3];
};
struct NativeProceduralFactoryTableShutdownAcquired {
    bool started{};
    void* captured_table{};
    bool table_entered{};
    bool table_returned{};
    bool free_entered{};
    bool table_freed{};
    bool completed{};
    bool source_failed{};
    NativeProceduralFactoryTableAcquired table;
};

// Distinct complete25B bodies. ECX is an actual4B owner-pointer slot, RET.
// Read current owner/profile/target0; seed flags1 AFTER target capture, invoke
// its genuine scalar provider, then clear the captured slot only after return.
// Borrow the SAME actual profile views as registry lookup. Qualified pairs:
// D64470/BBC650, D644AC/BBC7A0, D644E8/BBC890, D644F0/BBC8E0.
void destroy_native_procedural_factory_slot_00bbc4e0(void* actual_slot,
    const NativeProceduralFactorySlotFrame&, const NativeResourceRegistryLookupContext&,
    NativeProceduralFactorySlotAcquired&);
void destroy_native_procedural_factory_slot_00bbc500(void* actual_slot,
    const NativeProceduralFactorySlotFrame&, const NativeResourceRegistryLookupContext&,
    NativeProceduralFactorySlotAcquired&);

// BBC520[144], ECX actual10h table, RET. Preserve current slot/state order
// C/state2,8/state1,4/state0,0/state-1. Source failure consumes each EH state
// BEFORE reloading current cleanup_self for +8/+4/+0 via CC4B33/28/20. Stop
// on a second cleanup failure; retain residuals and reject replay. Does not
// free the table, dispose a throwing slot twice, or emulate native FH3/SEH.
void destroy_native_procedural_factory_table_00bbc520(void* actual_table,
    const NativeProceduralFactoryTableFrame&, const NativeResourceRegistryLookupContext&,
    NativeProceduralFactoryTableAcquired&);

// BBC5D0[29], no native argument. Read publication ONCE, destroy then free the
// captured nonnull table only after normal child return. Do not clear/reload
// publication; normal return may leave the native publication dangling.
void shutdown_native_procedural_factory_table_00bbc5d0(
    void* volatile& actual_publication_01090900, const NativeProceduralFactoryTableFrame&,
    const NativeResourceRegistryLookupContext&, NativeProceduralFactoryTableShutdownAcquired&);

// Distinct31B final-profile scalars. ECX actual4B owner, stack current flags,
// RET4/EAX captured pointer bits. LOW-byte flag test precedes D64468 stamp and
// genuine CRT free. Reuse reader providers' identical source effects; no extra
// count, unregister, callback, or post-free payload read.
void* delete_native_table_caustics_factory_00bbc890(void* actual_factory,
    const volatile std::uint32_t& flags);
void* delete_native_table_shore_wave_factory_00bbc8e0(void* actual_factory,
    const volatile std::uint32_t& flags);

// Admission: actual live slot/table/factory backing, current profile views and
// same CRT allocation domain. Only the reached view must be usable. Metadata,
// frame objects and native payload/cells are disjoint; explicit cell aliases
// remain caller-controlled. Context/frames/acquisitions survive the complete
// operation or retained failure. The entire reached acquisition tree is fresh.
// Registry node+14 BORROWS factory bits: quiesce all lookups before disposal;
// later genuine registry erase may free names/nodes without factory access.
// No base D64468/BBC460 support, ownership map, duplicate-owner/reentrancy fix,
// constructor, resource34h admission, global storage, atexit or startup call.
// Unsupported bindings throw at reached SOURCE sites, not as native exceptions.
// Original register/private-stack ABI, FH3/SEH and game behavior are unproved.
} // namespace bsp
