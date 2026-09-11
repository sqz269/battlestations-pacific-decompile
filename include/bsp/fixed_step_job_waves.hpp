#pragma once
#include <cstddef>
#include <cstdint>

// The four job waves of the fixed simulation step: the five 68h-stride groups
// at 00F876C0, the four descriptor factories and the four job bodies.
//
// bsp/in_mission_subsystem_tick.hpp reconstructed the driver 00875BB0 and left
// FixedStepHost::run_step_job_waves and run_interpolation_wave_00875670 as
// opaque host methods. This header is their body. docs/FIXED_STEP_JOB_WAVES.md
// carries the evidence, including the group layout correction: the group starts
// at 00F876C0, and 00F876C8 is the head sentinel's next field.
//
// Every name is a hypothesis, not a recovered symbol.

namespace bsp {

// ---------------------------------------------------------------------------
// The group array, 00F876C0 + index * 68h
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kJobWaveGroupArrayBase = 0x00f876c0; // 00874CB8
inline constexpr std::size_t kJobWaveGroupCount = 5;               // 00875CAF, MOV [ESP+10h],5
inline constexpr std::size_t kJobWaveGroupStride = 0x68;           // 00875D10, ADD EBX,68h

// The two sentinels inside a group. A node is 34h bytes with prev at +4h and
// next at +8h, so the head sentinel's next lands on group+8h and the tail
// sentinel's prev on group+38h.
inline constexpr std::size_t kJobWaveGroupHeadNodeOffset = 0x00;  // 00874CB8
inline constexpr std::size_t kJobWaveGroupFirstOffset = 0x08;     // 00875CB7, [EBX-2Ch]
inline constexpr std::size_t kJobWaveGroupTailNodeOffset = 0x34;  // 00874CCA, LEA ESI,[ECX+34h]
inline constexpr std::size_t kJobWaveGroupLastOffset = 0x38;      // 00874CC4

// ---------------------------------------------------------------------------
// The element, constructed by 00875890
// ---------------------------------------------------------------------------
inline constexpr std::size_t kTickElementPrevOffset = 0x04;      // 00875890 param_1[1]
inline constexpr std::size_t kTickElementNextOffset = 0x08;      // param_1[2]
inline constexpr std::size_t kTickElementGroupOffset = 0x14;     // param_1[5] = argument 3
inline constexpr std::size_t kTickElementLinkedOffset = 0x18;    // byte, 0 at construction
inline constexpr std::size_t kTickElementSubHeadOffset = 0x1c;   // param_1[7]
inline constexpr std::size_t kTickElementSubTailOffset = 0x20;   // param_1[8]
inline constexpr std::size_t kTickElementSubCountOffset = 0x24;  // param_1[9]
inline constexpr std::size_t kTickElementPayloadOffset = 0x28;   // param_1[10] = argument 2
inline constexpr std::size_t kTickElementTimerOffset = 0x30;     // param_1[12] = 00D7A260
inline constexpr std::size_t kTickElementSize = 0x34;            // = kJobWaveGroupStride / 2

// The element vtable slots the waves call, in the base table at 00D0DEC8.
inline constexpr std::size_t kTickElementStepSlot = 0x04;   // 008750B6, 00875172
inline constexpr std::size_t kTickElementPreSlot = 0x08;    // 00874FF6
inline constexpr std::size_t kTickElementPostSlot = 0x0c;   // 008750BF

// The two payload bytes every admission test reads.
inline constexpr std::size_t kTickPayloadGoneByteOffset = 0x5e;    // 00875CC3
inline constexpr std::size_t kTickPayloadActiveByteOffset = 0xbd;  // 00875CC9

// The pending list 00874C90 splices into the groups: head sentinel 00E0B6D0
// (its next slot is 00E0B6D8), tail sentinel 00E0B704 (its prev slot is
// 00E0B708). 00875890 appends here, never straight into a group.
inline constexpr std::uint32_t kPendingListHeadSentinel = 0x00e0b6d0; // 00874CF4
inline constexpr std::uint32_t kPendingListTailSentinel = 0x00e0b704; // 00874C95

// The element's own sub-list, walked by 008759B0 for wave 2.
inline constexpr std::size_t kTickSubNodePrevOffset = 0x08;     // piVar2[2]
inline constexpr std::size_t kTickSubNodeNextOffset = 0x0c;     // piVar2[3]
inline constexpr std::size_t kTickSubNodeExpiredOffset = 0x10;  // byte
inline constexpr std::size_t kTickSubNodeEnabledOffset = 0x11;  // byte

// ---------------------------------------------------------------------------
// The waves
// ---------------------------------------------------------------------------
enum class JobWavePhase : std::uint8_t {
    kWave1,          // 00875CAA..00875D19, factory 008755A0, body 008750A0
    kWave2,          // 00875D1A..00875D89, factory 00875750, body 00875B90
    kWave3,          // 00875D8A..00875DF9, factory 008754D0, body 00874FE0
    kInterpolation,  // 00875F55..00875FC0, factory 00875670, body 00875160
};

// What a wave reads out of one element. Everything else about the element is
// the host's business.
struct JobWaveElementView {
    bool payload_gone_5e{};    // [payload+5Eh] != 0
    bool payload_active_bd{};  // [payload+BDh] != 0
};

// The admission rule. Waves 1..3 want a payload that is not gone and is active;
// the two byte tests appear in opposite orders in waves 1/3 and wave 2, which is
// the same predicate. The interpolation wave drops the active test entirely:
// 00875F73 tests +5Eh and the next instruction is the queue path, with no
// second CMP.
bool fixed_step_wave_admits(JobWavePhase phase, const JobWaveElementView& element) noexcept;

// The descriptor singleton behind each wave.
struct JobWaveDescriptor {
    JobWavePhase phase;
    std::uint32_t factory;          // the lazy-singleton getter
    std::uint32_t singleton_global; // where the pointer is cached
    std::uint32_t allocation_size;  // operator new argument
    std::uint32_t body;             // vtable +0h, the job body
    std::uint32_t dtor_thunk;       // vtable +4h, SUB ECX,4 + JMP
    bool carries_leftover;          // true only for 00875670: [descriptor+8h]
};

inline constexpr std::size_t kJobWaveDescriptorCount = 4;
std::size_t fixed_step_job_wave_count() noexcept;
const JobWaveDescriptor& fixed_step_job_wave(std::size_t index) noexcept;
const JobWaveDescriptor& fixed_step_job_wave_for(JobWavePhase phase) noexcept;

// 00875F50: the interpolation descriptor's only field.
inline constexpr std::size_t kInterpolationJobLeftoverOffset = 0x08;

// ---------------------------------------------------------------------------
// The pool ABI, through BSP_FrameJobPool_GetSingleton 004C1130
//
// The interface is the base subobject at pool+4h (LEA ECX,[EAX+4]).
// docs/GAME_RENDER_FRAME.md reconstructed the pool itself; the render frame
// uses the same two slots.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kFrameJobPoolInterfaceOffset = 0x04;  // 00875CDA
inline constexpr std::size_t kFrameJobPoolQueueSlot = 0x04;        // 00BE3020, RET 8
inline constexpr std::size_t kFrameJobPoolDispatchSlot = 0x08;     // 00BE3150, RET 4
inline constexpr std::uint32_t kFrameJobPoolRunPassByte = 0x00e0b6ce; // 00875D06

// ---------------------------------------------------------------------------
// What 008759B0 does to one sub-node (wave 2's body)
// ---------------------------------------------------------------------------
enum class SubNodeAction : std::uint8_t {
    kSkip,     // neither enabled nor expired
    kRun,      // +11h set: sub->vtable[+0Ch](step)
    kRelease,  // +10h set: unlink under the lock and sub->vtable[0](1)
    kRunAndRelease,
};

SubNodeAction fixed_step_subnode_action(bool enabled_11, bool expired_10) noexcept;

// ---------------------------------------------------------------------------
// The host
//
// One method per native call site of a wave. The element's virtuals are host
// methods because their overrides are a different packet: the base vtable at
// 00D0DEC8 has six slots and ends there (00D0DEE0 is zero). The destructor is
// 00875920; the five after it are stubs: 0042BB70, 0042BB80 and 0042BBA0 are
// RET 4 (one float each), 0042BB90 is RET, and 0042BBB0 is XOR AL,AL / RET, a
// predicate whose default answer is false. The waves use +4h, +8h and +0Ch.
// ---------------------------------------------------------------------------
struct FixedStepJobWaveHost {
    virtual ~FixedStepJobWaveHost() = default;

