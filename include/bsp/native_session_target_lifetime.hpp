#pragma once
#include "bsp/native_session_transport_buffers.hpp"
#include "bsp/native_session_target_history.hpp"

namespace bsp {
struct TrackedCriticalSection;
// Base writes end at D73; the derived DC0 allocation and sockaddr at D74 are
// not constructed here. No defaults, automatic lifetime, or callable target
// class: profile_00 and the embedded buffer profile remain numeric stamps.
struct NativeSessionTargetStorage {
    std::uint32_t profile_00;
    TrackedCriticalSection* section_04;
    void* metadata_08;
    std::uint32_t flags_0c;
    NativeSessionTransportBuffersStorage buffers_10;
    std::uint32_t field_d4c;
    std::uint32_t field_d50;
    NativeSessionTargetHistoryStorage* history_d54;
    NativeSessionTargetHistoryStorage* history_d58;
    float field_d5c;
    float timeout_d60;
    float field_d64;
    float timestamp_d68;
    std::uint32_t index_d6c;
    std::uint16_t field_d70;
    std::uint16_t field_d72;
};
static_assert(sizeof(NativeSessionTargetStorage) == 0xd74);
static_assert(offsetof(NativeSessionTargetStorage, section_04) == 4);
static_assert(offsetof(NativeSessionTargetStorage, buffers_10) == 0x10);
static_assert(offsetof(NativeSessionTargetStorage, history_d54) == 0xd54);
static_assert(offsetof(NativeSessionTargetStorage, history_d58) == 0xd58);
static_assert(offsetof(NativeSessionTargetStorage, timestamp_d68) == 0xd68);
static_assert(offsetof(NativeSessionTargetStorage, index_d6c) == 0xd6c);
static_assert(offsetof(NativeSessionTargetStorage, field_d72) == 0xd72);

// Unlike the standalone history initializer, this composition installs an
// executable Source profile in each live history's raw profile word. Native
// D04268 has exactly the single scalar slot 783970. EDX is deliberately unused.
using NativeSessionTargetHistoryScalar = NativeSessionTargetHistoryStorage* (__fastcall*)(
    NativeSessionTargetHistoryStorage*, void*, std::uint32_t);
struct NativeSessionTargetHistoryProfile {
    NativeSessionTargetHistoryScalar scalar_00;
};
static_assert(sizeof(NativeSessionTargetHistoryProfile) == 4);
extern const NativeSessionTargetHistoryProfile native_session_target_history_profile_00d04268;

// Borrow actual application storage. No replacement global mask, constants,
// allocator, lifecycle phase, or default provider is supplied by this module.
struct NativeSessionTargetLifetimeBindings {
    volatile std::uint16_t& slots_f871b0;
    const double& three_d7a2b0;
    const double& zero_d7a258;
    const float& timeout_ce4bc4;
    const float& negative_one_d7a260;
};

// Complete [7839D0,783B85): native ECX target, metadata/length stack, RET8,
// EAX target. This interface is a new Source ABI. Construction requires raw
// storage; it does not release prior contents. Non-null metadata requests 98h
// bytes then copies the unchecked original length, as in the native body.
// After successful buffer construction only, an ordinary C++ exception frees
// a currently initializing 18h history (if any), then destroys the buffers.
// Already published lock/metadata/history allocations are not rolled back.
NativeSessionTargetStorage* construct_native_session_target_007839d0(
    NativeSessionTargetStorage* actual_target, const void* original_metadata,
    std::uint32_t original_length, const NativeSessionTargetLifetimeBindings& bindings);

// Complete [783B90,783C75): native ECX target, RET. Fresh history/profile/slot0
// dispatches use flags1. Captured lock is drained/deleted/freed but +4 is not
// cleared. Buffers are destroyed on normal completion or a C++ exception.
// Consumers must be quiescent and published owners canonically allocated.
void destroy_native_session_target_00783b90(NativeSessionTargetStorage* actual_target,
    volatile std::uint16_t& actual_slots_f871b0);

// Complete [784320,78433E): native ECX target, flags stack, RET4. Return captured
// target, optionally freed after completed destruction when flags bit0 is set.
NativeSessionTargetStorage* scalar_delete_native_session_target_00784320(
    NativeSessionTargetStorage* actual_target, std::uint32_t flags,
    volatile std::uint16_t& actual_slots_f871b0);

// Original private CRT/EH, hardware-fault unwinding, whole-class/native ABI,
// derived publication, sockets and game behavior are not supplied. The raw
// target D0426C word is deliberately uncallable; it is not a host vtable token.
} // namespace bsp
