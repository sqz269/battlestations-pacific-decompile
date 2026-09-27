#include "bsp/hud_movie_camera.hpp"

#include "bsp/mission_camera.hpp"
#include "bsp/plane_advance_pose.hpp"
#include "bsp/plane_pose_commit.hpp"

#include <algorithm>
#include <cmath>
#include <cctype>

// docs/HUD_PICK_SEGMENT_QUERY.md sections 8.6 and 8.7. Every branch is the
// listing's; the x87 intermediates are kept as float stores where the listing
// stores them (FSTP dword), and in double where it keeps them on the stack.

namespace bsp {
namespace {

using Vec3 = std::array<float, 3>;
using Mat16 = std::array<float, 16>;

constexpr float kOne = 1.0f;                       // 00D7A24C
constexpr double kDuplicateStartStep = 0.0010000000474974513;  // 00D7A318, FADD double
constexpr double kWindowScale = 0.9900000095367432;  // 00CED5D0, FMUL double
constexpr float kFiveSampleFloor = 0.001f;         // 00D7A23C
constexpr double kHalf = 0.5;                      // 00D7A280
constexpr double kTargetReach = 300.0;             // 00CE3CA8, 00791166
constexpr double kDegToRad = 0.01745329238474369;  // 00D046D8, FMUL double
constexpr double kForwardFloorSq = 1e-08;          // 00D7A350

bool iequals(const std::string& a, const char* b) {
    std::size_t i = 0;
    for (; i < a.size() && b[i] != '\0'; ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i]))) return false;
    }
    return i == a.size() && b[i] == '\0';
}

// 004142E0 BSP_Vector3f_TransformAffinePoint: p * rows 0..2 + row 3.
Vec3 transform_point(const Vec3& p, const Mat16& m) {
    Vec3 out;
    for (int i = 0; i < 3; ++i) {
        out[static_cast<std::size_t>(i)] = p[0] * m[static_cast<std::size_t>(i)]
            + p[1] * m[static_cast<std::size_t>(4 + i)]
            + p[2] * m[static_cast<std::size_t>(8 + i)]
            + m[static_cast<std::size_t>(12 + i)];
    }
    return out;
}

// 0078FCF0, RET 4: the quadratic ease-in-out, each product rounded to float.
float ease_0078fcf0(float x) {
    if (0.0f > x) return 0.0f;
    if (x > kOne) return 1.0f;
    if (0.5f > x) {
        const float sq = x * x;           // 0078FD24 FSTP
        return sq + sq;                   // 0078FD2E FSTP
    }
    const float r = 1.0f - x;             // 0078FD45 FSTP
    const float sq = r * r;               // 0078FD4F FSTP
    const float twice = sq + sq;          // 0078FD5B via FSUBP
    return 1.0f - twice;
}

// 00791020, begin(track, t0): state 1, the working copies, and the +D8h
// current-pose seed.
void begin_00791020(MovieKeyframe& k, const HudMovieCamera& camera) {
    k.state_f8 = 1;
    k.work_40 = k.local_34;
    k.work_up_64 = k.up_58;
    k.smooth_d0 = k.smooth_cc;                       // 0079106B
    if (k.fov_divisor_c0 > 0.0f) {                   // 00791064 COMISS 0.0
        k.zoom_c4 = camera.fov_410 / k.fov_divisor_c0 * k.zoom_c4;
    }
    k.zoom_c8 = k.zoom_c4;
    if (!k.use_current_d8) return;
    k.parent_attached_24 = false;                    // 007910D6
    k.transform_2c = 0;                              // 007910DA
    const float* pos = camera.matrix_528.data() + 12;   // +558h
    if (k.camera_28) {
        for (int i = 0; i < 3; ++i) {
            k.local_34[static_cast<std::size_t>(i)] = pos[i];
            k.up_58[static_cast<std::size_t>(i)] = camera.matrix_528[static_cast<std::size_t>(4 + i)];
        }
        k.work_40 = k.local_34;
        k.work_up_64 = k.up_58;
        return;
    }
    // 00791160..007911D5: the look-at point 300 ahead of the camera.
    const float* fwd = camera.matrix_528.data() + 8;    // +548h
    for (int i = 0; i < 3; ++i) {
        const float ahead = static_cast<float>(kTargetReach * fwd[i]);
        k.local_34[static_cast<std::size_t>(i)] = pos[i] + ahead;
    }
    k.work_40 = k.local_34;
}

