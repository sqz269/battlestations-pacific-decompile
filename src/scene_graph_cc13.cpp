#include "bsp/scene_graph_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>

// Segments 88/89 routines. Evidence and ABIs: docs/CC13_SCENE_GRAPH_NODES.md.
// x87 expressions are evaluated in double and rounded to float wherever the native code
// stores a float, which approximates the extended-precision intermediates.

namespace bsp::cc13_scene {
namespace {

float f(double v) noexcept { return static_cast<float>(v); }

// _ftol2 truncation.
std::int32_t ftol(double v) noexcept { return static_cast<std::int32_t>(v); }

// The sampler's segment interpolation for s in (0,1) between keys[i] and keys[i+1].
float interpolate_segment(const AnimTrack& track, std::int32_t i, float time, float offset) {
    const std::int32_t n = static_cast<std::int32_t>(track.keys.size());
    const AnimKey& key = track.keys[i];
    const AnimKey& next = track.keys[i + 1];
    const AnimKey* prev = i > 0 ? &track.keys[i - 1] : nullptr;
    const AnimKey* after = i + 2 < n ? &track.keys[i + 2] : nullptr;
    if (time == key.time) return f(key.value + offset);
    if (time == next.time) return f(next.value + offset);
    const float s = f((time - key.time) / (static_cast<double>(next.time) - key.time));
    switch (next.type) {
    case 0:
    case 1:
    case 2: {
        const HermiteBasis h = hermite_basis_00b75520(s);
        const float out = outgoing_tangent_00b75820(key, prev, next);
        const double acc = static_cast<double>(key.value) * h.p0 + static_cast<double>(next.value) * h.p1
            + static_cast<double>(out) * h.t0;  // stored as a double at 00B770C2
        const float in = incoming_tangent_00b75a20(next, after, key);
        return f(static_cast<double>(in) * h.t1 + acc + offset);
    }
    case 3: return f(key.value + static_cast<double>(s) * (static_cast<double>(next.value) - key.value) + offset);
    case 4: return f(key.value + offset);
    case 5: return f(static_cast<double>(bezier_segment_00b75720(key, next, time)) + offset);
    default: return offset;  // 00B7715F returns the offset unrounded
    }
}

}  // namespace

float wrap_time_00b75450(float time, float start, float end, std::int32_t* cycles) {
    const float length = f(static_cast<double>(end) - start);
    if (length == 0.0f) {  // the unordered (NaN) case takes the computing path
        if (cycles) *cycles = 0;
        return start;
    }
    const double shifted = static_cast<double>(start) + time;  // kept as a double
    // 00BF85B0 is the CRT floor: its x87 fallback 00C0F394 loads control word [00E16460] = 173Fh
    // (RC = round down) through _ctrlfp 00C138A7, rounds with __frnd 00C28548 (FRNDINT) and
    // reports errors as opcode 0Bh (OP_FLOOR).
    const double turns = std::floor(static_cast<double>(time) / length);
    const float wrapped = f(shifted - turns * length);
    if (cycles) {
        const double half = wrapped > time ? 0.5 : -0.5;
        *cycles = -ftol((static_cast<double>(wrapped) - time) / length + half);
    }
    return wrapped;
}

// cc13-fixture2 follow-up: 00B75520 and 00B75580 serve the animation-track sampler (00B76CE0,
// 00B75610, 00B75720) on the main thread, so each operation rounds to 24 significand bits
// (x87_*24) in listing order: 00B75538..4A (3*s2 - s3) - s3, 00B7555C 1 - p1, 00B75562 s3 - s2,
// 00B7556E..70 s + (t1 - s2).
HermiteBasis hermite_basis_00b75520(float s) {
    using namespace cc13x87;
    const float s2 = f(x87_mul24(s, s));
    const float s3 = f(x87_mul24(s2, s));
    HermiteBasis h{};
    h.p1 = f(x87_sub24(x87_sub24(x87_mul24(3.0, s2), s3), s3));
    h.p0 = f(x87_sub24(1.0, h.p1));
    h.t1 = f(x87_sub24(s3, s2));
    h.t0 = f(x87_add24(s, x87_sub24(h.t1, s2)));
    return h;
}

float outgoing_tangent_00b75820(const AnimKey& key, const AnimKey* prev, const AnimKey& next) {
    const double forward = static_cast<double>(next.value) - key.value;
    switch (key.type) {
    case 0: {  // TCB
        const float a = f((1.0 - key.continuity) * (1.0 - key.tension) * (1.0 - key.bias));
        const float d = f(forward);
        if (!prev) return f(static_cast<double>(d) * a);
        const float b = f((1.0 + key.bias) * ((1.0 - key.tension) * (key.continuity + 1.0)));
        const float scale = f((static_cast<double>(next.time) - key.time) / (static_cast<double>(next.time) - prev->time));
        return f(((static_cast<double>(key.value) - prev->value) * b + static_cast<double>(d) * a) * scale);
    }
    case 1:
    case 2:
        if (!prev) return key.in_b;
        return f((static_cast<double>(next.time) - key.time) / (static_cast<double>(next.time) - prev->time) * key.in_b);
    case 3: {
        const float d = f(forward);
        if (!prev) return d;
        const float scale = f((static_cast<double>(next.time) - key.time) / (static_cast<double>(next.time) - prev->time));
        return f(((static_cast<double>(key.value) - prev->value) + d) * scale);
    }
    case 5: {
        const float slope = f((static_cast<double>(next.time) - key.time) * key.out_b);
        if (std::fabs(key.out_a) > 9.99999975e-06f) return f(static_cast<double>(slope) / key.out_a);
        return f(static_cast<double>(slope) * 100000.0);
    }
    default: return 0.0f;
    }
}

float incoming_tangent_00b75a20(const AnimKey& key, const AnimKey* after, const AnimKey& before) {
    const double backward = static_cast<double>(key.value) - before.value;
    switch (key.type) {
    case 0: {
        const float a = f((1.0 - key.continuity) * (1.0 - key.tension) * (key.bias + 1.0));
        const float d = f(backward);
        if (!after) return f(static_cast<double>(a) * d);
        const float b = f((1.0 - key.bias) * ((1.0 - key.tension) * (key.continuity + 1.0)));
        const float scale = f((static_cast<double>(key.time) - before.time) / (static_cast<double>(after->time) - before.time));
        return f(((static_cast<double>(after->value) - key.value) * b + static_cast<double>(a) * d) * scale);
    }
    case 1:
    case 2:
        if (!after) return key.in_a;
        return f((static_cast<double>(key.time) - before.time) / (static_cast<double>(after->time) - before.time) * key.in_a);
    case 3: {
        const float d = f(backward);
        if (!after) return d;
        const float scale = f((static_cast<double>(key.time) - before.time) / (static_cast<double>(after->time) - before.time));
        return f(((static_cast<double>(after->value) - key.value) + d) * scale);
    }
    case 5: {
        const float slope = f((static_cast<double>(key.time) - before.time) * key.in_b);
        if (std::fabs(key.in_a) > 9.99999975e-06f) return f(static_cast<double>(slope) / key.in_a);
        return f(static_cast<double>(slope) * 100000.0);
    }
    default: return 0.0f;
    }
}

std::int32_t find_key_00b75c10(const AnimKey* keys, std::int32_t count, float time) {
    std::int32_t lo = 0;
    std::int32_t hi = count - 1;
    while (lo < hi - 1) {
        const float t_lo = keys[lo].time;
        const std::int32_t c = ftol((static_cast<double>(time) - t_lo) / (static_cast<double>(keys[hi].time) - t_lo)
            * (hi - lo) + lo);
        if (time > keys[c].time) {
            if (keys[c + 1].time >= time) return c;
            lo = c + 1;
        } else {
            hi = c;
        }
        const std::int32_t mid = (lo + hi) >> 1;
        if (time > keys[mid].time) lo = mid; else hi = mid;
    }
    return lo;
}

// 00B75580 in listing order: c = (p1 - p0)*3 (00B75598..A0), b = 3*(p2 - p1) - c (00B755AC..B8),
// a = ((p3 - p0) - c) - b (00B755C4..CE), then FMULP/FADDP on the stack at 00B755DA..F5:
// (s*c + (b*s2 + a*s3)) + p0, each operation rounded to 24 bits (see 00B75520 above).
double bezier_00b75580(float p0, float p1, float p2, float p3, float s) {
    using namespace cc13x87;
    const float s2 = f(x87_mul24(s, s));
    const float c = f(x87_mul24(x87_sub24(p1, p0), 3.0));
    const float b = f(x87_sub24(x87_mul24(3.0, x87_sub24(p2, p1)), c));
    const float a = f(x87_sub24(x87_sub24(x87_sub24(p3, p0), c), b));
    const float s3 = f(x87_mul24(s2, s));
    // cc13-fixture2 (fixture-found): 00B755F7 FSTP DWORD / FLD rounds the result to float.
    return f(x87_add24(x87_add24(x87_mul24(s, c), x87_add24(x87_mul24(b, s2), x87_mul24(a, s3))), p0));
}

float bezier_solve_00b75610(float& lo, float& hi, float p0, float p1, float p2, float p3,
    float target, int max_iterations) {
    for (int i = 0;; ++i) {
        const float mid = f((static_cast<double>(lo) + hi) * 0.5);
        const float x = f(bezier_00b75580(p0, p1, p2, p3, mid));
        const float error = std::fabs(f(static_cast<double>(target) - x));
        if (!(error > 9.99999975e-05) || i >= max_iterations) return mid;
        if (x > target) hi = mid; else lo = mid;
    }
}

float bezier_segment_00b75720(const AnimKey& key, const AnimKey& next, float time) {
    float lo = 0.0f;
    float hi = 1.0f;
    const float handle1_t = key.type == 5 ? f(static_cast<double>(key.out_a) + key.time)
                                          : f(key.time + (static_cast<double>(next.time) - key.time) / 3.0);
    const float handle2_t = f(static_cast<double>(next.time) + next.in_a);
    const float s = bezier_solve_00b75610(lo, hi, key.time, handle1_t, handle2_t, next.time, time);
    // A non-type-5 key uses a third of its out tangent; the next key's in-handle is read
    // from +18h/+1Ch whatever its type.
    const float handle1_v = f((key.type == 5 ? static_cast<double>(key.out_b) : key.in_b / 3.0) + key.value);
    const float handle2_v = f(static_cast<double>(next.value) + next.in_b);
    return f(bezier_00b75580(key.value, handle1_v, handle2_v, next.value, s));
}

float sample_track_00b76ce0(const AnimTrack& track, float time) {
    const std::int32_t n = static_cast<std::int32_t>(track.keys.size());
    if (n == 0) return 0.0f;
    if (n == 1) return track.keys[0].value;
    const AnimKey& first = track.keys[0];
    const AnimKey& last = track.keys[n - 1];
    float offset = 0.0f;
    std::int32_t cycles = 0;
    auto wrap = [&](bool count, bool reflect, bool relative) {
        const float wrapped = wrap_time_00b75450(time, first.time, last.time, count ? &cycles : nullptr);
        time = wrapped;
        if (reflect && (cycles % 2) != 0) time = f((static_cast<double>(last.time) - first.time) - wrapped);
        if (relative) offset = f((static_cast<double>(last.value) - first.value) * cycles);
    };
    OutOfRange mode{};
    bool outside = false;
    bool before = false;
    if (time < first.time) {
        outside = true;
        before = true;
        mode = track.before;
    } else if (time > last.time) {
        outside = true;
        mode = track.after;
    }
    if (outside) {
        switch (mode) {
        case OutOfRange::zero: return 0.0f;
        case OutOfRange::hold: return before ? first.value : last.value;
        case OutOfRange::cycle: wrap(false, false, false); break;
        case OutOfRange::ping_pong: wrap(true, true, false); break;
        case OutOfRange::relative_cycle: wrap(true, false, true); break;
        case OutOfRange::linear:
            if (before) {
                const float tangent = outgoing_tangent_00b75820(first, nullptr, track.keys[1]);
                const float slope = f(tangent / (static_cast<double>(track.keys[1].time) - first.time));
                return f((static_cast<double>(time) - first.time) * slope + first.value);
            } else {
                const AnimKey& prior = track.keys[n - 2];
                const float tangent = incoming_tangent_00b75a20(last, nullptr, prior);
                const float slope = f(tangent / (static_cast<double>(last.time) - prior.time));
                return f((static_cast<double>(time) - last.time) * slope + last.value);
            }
        default: break;  // other values sample the clamped search unchanged
        }
    }
    const std::int32_t i = find_key_00b75c10(track.keys.data(), n, time);
    if (i + 1 >= n) return f(track.keys[i].value + offset);  // guard; native reads past the array
    return interpolate_segment(track, i, time, offset);
}

void sample_channel_group_00b77220(const ChannelGroup& group, float position[3], float rotation[3],
    float scale[3], float time) {
    float* outputs[9] = {&position[0], &position[1], &position[2], &rotation[0], &rotation[1],
        &rotation[2], &scale[0], &scale[1], &scale[2]};
    for (std::size_t i = 0; i < 9 && i < group.channels.size(); ++i) {
        if (group.channels[i]) *outputs[i] = sample_track_00b76ce0(*group.channels[i], time);
    }
}

bool evaluate_animator_pose_00b77360(const NodeAnimatorView& animator, std::uint32_t group,
    float time, float position[3], float rotation[3], float scale[3]) {
    if (group >= animator.groups.size() || animator.groups[group] == nullptr) return false;
    for (int i = 0; i < 3; ++i) {
        position[i] = animator.translation[i];
        rotation[i] = animator.angles[i];
        scale[i] = 1.0f;
    }
    sample_channel_group_00b77220(*animator.groups[group], position, rotation, scale, time);
    return true;
}

bool evaluate_animator_channel_00b77430(const NodeAnimatorView& animator, std::uint32_t group,
    std::uint32_t channel, float time, float& out) {
    // Native indexes the channel array without a bound check.
    if (group >= animator.groups.size() || animator.groups[group] == nullptr) return false;
    if (channel >= animator.groups[group]->channels.size()) return false;
    const AnimTrack* track = animator.groups[group]->channels[channel];
    if (!track) return false;
    out = sample_track_00b76ce0(*track, time);
    return true;
}


namespace {
bool same_name_no_case(const std::string& a, const std::string& b) noexcept {
    // Native: lengths must match, two empty names match, otherwise __stricmp.
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i]))) return false;
    }
    return true;
}
}  // namespace

