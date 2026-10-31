#pragma once
#include "bsp/native_mesh_owner.hpp"
#include <cstdint>
#include <exception>

namespace bsp {
class NativeMeshStorageAccess;

// Ordinary Source interface, not an Original register-ABI adapter. Bind the
// genuine existing environment/constant homes; construction samples no values.
// Context, environment, pool/storage, providers and constants remain stable
// through every associated frame, callback and post-Native Source fence.
class NativeMeshStorageContext final {
public:
    NativeMeshStorageContext(NativeMeshEnvironment&, NativeMeshConstants);
    ~NativeMeshStorageContext() = default;
    NativeMeshStorageContext(const NativeMeshStorageContext&) = delete;
    NativeMeshStorageContext& operator=(const NativeMeshStorageContext&) = delete;
    NativeMeshStorageContext(NativeMeshStorageContext&&) = delete;
    NativeMeshStorageContext& operator=(NativeMeshStorageContext&&) = delete;
private:
    friend class NativeMeshStorageAccess;
    friend class NativeMeshStorageFrame;
    NativeMeshEnvironment& environment_;
    const NativeMeshConstants constants_;
};

// Retained, nonmoving HOST custody for ONE actual allocation and lifecycle.
// The new allocating entries themselves call the real receiver; an arbitrary
// prior raw address, count, profile or public progress cannot be adopted here.
// Serialize calls on this frame (distinct frames need no new whole-pool lock).
// Keep it and its complete borrowed graph alive through callbacks, exceptions
// and the caller's complete Source fence. When retention is required the caller
// must retain that graph; the destructor neither enforces a process-exit policy
// nor performs Native cleanup. No stack-unwind cleanup/recovery is supplied.
class NativeMeshStorageFrame final {
public:
    enum class Phase {
        fresh, selecting_receiver, allocating, allocated, null_allocation,
        constructing, live, destroying, payload_dead, returned, failed
    };
    struct Progress {
        Phase phase;
        std::uintptr_t actual_identity, count_identity, flags_home_identity;
        NativeMeshPool* allocation_receiver;
        bool active, poisoned, failed, static_wrapper_entered, receiver_selected;
        bool allocation_entered, allocation_returned, allocation_unknown;
        bool constructor_entered, constructor_returned, constructor_unknown;
        bool null_construction_skipped, cpp_payload_live;
        bool scalar_entered, destructor_entered, destructor_returned;
        bool flags_home_captured, flags_read, type_end_entered, type_ended;
        bool normal_payload_dead, pool_return_entered, pool_returned, pool_unknown;
        bool scalar_body_completed;
        std::uint8_t flags_low_byte;
    };

    explicit NativeMeshStorageFrame(NativeMeshStorageContext&);
    // Metadata only. Ending an unresolved frame violates the caller's retained
    // custody contract; this destructor adds no Native/terminal-policy action.
    ~NativeMeshStorageFrame() noexcept = default;
    NativeMeshStorageFrame(const NativeMeshStorageFrame&) = delete;
    NativeMeshStorageFrame& operator=(const NativeMeshStorageFrame&) = delete;
    NativeMeshStorageFrame(NativeMeshStorageFrame&&) = delete;
    NativeMeshStorageFrame& operator=(NativeMeshStorageFrame&&) = delete;

    Progress progress() const noexcept; // historical diagnostics, never authority
    bool requires_process_retention() const noexcept;
private:
    friend class NativeMeshStorageAccess;
    // This additive pool-module overload captures the actual static receiver.
    friend void* allocate_native_mesh_slot_00b73b60(NativeMeshStorageFrame&);
    void begin_allocation(bool static_wrapper);
    void enter_pool_allocation(NativeMeshPool&);
    void* allocation_normal_return(void*);
    void record_failure() noexcept;

    NativeMeshStorageContext& context_;
    Phase phase_{Phase::fresh};
    NativeMeshPool* receiver_{};
    NativeMeshStorage* live_payload_{};
    std::uintptr_t actual_{}, count_{}, flags_home_{};
    bool active_{}, poisoned_{}, failed_{};
    bool static_wrapper_entered_{}, receiver_selected_{};
    bool allocation_entered_{}, allocation_returned_{};
    bool constructor_entered_{}, constructor_returned_{};
    bool null_construction_skipped_{}, cpp_payload_live_{};
    bool scalar_entered_{}, destructor_entered_{}, destructor_returned_{};
    bool flags_home_captured_{}, flags_read_{};
    bool type_end_entered_{}, type_ended_{}, normal_payload_dead_{};
    bool pool_return_entered_{}, pool_returned_{}, scalar_body_completed_{};
    std::uint8_t flags_low_byte_{};
    std::exception_ptr primary_;
};

// Explicit-pool route: invokes this frame context's real pool once. The static
// route is allocate_native_mesh_slot_00b73b60(frame), declared in the pool HPP.
// A selection failure before allocation entry is not an allocation-unknown fact.
void* allocate_native_mesh_storage_slot(NativeMeshStorageFrame&);

// Only the same frame's actual normal allocation can reach the unchanged
// B73D70. A normal null allocation skips construction once; it creates no type
// or count. Post-constructor Source failure retains the already-born object.
NativeMeshStorage* construct_native_mesh_storage_00b73d70(NativeMeshStorageFrame&);

// Caller supplies a genuine live DWORD HOME, stable through the complete call.
// Constructor/extent facts do NOT authorize scalar disposition or mint current
// count/creator/canonical authority. The caller proves the genuine direct scalar
// action, real Native credit custody and complete borrower/lifetime exclusion.
// A later canonical caller must derive its action from actual zero/creator
// custody, never this frame's public progress. No count is sampled/decremented.
// Capture its ADDRESS before B73E60, read exactly its current low byte AFTER
// that whole ordinary C++ destructor returns, then end exactly BC and perform
// the original conditional pool return. No count decrement/retain is added.
// The HOME may alias a genuine payload DWORD: no access follows the last read.
// Existing noexcept callees remain terminal if they escape; no Original FH3,
// SEH or new catchable pool-return failure is claimed. Flags0/unknown retain
// real debt. There is no retry, explicit-return/recovery or receipt-apply API.
void* delete_native_mesh_storage_00b74280(
    const volatile std::uint32_t& flags_home, NativeMeshStorageFrame&);
} // namespace bsp
