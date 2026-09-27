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
    const std::size_t row = member / 3u;
    const std::size_t seat = member % 3u;
    // The class extents enter only under kSpawnNewPlacementBound, so section
    // 22's measured state is what the OFF side keeps.
    auto length = [&](std::size_t i) {
        return kSpawnNewPlacementBound && request.members[i].has_class_extents
            ? request.members[i].class_length_a0 : 0.0f;
    };
    auto width = [&](std::size_t i) {
        return kSpawnNewPlacementBound && request.members[i].has_class_extents
            ? request.members[i].class_width_a4 : 0.0f;
    };
    auto half = [](float a, float b) {
        return static_cast<float>((static_cast<double>(a) + b) * 0.5);   // [00D7A280]
    };
    auto row_lateral = [&](std::size_t r) {
        const std::size_t a = 3u * r;
        float l = fh;                                          // local_50 = max(0, +ACh)
        if (a + 1u < count) l = std::max(l, half(width(a + 1u), width(a)));
        if (a + 2u < count) l = std::max(l, half(width(a + 2u), width(a)));
        return static_cast<float>(static_cast<double>(l) * 2.5 + 5.0);   // [00CE3DE0] [00D7A370]
    };
    auto row_gap = [&](std::size_t r) {
        const std::size_t a = 3u * r;
        float g = fh;                                          // local_4c = max(0, +ACh)
        g = std::max(g, half(length(a - 3u), length(a)));
        if (a + 1u < count) g = std::max(g, half(length(a - 2u), length(a + 1u)));
        if (a + 2u < count) g = std::max(g, half(length(a - 1u), length(a + 2u)));
        return static_cast<float>(5.0 + static_cast<double>(g) * 1.5);   // [00D7A370] [00CE3D78]
    };
    const float lateral = row_lateral(row);
    float z = 0.0f;
    for (std::size_t r = 1; r <= row; ++r) z -= row_gap(r);   // param_2 -= G per row
    const bool has_b = 3u * row + 1u < count;
    const bool has_c = 3u * row + 2u < count;
    float x = 0.0f;
    if (!has_b || has_c) {
        if (seat == 1u) x = -lateral;          // 0.0 - L, [00D7A208] -0.0
        else if (seat == 2u) x = lateral;
    } else {
        const float half_l = static_cast<float>(static_cast<double>(lateral) * 0.5);
        x = seat == 0u ? -half_l : half_l;        // the two-member row
    }
    offset[0] = x;
    offset[1] = 0.0f;
    offset[2] = z;
    return true;
}

namespace {
const SpawnPlacementWorld* g_spawn_placement_world = nullptr;

void normalize_row(float* r) noexcept {
    // 0094A140's per-row normalise: 00419440 length, reciprocal 0 at length <= 0.
    const double n = std::sqrt(static_cast<double>(r[0]) * r[0] +
                               static_cast<double>(r[1]) * r[1] +
                               static_cast<double>(r[2]) * r[2]);
    const float inv = n > 0.0 ? static_cast<float>(1.0 / n) : 0.0f;
    for (int i = 0; i < 3; ++i) r[i] *= inv;
}

// One solved request, so every member of a request is placed on the same
// frame and the members created first do not change the later members' test.
struct SolvedPlacement {
    std::uint32_t serial{0};
    unsigned attempts{0};
    bool valid{false};
    SpawnPlacementResult result{};
};
SolvedPlacement g_solved;
}  // namespace

const SpawnPlacementResult* last_spawn_placement_0094a140() noexcept {
    return g_solved.valid ? &g_solved.result : nullptr;
}

void set_spawn_placement_world(const SpawnPlacementWorld* world) noexcept {
    g_spawn_placement_world = world;
}