SceneNodeView* find_child_by_name_00b6e750(SceneNodeView& node, const std::string& name, bool recursive) {
    for (SceneNodeView* child : node.children) {
        if (same_name_no_case(child->name, name)) return child;
    }
    if (recursive) {
        for (SceneNodeView* child : node.children) {
            if (SceneNodeView* hit = find_child_by_name_00b6e750(*child, name, true)) return hit;
        }
    }
    return nullptr;
}

void collect_descendants_matching_00b6f9a0(SceneNodeView& node, const std::string& filter,
    std::vector<SceneNodeView*>& out) {
    for (SceneNodeView* child : node.children) {
        collect_descendants_matching_00b6f9a0(*child, filter, out);
        // strstr on the copied name; an empty name never matches a non-empty filter.
        if (filter.empty() || (!child->name.empty() && child->name.find(filter) != std::string::npos))
            out.push_back(child);
    }
}

float node_visibility_from_channel_00b6e150(float sampled) noexcept {
    const float v = f(1.0 - sampled);
    if (0.0f > v) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

float camera_fov_from_channel_00b70de0(float sampled) noexcept {
    const float inverse = f(1.0 / sampled);
    const float half = f(std::atan(static_cast<double>(inverse)));
    return f(static_cast<double>(half) + half);
}

std::vector<std::vector<std::string>> resource_dump_rows_00b811c0(std::vector<ResourceDumpEntry> entries) {
    std::vector<std::vector<std::string>> rows;
    rows.push_back({"Name", "Size", "RefCounter", "Size / RefCounter"});
    std::stable_sort(entries.begin(), entries.end(),
        [](const ResourceDumpEntry& a, const ResourceDumpEntry& b) { return a.size > b.size; });
    for (const ResourceDumpEntry& e : entries) {
        rows.push_back({e.name, std::to_string(e.size), std::to_string(e.refcount),
            e.refcount != 0 ? std::to_string(e.size / e.refcount) : std::string()});
    }
    return rows;
}

std::string resource_dump_file_name_00b811c0(const std::string& suffix) {
    return "ResourceDump_" + suffix + ".csv";
}

// ---------------------------------------------------------------------------
// cc13-scene3 bodies (docs/CC13_SCENE_3.md).

namespace {
// FLD dword / FSTP dword: the value round-trips unchanged except that a signalling NaN is quieted.
float x87_reload(float v) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &v, sizeof bits);
    if ((bits & 0x7F800000u) == 0x7F800000u && (bits & 0x007FFFFFu) != 0u) bits |= 0x00400000u;
    float r;
    std::memcpy(&r, &bits, sizeof r);
    return r;
}