// 00795B45..00795BAE, the `terrainavoid` clearance every arm but the null
// parent joins: y = ground + 1.0 unless y is already above it. The sum is
// rounded to float at 00795B75 (FSTP [ESP+10h]) before the compare.
void terrain_avoid_00795b45(MovieKeyframe& k, MovieCameraParentHost& host) {
    if (!k.terrain_avoid_d9) return;                   // 00795B45
    float ground = 0.0f;
    if (!host.ground_height_00903860(k.eval_4c, ground)) return;   // 00795B62..00795B69
    const float floor = static_cast<float>(static_cast<double>(ground) + 1.0);   // 00D7A210
    if (k.eval_4c[1] > floor) return;                  // 00795B88 FCOMPI, JBE
    k.eval_4c[1] = floor;                              // 00795BA9
}

// 00795650: the keyframe's world position and up.
void evaluate_position_00795650(MovieKeyframe& k, MovieCameraParentHost& host) {
    if (!k.parent_attached_24) {
        k.eval_4c = k.work_40;
        k.eval_up_70 = k.work_up_64;
        terrain_avoid_00795b45(k, host);               // the unattached arm joins 00795B45
        return;
    }
    if (k.parent_1c == kMovieNoParent) return;       // 00795677
    const std::size_t parent = k.parent_1c;
    Vec3 offset{};
    bool ship = false;
    bool plane = false;
    if (k.wanderer_26) {
        if (host.is_kind(parent, 8)) {
            k.wanderer_26 = false;                    // 00795686
        } else if (host.is_kind(parent, 6)) {
            ship = true;                              // 007956B6
            Mat16 w{};
            host.world(parent, w);
            offset[1] = static_cast<float>(w[13] - host.class_height_a8(parent) * kHalf);
        } else if (host.is_kind(parent, 0x0f)) {
            plane = true;                             // 0079571B 0078FCC0
            offset = host.plane_velocity_810(parent);
        }
    }
    Mat16 m{};
    host.world(parent, m);
    if (host.is_kind(parent, 0x20)) host.unsupported("MovieKeyframe::gun_parent_matrix", 0x007957e8u);
    switch (k.transform_2c) {
    case 0: {                                          // 00795ABD: T + position
        for (int i = 0; i < 3; ++i) {
            k.eval_4c[static_cast<std::size_t>(i)] =
                m[static_cast<std::size_t>(12 + i)] + k.work_40[static_cast<std::size_t>(i)];
        }
        k.eval_up_70 = k.work_up_64;
        break;
    }
    case 2:
        if (!ship) {                                   // 007958FC
            m[4] = kWorldUp00f8758c[0];
            m[5] = kWorldUp00f8758c[1];
            m[6] = kWorldUp00f8758c[2];
            orthonormalize_pose_matrix_0085dc80(m.data());
            k.eval_4c = transform_point(k.work_40, m);
            if (k.then_none_30) {
                host.unsupported("MovieKeyframe::then_none", 0x00795976u);
            } else {
                m[12] = m[13] = m[14] = 0.0f;
                k.eval_up_70 = transform_point(k.work_up_64, m);
            }
            break;
        }
        [[fallthrough]];
    case 1: {                                          // 00795A0C
        AdvanceMatrix a{};
        for (std::size_t i = 0; i < 16; ++i) a.m[i] = m[i];
        a.m[4] = kWorldUp00f8758c[0];
        a.m[5] = kWorldUp00f8758c[1];
        a.m[6] = kWorldUp00f8758c[2];
        orthonormalize_up_first_0085dad0(a);
        for (std::size_t i = 0; i < 16; ++i) m[i] = a.m[i];
        k.eval_4c = transform_point(k.work_40, m);
        k.eval_up_70 = k.work_up_64;
        if (k.then_none_30) host.unsupported("MovieKeyframe::then_none", 0x00795a8eu);
        break;
    }
    case 3:
    default: {                                         // 0079582F
        k.eval_4c = transform_point(k.work_40, m);
        if (k.then_none_30) {
            host.unsupported("MovieKeyframe::then_none", 0x00795866u);
        } else {
            m[12] = m[13] = m[14] = 0.0f;
            k.eval_up_70 = transform_point(k.work_up_64, m);
        }
        break;
    }
    }
    if (plane || ship) {                               // 00795AF5..00795B45
        for (int i = 0; i < 3; ++i) {
            k.eval_4c[static_cast<std::size_t>(i)] -= offset[static_cast<std::size_t>(i)];
        }
    }
    terrain_avoid_00795b45(k, host);
}

