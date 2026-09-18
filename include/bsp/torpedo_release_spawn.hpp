// The plane's actuator block at unit+DECh, and the ordnance release request
// that drives one of its channels.
//
// Packet cc8_torpedo_release_spawn. docs/TORPEDO_RELEASE_SPAWN.md.
//
// The block is built by 007EABC0 from BSP_Plane_ReadPropertyBag: the allocation
// is PUSH 0x80 at 007D618E, the constructor call is 007D61AB and the store into
// the unit is 007D61B4 MOV [ESI+0xDEC],EAX. It holds three animated 0..1
// channels at +28h, +44h and +60h plus the aggregate moving flag at +11h. It is
// NOT an ordnance device: neither it nor its tick reads ordnance, calls a spawn
// or touches a projectile.
#ifndef BSP_TORPEDO_RELEASE_SPAWN_HPP
#define BSP_TORPEDO_RELEASE_SPAWN_HPP

namespace bsp {

// The PilotBot `SPNormal` row of the installed scripts/datatables/robots.lua,
// the three torpedo-run values the aim tick's profile seed wants. Authored
// content, not recovered code: the registry that would carry them into the
// units host is not reachable from there, and the difficulty index at
// [[unit+DF4h]+34h] that would pick the row is unmodelled, so the row is named
// rather than chosen. SPVeteran authors 5 / 800 / 1200 and the two MP rows
// 10 / 800 / 1200. docs/TORPEDO_RELEASE_GEOMETRY.md.
inline constexpr float kTorpReleaseAltSPNormal = 12.0f;       // "TorpReleaseAlt", metres
inline constexpr float kTorpReleaseDistNearSPNormal = 450.0f; // "TorpReleaseDistNear", metres
inline constexpr float kTorpReleaseDistFarSPNormal = 650.0f;  // "TorpReleaseDistFar", metres


// One channel of the block. The field names are hypotheses; the offsets are the
// ones 007DE1E0 reads, relative to the channel base.
struct PlaneActuatorChannel {
    bool enabled{false};   // +0, the channel is present
    bool target{false};    // +1, the commanded end state
    float value{-1.0f};    // +4, clamped into [0, 1]; the constructor leaves -1.0f
    bool moving{true};     // +8, the constructor leaves this set
    float rate{0.0f};      // +0Ch, per second
    unsigned frame{0};     // +18h, the DAT_00F876A4 stamp
};

// 007DE1E0 BSP_PlaneActuatorChannel_Step, void __thiscall(channel, float dt).
// When the value already sits at the end its target asks for (1.0f with the
// target set, 0.0f with it clear) it clears `moving` and returns. Otherwise it
// sets `moving`, integrates value += rate * (+1 or -1) * dt, clamps into
// [0, 1] and stamps the frame. DAT_00D7A24C is 1.0f and DAT_00D7A260 is -1.0f,
// both read from the image.
void plane_actuator_channel_step_007de1e0(PlaneActuatorChannel& channel, float dt,
    unsigned frame);

// The block at unit+DECh. Channel C is the one 007BBBA0 drives.
struct PlaneActuatorBlock {
    bool moving_any{true};             // +11h, the constructor leaves this set
    PlaneActuatorChannel channel_b{};  // +28h
    PlaneActuatorChannel channel_a{};  // +44h
    PlaneActuatorChannel channel_c{};  // +60h, the ordnance bay
};

// 007DE3A0 BSP_PlaneActuatorBlock_Step, void __thiscall(this, float dt), RET 4.
// Slot 3 of vtable 00D0862C. Steps +44h, +28h then +60h in that order, each
// call guarded by `enabled == 0 || value == 1.0f`, then clears +11h at 007DE417
// when all three moving bytes are down.
void plane_actuator_block_step_007de3a0(PlaneActuatorBlock& block, float dt,
    unsigned frame);

// What 007BBBA0 does to the block, with the two guards it applies: the channel
// must be enabled (+60h) and the bay must not already be fully open
// (+64h != 1.0f). Returns true when the request moved the channel, which is the
// condition under which the native raises +61h, +68h and +11h. The caller owns
// the third guard, the per-slot byte at unit+9C3h[[00F876B8]*8], because that
// byte lives on the unit and not on the block.
bool ordnance_release_request_007bbba0(PlaneActuatorBlock& block);

// 007BBC00, the last instruction before POP EBX/RET: the counter is raised on
// every path, past all three early exits, whether or not the block accepted the
// request.
int release_request_raise_007bbc00_counter(int counter);

}  // namespace bsp

#endif  // BSP_TORPEDO_RELEASE_SPAWN_HPP