SpawnGroupFrame spawn_reference_frame_0094a140(const SpawnNewRequest& request) noexcept {
    SpawnGroupFrame f;
    f.m[12] = request.ref_pos[0];
    f.m[13] = request.ref_pos[1];
    f.m[14] = request.ref_pos[2];
    if (request.has_look_at) {
        // 00949E96..00949EE0: row 2 = lookAt - refPos, unnormalised.
        f.m[8] = request.look_at[0] - request.ref_pos[0];
        f.m[9] = request.look_at[1] - request.ref_pos[1];
        f.m[10] = request.look_at[2] - request.ref_pos[2];
        // 0085DC80: row 1 orthogonalised against row 2, row 0 = row1 x row2.
        float fwd[3] = {f.m[8], f.m[9], f.m[10]};
        normalize_row(fwd);
        float* up = f.m + 4;
        const float d = up[0] * fwd[0] + up[1] * fwd[1] + up[2] * fwd[2];
        if (std::fabs(d) < 0.9990000128746033f) {
            for (int i = 0; i < 3; ++i) up[i] -= d * fwd[i];
            normalize_row(up);
            f.m[0] = up[1] * fwd[2] - up[2] * fwd[1];
            f.m[1] = up[2] * fwd[0] - up[0] * fwd[2];
            f.m[2] = up[0] * fwd[1] - up[1] * fwd[0];
            normalize_row(f.m);
        }
        // The near-vertical arm (|up . fwd| >= 0.999, 0085DD26) is not
        // reproduced: a lookAt straight below refPos keeps the identity rows 0/1.
    }
    normalize_row(f.m);
    normalize_row(f.m + 4);
    normalize_row(f.m + 8);
    return f;
}

SpawnGroupFrame spawn_candidate_frame_0094a140(const SpawnGroupFrame& r, float angle,
                                               float distance) noexcept {
    SpawnGroupFrame c = r;
    const double s = std::sin(static_cast<double>(angle));
    const double co = std::cos(static_cast<double>(angle));
    // (0, 0, d) . RotY(-a) = d * (sin(-a), 0, cos(-a)), then into R.
    const double lx = -s * distance;
    const double lz = co * distance;
    for (int i = 0; i < 3; ++i) {
        c.m[12 + i] = static_cast<float>(r.m[12 + i] + lx * r.m[i] + lz * r.m[8 + i]);
    }
    return c;
}

void spawn_member_position_00949300(const SpawnGroupFrame& f, const float o[3],
                                    float out[3]) noexcept {
    for (int i = 0; i < 3; ++i) {
        out[i] = static_cast<float>(static_cast<double>(o[0]) * f.m[i] +
                                    static_cast<double>(o[1]) * f.m[4 + i] +
                                    static_cast<double>(o[2]) * f.m[8 + i] + f.m[12 + i]);
    }
}

bool spawn_member_placement_legal_00941d30(const float p[3], std::int32_t party,
                                           const SpawnNewExcludeRadius& ex,
                                           const std::vector<SpawnPlacementEntity>& entities,
                                           bool outside_map) noexcept {
    if (outside_map) return false;                                 // 00941D48
    const float own_h2 = ex.own_horizontal * ex.own_horizontal;     // block+14h
    const float enemy_h2 = ex.enemy_horizontal * ex.enemy_horizontal;   // block+18h
    for (const SpawnPlacementEntity& e : entities) {
        const float dy = p[1] - e.position[1];
        float limit2;
        if (e.party == party) {
            if (std::fabs(dy) > ex.own_vertical) continue;          // block+8h
            limit2 = own_h2;
        } else {
            if (!(std::fabs(dy) <= ex.enemy_vertical)) continue;    // block+Ch
            limit2 = enemy_h2;
        }
        const float dx = e.position[0] - p[0];
        const float dz = e.position[2] - p[2];
        const float ey = e.position[1] - p[1];
        if (dz * dz + ey * ey + dx * dx < limit2) return false;
    }
    return true;
}