// 00795C10, weight(track, dt).
float weight_00795c10(MovieKeyframe& k, const HudMovieCamera& camera, float dt,
                      MovieCameraParentHost& host) {
    const float ts = camera.clock_3b0 + dt;            // 00795C4F FSTP
    if (k.start_f0 > ts) return 0.0f;
    if (k.state_f8 == 0) begin_00791020(k, camera);
    evaluate_position_00795650(k, host);
    const float e = ts - k.start_f0;
    const float b = k.blend_f4;
    if (e >= b) return kOne;                           // 00795CDE FCOMI, JC
    const float x = e / b;                             // 00795D0B FSTP
    float s = ease_0078fcf0(x);
    float nl = k.nonlinear_d4;
    while (nl > kOne) {                                // 00795D27..00795D6B
        nl = static_cast<float>(nl - 1.0);             // 00D7A210, FSUB double 1.0
        s = ease_0078fcf0(s);
    }
    return static_cast<float>((1.0f - nl) * x + nl * s);
}

// 007911E0, step(track): false while the keyframe has not started.
bool step_007911e0(MovieKeyframe& k, const HudMovieCamera& camera) {
    if (k.state_f8 == 0) {
        if (k.start_f0 > camera.clock_3b0) return false;
        begin_00791020(k, camera);
    }
    k.zoom_c8 = k.zoom_c4;                             // 0079126D
    if (k.state_f8 == 1 && camera.clock_3b0 >= k.blend_f4 + k.start_f0) {
        k.state_f8 = 2;                                // 007912E5 (finishscript: none)
    }
    return true;
}

bool is_cut(const MovieKeyframe& k) { return k.has_position_27 && 0.0f >= k.blend_f4; }