    // The walk. `group` is 0..4; the host yields the elements of that group in
    // list order (00875CB7 then [element+8h] until the tail sentinel) and fills
    // `view` for the admission test. Returns false when the group is exhausted.
    // It must be a stable read-only walk: the sequence walks a group twice, once
    // to queue and once to run what the pool would have run.
    virtual bool next_element(std::size_t group, std::size_t position,
                              JobWaveElementView& view) = 0;

    // 004C1130 then [[pool+4h]+4h](descriptor, element), once per admitted
    // element. Pool / factory / queue sites per wave:
    //   wave 1         00875CD5 / 00875CDD / 00875CEB
    //   wave 2         00875D45 / 00875D4D / 00875D5B
    //   wave 3         00875DB5 / 00875DBD / 00875DCB
    //   interpolation  00875F7C / 00875F84 / 00875F92
    virtual void queue_job(JobWavePhase phase, std::size_t group, std::size_t position) = 0;

    // 004C1130 then [[pool+4h]+8h](run_pass), once per group and only when the
    // group queued at least one element: 00875CF8/00875D0E, 00875D68/00875D7E,
    // 00875DD8/00875DEE and 00875F9F/00875FB5.
    virtual void dispatch_group(JobWavePhase phase, std::size_t group,
                                std::uint8_t run_pass) = 0;