SpawnPlacementResult solve_spawn_placement_0094a140(const SpawnNewRequest& request) noexcept {
    SpawnPlacementResult out;
    const SpawnGroupFrame ref = spawn_reference_frame_0094a140(request);
    const float lo = request.has_angle_range ? request.angle_low : 0.0f;
    const float hi = request.has_angle_range ? request.angle_high : 0.0f;
    const float mid = static_cast<float>((static_cast<double>(hi) + lo) * 0.5);
    float halfwidth = hi - lo;
    if (halfwidth <= 0.0f) halfwidth = -0.0f - halfwidth;           // [00D7A208]
    halfwidth = static_cast<float>(0.5 * halfwidth);
    std::vector<SpawnPlacementEntity> entities;
    if (g_spawn_placement_world != nullptr) g_spawn_placement_world->placement_entities(entities);
    auto legal = [&](const SpawnGroupFrame& frame) {
        // SUBSTITUTION, labelled: with no world registered every candidate
        // passes, which is 0094A140's first candidate.
        if (g_spawn_placement_world == nullptr) return true;
        for (std::size_t i = 0; i < request.members.size(); ++i) {
            float off[3] = {0.0f, 0.0f, 0.0f};
            if (!spawn_member_offset_00948cc0(request, i, off)) {
                off[0] = off[1] = off[2] = 0.0f;
            }
            float pos[3];
            spawn_member_position_00949300(frame, off, pos);
            if (!spawn_member_placement_legal_00941d30(pos, request.party, request.exclude,
                    entities, g_spawn_placement_world->point_outside_map_0071c4f0(pos))) {
                return false;
            }
        }
        return true;
    };
    out.entities = static_cast<int>(entities.size());
    auto nearest = [&](const SpawnGroupFrame& frame) {
        double best = -1.0;
        for (std::size_t i = 0; i < request.members.size(); ++i) {
            float off[3] = {0.0f, 0.0f, 0.0f};
            if (!spawn_member_offset_00948cc0(request, i, off)) off[0] = off[1] = off[2] = 0.0f;
            float pos[3];
            spawn_member_position_00949300(frame, off, pos);
            for (const SpawnPlacementEntity& e : entities) {
                const double dx = e.position[0] - pos[0];
                const double dy = e.position[1] - pos[1];
                const double dz = e.position[2] - pos[2];
                const double d = std::sqrt(dx * dx + dy * dy + dz * dz);
                if (best < 0.0 || d < best) best = d;
            }
        }
        return static_cast<float>(best);
    };
    const float dist_low = std::max(kSpawnNewDistRangeLowMinimum, request.dist_low);
    for (float d = dist_low; d <= request.dist_high; d += 250.0f) {      // [00CF8850]
        const float arc_max = d * halfwidth;
        if (!(0.0f <= arc_max)) continue;
        for (float arc = 0.0f; arc <= arc_max; arc += 250.0f) {
            const float plus = static_cast<float>(mid + arc / d);
            const float minus = static_cast<float>(mid - arc / d);
            SpawnGroupFrame f = spawn_candidate_frame_0094a140(ref, plus, d);
            ++out.candidates;
            if (legal(f)) {
                out.accepted = true; out.frame = f; out.angle = plus; out.distance = d; out.nearest = nearest(f);
                return out;
            }
            if (arc > 0.0f) {                                         // [00D7A218] < arc
                f = spawn_candidate_frame_0094a140(ref, minus, d);
                ++out.candidates;
                if (legal(f)) {
                    out.accepted = true; out.frame = f; out.angle = minus; out.distance = d; out.nearest = nearest(f);
                    return out;
                }
            }
        }
    }
    return out;
}

SpawnNewFrame spawn_member_frame_0094a140(const SpawnNewRequest& request,
                                          std::size_t member) noexcept {
    if constexpr (kSpawnNewPlacementBound) {
        if (request.exclude.present) {
            if (!(g_solved.valid && g_solved.serial == request.serial &&
                  g_solved.attempts == request.attempts)) {
                g_solved.serial = request.serial;
                g_solved.attempts = request.attempts;
                g_solved.result = solve_spawn_placement_0094a140(request);
                g_solved.valid = true;
            }
            SpawnNewFrame frame;
            frame.refused = !g_solved.result.accepted;
            const SpawnGroupFrame& f = g_solved.result.frame;
            float off[3] = {0.0f, 0.0f, 0.0f};
            if (!spawn_member_offset_00948cc0(request, member, off)) {
                off[0] = off[1] = off[2] = 0.0f;
            }
            spawn_member_position_00949300(f, off, frame.position);
            // The host builds the entity from a heading only (SpawnNewFrame);
            // SUBSTITUTION, labelled: the frame's pitch (lookAt below refPos) is
            // not carried into the created entity, only row 2's heading.
            frame.yaw = static_cast<float>(std::atan2(static_cast<double>(f.m[8]),
                                                      static_cast<double>(f.m[10])));
            return frame;
        }
    }
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