// 00798130, evaluate(track).
void evaluate_00798130(MovieTrack& track, const HudMovieCamera& camera,
                       MovieCameraParentHost& host) {
    const float clock = camera.clock_3b0;
    float span = std::min(track.window_78, clock);     // 00798160
    bool first = true;
    const std::size_t count = track.keys.size();
    for (std::size_t n = 0; n < count; ++n) {          // 00798190..00798365
        MovieKeyframe& k = *track.keys[n];
        const bool last = n + 1 == count;
        if (k.state_f8 == 3) continue;
        if (step_007911e0(k, camera)) {
            if (is_cut(k) || first) {
                const float t = static_cast<float>((clock - k.start_f0) * kWindowScale);
                if (!(t > span)) span = t;
            }
            if (last) span = 0.0f;
            first = false;
            continue;
        }
        const float d = static_cast<float>((k.start_f0 - clock) * kWindowScale);
        if (span > d && (is_cut(k) || last)) span = 0.0f > d ? 0.0f : d;
        if (is_cut(k) && k.state_f8 == 0) break;       // 00798363
    }
    const int samples = span > kFiveSampleFloor ? 5 : 1;   // 00798378
    std::array<Vec3, 5> pos{};
    std::array<Vec3, 5> up{};
    std::array<float, 5> zoom{};
    std::array<float, 5> window{};
    std::array<bool, 5> fresh{true, true, true, true, true};
    for (std::size_t n = 0; n < count; ++n) {          // 007983B5..0079893D
        MovieKeyframe& k = *track.keys[n];
        if (is_cut(k) && k.state_f8 == 0) break;       // 00798430
        if (k.state_f8 == 3) continue;
        for (int j = 0; j < samples; ++j) {
            float dt = 0.0f;                           // jump table 00798C10
            if (samples != 1) {
                switch (j) {
                case 0: dt = -0.0f - span; break;
                case 1: dt = static_cast<float>(-span * kHalf); break;
                case 2: dt = 0.0f; break;
                case 3: dt = static_cast<float>(kHalf * span); break;
                default: dt = span; break;
                }
            }
            const float w = weight_00795c10(k, camera, dt, host);
            if (!(w > 0.0f)) continue;
            const std::size_t s = static_cast<std::size_t>(j);
            if (w >= kOne || fresh[s]) {                // 0079882A
                if (k.has_position_27) {
                    pos[s] = k.eval_4c;
                    up[s] = k.eval_up_70;
                }
                zoom[s] = k.zoom_c8;
                window[s] = k.smooth_d0;
            } else {                                    // 00798520
                const float u = 1.0f - w;
                if (k.has_position_27) {
                    for (int i = 0; i < 3; ++i) {
                        const std::size_t c = static_cast<std::size_t>(i);
                        const float a = k.eval_4c[c] * w;
                        const float b = pos[s][c] * u;
                        pos[s][c] = a + b;
                        const float ua = k.eval_up_70[c] * w;
                        const float ub = up[s][c] * u;
                        up[s][c] = ua + ub;
                    }
                }
                zoom[s] = static_cast<float>(static_cast<double>(k.zoom_c8) * w
                    + static_cast<double>(zoom[s]) * u);
                window[s] = static_cast<float>(static_cast<double>(k.smooth_d0) * w
                    + static_cast<double>(window[s]) * u);
            }
            fresh[s] = false;
        }
    }
    if (samples > 1) {                                  // 00798942..00798B81
        for (int pass = 4; pass > 0; --pass) {
            for (int i = 0; i < pass; ++i) {
                const std::size_t a = static_cast<std::size_t>(i);
                for (int c = 0; c < 3; ++c) {
                    const std::size_t cc = static_cast<std::size_t>(c);
                    pos[a][cc] = static_cast<float>((pos[a + 1][cc] + pos[a][cc]) * kHalf);
                    up[a][cc] = static_cast<float>((up[a + 1][cc] + up[a][cc]) * kHalf);
                }
                const float xx = up[a][0] * up[a][0];
                const float yy = up[a][1] * up[a][1];
                const float zz = up[a][2] * up[a][2];
                const float len = static_cast<float>(std::sqrt(
                    static_cast<float>(static_cast<double>(xx) + yy + zz)));
                const float inv = len > 0.0f ? 1.0f / len : 0.0f;
                for (int c = 0; c < 3; ++c) up[a][static_cast<std::size_t>(c)] *= inv;
                zoom[a] = static_cast<float>((zoom[a + 1] + zoom[a]) * kHalf);
                window[a] = static_cast<float>((window[a + 1] + window[a]) * kHalf);
            }
        }
    }
    track.pos_04 = pos[0];
    track.up_10 = up[0];
    track.zoom_60 = zoom[0];
    track.window_78 = window[0];
}

// 007A0770, append(track, keyframe).
void append_007a0770(MovieTrack& track, std::unique_ptr<MovieKeyframe> k, float clock) {
    k->state_f8 = 0;
    k->start_f0 = clock + k->start_f0;
    const std::size_t size = track.keys.size();
    if (size <= 1) {
        if (size == 0) {
            k->start_f0 = 0.0f;
            k->blend_f4 = 0.0f;
        }
    } else if (track.keys.back()->start_f0 == k->start_f0) {
        const float moved = static_cast<float>(k->start_f0 + kDuplicateStartStep);
        k->start_f0 = moved;
        if (0.0f > moved) k->start_f0 = 0.0f;
    }
    track.keys.push_back(std::move(k));
}

