#include "bsp/scene_graph_cc13.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>

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
    const double turns = std::floor(static_cast<double>(time) / length);
    const float wrapped = f(shifted - turns * length);
    if (cycles) {
        const double half = wrapped > time ? 0.5 : -0.5;
        *cycles = -ftol((static_cast<double>(wrapped) - time) / length + half);
    }
    return wrapped;
}

HermiteBasis hermite_basis_00b75520(float s) {
    const float s2 = f(static_cast<double>(s) * s);
    const float s3 = f(static_cast<double>(s2) * s);
    HermiteBasis h{};
    h.p1 = f(3.0 * s2 - s3 - s3);
    h.p0 = f(1.0 - h.p1);
    h.t1 = f(static_cast<double>(s3) - s2);
    h.t0 = f(s + (static_cast<double>(h.t1) - s2));
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

double bezier_00b75580(float p0, float p1, float p2, float p3, float s) {
    const float s2 = f(static_cast<double>(s) * s);
    const float c = f((static_cast<double>(p1) - p0) * 3.0);
    const float b = f(3.0 * (static_cast<double>(p2) - p1) - c);
    const float a = f(((static_cast<double>(p3) - p0) - c) - b);
    const float s3 = f(static_cast<double>(s2) * s);
    return static_cast<double>(s) * c + (static_cast<double>(b) * s2 + static_cast<double>(a) * s3) + p0;
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
}  // namespace bsp::cc13_scene