void update_hires_projection(HiresCamera& camera) noexcept {
    for (float& v : camera.projection_1e0) v = x87_reload(v);
    for (int k = 0; k < 16; ++k) camera.projection_copy_2a0[k] = x87_reload(camera.projection_1e0[k]);
    camera.flags_2f0 = (camera.flags_2f0 & 0xFFFFFF4Bu) | 8u;
}
} // namespace

std::uint8_t camera_render_hires_tiles_00b708e0(HiresCamera& camera, const char* file, std::int32_t width,
    std::int32_t height, HiresTileCallback callback, void* context, HiresTileServices& s) {
    std::uint8_t result = 0;
    const bool was_in_scene = s.in_scene_2c();
    if (was_in_scene) s.end_scene_14();
    const std::int32_t vw = s.viewport_size_00b1f740()[0];
    const std::int32_t vh = s.viewport_size_00b1f740()[1];
    const std::int32_t tx = width / vw;   // IDIV: truncation toward zero
    const std::int32_t ty = height / vh;
    void* const target = s.create_surface_84(tx * vw, ty * vh, 0x16, 2);
    float saved[16];
    std::memcpy(saved, camera.projection_1e0, sizeof saved); // REP MOVSD
    camera.projection_1e0[0] = f(static_cast<double>(tx) * camera.projection_1e0[0]);
    camera.projection_1e0[5] = f(static_cast<double>(ty) * camera.projection_1e0[5]);
    if (tx > 0) {
        const float column_span = static_cast<float>(tx - 1); // CVTSI2SS
        std::int32_t x = 0;
        for (std::int32_t j = 0; j < tx; ++j) {
            camera.projection_1e0[8] = f(static_cast<double>(column_span) - 2.0 * j);
            if (ty > 0) {
                const float row_span = static_cast<float>(ty - 1);
                std::int32_t y = 0;
                for (std::int32_t i = 0; i < ty; ++i) {
                    camera.projection_1e0[9] = f(-(static_cast<double>(row_span) - 2.0 * i));
                    update_hires_projection(camera);
                    s.begin_scene_0c();
                    result = callback(context);
                    s.end_scene_14();
                    alignas(4) unsigned char holder[0x3C];
                    s.holder_construct_00b3f450(holder);
                    if (s.get_back_buffer_7c(holder)) {
                        s.surface_copy_from_38(target, holder, 0, HiresTileRect{x, y, x + vw, y + vh});
                    }
                    s.holder_destroy_00b3f4e0(holder);
                    y += vh;
                }
            }
            x += vw;
        }
    }
    std::memcpy(camera.projection_1e0, saved, sizeof saved);       // MOVSS, bit-exact
    std::memcpy(camera.projection_copy_2a0, saved, sizeof saved);
    camera.flags_2f0 = (camera.flags_2f0 & 0xFFFFFF4Bu) | 8u;
    s.surface_save_34(target, file, 0);
    s.surface_delete_04(target, 1);
    if (was_in_scene) s.begin_scene_0c();
    return result;
}

