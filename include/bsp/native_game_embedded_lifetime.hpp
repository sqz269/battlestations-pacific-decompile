#pragma once
#include <cstdint>

namespace bsp {
struct TrackedCriticalSection;
struct NativeGameEmbeddedLifetimeCalls {
    virtual ~NativeGameEmbeddedLifetimeCalls()=default;
    virtual void free_00bf65ac(void*);
    virtual void free_00bf6989(void*);
    virtual void call_0041cc80(void* actual_section_slot);
    virtual void invalid_parameter_00bf6713(); // real returning CRT default
    virtual void virtual_scalar(void* captured,std::uint32_t slot,std::uint32_t flags)=0;
    // Current peer+4 receiver, current vtable+4, one captured record pointer.
    // No payload class or default successful implementation is inferred.
    virtual void virtual_04(void* receiver,void* record)=0;
};
struct NativeGameEmbeddedLifetimeContext {
    volatile std::uint8_t& flag_00e0af14;
    NativeGameEmbeddedLifetimeCalls& calls;
};
struct NativeGameEmbeddedLifetimeProgress {
    void* owner{};
    void* peer{};
    void* cursor{};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    TrackedCriticalSection* entered_section{};
    bool entered_section_pending{};
    std::uint32_t keys[8]; // only the written prefix exists before zero fill
    std::uint32_t key_count{};
};
struct NativeGameEmbeddedLifetimeOperation final : NativeGameEmbeddedLifetimeProgress {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    NativeGameEmbeddedLifetimeContext* context{};
    NativeGameEmbeddedLifetimeOperation()=default;
    ~NativeGameEmbeddedLifetimeOperation();
    NativeGameEmbeddedLifetimeOperation(const NativeGameEmbeddedLifetimeOperation&)=delete;
    NativeGameEmbeddedLifetimeOperation& operator=(const NativeGameEmbeddedLifetimeOperation&)=delete;
    // Only after the caller resolves resources AND any retained entered lock.
    // No implicit free, lock release, rollback or native FH3 cleanup occurs.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Complete41B76A760: zero shared flag byte, then current nonnull188/18C+94.
void disable_native_game_embedded_owners_0076a760(void*,volatile std::uint8_t&) noexcept;
// Complete332B7849C0. Actual peer+44 is a nonnull tracked section. Snapshot
// up to8 raw keys from list+8, dispatch nonzero matching record+8 values in
// list+38 through CURRENT peer+4/vslot4, then free nodes and retain sentinel.
// Caller supplies <=8 first-list entries; native overflow corrupts its frame.
// The exit reloads peer+44. No RAII unlock or synthesized cleanup on failure.
void flush_native_game_peer_records_007849c0(void*,NativeGameEmbeddedLifetimeCalls&,
    NativeGameEmbeddedLifetimeProgress&);
// Complete117B76DB20: prefer current188 over18C, flush if nonnull, then walk
// captured cursor through a CURRENT begin+count end, scalar-delete nonnull
// entries without clearing cells, and zero count250 at normal completion.
void clear_native_game_embedded_entries_0076db20(void*,NativeGameEmbeddedLifetimeCalls&,
    NativeGameEmbeddedLifetimeProgress&);
// Complete223B76F000 normal body. StampD039CC, clear entries, scalar-delete
// current18C then188 and clear each after callback; actual298 lock release;
// free/clear current25C then24C; conditional large SBO free and sparse reset.
// Explicit source ABI. Failure retains graph/lock/stage and prohibits replay.
void destroy_native_game_embedded_state_0076f000(void*,NativeGameEmbeddedLifetimeContext&,
    NativeGameEmbeddedLifetimeOperation&);
} // namespace bsp
