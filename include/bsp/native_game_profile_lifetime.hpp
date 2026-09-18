#pragma once
#include "bsp/native_mission_progress_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstdint>

namespace bsp {
struct NativeGameProfileLifetimeContext;
struct NativeGameProfileLifetimeCalls : NativeProfileCollectionCalls {
    virtual ~NativeGameProfileLifetimeCalls()=default;
    virtual NativeStringPoolStorage* profile_pool_00419cc0(NativeStringRawPoolContext&);
    virtual void profile_return_00bd1510(NativeStringPoolStorage*,void*,std::uint32_t,
        NativeStringRawPoolContext&);
    virtual void profile_invalid_parameter_00bf6713();
    virtual void profile_call_007fd780(void*,NativeGameProfileLifetimeContext&,
        NativeMissionProgressOperation&);
    virtual void profile_call_004d05e0(void*,NativeGameProfileLifetimeContext&);
    virtual void profile_call_00432050(void* first,void* last,NativeGameProfileLifetimeContext&);
    // Consumed library full-range contracts. Both owner/node pairs must be the
    // actual current begin/end; other ranges are not exposed by these adapters.
    virtual void* profile_call_0058d860(void*,void*,void*,void*,void*,void*,
        NativeGameProfileLifetimeContext&);
    virtual void* profile_call_007fb230(void*,void*,void*,void*,void*,void*,
        NativeGameProfileLifetimeContext&);
    // Existing complete raw string-tree implementation, including partial erase.
    virtual void* profile_call_004d1a50(void*,void*,void*,void*,void*,void*,
        NativeGameProfileLifetimeContext&);
    virtual void profile_call_007f8310(void*);
};
struct NativeGameProfileLifetimeContext {
    NativeStringRawPoolContext& strings;
    NativeGameProfileLifetimeCalls& calls;
    // Retained mission progress borrows this bridge; context outlives operations.
    ActualNativeStringPoolStorage actual_strings;
    NativeGameProfileLifetimeContext(NativeStringRawPoolContext& raw,
        NativeGameProfileLifetimeCalls& services) noexcept
        :strings(raw),calls(services),actual_strings(raw.actual_published_01090aa8,
            raw.actual_small_returns_disabled_01090aa4,raw.actual_manager_publication_01090aa0){}
};
struct NativeGameProfileLifetimeProgress {
    void* owner{};
    void* cursor{};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    std::int32_t element_unwind_state{-1}; // 61F630 child; parent state stays separate
    std::uint32_t iterator_output[2];
};
struct NativeGameProfileLifetimeOperation final : NativeGameProfileLifetimeProgress {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    NativeGameProfileLifetimeContext* context{};
    NativeMissionProgressOperation mission;
    NativeGameProfileLifetimeOperation()=default;
    ~NativeGameProfileLifetimeOperation();
    NativeGameProfileLifetimeOperation(const NativeGameProfileLifetimeOperation&)=delete;
    NativeGameProfileLifetimeOperation& operator=(const NativeGameProfileLifetimeOperation&)=delete;
    // Resolve the graph and any failed mission child first. No rollback/free.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Complete152B61F630 normal body: three actual8h string headers at10,8,0,
// released in that order; null skips length/getter. Preserve every owner byte.
void destroy_native_profile_three_strings_0061f630(void*,NativeGameProfileLifetimeContext&,
    NativeGameProfileLifetimeProgress&);
// Complete70B7FB340: capture begin/end, ascend1Ch rows; free CURRENT begin;
// zero+4/+8/+C. Null begin still zeros all three; opaque+0 remains untouched.
void destroy_native_profile_bonus_records_007fb340(void*,NativeGameProfileLifetimeContext&,
    NativeGameProfileLifetimeProgress&);
// Complete36B7FF9F0 raw overload: stampD08D20, capture/release actual name8/C.
// The existing typed RaceRecord interface remains separate; shared raw domain.
void destroy_native_game_race_record_007ff9f0(void*,NativeGameProfileLifetimeContext&,
    NativeGameProfileLifetimeProgress&);
// Complete631B7FD8A0 normal schedule over actual profileF8h. Reuse actual raw
// mission/tree/list/string implementations, retaining current fields/captures.
// Source failures retain the graph and nested mission operation; no replay.
// Explicit source ABI, not native FH3/SEH, binary replacement or app admission.
void destroy_native_game_profile_007fd8a0(void*,NativeGameProfileLifetimeContext&,
    NativeGameProfileLifetimeOperation&);
} // namespace bsp
