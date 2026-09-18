#include "bsp/torpedo_release_spawn.hpp"

// Packet cc8_torpedo_release_spawn. docs/TORPEDO_RELEASE_SPAWN.md.
//
// Addresses: 007DE1E0, 007DE3A0, 007BBBA0, 007BBC00, 007EABC0.
//
// The two image constants these routines compare and clamp against:
//   DAT_00D7A24C = 1.0f   (00 00 80 3F)
//   DAT_00D7A260 = -1.0f  (00 00 80 BF)
// Both read from the image, not assumed.

namespace bsp {
namespace {

constexpr float kActuatorTop = 1.0f;     // DAT_00D7A24C
constexpr float kActuatorDown = -1.0f;   // DAT_00D7A260

}  // namespace

void plane_actuator_channel_step_007de1e0(PlaneActuatorChannel& channel, float dt,
    unsigned frame) {
    // 007DE1E5..007DE20x. The settle test is written as two disjunctions in the
    // image and reads as "the value is already at the end the target asks for":
    //   (value != 1.0f || target == 0) && (value != 0.0f || target != 0)
    // Negated, the early return is `(value == 1.0f && target) || (value == 0.0f
    // && !target)`. Kept in the image's form so the float comparisons stay
    // exact; a tolerance here would change which frame the channel stops on.
    const bool at_top = !(channel.value != kActuatorTop) && channel.target;
    const bool at_bottom = !(channel.value != 0.0f) && !channel.target;
    if (at_top || at_bottom) {
        channel.moving = false;
        return;
    }
    channel.moving = true;
    const float direction = channel.target ? kActuatorTop : kActuatorDown;
    channel.value = channel.rate * direction * dt + channel.value;
    // 007DE22x: the clamp is written as "if (0 <= value) { if (1 < value) value
    // = 1 } else value = 0", so a NaN would fall to the else and land on 0.
    if (0.0f <= channel.value) {
        if (kActuatorTop < channel.value) channel.value = kActuatorTop;
    } else {
        channel.value = 0.0f;
    }
    channel.frame = frame;
}

void plane_actuator_block_step_007de3a0(PlaneActuatorBlock& block, float dt,
    unsigned frame) {
    // 007DE3A9, 007DE3D2, 007DE3FA: the order is +44h, +28h, +60h, and each
    // call is guarded by `enabled == 0 || value == 1.0f`. The guard is the
    // image's, not a shortcut: a channel that is disabled or already at the top
    // still enters the step, which is where its moving byte gets cleared.
    PlaneActuatorChannel* const order[3] = {
        &block.channel_a, &block.channel_b, &block.channel_c};
    for (PlaneActuatorChannel* channel : order) {
        if (!channel->enabled || !(channel->value != kActuatorTop)) {
            plane_actuator_channel_step_007de1e0(*channel, dt, frame);
        }
    }
    // 007DE405..007DE417: +11h falls only when all three moving bytes are down.
    if (!block.channel_a.moving && !block.channel_b.moving && !block.channel_c.moving) {
        block.moving_any = false;
    }
}

bool ordnance_release_request_007bbba0(PlaneActuatorBlock& block) {
    // 007BBBA6: the channel's enabled byte at +60h.
    if (!block.channel_c.enabled) return false;
    // 007BBBB2: the bay must not already be fully open. The image compares
    // +64h against DAT_00D7A24C and leaves on equality.
    if (!(block.channel_c.value != kActuatorTop)) return false;
    // 007BBBD0: re-arm only when the channel is not already commanded open at
    // the top. Both conjuncts are the image's.
    if (block.channel_c.target != true || block.channel_c.value != kActuatorTop) {
        block.channel_c.value = kActuatorTop;
        block.channel_c.target = true;
        block.channel_c.moving = true;
    }
    // 007BBBF7: the aggregate flag is raised only when the channel is moving.
    if (block.channel_c.moving) {
        block.moving_any = true;
        return true;
    }
    return false;
}

int release_request_raise_007bbc00_counter(int counter) {
    // 007BBC00 ADD dword [ECX+0xC20], EBX with EBX = 1 from 007BBBAB, past all
    // three early exits at 007BBBB0, 007BBBC6 and 007BBBDC.
    return counter + 1;
}

}  // namespace bsp
