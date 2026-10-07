#pragma once
#include "bsp/native_pilot_bot_task_owner.hpp"

namespace bsp {
// Stable borrowed SOURCE identities. These are not binary overlays, native
// addresses or ownership solutions; every nonzero mapping must remain live.
struct NativePilotBotSquadronHandle { std::uintptr_t identity; };
struct NativePilotBotCommandDescriptorHandle { std::uintptr_t identity; };
struct NativePilotBotCommandTargetHandle { std::uintptr_t identity; };
struct NativePilotBotAirBlockHandle { std::uintptr_t identity; };

// Every provider is REQUIRED and bound to the same bot as the passed owner.
// Actual native profiles, allocator/task arena, target resolver and world
// predicates remain external. Unsupported world/profile domains are excluded.
class NativePilotBotCommandProducerCalls {
 public:
    virtual ~NativePilotBotCommandProducerCalls() = default;
    // Fresh bot+50h -> plane+9D4h read at EACH native observation. Only the
    // initial read has a null-squadron early exit; later callees need their
    // actual admitted receiver domain. Never substitute a task's cached squad.
    virtual NativePilotBotSquadronHandle current_squadron_50_9d4() = 0;
    virtual NativePilotBotCommandControllerHandle command_controller_114(
        NativePilotBotSquadronHandle) = 0;
    virtual std::uint32_t command_token_0071be40(NativePilotBotCommandControllerHandle) = 0;
    virtual NativePilotBotCommandDescriptorHandle active_descriptor_0071eb60(
        NativePilotBotCommandControllerHandle) = 0;
    virtual NativePilotBotCommandTargetHandle resolve_target_00521ea0(
        NativePilotBotCommandDescriptorHandle) = 0;
    // Native stack captured target/1/1, ECX=fresh current squadron.
    virtual std::uint32_t choose_attack_token_007eec50(NativePilotBotSquadronHandle,
        NativePilotBotCommandTargetHandle, bool prefer_ordnance, bool allow_guns) = 0;
    // COMPLETE actual007F16D0 service, including normal world/command side
    // effects and real output-record production. This facade observes only
    // word0 of its native1Ch output; the initial captured target stays intact.
    // This signature does not reproduce the native output-record/stack ABI.
    virtual std::uint32_t resolve_return_token_007f16d0(NativePilotBotSquadronHandle) = 0;
    virtual bool target_kind_5c(NativePilotBotCommandTargetHandle, std::uint32_t kind) = 0;
    virtual bool target_attackable_009229f0(NativePilotBotCommandTargetHandle,
        std::uint32_t kind) = 0;  // full native EDX=6, AL result
    virtual NativePilotBotAirBlockHandle air_block_006bcd20(
        NativePilotBotCommandTargetHandle, bool native_dl_flag) = 0; // DL=1; meaning unproved
    virtual bool squadron_admitted_006c4790(NativePilotBotAirBlockHandle,
        NativePilotBotSquadronHandle) = 0;

    // Actual factories: ECX=same bot; EDX=the named argument where present.
    // Successful results are stable live tasks, suitable for the same owner's
    // active/retired arrays and required profile callbacks. Zero means no task.
    virtual NativePilotBotTaskHandle make_default_009c3c40(NativePilotBotCommandDescriptorHandle) = 0;
    virtual NativePilotBotTaskHandle make_moveto_009c3be0(NativePilotBotCommandDescriptorHandle) = 0;
    virtual NativePilotBotTaskHandle make_moveonpath_009bdbb0() = 0;
    virtual NativePilotBotTaskHandle make_divebomb_009c8c70(NativePilotBotCommandTargetHandle) = 0;
    virtual NativePilotBotTaskHandle make_levelbomb_009b9030(NativePilotBotCommandTargetHandle) = 0;
    virtual NativePilotBotTaskHandle make_dropkamikaze_009aebe0(NativePilotBotCommandTargetHandle) = 0;
    virtual NativePilotBotTaskHandle make_torpedo_009d4e30(NativePilotBotCommandTargetHandle) = 0;
    virtual NativePilotBotTaskHandle make_strafe_009cd300(NativePilotBotCommandTargetHandle) = 0;
    virtual NativePilotBotTaskHandle make_rocket_007b7fd0(NativePilotBotCommandTargetHandle) = 0;
    virtual NativePilotBotTaskHandle make_kamikaze_009af720(NativePilotBotCommandTargetHandle) = 0;
    virtual NativePilotBotTaskHandle make_dogfight_009ab570(NativePilotBotCommandTargetHandle) = 0;
    virtual NativePilotBotTaskHandle make_land_009b41c0(NativePilotBotAirBlockHandle) = 0;
    virtual NativePilotBotTaskHandle make_close_009a2f40(NativePilotBotCommandTargetHandle) = 0;
    virtual NativePilotBotTaskHandle make_depthcharge_009a6970(NativePilotBotCommandTargetHandle) = 0;
    virtual NativePilotBotTaskHandle make_retreat_009ca2b0() = 0;
    virtual NativePilotBotTaskHandle make_stop_009badb0() = 0;
};

// Complete conditional normal CALLER flow of0099A170..0099A49D, ECX=bot,
// plain RET. Providers return normally and cannot reenter this operation,
// mutate owner array structure, invalidate captured controller/target/block
// mappings or alias array storage. Current squadron/descriptor observations
// may change between callbacks; the source preserves EACH fresh read.
// The existing append provider domain (successful disjoint2*n+2 allocation)
// applies if a nonzero task is produced. No retired drain or scheduler/head54
// call is introduced. Actual callee implementations and native ABI/private
// EH/world lifetimes/game binding remain unbound. This is a new C++ API.
void install_native_pilot_bot_command_task_0099a170(NativePilotBotTaskOwnerView&,
    NativePilotBotTaskOwnerCalls&, NativePilotBotCommandProducerCalls&);
} // namespace bsp
