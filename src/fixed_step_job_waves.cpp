#include "bsp/fixed_step_job_waves.hpp"

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