// 00799D70, the parent: none when destroyed, a squadron's +3D0h leader.
std::size_t resolve_parent_00799d70(std::size_t entity, MovieCameraParentHost& host) {
    if (entity == kMovieNoParent) return kMovieNoParent;
    if (host.destroyed_5e(entity)) return kMovieNoParent;
    if (host.is_kind(entity, 0x18)) return host.squadron_leader_3d0(entity);
    return entity;
}

// 007A0EB0, parse(keyframe, camera).
void parse_007a0eb0(MovieKeyframe& k, HudMovieCamera& camera, const MovieKeyframeInput& in,
                    MovieCameraParentHost& host) {
    const bool named = iequals(in.postype, k.camera_28 ? "camera" : "target");
    MovieTrack& track = k.camera_28 ? camera.camera_498 : camera.target_414;
    if (!track.keys.empty()) {                          // 007A0FFF / 007A1034
        const MovieKeyframe& last = *track.keys.back();
        k.smooth_cc = last.smooth_cc;
        k.zoom_c4 = last.zoom_c4;
        k.wanderer_26 = last.wanderer_26;
        k.transform_2c = last.transform_2c;
        k.camera_28 = last.camera_28;
    } else {
        k.wanderer_26 = k.camera_28;
        k.transform_2c = 1;
        k.zoom_c4 = kOne;
    }
    if (in.transformtype) {                              // 007A1151
        const std::string& t = *in.transformtype;
        if (iequals(t, "keepall")) { k.then_none_30 = false; k.transform_2c = 3; }
        else if (iequals(t, "keepz")) { k.transform_2c = 2; k.then_none_30 = false; }
        else if (iequals(t, "keepy")) { k.transform_2c = 1; k.then_none_30 = false; }
        else if (iequals(t, "keepnone")) { k.transform_2c = 0; k.then_none_30 = false; }
        else if (iequals(t, "keepz_thennone")) { k.transform_2c = 2; k.then_none_30 = true; }
        else if (iequals(t, "keepy_thennone")) { k.transform_2c = 1; k.then_none_30 = true; }
        else if (iequals(t, "keepall_thennone")) { k.then_none_30 = true; k.transform_2c = 3; }
    }
    if (in.wanderer) k.wanderer_26 = *in.wanderer;
    bool nothing_placed = true;
    if (!in.has_position) {
        k.has_position_27 = false;
    } else {
        k.has_position_27 = true;
        k.parent_attached_24 = false;
        k.parent_1c = kMovieNoParent;
        k.local_34 = {};
        std::size_t entity = kMovieNoParent;
        if (in.has_parent && in.parent != kMovieNoParent) {
            entity = in.parent;
            nothing_placed = false;
            k.parent_1c = resolve_parent_00799d70(entity, host);
            k.parent_attached_24 = k.parent_1c != kMovieNoParent;
        }
        if (in.terrainavoid) k.terrain_avoid_d9 = *in.terrainavoid;   // 007A1488
        k.local_34 = {};                                // 007A1500: no integer `deckpos`
        if (k.camera_28 || named) {
            if (in.pos) {
                k.local_34 = *in.pos;
                nothing_placed = false;
            }
            if (in.polar) {                              // 007A16C0..007A17A8
                const float d = (*in.polar)[0];
                const float theta = static_cast<float>((*in.polar)[1] * kDegToRad);
                const float rho = static_cast<float>((*in.polar)[2] * kDegToRad);
                const float sr = static_cast<float>(std::sin(rho));
                const float ct = static_cast<float>(std::cos(theta));
                k.local_34[0] = k.local_34[0] - sr * d * ct;
                const float st = static_cast<float>(std::sin(theta));
                k.local_34[1] = st * d + k.local_34[1];
                const float cr = static_cast<float>(std::cos(rho));
                k.local_34[2] = cr * d * ct + k.local_34[2];
                nothing_placed = false;
            }
        }
        if (entity != kMovieNoParent && k.parent_1c == kMovieNoParent) {
            // 007A1FC0..007A2012: the entity's own world position, added once.
            Mat16 w{};
            host.world(entity, w);
            for (int i = 0; i < 3; ++i) {
                k.local_34[static_cast<std::size_t>(i)] += w[static_cast<std::size_t>(12 + i)];
            }
        }
    }
    if (k.has_position_27 && nothing_placed) k.use_current_d8 = true;   // 007A2034
    if (in.starttime) k.start_f0 = *in.starttime;
    if (in.blendtime) k.blend_f4 = *in.blendtime;
    if (in.linearblend) {
        const float v = static_cast<float>(1.0 - *in.linearblend);
        k.nonlinear_d4 = 0.0f <= v ? (v > kOne ? kOne : v) : 0.0f;
    }
    if (in.nonlinearblend) {
        const float v = *in.nonlinearblend;
        k.nonlinear_d4 = 0.0f <= v ? (v > kOne ? kOne : v) : 0.0f;
    }
    if (in.zoom) k.zoom_c4 = *in.zoom;
    if (in.smoothtime) k.smooth_cc = *in.smoothtime;
    for (const std::string& key : in.unsupported_keys) {
        static_cast<void>(key);
        host.unsupported("MovieKeyframe::unsupported_key", 0x007a0eb0u);
    }
}

}  // namespace

