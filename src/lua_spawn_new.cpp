// Packet cc8_spawn_new_route. See include/bsp/lua_spawn_new.hpp for the route
// and the addresses; this file holds only the queue and the frame the drain
// needs. No host, no Lua: the mission Lua host in src/game_hosts_lua.cpp is the
// only caller.
#include "bsp/lua_spawn_new.hpp"

#include <algorithm>
#include <cmath>

namespace bsp {
namespace {

// 00E0CF74. A process global, not a per-mission one.
std::uint32_t g_spawn_request_serial = 1u;
constexpr std::uint32_t kSerialWrapAbove = 16000u;

}  // namespace

std::uint32_t next_spawn_request_serial_00949f2b() noexcept {
    // 00949F2B MOV EAX,[00E0CF74] / CMP EAX,0x3e80 / JBE +5 / MOV EAX,1, then
    // 00949F41 MOV ECX,EAX (the serial this request gets) / ADD EAX,1 /
    // 00949F47 MOV [00E0CF74],EAX. The wrap is tested BEFORE the value is
    // handed out, so 16001 is never issued.
    if (g_spawn_request_serial > kSerialWrapAbove) g_spawn_request_serial = 1u;
    const std::uint32_t issued = g_spawn_request_serial;
    g_spawn_request_serial = issued + 1u;
    return issued;
}

void reset_spawn_request_serial_for_tests() noexcept { g_spawn_request_serial = 1u; }

void SpawnRequestQueue::enqueue_00949530(SpawnNewRequest request) {
    requests_.push_back(std::move(request));
}

void SpawnRequestQueue::requeue_009478b0(SpawnNewRequest request) {
    requests_.push_back(std::move(request));
}

bool SpawnRequestQueue::attempt_due(float now, float interval) const noexcept {
    // 0094C4EA FADD float ptr [EDI + 0xc] then the compare against DAT_00F876A4,
    // whose fail arm returns. The sense here is the arm that does NOT return.
    return !(now < interval + last_attempt_);
}

std::size_t SpawnRequestQueue::select_0094c508(
    const std::vector<bool>& party_active) const noexcept {
    if (requests_.empty()) return 0;
    // 0094C4F7 CMP EAX,2 / JC 0094C56B: fewer than two records takes the head
    // without testing its party at all.
    if (requests_.size() < 2) return 0;
    for (std::size_t i = 0; i < requests_.size(); ++i) {
        const std::int32_t party = requests_[i].party;
        // 0094C538 CMP EAX,EBX / JL 0094C553 skips a negative party.
        if (party < 0) continue;
        if (static_cast<std::size_t>(party) >= party_active.size()) continue;
        if (party_active[static_cast<std::size_t>(party)]) return i;
    }
    // The walk fell off the end at 0094C518 and joined 0094C56B, the head arm.
    return 0;
}

SpawnNewRequest SpawnRequestQueue::erase_009439b0(std::size_t index) {
    SpawnNewRequest taken = std::move(requests_[index]);
    requests_.erase(requests_.begin() + static_cast<std::ptrdiff_t>(index));
    return taken;
}

bool SpawnRequestQueue::id_is_requested_00945850(const std::string& id) const noexcept {
    for (const SpawnNewRequest& request : requests_) {
        if (request.id == id) return true;
    }
    return false;
}

std::size_t SpawnRequestQueue::remove_id_00945a20(const std::string& id) {
    // Every match, not the first: the twelve bytes at 00945BA7 that Ghidra left
    // undisassembled jump back into the scan at 00945B07.
    const std::size_t before = requests_.size();
    requests_.erase(std::remove_if(requests_.begin(), requests_.end(),
                                   [&id](const SpawnNewRequest& r) { return r.id == id; }),
                    requests_.end());
    return before - requests_.size();
}

void SpawnRequestQueue::clear() noexcept {
    requests_.clear();
    last_attempt_ = 0.0f;
}

SpawnRequestQueue& spawn_request_queue() {
    static SpawnRequestQueue queue;
    return queue;
}

namespace {
SpawnQueueDrain* g_spawn_queue_drain = nullptr;
}  // namespace

void set_spawn_queue_drain(SpawnQueueDrain* drain) noexcept {
    g_spawn_queue_drain = drain;
}

void run_spawn_queue_step_0094c8f0(float scaled_delta) {
    if (g_spawn_queue_drain == nullptr) return;
    g_spawn_queue_drain->run_spawn_queue_0094c490(scaled_delta);
}

SpawnNewFrame spawn_member_frame_0094a140(const SpawnNewRequest& request,
                                          std::size_t member) noexcept {
    SpawnNewFrame frame;
    frame.position[0] = request.ref_pos[0];
    frame.position[1] = request.ref_pos[1];
    frame.position[2] = request.ref_pos[2];

    // CONTRACT, not a reading of 0094A140. What is read from the listing is
    // that the routine composes a frame from the record's ranges, hands it to
    // 00949300 and, when that refuses, rebuilds it through
    // BSP_Matrix_BuildRotationY with a different angle and tries again. The
    // sampling rule inside that loop was not decoded, and the four floats
    // 00949750 stages at 00949F67..00949F8C were not traced to their record
    // slots either - the push accounting across the SUB ESP,0x10 and the five
    // intervening pushes did not close, and a slot claim that does not close is
    // exactly the trap AGENTS.md names. So the fan-out below is this process's
    // own rule over the ranges the binding DID read by name.
    const std::size_t count = request.members.empty() ? 1 : request.members.size();
    const float t = count < 2 ? 0.5f
                              : static_cast<float>(member) / static_cast<float>(count - 1);
    const float angle = request.has_angle_range
                            ? request.angle_low + (request.angle_high - request.angle_low) * t
                            : 0.0f;
    // `distRange` is absent from every call site in this installation's
    // usn_19_coralus.lua, so the pair that actually runs is the image default
    // 200.0/2500.0 with the low end clamped to at least 10.0. Taking the low
    // end keeps a script-placed group where the script put it: the scripts
    // compute `refPos` from a named ship and then offset it by thousands of
    // metres themselves, so spreading across the whole 2500 m band would undo
    // the placement the mission author chose.
    const float distance = std::max(kSpawnNewDistRangeLowMinimum, request.dist_low);
    frame.position[0] += std::sin(angle) * distance;
    frame.position[2] += std::cos(angle) * distance;

    // `lookAt` turns the group towards a point; without one the heading is the
    // spawn angle itself.
    if (request.has_look_at) {
        const float dx = request.look_at[0] - frame.position[0];
        const float dz = request.look_at[2] - frame.position[2];
        if (dx != 0.0f || dz != 0.0f) {
            frame.yaw = std::atan2(dx, dz);
            return frame;
        }
    }
    frame.yaw = angle;
    return frame;
}

}  // namespace bsp