    // The job bodies, once the pool runs them. They are host methods because
    // what element->vtable[+4h] / [+8h] / [+0Ch] does is not this packet's.
    //
    // 008750A0: vtable[+4h](step), vtable[+0Ch](), then the pose refresh and the
    // cached inverse.
    virtual void run_wave1_job_008750a0(std::size_t group, std::size_t position,
                                        float step) = 0;
    // 00875B90 -> 008759B0: the element's sub-list.
    virtual void run_wave2_job_00875b90(std::size_t group, std::size_t position,
                                        float step) = 0;
    // 00874FE0: vtable[+8h](step), then the pose refresh.
    virtual void run_wave3_job_00874fe0(std::size_t group, std::size_t position,
                                        float step) = 0;
    // 00875160: vtable[+4h](leftover), nothing else.
    virtual void run_interpolation_job_00875160(std::size_t group, std::size_t position,
                                                float leftover) = 0;
};

// One wave over one group. Returns the number of elements it queued, which is
// what the driver's EBP counter holds and what decides the dispatch.
std::size_t run_fixed_step_job_wave_group(
    JobWavePhase phase, std::size_t group, float step_or_leftover,
    std::uint8_t run_pass, FixedStepJobWaveHost& host);

// One wave over all five groups.
std::size_t run_fixed_step_job_wave(
    JobWavePhase phase, float step_or_leftover, std::uint8_t run_pass,
    FixedStepJobWaveHost& host);

// The three per-step waves in listing order, 00875CAA..00875DF9.
void run_fixed_step_job_waves(float step, std::uint8_t run_pass,
                              FixedStepJobWaveHost& host);

} // namespace bsp
