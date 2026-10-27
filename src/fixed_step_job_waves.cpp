#include "bsp/fixed_step_job_waves.hpp"

#include <cstring>

// The four job waves of BSP_Game_RunFixedSimulationSteps 00875BB0:
// 00875CAA..00875D19, 00875D1A..00875D89, 00875D8A..00875DF9 inside the step
// loop, and 00875F55..00875FC0 once per frame. docs/FIXED_STEP_JOB_WAVES.md.
//
// Every wave is the same loop: walk the group's elements, queue a job for each
// one the admission test accepts, and dispatch the group once when at least one
// job was queued. The four differ only in the descriptor factory, the admission
// test and the float the job body passes on.

namespace bsp {
namespace {

constexpr JobWaveDescriptor kDescriptors[kJobWaveDescriptorCount] = {
    // phase, factory, singleton, allocation, body, dtor thunk, carries leftover
    {JobWavePhase::kWave1, 0x008755a0, 0x00f878d4, 0x08, 0x008750a0, 0x00875070, false},
    {JobWavePhase::kWave2, 0x00875750, 0x00f878dc, 0x08, 0x00875b90, 0x00875210, false},
    {JobWavePhase::kWave3, 0x008754d0, 0x00f878d0, 0x08, 0x00874fe0, 0x00874fb0, false},
    {JobWavePhase::kInterpolation, 0x00875670, 0x00f878d8, 0x0c, 0x00875160, 0x00875180, true},
};

} // namespace

bool fixed_step_wave_admits(JobWavePhase phase, const JobWaveElementView& element) noexcept
{
    if (phase == JobWavePhase::kInterpolation) {
        return !element.payload_gone_5e; // 00875F73 only
    }
    // 00875CC3 + 00875CC9 (wave 1), 00875D33 + 00875D3A in the other order
    // (wave 2), 00875DA3 + 00875DA9 (wave 3).
    return !element.payload_gone_5e && element.payload_active_bd;
}

std::size_t fixed_step_job_wave_count() noexcept
{
    return kJobWaveDescriptorCount;
}

const JobWaveDescriptor& fixed_step_job_wave(std::size_t index) noexcept
{
    if (index >= kJobWaveDescriptorCount) {
        index = kJobWaveDescriptorCount - 1;
    }
    return kDescriptors[index];
}

const JobWaveDescriptor& fixed_step_job_wave_for(JobWavePhase phase) noexcept
{
    for (const JobWaveDescriptor& row : kDescriptors) {
        if (row.phase == phase) {
            return row;
        }
    }
    return kDescriptors[0];
}

SubNodeAction fixed_step_subnode_action(bool enabled_11, bool expired_10) noexcept
{
    // 008759B0: the enabled test comes first and the expired test decides
    // whether the same sub-node is then unlinked and destroyed, so both can
    // happen to one sub-node in one visit.
    if (enabled_11 && expired_10) {
        return SubNodeAction::kRunAndRelease;
    }
    if (enabled_11) {
        return SubNodeAction::kRun;
    }
    if (expired_10) {
        return SubNodeAction::kRelease;
    }
    return SubNodeAction::kSkip;
}

namespace {

std::uint8_t* load_ptr(const std::uint8_t* base, std::size_t offset) noexcept
{
    std::uint8_t* value = nullptr;
    std::memcpy(&value, base + offset, sizeof(value));
    return value;
}

void store_ptr(std::uint8_t* base, std::size_t offset, std::uint8_t* value) noexcept
{
    std::memcpy(base + offset, &value, sizeof(value));
}

void add_i32(std::uint8_t* base, std::size_t offset, std::int32_t delta) noexcept
{
    std::int32_t value = 0;
    std::memcpy(&value, base + offset, sizeof(value));
    value = static_cast<std::int32_t>(static_cast<std::uint32_t>(value) + static_cast<std::uint32_t>(delta));
    std::memcpy(base + offset, &value, sizeof(value));
}

std::int32_t load_i32(const std::uint8_t* base, std::size_t offset) noexcept
{
    std::int32_t value = 0;
    std::memcpy(&value, base + offset, sizeof(value));
    return value;
}

} // namespace

void tick_element_run_subnodes_008759b0(std::uint8_t* element, float step,
                                        TickSubNodeHost& host) noexcept
{
    std::uint8_t* sub = load_ptr(element, kTickElementSubHeadOffset); // 008759B6
    while (sub != nullptr) {                                          // 008759BD / 00875A69
        std::uint8_t* const next = load_ptr(sub, kTickSubNodeNextOffset); // 008759C7, [ESP+10h]
        if (sub[kTickSubNodeEnabledOffset] != 0) {                    // 008759C4
            host.step_subnode(sub, step);                             // 008759DF
        }
        if (sub[kTickSubNodeExpiredOffset] != 0) {                    // 008759E1
            std::uint8_t* const lock = host.pending_registry_lock();  // 008759E6..008759EB
            if (lock != nullptr) {
                host.enter_lock(lock);                                // 008759F3
                add_i32(lock, kTickSubListLockDepthOffset, 1);        // 008759F9
            }
            // The links are read again here: the step call may have moved them.
            std::uint8_t* const prev = load_ptr(sub, kTickSubNodePrevOffset); // 008759FD
            const bool off_list = prev == nullptr &&
                load_ptr(sub, kTickSubNodeNextOffset) == nullptr &&   // 00875A04
                load_i32(element, kTickElementSubCountOffset) > 1;    // 00875A09 JG
            if (!off_list) {
                std::uint8_t* const after = load_ptr(sub, kTickSubNodeNextOffset);
                if (prev != nullptr) {
                    store_ptr(prev, kTickSubNodeNextOffset, after);   // 00875A16
                } else {
                    store_ptr(element, kTickElementSubHeadOffset, after); // 00875A1E
                }
                std::uint8_t* const before = load_ptr(sub, kTickSubNodePrevOffset);
                if (after != nullptr) {
                    store_ptr(after, kTickSubNodePrevOffset, before); // 00875A2B
                } else {
                    store_ptr(element, kTickElementSubTailOffset, before); // 00875A33
                }
                store_ptr(sub, kTickSubNodeNextOffset, nullptr);      // 00875A36
                store_ptr(sub, kTickSubNodePrevOffset, nullptr);      // 00875A39
                add_i32(element, kTickElementSubCountOffset, -1);     // 00875A3C
            }
            store_ptr(sub, kTickSubNodeOwnerOffset, nullptr);         // 00875A42
            if (lock != nullptr) {
                add_i32(lock, kTickSubListLockDepthOffset, -1);       // 00875A47
                host.leave_lock(lock);                                // 00875A4C
            }
            host.destroy_subnode(sub);                                // 00875A5A
            if (load_i32(element, kTickElementSubCountOffset) == 0) { // 00875A5C JZ exit
                break;
            }
        }
        sub = next;                                                   // 00875A61 / 00875A67
    }
}

std::size_t run_fixed_step_job_wave_group(
    JobWavePhase phase, std::size_t group, float step_or_leftover,
    std::uint8_t run_pass, FixedStepJobWaveHost& host)
{
    // Pass 1, the walk: 00875CC0..00875CF2. Every admitted element is queued;
    // nothing runs yet.
    std::size_t queued = 0; // EBP at 00875CBA, EBX at 00875F65
    std::size_t position = 0;
    JobWaveElementView view{};
    while (host.next_element(group, position, view)) {
        if (fixed_step_wave_admits(phase, view)) {
            ++queued;
            host.queue_job(phase, group, position);
        }
        ++position;
        view = JobWaveElementView{};
    }
    if (queued == 0) { // 00875CF4 TEST EBP,EBP / JE 0x875D10
        return 0;
    }

    // 00875CF8..00875D0E: one dispatch for the whole group.
    host.dispatch_group(phase, group, run_pass);

    // The queued bodies run inside that dispatch, on the pool's workers, in an
    // order the driver does not fix. A single-threaded host runs them here in
    // list order instead. The second walk sees the same set because the
    // admission test reads only the two payload bytes and nothing between the
    // walks writes them; a host whose next_element is not a stable read-only
    // walk must run the bodies from dispatch_group and leave these unused.
    position = 0;
    view = JobWaveElementView{};
    while (host.next_element(group, position, view)) {
        if (fixed_step_wave_admits(phase, view)) {
            switch (phase) {
            case JobWavePhase::kWave1:
                host.run_wave1_job_008750a0(group, position, step_or_leftover);
                break;
            case JobWavePhase::kWave2:
                host.run_wave2_job_00875b90(group, position, step_or_leftover);
                break;
            case JobWavePhase::kWave3:
                host.run_wave3_job_00874fe0(group, position, step_or_leftover);
                break;
            case JobWavePhase::kInterpolation:
                host.run_interpolation_job_00875160(group, position, step_or_leftover);
                break;
            }
        }
        ++position;
        view = JobWaveElementView{};
    }
    return queued;
}

std::size_t run_fixed_step_job_wave(
    JobWavePhase phase, float step_or_leftover, std::uint8_t run_pass,
    FixedStepJobWaveHost& host)
{
    std::size_t queued = 0;
    for (std::size_t group = 0; group < kJobWaveGroupCount; ++group) {
        queued += run_fixed_step_job_wave_group(
            phase, group, step_or_leftover, run_pass, host);
    }
    return queued;
}

void run_fixed_step_job_waves(float step, std::uint8_t run_pass,
                              FixedStepJobWaveHost& host)
{
    run_fixed_step_job_wave(JobWavePhase::kWave1, step, run_pass, host); // 00875CAA
    run_fixed_step_job_wave(JobWavePhase::kWave2, step, run_pass, host); // 00875D1A
    run_fixed_step_job_wave(JobWavePhase::kWave3, step, run_pass, host); // 00875D8A
}

} // namespace bsp
