#pragma once

#include "bsp/sound_configuration.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {

// External FMOD contracts: import thunks 00C2DE30/2A/24 and 00C2DDEE.
// This build's stats import takes two output pointers. Results other than
// 2Bh are ignored; extra owner handles in 00A7F560 ignore every result.
class SoundConfigurationShutdownFmodHost {
public:
    virtual ~SoundConfigurationShutdownFmodHost() = default;
    virtual FmodResult dsp_remove(void* dsp) = 0;
    virtual FmodResult dsp_release(void* dsp) = 0;
    virtual FmodResult channel_group_release(void* group) = 0;
    virtual void memory_get_stats(std::int32_t* current,
        std::int32_t* maximum) = 0;
};

// 00A7F560..00A7F63D: ECX=manager, no stack args, RET. Complete release
// policy over existing projections. words_144 is native +144/+148/+14C/
// +150/+154; it must be the owner's live array, not a snapshot. Rereads
// after host calls are intentional. Master +140 and system DSP list +134
// are untouched here. Native dead headers and allocator ABI are not modeled.
void shutdown_sound_configuration_effects_00a7f560(SoundConfigurationState&,
    std::array<std::uint32_t, 5>& words_144,
    SoundConfigurationShutdownFmodHost&);

// 00A7AAF0..00A7AB84: ECX=18h group record, no stack args, RET. Drain DSP
// handles from the current back, then release the group even when null.
// Host callbacks may alter handles/counts, but must not invalidate the live
// group object or leave an empty DSP list before a native back-element read.
void release_sound_configured_group_effects_00a7aaf0(
    SoundConfiguredChannelGroup&, SoundConfigurationShutdownFmodHost&);

// 00A7BFE0..00A7C051 / 00A7C6C0..00A7C731: ECX=record, no stack args,
// RET. Complete ordinary storage-destruction policy, including disk-verified
// tails missing from saved Ghidra bodies. Free inner handle-array storage,
// then name storage; never release pointed-to FMOD objects. std::vector and
// std::string substitute native array/pooled-string allocators and SEH.
// Native leaves dead pointer/capacity words; C++ empties owned containers.
void destroy_sound_configured_group_storage_00a7bfe0(
    SoundConfiguredChannelGroup&);
void destroy_sound_configured_type_storage_00a7c6c0(SoundConfiguredType&);

// Partial projections of native __thiscall(header, signed count), RET4.
// Require requested_count <= current size <= INT32_MAX and a valid header.
// Cover only shrink/equal paths, retaining outer backing and logical capacity.
// Growth/reserve branches are excluded, not stubbed. Groups/types decrement
// the visible count before destroying each removed record; neither releases
// FMOD objects. Native ranges and excluded branches: SOUND_CONFIGURATION_SHUTDOWN.md.
void shrink_sound_system_dsps_00a7a980_fragment(SoundConfigurationState&,
    std::size_t requested_count);
void shrink_sound_configured_groups_00a7f240_fragment(SoundConfigurationState&,
    std::size_t requested_count);
void shrink_sound_configured_types_00a7fcc0_fragment(SoundConfigurationState&,
    std::size_t requested_count);

} // namespace bsp