void movie_camera_construct_0079d020(HudMovieCamera& camera) noexcept {
    camera = HudMovieCamera{};
}

void movie_camera_seed_007a0860(HudMovieCamera& camera, const std::array<float, 16>& world) {
    camera.matrix_528 = world;                          // 007A0886 004134F0
    if (!camera.camera_498.keys.empty()) return;        // 007A0895
    auto make = [](bool is_camera) {
        auto k = std::make_unique<MovieKeyframe>();
        k->camera_28 = is_camera;
        k->parent_attached_24 = false;
        k->use_current_d8 = true;
        k->blend_f4 = 0.0f;
        k->start_f0 = 0.0f;
        return k;
    };
    append_007a0770(camera.camera_498, make(true), camera.clock_3b0);
    append_007a0770(camera.target_414, make(false), camera.clock_3b0);
}

int movie_camera_add_position_007a42c0(HudMovieCamera& camera, const MovieKeyframeInput& in,
                                       MovieCameraParentHost& host) {
    bool build_camera = false;
    bool build_target = false;
    if (iequals(in.postype, "camera")) {
        build_camera = true;
    } else if (iequals(in.postype, "target")) {
        build_target = true;
    } else if (iequals(in.postype, "cameraandtarget")) {
        build_camera = true;
        build_target = true;
    } else {
        return 0;                                        // 007A44A5
    }
    int added = 0;
    bool built_camera = false;
    if (build_camera) {                                  // 007A4389
        auto k = std::make_unique<MovieKeyframe>();
        k->camera_28 = true;
        parse_007a0eb0(*k, camera, in, host);
        if (k->camera_28 && k->use_current_d8) build_target = true;
        append_007a0770(camera.camera_498, std::move(k), camera.clock_3b0);
        built_camera = true;
        ++added;
    }
    if (build_target) {
        auto k = std::make_unique<MovieKeyframe>();
        k->camera_28 = false;
        static_cast<void>(built_camera);                 // the +20h/+DAh pairing is ids only
        parse_007a0eb0(*k, camera, in, host);
        append_007a0770(camera.target_414, std::move(k), camera.clock_3b0);
        ++added;
    }
    return added;
}

