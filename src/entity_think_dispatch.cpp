// Entity think dispatch, 00929460 and its helpers. docs/ENTITY_THINK_DISPATCH.md.
// Read-only analysis of the image; no Ghidra mutation was made from this packet.
#include "bsp/entity_think_dispatch.hpp"

#include <algorithm>

namespace bsp {
namespace {

// The three think fields are contiguous on the entity: a char* at +1D8h, the
// delay-armed byte at +1DCh and the float delay at +1E0h. The first two are declared
// by docs/LUA_BINDING_CORE.md's header and reused here rather than redeclared.
static_assert(kEntityThinkStateByteOffset == kEntityThinkScriptNameOffset + 4,
              "0088A333 and 0088A37F");
static_assert(kEntityThinkDelayOffset == kEntityThinkStateByteOffset + 4,
              "008982CB and 008982D1");

const EntityThinkFields* find_fields(const std::vector<EntityThinkFields>& fields,
                                     std::uint32_t entity) noexcept {
    for (const EntityThinkFields& candidate : fields) {
        if (candidate.entity == entity) {
            return &candidate;
        }
    }
    return nullptr;
}

} // namespace

bool entity_think_entry_eligible(const EntityThinkFields& fields) noexcept {
    // 00929487 JZ, 00929494 JNZ, 0092949A JNZ, 009294A0 JNZ, 009294A9 JZ: one failure
    // is enough and the order is initialised, 5Dh, 60h, 5Eh, name.
    return fields.initialised && !fields.blocked_5d && !fields.blocked_60 &&
           !fields.blocked_5e && fields.has_think_name;
}

float entity_think_delay_after_step(float delay_seconds, float step) noexcept {
    return delay_seconds - step; // 009294CB FSUB float ptr [ESP+10h]
}

float script_countdown_after_step(float countdown, float step) noexcept {
    return countdown - step; // 0092946B FSUB float ptr [ESP+4]
}

bool script_countdown_expired(float countdown) noexcept {
    // 009294EC COMISS XMM1,[00F89A04] with XMM1 zeroed, JBE skips the call: the test
    // is strict, zero does not fire.
    return countdown < 0.0f;
}

float script_countdown_refilled(float countdown) noexcept {
    // 00929557 FADD double ptr [00D7A2B0]. Additive, so the phase of the pass is kept.
    return static_cast<float>(static_cast<double>(countdown) + kScriptCountdownPeriodSeconds);
}

float clamp_think_delay(float requested_seconds) noexcept {
    // 008982B0 COMISS against 00CE3800 = 0.5f, the larger value wins.
    return std::max(requested_seconds, kMinimumThinkDelaySeconds);
}

EntityThinkBranch entity_think_branch(const EntityThinkFields& fields,
                                      float step,
                                      float countdown) noexcept {
    if (!entity_think_entry_eligible(fields)) {
        return EntityThinkBranch::Erase;
    }
    // 009294AB CMP byte [ECX+1DCh],0 / 009294BC COMISS: an armed delay that has
    // already run out falls through to the untimed branch, it is not re-armed here.
    if (fields.delay_armed && fields.delay_seconds > 0.0f) {
        const float remaining = entity_think_delay_after_step(fields.delay_seconds, step);
        return remaining <= 0.0f ? EntityThinkBranch::TimedFire : EntityThinkBranch::TimedWait;
    }
    return script_countdown_expired(countdown) ? EntityThinkBranch::UntimedFire
                                               : EntityThinkBranch::UntimedWait;
}

EntityThinkRunSummary run_entity_think_list_00929460(
    float step,
    float& countdown,
    EntityThinkList& live,
    EntityThinkList& pending,
    const std::vector<EntityThinkFields>& fields,
    EntityThinkHost& host) {
    EntityThinkRunSummary summary{};

    // 1. 00929460-00929477: the step comes off the countdown before the walk, so the
    //    untimed branch tests the value this call already reduced.
    countdown = script_countdown_after_step(countdown, step);

    // 2. 0092947D-0092953B. The native cursor captures the successor at 0092948B
    //    before the body runs, which is why an erase does not end the walk: 00929531
    //    falls into 00929534 TEST ESI,ESI and the loop resumes at 00929484. The
    //    earlier reading in docs/FIXED_STEP_FANOUT.md said the routine returned after
    //    the first erase; the listing says otherwise.
    std::vector<EntityThinkNode> survivors{};
    survivors.reserve(live.nodes.size());
    for (const EntityThinkNode& node : live.nodes) {
        const EntityThinkFields* state = find_fields(fields, node.entity);
        const EntityThinkBranch branch =
            state ? entity_think_branch(*state, step, countdown) : EntityThinkBranch::Erase;
        switch (branch) {
            case EntityThinkBranch::Erase:
                host.free_think_node_0092952c(node); // 0092952C, count -= 1 at 00929524
                ++summary.nodes_erased;
                continue;
            case EntityThinkBranch::TimedFire:
                host.run_entity_think_00929150(node.entity); // 009294E5
                ++summary.thinks_run;
                break;
            case EntityThinkBranch::UntimedFire:
                host.run_entity_think_00929150(node.entity); // 009294F5
                ++summary.thinks_run;
                break;
            case EntityThinkBranch::TimedWait:
            case EntityThinkBranch::UntimedWait:
                break;
        }
        survivors.push_back(node);
    }
    live.nodes.swap(survivors);

    // 3. 00929542-0092958B. The refill and the collection happen after the walk, so
    //    every untimed think of this call and the collection share one expiry.
    if (script_countdown_expired(countdown)) {
        summary.countdown_expired = true;
        countdown = script_countdown_refilled(countdown);
        if (host.gc_gate_predicate_0109cefc_vtable0c()) { // 00929568, AL tested at 0092956A
            host.lua_run_string_006b8ad0(kCollectGarbageChunk, kCollectGarbageRunMode);
            summary.garbage_collected = true;
        }
    }

    // 4. 0092958D-009295A1: splice, then clear. A registration made during this call
    //    first thinks on the next one.
    summary.pending_spliced = pending.nodes.size();
    host.splice_pending_into_live_00928380(live, pending);
    host.clear_pending_00928330(pending);
    return summary;
}

void register_pending_think_entity_0088a240(EntityThinkList& pending,
                                            std::uint32_t entity) {
    // 0088A240: tail append of a fresh 0Ch-byte node, count += 1 at 0088A28D. The
    // caller 0088A330 reaches it only when entity+1D8h was null and a name is being
    // set (0088A333 / 0088A341), so duplicates do not arise from SetThink.
    pending.nodes.push_back(EntityThinkNode{entity});
}

bool erase_think_entity_00928300(EntityThinkList& list, std::uint32_t entity) noexcept {
    // 00928300 walks from the head comparing node+8h, then 00928030 unlinks, drops the
    // count and frees, returning the successor. Both call sites (00929AA7, 00929AB2)
    // ignore the result. Only the first match is removed.
    for (std::size_t i = 0; i < list.nodes.size(); ++i) {
        if (list.nodes[i].entity == entity) {
            list.nodes.erase(list.nodes.begin() + static_cast<std::ptrdiff_t>(i));
            return true;
        }
    }
    return false;
}

} // namespace bsp
