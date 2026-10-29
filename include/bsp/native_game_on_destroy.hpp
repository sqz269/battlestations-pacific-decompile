#pragma once

#include "bsp/native_game_class_cleanup.hpp"
#include <cstdint>

namespace bsp {
struct NativeGameStorage;
struct NativeGameLifetimeContext;
struct NativeGameOnDestroyContext;

// Required whole-callee bindings for the actual raw Game graph. No successful
// empty defaults are supplied. Older screen-level, frontend and Lua projections
// do not satisfy these contracts merely because their names contain an address.
struct NativeGameOnDestroyCalls {
    virtual ~NativeGameOnDestroyCalls()=default;
    virtual void call_0046f3e0()=0;
    // The Game argument is the original call-site ECX, not a projected class.
    virtual void call_004da780(NativeGameStorage& actual_game_ecx)=0;
    virtual void call_004da650(NativeGameStorage& actual_game_ecx)=0;
    // Current table byte slot8, ECX=captured receiver, no stacked arguments.
    // The receiver/code/table must be genuinely callable in this process.
    virtual void current_online_slot_08(void* captured_receiver)=0;
    // Observe this callee's CURRENT actual globals after the slot8 callback;
    // do not substitute the captured receiver or a copied publication value.
    virtual void call_00992ac0()=0;
    // Unconditionally called on the current Game+1A08 value, even when null.
    // Requires the whole raw helper, not MissionLuaHostState projection storage.
    virtual void call_008844f0(void* actual_host_ecx)=0;
};

struct NativeGameOnDestroyContext {
    // SAME retained owner/calls/profile/classes/publications as Game lifetime.
    // Reached scalar receivers must satisfy its actual dispatch/heap contract.
    NativeGameLifetimeContext& lifetime;
    void* volatile& actual_online_00f8a2fc;
    const char* actual_trace_00ce7c98;
    NativeGameOnDestroyCalls& required_calls;
};

struct NativeGameOnDestroyOperation final {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    NativeGameStorage* owner{};
    NativeGameOnDestroyContext* context{};
    std::uint32_t native_site{};
    // Historical bits only; a completed callback may already have freed it.
    std::uintptr_t captured_address{};
    NativeGameClassCleanupOperation classes;

    NativeGameOnDestroyOperation()=default;
    ~NativeGameOnDestroyOperation();
    NativeGameOnDestroyOperation(const NativeGameOnDestroyOperation&)=delete;
    NativeGameOnDestroyOperation& operator=(const NativeGameOnDestroyOperation&)=delete;
    // Caller resolves the partial graph first. This performs no native cleanup,
    // release, rollback, retry, publication repair or callback restoration.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// Complete normal 004DC5C0..004DC69A, 219 bytes/67 instructions. Original
// ECX=actual 71A0h Game, no stack arguments, final JMP004A9AC0. New void C++
// interface: no native tail/register-result, FH3/SEH/fault or binary-entry ABI claim.
// The sole native caller discards EAX. The qualified class-cleanup default retains
// its matching calls/string/manager checks; its operation is separate from the
// later Game storage destructor's class-clear operation.
//
// Require a live actual raw Game, genuine providers, and serialized ownership
// of this operation and graph. Callback-driven field/publication changes are
// observed at native reload epochs; this is not arbitrary parent reentrancy or
// concurrent-mutation support. One attempt per operation; C++ failure retains
// effects and diagnostics and rethrows, with no automatic recovery or replay.
void on_destroy_native_game_004dc5c0(NativeGameStorage&,
    NativeGameOnDestroyContext&,NativeGameOnDestroyOperation&);
} // namespace bsp
