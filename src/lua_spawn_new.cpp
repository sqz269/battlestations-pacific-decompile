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
    // 0094C4E4 FLD [config+2DCh] / 0094C4EA FADD [EDI+0Ch] leaves
    // ST0 = interval + lastAttempt over ST1 = now; 0094C4ED FCOMIP ST0,ST1 and
    // 0094C4F1 JA return when ST0 > ST1. So the drain proceeds on `!(now <
    // interval + last)`, which is the arm written here.
    return !(now < interval + last_attempt_);
}

std::size_t SpawnRequestQueue::select_0094c508(
    const std::vector<bool>& party_active) const noexcept {
    if (requests_.empty()) return 0;
    // 0094C4FA CMP EAX,0x1 / 0094C4FF JBE 0094C56B: one record or none takes the
    // head without testing its party at all.
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

bool spawn_member_offset_00948cc0(const SpawnNewRequest& request, std::size_t member,
                                  float offset[3]) noexcept {
    if (!request.exclude.present || !(request.exclude.formation_horizontal > 0.0f)) {
        return false;
    }
    const std::size_t count = request.members.size();
    if (member >= count) return false;
    const float fh = request.exclude.formation_horizontal;   // record+ACh, >= 0
    // Class extents taken as 0 (labelled in the header), so both maxima are fH.
    const float lateral = static_cast<float>(static_cast<double>(fh) * 2.5 + 5.0);
    const float gap = static_cast<float>(5.0 + static_cast<double>(fh) * 1.5);
    const std::size_t row = member / 3u;
    const std::size_t seat = member % 3u;
    float z = 0.0f;
    for (std::size_t r = 1; r <= row; ++r) z -= gap;   // 00949209 area, param_2 -= G
    const bool has_b = 3u * row + 1u < count;
    const bool has_c = 3u * row + 2u < count;
    float x = 0.0f;
    if (!has_b || has_c) {
        if (seat == 1u) x = -lateral;          // 0.0 - L, [00D7A208] -0.0
        else if (seat == 2u) x = lateral;
    } else {
        const float half = static_cast<float>(static_cast<double>(lateral) * 0.5);
        x = seat == 0u ? -half : half;         // the two-member row
    }
    offset[0] = x;
    offset[1] = 0.0f;
    offset[2] = z;
    return true;
}

SpawnNewFrame spawn_member_frame_0094a140(const SpawnNewRequest& request,
                                          std::size_t member) noexcept {
    if constexpr (kSpawnNewMemberOffsetsBound) {
        float offset[3];
        if (spawn_member_offset_00948cc0(request, member, offset)) {
            // The group frame: 0094A140's first candidate is the mid angle
            // (fVar2 + 0/d) at the low distance (local_358 = record+70h), the
            // same candidate this function's contract gives at t = 0.5. Its
            // axes are taken from the host's yaw toward `lookAt` (SUBSTITUTION,
            // labelled: 0094A140's rotation pair around the reference frame
            // from 008F8680 was not decoded).
            SpawnNewRequest one = request;
            one.members.resize(1);
            SpawnNewFrame frame = spawn_member_frame_0094a140_contract(one, 0);
            const float c = std::cos(frame.yaw);
            const float s = std::sin(frame.yaw);
            // Row vector times the frame: x along row 0 (c, 0, -s), z along
            // row 2 (s, 0, c), as apply_scene_yaw_00467050 lays the rows.
            frame.position[0] += offset[0] * c + offset[2] * s;
            frame.position[1] += offset[1];
            frame.position[2] += -offset[0] * s + offset[2] * c;
            return frame;
        }
    }
    return spawn_member_frame_0094a140_contract(request, member);
}

SpawnNewFrame spawn_member_frame_0094a140_contract(const SpawnNewRequest& request,
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