void flare_build_geometry_00b8dd60(cc13_scene2::FlareView& flare, const void* elements_source,
    std::uint32_t& vertex_layout_1cc, FlareGeometryServices& s) {
    s.assign_elements_00b8d850(flare, elements_source);
    void* const mesh = s.new_mesh_object_00b73b60();
    const auto count = static_cast<std::uint32_t>(flare.elements.size()); // flare +190h
    void* const declaration = s.create_vertex_declaration_38("simplecolor2.mvfm");
    void* const vertices = s.create_vertex_buffer_5c(count * 4u, 1, declaration);
    s.release(declaration);
    vertex_layout_1cc = s.create_vertex_layout_40(declaration); // the record keeps the (released) pointer
    s.geometry_set_vertex_stream_00b73bb0(mesh, 0, vertices);
    s.release(vertices);
    void* const indices = s.create_index_buffer_60(count * 6u, 1, 0x65);
    std::uint16_t* out = s.lock_indices_0c(indices);
    for (std::uint32_t i = 0; i < count; ++i, out += 6) {
        const auto base = static_cast<std::uint16_t>(i * 4u);
        out[0] = base;
        out[1] = static_cast<std::uint16_t>(base + 1);
        out[2] = static_cast<std::uint16_t>(base + 2);
        out[3] = static_cast<std::uint16_t>(base + 2);
        out[4] = static_cast<std::uint16_t>(base + 3);
        out[5] = base;
        cc13_scene2::FlareElement& e = *flare.elements[i];
        for (int k = 0; k < 2; ++k) {
            // Compared with a fresh empty string: an empty name gives a null rect.
            e.rects[k] = e.names[k].empty() ? nullptr : s.atlas_find_item_00aefb20(e.names[k].c_str());
        }
        e.size[0] = x87_reload(e.origin[0]);
        e.size[1] = x87_reload(e.origin[1]);
    }
    s.unlock_indices_10(indices);
    s.geometry_set_index_stream_00b73b70(mesh, indices);
    s.release(indices);
    void* const flare_material = s.material_create_00535320("flare.mshd");
    void* const flare2_material = s.material_create_00535320("flare2.mshd");
    std::uint32_t n0 = 0, n1 = 0, n2 = 0;
    for (std::uint32_t i = 0; i < count; ++i) {
        cc13_scene2::FlareElement& e = *flare.elements[i];
        switch (e.type) {
        case 0:
            if (n1 == 0 && n0 == 0) {
                s.material_set_texture_slot_00b189f0(flare_material, 0, s.atlas_item_texture(e.rects[0]));
            }
            ++n0;
            break;
        case 1:
            if (n1 == 0 && n0 == 0) {
                s.material_set_texture_slot_00b189f0(flare_material, 0, s.atlas_item_texture(e.rects[0]));
            }
            ++n1;
            break;
        case 2:
            if (n2 == 0) {
                for (std::uint32_t k = 0; k < 2; ++k) {
                    void* const texture = e.rects[k] != nullptr ? s.atlas_item_texture(e.rects[k]) : nullptr;
                    s.material_set_texture_slot_00b189f0(flare2_material, k, texture);
                }
            }
            ++n2;
            break;
        default:
            break;
        }
    }
    std::uint32_t first_vertex = 0;
    std::uint32_t first_index = 0;
    const auto add_section = [&](std::uint32_t quads, void* material) {
        void* const section = s.new_draw_section_00b85ee0();
        FlareDrawRanges ranges;
        ranges.first_vertex_0c = first_vertex;
        ranges.vertex_count_10 = quads * 4u;
        ranges.first_index_14 = first_index;
        ranges.primitive_count_18 = (quads * 4u) >> 1;
        s.section_write_ranges(section, ranges); // the listing writes through a possibly null section
        s.section_set_material_00b864c0(section, material);
        s.section_append_vertex_stream_00b85b80(section, s.geometry_vertex_stream_00b73260(mesh, 0));
        s.section_set_vertex_layout_00b86650(section, vertex_layout_1cc);
        s.geometry_append_section_00b73c60(mesh, section);
        s.release(section);
    };
    if (n0 != 0) {
        add_section(n0, flare_material); // the listing stores 0 for both starts here
        first_index = n0 * 6u;
        first_vertex = n0 * 4u;
    }
    if (n1 != 0) {
        add_section(n1, flare_material);
        first_vertex += n1 * 4u;
        first_index += n1 * 6u;
    }
    if (n2 != 0) add_section(n2, flare2_material);
    s.release(flare_material);
    s.release(flare2_material);
    s.set_geometry_00b75170(flare, 0, mesh, -1.0f, -1.0f);
    s.release(mesh);
    s.initialize_occlusion_00b8dac0(flare);
}
}  // namespace bsp::cc13_scene
