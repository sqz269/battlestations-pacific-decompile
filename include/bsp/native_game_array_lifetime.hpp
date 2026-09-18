#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
class NativeObserverLifetime;
struct NativeGameArrayLifetimeCalls {
    virtual ~NativeGameArrayLifetimeCalls()=default;
    virtual void free_00bf65ac(void*);
    // The implementation must resolve the captured object's current vslot0.
    // No payload type, extra reference count or successful empty default exists.
    virtual void virtual_terminal(void* captured)=0;
};
struct NativeGameArrayLifetimeContext {
    NativeStringRawPoolContext& strings;
    NativeObserverLifetime& observers;
    NativeGameArrayLifetimeCalls& calls;
};
struct NativeGameArrayLifetimeStage {
    void* owner{};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
};
struct NativeGameArrayLifetimeOperation final {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    void* base{};
    std::uint32_t stride{},count{},destructor{},completed{};
    NativeGameArrayLifetimeStage element;
    NativeGameArrayLifetimeOperation()=default;
    ~NativeGameArrayLifetimeOperation();
    NativeGameArrayLifetimeOperation(const NativeGameArrayLifetimeOperation&)=delete;
    NativeGameArrayLifetimeOperation& operator=(const NativeGameArrayLifetimeOperation&)=delete;
    // Acknowledge externally resolved resources; no rollback or implicit frees.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Complete normal bodies. These helpers borrow actual raw storage and update
// the diagnostic stage; the array driver below owns the one-shot operation.
void destroy_native_game_vector_004cafd0(void*,NativeGameArrayLifetimeCalls&,
    NativeGameArrayLifetimeStage&);
void destroy_native_game_list_004c1990(void*,NativeGameArrayLifetimeCalls&,
    NativeGameArrayLifetimeStage&);
void destroy_native_game_list_thunk_004c4600(void*,NativeGameArrayLifetimeCalls&,
    NativeGameArrayLifetimeStage&);
void destroy_native_game_participant_pairs_004c5860(void*,NativeGameArrayLifetimeCalls&,
    NativeGameArrayLifetimeStage&);
void release_native_game_participant_references_007f8180(void*,NativeGameArrayLifetimeCalls&,
    NativeGameArrayLifetimeStage&);
void destroy_native_game_participant_observer_004b7ef0(void*,NativeObserverLifetime&,
    NativeGameArrayLifetimeStage&);
void destroy_native_game_participant_004cb2f0(void*,NativeGameArrayLifetimeContext&,
    NativeGameArrayLifetimeStage&);
// Normal CRT reverse iteration for the three exact game-array destructor/stride
// pairs: 4CAFD0/10h,4CB2F0/118h,4C4600/0Ch. Existing raw string pool and observer
// services are called over actual embedded storage. On failure retain completed
// suffix/current element and prohibit replay. This adapter does not reconstruct
// CRT/FH3 exception cleanup or supply original binary calling conventions.
void destroy_native_game_array_00bf7c6e(void*,std::uint32_t stride,std::uint32_t count,
    std::uint32_t destructor,NativeGameArrayLifetimeContext&,NativeGameArrayLifetimeOperation&);
} // namespace bsp
