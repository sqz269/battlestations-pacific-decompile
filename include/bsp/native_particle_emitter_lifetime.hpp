#pragma once

#include "bsp/random_threads.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual Win32 storage, independently owned by emitter+10. This container has
// NO reference count: +04/+08 borrow the existing model/emitter identities.
// Rows have a DWORD element-count cookie immediately before their allocation.
struct NativeParticleEmitterRow {
    std::uint16_t state_id_00;
    std::uint16_t untouched_02;
    float value_04;
};
struct NativeParticleEmitterRows {
    NativeParticleEmitterRow* data_00;
    std::uint32_t capacity_04;
};
struct NativeParticleEmitterStates {
    void* data_00;
    std::uint32_t capacity_04;
};
struct NativeParticleEmitterContainer {
    std::uint32_t native_vtable_00;
    void* model_04;
    void* emitter_08;
    NativeParticleEmitterRows rows_0c;
    NativeParticleEmitterStates states_14;
    std::uint32_t count_1c;
    std::uint32_t word_20;
    std::uint32_t word_24;
    std::uint32_t word_28;
    std::uint32_t word_2c;
};
static_assert(sizeof(NativeParticleEmitterRow) == 8);
static_assert(offsetof(NativeParticleEmitterRow, value_04) == 4);
static_assert(sizeof(NativeParticleEmitterContainer) == 0x30);
static_assert(offsetof(NativeParticleEmitterContainer, rows_0c) == 0x0c);
static_assert(offsetof(NativeParticleEmitterContainer, states_14) == 0x14);
static_assert(offsetof(NativeParticleEmitterContainer, count_1c) == 0x1c);
static_assert(offsetof(NativeParticleEmitterContainer, word_2c) == 0x2c);

// Complete B04900..B0490F / B04910. ECX actual row, RET. Constructor writes
// WORD0 and float4 only; B04910 really is a one-instruction empty destructor.
NativeParticleEmitterRow* initialize_native_particle_emitter_row_00b04900(void*) noexcept;
void destroy_native_particle_emitter_row_00b04910(NativeParticleEmitterRow&) noexcept;

// Complete B04B50..B04BFE, ECX actual embedded +0C, stack DWORD count, RET4.
// Replace even if capacity is sufficient. Destroy old rows backwards using
// the COOKIE, free, allocate saturated(count*8+4), initialize forwards, publish.
// A throwing replacement allocation leaves the old dangling pointer/capacity.
void replace_native_particle_emitter_rows_00b04b50(NativeParticleEmitterRows&, std::uint32_t);
// Complete B04C40..B04C76 / B04A80..B04AA0; ECX actual embedded member, RET.
// Release backing and THEN zero capacity/pointer. State bytes have no destructor.
void destroy_native_particle_emitter_rows_00b04c40(NativeParticleEmitterRows&) noexcept;
void destroy_native_particle_emitter_states_00b04a80(NativeParticleEmitterStates&) noexcept;

// Complete B04E30..B04EF5 / B053D0..B053FB: ECX actual raw30h slot, stack
// (actual model, actual emitter, DWORD capacity), EAX same slot, RET0C.
// Base writes D5DF20, zeroes members except2C; positive signed capacity allocates
// actual rows and uninitialized6Ch states. Derived writes D5DF24 then clears2C.
// Base unwind destroys states before rows; no owner/model/definition retain.
NativeParticleEmitterContainer* construct_native_particle_emitter_base_00b04e30(
    void* actual_slot, void* actual_model, void* actual_emitter, std::uint32_t capacity);
NativeParticleEmitterContainer* construct_native_particle_emitter_container_00b053d0(
    void* actual_slot, void* actual_model, void* actual_emitter, std::uint32_t capacity);
// Complete AFF690..AFF6F5, ECX actual28h emitter, EAX current emitter10, RET.
// Allocate30h using the existing CRT service iff null. Read definition0C+20
// BEFORE model08, build actual storage, publish only on success, then reload10.
// Construction failure frees the raw30h allocation without publishing it.
void* acquire_native_particle_emitter_container_00aff690(void* actual_emitter);

// All pointers are actual objects. Capture is a nonmutating dispatch lookup of
// the ALREADY captured definition/table/slot; it must not reread those fields.
// No modeled point-light companion can stand in for actual native storage.
struct NativeParticleDefinitionCleanupCall {
    void* context;
    std::uint8_t (*invoke)(void* context, void* actual_definition,
        void* actual_state, std::uint32_t argument);
};
class NativeParticleEmitterCleanupBindings {
public:
    virtual ~NativeParticleEmitterCleanupBindings() = default;
    virtual NativeParticleDefinitionCleanupCall capture_definition_virtual1c(
        void* actual_definition, std::uint32_t captured_table,
        std::uint32_t captured_target) = 0;
    // Complete actual72B740 getter, including its existing0108FF50 publication
    // and shared01090AA0 lifetime domain. Return actual raw8h owner; +04 points
    // to a real physical1Ch TrackedCriticalSection, NOT a projection object.
    virtual void* call_0072b740() = 0;
    // Required complete current actual-light unlink and actual-node release.
    // B7C160: current light+1E0 backlinks, B6F3C0 each model, clear own array.
    // B6DFA0: actual parent/root unlink then CURRENT virtual18, sole actual04
    // reference count and canonical terminal destruction/pool association.
    // Neither callback may report success while omitting any of those effects.
    virtual void call_00b7c160(void* actual_point_light) = 0;
    virtual void call_00b6dfa0(void* actual_node) = 0;
};

// Complete B04F00..B04FB0 through REQUIRED real callbacks above. ECX actual
// 6Ch state, stack full DWORD argument, RET4, AL original definition result.
// Current definition64 virtual1C uses ECX definition and stack(state,arg)/RET8.
// After it returns: test current60, get/capture lock, enter/+18++, reload60 for
// unlink, reload60 for node release, clear60, decrement/leave captured lock.
// On unlink/release exception, destroy actual raw guard using canonical411EE0;
// state60 remains current. Getter/Enter failure happens before unwind is armed.
std::uint8_t cleanup_native_particle_emitter_state_00b04f00(
    void* actual_state, std::uint32_t argument, NativeParticleEmitterCleanupBindings&);

// These are new C++ interfaces, not original vtables/SEH/binary replacements.
// Valid spans/lifetimes and caller synchronization remain native preconditions.
// Positive capacities must admit their row/state allocations; no extra clamp,
// second model/count, substitute lock, owner release, or successful fallback.
} // namespace bsp