bool movie_camera_fixed_step_00798c80(HudMovieCamera& camera, float dt) noexcept {
    ++camera.fixed_steps;
    if (camera.running_391) return false;
    if (!(dt > 0.0f)) return false;                      // 00798C93 COMISS 0.0
    camera.running_391 = true;                           // 00798CA6
    return true;
}

void movie_camera_store_step_draw_00798cd9(HudMovieCamera& camera, float dt,
                                           float draw) noexcept {
    ++camera.steps_408;                                  // 00798CDF
    camera.step_seconds_40c = camera.step_seconds_40c + dt;   // 00798CE6..00798CF2
    // 00798D0C 00BF7420: the float-to-integer conversion of the draw, taken as
    // truncation (unverified; the value has no reader).
    camera.draw_ring_3c4[static_cast<std::size_t>(camera.draw_index_404 & 15)] =
        static_cast<std::int32_t>(draw);                 // 00798D17
    camera.draw_index_404 += 1;                          // 00798D1E
    if (camera.draw_index_404 > 15) camera.draw_index_404 = 0;   // 00798D25..00798D2E
}

bool movie_camera_update_0079a3b0(HudMovieCamera& camera, MovieCameraParentHost& host,
                                  float dt, std::array<float, 16>& world, float& fov) {
    if (camera.running_391) {                            // 0079A3C3
        camera.clock_3b0 = dt + camera.clock_3b0;
    } else {
        camera.clock_3b0 = 0.0f;
    }
    if (camera.camera_498.keys.empty() || camera.target_414.keys.empty()) return false;
    evaluate_00798130(camera.target_414, camera, host);  // 0079B2A8
    camera.look_384 = camera.target_414.pos_04;
    evaluate_00798130(camera.camera_498, camera, host);  // 0079B2D7
    const Vec3 target = camera.target_414.pos_04;
    Vec3 pos = camera.camera_498.pos_04;
    const Vec3 up = camera.camera_498.up_10;
    const float zoom = camera.camera_498.zoom_60;        // +4F8h
    // 0079B634..0079B6A8: +4FCh clear, so 0078FAF0 runs on the zero state.
    if (dt > 0.0f) {
        const float v = camera.ground_51c[0] + camera.ground_51c[0] + camera.ground_51c[2];
        camera.ground_51c[1] = v > 0.0f ? static_cast<float>(-v * 0.6000000238418579) : -0.0f - v;
    }
    pos[1] = camera.ground_51c[2] + pos[1];
    Vec3 f{};
    for (int i = 0; i < 3; ++i) {
        f[static_cast<std::size_t>(i)] = target[static_cast<std::size_t>(i)] - pos[static_cast<std::size_t>(i)];
    }
    const float len2 = static_cast<float>(static_cast<double>(f[0]) * f[0]
        + static_cast<double>(f[1]) * f[1] + static_cast<double>(f[2]) * f[2]);
    if (kForwardFloorSq > len2) {                        // 0079B71A
        f = {0.0f, 0.0f, 1.0f};
    } else {                                             // 0079B73B 00419510
        const float inv = 1.0f / std::sqrt(len2);
        for (float& c : f) c *= inv;
    }
    for (int i = 0; i < 3; ++i) {                         // 0079B794..0079B7D7
        camera.matrix_528[static_cast<std::size_t>(8 + i)] = f[static_cast<std::size_t>(i)];
        camera.matrix_528[static_cast<std::size_t>(4 + i)] = up[static_cast<std::size_t>(i)];
    }
    orthonormalize_pose_matrix_0085dc80(camera.matrix_528.data());   // 0079B7DF
    for (int i = 0; i < 3; ++i) {
        camera.matrix_528[static_cast<std::size_t>(12 + i)] = pos[static_cast<std::size_t>(i)];
    }
    camera.matrix_528[15] = 1.0f;
    fov = camera.fov_410 / zoom;                          // 0079B815
    camera.local_74 = camera.matrix_528;                  // 0079B83C
    world = camera.local_74;                              // 00435410 -> 004329D0
    return true;
}

}  // namespace bsp
